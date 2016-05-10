/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


namespace Player
{
    const int PartySize = 4;
    const int MaxItems = 99;
    const int ItemTypes = 108;
    const int PotionItemsBaseId = 22;
    const int UsableItemsBaseId = 25;
    const int UsableItems = 3;
    const int EquippableItemsBaseId = 28;
    const int WeaponsBaseId = 28;
    const int ArmorBaseId = 68;
    const int MoneyBaseId = 108;
    const int FileSize = 965;


    enum BasicStat
    {
        Stat_Strength,
        Stat_Agility,
        Stat_Intelligence,
        Stat_Vitality,
        Stat_Luck,
        Stat_Max,
    };

    enum ArmorSlot
    {
        Armor_Body,
        Armor_Shield,
        Armor_Head,
        Armor_Hand,
        Armor_Max,
    };

    struct PlayerInfo
    {
        int Class;
        char Name[NameLength + 1];
    };

    struct Character : public Actor
    {
        // constant
        char    name[7];

        // increase throughout game
        int     _class;
        int     level;
        int     xp;
        int     maxHp;

        // current basics
        int     hp;
        int     basicStats[Stat_Max];
        int     status;

        // current derived
        int     damage;
        int     hitRate;
        int     absorb;
        int     evadeRate;
        int     magicAbsorb;
        int     elemStrength;

        // battle
        int     maxHits;
        int     hitMultiplier;
        int     critHitRate;

        // items
        int     weaponId;
        uint8_t armorIds[Armor_Max];

        // magic
        uint8_t spells[8][4];
        uint8_t spellCharge[8];
        uint8_t spellMaxCharge[8];

        virtual int  GetHp();
        virtual void SetHp( int value );
        virtual int  GetStatus();
        virtual void SetStatus( int value );
        virtual int  GetHitMultiplier();
        virtual void SetHitMultiplier( int value );
        virtual int  GetMorale();
        virtual void SetMorale( int value );
        virtual int  GetAbsorb();
        virtual void SetAbsorb( int value );
        virtual int  GetEvadeRate();
        virtual void SetEvadeRate( int value );
        virtual int  GetElementStrength();
        virtual void SetElementStrength( int value );
        virtual int  GetDamage();
        virtual void SetDamage( int value );
        virtual int  GetHitRate();
        virtual void SetHitRate( int value );

        virtual int GetElementWeakness();
        virtual int GetMagicAbsorb();
        virtual int GetEnemyClasses();
        virtual int GetMaxHp();

        virtual int GetCritHitRate();
        virtual int GetMaxHits();
        virtual int GetTargetClasses();
        virtual int GetTargetElement();
        virtual int GetTargetStatus();
    };

    struct WeaponAttr
    {
        uint8_t HitRate;
        uint8_t Damage;
        uint8_t CritHitRate;
        uint8_t MagicId;
        uint8_t Element;
        uint8_t Classes;
        uint8_t Reserved[2];
    };

    struct ArmorAttr
    {
        uint8_t EvadeRate;
        uint8_t Absorb;
        uint8_t Element;
        uint8_t MagicId;
    };

    struct MagicAttr
    {
        uint8_t HitRate;
        uint8_t PowerStatus;
        uint8_t Element;
        uint8_t Target;
        uint8_t Effect;
        uint8_t NesTile;
        uint8_t NesColor;
        uint8_t Reserved;
    };


    Character Party[];
    uint8_t Items[];
    WeaponAttr weaponAttrs[];
    ArmorAttr armorAttrs[];
    MagicAttr magicAttrs[];
    MagicAttr specialAttrs[];
    int levelXp[];


    bool Init();

    void InitFile( const PlayerInfo* playerInfos );
    void LoadFile( FILE* file );
    void SaveFile( FILE* file );

    bool IsPlayerAlive( int index );
    bool IsPlayerActive( int index );
    bool CalcLevelUp( int playerId );
    void CalcDerivedStats( int playerId );
    void SwitchPlayers( int id1, int id2 );
    void FullyRevive( int playerId );
    bool DealPoisonDamage();

    int GetG();
    void SubG( int value );
    void AddG( int value );
    void AddXP( int value );

    const char* GetMagicName( int level, int id );
    const char* GetSpecialName( int spellIndex );
    const char* GetItemName( int itemId );
    const char* GetClassName( int classId );

    bool CanLearnSpell( int itemId, int classId );
    void GetSpellId( int itemId, int& level, int& sil );

    bool CanEquipWeapon( int itemId, int classId );
    bool CanEquipArmor( int itemId, int classId );
    ArmorSlot GetArmorType( int itemId );

    int GetSpellForItem( int itemId );
    void SpendItem( int itemId );
    void ReturnItem( int itemId );

    Vehicle GetVehicles();
    void SetVehicles( Vehicle value );

    Vehicle GetActiveVehicle();
    void SetActiveVehicle( Vehicle value );

    Point GetShipRowCol();
    void SetShipRowCol( const Point& position );

    Point GetAirshipRowCol();
    void SetAirshipRowCol( const Point& position );

    Point GetPlayerPos();
    void SetPlayerPos( const Point& position );

    int GetAirshipVisibility();
    void SetAirshipVisibility( int visibility );

    bool IsCanalBlocked();
    void SetCanalBlocked( bool value );

    bool IsBridgeVisible();
    void SetBridgeVisible( bool value );

    bool WasOpeningScenePlayed();
    void SetOpeningScenePlayed( bool value );

    void ClearGameFlags();

    bool GetEvent( int index );
    void SetEvent( int index, bool value );

    bool GetObjVisible( int index );
    void SetObjVisible( int index, bool value );

    bool GetChestOpened( int index );
    void SetChestOpened( int index, bool value );
}
