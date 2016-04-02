/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "ObjEvents.h"
#include "Player.h"
#include "Ids.h"
#include "Sound.h"


extern CheckRoutine checkRoutines[ObjectTypes];

    
static void PlayFanfare()
{
    Sound::PushTrack( Sound_Fanfare, 0 );
}

static void PlayGotItem()
{
    Sound::PushTrack( Sound_GotItem, 0 );
}


static void Talk_None( CheckParams params, CheckResult& result )
{
}


CheckRoutine GetObjectRoutine( int type )
{
    if ( type < 0 || type >= _countof( checkRoutines ) )
        return Talk_None;

    return checkRoutines[type];
}


static void Talk_norm( CheckParams params, CheckResult& result )
{
    result.Message = params[1];
}

static void Talk_ifevent( CheckParams params, CheckResult& result )
{
    if ( Player::GetEvent( params[0] ) )
        result.Message = params[2];
    else
        result.Message = params[1];
}

static void Talk_ifvis( CheckParams params, CheckResult& result )
{
    if ( Player::GetObjVisible( params[0] ) )
        result.Message = params[2];
    else
        result.Message = params[1];
}

static void Talk_ifitem( CheckParams params, CheckResult& result )
{
    if ( Player::Items[ params[0] ] == 0 )
        result.Message = params[2];
    else
        result.Message = params[1];
}

static void Talk_fight( CheckParams params, CheckResult& result )
{
    Player::SetObjVisible( params[0], false );
    result.Fight = true;
    result.FormationId = params[3];
    result.Message = params[1];
}

static void Talk_Replace( CheckParams params, CheckResult& result )
{
    Player::SetObjVisible( params[0], false );
    Player::SetObjVisible( params[3], true );
    result.Message = params[1];
}

static void Talk_4Orb( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_OrbAir] != 0 
        && Player::Items[Item_OrbEarth] != 0 
        && Player::Items[Item_OrbFire] != 0 
        && Player::Items[Item_OrbWater] != 0 )
    {
        result.Message = params[1];
    }
    else
    {
        result.Message = params[2];
    }
}

static void Talk_KingConeria( CheckParams params, CheckResult& result )
{
    if ( !Player::GetObjVisible( Obj_Princess2 ) )
    {
        result.Message = params[1];
    }
    else if ( !Player::IsBridgeVisible() )
    {
        Player::SetBridgeVisible( true );
        PlayFanfare();
        result.Message = params[2];
    }
    else
    {
        result.Message = params[3];
    }
}

static void Talk_Garland( CheckParams params, CheckResult& result )
{
    Player::SetObjVisible( Obj_Garland, false );
    result.Message = params[1];
    result.Fight = true;
    result.FormationId = Fight_Garland;
}

static void Talk_Princess1( CheckParams params, CheckResult& result )
{
    Player::SetObjVisible( Obj_Princess1, false );
    Player::SetObjVisible( Obj_Princess2, true );
    result.Message = params[1];
    result.Teleport = true;
    result.TeleportId = LTele_SavedPrincess;
}

static void Talk_Bikke( CheckParams params, CheckResult& result )
{
    if ( !Player::GetEvent( Obj_Bikke ) )
    {
        Player::SetEvent( Obj_Bikke, true );
        result.Fight = true;
        result.FormationId = Fight_Bikke;
        result.Message = params[1];
    }
    else if ( (Player::GetVehicles() & Vehicle_Ship) == 0 )
    {
        result.Message = params[2];
        Player::SetVehicles( (Vehicle) (Player::GetVehicles() | Vehicle_Ship) );
        Player::SetObjVisible( Obj_Pravoka1, true );
        Player::SetObjVisible( Obj_Pravoka2, true );
        Player::SetObjVisible( Obj_Pravoka3, true );
        PlayFanfare();
    }
    else
    {
        result.Message = params[3];
    }
}

static void Talk_ElfDoc( CheckParams params, CheckResult& result )
{
    if ( Player::GetEvent( Obj_ElfPrince ) )
    {
        result.Message = params[3];
    }
    else if ( Player::Items[Item_Herb] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        Player::Items[Item_Herb]--;
        Player::SetEvent( Obj_ElfPrince, true );
        PlayFanfare();
        result.Message = params[2];
    }
}

static void Talk_ElfPrince( CheckParams params, CheckResult& result )
{
    if ( !Player::GetEvent( Obj_ElfPrince ) )
    {
        result.Message = params[3];
    }
    else if ( Player::Items[Item_MysticKey] != 0 )
    {
        result.Message = params[2];
    }
    else
    {
        Player::Items[Item_MysticKey]++;
        PlayFanfare();
        result.Message = params[1];
    }
}

static void Talk_Astos( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Crown] == 0 )
        result.Message = params[1];
    else
    {
        Player::Items[Item_Crystal]++;
        Player::SetObjVisible( Obj_Astos, false );
        PlayFanfare();
        result.Fight = true;
        result.FormationId = Fight_Astos;
        result.Message = params[2];
    }
}

static void Talk_Nerrick( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Tnt] == 0 )
        result.Message = params[1];
    else
    {
        Player::Items[Item_Tnt]--;
        Player::SetCanalBlocked( false );
        Player::SetObjVisible( Obj_Nerrick, false );
        PlayFanfare();
        result.Message = params[2];
    }
}

static void Talk_Smith( CheckParams params, CheckResult& result )
{
    if ( Player::GetEvent( Obj_Smith ) )
    {
        result.Message = params[3];
    }
    else if ( Player::Items[Item_Adamant] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        // always fits in inventory
        Player::Items[Item_Xcalbur]++;
        Player::SetEvent( Obj_Smith, true );
        Player::Items[Item_Adamant]--;
        PlayFanfare();
        result.Message = params[2];
    }
}

static void Talk_Matoya( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Herb] != 0 )
    {
        result.Message = params[3];
    }
    else
    {
        if ( Player::Items[Item_Crystal] == 0 )
        {
            if ( !Player::GetEvent( Obj_ElfPrince ) )
                result.Message = params[1];
            else
                result.Message = params[3];
        }
        else
        {
            Player::Items[Item_Crystal]--;
            Player::Items[Item_Herb]++;
            PlayFanfare();
            result.Message = params[2];
        }
    }
}

static void Talk_Unne( CheckParams params, CheckResult& result )
{
    if ( Player::GetEvent( Obj_Unne ) )
    {
        result.Message = params[3];
    }
    else if ( Player::Items[Item_Slab] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        Player::Items[Item_Slab]--;
        Player::SetEvent( Obj_Unne, true );
        PlayFanfare();
        result.Message = params[2];
    }
}

static void Talk_Vampire( CheckParams params, CheckResult& result )
{
    Player::SetObjVisible( Obj_Vampire, false );
    result.Fight = true;
    result.FormationId = Fight_Vampire;
    result.Message = params[1];
}

static void Talk_Sarda( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Rod] == 0 
        && !Player::GetObjVisible( Obj_Vampire ) )
    {
        Player::Items[Item_Rod]++;
        PlayFanfare();
        result.Message = params[1];
    }
    else
    {
        result.Message = params[2];
    }
}

static void UpgradeClass()
{
    const PlayerClass Upgrades[6] = 
    {
        Class_Knight,
        Class_Ninja,
        Class_Master,
        Class_RedWizard,
        Class_WhiteWizard,
        Class_BlackWizard
    };

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        PlayerClass oldClass = (PlayerClass) Player::Party[i]._class;
        Player::Party[i]._class = Upgrades[oldClass];
    }
}

static void Talk_Bahamut( CheckParams params, CheckResult& result )
{
    if ( Player::GetEvent( Obj_Bahamut ) )
    {
        result.Message = params[3];
    }
    else if ( Player::Items[Item_Tail] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        Player::Items[Item_Tail]--;
        Player::SetEvent( Obj_Bahamut, true );
        UpgradeClass();
        PlayFanfare();
        result.Message = params[2];
    }
}

static void Talk_SubEng( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Oxyale] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        Player::SetObjVisible( Obj_SubEngineer, false );
        result.Message = params[2];
    }
}

static void Talk_CubeBot( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Cube] != 0 )
        result.Message = params[2];
    else
    {
        result.Message = params[1];
        Player::Items[Item_Cube]++;
        PlayFanfare();
    }
}

static void Talk_Princess2( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Lute] != 0 )
        result.Message = params[2];
    else
    {
        result.Message = params[1];
        Player::Items[Item_Lute]++;
        PlayFanfare();
    }
}

static void Talk_Fairy( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Oxyale] != 0 )
        result.Message = params[2];
    else
    {
        result.Message = params[1];
        Player::Items[Item_Oxyale]++;
        PlayFanfare();
    }
}

static void Talk_Titan( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Ruby] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        Player::Items[Item_Ruby]--;
        Player::SetObjVisible( Obj_Titan, false );
        PlayFanfare();
        result.Message = params[2];
    }
}

static void Talk_CanoeSage( CheckParams params, CheckResult& result )
{
    if ( (Player::GetVehicles() & Vehicle_Canoe) == 0
        && Player::Items[Item_OrbEarth] != 0 )
    {
        result.Message = params[1];
        Player::Items[Item_Canoe]++;
        Player::SetVehicles( (Vehicle) (Player::GetVehicles() | Vehicle_Canoe) );
        PlayFanfare();
    }
    else
    {
        result.Message = params[2];
    }
}

static void Talk_ifbridge( CheckParams params, CheckResult& result )
{
    if ( Player::IsBridgeVisible() )
        result.Message = params[1];
    else
        result.Message = params[2];
}

static void Talk_GoBridge( CheckParams params, CheckResult& result )
{
    if ( Player::GetObjVisible( Obj_Princess2 ) 
        && !Player::IsBridgeVisible() )
    {
        result.Message = params[1];
    }
    else
        result.Message = params[2];
}

static void Talk_Invis( CheckParams params, CheckResult& result )
{
    if ( !Player::GetObjVisible( Obj_Princess2 ) 
        && Player::Items[Item_Lute] == 0 )
    {
        result.Message = params[1];
    }
    else
        result.Message = params[2];
}

static void Talk_ifcanoe( CheckParams params, CheckResult& result )
{
    if ( (Player::GetVehicles() & Vehicle_Canoe) != 0 )
        result.Message = params[1];
    else
        result.Message = params[2];
}

static void Talk_ifcanal( CheckParams params, CheckResult& result )
{
    if ( !Player::IsCanalBlocked() )
        result.Message = params[1];
    else
        result.Message = params[2];
}

static void Talk_ifkeytnt( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_MysticKey] != 0 
        && Player::Items[Item_Tnt] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        result.Message = params[2];
    }
}

static void Talk_ifairship( CheckParams params, CheckResult& result )
{
    if ( (Player::GetVehicles() & Vehicle_Airship) == 0 )
        result.Message = params[1];
    else
        result.Message = params[2];
}

static void Talk_ifearthvamp( CheckParams params, CheckResult& result )
{
    if ( !Player::GetObjVisible( Obj_Vampire )
        && Player::Items[Item_OrbEarth] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        result.Message = params[2];
    }
}

static void Talk_CoOGuy( CheckParams params, CheckResult& result )
{
    Player::SetObjVisible( params[0], false );
    result.Message = params[1];
}

static void Talk_ifearthfire( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_OrbEarth] != 0 
        && Player::Items[Item_OrbFire] == 0 )
    {
        result.Message = params[1];
    }
    else
    {
        result.Message = params[2];
    }
}

static void Talk_CubeBotBad( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_Cube] != 0 )
        result.Message = params[2];
    else
    {
        result.Message = params[1];
        Player::Items[Item_Cube]++;
        // like Talk_CubeBot, but no fanfare
    }
}

static void Talk_Chime( CheckParams params, CheckResult& result )
{
    if ( !Player::GetEvent( Obj_Unne ) )
        result.Message = params[3];
    else if ( Player::Items[Item_Chime] != 0 )
        result.Message = params[2];
    else
    {
        Player::Items[Item_Chime]++;
        PlayFanfare();
        result.Message = params[1];
    }
}

static void Talk_BlackOrb( CheckParams params, CheckResult& result )
{
    if ( Player::Items[Item_OrbAir] != 0 
        && Player::Items[Item_OrbEarth] != 0 
        && Player::Items[Item_OrbFire] != 0 
        && Player::Items[Item_OrbWater] != 0 )
    {
        Player::SetObjVisible( Obj_BlackOrb, false );
        PlayGotItem();
        result.Message = params[1];
    }
    else
    {
        result.Message = params[2];
    }
}


static CheckRoutine checkRoutines[ObjectTypes] = 
{
/* 00 */    Talk_None,
/* 01 */    Talk_KingConeria,
/* 02 */    Talk_Garland,
/* 03 */    Talk_Princess1,
/* 04 */    Talk_Bikke,
/* 05 */    Talk_ElfDoc,
/* 06 */    Talk_ElfPrince,
/* 07 */    Talk_Astos,
/* 08 */    Talk_Nerrick,
/* 09 */    Talk_Smith,
/* 0A */    Talk_Matoya,
/* 0B */    Talk_Unne,
/* 0C */    Talk_Vampire,
/* 0D */    Talk_Sarda,
/* 0E */    Talk_Bahamut,
/* 0F */    Talk_ifvis,
/* 10 */    Talk_SubEng,
/* 11 */    Talk_CubeBot,
/* 12 */    Talk_Princess2,
/* 13 */    Talk_Fairy,
/* 14 */    Talk_Titan,
/* 15 */    Talk_CanoeSage,
/* 16 */    Talk_norm,
/* 17 */    Talk_norm,
/* 18 */    Talk_Replace,
/* 19 */    Talk_Replace,
/* 1A */    Talk_fight,
/* 1B */    Talk_fight,
/* 1C */    Talk_fight,
/* 1D */    Talk_fight,
/* 1E */    Talk_fight,
/* 1F */    Talk_None,
/* 20 */    Talk_ifvis,
/* 21 */    Talk_ifvis,
/* 22 */    Talk_ifvis,
/* 23 */    Talk_ifitem,
/* 24 */    Talk_ifvis,
/* 25 */    Talk_ifvis,
/* 26 */    Talk_Invis,
/* 27 */    Talk_ifbridge,
/* 28 */    Talk_ifvis,
/* 29 */    Talk_ifvis,
/* 2A */    Talk_ifvis,
/* 2B */    Talk_ifvis,
/* 2C */    Talk_ifitem,
/* 2D */    Talk_ifvis,
/* 2E */    Talk_ifitem,
/* 2F */    Talk_ifevent,
/* 30 */    Talk_ifvis,
/* 31 */    Talk_ifvis,
/* 32 */    Talk_GoBridge,
/* 33 */    Talk_ifvis,
/* 34 */    Talk_4Orb,
/* 35 */    Talk_norm,
/* 36 */    Talk_norm,
/* 37 */    Talk_ifvis,
/* 38 */    Talk_ifvis,
/* 39 */    Talk_norm,
/* 3A */    Talk_4Orb,
/* 3B */    Talk_4Orb,
/* 3C */    Talk_4Orb,
/* 3D */    Talk_4Orb,
/* 3E */    Talk_4Orb,
/* 3F */    Talk_norm,
/* 40 */    Talk_norm,
/* 41 */    Talk_norm,
/* 42 */    Talk_ifevent,
/* 43 */    Talk_ifevent,
/* 44 */    Talk_ifevent,
/* 45 */    Talk_norm,
/* 46 */    Talk_ifevent,
/* 47 */    Talk_ifitem,
/* 48 */    Talk_norm,
/* 49 */    Talk_ifevent,
/* 4A */    Talk_ifevent,
/* 4B */    Talk_ifitem,
/* 4C */    Talk_ifevent,
/* 4D */    Talk_ifevent,
/* 4E */    Talk_ifevent,
/* 4F */    Talk_ifevent,
/* 50 */    Talk_ifevent,
/* 51 */    Talk_ifevent,
/* 52 */    Talk_norm,
/* 53 */    Talk_ifcanoe,
/* 54 */    Talk_ifitem,
/* 55 */    Talk_ifevent,
/* 56 */    Talk_ifevent,
/* 57 */    Talk_norm,
/* 58 */    Talk_norm,
/* 59 */    Talk_ifcanal,
/* 5A */    Talk_norm,
/* 5B */    Talk_norm,
/* 5C */    Talk_ifitem,
/* 5D */    Talk_ifitem,
/* 5E */    Talk_norm,
/* 5F */    Talk_ifcanal,
/* 60 */    Talk_ifkeytnt,
/* 61 */    Talk_norm,
/* 62 */    Talk_ifcanal,
/* 63 */    Talk_norm,
/* 64 */    Talk_norm,
/* 65 */    Talk_norm,
/* 66 */    Talk_norm,
/* 67 */    Talk_norm,
/* 68 */    Talk_ifvis,
/* 69 */    Talk_norm,
/* 6A */    Talk_ifearthvamp,
/* 6B */    Talk_ifitem,
/* 6C */    Talk_ifvis,
/* 6D */    Talk_ifearthvamp,
/* 6E */    Talk_norm,
/* 6F */    Talk_norm,
/* 70 */    Talk_ifitem,
/* 71 */    Talk_ifairship,
/* 72 */    Talk_norm,
/* 73 */    Talk_ifevent,
/* 74 */    Talk_ifitem,
/* 75 */    Talk_norm,
/* 76 */    Talk_norm,
/* 77 */    Talk_norm,
/* 78 */    Talk_4Orb,
/* 79 */    Talk_4Orb,
/* 7A */    Talk_4Orb,
/* 7B */    Talk_4Orb,
/* 7C */    Talk_4Orb,
/* 7D */    Talk_4Orb,
/* 7E */    Talk_4Orb,
/* 7F */    Talk_ifitem,
/* 80 */    Talk_ifearthfire,
/* 81 */    Talk_ifitem,
/* 82 */    Talk_norm,
/* 83 */    Talk_norm,
/* 84 */    Talk_CoOGuy,
/* 85 */    Talk_norm,
/* 86 */    Talk_norm,
/* 87 */    Talk_norm,
/* 88 */    Talk_norm,
/* 89 */    Talk_norm,
/* 8A */    Talk_norm,
/* 8B */    Talk_norm,
/* 8C */    Talk_norm,
/* 8D */    Talk_norm,
/* 8E */    Talk_norm,
/* 8F */    Talk_norm,
/* 90 */    Talk_norm,
/* 91 */    Talk_norm,
/* 92 */    Talk_norm,
/* 93 */    Talk_norm,
/* 94 */    Talk_ifitem,
/* 95 */    Talk_norm,
/* 96 */    Talk_norm,
/* 97 */    Talk_norm,
/* 98 */    Talk_norm,
/* 99 */    Talk_norm,
/* 9A */    Talk_ifitem,
/* 9B */    Talk_ifevent,
/* 9C */    Talk_norm,
/* 9D */    Talk_norm,
/* 9E */    Talk_norm,
/* 9F */    Talk_norm,
/* A0 */    Talk_norm,
/* A1 */    Talk_norm,
/* A2 */    Talk_CubeBotBad,
/* A3 */    Talk_norm,
/* A4 */    Talk_norm,
/* A5 */    Talk_norm,
/* A6 */    Talk_norm,
/* A7 */    Talk_norm,
/* A8 */    Talk_norm,
/* A9 */    Talk_ifitem,
/* AA */    Talk_norm,
/* AB */    Talk_norm,
/* AC */    Talk_norm,
/* AD */    Talk_norm,
/* AE */    Talk_norm,
/* AF */    Talk_ifevent,
/* B0 */    Talk_norm,
/* B1 */    Talk_norm,
/* B2 */    Talk_norm,
/* B3 */    Talk_norm,
/* B4 */    Talk_norm,
/* B5 */    Talk_norm,
/* B6 */    Talk_norm,
/* B7 */    Talk_norm,
/* B8 */    Talk_norm,
/* B9 */    Talk_norm,
/* BA */    Talk_norm,
/* BB */    Talk_Chime,
/* BC */    Talk_ifevent,
/* BD */    Talk_ifevent,
/* BE */    Talk_ifevent,
/* BF */    Talk_ifevent,
/* C0 */    Talk_ifevent,
/* C1 */    Talk_ifevent,
/* C2 */    Talk_ifevent,
/* C3 */    Talk_ifevent,
/* C4 */    Talk_ifevent,
/* C5 */    Talk_ifevent,
/* C6 */    Talk_ifevent,
/* C7 */    Talk_ifevent,
/* C8 */    Talk_ifevent,
/* C9 */    Talk_ifevent,
/* CA */    Talk_BlackOrb,
/* CB */    Talk_norm,
/* CC */    Talk_norm,
/* CD */    Talk_norm,
/* CE */    Talk_norm,
/* CF */    Talk_norm,
/* D0 */    Talk_norm,
/* D1 */    Talk_norm,
/* D2 */    Talk_norm,
/* D3 */    Talk_norm,
/* D4 */    Talk_norm,
/* D5 */    Talk_norm,
/* D6 */    Talk_norm,
/* D7 */    Talk_norm,
};
