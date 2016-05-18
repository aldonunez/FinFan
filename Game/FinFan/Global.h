/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum Party
{
    Party_None      = 0,
    Party_Enemies   = 1,
    Party_Players   = 2,
};

enum PlayerClass
{
    Class_Fighter,
    Class_Thief,
    Class_BlackBelt,
    Class_RedMage,
    Class_WhiteMage,
    Class_BlackMage,

    Class_Knight,
    Class_Ninja,
    Class_Master,
    Class_RedWizard,
    Class_WhiteWizard,
    Class_BlackWizard
};

enum EnemyClass
{
    EnemyClass_MagicBeast   = 1,
    EnemyClass_Dragon       = 2,
    EnemyClass_Giant        = 4,
    EnemyClass_Undead       = 8,
    EnemyClass_Were         = 0x10,
    EnemyClass_Water        = 0x20,
    EnemyClass_Mage         = 0x40,
    EnemyClass_Regen        = 0x80
};

enum Element
{
    Elem_None,
    Elem_Status = 1,
    Elem_Poison = 2,
    Elem_Time   = 4,
    Elem_Death  = 8,
    Elem_Fire   = 0x10,
    Elem_Cold   = 0x20,
    Elem_Lit    = 0x40,
    Elem_Earth  = 0x80
};

enum Status
{
    Status_None,
    Status_Death        = 1,
    Status_Stone        = 2,
    Status_Poison       = 4,
    Status_Dark         = 8,
    Status_Paralysis    = 0x10,
    Status_Sleep        = 0x20,
    Status_Silence      = 0x40,
    Status_Confusion    = 0x80,

    Status_AllLiving    = 0xfe,
    Status_All          = 0xff,

    Status_NoInput      = Status_Death | Status_Stone | Status_Paralysis | 
                          Status_Sleep | Status_Confusion,
    Status_AllStopped   = Status_Death | Status_Stone | Status_Paralysis | 
                          Status_Sleep,
};


enum Direction
{
    Dir_None,
    Dir_Right,
    Dir_Left,
    Dir_Down,
    Dir_Up
};


enum Vehicle
{
    Vehicle_Foot    = 1,
    Vehicle_Canoe   = 2,
    Vehicle_Ship    = 4,
    Vehicle_Airship = 8,
};


struct Bounds
{
    uint8_t X;
    uint8_t Y;
    uint8_t Width;
    uint8_t Height;
};

struct Bounds16
{
    uint16_t X;
    uint16_t Y;
    uint16_t Width;
    uint16_t Height;
};

struct Point
{
    uint8_t X;
    uint8_t Y;
};

struct LTeleport
{
    uint8_t Col;
    uint8_t Row;
    uint8_t MapId;
    uint8_t Reserved;
};

struct OWTeleport
{
    uint8_t Col;
    uint8_t Row;
};


struct Actor
{
    virtual int  GetHp() = 0;
    virtual void SetHp( int value ) = 0;
    virtual int  GetStatus() = 0;
    virtual void SetStatus( int value ) = 0;
    virtual int  GetHitMultiplier() = 0;
    virtual void SetHitMultiplier( int value ) = 0;
    virtual int  GetMorale() = 0;
    virtual void SetMorale( int value ) = 0;
    virtual int  GetAbsorb() = 0;
    virtual void SetAbsorb( int value ) = 0;
    virtual int  GetEvadeRate() = 0;
    virtual void SetEvadeRate( int value ) = 0;
    virtual int  GetElementStrength() = 0;
    virtual void SetElementStrength( int value ) = 0;
    virtual int  GetDamage() = 0;
    virtual void SetDamage( int value ) = 0;
    virtual int  GetHitRate() = 0;
    virtual void SetHitRate( int value ) = 0;

    virtual int GetElementWeakness() = 0;
    virtual int GetMagicAbsorb() = 0;
    virtual int GetEnemyClasses() = 0;
    virtual int GetMaxHp() = 0;

    virtual int GetCritHitRate() = 0;
    virtual int GetMaxHits() = 0;
    virtual int GetTargetClasses() = 0;
    virtual int GetTargetElement() = 0;
    virtual int GetTargetStatus() = 0;

    void AddHp( int value );
    void AddStatus( int value );
    void RemoveStatus( int value );
    void AddHitMultiplier( int value );
    void LowerMorale( int value );
    void AddElementStrength( int value );
    void AddAbsorb( int value );
    void AddDamage( int value );
    void AddHitRate( int value );
    void AddEvadeRate( int value );
    bool IsAlive();
    bool IsStrictlyAlive();
};

struct ActionResult
{
    Party   TargetParty;
    int     TargetIndex;
    bool    Missed;
    bool    Died;
    bool    CritHit;
    bool    DealtDamage;
    int     Damage;
    // Used in ATB only
    int     OrigStatus;
};


const int Players = 4;
const int NameLength = 6;

const int MaxXP = 999999;
const int MaxG = 999999;
const int MaxMorale = 0xff;

const int SpellLevels = 8;
const int SpellSlots = 3;
const int SpellsInLevel = 8;

const int NoWeapon = 0xff;
const int NoArmor = 0xff;
const int NoMagic = 0;
const int NoItem = 0;

const int ConfirmKey = ALLEGRO_KEY_F;
const int CancelKey = ALLEGRO_KEY_D;
const int MenuKey = ALLEGRO_KEY_ENTER;

const int StdViewWidth = 256;
const int StdViewHeight = 240;


int GetNextRandom( int range );
Direction GetOppositeDir( Direction direction );
int GetFrameCounter();
int GetScreenScale();


class Color
{
public:
    static ALLEGRO_COLOR White()
    {
        return al_map_rgba( 255, 255, 255, 255 );
    }

    static ALLEGRO_COLOR Black()
    {
        return al_map_rgba( 0, 0, 0, 255 );
    }

    static ALLEGRO_COLOR Transparent()
    {
        return al_map_rgba( 0, 0, 0, 0 );
    }
};


class ResourceLoader
{
public:
    virtual bool Load( FILE* file, size_t fileSize ) = 0;
};


template <typename T, int Length>
class Table : public ResourceLoader
{
    uint16_t    offsets[Length];
    uint8_t*    buffer;

public:
    Table()
        :   buffer( nullptr )
    {
    }

    ~Table()
    {
        delete [] buffer;
    }

    virtual bool Load( FILE* file, size_t fileSize ) override
    {
        assert( file != nullptr );
        assert( buffer == nullptr );

        int heapSize = fileSize - sizeof offsets;

        buffer = new uint8_t[heapSize];
        if ( buffer == nullptr )
            return false;

        fread( offsets, 1, sizeof offsets, file );
        fread( buffer, 1, heapSize, file );

        return true;
    }

    const T* GetItem( size_t index )
    {
        if ( index >= Length )
            return nullptr;

        return (T*) (buffer + offsets[index]);
    }
};


inline bool operator==( const Point& left, const Point& right )
{
    return left.X == right.X && left.Y == right.Y;
}

inline bool operator!=( const Point& left, const Point& right )
{
    return !(left == right);
}


void DecompressMap( const uint8_t* compressedCells, uint8_t* uncompressedCells );

bool LoadResource( const char* filename, ResourceLoader* loader );

template <typename T>
bool LoadList( const char* filename, T* list, size_t length )
{
    FILE* file = nullptr;

    errno_t err = fopen_s( &file, filename, "rb" );
    if ( err != 0 )
        return false;

    fread( list, sizeof T, length, file );
    fclose( file );

    return true;
}


class Global
{
    static const int Domains = 128;
    static const int DomainFormations = 8;
    static const int FormationWeights = 64;
    static const int Prices = 256;

    static uint8_t domains[Domains][DomainFormations];
    static uint8_t formationWeights[FormationWeights];
    static uint16_t prices[Prices];

public:
    static bool Init();

    static int GetBattleFormation( int domain );
    static int GetPrice( int itemId );

    static uint32_t GetTime();
    static void SetTime( uint32_t timeMillis );
    static void GetHoursMinutes( uint32_t millis, uint32_t& hours, uint32_t& minutes );

    static ALLEGRO_COLOR GetSystemColor( int colorIndex );
};
