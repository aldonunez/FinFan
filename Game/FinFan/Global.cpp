/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Global.h"


static uint32_t timeBaseMillis;
static uint32_t runStartMillis;


int GetNextRandom( int range )
{
    // The original game uses two tables of 256 random bytes with indexes for different purposes.

    return rand() % range;
}

Direction GetOppositeDir( Direction direction )
{
    switch ( direction )
    {
    case Dir_Right: return Dir_Left;
    case Dir_Left:  return Dir_Right;
    case Dir_Down:  return Dir_Up;
    case Dir_Up:    return Dir_Down;
    default:        return Dir_None;
    }
}

void DecompressMap( const uint8_t* compressedCells, uint8_t* uncompressedCells )
{
    for ( ; *compressedCells != 0xff; compressedCells++ )
    {
        if ( (int8_t) *compressedCells < 0 )
        {
            int tileRef = *compressedCells++ & 0x7f;
            int count = *compressedCells;

            if ( count == 0 )
                count = 256;

            for ( int i = 0; i < count; i++ )
            {
                *uncompressedCells++ = tileRef;
            }
        }
        else
        {
            *uncompressedCells++ = *compressedCells;
        }
    }
}

bool LoadResource( const char* filename, ResourceLoader* loader )
{
    FILE* file = nullptr;

    errno_t err = fopen_s( &file, filename, "rb" );
    if ( err != 0 )
        return false;

    fseek( file, 0, SEEK_END );
    int fileSize = ftell( file );
    fseek( file, 0, SEEK_SET );

    if ( !loader->Load( file, fileSize ) )
        return false;

    fclose( file );

    return true;
}

struct ColorInt24
{
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
};

uint8_t Global::domains[Domains][DomainFormations];
uint8_t Global::formationWeights[FormationWeights];
uint16_t Global::prices[Prices];
static ColorInt24   nesColors[64];


bool Global::Init()
{
    if ( !LoadList( "domains.dat", domains, Domains ) )
        return false;

    if ( !LoadList( "formationWeights.dat", formationWeights, FormationWeights ) )
        return false;

    if ( !LoadList( "prices.dat", prices, Prices ) )
        return false;

    if ( !LoadList( "nesColors.dat", nesColors, _countof( nesColors ) ) )
        return false;

    return true;
}

ALLEGRO_COLOR Global::GetSystemColor( int colorIndex )
{
    ColorInt24& color = nesColors[colorIndex];
    return al_map_rgb( color.Red, color.Green, color.Blue );
}

int Global::GetBattleFormation( int domain )
{
    int baseIndex = domain * DomainFormations;
    int r = GetNextRandom( FormationWeights );
    int index = formationWeights[r];

    return domains[domain][index];
}

int Global::GetPrice( int itemId )
{
    return prices[itemId];
}

uint32_t Global::GetTime()
{
    uint32_t procMillis = al_get_time() * 1000;
    uint32_t timeMillis = timeBaseMillis + (procMillis - runStartMillis);

    return timeMillis;
}

void Global::SetTime( uint32_t timeMillis )
{
    timeBaseMillis = timeMillis;
    runStartMillis = al_get_time() * 1000;
}

void Global::GetHoursMinutes( uint32_t millis, uint32_t& hours, uint32_t& minutes )
{
    uint32_t totalMinutes = millis / (1000 * 60);
    hours = totalMinutes / 60;
    minutes = totalMinutes - (hours * 60);
}

void Actor::AddHp( int value )
{
    int hp = GetHp() + value;
    if ( hp <= 0 )
    {
        hp = 0;
        SetStatus( GetStatus() | Status_Death );
    }
    else if ( hp > GetMaxHp() )
    {
        hp = GetMaxHp();
    }
    SetHp( hp );
}

void Actor::AddStatus( int value )
{
    SetStatus( GetStatus() | value );

    if ( (value & Status_Death) != 0 )
        SetHp( 0 );

    if ( (value & Status_Stone) != 0
        && GetEnemyClasses() != 0 )
    {
        SetHp( 0 );
        SetStatus( GetStatus() | Status_Death );
    }
}

void Actor::RemoveStatus( int value )
{
    SetStatus( GetStatus() & ~value );
}

void Actor::AddHitMultiplier( int value )
{
    int multiplier = GetHitMultiplier() + value;
    if ( multiplier < 0 )
        multiplier = 0;
    else if ( multiplier > 2 )
        multiplier = 2;
    SetHitMultiplier( multiplier );
}

void Actor::LowerMorale( int value )
{
    int morale = GetMorale();
    if ( morale < MaxMorale )
    {
        if ( value > morale )
            morale = 0;
        else
            morale -= value;
        SetMorale( morale );
    }
    // bosses have max morale and don't flee
}

void Actor::AddElementStrength( int value )
{
    SetElementStrength( GetElementStrength() | value );
}

void Actor::AddAbsorb( int value )
{
    int absorb = GetAbsorb() + value;
    SetAbsorb( absorb );
}

void Actor::AddDamage( int value )
{
    int damage = GetDamage() + value;
    SetDamage( damage );
}

void Actor::AddHitRate( int value )
{
    int hitRate = GetHitRate() + value;
    SetHitRate( hitRate );
}

void Actor::AddEvadeRate( int value )
{
    int EvadeRate = GetEvadeRate() + value;
    SetEvadeRate( EvadeRate );
}

bool Actor::IsAlive()
{
    return GetHp() > 0 
        && (GetStatus() & Status_Death) == 0
        && (GetStatus() & Status_Stone) == 0;
}

bool Actor::IsStrictlyAlive()
{
    return GetHp() > 0 
        && (GetStatus() & Status_Death) == 0;
}
