/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Overworld.h"
#include "MapSprite.h"
#include "OWTile.h"
#include "Player.h"
#include "VehicleSprites.h"
#include "SceneStack.h"
#include "Ids.h"
#include "Sound.h"


Overworld* Overworld::instance;

IMapSprite* vehicleSprite;

MapSprite* playerSprite;
CanoeSprite* canoeSprite;
ShipSprite* shipSprite;
AirshipSprite* airshipSprite;

const int IsmusCol = 102;
const int IsmusRow = 164;
const int BridgeCol = 152;
const int BridgeRow = 152;


Overworld::Overworld()
    :   tiles( nullptr ),
        offsetX( 0 ),
        offsetY( 0 ),
        uncompStartRow( 0 ),
        topRow( 0 ),
        leftCol( 0 ),
        playerImage( nullptr ),
        movingDir( Dir_None ),
        curUpdate( &Overworld::UpdateFootIdle ),
        shopPending( false ),
        shopId ( 0 ),
        poisonMove( false )
{
    instance = this;
}

Overworld::~Overworld()
{
    instance = nullptr;

    al_destroy_bitmap( tiles );
    al_destroy_bitmap( playerImage );

    delete playerSprite;
    playerSprite = nullptr;

    delete canoeSprite;
    canoeSprite = nullptr;

    delete shipSprite;
    shipSprite = nullptr;

    delete airshipSprite;
    airshipSprite = nullptr;
}

IPlayfield* Overworld::AsPlayfield()
{
    return this;
}

Point Overworld::GetCurrentPos()
{
    return GetPlayerRowCol();
}

int Overworld::GetInRoom()
{
    return 0;
}

void Overworld::HandleShopClosed()
{
    SceneStack::BeginFade( 15, Color::Black(), Color::Transparent(), [] {} );
    Sound::PlayTrack( Sound_Field, 0, true );
}

void Overworld::HandleFightEnded()
{
    SceneStack::BeginFade( 15, Color::Black(), Color::Transparent(), [] {} );
    Sound::PlayTrack( Sound_Field, 0, true );
}

void Overworld::HandleOpeningEnded()
{
    SceneStack::BeginFade( 60, Color::White(), Color::Transparent(), [] {} );
    Sound::PlayTrack( Sound_Field, 0, true );
}

void Overworld::Init( int startCol, int startRow )
{
    if ( !LoadResource( "owMap.tab", &compressedRows ) )
        return;

    tiles = al_load_bitmap( "owTiles.png" );
    if ( tiles == nullptr )
        return;

    playerImage = al_load_bitmap( "mapPlayer.png" );
    if ( playerImage == nullptr )
        return;

    if ( !LoadList( "owTileAttr.dat", tileAttr, TileTypes ) )
        return;

    if ( !LoadList( "tileBackdrops.dat", tileBackdrops, TileTypes ) )
        return;

    if ( !LoadList( "enterTeleports.dat", enterTeleports, EnterTeleports ) )
        return;

    LoadMap( startCol, startRow );

    playerSprite = new MapSprite( playerImage );
    playerSprite->SetX( startCol * TileWidth );
    playerSprite->SetY( startRow * TileHeight );
    playerSprite->SetFrames( 0 * 16 );
    playerSprite->SetDirection( Dir_Down );

    canoeSprite = new CanoeSprite( playerImage );

    shipSprite = new ShipSprite( playerImage );

    airshipSprite = new AirshipSprite( playerImage );

    uint8_t ref = GetTileRef( startCol, startRow );
    uint16_t attrs = tileAttr[ref];

    if ( OWTile::CanCanoe( attrs ) )
    {
        Player::SetActiveVehicle( Vehicle_Canoe );

        canoeSprite->SetDirection( Dir_Right );
        canoeSprite->SetX( playerSprite->GetX() );
        canoeSprite->SetY( playerSprite->GetY() );
        vehicleSprite = canoeSprite;

        curUpdate = &Overworld::UpdateCanoeIdle;
    }
    else
    {
        vehicleSprite = playerSprite;
    }

    Sound::PlayTrack( Sound_Field, 0, true );

    SceneStack::BeginFade( 15, Color::Black(), Color::Transparent(), [] {} );
}

void Overworld::LoadMap( int middleCol, int middleRow )
{
    topRow = (middleRow - MiddleRow + RowCount) % RowCount;
    leftCol = (middleCol - MiddleCol + ColCount) % ColCount;

    uncompStartRow = topRow % VisibleRows;

    for ( int i = 0; i < VisibleRows; i++ )
    {
        int bufRow = (uncompStartRow + i) % VisibleRows;
        int mapRow = (topRow + i) % RowCount;
        DecompressMap( compressedRows.GetItem( mapRow ), tileRefs[bufRow] );
    }
}

void Overworld::Update()
{
    if ( SceneStack::IsFading() )
        return;

    (this->*curUpdate)();

    vehicleSprite->Update();

    if (  (Player::GetVehicles() & Vehicle_Airship) != 0 
        && Player::GetAirshipVisibility() < 0x1F )
    {
        Player::SetAirshipVisibility( Player::GetAirshipVisibility() + 1 );
    }
}

void Overworld::UpdateLift()
{
    if ( airshipSprite->FinishedLiftLand() )
    {
        Sound::PlayEffect( SEffect_Airship, true );

        curUpdate = &Overworld::UpdateAirshipIdle;
    }
}

bool Overworld::CanWalk( uint16_t attrs )
{
    if ( !OWTile::CanWalk( attrs ) )
        return false;

    if ( OWTile::GetSpecial( attrs ) == OWTile::S_Chime
        && Player::Items[Item_Chime] == 0 )
        return false;

    if ( OWTile::GetSpecial( attrs ) == OWTile::S_Caravan )
    {
        if ( !Player::GetObjVisible( Obj_Fairy ) )
        {
            shopPending = true;
            shopId = Shop_Caravan;
        }
    }

    return true;
}

static bool CanWalkSpecial( int col, int row )
{
    if ( Player::IsBridgeVisible()
        && col == BridgeCol
        && row == BridgeRow )
        return true;

    if ( Player::IsCanalBlocked() 
        && col == IsmusCol
        && row == IsmusRow )
        return true;

    return false;
}

static bool CanShipSpecial( int col, int row )
{
    if ( !Player::IsCanalBlocked() 
        || col != IsmusCol
        || row != IsmusRow )
        return true;

    return false;
}

void Overworld::UpdateFootIdle()
{
    if ( Input::IsKeyPressing( MenuKey ) )
    {
        SceneStack::ShowMenu();
        return;
    }

    if ( Input::IsKeyDown( ConfirmKey ) )
    {
        if ( (Player::GetVehicles() & Vehicle_Airship) != 0 
            && GetPlayerRowCol() == Player::GetAirshipRowCol() )
        {
            Player::SetActiveVehicle( Vehicle_Airship );

            airshipSprite->SetX( playerSprite->GetX() );
            airshipSprite->SetY( playerSprite->GetY() );
            vehicleSprite = airshipSprite;

            airshipSprite->SetState( AirshipSprite::Lifting );
            curUpdate = &Overworld::UpdateLift;

            Sound::PlayTrack( Sound_Airship, 0, true );
            Sound::PlayEffect( SEffect_Lift );
        }

        return;
    }

    Direction dir = Input::GetInputDirection();

    if ( dir != Dir_None )
    {
        Point facingPos = GetFacingRowCol( dir );
        uint8_t ref = GetTileRef( facingPos.X, facingPos.Y );
        uint16_t attrs = tileAttr[ref];
        bool allowMove = false;

        if ( CanWalk( attrs ) || CanWalkSpecial( facingPos.X, facingPos.Y ) )
            allowMove = true;
        else if ( OWTile::CanCanoe( attrs ) && (Player::GetVehicles() & Vehicle_Canoe) != 0 )
            allowMove = true;
        else if ( OWTile::CanShip( attrs ) && (Player::GetVehicles() & Vehicle_Ship) != 0 )
        {
            Point curRowCol = GetPlayerRowCol();
            uint8_t hereRef = GetTileRef( curRowCol.X, curRowCol.Y );
            uint16_t hereAttrs = tileAttr[hereRef];

            if ( OWTile::IsDock( hereAttrs ) && facingPos == Player::GetShipRowCol() )
                allowMove = true;
        }

        playerSprite->SetDirection( dir );

        if ( allowMove )
        {
            movingSpeed = 1;
            movingDir = dir;
            playerSprite->ShowBottom( !OWTile::IsForest( attrs ) );
            playerSprite->Start();

            curUpdate = &Overworld::UpdateMoving;

            if ( Player::DealPoisonDamage() )
                poisonMove = true;
        }
    }
}

void Overworld::UpdateCanoeIdle()
{
    if ( Input::IsKeyPressing( MenuKey ) )
    {
        SceneStack::ShowMenu();
        return;
    }

    Direction dir = Input::GetInputDirection();

    if ( dir != Dir_None )
    {
        uint8_t ref = GetFacingTileRef( dir );
        uint16_t attrs = tileAttr[ref];
        bool allowMove = false;

        if ( OWTile::CanWalk( attrs ) )
        {
            allowMove = true;

            Player::SetActiveVehicle( Vehicle_Foot );

            playerSprite->ShowBottom( !OWTile::IsForest( attrs ) );
            playerSprite->SetX( canoeSprite->GetX() );
            playerSprite->SetY( canoeSprite->GetY() );
            vehicleSprite = playerSprite;
        }
        else if ( OWTile::CanCanoe( attrs ) )
            allowMove = true;
        else if ( OWTile::CanShip( attrs ) && (Player::GetVehicles() & Vehicle_Ship) != 0
            && Player::GetShipRowCol() == GetFacingRowCol( dir ) )
            allowMove = true;

        if ( allowMove )
        {
            movingSpeed = 1;
            movingDir = dir;
            vehicleSprite->SetDirection( dir );
            vehicleSprite->Start();

            curUpdate = &Overworld::UpdateMoving;
        }
    }
}

void Overworld::UpdateShipIdle()
{
    if ( Input::IsKeyPressing( MenuKey ) )
    {
        SceneStack::ShowMenu();
        return;
    }

    Direction dir = Input::GetInputDirection();

    if ( dir != Dir_None )
    {
        Point facingPos = GetFacingRowCol( dir );
        uint8_t ref = GetFacingTileRef( dir );
        uint16_t attrs = tileAttr[ref];
        bool allowMove = false;

        if ( OWTile::IsDock( attrs ) 
            || (OWTile::CanCanoe( attrs ) && (Player::GetVehicles() & Vehicle_Canoe) != 0) )
        {
            movingSpeed = 1;
            allowMove = true;

            Player::SetActiveVehicle( Vehicle_Foot );

            Sound::PlayTrack( Sound_Field, 0, true );
            Sound::StopEffect();

            playerSprite->ShowBottom( !OWTile::IsForest( attrs ) );
            playerSprite->SetX( shipSprite->GetX() );
            playerSprite->SetY( shipSprite->GetY() );
            vehicleSprite = playerSprite;

            Player::SetShipRowCol( GetPlayerRowCol() );
        }
        else if ( OWTile::CanShip( attrs ) && CanShipSpecial( facingPos.X, facingPos.Y ) )
        {
            movingSpeed = 2;
            allowMove = true;
        }

        if ( allowMove )
        {
            movingDir = dir;
            vehicleSprite->SetDirection( dir );
            vehicleSprite->Start();

            curUpdate = &Overworld::UpdateMoving;
        }
    }
}

void Overworld::UpdateLand()
{
    if ( !airshipSprite->FinishedLiftLand() )
        return;

    Point curRowCol = GetPlayerRowCol();
    uint8_t hereRef = GetTileRef( curRowCol.X, curRowCol.Y );
    uint16_t hereAttrs = tileAttr[hereRef];

    if ( OWTile::CanAirship( hereAttrs ) )
    {
        Player::SetActiveVehicle( Vehicle_Foot );

        playerSprite->SetX( airshipSprite->GetX() );
        playerSprite->SetY( airshipSprite->GetY() );
        vehicleSprite = playerSprite;

        Player::SetAirshipRowCol( curRowCol );

        Sound::PlayTrack( Sound_Field, 0, true );

        curUpdate = &Overworld::UpdateFootIdle;
    }
    else
    {
        airshipSprite->SetState( AirshipSprite::Lifting );
        curUpdate = &Overworld::UpdateLift;
    }
}

void Overworld::UpdateAirshipIdle()
{
    if ( Input::IsKeyPressing( MenuKey ) )
    {
        SceneStack::ShowMenu();
        return;
    }

    if ( Input::IsKeyDown( ConfirmKey ) )
    {
        Sound::PlayEffect( SEffect_Land );
        airshipSprite->SetState( AirshipSprite::Landing );
        curUpdate = &Overworld::UpdateLand;
        return;
    }

    Direction dir = Input::GetInputDirection();

    if ( dir != Dir_None )
    {
        movingSpeed = 4;
        movingDir = dir;
        vehicleSprite->SetDirection( dir );
        vehicleSprite->Start();

        curUpdate = &Overworld::UpdateMoving;
    }
}

void Overworld::UpdateMoving()
{
    // don't go faster than a tile edge
    int speed = movingSpeed;
    int shiftX = 0;
    int shiftY = 0;
    bool playOpening = false;
    bool skipBattle = false;

    switch ( movingDir )
    {
    case Dir_Right: shiftX = speed; break;
    case Dir_Left:  shiftX = -speed; break;
    case Dir_Down:  shiftY = speed; break;
    case Dir_Up:    shiftY = -speed; break;
    }

    ShiftMap( shiftX, shiftY );

    vehicleSprite->SetX( (uint32_t) (vehicleSprite->GetX() + shiftX) % (ColCount*TileWidth) );
    vehicleSprite->SetY( (uint32_t) (vehicleSprite->GetY() + shiftY) % (RowCount*TileHeight) );

    if ( poisonMove )
        Sound::PlayEffect( SEffect_Step );

    if ( offsetX == 0 && offsetY == 0 )
    {
        movingDir = Dir_None;
        vehicleSprite->Stop();
        poisonMove = false;

        Point curCell = GetPlayerRowCol();
        uint8_t ref = GetTileRef( curCell.X, curCell.Y );
        uint16_t attrs = tileAttr[ref];

        if ( Player::GetActiveVehicle() == Vehicle_Airship )
        {
            curUpdate = &Overworld::UpdateAirshipIdle;
        }
        else if ( OWTile::CanCanoe( attrs ) )
        {
            if ( Player::GetActiveVehicle() != Vehicle_Canoe )
            {
                Player::SetActiveVehicle( Vehicle_Canoe );

                canoeSprite->SetDirection( vehicleSprite->GetDirection() );
                canoeSprite->SetX( vehicleSprite->GetX() );
                canoeSprite->SetY( vehicleSprite->GetY() );
                vehicleSprite = canoeSprite;
                skipBattle = true;
            }

            curUpdate = &Overworld::UpdateCanoeIdle;
        }
        else if ( OWTile::CanShip( attrs ) )
        {
            if ( Player::GetActiveVehicle() != Vehicle_Ship && curCell == Player::GetShipRowCol() )
            {
                Player::SetActiveVehicle( Vehicle_Ship );

                Sound::PlayTrack( Sound_Ship, 0, true );
                Sound::PlayEffect( SEffect_Sea, true );

                shipSprite->SetDirection( vehicleSprite->GetDirection() );
                shipSprite->SetX( vehicleSprite->GetX() );
                shipSprite->SetY( vehicleSprite->GetY() );
                vehicleSprite = shipSprite;
                skipBattle = true;

                curUpdate = &Overworld::UpdateShipIdle;
            }
            else if ( Player::GetActiveVehicle() != Vehicle_Ship && curCell != Player::GetShipRowCol() )
            {
                // special case for walking over a special tile that's usually water
                curUpdate = &Overworld::UpdateFootIdle;

                if ( (curCell.X == BridgeCol && curCell.Y == BridgeRow) 
                    && !Player::WasOpeningScenePlayed() )
                {
                    playOpening = true;
                }
            }
            else
            {
                curUpdate = &Overworld::UpdateShipIdle;
            }
        }
        else
            curUpdate = &Overworld::UpdateFootIdle;

        int formationId = 0;
        int teleportId = 0;

        if ( playOpening )
        {
            Player::SetOpeningScenePlayed( true );
            SceneStack::BeginFade( 60, Color::Transparent(), Color::White(),
                [] { SceneStack::ShowOpening(); } );
        }
        else if ( shopPending )
        {
            shopPending = false;
            SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
                [this] { SceneStack::ShowShop( shopId ); } );
        }
        else if ( GetTriggeredTeleport( teleportId ) )
        {
            LTeleport& teleport = enterTeleports[teleportId];
            SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
                [this, teleport] 
                { SceneStack::PushLevel( teleport.MapId, teleport.Col, teleport.Row ); } );
        }
        else if ( !skipBattle && GetTriggeredBattle( formationId ) )
        {
            Point pos = GetPlayerRowCol();
            int tile = GetTileRef( pos.X, pos.Y );

            SceneStack::BeginFade( 15, Color::Transparent(), Color::Black(), 
                [this, formationId, tile] 
                { SceneStack::EnterBattle( formationId, tileBackdrops[tile] ); } );
            Sound::PlayEffect( SEffect_Fight );
        }
        else
        {
            if ( curUpdate != &Overworld::UpdateMoving )
            {
                (this->*curUpdate)();
            }
        }
    }
}

bool Overworld::GetTriggeredTeleport( int& teleportId )
{
    Point curPos = GetPlayerRowCol();
    int ref = GetTileRef( curPos.X, curPos.Y );
    uint16_t attrs = tileAttr[ref];

    if ( Player::GetActiveVehicle() == Vehicle_Airship )
        return false;

    if ( !OWTile::IsTeleport( attrs ) )
        return false;

    teleportId = OWTile::GetTeleport( attrs );

    return true;
}

bool Overworld::GetTriggeredBattle( int& formationId )
{
    Point curPos = GetPlayerRowCol();
    int ref = GetTileRef( curPos.X, curPos.Y );
    uint16_t attrs = tileAttr[ref];

    if ( Player::GetActiveVehicle() == Vehicle_Airship )
        return false;

    if ( !OWTile::CanFight( attrs ) )
        return false;

    if ( (curPos.X == BridgeCol && curPos.Y == BridgeRow)
        || (curPos.X == IsmusCol && curPos.Y == IsmusRow) )
        return false;

    int generalDomain = 0;
    int domain = 0;
    bool triggered = false;
    int r = GetNextRandom( 256 );

    generalDomain = OWTile::GetFightDomain( attrs );

    if ( generalDomain == OWTile::Sea )
        triggered = r < 3;
    else
        triggered = r < 10;

    if ( !triggered )
        return false;

    if ( generalDomain == OWTile::Sea )
    {
        domain = 0x42;
    }
    else if ( generalDomain == OWTile::River )
    {
        if ( curPos.Y < 128 )
            domain = 0x40;
        else
            domain = 0x41;
    }
    else // Land
    {
        int domainCol = curPos.X / 32;
        int domainRow = curPos.Y / 32;

        domain = domainCol | (domainRow << 3);
    }

    formationId = Global::GetBattleFormation( domain );

    return true;
}

Point Overworld::GetPlayerRowCol()
{
    int col = (uint8_t) (leftCol + MiddleCol);
    int row = (uint8_t) (topRow + MiddleRow);
    Point pos = { col, row };

    return pos;
}

Point Overworld::GetFacingRowCol( Direction direction )
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
    pos.X += shiftCol;
    pos.Y += shiftRow;

    return pos;
}

int Overworld::GetFacingTileRef( Direction direction )
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

    int col = (uint8_t) ((vehicleSprite->GetX() / TileWidth) + shiftCol);
    int row = (uint8_t) ((vehicleSprite->GetY() / TileHeight) + shiftRow);

    return GetTileRef( col, row );
}

int Overworld::GetTileRef( int col, int row )
{
    int relRow = (uint8_t) (row - topRow);
    int bufRow = (uncompStartRow + relRow) % VisibleRows;

    return tileRefs[bufRow][col];
}

void Overworld::ShiftMap( int shiftX, int shiftY )
{
    int newBufRow = -1;
    int newMapRow = -1;

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
            uncompStartRow = (uncompStartRow - 1 + VisibleRows) % VisibleRows;
            newBufRow = uncompStartRow;
            newMapRow = topRow;
        }
    }
    else if ( shiftY > 0 )
    {
        offsetY += shiftY;
        if ( offsetY >= TileHeight )
        {
            offsetY -= TileHeight;
            topRow = (topRow + 1) % RowCount;
            newBufRow = uncompStartRow;
            uncompStartRow = (uncompStartRow + 1) % VisibleRows;
            newMapRow = (topRow + (VisibleRows - 1)) % RowCount;
        }
    }

    if ( newBufRow >= 0 )
    {
        DecompressMap( compressedRows.GetItem( newMapRow ), tileRefs[newBufRow] );
    }
}

void Overworld::Draw()
{
    DrawMap();

    DrawVehicles();

    if ( Player::IsCanalBlocked() )
        DrawIsmus();

    if ( Player::IsBridgeVisible() && Player::GetActiveVehicle() != Vehicle_Ship )
        DrawBridge();

    DrawPlayer();

    if ( Player::IsBridgeVisible() && Player::GetActiveVehicle() == Vehicle_Ship )
        DrawBridge();
}

void Overworld::DrawMap()
{
    al_hold_bitmap_drawing( true );

    for ( int i = 0; i < VisibleRows; i++ )
    {
        for ( int j = 0; j < VisibleCols; j++ )
        {
            int bufRow = (uncompStartRow + i) % VisibleRows;
            int bufCol = (leftCol + j) % ColCount;
            int tileRef = tileRefs[bufRow][bufCol];
            int srcX = (tileRef % 16) * TileWidth;
            int srcY = (tileRef / 16) * TileHeight;
            int destX = j * TileWidth - offsetX;
            int destY = i * TileHeight - offsetY;

            al_draw_bitmap_region( tiles, srcX, srcY, TileWidth, TileHeight, destX, destY, 0 );
        }
    }

    al_hold_bitmap_drawing( false );
}

void Overworld::DrawPlayer()
{
    vehicleSprite->DrawAt( MiddleCol * TileWidth, MiddleRow * TileHeight );
}

void Overworld::DrawVehicles()
{
    Vehicle vehicles = Player::GetVehicles();
    Vehicle activeVehicle = Player::GetActiveVehicle();

    if ( (vehicles & Vehicle_Ship) != 0 )
    {
        if ( activeVehicle != Vehicle_Ship )
        {
            Point pos = Player::GetShipRowCol();
            DrawImage( pos.X, pos.Y, 0, 14 );
        }
    }

    if ( (vehicles & Vehicle_Airship) != 0 )
    {
        if ( activeVehicle != Vehicle_Airship )
        {
            Point pos = Player::GetAirshipRowCol();
            if ( (Player::GetAirshipVisibility() & 1) == 1 )
                DrawImage( pos.X, pos.Y, 0, 16 );
        }
    }
}

void Overworld::DrawImage( int col, int row, int imageCol, int imageRow )
{
    col = (uint8_t) (col - leftCol);
    row = (uint8_t) (row - topRow);

    al_draw_bitmap_region( 
        playerImage, 
        imageCol * 16, 
        imageRow * 16, 
        16, 
        16, 
        col * TileWidth - offsetX, 
        row * TileHeight - offsetY, 
        0 );
}

// I don't like the English spelling isthmus
void Overworld::DrawIsmus()
{
    DrawImage( IsmusCol, IsmusRow, 2, 12 );
}

void Overworld::DrawBridge()
{
    DrawImage( BridgeCol, BridgeRow, 1, 12 );
}

bool Overworld::IsActive()
{
    return instance != nullptr;
}

uint16_t Overworld::GetCurrentTileAttr()
{
    if ( instance == nullptr )
        return 0;

    Point curPos = instance->GetPlayerRowCol();
    int ref = instance->GetTileRef( curPos.X, curPos.Y );

    return instance->tileAttr[ref];
}

Point Overworld::GetPlayerPos()
{
    if ( instance == nullptr )
        return Point();

    return instance->GetCurrentPos();
}
