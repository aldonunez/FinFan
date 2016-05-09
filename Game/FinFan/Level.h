/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Module.h"
#include "Dialog.h"
#include "ObjEvents.h"

class MapSprite;
class IMapSprite;


class Level : public IModule, public IPlayfield
{
    enum InOut
    {
        Out,
        In,
        InLocked,
    };

    enum ObjFlags
    {
        Obj_Flyer   = 0x20,
        Obj_Stand   = 0x40,
        Obj_InRoom  = 0x80
    };

    typedef void (Level::*UpdateFunc)();

    struct ObjectSpec
    {
        uint8_t Type;
        uint8_t Col;
        uint8_t Row;
        uint8_t Flags;
    };

    struct Object
    {
        uint8_t Type;
        uint8_t OrigType;
        uint8_t Flags;
        uint8_t MoveTimer;
        uint8_t Col;
        uint8_t Row;
        uint8_t LastCol;
        uint8_t LastRow;
        float   OffsetX;
        float   OffsetY;
        float   SpeedX;
        float   SpeedY;
    };

    static const int TileTypes = 128;
    static const int RowCount = 64;
    static const int ColCount = 64;
    static const int TileWidth = 16;
    static const int TileHeight = 16;
    static const int VisibleRows = 16;
    static const int VisibleCols = 17;
    static const int MiddleRow = 7;
    static const int MiddleCol = 7;
    static const int WorldWidth = ColCount * TileWidth;
    static const int WorldHeight = RowCount * TileHeight;
    static const int MapCount = 64;
    static const int TileSets = 8;
    static const int SwapTeleports = 64;
    static const int ExitTeleports = 16;
    static const int FirstLevelDomain = 64;
    static const int Objects = 16;
    static const int DialogMessages = 256;
    static const int Chests = 256;
    static const int NoObject = 0xff;

    static Level* instance;

    // Maps must be uncompressed to get the tile references. 
    // Uncompress the whole current map into a buffer.

    uint8_t tileRefs[RowCount][ColCount];

    // The tile graphics are laid out as a grid of 16 tiles in each row
    // Going left to right, top to bottom, the tiles are in the tile reference order
    // Two sets of tiles are used: outside and inside rooms

    ALLEGRO_BITMAP* tiles[2];

    ALLEGRO_BITMAP* objectsImage;
    ALLEGRO_BITMAP* playerImage;

    uint16_t tileAttr[TileTypes];

    LTeleport swapTeleports[SwapTeleports];
    OWTeleport exitTeleports[ExitTeleports];

    Table<char, DialogMessages> messages;
    CheckParams checkParams[ObjectTypes];
    uint8_t chests[Chests];

    int mapId;
    InOut inRoom;
    int battleRate;
    int backdrop;
    int song;

    // Keep track of the alignment of the visible region relative to the screen

    int8_t  offsetX;
    int8_t  offsetY;
    uint8_t topRow;
    uint8_t leftCol;

    MapSprite* playerSprite;
    IMapSprite* objectSprites[Objects];
    Object objects[Objects];
    int nextObjIndex;

    UpdateFunc curUpdate;
    Direction movingDir;
    Direction facingDir;
    uint8_t playerCol;
    uint8_t playerRow;

    Dialog dialog;
    int talkingObjIndex;
    Direction talkingObjOrigDir;

    bool fightPending;
    int formationId;

    bool teleportPending;
    int teleportType;
    int teleportId;

    bool shopPending;
    int shopId;
    int origShopDoor;

public:
    Level();
    ~Level();

    void Init( int mapId, int startCol, int startRow, int inRoom );

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    virtual Point GetCurrentPos();
    virtual int GetInRoom();

    virtual void HandleShopClosed();
    virtual void HandleFightEnded();
    virtual void HandleOpeningEnded();

    static bool IsActive();
    static uint16_t GetCurrentTileAttr();

private:
    void DrawMap();
    void DrawPlayer();
    void DrawObjects();

    void UpdateFootIdle();
    void UpdateMoving();
    void UpdateDialog();

    void UpdateMoveObjects();
    void UpdateObject();
    void UpdateObjectSprites();

    void MakeObjects( const ObjectSpec* objSpecs, int count );
    bool IsVisible( int objIndex );
    bool CanObjectMove( int index, int col, int row );
    bool CanPlayerMove( int col, int row );
    bool CanPlayerTalk( int col, int row, int& objIndex );
    void CheckObject( int type, CheckResult& result );
    void RefreshVisibleObjects();
    void MakeObjectSprite( int index );
    void CheckTile( int col, int row, CheckResult& result );
    void OpenChest( int chestId, int col, int row, CheckResult& result );
    bool CanWalk( int col, int row );
    bool CanWalkSpecial( int col, int row );

    void OpenDoor( int col, int row );
    bool TryUnlockDoor( int col, int row );
    void CloseDoor( int col, int row );
    void WalkBattle( uint16_t attrs );
    bool WalkIfItem( int itemId );
    bool WalkIf4Orbs();
    void TakeOrb( int itemId );

    void ShiftMap( int shiftX, int shiftY );

    Point GetPlayerRowCol();
    Point GetFacingRowCol( Direction direction );

    int GetTileRef( int col, int row );
    void SetTileRef( int col, int row, int tileRef );

    int GetBattleFormation();
    void DealMoveDamage( int col, int row );

    bool CheckPendingAction();

    void ChangeTiles();
};
