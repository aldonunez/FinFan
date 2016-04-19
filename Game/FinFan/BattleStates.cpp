/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "BattleStates.h"
#include "Battle.h"
#include "BattleCalc.h"
#include "BattleEffects.h"
#include "BattleMenus.h"
#include "Player.h"
#include "SceneStack.h"
#include "Sound.h"
#include "Sprite.h"
#include "Utility.h"


namespace Battle
{

const int LastWonState = 3;
const int PlayerFlag = 0x80;

const int EnemyFleeFrames = 30;
const int EnemyDieFrames = 30;

const ALLEGRO_COLOR White = { 1, 1, 1, 1 };
const ALLEGRO_COLOR Black = { 0, 0, 0, 0 };
const ALLEGRO_COLOR Green = { 0.5, 1, 0.5, 1 };


typedef void (*UpdateFunc)();

UpdateFunc  curUpdate;
int         gTimer;

Command     commands[Player::PartySize];
int         shuffledActors[MaxActors];
int         curActorIndex;
Command     curCmd;
int         gNextPlayerId;
bool        gAtEndOfRound;

int gXPWon;
int gGWon;
int gWonState;
int gLevelUpPlayerId;


void GotoFirstMenu();
void GotoFirstCommand();
void GotoOpeningMessage();
void GotoRunCommand();
void GotoNextCommand();
void GotoNumbers();
void GotoPlayerMagicEffect();
void GotoEnemyFight();
void GotoRunMenu();
void GotoCloseMenu( int nextPlayerId );
void GotoPlayerFight();
void GotoPlayerItem();
void GotoPlayerCastMagic();
void GotoDisengage();


int FindNextActivePlayer()
{
    return FindNextActivePlayer( curActorIndex );
}

int FindPrevActivePlayer()
{
    return FindPrevActivePlayer( curActorIndex );
}

void UpdateState()
{
    curUpdate();
}

void UpdateOpeningMessage()
{
    if ( gTimer == 0 )
    {
        gMessage[0] = '\0';

        if ( gEncounter == Encounter_EnemyFirst )
            GotoFirstCommand();
        else
            GotoFirstMenu();
    }
    else
    {
        gTimer--;
    }
}

void GotoOpeningMessage()
{
    if ( gEncounter == Encounter_PlayerFirst )
        strcpy_s( gMessage, "Chance to strike first" );
    else if ( gEncounter == Encounter_EnemyFirst )
        strcpy_s( gMessage, "Monsters strike first" );
    else
        gMessage[0] = '\0';

    gTimer = 90;

    curUpdate = UpdateOpeningMessage;
}

void UpdateOpenMenu()
{
    int playerId = curActorIndex;
    Sprite* sprite = playerSprites[playerId];

    if ( sprite->GetX() > PartyX - 16 )
    {
        sprite->SetX( sprite->GetX() - 2 );
    }
    else
    {
        GotoRunMenu();
    }
}

void GotoOpenMenu( int playerId )
{
    Command& cmd = commands[playerId];
    int _class = Player::Party[playerId]._class;
    playerSprites[playerId]->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );

    cmd.actorParty = Party_Players;
    cmd.actorIndex = playerId;

    curActorIndex = playerId;

    curUpdate = UpdateOpenMenu;
}

void GotoFirstMenu()
{
    int firstPlayerId = FindNextActivePlayer( -1 );

    if ( firstPlayerId != NoneIndex )
        GotoOpenMenu( firstPlayerId );
    else
        GotoFirstCommand();
}

void UpdateRunMenu()
{
    MenuAction menuAction = Menu_None;
    Menu* nextMenu = nullptr;

    menuAction = activeMenu->Update( nextMenu );

    if ( menuAction == Menu_Push )
    {
        nextMenu->prevMenu = activeMenu;
        activeMenu = nextMenu;
    }
    else if ( menuAction == Menu_Pop )
    {
        Menu* menu = activeMenu;
        activeMenu = activeMenu->prevMenu;
        delete menu;

        if ( activeMenu == nullptr )
        {
            int next = FindPrevActivePlayer();
            if ( next == NoneIndex )
                next = curActorIndex;

            GotoCloseMenu( next );
        }
    }
    else if ( menuAction == Menu_PopAll )
    {
        while ( activeMenu != nullptr )
        {
            Menu* menu = activeMenu;
            activeMenu = activeMenu->prevMenu;
            delete menu;
        }

        int next = FindNextActivePlayer();
        GotoCloseMenu( next );
    }
}

void GotoRunMenu()
{
    int playerId = curActorIndex;
    int _class = Player::Party[playerId]._class;
    playerSprites[playerId]->SetFrames( standFrames, _class * PlayerSpriteRowHeight );

    activeMenu = new BattleMenu();
    activeMenu->prevMenu = nullptr;

    curUpdate = UpdateRunMenu;
}

void UpdateCloseMenu()
{
    int playerId = curActorIndex;
    Sprite* sprite = playerSprites[playerId];

    if ( sprite->GetX() < PartyX )
    {
        sprite->SetX( sprite->GetX() + 2 );
    }
    else
    {
        UpdateIdleSprite( playerId );

        if ( gNextPlayerId == NoneIndex )
        {
            UndoAllCommands();
            GotoFirstCommand();
        }
        else
        {
            if ( gNextPlayerId > curActorIndex )
            {
                CommitCommand( curActorIndex );
            }
            else if ( gNextPlayerId < curActorIndex )
            {
                UndoCommand( gNextPlayerId );
            }

            GotoOpenMenu( gNextPlayerId );
        }
    }
}

void GotoCloseMenu( int nextPlayerId )
{
    int playerId = curActorIndex;
    int _class = Player::Party[playerId]._class;
    playerSprites[playerId]->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );

    gNextPlayerId = nextPlayerId;

    curUpdate = UpdateCloseMenu;
}

void GotoFirstCommand()
{
    gAtEndOfRound = false;
    PrepActions();
    ResetRunningCommands();
    GotoNextCommand();
}

void UpdateWon()
{
    if ( gTimer == 0 )
    {
        if ( gWonState == 0 )
        {
            gWonState = 1;
            gTimer = 120;

            sprintf_s( gMessage, "Gained %d Gil", gGWon );
        }
        else if ( gWonState == 1 )
        {
            sprintf_s( gMessage, "Gained %d Exp.", gXPWon );

            gTimer = 120;
            gLevelUpPlayerId = CalcNextLevelUp( -1 );

            if ( gLevelUpPlayerId < Player::PartySize )
                gWonState = 2;
            else
                gWonState = LastWonState;
        }
        else if ( gWonState == 2 )
        {
            sprintf_s( gMessage, "%s level up!", Player::Party[gLevelUpPlayerId].name );

            gTimer = 120;
            gLevelUpPlayerId = CalcNextLevelUp( gLevelUpPlayerId );

            if ( gLevelUpPlayerId == Player::PartySize )
                gWonState = LastWonState;
            // else, stay in state 2
        }
        else if ( gWonState == LastWonState )
        {
            if ( Input::IsKeyPressing( ConfirmKey ) )
                LeaveBattle();
        }
    }
    else
    {
        gTimer--;

        if ( Input::IsKeyPressing( ConfirmKey ) )
            gTimer = 0;
    }
}

void GotoWon()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( Player::IsPlayerAlive( i ) )
        {
            int _class = Player::Party[i]._class;
            playerSprites[i]->SetFrames( fanfareFrames, _class * PlayerSpriteRowHeight );
        }
    }

    int xp = 0;
    int g = 0;

    for ( int i = 0; i < _countof( enemies ); i++ )
    {
        int type = enemies[i].Type;

        if ( type != InvalidEnemyType )
        {
            xp += enemyAttrs[type].Xp;
            g += enemyAttrs[type].G;
        }
    }

    gXPWon = xp;
    gGWon = g;

    Player::AddXP( xp );
    Player::AddG( g );

    Sound::PlayTrack( Sound_Victory, 0, true );

    if ( gXPWon > 0 || gGWon > 0 )
        gWonState = 0;
    else
        gWonState = LastWonState;

    gTimer = 120;

    curUpdate = UpdateWon;
}

void UpdateLost()
{
    if ( Input::IsKeyPressing( ConfirmKey ) || Input::IsKeyPressing( CancelKey ) )
        SceneStack::SwitchScene( SceneId_Title );
}

void GotoLost()
{
    strcpy_s( gMessage, "The party perished." );

    Sound::PlayTrack( Sound_Dead, 0, true );

    curUpdate = UpdateLost;
}

void GotoCheckWonOrLost()
{
    UpdateAllIdleSprites();

    if ( HasLost() )
        GotoLost();
    else if ( HasWon() )
        GotoWon();
    else
        GotoFirstMenu();
}

void GotoAutoHP()
{
    resultCount = 0;

    CalcEnemyAutoHP();

    // if all enemies die by poison, then the battle was won,
    // even if players could have died by poison

    if ( !HasWon() )
        CalcPlayerAutoHP();

    if ( resultCount > 0 )
    {
        gAtEndOfRound = true;
        curCmd.actorParty = Party_None;
        curCmd.action = Action_None;

        GotoNumbers();
    }
    else
    {
        GotoCheckWonOrLost();
    }
}

void GotoNextCommand()
{
    if ( HasLost() )
    {
        GotoLost();
        return;
    }

    if ( HasWon() )
    {
        GotoWon();
        return;
    }

    if ( curActorIndex < MaxActors )
        curActorIndex++;

    if ( AreCommandsFinished() )
    {
        // no matter what kind of encounter was started, after the first round, it's normal
        gEncounter = Encounter_Normal;

        GotoAutoHP();
    }
    else
    {
        GotoRunCommand();
    }
}

void UpdateEngage()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];

    if ( sprite->GetX() > PartyX - 16 )
    {
        sprite->SetX( sprite->GetX() - 1 );
    }
    else
    {
        if ( cmd.action == Action_Fight )
            GotoPlayerFight();
        else if ( cmd.action == Action_Magic )
            GotoPlayerCastMagic();
        else if ( cmd.action == Action_Item )
            GotoPlayerItem();
        else
            GotoDisengage();
    }
}

void GotoEngage()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    playerSprites[playerId]->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );

    curUpdate = UpdateEngage;
}

void UpdateRunAway()
{
    int playerId = FindNextActivePlayer( -1 );
    Sprite* sprite = playerSprites[playerId];

    if ( sprite->GetX() < 256 )
    {
        for ( int i = 0; i < Player::PartySize; i++ )
        {
            if ( Player::IsPlayerAlive( i ) )
            {
                playerSprites[i]->SetX( playerSprites[i]->GetX() + 2 );
            }
        }
    }
    else
    {
        LeaveBattle();
    }
}

void GotoRunAway()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( Player::IsPlayerAlive( i ) )
        {
            int _class = Player::Party[i]._class;
            playerSprites[i]->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );
            playerSprites[i]->SetBitmapFlags( ALLEGRO_FLIP_HORIZONTAL );
        }
    }

    curUpdate = UpdateRunAway;
}

void UpdateRunAwayFailed()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];

    if ( sprite->GetFlags() == ALLEGRO_FLIP_HORIZONTAL )
    {
        if ( sprite->GetX() < 256 - 16 )
        {
            sprite->SetX( sprite->GetX() + 2 );
        }
        else
        {
            sprite->SetBitmapFlags( 0 );
        }
    }
    else
    {
        if ( sprite->GetX() > PartyX )
        {
            sprite->SetX( sprite->GetX() - 2 );
        }
        else
        {
            UpdateIdleSprite( playerId );

            GotoNextCommand();
        }
    }
}

void GotoRunAwayFailed()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];

    sprite->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );
    sprite->SetBitmapFlags( ALLEGRO_FLIP_HORIZONTAL );

    curUpdate = UpdateRunAwayFailed;
}

void GotoEnemyMagicEffect()
{
    Command& cmd = curCmd;
    const char* name = nullptr;

    if ( cmd.action == Action_Magic )
    {
        int spellIndex = cmd.actionId;
        int level = spellIndex / 8;
        int c = (spellIndex % 8) + 1;

        name = Player::GetMagicName( level, c );
    }
    else
    {
        int spellIndex = cmd.actionId;
        name = Player::GetSpecialName( spellIndex );
    }

    strcpy_s( gMessage, name );

    GotoPlayerMagicEffect();
}

void UpdateEnemyFlee()
{
    Command& cmd = curCmd;

    if ( gTimer == 0 )
    {
        // when enemies flee, it's like they were never in battle
        // they don't give XP nor Gil

        enemies[cmd.actorIndex].Type = InvalidEnemyType;

        GotoNextCommand();
    }
    else
    {
        gTimer--;

        const int Step = 255 / EnemyFleeFrames;
        int c = gTimer * Step;
        enemies[cmd.actorIndex].Color = al_map_rgba( c, c, c, c );
    }
}

void GotoEnemyFlee()
{
    gTimer = EnemyFleeFrames;

    curUpdate = UpdateEnemyFlee;
}

void UpdateEnemyEngage()
{
    int enemyId = shuffledActors[curActorIndex];
    Enemy& enemy = enemies[enemyId];

    if ( gTimer == 0 )
    {
        enemy.Color = White;

        if ( curCmd.action == Action_Fight )
            GotoEnemyFight();
        else if ( curCmd.action == Action_Magic
            || curCmd.action == Action_Special )
            GotoEnemyMagicEffect();
        else if ( curCmd.action == Action_Run )
            GotoEnemyFlee();
        else
            GotoNextCommand();
    }
    else
    {
        gTimer--;

        if ( (gTimer % 8) < 4 )
            enemy.Color = White;
        else
            enemy.Color = Black;
    }
}

void GotoEnemyEngage()
{
    gTimer = 24;

    curUpdate = UpdateEnemyEngage;
}

void GotoTryRecoverDisabling()
{
    int actorId = shuffledActors[curActorIndex];

    if ( (actorId & PlayerFlag) != 0 )
    {
        int playerId = actorId & ~PlayerFlag;
        Player::Character& player = Player::Party[playerId];

        if ( (player.status & Status_Paralysis) != 0 )
        {
            int r = GetNextRandom( 100 );
            if ( r < 25 )
                player.status &= ~Status_Paralysis;
        }

        if ( (player.status & Status_Sleep) != 0 )
        {
            int r = GetNextRandom( 81 );
            if ( r < player.maxHp )
                player.status &= ~Status_Sleep;
        }

        UpdateIdleSprite( playerId );
    }
    else
    {
        int enemyId = actorId;
        Enemy& enemy = enemies[enemyId];

        if ( (enemy.Status & Status_Paralysis) != 0 )
        {
            int r = GetNextRandom( 256 );
            if ( r < 25 )
                enemy.Status &= ~Status_Paralysis;
        }

        if ( (enemy.Status & Status_Sleep) != 0 )
        {
            int r = GetNextRandom( 81 );
            if ( r < enemyAttrs[enemy.Type].Hp )
                enemy.Status &= ~Status_Sleep;
        }
    }

    GotoNextCommand();
}

void GotoRunCommand()
{
    int actorId = shuffledActors[curActorIndex];

    if ( (actorId & PlayerFlag) != 0 )
    {
        int playerId = actorId & ~PlayerFlag;
        Player::Character& player = Player::Party[playerId];

        curCmd = commands[playerId];

        if ( gEncounter == Encounter_EnemyFirst )
            GotoNextCommand();
        else if ( !Player::IsPlayerAlive( curCmd.actorIndex ) )
            GotoNextCommand();
        else if ( (player.status & (Status_Paralysis | Status_Sleep)) != 0 )
            GotoTryRecoverDisabling();
        else if ( curCmd.action == Action_Fight )
            GotoEngage();
        else if ( curCmd.action == Action_Magic )
            GotoEngage();
        else if ( curCmd.action == Action_Item )
            GotoEngage();
        else if ( curCmd.action == Action_Run )
        {
            if ( CanRunAway() )
                GotoRunAway();
            else
                GotoRunAwayFailed();
        }
        else
            GotoNextCommand();
    }
    else
    {
        int enemyId = actorId;
        Enemy& enemy = enemies[enemyId];

        if ( gEncounter == Encounter_PlayerFirst )
            GotoNextCommand();
        else if ( enemy.Type == InvalidEnemyType || enemy.Hp == 0 )
            GotoNextCommand();
        else if ( (enemy.Status & (Status_Paralysis | Status_Sleep)) != 0 )
            GotoTryRecoverDisabling();
        else if ( (enemy.Status & Status_Confusion) != 0 )
        {
            if ( TryRecoverConfuse( curActorIndex ) )
                GotoNextCommand();
            else
            {
                MakeConfuseAction( curActorIndex );
                GotoEnemyEngage();
            }
        }
        else
        {
            MakeEnemyAction( curActorIndex );
            GotoEnemyEngage();
        }
    }
}

void UpdateDisengage()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];

    if ( sprite->GetX() < PartyX )
    {
        sprite->SetX( sprite->GetX() + 1 );
    }
    else
    {
        UpdateIdleSprite( playerId );
        GotoNextCommand();
    }
}

void GotoDisengage()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];

    sprite->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );
    curUpdate = UpdateDisengage;
}

void UpdateEnemyDie()
{
    Command& cmd = curCmd;

    if ( gTimer == 0 )
    {
        for ( int i = 0; i < resultCount; i++ )
        {
            int index = actionResults[i].TargetIndex;
            if ( actionResults[i].TargetParty == Party_Enemies && actionResults[i].Died )
            {
                Enemy* enemy = &enemies[index];

                enemy->Prev->Next = enemy->Next;
                enemy->Next->Prev = enemy->Prev;
                enemy->Next = nullptr;
                enemy->Prev = nullptr;

                enemy->Counter->Count--;
            }
        }

        if ( gAtEndOfRound )
            GotoCheckWonOrLost();
        else if ( cmd.actorParty == Party_Players )
            GotoDisengage();
        else
            GotoNextCommand();
    }
    else
    {
        gTimer--;

        const int Step = 255 / EnemyDieFrames;
        int c = gTimer * Step;

        for ( int i = 0; i < resultCount; i++ )
        {
            int index = actionResults[i].TargetIndex;
            if ( actionResults[i].TargetParty == Party_Enemies && actionResults[i].Died )
                enemies[index].Color = al_map_rgba( c, 0, c, c );
        }
    }
}

void GotoEnemyDie()
{
    gTimer = EnemyDieFrames;

    curUpdate = UpdateEnemyDie;
}

void UpdateNumbers()
{
    Command& cmd = curCmd;

    if ( gTimer == 0 )
    {
        bool enemyDied = false;

        for ( int i = 0; i < resultCount; i++ )
        {
            if ( magicEffects[i] != nullptr )
            {
                delete magicEffects[i];
                magicEffects[i] = nullptr;
            }

            if ( !actionResults[i].Missed
                && actionResults[i].TargetParty == Party_Enemies && actionResults[i].Died )
                enemyDied = true;
        }

        UpdateIdleSprites();

        if ( enemyDied )
            GotoEnemyDie();
        else if ( gAtEndOfRound )
            GotoCheckWonOrLost();
        else if ( cmd.actorParty == Party_Players )
            GotoDisengage();
        else
            GotoNextCommand();
    }
    else
    {
        gTimer--;

        for ( int i = 0; i < resultCount; i++ )
        {
            if ( magicEffects[i] != nullptr )
                magicEffects[i]->Update();
        }
    }
}

void GotoNumbers()
{
    Command& cmd = curCmd;

    gTimer = 40;

    for ( int i = 0; i < resultCount; i++ )
    {
        int index = actionResults[i].TargetIndex;
        Bounds bounds;
        if ( actionResults[i].TargetParty == Party_Enemies )
        {
            bounds = enemies[index].Bounds;
        }
        else
        {
            int x = PartyX;
            int y = PartyY + (index * PlayerSpriteHeight);

            if ( cmd.actorParty == Party_Players && index == cmd.actorIndex )
                x -= 16;

            Bounds bounds2 = { x, y, PlayerSpriteWidth, PlayerSpriteHeight };

            bounds = bounds2;
        }

        if ( actionResults[i].Missed )
        {
            int index = actionResults[i].TargetIndex;
            // in font B this is "Miss"
            char numbers[11] = "ABCD";
            magicEffects[i] = new NumbersEffect( bounds, numbers );
            magicEffects[i]->Update();
        }
        else if ( actionResults[i].DealtDamage )
        {
            int index = actionResults[i].TargetIndex;
            char numbers[11];
            ALLEGRO_COLOR color = White;
            int number = actionResults[i].Damage;

            if ( number < 0 )
            {
                color = Green;
                number = -number;
            }

            sprintf_s( numbers, "%d", number );
            magicEffects[i] = new NumbersEffect( bounds, numbers, color );
            magicEffects[i]->Update();
        }
    }

    curUpdate = UpdateNumbers;
}

void UpdatePlayerFight()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];
    int weaponId = Player::Party[playerId].weaponId;

    if ( gTimer == 0 )
    {
        gShowWeapon = false;

        if ( curEffect != nullptr )
        {
            delete curEffect;
            curEffect = nullptr;
        }

        UpdateIdleSprite( playerId );

        if ( resultCount > 0 )
            GotoNumbers();
        else
            GotoDisengage();
    }
    else
    {
        gTimer--;

        if ( (gTimer + 4) % 16 < 8 
            && ((_class != Class_BlackBelt && _class != Class_Master) || weaponId != NoWeapon) )
        {
            weaponSprite->SetX( PartyX - 16 + 16 );
            weaponSprite->SetY( PartyY + cmd.actorIndex * PlayerSpriteHeight - 8 );
            weaponSprite->SetBitmapFlags( ALLEGRO_FLIP_HORIZONTAL );
        }
        else
        {
            weaponSprite->SetX( PartyX - 16 - 16 );
            weaponSprite->SetY( PartyY + cmd.actorIndex * PlayerSpriteHeight );
            weaponSprite->SetBitmapFlags( 0 );
        }

        weaponSprite->Update();

        if ( curEffect != nullptr )
            curEffect->Update();

        if ( gTimer == 30 )
        {
            if ( strikeResult.CritHit )
            {
                gShowFullScreenColor = true;
                gFullScreenColor = al_map_rgb( 255, 255, 255 );
            }
        }
        else
        {
            gShowFullScreenColor = false;
        }
    }
}

void GotoPlayerFight()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];
    int weaponId = Player::Party[playerId].weaponId;

    sprite->SetFrames( strikeFrames, _class * PlayerSpriteRowHeight );
    sprite->SetTimer( 2 );

    if ( (_class == Class_BlackBelt || _class == Class_Master) && weaponId == NoWeapon )
    {
        weaponSprite->SetFrames( punchFrames, 40 * 16 );
        weaponSprite->SetTimer( 2 );

        weaponSprite->SetX( PartyX - 16 - 16 );
        weaponSprite->SetY( PartyY + cmd.actorIndex * PlayerSpriteHeight );
        weaponSprite->SetBitmapFlags( 0 );
    }
    else
    {
        weaponSprite->SetFrames( weaponFrames, weaponId * 16 );
        weaponSprite->SetTimer( 2 );

        weaponSprite->SetX( PartyX - 16 + 16 );
        weaponSprite->SetY( PartyY + cmd.actorIndex * PlayerSpriteHeight - 8 );
        weaponSprite->SetBitmapFlags( ALLEGRO_FLIP_HORIZONTAL );
    }

    gTimer = 32;
    gShowWeapon = true;

    CalcPlayerPhysDamage();

    if ( !strikeResult.Missed )
    {
        // turn on strike effect
        int index = strikeResult.TargetIndex;
        int effectIndex = weaponId;
        if ( weaponId == NoWeapon )
        {
            if ( _class == Class_BlackBelt || _class == Class_Master )
                effectIndex = 40;
            else
                effectIndex = 40 + playerId;
        }
        curEffect = new StrikeEffect( enemies[index].Bounds, effectIndex );

        Sound::PlayEffect( SEffect_Strike );
    }

    curUpdate = UpdatePlayerFight;
}

void UpdateEnemyFight()
{
    Command& cmd = curCmd;
    int playerId = strikeResult.TargetIndex;
    Sprite* sprite = playerSprites[playerId];

    if ( gTimer == 0 )
    {
        if ( curEffect != nullptr )
        {
            delete curEffect;
            curEffect = nullptr;
        }

        sprite->Flash( false );

        if ( resultCount > 0 )
        {
            GotoNumbers();
        }
        else
            GotoNextCommand();
    }
    else
    {
        gTimer--;

        if ( curEffect != nullptr )
            curEffect->Update();

        if ( gTimer == 30 )
        {
            if ( strikeResult.CritHit )
            {
                gShowFullScreenColor = true;
                gFullScreenColor = al_map_rgb( 255, 255, 255 );
            }
        }
        else
        {
            gShowFullScreenColor = false;
        }
    }
}

void GotoEnemyFight()
{
    Command& cmd = curCmd;

    gTimer = 32;

    CalcEnemyPhysDamage();

    if ( !strikeResult.Missed )
    {
        // turn on strike effect
        int index = strikeResult.TargetIndex;
        int y = PartyY + index * PlayerSpriteHeight;
        Bounds bounds = { PartyX, y, PlayerSpriteWidth, PlayerSpriteHeight };

        Sprite* sprite = playerSprites[index];
        sprite->Flash( true );

        curEffect = new StrikeEffect( bounds, 0 );

        Sound::PlayEffect( SEffect_Hurt );
    }

    curUpdate = UpdateEnemyFight;
}

void UpdatePlayerMagicEffect()
{
    if ( gTimer == 0 )
    {
        gMessage[0] = '\0';

        for ( int i = 0; i < resultCount; i++ )
        {
            if ( magicEffects[i] != nullptr )
            {
                delete magicEffects[i];
                magicEffects[i] = nullptr;
            }
        }

        GotoNumbers();
    }
    else
    {
        gTimer--;

        for ( int i = 0; i < resultCount; i++ )
        {
            if ( magicEffects[i] != nullptr )
                magicEffects[i]->Update();
        }
    }
}

void GotoPlayerMagicEffect()
{
    Command& cmd = curCmd;
    int spellIndex = 0;

    gTimer = 32;
    resultCount = 0;

    if ( !IsMute( cmd.actorParty, cmd.actorIndex ) )
    {
        CalcMagicEffect();

        if ( cmd.actorParty == Party_Players )
        {
            spellIndex = cmd.actionId;
            int level = spellIndex / 8;
            int playerId = cmd.actorIndex;

            Player::Party[playerId].spellCharge[level]--;
        }
    }

    for ( int i = 0; i < resultCount; i++ )
    {
        // turn on strike effect
        int index = actionResults[i].TargetIndex;
        if ( actionResults[i].TargetParty == Party_Enemies )
        {
            magicEffects[i] = new SpellEffect( enemies[index].Bounds, spellIndex );
        }
        else
        {
            int x = PartyX;
            int y = PartyY + (index * PlayerSpriteHeight);

            if ( cmd.actorParty == Party_Players && index == cmd.actorIndex )
                x -= 16;

            Bounds bounds = { x, y, PlayerSpriteWidth, PlayerSpriteHeight };

            magicEffects[i] = new SpellEffect( bounds, spellIndex );
        }
    }

    curUpdate = UpdatePlayerMagicEffect;
}

void UpdatePlayerCastMagic()
{
    Command& cmd = curCmd;
    int spellIndex = cmd.actionId;

    if ( gTimer == 0 )
    {
        gShowWeapon = false;
        gShowBackgroundColor = false;

        GotoPlayerMagicEffect();
    }
    else
    {
        gTimer--;

        weaponSprite->Update();

        if ( (gTimer / 5) % 2 == 1 )
        {
            gShowBackgroundColor = true;
            ColorInt24& color = nesColors[Player::magicAttrs[spellIndex].NesColor];
            gFullScreenColor = al_map_rgb( color.Red, color.Green, color.Blue );
        }
        else
        {
            gShowBackgroundColor = false;
        }
    }
}

void GotoPlayerCastMagic()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];
    int spellIndex = cmd.actionId;

    sprite->SetFrames( castFrames, _class * PlayerSpriteRowHeight );

    weaponSprite->SetFrames( spellFrames, spellIndex * 16 );

    weaponSprite->SetX( PartyX - 16 - 16 );
    weaponSprite->SetY( PartyY + cmd.actorIndex * PlayerSpriteHeight );

    gTimer = 32;
    gShowWeapon = true;

    int level = spellIndex / 8;
    int c = spellIndex % 8;
    const char* name = Player::GetMagicName( level, c + 1 );

    strcpy_s( gMessage, name );

    Sound::PlayEffect( SEffect_Magic );

    curUpdate = UpdatePlayerCastMagic;
}

void UpdatePlayerItem()
{
    Command& cmd = curCmd;
    Sprite* sprite = playerSprites[cmd.targetIndex];
    int itemId = cmd.actionId;

    if ( gTimer == 0 )
    {
        sprite->Flash( false );

        gMessage[0] = '\0';

        if ( resultCount > 0 && actionResults[0].DealtDamage )
        {
            GotoNumbers();
        }
        else
        {
            GotoDisengage();
        }
    }
    else
    {
        if ( gTimer == 30 )
        {
            CalcItemEffect();

            Player::SpendItem( itemId );

            if ( !actionResults[0].Missed )
                sprite->Flash( true );
        }

        gTimer--;
    }
}

void GotoPlayerItem()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];
    int itemId = cmd.actionId;

    sprite->SetFrames( castFrames, _class * PlayerSpriteRowHeight );

    const char* name = Player::GetItemName( itemId );

    strcpy_s( gMessage, name );

    gTimer = 60;

    curUpdate = UpdatePlayerItem;
}

void ResetRunningCommands()
{
    curActorIndex = -1;
}

bool AreCommandsFinished()
{
    return curActorIndex == MaxActors;
}

void ShuffleActors()
{
    for ( int i = 0; i < MaxActors; i++ )
    {
        if ( i < Player::PartySize )
            shuffledActors[i] = i | PlayerFlag;
        else
            shuffledActors[i] = i - Player::PartySize;
    }

    ShuffleArray( shuffledActors, MaxActors );
}

Command& GetCommandBuilder()
{
    return commands[curActorIndex];
}

void AddCommand( const Command& cmd )
{
}

}
