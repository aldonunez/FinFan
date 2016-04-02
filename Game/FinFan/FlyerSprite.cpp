/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "FlyerSprite.h"


const Bounds16 vehicleGoUpFrames[2] = 
{
    { 1 * 16, 0 * 16, 16, 16 },
    { 0 * 16, 0 * 16, 16, 16 },
};

const Bounds16 vehicleGoLeftFrames[2] = 
{
    { 3 * 16, 0 * 16, 16, 16 },
    { 2 * 16, 0 * 16, 16, 16 },
};


FlyerSprite::FlyerSprite( ALLEGRO_BITMAP* bmp )
    :   sprite( bmp ),
        direction( Dir_Down ),
        frameOffsetY( 0 )
{
    SetFrames();
}

int FlyerSprite::GetX()
{
    return sprite.GetX();
}

int FlyerSprite::GetY()
{
    return sprite.GetY();
}

Direction FlyerSprite::GetDirection()
{
    return direction;
}

void FlyerSprite::SetX( int x )
{
    sprite.SetX( x );
}

void FlyerSprite::SetY( int y )
{
    sprite.SetY( y );
}

void FlyerSprite::SetFrames( int yOffset )
{
    frameOffsetY = yOffset;
    SetFrames();
}

void FlyerSprite::SetDirection( Direction direction )
{
    this->direction = direction;
    SetFrames();
}

void FlyerSprite::Start()
{
}

void FlyerSprite::Stop()
{
}

void FlyerSprite::Update()
{
    sprite.Update();
}

void FlyerSprite::DrawAt( int screenX, int screenY )
{
    sprite.DrawAt( screenX, screenY );
}

void FlyerSprite::SetFrames()
{
    switch ( direction )
    {
    case Dir_Right: 
    case Dir_Left:
        sprite.SetFrames( vehicleGoLeftFrames, frameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;

    case Dir_Down:
    case Dir_Up:
        sprite.SetFrames( vehicleGoUpFrames, frameOffsetY );
        sprite.SetBitmapFlags( 0 );
        break;
    }
}
