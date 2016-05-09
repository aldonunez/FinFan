/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Level.h"
#include "MapSprite.h"
#include "FlyerSprite.h"
#include "LTile.h"
#include "ObjEvents.h"
#include "Player.h"
#include "Ids.h"
#include "SceneStack.h"
#include "Sound.h"
#include <allegro5\allegro_primitives.h>


enum
{
    Dialog_OpenChest    = 0xf0,
    Dialog_CantCarry    = 0xf1,
    Dialog_EmptyChest   = 0xf2,
};

enum
{
    Tile_UnlockedDoor   = 0x36,
    Tile_OpenDoor       = 0x37,
    Tile_LockedDoor     = 0x3b,

    // This tile didn't exist in the original. Now it's shown using the two open chest bitmaps 
    // that I made and the extractor writes. One is for the futuristic tileset of the Sky Castle.
    // The other is for all other levels.

    // This tile's attributes are left 0, which means that when you check an open chest, it says 
    // "Nothing here". If instead you wanted it to say "The treasure box is empty!" like in the 
    // original game, give it the chest attribute and a suitable chest ID.

    Tile_OpenChest      = 0x7f,
};


Level* Level::instance;


Level::Level()
    :   mapId( 0 ),
        inRoom( Out ),
        battleRate( 0 ),
        backdrop( 0 ),
        song( 0 ),
        offsetX( 0 ),
        offsetY( 0 ),
        leftCol( 0 ),
        topRow( 0 ),
        objectsImage( nullptr ),
        playerImage( nullptr ),
        playerSprite( nullptr ),
        nextObjIndex( 0 ),
        curUpdate( &Level::UpdateFootIdle ),
        movingDir( Dir_None ),
        facingDir( Dir_Down ),
        playerCol( 0 ),
        playerRow( 0 ),
        talkingObjIndex( NoObject ),
        talkingObjOrigDir( Dir_None ),
        fightPending( false ),
        formationId( 0 ),
        teleportPending( false ),
        teleportType( 0 ),
        teleportId( 0 ),
        shopPending( false ),
        shopId( 0 ),
        origShopDoor( 0 ),
        flashMove( false )
{
    instance = this;

    tiles[0] = nullptr;
    tiles[1] = nullptr;

    memset( objectSprites, 0, sizeof objectSprites );
    memset( objects, 0, sizeof objects );
}

Level::~Level()
{
    instance = nullptr;

    for ( int i = 0; i < _countof( tiles ); i++ )
    {
        al_destroy_bitmap( tiles[i] );
    }

    al_destroy_bitmap( objectsImage );
    al_destroy_bitmap( playerImage );

    delete playerSprite;

    for ( int i = 0; i < _countof( objectSprites ); i++ )
    {
        delete objectSprites[i];
    }
}

void Level::Init( int mapId, int startCol, int startRow, int inRoomState )
{
    const int AllTileAttrCount = TileSets * TileTypes;
    const int AllObjSpecCount = MapCount * Objects;

    Table<uint8_t, MapCount> compressedMaps;
    uint8_t imagesets[MapCount];
    uint8_t tilesets[MapCount];
    uint16_t allTileAttr[AllTileAttrCount];
    uint8_t battleRates[MapCount];
    uint8_t backdrops[MapCount];
    uint8_t levelSongs[MapCount];
    ObjectSpec allObjectSpecs[AllObjSpecCount];

    if ( !LoadResource( "levelMaps.tab", &compressedMaps ) )
        return;

    if ( !LoadList( "levelGraphicSets.dat", imagesets, MapCount ) )
        return;

    if ( !LoadList( "levelTilesets.dat", tilesets, MapCount ) )
        return;

    if ( !LoadList( "levelTileAttr.dat", allTileAttr, AllTileAttrCount ) )
        return;

    if ( !LoadList( "levelMusic.dat", levelSongs, MapCount ) )
        return;

    song = levelSongs[mapId];

    int tileset = tilesets[mapId];
    int tileAttrSize = TileTypes * sizeof tileAttr[0];

    memcpy( tileAttr, &allTileAttr[tileset * TileTypes], tileAttrSize );

    char filename[MAX_PATH] = "";

    sprintf_s( filename, "levelTilesOut%02x.png", imagesets[mapId] );
    tiles[Out] = al_load_bitmap( filename );
    if ( tiles[Out] == nullptr )
        return;

    sprintf_s( filename, "levelTilesIn%02x.png", imagesets[mapId] );
    tiles[In] = al_load_bitmap( filename );
    if ( tiles[In] == nullptr )
        return;

    objectsImage = al_load_bitmap( "mapObjects.png" );
    if ( objectsImage == nullptr )
        return;

    playerImage = al_load_bitmap( "mapPlayer.png" );
    if ( playerImage == nullptr )
        return;

    if ( !LoadList( "exitTeleports.dat", exitTeleports, ExitTeleports ) )
        return;

    if ( !LoadList( "swapTeleports.dat", swapTeleports, SwapTeleports ) )
        return;

    if ( !LoadList( "battleRates.dat", battleRates, MapCount ) )
        return;

    if ( !LoadList( "levelBackdrops.dat", backdrops, MapCount ) )
        return;

    if ( !LoadList( "objects.dat", allObjectSpecs, AllObjSpecCount ) )
        return;

    if ( !LoadResource( "dialogue.tab", &messages ) )
        return;

    if ( !LoadList( "talkParams.dat", checkParams, ObjectTypes ) )
        return;

    if ( !LoadList( "treasure.dat", chests, Chests ) )
        return;

    // skip the first entry
    battleRate = battleRates[mapId+1];
    backdrop = backdrops[mapId];

    DecompressMap( compressedMaps.GetItem( mapId ), (uint8_t*) tileRefs );
    ChangeTiles();

    playerCol = startCol;
    playerRow = startRow;

    leftCol = (playerCol - MiddleCol + ColCount) % ColCount;
    topRow = (playerRow - MiddleRow + RowCount) % RowCount;

    playerSprite = new MapSprite( playerImage );
    playerSprite->SetX( MiddleCol * TileWidth );
    playerSprite->SetY( MiddleRow * TileHeight );
    playerSprite->SetFrames( Player::Party[0]._class * 16 );
    playerSprite->SetDirection( facingDir );

    this->mapId = mapId;
    inRoom = (InOut) inRoomState;

    MakeObjects( &allObjectSpecs[mapId * Objects], Objects );

    Sound::PlayTrack( song, 0, true );

    SceneStack::BeginFade( 15, Color::Black(), Color::Transparent(), [] {} );
}

void Level::ChangeTiles()
{
    for ( int row = 0; row < RowCount; row++ )
    {
        for ( int col = 0; col < ColCount; col++ )
        {
            uint8_t ref = tileRefs[row][col];
            uint16_t attrs = tileAttr[ref];

            if ( LTile::GetSpecial( attrs ) == LTile::S_Treasure )
            {
                int chestId = LTile::GetChest( attrs );
                if ( Player::GetChestOpened( chestId ) )
                    tileRefs[row][col] = Tile_OpenChest;
            }
        }
    }
}

void Level::MakeObjects( const ObjectSpec* objSpecs, int count )
{
    for ( int i = 0; i < count; i++ )
    {
        const auto& spec = objSpecs[i];

        if ( spec.Type == 0 )
            break;

        auto& obj = objects[i];

        obj.OrigType = spec.Type;
        obj.Flags = spec.Flags;
        obj.Col = spec.Col;
        obj.Row = spec.Row;
        obj.LastCol = spec.Col;
        obj.LastRow = spec.Row;

        if ( Player::GetObjVisible( spec.Type ) )
        {
            obj.Type = spec.Type;

            MakeObjectSprite( i );
        }
    }
}

void Level::MakeObjectSprite( int index )
{
    auto& obj = objects[index];

    ICustomMapSprite* sprite = nullptr;
    int frameOffsetRow = obj.Type;

    if ( (obj.Flags & Obj_Flyer) != 0 )
        sprite = new FlyerSprite( objectsImage );
    else
        sprite = new MapSprite( objectsImage );

    sprite->SetFrames( frameOffsetRow * TileHeight );
    sprite->SetX( obj.Col * TileWidth );
    sprite->SetY( obj.Row * TileHeight );

    if ( (obj.Flags & Obj_Stand) != 0
        || (obj.Flags & Obj_Flyer) != 0 )
    {
        sprite->Start();
    }

    objectSprites[index] = sprite;
}

void Level::Update()
{
    if ( SceneStack::IsFading() )
        return;

    (this->*curUpdate)();

    playerSprite->Update();

    if ( dialog.IsClosed() )
    {
        UpdateMoveObjects();
        UpdateObject();
        UpdateObjectSprites();
    }
}

void Level::ShiftMap( int shiftX, int shiftY )
{
    if ( shiftX < 0 )
    {
        offsetX += shiftX;
        if ( offsetX < 0 )
        {
            offsetX += TileWidth;
            leftCol = (leftCol - 1 + ColCount) % ColCount;
        }
    }
    else if ( shiftX > 0 )
    {
        offsetX += shiftX;
        if ( offsetX >= TileWidth )
        {
            offsetX -= TileWidth;
            leftCol = (leftCol + 1) % ColCount;
        }
    }

    if ( shiftY < 0 )
    {
        offsetY += shiftY;
        if ( offsetY < 0 )
        {
            offsetY += TileHeight;
            topRow = (topRow - 1 + RowCount) % RowCount;
        }
    }
    else if ( shiftY > 0 )
    {
        offsetY += shiftY;
        if ( offsetY >= TileHeight )
        {
            offsetY -= TileHeight;
            topRow = (topRow + 1) % RowCount;
        }
    }
}

void Level::Draw()
{
    DrawMap();
    DrawPlayer();
    DrawObjects();

    if ( !dialog.IsClosed() )
        dialog.Draw();

    if ( flashMove )
    {
        int offset = offsetX + offsetY;
        bool flashNow = (offset & 1) == 1;
        if ( flashNow )
        {
            int op, src, dst;
            al_get_blender( &op, &src, &dst );
            al_set_blender( ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE );
            al_draw_filled_rectangle( 0, 0, StdViewWidth, StdViewHeight, al_map_rgb( 128, 128, 128 ) );
            al_set_blender( op, src, dst );
        }
    }
}

void Level::DrawMap()
{
    int tileSet = (inRoom == Out) ? 0 : 1;
    ALLEGRO_BITMAP* bmp = tiles[tileSet];

    al_hold_bitmap_drawing( true );

    for ( int i = 0; i < VisibleRows; i++ )
    {
        for ( int j = 0; j < VisibleCols; j++ )
        {
            int bufRow = (topRow + i) % RowCount;
            int bufCol = (leftCol + j) % ColCount;
            int tileRef = tileRefs[bufRow][bufCol];
            int srcX = (tileRef % 16) * TileWidth;
            int srcY = (tileRef / 16) * TileHeight;
            int destX = j * TileWidth - offsetX;
            int destY = i * TileHeight - offsetY;

            al_draw_bitmap_region( bmp, srcX, srcY, TileWidth, TileHeight, destX, destY, 0 );
        }
    }

    al_hold_bitmap_drawing( false );
}

void Level::DrawPlayer()
{
    playerSprite->DrawAt( MiddleCol * TileWidth, MiddleRow * TileHeight );
}

void Level::DrawObjects()
{
    const int WorldWidth = ColCount * TileWidth;
    const int WorldHeight = RowCount * TileHeight;

    int left = leftCol * TileWidth + offsetX;
    int top = topRow * TileHeight + offsetY;

    for ( int i = 0; i < Objects; i++ )
    {
        if ( objectSprites[i] != nullptr && IsVisible( i ) )
        {
            IMapSprite* sprite = objectSprites[i];

            // if we need to draw parts of sprites on the left edge of screen,
            // then we'll have to sign extend screenX and Y based on WorldWidth and Height

            int screenX = (sprite->GetX() - left + WorldWidth) % WorldWidth;
            int screenY = (sprite->GetY() - top + WorldHeight) % WorldHeight;

            sprite->DrawAt( screenX, screenY );
        }
    }
}

bool Level::IsVisible( int objIndex )
{
    const auto& obj = objects[objIndex];

    return ((obj.Flags & Obj_InRoom) != 0) == (inRoom == In);
}

Point Level::GetPlayerRowCol()
{
    Point pos = { playerCol, playerRow };

    return pos;
}

Point Level::GetFacingRowCol( Direction direction )
{
    int shiftCol = 0;
    int shiftRow = 0;

    switch ( direction )
    {
    case Dir_Right: shiftCol = 1; break;
    case Dir_Left:  shiftCol = -1; break;
    case Dir_Down:  shiftRow = 1; break;
    case Dir_Up:    shiftRow = -1; break;
    }

    Point pos = GetPlayerRowCol();
    pos.X = (uint8_t) (pos.X + shiftCol) % ColCount;
    pos.Y = (uint8_t) (pos.Y + shiftRow) % RowCount;

    return pos;
}

int Level::GetTileRef( int col, int row )
{
    return tileRefs[row][col];
}

void Level::SetTileRef( int col, int row, int tileRef )
{
    tileRefs[row][col] = tileRef;
}

void Level::OpenDoor( int col, int row )
{
    if ( inRoom == Out )
    {
        int tile = tileRefs[row][col];

        tileRefs[row][col] = Tile_OpenDoor;

        if ( tile == Tile_LockedDoor )
            inRoom = InLocked;
        else
            inRoom = In;

        Sound::PlayEffect( SEffect_Door );

        // check the original tile for a shop ID
        uint16_t attrs = tileAttr[tile];
        int shopId = LTile::GetShop( attrs );

        if ( LTile::GetSpecial( attrs ) == LTile::S_Locked )
            shopId = 0;

        if ( shopId != 0 )
        {
            shopPending = true;
            this->shopId = shopId;
            origShopDoor = tile;
        }
    }
}

bool Level::TryUnlockDoor( int col, int row )
{
    if ( Player::Items[Item_MysticKey] == 0 )
        return false;

    OpenDoor( col, row );
    return true;
}

void Level::CloseDoor( int col, int row )
{
    if ( inRoom != Out )
    {
        int tile = (inRoom == InLocked) ? Tile_LockedDoor : Tile_UnlockedDoor;
        int doorRow = (row - 1 + RowCount) % RowCount;

        tileRefs[doorRow][col] = tile;

        Sound::PlayEffect( SEffect_Door );

        inRoom = Out;
    }
}

void Level::WalkBattle( uint16_t attrs )
{
    int formation = LTile::GetFormation( attrs );

    if ( LTile::IsRandomBattle( formation ) )
    {
        int r = GetNextRandom( 256 );

        // no fight
        if ( r >= battleRate )
            return;

        formation = GetBattleFormation();
    }

    fightPending = true;
    formationId = formation;
}

bool Level::WalkIfItem( int itemId )
{
    if ( Player::Items[itemId] == 0 )
        return false;

    Sound::PushTrack( Sound_Fanfare, 0 );
    return true;
}

bool Level::WalkIf4Orbs()
{
    if ( Player::Items[Item_OrbAir] == 0
        || Player::Items[Item_OrbEarth] == 0
        || Player::Items[Item_OrbFire] == 0
        || Player::Items[Item_OrbWater] == 0 )
        return false;

    Sound::PushTrack( Sound_Fanfare, 0 );
    return true;
}

void Level::TakeOrb( int id )
{
    if ( Player::Items[id] == 0 )
    {
        Player::Items[id] = 1;
        // TODO: altar effect
    }
}

bool Level::CanWalkSpecial( int col, int row )
{
    uint8_t ref = GetTileRef( col, row );
    uint16_t attrs = tileAttr[ref];
    int tileSpecial = LTile::GetSpecial( attrs );

    switch ( tileSpecial )
    {
    case LTile::S_OpenDoor:
        OpenDoor( col, row );
        return true;

    case LTile::S_Locked:
        return TryUnlockDoor( col, row );

    case LTile::S_CloseDoor:
        CloseDoor( col, row );
        return true;

    case LTile::S_Treasure:
        return false;

    case LTile::S_Battle:
        WalkBattle( attrs );
        return true;

    case LTile::S_Crown:
        return WalkIfItem( Item_Crown );

    case LTile::S_Cube:
        return WalkIfItem( Item_Cube );

    case LTile::S_4Orbs:
        return WalkIf4Orbs();

    case LTile::S_EarthOrb:
        TakeOrb( Item_OrbEarth );
        return true;

    case LTile::S_FireOrb:
        TakeOrb( Item_OrbFire );
        return true;

    case LTile::S_WaterOrb:
        TakeOrb( Item_OrbWater );
        return true;

    case LTile::S_AirOrb:
        TakeOrb( Item_OrbAir );
        return true;

    default:
        return true;
    }
}

bool Level::CanWalk( int col, int row )
{
    uint8_t ref = GetTileRef( col, row );
    uint16_t attrs = tileAttr[ref];

    if ( !LTile::CanWalk( attrs ) && LTile::GetSpecial( attrs ) == 0 )
        return false;

    if ( !CanWalkSpecial( col, row ) )
        return false;

    return true;
}

bool Level::CanPlayerMove( int col, int row )
{
    for ( int i = 0; i < Objects; i++ )
    {
        auto& obj = objects[i];

        if ( obj.Type == 0 )
            continue;

        if ( col == obj.Col && row == obj.Row )
        {
            if ( obj.MoveTimer >= 8 )
                obj.MoveTimer = 6;
            return false;
        }

        if ( col == obj.LastCol && row == obj.LastRow )
        {
            // TODO: animate the sprite faster
            if ( obj.SpeedX < 0 )
                obj.SpeedX = -0.5f;
            else if ( obj.SpeedX > 0 )
                obj.SpeedX = 0.5f;
            else if ( obj.SpeedY < 0 )
                obj.SpeedY = -0.5f;
            else if ( obj.SpeedY > 0 )
                obj.SpeedY = 0.5f;
            break;
        }
    }

    if ( !CanWalk( col, row ) )
        return false;

    return true;
}

bool Level::CanPlayerTalk( int col, int row, int& objIndex )
{
    for ( int i = 0; i < Objects; i++ )
    {
        auto& obj = objects[i];

        if ( obj.Type == 0 )
            continue;

        int intOffsetX = (int) obj.OffsetX;
        int intOffsetY = (int) obj.OffsetY;

        if ( abs( intOffsetX ) >= TileWidth / 2 || abs( intOffsetY ) >= TileHeight / 2 )
        {
            if ( col == obj.Col && row == obj.Row )
            {
                objIndex = i;
                return true;
            }
        }
        else
        {
            if ( col == obj.LastCol && row == obj.LastRow )
            {
                objIndex = i;
                return true;
            }
        }
    }

    return false;
}

void Level::CheckObject( int type, CheckResult& result )
{
    CheckRoutine routine = GetObjectRoutine( type );

    result.Message = 0;
    result.Fight = false;
    result.Teleport = false;

    routine( checkParams[type], result );

    fightPending = result.Fight;
    formationId = result.FormationId;

    teleportPending = result.Teleport;
    teleportType = LTile::TT_Swap;
    teleportId = result.TeleportId;
}

void Level::OpenChest( int chestId, int col, int row, CheckResult& result )
{
    if ( Player::GetChestOpened( chestId ) )
    {
        result.Message = Dialog_EmptyChest;
    }
    else
    {
        int itemId = chests[chestId];

        if ( itemId < Player::MoneyBaseId )
        {
            if ( Player::Items[itemId] < Player::MaxItems )
            {
                Player::Items[itemId]++;

                if ( itemId < Player::PotionItemsBaseId )
                    Sound::PushTrack( Sound_Fanfare, 0 );
                else
                    Sound::PushTrack( Sound_GotItem, 0 );

                Player::SetChestOpened( chestId, true );
                SetTileRef( col, row, Tile_OpenChest );
                result.Message = Dialog_OpenChest;
                result.ItemName = Player::GetItemName( itemId );
            }
            else
            {
                result.Message = Dialog_CantCarry;
            }
        }
        else
        {
            Player::AddG( Global::GetPrice( itemId ) );

            Sound::PushTrack( Sound_GotItem, 0 );

            Player::SetChestOpened( chestId, true );
            SetTileRef( col, row, Tile_OpenChest );
            result.Message = Dialog_OpenChest;
            result.ItemName = Player::GetItemName( itemId );
        }
    }
}

void Level::CheckTile( int col, int row, CheckResult& result )
{
    uint8_t ref = GetTileRef( col, row );
    uint16_t attrs = tileAttr[ref];

    result.Fight = false;
    result.Teleport = false;

    if ( LTile::GetSpecial( attrs ) == LTile::S_Treasure )
    {
        int chestId = LTile::GetChest( attrs );

        OpenChest( chestId, col, row, result );
    }
    else if ( LTile::HasMessage( attrs ) )
    {
        result.Message = LTile::GetMessage( attrs );
    }
    else
    {
        result.Message = 0;
    }
}

static void DealTileDamage()
{
    for ( int i = 0; i < Players; i++ )
    {
        Player::Character& player = Player::Party[i];

        if ( player.hp > 1 )
            player.hp--;
    }
}

void Level::DealMoveDamage( int col, int row )
{
    uint8_t ref = GetTileRef( col, row );
    uint16_t attrs = tileAttr[ref];

    if ( LTile::GetSpecial( attrs ) == LTile::S_Damage )
    {
        DealTileDamage();
        flashMove = true;
    }

    Player::DealPoisonDamage();
}

void Level::UpdateFootIdle()
{
    if ( Input::IsKeyPressing( MenuKey ) )
    {
        SceneStack::ShowMenu();
        return;
    }

    if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Point facingPos = GetFacingRowCol( facingDir );
        int objIndex = 0;
        CheckResult result = { 0 };

        if ( CanPlayerTalk( facingPos.X, facingPos.Y, objIndex ) )
        {
            CheckObject( objects[objIndex].Type, result );

            talkingObjIndex = objIndex;
            talkingObjOrigDir = objectSprites[objIndex]->GetDirection();
            Direction dirToPlayer = GetOppositeDir( facingDir );
            objectSprites[objIndex]->SetDirection( dirToPlayer );
        }
        else 
        {
            CheckTile( facingPos.X, facingPos.Y, result );

            talkingObjIndex = NoObject;
        }

        const char* text = messages.GetItem( result.Message );

        dialog.Reinit( text, result.ItemName );

        curUpdate = &Level::UpdateDialog;

        return;
    }

    Direction dir = Input::GetInputDirection();

    if ( dir != Dir_None )
    {
        Point facingPos = GetFacingRowCol( dir );

        facingDir = dir;
        playerSprite->SetDirection( dir );

        if ( CanPlayerMove( facingPos.X, facingPos.Y ) )
        {
            movingDir = dir;
            playerCol = facingPos.X;
            playerRow = facingPos.Y;
            playerSprite->Start();

            curUpdate = &Level::UpdateMoving;

            DealMoveDamage( facingPos.X, facingPos.Y );
        }
    }
}

void Level::UpdateMoving()
{
    // don't go faster than a tile edge
    int speed = 1;
    int shiftX = 0;
    int shiftY = 0;

    switch ( movingDir )
    {
    case Dir_Right: shiftX = speed; break;
    case Dir_Left:  shiftX = -speed; break;
    case Dir_Down:  shiftY = speed; break;
    case Dir_Up:    shiftY = -speed; break;
    }

    ShiftMap( shiftX, shiftY );

    if ( offsetX == 0 && offsetY == 0 )
    {
        movingDir = Dir_None;
        playerSprite->Stop();
        flashMove = false;

        curUpdate = &Level::UpdateFootIdle;

        if ( !CheckPendingAction() )
            UpdateFootIdle();
    }
}

bool Level::CheckPendingAction()
{
    Point curRowCol = GetPlayerRowCol();
    uint8_t ref = GetTileRef( curRowCol.X, curRowCol.Y );
    uint16_t attrs = tileAttr[ref];

    if ( fightPending )
    {
        fightPending = false;
        SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
            [this] { SceneStack::EnterBattle( formationId, backdrop ); } );
        Sound::PlayEffect( SEffect_Fight );
        return true;
    }
    else if ( shopPending )
    {
        shopPending = false;

        SceneStack::FadeEndProc p =
            [this, curRowCol]
            {
                SceneStack::ShowShop( shopId );

                // prepare for player coming out of shop

                tileRefs[curRowCol.Y][curRowCol.X] = origShopDoor;
                inRoom = Out;
                playerSprite->SetDirection( Dir_Down );
            };
        SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), p );
        return true;
    }
    else
    {
        if ( !teleportPending )
        {
            teleportType = LTile::GetTeleportType( attrs );
            if ( teleportType != LTile::TT_None )
            {
                teleportPending = true;
                teleportId = LTile::GetTeleport( attrs );
            }
        }

        if ( teleportPending )
        {
            teleportPending = false;

            if ( teleportType == LTile::TT_Warp )
            {
                SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
                    [this] { SceneStack::PopLevel(); } );
            }
            else if ( teleportType == LTile::TT_Exit )
            {
                OWTeleport& teleport = exitTeleports[teleportId];
                SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
                    [this, teleport] 
                    { SceneStack::SwitchToField( teleport.Col, teleport.Row ); } );
            }
            else if ( teleportType == LTile::TT_Swap )
            {
                LTeleport& teleport = swapTeleports[teleportId];
                SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
                    [this, teleport] 
                    { SceneStack::PushLevel( teleport.MapId, teleport.Col, teleport.Row ); } );
            }
            return true;
        }
    }
    return false;
}

IPlayfield* Level::AsPlayfield()
{
    return this;
}

Point Level::GetCurrentPos()
{
    return GetPlayerRowCol();
}

int Level::GetInRoom()
{
    return inRoom;
}

void Level::HandleShopClosed()
{
    SceneStack::BeginFade( 15, Color::Black(), Color::Transparent(), [] {} );
    Sound::PlayTrack( song, 0, true );
}

void Level::HandleFightEnded()
{
    if ( formationId == Fight_Chaos )
        SceneStack::SwitchScene( SceneId_Ending );
    else
    {
        SceneStack::BeginFade( 15, Color::Black(), Color::Transparent(), [] {} );
        Sound::PlayTrack( song, 0, true );
    }
}

void Level::HandleOpeningEnded()
{
    SceneStack::BeginFade( 15, Color::White(), Color::Transparent(), [] {} );
    Sound::PlayTrack( song, 0, true );
}

void Level::RefreshVisibleObjects()
{
    // update the player sprite, in case the player changed class

    playerSprite->SetFrames( Player::Party[0]._class * 16 );

    for ( int i = 0; i < Objects; i++ )
    {
        auto& obj = objects[i];

        if ( obj.OrigType == 0 )
            continue;

        if ( obj.Type == 0 && Player::GetObjVisible( obj.OrigType ) )
        {
            obj.Type = obj.OrigType;
            MakeObjectSprite( i );
        }
        else if ( obj.Type != 0 && !Player::GetObjVisible( obj.OrigType ) )
        {
            obj.Type = 0;
            delete objectSprites[i];
            objectSprites[i] = nullptr;
        }
    }
}

void Level::UpdateDialog()
{
    dialog.Update();

    if ( dialog.IsClosed() )
    {
        RefreshVisibleObjects();

        if ( talkingObjIndex != NoObject
            && objectSprites[talkingObjIndex] != nullptr )
            objectSprites[talkingObjIndex]->SetDirection( talkingObjOrigDir );

        curUpdate = &Level::UpdateFootIdle;

        CheckPendingAction();
    }
}

void Level::UpdateObject()
{
    const Direction dirs[] = 
    {
        Dir_Right,
        Dir_Left,
        Dir_Down,
        Dir_Up,
    };

    int index = nextObjIndex;
    auto& obj = objects[index];

    nextObjIndex = (nextObjIndex + 1) % Objects;

    if ( obj.Type == 0
        || (obj.Flags & Obj_Stand) != 0
        || (obj.SpeedX != 0 || obj.SpeedY != 0) )
        return;

    if ( obj.MoveTimer == 0 )
    {
        int r = GetNextRandom( 4 );
        Direction nextDir = dirs[r];

        int shiftCol = 0;
        int shiftRow = 0;

        switch ( nextDir )
        {
        case Dir_Right: shiftCol = 1; break;
        case Dir_Left:  shiftCol = -1; break;
        case Dir_Down:  shiftRow = 1; break;
        case Dir_Up:    shiftRow = -1; break;
        }

        uint8_t nextCol = (obj.Col + shiftCol + ColCount) % ColCount;
        uint8_t nextRow = (obj.Row + shiftRow + RowCount) % RowCount;

        if ( CanObjectMove( index, nextCol, nextRow ) )
        {
            obj.SpeedX = shiftCol * 0.25f;
            obj.SpeedY = shiftRow * 0.25f;

            obj.OffsetX = 0;
            obj.OffsetY = 0;

            obj.Col = nextCol;
            obj.Row = nextRow;

            objectSprites[index]->SetDirection( nextDir );
            objectSprites[index]->Start();

            obj.MoveTimer = GetNextRandom( 8 ) * 2;
        }
        // else leave the timer 0 to pick another direction next time
    }
    else
    {
        obj.MoveTimer--;
    }
}

bool Level::CanObjectMove( int index, int col, int row )
{
    int ref = GetTileRef( col, row );
    uint16_t attrs = tileAttr[ref];

    if ( !LTile::CanWalk( attrs ) 
        || LTile::GetTeleportType( attrs ) != LTile::TT_None
        || (col == playerCol && row == playerRow) )
        return false;

    for ( int i = 0; i < Objects; i++ )
    {
        if ( i == index || objects[i].Type == 0 )
            continue;

        if ( col == objects[i].Col && row == objects[i].Row )
            return false;
    }

    return true;
}

void Level::UpdateMoveObjects()
{
    for ( int i = 0; i < Objects; i++ )
    {
        auto& obj = objects[i];

        if ( obj.Type == 0 )
            continue;
        if ( (obj.Flags & Obj_Stand) != 0 )
            continue;
        if ( obj.SpeedX == 0 && obj.SpeedY == 0 )
            continue;

        obj.OffsetX += obj.SpeedX;
        obj.OffsetY += obj.SpeedY;

        int intOffsetX = (int) obj.OffsetX;
        int intOffsetY = (int) obj.OffsetY;

        int nextX = (obj.LastCol * TileWidth + intOffsetX + WorldWidth) % WorldWidth;
        int nextY = (obj.LastRow * TileHeight + intOffsetY + WorldHeight) % WorldHeight;

        objectSprites[i]->SetX( nextX );
        objectSprites[i]->SetY( nextY );

        if ( abs( intOffsetX ) >= TileWidth || abs( intOffsetY ) >= TileHeight )
        {
            obj.SpeedX = 0;
            obj.SpeedY = 0;
            obj.LastCol = obj.Col;
            obj.LastRow = obj.Row;

            objectSprites[i]->Stop();
        }
    }
}

void Level::UpdateObjectSprites()
{
    for ( int i = 0; i < Objects; i++ )
    {
        if ( objectSprites[i] != nullptr )
        {
            objectSprites[i]->Update();
        }
    }
}

int Level::GetBattleFormation()
{
    int domain = FirstLevelDomain + mapId;

    return Global::GetBattleFormation( domain );
}

bool Level::IsActive()
{
    return instance != nullptr;
}

uint16_t Level::GetCurrentTileAttr()
{
    if ( instance == nullptr )
        return 0;

    Point curPos = instance->GetPlayerRowCol();
    int ref = instance->GetTileRef( curPos.X, curPos.Y );

    return instance->tileAttr[ref];
}
