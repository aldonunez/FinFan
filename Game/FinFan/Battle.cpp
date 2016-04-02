/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Battle.h"
#include "BattleMod.h"
#include "BattleMenus.h"
#include "Player.h"
#include "Text.h"
#include "Sprite.h"
#include "Magic.h"
#include "SceneStack.h"
#include "Sound.h"


namespace Battle
{
    void Init( int formationId, int backdropId );
    void Uninit();
    void Update();
    void Draw();
    void MakeEnemies();
    void MakeMixedEnemies( const ::Point* places, int maxPlaces, int placeWidth );

    void UpdateOpenMenu();
    void UpdateRunMenu();
    void UpdateCloseMenu();
    void GotoFirstMenu();
    void UpdateEngage();
    void UpdateDisengage();
}


BattleMod::~BattleMod()
{
    Battle::Uninit();
}

void BattleMod::Init( int formationId, int backdropId )
{
    Battle::Init( formationId, backdropId );
}

void BattleMod::Update()
{
    Battle::Update();
}

void BattleMod::Draw()
{
    Battle::Draw();
}

IPlayfield* BattleMod::AsPlayfield()
{
    return nullptr;
}


namespace Battle
{

const int PartyBoxLeft = 120;
const int PartyBoxTop = 152;
const int PartyInfoTop = 168;
const int PartyInfoLineHeight = 16;
const int PartyNameLeft = 136;
const int PartyMaxHPRight = 248;
const int PartyHPRight = PartyMaxHPRight - 4 * 8;
const int PartyHPLeft = PartyNameLeft + 7 * 8;
const int EnemyLeft = 16;
const int EnemyBoxTop = 152;
const int EnemyInfoTop = 168;
const int EnemyNameLeft = 8;
const int EnemyCountRight = 112;
const int EnemyZoneWidth = 112;
const int EnemyZoneHeight = 96;

const int PlayerFlag = 0x80;
const int InvalidAttackListId = 0xff;
const int MaxActors = MaxEnemies + Player::PartySize;


enum FormationType
{
    FormType_Small,
    FormType_Big,
    FormType_Mixed,
    FormType_Boss,
    FormType_BigBoss,
};

enum EncounterType
{
    Encounter_Normal,
    Encounter_PlayerFirst,
    Encounter_EnemyFirst,
};

struct ColorInt24
{
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
};


ALLEGRO_BITMAP* backdrops;
Formation       formations[128];
Bounds          enemySourcePos[128];
ALLEGRO_BITMAP* enemyImages;
uint16_t        nameOffsets[128];
char*           names;
ALLEGRO_BITMAP* playerImages;
ALLEGRO_BITMAP* battleSprites;
EnemyAttr       enemyAttrs[128];
AttackList      attackLists[44];
ColorInt24      nesColors[64];

TypeCounter typeCounts[4];
Enemy       enemies[9];
Enemy       enemiesHead;
int         enemyCount;

Menu*       activeMenu;

Command     commands[Player::PartySize];
int         shuffledActors[MaxActors];
int         curActorIndex;
Command     curCmd;

Sprite* playerSprites[Player::PartySize];

int gFormationId;
int gBackdropId;
int gSetId;

EncounterType gEncounter;
char gMessage[256];

typedef void (*UpdateFunc)();

UpdateFunc curUpdate;
int gNextPlayerId;

int gTimer;
bool gShowWeapon;
Sprite* weaponSprite;
ActionResult actionResults[9+Players];
ActionResult& strikeResult = actionResults[0];
int resultCount;


class Effect
{
public:
    virtual void Update() = 0;
    virtual void Draw() = 0;
};


class SpecialEffect : public Effect
{
    int timer;
    Bounds bounds;
    bool visible[4];
    Point pos[4];
    int frameCount;
    int frame;
    int index;

public:
    SpecialEffect( const Bounds& bounds, int index, int frameCount )
        :   timer( 0 ),
            bounds( bounds ),
            frame( 0 ),
            index( index ),
            frameCount( frameCount )
    {
        memset( visible, 0, sizeof visible );
    }

    virtual void Update()
    {
        int t = timer % 16;

        if ( timer % 4 == 0 )
        {
            int i = (timer % 16) / 4;

            visible[i] = true;
            pos[i].X = bounds.X + GetNextRandom( bounds.Width - 16 + 8 );
            pos[i].Y = bounds.Y + GetNextRandom( bounds.Height - 16 + 8 );
            frame = (frame + 1) % frameCount;
        }

        timer++;
    }

    virtual void Draw()
    {
        for ( int i = 0; i < _countof( visible ); i++ )
        {
            if ( visible[i] )
            {
                al_draw_bitmap_region(
                    battleSprites,
                    48 + frame * 16,
                    index * 16,
                    16,
                    16,
                    pos[i].X,
                    pos[i].Y,
                    0 );
            }
        }
    }
};

class StrikeEffect : public SpecialEffect
{
public:
    StrikeEffect( const Bounds& bounds, int index )
        :   SpecialEffect( bounds, index, 1 )
    {
    }
};

class SpellEffect : public SpecialEffect
{
public:
    SpellEffect( const Bounds& bounds, int index )
        :   SpecialEffect( bounds, index, 3 )
    {
    }
};

const int NumbersPathY[] = 
{
    0,
    9,
    13,
    15,
    16,
    15,
    13,
    9,
    0,
    4,
    6,
    7,
    6,
    4,
    0
};

const ALLEGRO_COLOR White = { 1, 1, 1, 1 };
const ALLEGRO_COLOR Black = { 0, 0, 0, 0 };
const ALLEGRO_COLOR Green = { 0.5, 1, 0.5, 1 };

class NumbersEffect : public Effect
{
    int timer;
    Bounds bounds;
    char text[11];
    Point pos;
    ALLEGRO_COLOR color;

public:
    NumbersEffect( const Bounds& bounds, const char* text, ALLEGRO_COLOR color = White )
        :   timer( 0 ),
            bounds( bounds ),
            color( color )
    {
        strcpy_s( this->text, text );
        pos.X = bounds.X + bounds.Width;
    }

    virtual void Update()
    {
        if ( timer % 2 == 0 )
        {
            int offset = 0;
            int i = timer / 2;

            if ( i < _countof( NumbersPathY ) )
                offset = NumbersPathY[i];

            pos.Y = bounds.Y + bounds.Height - 8 - offset;
        }

        timer++;
    }

    virtual void Draw()
    {
        Text::DrawStringRight( text, Text::FontB, color, pos.X, pos.Y );
    }
};

Effect* curEffect;
Effect* magicEffects[9];
bool gShowFullScreenColor;
bool gShowBackgroundColor;
ALLEGRO_COLOR gFullScreenColor;


Bounds standFrames[2] = 
{
    { 0, 0, 2 * 8, 3 * 8 },
    { 0, 0, 2 * 8, 3 * 8 },
};

Bounds walkFrames[2] = 
{
    { 0, 0, 2 * 8, 3 * 8 },
    { 2 * 8, 0, 2 * 8, 3 * 8 },
};

Bounds strikeFrames[2] = 
{
    { 2 * 8, 0, 2 * 8, 3 * 8 },
    { 4 * 8, 0, 2 * 8, 3 * 8 },
};

Bounds castFrames[2] = 
{
    { 6 * 8, 0, 2 * 8, 3 * 8 },
    { 6 * 8, 0, 2 * 8, 3 * 8 },
};

Bounds fanfareFrames[2] = 
{
    { 6 * 8, 0, 2 * 8, 3 * 8 },
    { 0 * 8, 0, 2 * 8, 3 * 8 },
};

Bounds weakFrames[2] = 
{
    { 8 * 8, 0, 2 * 8, 3 * 8 },
    { 8 * 8, 0, 2 * 8, 3 * 8 },
};

Bounds stoneFrames[2] = 
{
    { 10 * 8, 0, 2 * 8, 3 * 8 },
    { 10 * 8, 0, 2 * 8, 3 * 8 },
};

Bounds deadFrames[2] = 
{
    { 12 * 8, 0, 3 * 8, 3 * 8 },
    { 12 * 8, 0, 3 * 8, 3 * 8 },
};

Bounds16 weaponFrames[2] = 
{
    { 0, 0, 16, 16 },
    { 0, 0, 16, 16 },
};

Bounds16 punchFrames[2] = 
{
    { 0, 0, 16, 16 },
    { 0, 0, 0, 0 },
};

Bounds16 spellFrames[2] = 
{
    { 16, 0, 16, 16 },
    { 32, 0, 16, 16 },
};


void UpdateIdleSprite( int playerId );
EncounterType GetNextEncounterType();
void GotoFirstCommand();
void GotoOpeningMessage();
int FindLeader();

void Init( int formationId, int backdropId )
{
    backdrops = nullptr;

    gFormationId = formationId & 0x7f;
    gBackdropId = backdropId;
    gSetId = (formationId & 0x80) >> 7;

    gMessage[0] = '\0';
    gShowFullScreenColor = false;
    gShowBackgroundColor = false;
    gShowWeapon = false;

    backdrops = al_load_bitmap( "backdrops.png" );
    if ( backdrops == nullptr )
        return;

    FILE*   file = nullptr;
    errno_t err = 0;

    err = fopen_s( &file, "formations.dat", "rb" );
    if ( err != 0 )
        return;

    fread( formations, sizeof formations[0], _countof( formations ), file );

    fclose( file );


    err = fopen_s( &file, "enemyPos.dat", "rb" );
    if ( err != 0 )
        return;

    fread( enemySourcePos, sizeof enemySourcePos[0], _countof( enemySourcePos ), file );

    fclose( file );


    err = fopen_s( &file, "enemyNames.tab", "rb" );
    if ( err != 0 )
        return;

    fseek( file, 0, SEEK_END );
    int fileSize = ftell( file );
    int heapSize = fileSize - 128 * 2;
    fseek( file, 0, SEEK_SET );

    fread( nameOffsets, sizeof nameOffsets[0], _countof( nameOffsets ), file );
    names = new char[fileSize - 128 * 2];
    fread( names, 1, heapSize, file );
    fclose( file );


    char filename[256] = "";
    int pattern = formations[gFormationId].Pattern;
    sprintf_s( filename, "enemies%X.png", pattern );

    enemyImages = al_load_bitmap( filename );

    battleSprites = al_load_bitmap( "battleSprites.png" );
    playerImages = al_load_bitmap( "playerSprites.png" );


    err = fopen_s( &file, "enemyAttr.dat", "rb" );
    if ( err != 0 )
        return;

    fread( enemyAttrs, sizeof enemyAttrs[0], _countof( enemyAttrs ), file );
    fclose( file );


    err = fopen_s( &file, "attackLists.dat", "rb" );
    if ( err != 0 )
        return;

    fread( attackLists, sizeof attackLists[0], _countof( attackLists ), file );
    fclose( file );


    err = fopen_s( &file, "nesColors.dat", "rb" );
    if ( err != 0 )
        return;

    fread( nesColors, sizeof nesColors[0], _countof( nesColors ), file );
    fclose( file );




    MakeEnemies();

    Input::ResetRepeat();

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        int rowY = Player::Party[i]._class * PlayerSpriteRowHeight;
        playerSprites[i] = new Sprite( playerImages );
        playerSprites[i]->SetX( PartyX );
        playerSprites[i]->SetY( PartyY + i * PlayerSpriteRowHeight );
        UpdateIdleSprite( i );
    }

    weaponSprite = new Sprite( battleSprites );

    gEncounter = GetNextEncounterType();

    if ( gEncounter == Encounter_Normal )
        GotoFirstMenu();
    else
        GotoOpeningMessage();

    Sound::PlayTrack( Sound_Battle, 0, true );
}

void Uninit()
{
    al_destroy_bitmap( backdrops );
    backdrops = nullptr;

    al_destroy_bitmap( enemyImages );
    enemyImages = nullptr;

    al_destroy_bitmap( battleSprites );
    battleSprites = nullptr;

    al_destroy_bitmap( playerImages );
    playerImages = nullptr;

    delete [] names;
    names = nullptr;

    delete activeMenu;
    activeMenu = nullptr;

    for ( int i = 0; i < _countof( playerSprites ); i++ )
    {
        delete playerSprites[i];
        playerSprites[i] = nullptr;
    }

    delete weaponSprite;
    weaponSprite = nullptr;

    delete curEffect;
    curEffect = nullptr;

    for ( int i = 0; i < _countof( magicEffects ); i++ )
    {
        delete magicEffects[i];
        magicEffects[i] = nullptr;
    }
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

EncounterType GetNextEncounterType()
{
    if ( (formations[gFormationId].Flags & Formation_CantRun) != 0 )
        return Encounter_Normal;

    int leaderId = FindLeader();
    Player::Character& leader = Player::Party[leaderId];
    int agility = leader.basicStats[Player::Stat_Agility];
    int luck = leader.basicStats[Player::Stat_Luck];
    int initiative = 0;

    initiative = (agility + luck) / 8;

    int range = 100 - initiative + 1;
    int r = GetNextRandom( range );

    int v = initiative + r - formations[gFormationId].SurpriseRate;

    if ( v <= 10 )
        return Encounter_EnemyFirst;
    else if ( v >= 90 )
        return Encounter_PlayerFirst;
    else
        return Encounter_Normal;
}

bool HasLost()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( Player::IsPlayerAlive( i ) )
            return false;
    }

    return true;
}

bool HasWon()
{
    for ( int i = 0; i < MaxEnemies; i++ )
    {
        if ( enemies[i].Type != InvalidEnemyType && enemies[i].Hp > 0 )
            return false;
    }

    return true;
}

int FindLeader()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( Player::IsPlayerAlive( i ) )
            return i;
    }

    return NoneIndex;
}

int FindNextActivePlayer( int prevPlayer )
{
    int i = prevPlayer + 1;

    for ( ; i < Player::PartySize; i++ )
    {
        if ( Player::IsPlayerActive( i ) )
            return i;
    }

    return NoneIndex;
}

int FindNextActivePlayer()
{
    return FindNextActivePlayer( curActorIndex );
}

int FindPrevActivePlayer( int nextPlayer )
{
    int i = nextPlayer - 1;

    for ( ; i >= 0; i-- )
    {
        if ( Player::IsPlayerActive( i ) )
            return i;
    }

    return NoneIndex;
}

int FindPrevActivePlayer()
{
    return FindPrevActivePlayer( curActorIndex );
}

void GetLivingPlayers( int* playerIds, int& count )
{
    count = 0;

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( Player::IsPlayerAlive( i ) )
        {
            playerIds[count] = i;
            count++;
        }
    }
}

int GetNextAttackId( const uint8_t* list, int& nextIndex )
{
    int actionId = list[nextIndex];
    nextIndex++;
    if ( list[nextIndex] == 0xff )
        nextIndex = 0;
    return actionId;
}

Target SwitchParty( Target target )
{
    switch ( target )
    {
    case Target_AllEnemies: return Target_AllPlayers;
    case Target_OneEnemy:   return Target_OnePlayer;
    case Target_AllPlayers: return Target_AllEnemies;
    case Target_OnePlayer:  return Target_OneEnemy;
    default:                return target;
    }
}

void MakeEnemyAction( int id, int totalLevel, int* livingPlayerIds, int livingPlayerCount )
{
    Enemy& enemy = enemies[id];
    int type = enemies[id].Type;
    EnemyAttr& attrs = enemyAttrs[type];
    Command& cmd = curCmd;

    cmd.actorParty = Party_Enemies;
    cmd.actorIndex = id;

    if ( (enemy.Status & (Status_Paralysis | Status_Sleep)) != 0 )
    {
        cmd.action = Action_None;
        cmd.target = Target_None;
    }
    else if ( totalLevel > attrs.Morale && GetNextRandom( 100 ) < 25 )
    {
        cmd.action = Action_Run;
        cmd.target = Target_None;
    }
    else if ( attrs.AttackListId == InvalidAttackListId )
    {
        cmd.action = Action_Fight;
        cmd.target = Target_OnePlayer;
    }
    else
    {
        const AttackList& list = attackLists[attrs.AttackListId];
        int r = GetNextRandom( 128 );

        if ( r < list.MagicRate )
        {
            cmd.action = Action_Magic;
            cmd.actionId = GetNextAttackId( list.MagicIds, enemy.NextSpellIndex );

            Player::MagicAttr& magicAttr = Player::magicAttrs[cmd.actionId];
            cmd.target = SwitchParty( (Target) magicAttr.Target );
        }
        else if ( r < (list.MagicRate + list.SpecialRate) )
        {
            cmd.action = Action_Special;
            cmd.actionId = GetNextAttackId( list.SpecialIds, enemy.NextSpecialIndex );

            Player::MagicAttr& specialAttr = Player::specialAttrs[cmd.actionId];
            cmd.target = SwitchParty( (Target) specialAttr.Target );
        }
        else
        {
            cmd.action = Action_Fight;
            cmd.target = Target_OnePlayer;
        }
    }

    if ( cmd.target == Target_OnePlayer )
    {
        if ( livingPlayerCount > 0 )
        {
            int r = GetNextRandom( livingPlayerCount );
            cmd.targetIndex = livingPlayerIds[r];
        }
        else
        {
            cmd.action = Action_None;
        }
    }
}

// PlayerWeights depends on a certain max player count
const int PlayerWeights = 8;
static_assert( Player::PartySize == 4, "There are more players than expected." );

void GetWeightedPlayers( const int* playerIds, int playerCount, int* weightedPlayerIds, int& count )
{
    assert( playerCount <= Players );

    count = 0;

    for ( int i = 0; i < playerCount; i++ )
    {
        int chances = 1;

        if ( playerIds[i] == 0 )
            chances = 4;
        else if ( playerIds[i] == 1 )
            chances = 2;

        for ( ; chances > 0; chances-- )
        {
            weightedPlayerIds[count] = playerIds[i];
            count++;
        }
    }
}

void MakeEnemyAction()
{
    int livingPlayerIds[Player::PartySize];
    int livingPlayerCount = 0;
    int weightedPlayerIds[PlayerWeights];
    int weightedPlayerCount = 0;
    int totalLevel = 0;
    int enemyId = shuffledActors[curActorIndex];

    GetLivingPlayers( livingPlayerIds, livingPlayerCount );

    for ( int i = 0; i < livingPlayerCount; i++ )
    {
        int playerId = livingPlayerIds[i];

        totalLevel += Player::Party[playerId].level;
    }

    GetWeightedPlayers( livingPlayerIds, livingPlayerCount, weightedPlayerIds, weightedPlayerCount );

    MakeEnemyAction( enemyId, totalLevel, weightedPlayerIds, weightedPlayerCount );
}

void MakeDisabledPlayerActions()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( !Player::IsPlayerActive( i ) )
        {
            Command& cmd = commands[i];

            cmd.actorParty = Party_Players;
            cmd.actorIndex = i;
            cmd.action = Action_None;
        }
    }
}

void ShuffleArray( int* array, int length )
{
    for ( int i = length - 1; i >= 1; i-- )
    {
        int r = GetNextRandom( i + 1 );
        int orig = array[i];
        array[i] = array[r];
        array[r] = orig;
    }
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

void ResetRunningCommands()
{
    curActorIndex = -1;
}

bool AreCommandsFinished()
{
    return curActorIndex == MaxActors;
}

void PrepActions()
{
    MakeDisabledPlayerActions();
    ShuffleActors();
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

void GotoRunMenu()
{
    int playerId = curActorIndex;
    int _class = Player::Party[playerId]._class;
    playerSprites[playerId]->SetFrames( standFrames, _class * PlayerSpriteRowHeight );

    activeMenu = new BattleMenu();
    activeMenu->prevMenu = nullptr;

    curUpdate = UpdateRunMenu;
}

void GotoCloseMenu( int nextPlayerId )
{
    int playerId = curActorIndex;
    int _class = Player::Party[playerId]._class;
    playerSprites[playerId]->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );

    gNextPlayerId = nextPlayerId;

    curUpdate = UpdateCloseMenu;
}

void GotoRunCommand();
void GotoNextCommand();

bool gAtEndOfRound;

void GotoFirstCommand()
{
    gAtEndOfRound = false;
    PrepActions();
    ResetRunningCommands();
    GotoNextCommand();
}

const int LastWonState = 3;

int gXPWon;
int gGWon;
int gWonState;
int gLevelUpPlayerId;

int CalcNextLevelUp( int prevPlayerId )
{
    int i;

    for ( i = prevPlayerId + 1; i < Player::PartySize; i++ )
    {
        if ( Player::CalcLevelUp( i ) )
            break;
    }

    return i;
}

void LeaveBattle()
{
    for ( int i = 0; i < Players; i++ )
    {
        Player::Party[i].status &= (Status_Death | Status_Stone | Status_Poison);
        Player::Party[i].hitMultiplier = 1;

        Player::CalcDerivedStats( i );
    }

    SceneStack::LeaveBattle();
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

void CalcEnemyAutoHP()
{
    for ( int i = 0; i < MaxEnemies; i++ )
    {
        Enemy& enemy = enemies[i];

        if ( enemy.Type == InvalidEnemyType || enemy.Hp == 0 )
            continue;

        bool isPoison = (enemy.Status & Status_Poison) != 0;
        bool isRegen = (enemyAttrs[enemy.Type].Classes & EnemyClass_Regen) != 0;

        if ( isRegen != isPoison )
        {
            int hpBoost = enemy.GetMaxHp() / 20;
            if ( hpBoost == 0 )
                hpBoost = 1;

            if ( isPoison )
                hpBoost = -hpBoost;

            enemy.AddHp( hpBoost );

            actionResults[resultCount].Missed = false;
            actionResults[resultCount].DealtDamage = true;
            actionResults[resultCount].Damage = -hpBoost;
            actionResults[resultCount].Died = (enemy.Hp == 0);
            actionResults[resultCount].TargetParty = Party_Enemies;
            actionResults[resultCount].TargetIndex = i;
            resultCount++;
        }
    }
}

void CalcPlayerAutoHP()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Player::Character& player = Player::Party[i];

        if ( !Player::IsPlayerAlive( i ) || (player.status & Status_Poison) == 0 )
            continue;

        int hpBoost = player.GetMaxHp() / 20;
        if ( hpBoost == 0 )
            hpBoost = 1;

        hpBoost = -hpBoost;

        player.AddHp( hpBoost );

        actionResults[resultCount].Missed = false;
        actionResults[resultCount].DealtDamage = true;
        actionResults[resultCount].Damage = -hpBoost;
        actionResults[resultCount].Died = (player.hp == 0);
        actionResults[resultCount].TargetParty = Party_Players;
        actionResults[resultCount].TargetIndex = i;
        resultCount++;
    }
}

void UpdateAllIdleSprites();
void GotoNumbers();

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

void UpdateIdleSprite( int playerId );

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

bool CanRunAway()
{
    // formation doesn't allow running
    if ( (formations[gFormationId].Flags & Formation_CantRun) != 0 )
        return false;

    // can always leave if striking first
    if ( gEncounter == Encounter_PlayerFirst )
        return true;

    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    Player::Character& player = Player::Party[playerId];
    int r = GetNextRandom( player.level + 15 + 1 );

    return r < player.basicStats[Player::Stat_Luck];
}

void GotoPlayerMagicEffect();

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

const int EnemyFleeFrames = 30;

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

void GotoEnemyFight();

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

bool TryRecoverConfuse()
{
    int enemyId = shuffledActors[curActorIndex];
    Enemy& enemy = enemies[enemyId];

    int r = GetNextRandom( 100 );
    if ( r < 25 )
    {
        enemy.Status &= ~Status_Confusion;
        return true;
    }

    return false;
}

void MakeConfuseAction()
{
    int enemyId = shuffledActors[curActorIndex];

    curCmd.action = Action_Magic;
    curCmd.actorParty = Party_Enemies;
    curCmd.actorIndex = enemyId;
    curCmd.actionId = 4;    // FIRE
    curCmd.target = Target_OneEnemy;

    // how many active enemies to skip
    int skip = GetNextRandom( enemyCount );

    for ( int i = 0; i < MaxEnemies; i++ )
    {
        if ( enemies[i].Type != InvalidEnemyType && enemies[i].Hp > 0 )
        {
            if ( skip == 0 )
            {
                curCmd.targetIndex = i;
                break;
            }

            skip--;
        }
    }
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
            if ( TryRecoverConfuse() )
                GotoNextCommand();
            else
            {
                MakeConfuseAction();
                GotoEnemyEngage();
            }
        }
        else
        {
            MakeEnemyAction();
            GotoEnemyEngage();
        }
    }
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

void CommitCommand( int index )
{
    Command& cmd = commands[index];
    if ( cmd.action == Action_Item )
        Player::SpendItem( cmd.actionId );
}

void UndoCommand( int index )
{
    Command& cmd = commands[index];
    if ( cmd.action == Action_Item )
        Player::ReturnItem( cmd.actionId );
}

void UndoAllCommands()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Command& cmd = commands[i];
        if ( cmd.action == Action_Item )
            Player::ReturnItem( cmd.actionId );
    }
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

void GotoDisengage()
{
    Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    int _class = Player::Party[playerId]._class;
    Sprite* sprite = playerSprites[playerId];

    sprite->SetFrames( walkFrames, _class * PlayerSpriteRowHeight );
    curUpdate = UpdateDisengage;
}

const int EnemyDieFrames = 30;

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

void UpdateIdleSprite( int index )
{
    int _class = Player::Party[index]._class;
    Player::Character& player = Player::Party[index];
    Sprite* sprite = playerSprites[index];
    const Bounds* frames = nullptr;

    if ( (player.status & Status_Death) != 0 )
    {
        frames = deadFrames;
    }
    else if ( (player.status & Status_Stone) != 0 )
    {
        frames = stoneFrames;
    }
    else if ( player.hp < player.maxHp / 4 )
    {
        frames = weakFrames;
    }
    else if ( (player.status & (Status_Paralysis | Status_Sleep)) != 0 )
    {
        frames = weakFrames;
    }
    else
    {
        frames = standFrames;
    }

    sprite->SetFrames( frames, _class * PlayerSpriteHeight );
}

void UpdateIdleSprites()
{
    for ( int i = 0; i < resultCount; i++ )
    {
        // If we allow a player to die during his turn, then it'll have to wait until after
        // the disengage. For now it's not allowed, so they all die at the same time.

        int index = actionResults[i].TargetIndex;
        if ( actionResults[i].TargetParty == Party_Players )
        {
            UpdateIdleSprite( index );
        }
    }
}

void UpdateAllIdleSprites()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        UpdateIdleSprite( i );
    }
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

int GetHitChance( Actor* actor, Actor* target )
{
    int chance = 168 + actor->GetHitRate();

    if ( (target->GetStatus() & (Status_Paralysis | Status_Sleep)) == 0 )
        chance -= target->GetEvadeRate();

    if ( (actor->GetStatus() & Status_Dark) != 0 )
        chance -= 40;

    if ( (target->GetStatus() & Status_Dark) != 0 )
        chance += 40;

    if ( (actor->GetTargetClasses() & target->GetEnemyClasses()) != 0 )
        chance += 40;

    if ( (actor->GetTargetElement() & target->GetElementWeakness()) != 0 )
        chance += 40;

    if ( chance < 0 )
        chance = 0;

    return chance;
}

int GetStatusChance( Actor* actor, Actor* target )
{
    int chance = 100;

    if ( (actor->GetTargetElement() & target->GetElementStrength()) != 0 )
        chance = 0;

    chance -= target->GetMagicAbsorb();

    if ( chance < 0 )
        chance = 0;

    return chance;
}

void CalcPhysDamage( Actor* actor, Actor* target )
{
    bool missed = false;
    int totalDamage = 0;
    bool critHit = false;

    int attack = actor->GetDamage();
    int critHitRate = actor->GetCritHitRate();
    int maxHits = actor->GetMaxHits();

    if ( (actor->GetTargetClasses() & target->GetEnemyClasses()) != 0 )
        attack += 4;

    if ( (actor->GetTargetElement() & target->GetElementWeakness()) != 0 )
        attack += 4;

    if ( (target->GetStatus() & (Status_Paralysis | Status_Sleep)) != 0 )
        attack = (attack * 5) / 4;

    if ( attack < 1 )
        attack = 1;

    maxHits *= actor->GetHitMultiplier();
    if ( maxHits == 0 )
        maxHits = 1;

    maxHits = GetNextRandom( maxHits ) + 1;
    int hitCount = 0;

    int hitChance = GetHitChance( actor, target );
    int statusChance = GetStatusChance( actor, target );

    for ( int i = 0; i < maxHits; i++ )
    {
        int hitR = GetNextRandom( 201 );

        if ( hitR > hitChance )
            continue;

        // for (0..attack), range = attack + 1
        int r = GetNextRandom( attack + 1 );

        int damage = attack + r - target->GetAbsorb();

        if ( damage < 1 )
            damage = 1;

        if ( hitR <= critHitRate )
        {
            damage *= 2;
            critHit = true;
        }

        if ( target->GetTargetStatus() != 0 )
        {
            r = GetNextRandom( 201 );

            if ( r <= statusChance )
                target->AddStatus( actor->GetTargetStatus() );
        }

        hitCount++;
        totalDamage += damage;
    }

    target->AddHp( -totalDamage );

    strikeResult.Missed = hitCount == 0;
    strikeResult.Damage = totalDamage;
    strikeResult.CritHit = critHit;
}

void CalcPlayerPhysDamage()
{
    Command& cmd = curCmd;
    Player::Character& actor = Player::Party[ cmd.actorIndex ];
    int enemyId = cmd.targetIndex;
    int i = 0;
    Enemy* enemy = nullptr;

    strikeResult.TargetParty = Party_Enemies;

    if ( enemies[enemyId].Type == InvalidEnemyType || enemies[enemyId].Hp == 0 )
    {
        // look for another enemy, because this one's dead

        for ( i = 0; i < MaxEnemies; i++ )
        {
            if ( i != enemyId && enemies[i].Type != InvalidEnemyType && enemies[i].Hp > 0 )
            {
                enemyId = i;
                break;
            }
        }

        if ( i == MaxEnemies )
        {
            // no enemies available, no results
            resultCount = 0;
            return;
        }
    }

    enemy = &enemies[enemyId];

    CalcPhysDamage( &actor, enemy );

    resultCount = 1;
    strikeResult.TargetIndex = enemyId;
    strikeResult.Died = enemy->Hp == 0;
    strikeResult.DealtDamage = true;
}

void CalcEnemyPhysDamage()
{
    Command& cmd = curCmd;
    Enemy& actor = enemies[ cmd.actorIndex ];
    int playerId = cmd.targetIndex;
    int i = 0;
    Player::Character* player = nullptr;

    strikeResult.TargetParty = Party_Players;

    player = &Player::Party[playerId];

    CalcPhysDamage( &actor, player );

    resultCount = 1;
    strikeResult.TargetIndex = playerId;
    strikeResult.Died = player->hp == 0;
    strikeResult.DealtDamage = true;
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

void ApplyMagicToEnemy( const Player::MagicAttr& magicAttr, CalcMagicFunc calcFunc, int index )
{
    calcFunc( magicAttr, &enemies[index], actionResults[resultCount] );

    actionResults[resultCount].TargetParty = Party_Enemies;
    actionResults[resultCount].TargetIndex = index;
    actionResults[resultCount].Died = 
        (enemies[index].Status & Status_Death) != 0 || enemies[index].Hp == 0;

    resultCount++;
}

void ApplyMagicToPlayer( const Player::MagicAttr& magicAttr, CalcMagicFunc calcFunc, int index )
{
    calcFunc( magicAttr, &Player::Party[index], actionResults[resultCount] );

    actionResults[resultCount].TargetParty = Party_Players;
    actionResults[resultCount].TargetIndex = index;
    actionResults[resultCount].Died = 
        (Player::Party[index].status & Status_Death) != 0 || Player::Party[index].hp == 0;
    resultCount++;
}

Player::MagicAttr& GetMagicAttr( const Command& cmd )
{
    if ( cmd.action == Action_Magic || cmd.action == Action_Item )
        return Player::magicAttrs[cmd.actionId];
    else
        return Player::specialAttrs[cmd.actionId];
}

void CalcMagicEffect( const Command& cmd )
{
    Player::MagicAttr& magicAttr = GetMagicAttr( cmd );
    CalcMagicFunc calcFunc = nullptr;

    calcFunc = GetMagicProc( magicAttr.Effect );
    resultCount = 0;

    switch ( cmd.target )
    {
    case Target_AllEnemies:
        for ( int i = 0; i < _countof( enemies ); i++ )
        {
            if ( enemies[i].Type != InvalidEnemyType 
                && enemies[i].Hp > 0 )
            {
                ApplyMagicToEnemy( magicAttr, calcFunc, i );
            }
        }
        break;

    case Target_AllPlayers:
        for ( int i = 0; i < Player::PartySize; i++ )
        {
            ApplyMagicToPlayer( magicAttr, calcFunc, i );
        }
        break;

    case Target_OneEnemy:
        for ( int i = 0; i < _countof( enemies ); i++ )
        {
            int targetIndex = (cmd.targetIndex + i) % _countof( enemies );
            if ( enemies[targetIndex].Type != InvalidEnemyType 
                && enemies[targetIndex].Hp > 0 )
            {
                ApplyMagicToEnemy( magicAttr, calcFunc, targetIndex );
                break;
            }
        }
        break;

    case Target_OnePlayer:
        ApplyMagicToPlayer( magicAttr, calcFunc, cmd.targetIndex );
        break;

    case Target_Self:
        if ( cmd.actorParty == Party_Enemies )
            ApplyMagicToEnemy( magicAttr, calcFunc, cmd.actorIndex );
        else
            ApplyMagicToPlayer( magicAttr, calcFunc, cmd.actorIndex );
        break;
    }
}

void CalcMagicEffect()
{
    Command& cmd = curCmd;

    CalcMagicEffect( cmd );
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

// Use IsMute as needed. Magic action will need it, but not Item.
// In the original game, it wasn't like this. Mute blocked items, too.

bool IsMute( Party party, int index )
{
    if ( party == Party_Players )
    {
        return (Player::Party[index].status & Status_Silence) != 0;
    }
    else
    {
        return (enemies[index].Status & Status_Silence) != 0;
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

void CalcItemEffect()
{
    Command& cmd = curCmd;
    Command magicCmd = cmd;
    int spellId = Player::GetSpellForItem( cmd.actionId );

    resultCount = 0;

    if ( spellId != 0 )
    {
        magicCmd.actionId = spellId - 1;

        CalcMagicEffect( magicCmd );
    }
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

void Update()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        playerSprites[i]->Update();
    }

    curUpdate();
}

void DrawBackdrop()
{
    for ( int i = 0; i < 3; i++ )
    {
        al_draw_bitmap_region( 
            backdrops, 
            0, 
            BackdropSegHeight * gBackdropId, 
            BackdropSegWidth, 
            BackdropSegHeight, 
            BackdropSegWidth * i, 
            0, 
            0 );
    }
}

void DrawEnemies()
{
    Enemy*  cur = enemiesHead.Next;

    for ( ; cur != &enemiesHead; cur = cur->Next )
    {
        int enemyId = cur->Type;

        al_draw_tinted_bitmap_region(
            enemyImages,
            cur->Color,
            enemySourcePos[enemyId].X,
            enemySourcePos[enemyId].Y,
            enemySourcePos[enemyId].Width,
            enemySourcePos[enemyId].Height,
            cur->Bounds.X,
            cur->Bounds.Y,
            0 );
    }
}

void DrawEnemyTypes()
{
    char str[8] = "";
    int j = 0;

    Text::DrawBox( EnemyNameLeft - 8, EnemyBoxTop, 120, 88 );

    for ( int i = 0; i < _countof( typeCounts ); i++ )
    {
        if ( typeCounts[i].Type == InvalidEnemyType || typeCounts[i].Count == 0 )
            continue;

        char* name = names + nameOffsets[typeCounts[i].Type];
        Text::DrawString( name, EnemyNameLeft, EnemyInfoTop + j * 8 );

        if ( typeCounts[i].Count > 1 )
        {
            sprintf_s( str, "%d", typeCounts[i].Count );

            Text::DrawStringRight( str, EnemyCountRight, EnemyInfoTop + j * 8 );
        }

        j++;
    }
}

void DrawPartyInfo()
{
    char str[8] = "";

    Text::DrawBox( PartyBoxLeft, PartyBoxTop, 136, 88 );

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        int y = PartyInfoTop + i * PartyInfoLineHeight;

        Text::DrawString( Player::Party[i].name, PartyNameLeft, y );

        sprintf_s( str, "%3d/%3d", Player::Party[i].hp, Player::Party[i].maxHp );
        Text::DrawString( str, PartyHPLeft, y );
    }
}

void DrawParty()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        playerSprites[i]->Draw();
    }

    if ( gShowWeapon )
    {
        weaponSprite->Draw();
    }
}

void DrawMessage()
{
    if ( gMessage[0] != '\0' )
    {
        Text::DrawBox( 0, 0, 256, 3 * 8 );
        Text::DrawStringCenter( gMessage, 8, 8, 256 - 8*2 );
    }
}

void Draw()
{
    if ( gShowFullScreenColor )
    {
        al_clear_to_color( gFullScreenColor );
        return;
    }

    if ( gShowBackgroundColor )
        al_clear_to_color( gFullScreenColor );
    else
        al_clear_to_color( al_map_rgb( 0, 0, 0 ) );

    DrawBackdrop();

    DrawEnemies();
    DrawEnemyTypes();

    DrawParty();
    DrawPartyInfo();

    if ( activeMenu != nullptr )
    {
        activeMenu->Draw();
        activeMenu->DrawCursor();
    }

    if ( curEffect != nullptr )
    {
        curEffect->Draw();
    }

    for ( int i = 0; i < resultCount; i++ )
    {
        if ( magicEffects[i] != nullptr )
            magicEffects[i]->Draw();
    }

    DrawMessage();
}


const int BigEnemyWidth = 6 * 8;
const int BigEnemyHeight = 6 * 8;
const int SmallEnemyWidth = 4 * 8;
const int SmallEnemyHeight = 4 * 8;


Point   sBigEnemyPos[4] = 
{
    { EnemyLeft,                                    BackdropSegHeight },
    { EnemyLeft,                                    BackdropSegHeight + BigEnemyWidth },
    { EnemyLeft + BigEnemyWidth,                    BackdropSegHeight },
    { EnemyLeft + BigEnemyWidth,                    BackdropSegHeight + BigEnemyHeight }
};

Point   sSmallEnemyPos[9] = 
{
    { EnemyLeft,                                    BackdropSegHeight + SmallEnemyHeight },
    { EnemyLeft,                                    BackdropSegHeight },
    { EnemyLeft,                                    BackdropSegHeight + (SmallEnemyHeight * 2) },

    { EnemyLeft + SmallEnemyWidth,                  BackdropSegHeight + SmallEnemyHeight },
    { EnemyLeft + SmallEnemyWidth,                  BackdropSegHeight },
    { EnemyLeft + SmallEnemyWidth,                  BackdropSegHeight + (SmallEnemyHeight * 2) },

    { EnemyLeft + SmallEnemyWidth * 2,              BackdropSegHeight + SmallEnemyHeight },
    { EnemyLeft + SmallEnemyWidth * 2,              BackdropSegHeight },
    { EnemyLeft + SmallEnemyWidth * 2,              BackdropSegHeight + (SmallEnemyHeight * 2) },
};

Point   sMixedEnemyPosBig[2] = 
{
    { EnemyLeft,                                    BackdropSegHeight },
    { EnemyLeft,                                    BackdropSegHeight + BigEnemyWidth },
};

Point   sMixedEnemyPosSmall[6] = 
{
    { EnemyLeft + BigEnemyWidth,                    BackdropSegHeight + SmallEnemyHeight },
    { EnemyLeft + BigEnemyWidth,                    BackdropSegHeight },
    { EnemyLeft + BigEnemyWidth,                    BackdropSegHeight + (SmallEnemyHeight * 2) },

    { EnemyLeft + BigEnemyWidth + SmallEnemyWidth,  BackdropSegHeight + SmallEnemyHeight },
    { EnemyLeft + BigEnemyWidth + SmallEnemyWidth,  BackdropSegHeight },
    { EnemyLeft + BigEnemyWidth + SmallEnemyWidth,  BackdropSegHeight + (SmallEnemyHeight * 2) },
};


void MakeCenteredEnemy();

void MakeEnemies()
{
    Formation* formation = &formations[gFormationId];
    int bigEnemyCount = 0;

    for ( int i = 0; i < _countof( typeCounts ); i++ )
    {
        typeCounts[i].Type = InvalidEnemyType;
        typeCounts[i].Count = 0;
    }

    for ( int i = 0; i < _countof( enemies ); i++ )
    {
        enemies[i].Type = InvalidEnemyType;
    }

    enemiesHead.Next = &enemiesHead;
    enemiesHead.Prev = &enemiesHead;
    enemyCount = 0;

    switch ( formation->Type )
    {
    case FormType_Big:
        MakeMixedEnemies( sBigEnemyPos, _countof( sBigEnemyPos ), BigEnemyWidth );
        break;

    case FormType_Small:
        MakeMixedEnemies( sSmallEnemyPos, _countof( sSmallEnemyPos ), SmallEnemyWidth );
        break;

    case FormType_Mixed:
        MakeMixedEnemies( sMixedEnemyPosBig, _countof( sMixedEnemyPosBig ), BigEnemyWidth );
        // the mixed enemy map depends on small enemies starting at index 3
        bigEnemyCount = enemyCount;
        enemyCount = 3;
        MakeMixedEnemies( sMixedEnemyPosSmall, _countof( sMixedEnemyPosSmall ), SmallEnemyWidth );
        enemyCount = enemyCount - (3 - bigEnemyCount);
        break;

    case FormType_Boss:
    case FormType_BigBoss:
        MakeCenteredEnemy();
        break;
    }
}

TypeCounter* RegisterType( int type )
{
    for ( int i = 0; i < _countof( typeCounts ); i++ )
    {
        TypeCounter& counter = typeCounts[i];

        if ( counter.Type == InvalidEnemyType )
        {
            counter.Count = 1;
            counter.Type = type;
            return &counter;
        }
        else if ( counter.Type == type )
        {
            counter.Count++;
            return &counter;
        }
    }

    return nullptr;
}

void MakeOneEnemy( const ::Point& place, int type )
{
    Enemy& enemy = enemies[enemyCount];

    enemy.Type = type;
    enemy.Counter = RegisterType( enemy.Type );

    enemy.Prev = enemiesHead.Prev;
    enemy.Next = &enemiesHead;
    enemiesHead.Prev->Next = &enemy;
    enemiesHead.Prev = &enemy;

    enemy.Bounds.X = place.X;
    enemy.Bounds.Y = place.Y;
    enemy.Bounds.Width = enemySourcePos[enemy.Type].Width;
    enemy.Bounds.Height = enemySourcePos[enemy.Type].Height;

    enemy.Color = al_map_rgba( 255, 255, 255, 255 );

    enemy.Hp = enemyAttrs[enemy.Type].Hp;
    enemy.Status = 0;
    enemy.HitRate = enemyAttrs[enemy.Type].HitRate;
    enemy.Morale = enemyAttrs[enemy.Type].Morale;
    enemy.Absorb = enemyAttrs[enemy.Type].Defense;
    enemy.ElemStrength = enemyAttrs[enemy.Type].ElementStrength;
    enemy.HitMultiplier = 1;
    enemy.EvadeRate = enemyAttrs[enemy.Type].Evade;
    enemy.Damage = enemyAttrs[enemy.Type].Damage;

    enemy.NextSpellIndex = 0;
    enemy.NextSpecialIndex = 0;
}

void MakeCenteredEnemy()
{
    Formation* formation = &formations[gFormationId];
    int type = formation->Ids[0];
    Point place;

    place.X = EnemyLeft + (EnemyZoneWidth - enemySourcePos[type].Width) / 2;
    place.Y = BackdropSegHeight + (EnemyZoneHeight - enemySourcePos[type].Height) / 2;

    MakeOneEnemy( place, type );
}

void MakeMixedEnemies( const ::Point* places, int maxPlaces, int placeWidth )
{
    Formation* formation = &formations[gFormationId];
    uint8_t* set = nullptr;
    int setSize = 0;
    int count = 0;

    if ( gSetId == 0 )
    {
        set = &formation->MinMax[0];
        setSize = 4;
    }
    else
    {
        set = &formation->MinMax[4];
        setSize = 2;
    }

    for ( int i = 0; i < setSize; i++ )
    {
        if ( set[i] == 0 )
            continue;
        if ( enemySourcePos[formation->Ids[i]].Width != placeWidth )
            continue;

        int max = set[i] & 0xf;
        int min = set[i] >> 4;
        int range = max - min + 1;
        int n = min + GetNextRandom( range );

        for ( ; n > 0; n--, enemyCount++, count++ )
        {
            if ( count == maxPlaces || enemyCount == _countof( enemies ) )
                goto NoMoreEnemies;

            MakeOneEnemy( places[count], formation->Ids[i] );
        }
    }

NoMoreEnemies:
    ;
}

EnemyMap smallEnemyMap = 
{
    {
        { 1, 4, 7 },
        { 0, 3, 6 },
        { 2, 5, 8 }
    }
};

EnemyMap bigEnemyMap = 
{
    {
        { 0, 2, -1 },
        { 1, 3, -1 },
        { -1, -1, -1 }
    }
};

EnemyMap mixedEnemyMap = 
{
    {
        { 0, 4, 7 },
        { 1, 3, 6 },
        { -1, 5, 8 },
    }
};

EnemyMap bossEnemyMap = 
{
    {
        { 0, -1, -1 },
        { -1, -1, -1 },
        { -1, -1, -1 }
    }
};

Enemy* GetEnemies()
{
    return enemies;
}

const EnemyMap* GetEnemyMap()
{
    Formation* formation = &formations[gFormationId];

    switch ( formation->Type )
    {
    case FormType_Big:      return &bigEnemyMap;
    case FormType_Small:    return &smallEnemyMap;
    case FormType_Mixed:    return &mixedEnemyMap;
    case FormType_Boss:
    case FormType_BigBoss:  return &bossEnemyMap;
    default:                return &smallEnemyMap;
    }
}

Command& GetCommandBuilder()
{
    return commands[curActorIndex];
}

void AddCommand( const Command& cmd )
{
}

    int Enemy::GetHp()
    {
        return Hp;
    }

    void Enemy::SetHp( int value )
    {
        Hp = value;
    }

    int Enemy::GetStatus()
    {
        return Status;
    }

    void Enemy::SetStatus( int value )
    {
        Status = value;
    }

    int Enemy::GetHitMultiplier()
    {
        return HitMultiplier;
    }

    void Enemy::SetHitMultiplier( int value )
    {
        HitMultiplier = value;
    }

    int Enemy::GetMorale()
    {
        return Morale;
    }

    void Enemy::SetMorale( int value )
    {
        Morale = value;
    }

    int Enemy::GetAbsorb()
    {
        return Absorb;
    }

    void Enemy::SetAbsorb( int value )
    {
        Absorb = value;
    }

    int Enemy::GetEvadeRate()
    {
        return EvadeRate;
    }

    void Enemy::SetEvadeRate( int value )
    {
        EvadeRate = value;
    }

    int Enemy::GetElementStrength()
    {
        return ElemStrength;
    }

    void Enemy::SetElementStrength( int value )
    {
        ElemStrength = value;
    }

    int Enemy::GetDamage()
    {
        return Damage;
    }

    void Enemy::SetDamage( int value )
    {
        Damage = value;
    }

    int Enemy::GetHitRate()
    {
        return HitRate;
    }

    void Enemy::SetHitRate( int value )
    {
        HitRate = value;
    }


    int Enemy::GetElementWeakness()
    {
        return enemyAttrs[Type].ElementWeakness;
    }

    int Enemy::GetMagicAbsorb()
    {
        return enemyAttrs[Type].MagicDefense;
    }

    int Enemy::GetEnemyClasses()
    {
        return enemyAttrs[Type].Classes;
    }

    int Enemy::GetMaxHp()
    {
        return enemyAttrs[Type].Hp;
    }

    int Enemy::GetCritHitRate()
    {
        return enemyAttrs[Type].CritHitRate;
    }

    int Enemy::GetMaxHits()
    {
        return enemyAttrs[Type].Hits;
    }

    int Enemy::GetTargetClasses()
    {
        return 0;
    }

    int Enemy::GetTargetElement()
    {
        return enemyAttrs[Type].AttackElement;
    }

    int Enemy::GetTargetStatus()
    {
        return enemyAttrs[Type].Status;
    }
}
