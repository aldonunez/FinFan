/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "BattleCalc.h"
#include "Battle.h"
#include "Magic.h"
#include "Player.h"


namespace Battle
{

const int InvalidAttackListId = 0xff;


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

EncounterType GetNextEncounterType()
{
    const Formation& formation = GetFormation();

    if ( (formation.Flags & Formation_CantRun) != 0 )
        return Encounter_Normal;

    int leaderId = FindLeader();
    Player::Character& leader = Player::Party[leaderId];
    int agility = leader.basicStats[Player::Stat_Agility];
    int luck = leader.basicStats[Player::Stat_Luck];
    int initiative = 0;

    initiative = (agility + luck) / 8;

    int range = 100 - initiative + 1;
    int r = GetNextRandom( range );

    int v = initiative + r - formation.SurpriseRate;

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

void MakeEnemyAction( int id, int totalLevel, int* livingPlayerIds, int livingPlayerCount, 
    Command& curCmd )
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

void MakeEnemyAction( int enemyId, Command& curCmd )
{
    int livingPlayerIds[Player::PartySize];
    int livingPlayerCount = 0;
    int weightedPlayerIds[PlayerWeights];
    int weightedPlayerCount = 0;
    int totalLevel = 0;

    GetLivingPlayers( livingPlayerIds, livingPlayerCount );

    for ( int i = 0; i < livingPlayerCount; i++ )
    {
        int playerId = livingPlayerIds[i];

        totalLevel += Player::Party[playerId].level;
    }

    GetWeightedPlayers( livingPlayerIds, livingPlayerCount, weightedPlayerIds, weightedPlayerCount );

    MakeEnemyAction( enemyId, totalLevel, weightedPlayerIds, weightedPlayerCount, curCmd );
}

bool TryRecoverConfuse( int enemyId )
{
    Enemy& enemy = enemies[enemyId];

    int r = GetNextRandom( 100 );
    if ( r < 25 )
    {
        enemy.Status &= ~Status_Confusion;
        return true;
    }

    return false;
}

void MakeConfuseAction( int enemyId, Command& curCmd )
{
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

void MakeDisabledPlayerActions( Command* commands )
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


//----------------------------------------------------------------------------
// Physical
//----------------------------------------------------------------------------

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

void CalcPhysDamage( Actor* actor, Actor* target, ActionResult& strikeResult )
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

void CalcPlayerPhysDamage( const Command& curCmd, ActionResult& strikeResult, int& resultCount )
{
    const Command& cmd = curCmd;
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

    CalcPhysDamage( &actor, enemy, strikeResult );

    resultCount = 1;
    strikeResult.TargetIndex = enemyId;
    strikeResult.Died = enemy->Hp == 0;
    strikeResult.DealtDamage = true;
}

void CalcEnemyPhysDamage( const Command& curCmd, ActionResult& strikeResult, int& resultCount )
{
    const Command& cmd = curCmd;
    Enemy& actor = enemies[ cmd.actorIndex ];
    int playerId = cmd.targetIndex;
    int i = 0;
    Player::Character* player = nullptr;

    strikeResult.TargetParty = Party_Players;

    player = &Player::Party[playerId];

    CalcPhysDamage( &actor, player, strikeResult );

    resultCount = 1;
    strikeResult.TargetIndex = playerId;
    strikeResult.Died = player->hp == 0;
    strikeResult.DealtDamage = true;
}


//----------------------------------------------------------------------------
// Magic
//----------------------------------------------------------------------------

void ApplyMagicToEnemy( const Player::MagicAttr& magicAttr, CalcMagicFunc calcFunc, int index,
    ActionResult* actionResults, int& resultCount )
{
    calcFunc( magicAttr, &enemies[index], actionResults[resultCount] );

    actionResults[resultCount].TargetParty = Party_Enemies;
    actionResults[resultCount].TargetIndex = index;
    actionResults[resultCount].Died = 
        (enemies[index].Status & Status_Death) != 0 || enemies[index].Hp == 0;

    resultCount++;
}

void ApplyMagicToPlayer( const Player::MagicAttr& magicAttr, CalcMagicFunc calcFunc, int index,
    ActionResult* actionResults, int& resultCount)
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

void CalcMagicEffect( const Command& cmd, ActionResult* actionResults, int& resultCount )
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
                ApplyMagicToEnemy( magicAttr, calcFunc, i, actionResults, resultCount );
            }
        }
        break;

    case Target_AllPlayers:
        for ( int i = 0; i < Player::PartySize; i++ )
        {
            ApplyMagicToPlayer( magicAttr, calcFunc, i, actionResults, resultCount );
        }
        break;

    case Target_OneEnemy:
        for ( int i = 0; i < _countof( enemies ); i++ )
        {
            int targetIndex = (cmd.targetIndex + i) % _countof( enemies );
            if ( enemies[targetIndex].Type != InvalidEnemyType 
                && enemies[targetIndex].Hp > 0 )
            {
                ApplyMagicToEnemy( magicAttr, calcFunc, targetIndex, actionResults, resultCount );
                break;
            }
        }
        break;

    case Target_OnePlayer:
        ApplyMagicToPlayer( magicAttr, calcFunc, cmd.targetIndex, actionResults, resultCount );
        break;

    case Target_Self:
        if ( cmd.actorParty == Party_Enemies )
            ApplyMagicToEnemy( magicAttr, calcFunc, cmd.actorIndex, actionResults, resultCount );
        else
            ApplyMagicToPlayer( magicAttr, calcFunc, cmd.actorIndex, actionResults, resultCount );
        break;
    }
}


//----------------------------------------------------------------------------
// Item
//----------------------------------------------------------------------------

void CalcItemEffect( const Command& curCmd, ActionResult* actionResults, int& resultCount )
{
    const Command& cmd = curCmd;
    Command magicCmd = cmd;
    int spellId = Player::GetSpellForItem( cmd.actionId );

    resultCount = 0;

    if ( spellId != 0 )
    {
        magicCmd.actionId = spellId - 1;

        CalcMagicEffect( magicCmd, actionResults, resultCount );
    }
}

void CalcEnemyAutoHP( ActionResult* actionResults, int& resultCount )
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

void CalcPlayerAutoHP( ActionResult* actionResults, int& resultCount )
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

bool CanRunAway( const Command& curCmd )
{
    // formation doesn't allow running
    if ( (GetFormation().Flags & Formation_CantRun) != 0 )
        return false;

    // can always leave if striking first
    if ( GetEncounterType() == Encounter_PlayerFirst )
        return true;

    const Command& cmd = curCmd;
    int playerId = cmd.actorIndex;
    Player::Character& player = Player::Party[playerId];
    int r = GetNextRandom( player.level + 15 + 1 );

    return r < player.basicStats[Player::Stat_Luck];
}

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

void CommitCommand( Command& cmd )
{
    if ( cmd.action == Action_Item )
        Player::SpendItem( cmd.actionId );
}

void UndoCommand( Command& cmd )
{
    if ( cmd.action == Action_Item )
        Player::ReturnItem( cmd.actionId );
}

void UndoAllCommands( Command* commands )
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Command& cmd = commands[i];
        if ( cmd.action == Action_Item )
            Player::ReturnItem( cmd.actionId );
    }
}

}
