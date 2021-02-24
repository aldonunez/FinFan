/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Player.h"


namespace Player
{
    enum GameFlags
    {
        Game_ObjVisible     = 1,
        Game_Event          = 2,
        Game_ChestOpened    = 4,
    };

    enum LevelUpAttr
    {
        LevelUpAttr_Strong = 0x20,
    };

    struct ClassInit
    {
        uint8_t Class;
        uint8_t MaxHp;
        uint8_t BasicStats[Stat_Max];
        uint8_t Damage;
        uint8_t HitRate;
        uint8_t EvadeRate;
        uint8_t MagicAbsorb;
        uint8_t Reserved[5];
    };

    typedef uint8_t SpellCharges[50];
    typedef uint8_t LevelUpAttrs[50];


    const int MagicNamesBase = 0xB0;
    const int ClassNamesBase = 0xF0;
    const int GameFlagCount = 256;

    const uint16_t ClassEquipBit[] =
    {
        0x800,
        0x400,
        0x200,
        0x100,
        0x080,
        0x040,

        0x020,
        0x010,
        0x008,
        0x004,
        0x002,
        0x001
    };


    Character Party[PartySize];
    uint8_t Items[ItemTypes];
    int gil;
    Vehicle vehicles;
    Vehicle activeVehicle;
    Point playerPos;
    Point shipPos;
    Point airshipPos;
    uint8_t airshipVisibility;
    bool isCanalBlocked;
    bool isBridgeVisible;
    bool wasOpeningScenePlayed;
    uint8_t gameFlags[GameFlagCount];

    WeaponAttr      weaponAttrs[40];
    ArmorAttr       armorAttrs[40];
    MagicAttr       magicAttrs[64];
    MagicAttr       specialAttrs[26];
    int             levelXp[49];
    SpellCharges    spellChargeBoosts[12];
    LevelUpAttrs    levelUpAttrBoosts[12];
    ClassInit       classInit[12];
    uint16_t        itemNameOffsets[256];
    char*           itemNames;
    uint16_t        specialNameOffsets[26];
    char*           specialNames;
    uint8_t         armorTypes[40];
    uint16_t        armorPerms[40];
    uint16_t        weaponPerms[40];
    uint8_t         magicPerms[12][8];


    bool Init()
    {
        FILE* file = nullptr;
        errno_t err = 0;


        err = fopen_s( &file, "magicPerms.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( magicPerms, sizeof magicPerms[0], _countof( magicPerms ), file );
        fclose( file );


        err = fopen_s( &file, "weaponPerms.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( weaponPerms, sizeof weaponPerms[0], _countof( weaponPerms ), file );
        fclose( file );


        err = fopen_s( &file, "armorPerms.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( armorPerms, sizeof armorPerms[0], _countof( armorPerms ), file );
        fclose( file );


        err = fopen_s( &file, "armorTypes.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( armorTypes, sizeof armorTypes[0], _countof( armorTypes ), file );
        fclose( file );


        err = fopen_s( &file, "weaponAttr.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( weaponAttrs, sizeof weaponAttrs[0], _countof( weaponAttrs ), file );
        fclose( file );


        err = fopen_s( &file, "armorAttr.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( armorAttrs, sizeof armorAttrs[0], _countof( armorAttrs ), file );
        fclose( file );


        err = fopen_s( &file, "magicAttr.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( magicAttrs, sizeof magicAttrs[0], _countof( magicAttrs ), file );
        fclose( file );


        err = fopen_s( &file, "specialAttr.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( specialAttrs, sizeof specialAttrs[0], _countof( specialAttrs ), file );
        fclose( file );


        err = fopen_s( &file, "xp.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( levelXp, sizeof levelXp[0], _countof( levelXp ), file );
        fclose( file );


        err = fopen_s( &file, "chargeBoost.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( spellChargeBoosts, sizeof spellChargeBoosts, 1, file );
        fclose( file );


        err = fopen_s( &file, "levelUpAttrs.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( levelUpAttrBoosts, sizeof levelUpAttrBoosts, 1, file );
        fclose( file );


        err = fopen_s( &file, "initClass.dat", "rb" );
        if ( err != 0 )
            return false;

        fread( classInit, sizeof classInit[0], _countof( classInit ), file );
        fclose( file );

        memcpy( classInit + 6, classInit, sizeof ClassInit * 6 );


        err = fopen_s( &file, "itemNames.tab", "rb" );
        if ( err != 0 )
            return false;

        fseek( file, 0, SEEK_END );
        int fileSize = ftell( file );
        int heapSize = fileSize - sizeof itemNameOffsets;
        fseek( file, 0, SEEK_SET );

        fread( itemNameOffsets, 1, sizeof itemNameOffsets, file );
        itemNames = new char[heapSize];
        fread( itemNames, 1, heapSize, file );
        fclose( file );


        err = fopen_s( &file, "specialNames.tab", "rb" );
        if ( err != 0 )
            return false;

        fseek( file, 0, SEEK_END );
        fileSize = ftell( file );
        heapSize = fileSize - sizeof specialNameOffsets;
        fseek( file, 0, SEEK_SET );

        fread( specialNameOffsets, 1, sizeof specialNameOffsets, file );
        specialNames = new char[heapSize];
        fread( specialNames, 1, heapSize, file );
        fclose( file );


        return true;
    }

    void InitParty( const PlayerInfo* playerInfos )
    {
        for ( int i = 0; i < Players; i++ )
        {
            Character& player = Party[i];
            ClassInit& init = classInit[playerInfos[i].Class];

            strcpy_s( player.name, playerInfos[i].Name );

            player._class = init.Class;
            player.maxHp = init.MaxHp;
            player.damage = init.Damage;
            player.hitRate = init.HitRate;
            player.evadeRate = init.EvadeRate;
            player.magicAbsorb = init.MagicAbsorb;

            for ( int j = 0; j < Stat_Max; j++ )
                player.basicStats[j] = init.BasicStats[j];

            player.level = 1;
            player.xp = 0;
            player.hp = player.maxHp;
            player.status = 0;
            player.hitMultiplier = 1;
            player.weaponId = NoWeapon;

            for ( int j = 0; j < Armor_Max; j++ )
                player.armorIds[j] = NoArmor;

            uint8_t spellBoostL1 = spellChargeBoosts[player._class][0];

            for ( int j = 0; j < SpellLevels; j++ )
            {
                player.spellMaxCharge[j] = (j == 0) ? spellBoostL1 : 0;
                player.spellCharge[j] = player.spellMaxCharge[j];

                for ( int k = 0; k < SpellSlots; k++ )
                    player.spells[j][k] = NoMagic;
            }

            CalcDerivedStats( i );
        }
    }

    void InitFile( const PlayerInfo* playerInfos )
    {
        gil = 400;
        vehicles = Vehicle_Foot;
        activeVehicle = Vehicle_Foot;
        playerPos.X = 153;
        playerPos.Y = 162;
        shipPos.X = 210;
        shipPos.Y = 153;
        airshipPos.X = 221;
        airshipPos.Y = 237;
        airshipVisibility = 0;
        isCanalBlocked = true;
        isBridgeVisible = false;
        wasOpeningScenePlayed = false;

        InitParty( playerInfos );
        memset( Items, 0, sizeof Items );
        ClearGameFlags();

        Global::SetTime( 0 );
    }

    void LoadFile( FILE* file )
    {
        const int CharDataSize = sizeof( Character ) - sizeof( void* );

        for ( int i = 0; i < Players; i++ )
        {
            fread( Party[i].name, CharDataSize, 1, file );
        }

        uint32_t time = 0;

        fread( &time, sizeof time, 1, file );
        fread( &gil, sizeof gil, 1, file );
        fread( &vehicles, sizeof vehicles, 1, file );
        fread( &activeVehicle, sizeof activeVehicle, 1, file );
        fread( &shipPos, sizeof shipPos, 1, file );
        fread( &airshipPos, sizeof airshipPos, 1, file );
        fread( &playerPos, sizeof playerPos, 1, file );
        fread( &airshipVisibility, sizeof airshipVisibility, 1, file );
        fread( &isCanalBlocked, sizeof isCanalBlocked, 1, file );
        fread( &isBridgeVisible, sizeof isBridgeVisible, 1, file );
        fread( &wasOpeningScenePlayed, sizeof wasOpeningScenePlayed, 1, file );

        fread( Items, sizeof Items, 1, file );
        fread( gameFlags, sizeof gameFlags, 1, file );

        Global::SetTime( time );
    }

    void SaveFile( FILE* file )
    {
        const int CharDataSize = sizeof( Character ) - sizeof( void* );

        for ( int i = 0; i < Players; i++ )
        {
            fwrite( Party[i].name, CharDataSize, 1, file );
        }

        uint32_t time = Global::GetTime();

        fwrite( &time, sizeof time, 1, file );
        fwrite( &gil, sizeof gil, 1, file );
        fwrite( &vehicles, sizeof vehicles, 1, file );
        fwrite( &activeVehicle, sizeof activeVehicle, 1, file );
        fwrite( &shipPos, sizeof shipPos, 1, file );
        fwrite( &airshipPos, sizeof airshipPos, 1, file );
        fwrite( &playerPos, sizeof playerPos, 1, file );
        fwrite( &airshipVisibility, sizeof airshipVisibility, 1, file );
        fwrite( &isCanalBlocked, sizeof isCanalBlocked, 1, file );
        fwrite( &isBridgeVisible, sizeof isBridgeVisible, 1, file );
        fwrite( &wasOpeningScenePlayed, sizeof wasOpeningScenePlayed, 1, file );

        fwrite( Items, sizeof Items, 1, file );
        fwrite( gameFlags, sizeof gameFlags, 1, file );
    }

    const char* GetMagicName( int level, int id )
    {
        int strIndex = MagicNamesBase + level * 8 + (id - 1);
        return itemNames + itemNameOffsets[strIndex];
    }

    const char* GetSpecialName( int spellIndex )
    {
        return specialNames + specialNameOffsets[spellIndex];
    }

    const char* GetItemName( int itemId )
    {
        return itemNames + itemNameOffsets[itemId];
    }

    const char* GetClassName( int classId )
    {
        return itemNames + itemNameOffsets[ClassNamesBase + classId];
    }

    bool CanLearnSpell( int itemId, int classId )
    {
        int s = itemId - MagicNamesBase;
        int level = s / 8;
        int s2 = s % 8;

        return (magicPerms[classId][level] & (0x80u >> s2)) == 0;
    }

    void GetSpellId( int itemId, int& level, int& sil )
    {
        int s = itemId - MagicNamesBase;

        level = s / 8;
        sil = (s % 8) + 1;
    }

    bool CanEquipWeapon( int itemId, int classId )
    {
        uint32_t index = itemId - WeaponsBaseId;

        return (weaponPerms[index] & ClassEquipBit[classId]) == 0;
    }

    bool CanEquipArmor( int itemId, int classId )
    {
        uint32_t index = itemId - ArmorBaseId;

        return (armorPerms[index] & ClassEquipBit[classId]) == 0;
    }

    ArmorSlot GetArmorType( int itemId )
    {
        uint32_t index = itemId - ArmorBaseId;

        return (ArmorSlot) armorTypes[index];
    }

    bool IsPlayerAlive( int index )
    {
        Character& player = Party[index];

        return player.hp > 0
            && (player.status & Status_Death) == 0
            && (player.status & Status_Stone) == 0;
    }

    bool IsPlayerActive( int index )
    {
        Character& player = Party[index];

        return player.hp > 0
            && (player.status & Status_Death) == 0
            && (player.status & Status_Stone) == 0
            && (player.status & Status_Paralysis) == 0
            && (player.status & Status_Sleep) == 0;
    }

    void SwitchPlayers( int id1, int id2 )
    {
        Character& player1 = Party[id1];
        Character& player2 = Party[id2];
        Character temp;

        memcpy( &temp, &player1, sizeof temp );
        memcpy( &player1, &player2, sizeof temp );
        memcpy( &player2, &temp, sizeof temp );
    }

    void FullyRevive( int playerId )
    {
        Player::Character& player = Player::Party[playerId];

        player.SetHp( player.GetMaxHp() );
        player.SetStatus( 0 );

        for ( int i = 0; i < 8; i++ )
        {
            player.spellCharge[i] = player.spellMaxCharge[i];
        }
    }

    bool DealPoisonDamage()
    {
        bool hasPoison = false;

        for ( int i = 0; i < Players; i++ )
        {
            Player::Character& player = Player::Party[i];

            if ( (player.status & Status_Poison) != 0 
                && player.hp > 1 )
            {
                player.hp--;
            }

            if ( (player.status & Status_Poison) != 0 )
                hasPoison = true;
        }

        return hasPoison;
    }

    int GetG()
    {
        return gil;
    }

    void SubG( int value )
    {
        if ( value <= gil )
        {
            gil -= value;
        }
    }

    void AddG( int value )
    {
        if ( value > 0 )
        {
            gil += value;

            if ( gil > MaxG || gil < 0 )
                gil = MaxG;
        }
    }

    int GetLivingPlayerCount()
    {
        int count = 0;

        for ( int i = 0; i < PartySize; i++ )
        {
            if ( IsPlayerAlive( i ) )
                count++;
        }

        return count;
    }

    void AddXP( int value )
    {
        int livingPlayers = GetLivingPlayerCount();

        if ( livingPlayers == 0 )
            return;

        // ignore any left over amount from int division
        int xp = value / livingPlayers;

        if ( xp == 0 )
            xp = 1;

        for ( int i = 0; i < PartySize; i++ )
        {
            if ( IsPlayerAlive( i ) )
            {
                Character& character = Party[i];

                character.xp += xp;

                if ( character.xp > MaxXP || character.xp < 0 )
                    character.xp = MaxXP;
            }
        }
    }

    // Index by player class
    const uint8_t HitRateRaises[] = 
    {
        3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1
    };

    // Index by player class
    const uint8_t MagicAbsorbRaises[] = 
    {
        3, 2, 4, 2, 2, 2, 3, 2, 1, 2, 2, 2
    };

    const uint8_t MaxSpellCharge[12] = 
    {
        0,
        0,
        0,
        9,
        9,
        9,

        4,
        4,
        0,
        9,
        9,
        9,
    };

    int BasicStatIdToBit( int statId )
    {
        return 1 << (4 - statId);
    }

    bool CalcPlayerStatRaise( int playerId, int statId )
    {
        int _class = Party[playerId]._class;
        int level = Party[playerId].level;
        int allBoosts = levelUpAttrBoosts[_class][level - 1];
        int guaranteedBoost = allBoosts & BasicStatIdToBit( statId );
        bool increase = false;

        if ( guaranteedBoost )
        {
            increase = true;
        }
        else
        {
            int r = GetNextRandom( 4 );
            increase = (r == 0);
        }

        if ( increase )
        {
            if ( Party[playerId].basicStats[statId] < 100 )
                Party[playerId].basicStats[statId]++;
            return true;
        }

        return false;
    }

    void CalcDerivedStats( int playerId )
    {
        Character& character = Party[playerId];

        // Evade goes up 1 every time Agility increases. The 48 comes from:
        // (initial evade) - (initial agility)

        character.evadeRate = character.basicStats[Stat_Agility] + 48;
        if ( character.evadeRate > 200 )
            character.evadeRate = 200;

        character.hitRate = classInit[character._class].HitRate;
        character.hitRate += (character.level - 1) * HitRateRaises[character._class];
        if ( character.hitRate > 200 )
            character.hitRate = 200;

        character.magicAbsorb = classInit[character._class].MagicAbsorb;
        character.magicAbsorb += (character.level - 1) * MagicAbsorbRaises[character._class];
        if ( character.magicAbsorb > 200 )
            character.magicAbsorb = 200;

        character.elemStrength = 0;
        character.absorb = 0;

        for ( int i = 0; i < Armor_Max; i++ )
        {
            int armorId = character.armorIds[i];
            if ( armorId != NoArmor )
            {
                character.absorb += armorAttrs[armorId].Absorb;
                character.evadeRate -= armorAttrs[armorId].EvadeRate;
                character.elemStrength |= armorAttrs[armorId].Element;
            }
        }

        if ( (character._class == Class_BlackBelt || character._class == Class_Master)
            && character.absorb == 0 )
        {
            character.absorb = character.level;
        }

        if ( character.evadeRate < 0 )
            character.evadeRate = 0;

        if ( (character._class == Class_BlackBelt || character._class == Class_Master)
            && character.weaponId == NoWeapon )
        {
            character.damage = character.level * 2;
        }
        else
        {
            // The original game adds the carry flag to BB/MA's strength in the
            // damage calculation when he has no weapon, maybe unintentionally.
            // So, odd strength => +1, even strength => +0. Keep it simple here.

            // Except that, because the original game increases damage when strength
            // becomes even, and Black Mage starts with Strength=1 and Damage=1;
            // this simple calculation would need to be capped at 1.

            // Or, add 2 to give Black Mage the same small boost as in the original.

            int strength = character.basicStats[Stat_Strength];

            if ( classInit[character._class].BasicStats[Stat_Strength] == 1 )
                strength += 2;

            character.damage = strength / 2;

            if ( character.weaponId != NoWeapon )
            {
                character.damage += weaponAttrs[character.weaponId].Damage;
                character.hitRate += weaponAttrs[character.weaponId].HitRate;
            }
        }

        character.critHitRate = 0;

        if ( (character._class == Class_BlackBelt || character._class == Class_Master)
            && character.weaponId == NoWeapon )
        {
            character.critHitRate = character.level * 2;
        }
        else
        {
            if ( character.weaponId != NoWeapon )
                character.critHitRate = weaponAttrs[character.weaponId].CritHitRate;
            // else, stay 0
        }

        character.maxHits = character.hitRate / 32 + 1;

        if ( (character._class == Class_BlackBelt || character._class == Class_Master)
            && character.weaponId == NoWeapon )
        {
            character.maxHits *= 2;
        }
    }

    void CalcOneLevelUpBase( int playerId )
    {
        Character& character = Party[playerId];

        character.level++;

        int hpRaise = character.basicStats[Stat_Vitality] / 4 + 1;

        if ( levelUpAttrBoosts[character._class][character.level - 1] & LevelUpAttr_Strong )
        {
            int r = GetNextRandom( 6 );
            hpRaise += 20 + r;
        }

        character.maxHp += hpRaise;

        // The original game caps the maximum HP at 999.

        for ( int j = 0; j < Stat_Max; j++ )
        {
            CalcPlayerStatRaise( playerId, j );
        }

        SpellCharges& charges = spellChargeBoosts[character._class];
        int chargeBoostBits = charges[character.level - 1];

        for ( int i = 0; i < 8; i++ )
        {
            int boost = (chargeBoostBits >> i) & 1;

            if ( character.spellMaxCharge[i] < MaxSpellCharge[character._class] )
                character.spellMaxCharge[i] += boost;
        }
    }

    bool CalcLevelUp( int playerId )
    {
        Character& character = Party[playerId];
        int levelsUp = 0;
        int level = character.level;

        while ( level <= _countof( levelXp ) && character.xp >= levelXp[level-1] )
            level++;

        levelsUp = level - character.level;

        for ( int i = 0; i < levelsUp; i++ )
        {
            CalcOneLevelUpBase( playerId );
        }

        if ( levelsUp > 0 )
            CalcDerivedStats( playerId );

        return levelsUp > 0;
    }

    // Gets the spell ID for an item.

    int GetSpellForItem( int itemId )
    {
        if ( itemId < UsableItemsBaseId )
        {
            return 0;
        }
        else if ( itemId < EquippableItemsBaseId )
        {
            // TODO: get this from a data file
            int itemSpells[] = 
            {
                1,  // HEAL Potion is CURE Spell
                25, // PURE
                41, // SOFT
            };
            return itemSpells[itemId - UsableItemsBaseId];
        }
        else if ( itemId < ArmorBaseId )
        {
            WeaponAttr& weaponAttr = weaponAttrs[itemId - WeaponsBaseId];

            return weaponAttr.MagicId;
        }
        else if ( itemId < ArmorBaseId + 40 )
        {
            ArmorAttr& armorAttr = armorAttrs[itemId - ArmorBaseId];

            return armorAttr.MagicId;
        }

        return 0;
    }

    void SpendItem( int itemId )
    {
        if ( itemId < EquippableItemsBaseId )
            Items[itemId]--;
    }

    void ReturnItem( int itemId )
    {
        if ( itemId < EquippableItemsBaseId )
            Items[itemId]++;
    }

    Vehicle GetVehicles()
    {
        return vehicles;
    }

    void SetVehicles( Vehicle value )
    {
        vehicles = value;
    }

    Vehicle GetActiveVehicle()
    {
        return activeVehicle;
    }

    void SetActiveVehicle( Vehicle value )
    {
        activeVehicle = value;
    }

    Point GetShipRowCol()
    {
        return shipPos;
    }

    void SetShipRowCol( const Point& position )
    {
        shipPos = position;
    }

    Point GetAirshipRowCol()
    {
        return airshipPos;
    }

    void SetAirshipRowCol( const Point& position )
    {
        airshipPos = position;
    }

    Point GetPlayerPos()
    {
        return playerPos;
    }

    void SetPlayerPos( const Point& position )
    {
        playerPos = position;
    }

    int GetAirshipVisibility()
    {
        return airshipVisibility;
    }

    void SetAirshipVisibility( int visibility )
    {
        airshipVisibility = visibility;
    }

    bool IsCanalBlocked()
    {
        return isCanalBlocked;
    }

    void SetCanalBlocked( bool value )
    {
        isCanalBlocked = value;
    }

    bool IsBridgeVisible()
    {
        return isBridgeVisible;
    }

    void SetBridgeVisible( bool value )
    {
        isBridgeVisible = value;
    }

    bool WasOpeningScenePlayed()
    {
        return wasOpeningScenePlayed;
    }

    void SetOpeningScenePlayed( bool value )
    {
        wasOpeningScenePlayed = value;
    }

    void ClearGameFlags()
    {
        LoadList( "initFlags.dat", gameFlags, GameFlagCount );
    }

    bool GetEvent( int index )
    {
        return (gameFlags[index] & Game_Event) != 0;
    }

    void SetEvent( int index, bool value )
    {
        if ( value )
            gameFlags[index] |= Game_Event;
        else
            gameFlags[index] &= ~Game_Event;
    }

    bool GetObjVisible( int index )
    {
        return (gameFlags[index] & Game_ObjVisible) != 0;
    }

    void SetObjVisible( int index, bool value )
    {
        if ( value )
            gameFlags[index] |= Game_ObjVisible;
        else
            gameFlags[index] &= ~Game_ObjVisible;
    }

    bool GetChestOpened( int index )
    {
        return (gameFlags[index] & Game_ChestOpened) != 0;
    }

    void SetChestOpened( int index, bool value )
    {
        if ( value )
            gameFlags[index] |= Game_ChestOpened;
        else
            gameFlags[index] &= ~Game_ChestOpened;
    }


    int Character::GetHp()
    {
        return hp;
    }

    void Character::SetHp( int value )
    {
        hp = value;
    }

    int Character::GetStatus()
    {
        return status;
    }

    void Character::SetStatus( int value )
    {
        status = value;
    }

    int Character::GetHitMultiplier()
    {
        return hitMultiplier;
    }

    void Character::SetHitMultiplier( int value )
    {
        hitMultiplier = value;
    }

    int Character::GetMorale()
    {
        return 0;
    }

    void Character::SetMorale( int value )
    {
    }

    int Character::GetAbsorb()
    {
        return absorb;
    }

    void Character::SetAbsorb( int value )
    {
        absorb = value;
    }

    int Character::GetEvadeRate()
    {
        return evadeRate;
    }

    void Character::SetEvadeRate( int value )
    {
        evadeRate = value;
    }

    int Character::GetElementStrength()
    {
        return elemStrength;
    }

    void Character::SetElementStrength( int value )
    {
        elemStrength = value;
    }

    int Character::GetDamage()
    {
        return damage;
    }

    void Character::SetDamage( int value )
    {
        damage = value;
    }

    int Character::GetHitRate()
    {
        return hitRate;
    }

    void Character::SetHitRate( int value )
    {
        hitRate = value;

        maxHits = hitRate / 32 + 1;

        if ( (_class == Class_BlackBelt || _class == Class_Master)
            && weaponId == NoWeapon )
        {
            maxHits *= 2;
        }
    }


    int Character::GetElementWeakness()
    {
        return 0;
    }

    int Character::GetMagicAbsorb()
    {
        return magicAbsorb;
    }

    int Character::GetEnemyClasses()
    {
        return 0;
    }

    int Character::GetMaxHp()
    {
        return maxHp;
    }


    int Character::GetCritHitRate()
    {
        return critHitRate;
    }

    int Character::GetMaxHits()
    {
        return maxHits;
    }

    int Character::GetTargetClasses()
    {
        if ( weaponId == NoWeapon )
            return 0;

        return weaponAttrs[weaponId].Classes;
    }

    int Character::GetTargetElement()
    {
        if ( weaponId == NoWeapon )
            return 0;

        return weaponAttrs[weaponId].Element;
    }

    int Character::GetTargetStatus()
    {
        return 0;
    }
}
