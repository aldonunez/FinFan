/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "VehicleSprites.h"


const Bounds16 vehicleStandUpFrames[2] = 
{
    { 0 * 16, 13 * 16, 16, 16 },
    { 0 * 16, 13 * 16, 16, 16 },
};

const Bounds16 vehicleGoUpFrames[2] = 
{
    { 0 * 16, 13 * 16, 16, 16 },
    { 1 * 16, 13 * 16, 16, 16 },
};

const Bounds16 vehicleStandDownFrames[2] = 
{
    { 2 * 16, 13 * 16, 16, 16 },
    { 2 * 16, 13 * 16, 16, 16 },
};

const Bounds16 vehicleGoDownFrames[2] = 
{
    { 2 * 16, 13 * 16, 16, 16 },
    { 3 * 16, 13 * 16, 16, 16 },
};

const Bounds16 vehicleStandLeftFrames[2] = 
{
    { 0 * 16, 14 * 16, 16, 16 },
    { 0 * 16, 14 * 16, 16, 16 },
};

const Bounds16 vehicleGoLeftFrames[2] = 
{
    { 0 * 16, 14 * 16, 16, 16 },
    { 1 * 16, 14 * 16, 16, 16 },
};


//----------------------------------------------------------------------------
//  CanoeSprite
//----------------------------------------------------------------------------

CanoeSprite::CanoeSprite( ALLEGRO_BITMAP* bmp )
    :   VehicleSprite( bmp, 4 * 16 )
{
}


//----------------------------------------------------------------------------
//  ShipSprite
//----------------------------------------------------------------------------

ShipSprite::ShipSprite( ALLEGRO_BITMAP* bmp )
    :   VehicleSprite( bmp, 0 )
{
    VehicleSprite::Start();
}

void ShipSprite::Start()
{
    // the ship always animates
}

void ShipSprite::Stop()
{
    // the ship always animates
}

    
//----------------------------------------------------------------------------
//  VehicleSprite
//----------------------------------------------------------------------------

VehicleSprite::VehicleSprite( ALLEGRO_BITMAP* bmp, int frameOffsetY )
    :   sprite( bmp ),
        moving( false ),
        direction( Dir_Down ),
        frameOffsetY( frameOffsetY )
{
    SetFrames();
}

int VehicleSprite::GetX()
{
    return sprite.GetX();
}

int VehicleSprite::GetY()
{
    return sprite.GetY();
}

void VehicleSprite::SetX( int x )
{
    sprite.SetX( x );
}

void VehicleSprite::SetY( int y )
{
    sprite.SetY( y );
}

Direction VehicleSprite::GetDirection()
{
    return direction;
}

void VehicleSprite::SetDirection( Direction direction )
{
    if ( this->direction != direction )
    {
        this->direction = direction;
        SetFrames();
    }
}

void VehicleSprite::Start()
{
    moving = true;
    SetFrames();
}

void VehicleSprite::Stop()
{
    moving = false;
    SetFrames();
}

void VehicleSprite::Update()
{
    sprite.Update();
}

void VehicleSprite::DrawAt( int screenX, int screenY )
{
    sprite.DrawAt( screenX, screenY );
}

void VehicleSprite::SetFrames()
{
    switch ( direction )
    {
    case Dir_Right: 
        sprite.SetFrames( moving ? vehicleGoLeftFrames : vehicleStandLeftFrames, frameOffsetY );
        sprite.SetBitmapFlags( ALLEGRO_FLIP_HORIZONTAL );
        break;

    case Dir_Left:
        sprite.SetFrames( moving ? vehicleGoLeftFrames : vehicleStandLeftFrames, frameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;

    case Dir_Down:
        sprite.SetFrames( moving ? vehicleGoDownFrames : vehicleStandDownFrames, frameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;

    case Dir_Up:
        sprite.SetFrames( moving ? vehicleGoUpFrames : vehicleStandUpFrames, frameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;
    }
}


//----------------------------------------------------------------------------
//  AirshipSprite
//----------------------------------------------------------------------------

AirshipSprite::AirshipSprite( ALLEGRO_BITMAP* bmp )
    :   sprite( bmp ),
        direction( Dir_Down ),
        bitmap( bmp ),
        x( 0 ),
        y( 0 ),
        liftY( 0 ),
        state( Lifting )
{
    SetFrames();
}

int AirshipSprite::GetX()
{
    return x;
}

int AirshipSprite::GetY()
{
    return y;
}

void AirshipSprite::SetX( int x )
{
    this->x = x;
}

void AirshipSprite::SetY( int y )
{
    this->y = y;
}

Direction AirshipSprite::GetDirection()
{
    return direction;
}

void AirshipSprite::SetDirection( Direction direction )
{
    if ( this->direction != direction )
    {
        this->direction = direction;
        SetFrames();
    }
}

void AirshipSprite::Start()
{
    // the airship always animates
}

void AirshipSprite::Stop()
{
    // the airship always animates
}

void AirshipSprite::Update()
{
    if ( state == Lifting )
    {
        if ( liftY < LiftHeight )
            liftY += LiftSpeed;
    }
    else if ( state == Landing )
    {
        if ( liftY > 0 )
            liftY -= LiftSpeed;
    }

    sprite.Update();
}

void AirshipSprite::DrawAt( int screenX, int screenY )
{
    al_draw_bitmap_region( bitmap, 0, 12 * 16, 16, 16, screenX, screenY, 0 );

    sprite.DrawAt( screenX, screenY - liftY );
}

void AirshipSprite::SetFrames()
{
    switch ( direction )
    {
    case Dir_Right: 
        sprite.SetFrames( vehicleGoLeftFrames, FrameOffsetY );
        sprite.SetBitmapFlags( ALLEGRO_FLIP_HORIZONTAL );
        break;

    case Dir_Left:
        sprite.SetFrames( vehicleGoLeftFrames, FrameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;

    case Dir_Down:
        sprite.SetFrames( vehicleGoDownFrames, FrameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;

    case Dir_Up:
        sprite.SetFrames( vehicleGoUpFrames, FrameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;
    }
}

void AirshipSprite::SetState( State state )
{
    if ( state == Lifting )
        liftY = 0;
    else if ( state == Landing )
        liftY = LiftHeight;

    this->state = state;
}

bool AirshipSprite::FinishedLiftLand()
{
    if ( state == Lifting )
    {
        return liftY == LiftHeight;
    }
    else if ( state == Landing )
    {
        return liftY == 0;
    }

    return true;
}
