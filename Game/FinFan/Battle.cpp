/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Battle.h"
#include "BattleCalc.h"
#include "BattleMod.h"
#include "BattleStates.h"
#include "Player.h"
#include "Text.h"
#include "Sprite.h"
#include "SceneStack.h"
#include "Sound.h"


namespace Battle
{
    void Init( int formationId, int backdropId );
    void Uninit();
    void Update();
    void Draw();
    void MakeEnemies();
    void MakeOneSizeEnemies( const ::Point* places, int maxPlaces, int placeWidth );
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



enum FormationType
{
    FormType_Small,
    FormType_Big,
    FormType_Mixed,
    FormType_Boss,
    FormType_BigBoss,
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

TypeCounter typeCounts[4];
Enemy       enemies[9];
Enemy       enemiesHead;
int         enemyCount;

Menu*       activeMenu;

Sprite* playerSprites[Player::PartySize];

int gFormationId;
int gBackdropId;
int gSetId;

EncounterType gEncounter;
char gMessage[256];


bool gShowWeapon;
Sprite* weaponSprite;


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
    int pattern = GetFormation().Pattern;
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




    MakeEnemies();

    Input::ResetRepeat();

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        playerSprites[i] = new Sprite( playerImages );
        playerSprites[i]->SetX( PartyX );
        playerSprites[i]->SetY( PartyY + i * PlayerSpriteRowHeight );
        UpdateIdleSprite( i );
    }

    weaponSprite = new Sprite( battleSprites );

    gEncounter = GetNextEncounterType();

    GotoFirstState();
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

void UpdateAllIdleSprites()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        UpdateIdleSprite( i );
    }
}

void Update()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        playerSprites[i]->Update();
    }

    UpdateState();
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

    for ( int i = 0; i < GetResultCount(); i++ )
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
    const Formation& formation = GetFormation();
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

    switch ( formation.Type )
    {
    case FormType_Big:
        MakeOneSizeEnemies( sBigEnemyPos, _countof( sBigEnemyPos ), BigEnemyWidth );
        break;

    case FormType_Small:
        MakeOneSizeEnemies( sSmallEnemyPos, _countof( sSmallEnemyPos ), SmallEnemyWidth );
        break;

    case FormType_Mixed:
        MakeOneSizeEnemies( sMixedEnemyPosBig, _countof( sMixedEnemyPosBig ), BigEnemyWidth );
        // the mixed enemy map depends on small enemies starting at index 3
        bigEnemyCount = enemyCount;
        enemyCount = 3;
        MakeOneSizeEnemies( sMixedEnemyPosSmall, _countof( sMixedEnemyPosSmall ), SmallEnemyWidth );
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
    const Formation& formation = GetFormation();
    int type = formation.Ids[0];
    Point place;

    place.X = EnemyLeft + (EnemyZoneWidth - enemySourcePos[type].Width) / 2;
    place.Y = BackdropSegHeight + (EnemyZoneHeight - enemySourcePos[type].Height) / 2;

    MakeOneEnemy( place, type );
}

void MakeOneSizeEnemies( const ::Point* places, int maxPlaces, int placeWidth )
{
    const Formation& formation = GetFormation();
    const uint8_t* set = nullptr;
    int setSize = 0;
    int count = 0;

    if ( gSetId == 0 )
    {
        set = &formation.MinMax[0];
        setSize = 4;
    }
    else
    {
        set = &formation.MinMax[4];
        setSize = 2;
    }

    for ( int i = 0; i < setSize; i++ )
    {
        if ( set[i] == 0 )
            continue;
        if ( enemySourcePos[formation.Ids[i]].Width != placeWidth )
            continue;

        int max = set[i] & 0xf;
        int min = set[i] >> 4;
        int range = max - min + 1;
        int n = min + GetNextRandom( range );

        for ( ; n > 0; n--, enemyCount++, count++ )
        {
            if ( count == maxPlaces || enemyCount == _countof( enemies ) )
                goto NoMoreEnemies;

            MakeOneEnemy( places[count], formation.Ids[i] );
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

EncounterType GetEncounterType()
{
    return gEncounter;
}

void SetEncounterType( EncounterType value )
{
    gEncounter = value;
}

const Formation& GetFormation()
{
    return formations[gFormationId];
}

Enemy* GetEnemies()
{
    return enemies;
}

const EnemyMap* GetEnemyMap()
{
    const Formation& formation = GetFormation();

    switch ( formation.Type )
    {
    case FormType_Big:      return &bigEnemyMap;
    case FormType_Small:    return &smallEnemyMap;
    case FormType_Mixed:    return &mixedEnemyMap;
    case FormType_Boss:
    case FormType_BigBoss:  return &bossEnemyMap;
    default:                return &smallEnemyMap;
    }
}

}
