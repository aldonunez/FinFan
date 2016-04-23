/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Module.h"


struct Point;
class Sprite;


namespace Battle
{
    struct Formation
    {
        uint8_t Type;
        uint8_t Pattern;
        uint8_t Ids[4];
        uint8_t MinMax[6];
        uint8_t SurpriseRate;
        uint8_t Flags;
        uint8_t Song;
        uint8_t Reserved;
    };

    enum FormationFlags
    {
        Formation_None,
        Formation_CantRun = 1,
    };

    struct EnemyAttr
    {
        uint16_t Xp;
        uint16_t G;
        uint16_t Hp;
        uint8_t  Morale;
        uint8_t  AttackListId;
        uint8_t  Evade;
        uint8_t  Defense;
        uint8_t  Hits;
        uint8_t  HitRate;
        uint8_t  Damage;
        uint8_t  CritHitRate;
        uint8_t  AttackElement;
        uint8_t  Status;
        uint8_t  Classes;
        uint8_t  MagicDefense;
        uint8_t  ElementWeakness;
        uint8_t  ElementStrength;
    };

    struct AttackList
    {
        uint8_t MagicRate;
        uint8_t SpecialRate;
        uint8_t MagicIds[9];
        uint8_t SpecialIds[5];
    };

    struct TypeCounter
    {
        int Type;
        int Count;
    };

    struct Enemy : public Actor
    {
        int             Type;
        TypeCounter*    Counter;
        Bounds          Bounds;
        int             Hp;
        int             Status;
        int             HitRate;
        int             Morale;
        int             Absorb;
        int             ElemStrength;
        int             HitMultiplier;
        int             EvadeRate;
        int             Damage;
        int             NextSpellIndex;
        int             NextSpecialIndex;
        ALLEGRO_COLOR   Color;
        Enemy*          Prev;
        Enemy*          Next;

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

    enum Action
    {
        Action_None,
        Action_Fight,
        Action_Magic,
        Action_Item,
        Action_Run,
        Action_Special,
    };

    enum Target
    {
        Target_None,
        Target_AllEnemies = 1,
        Target_OneEnemy = 2,
        Target_Self = 4,
        Target_AllPlayers = 8,
        Target_OnePlayer = 16,
    };

    struct Command
    {
        // actionId: 
        //  spell index for magic
        //  item ID for item

        Action  action;
        int     actionId;
        Party   actorParty;
        int     actorIndex;
        Target  target;
        int     targetIndex;
    };

    enum MenuAction
    {
        Menu_None,
        Menu_Push,
        Menu_Pop,
        Menu_PopAll,
    };

    class Menu
    {
    public:
        Menu* prevMenu;

        virtual MenuAction Update( Menu*& nextMenu ) = 0;
        virtual void Draw() = 0;
        virtual void DrawCursor() = 0;
    };

    const int EnemyMapCols = 3;
    const int EnemyMapRows = 3;

    struct EnemyMap
    {
        int8_t Indexes[EnemyMapRows][EnemyMapCols];
    };

    enum EncounterType
    {
        Encounter_Normal,
        Encounter_PlayerFirst,
        Encounter_EnemyFirst,
    };


    const int BackdropSegWidth = 112;
    const int BackdropSegHeight = 32;
    const int ArenaTop = BackdropSegHeight;

    const int PlayerSpriteRowHeight = 3 * 8;
    const int PlayerSpriteWidth = 2 * 8;
    const int PlayerSpriteHeight = 3 * 8;

    const int MaxEnemies = 9;
    const int InvalidEnemyType = -1;

    const int PartyX = 208;
    const int PartyY = ArenaTop;

    const int AllIndex = -2;
    const int NoneIndex = -1;


    const Formation& GetFormation();
    Enemy* GetEnemies();
    const EnemyMap* GetEnemyMap();
    EncounterType GetEncounterType();
    void SetEncounterType( EncounterType value );

    Command& GetCommandBuilder();
    void AddCommand( const Command& cmd );


    Bounds standFrames[];
    Bounds walkFrames[];
    Bounds strikeFrames[];
    Bounds castFrames[];
    Bounds fanfareFrames[];
    Bounds weakFrames[];
    Bounds deadFrames[];

    Sprite* playerSprites[];


    class Effect
    {
    public:
        virtual void Update() = 0;
        virtual void Draw() = 0;
    };

    const int MaxActors = MaxEnemies + Players;

    void UpdateAllIdleSprites();
    void UpdateIdleSprite( int playerId );
    void LeaveBattle();

    extern EnemyAttr       enemyAttrs[128];
    extern AttackList      attackLists[44];
    extern ALLEGRO_BITMAP* battleSprites;

    extern Bounds16 weaponFrames[2];
    extern Bounds16 punchFrames[2];
    extern Bounds16 spellFrames[2];

    extern Enemy       enemies[9];
    extern int         enemyCount;

    extern Menu*       activeMenu;
    extern char gMessage[256];
    extern bool gShowFullScreenColor;
    extern bool gShowBackgroundColor;
    extern ALLEGRO_COLOR gFullScreenColor;
    extern bool gShowWeapon;
    extern Sprite* weaponSprite;
    extern Effect* magicEffects[9];
    extern Effect* curEffect;
}
