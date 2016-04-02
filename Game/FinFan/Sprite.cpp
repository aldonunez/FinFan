/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Sprite.h"


const int DefaultTimer = 8;


Sprite::Sprite( ALLEGRO_BITMAP* bitmap )
    :   bmp( bitmap ),
        x( 0 ),
        y( 0 ),
        flash( false ),
        frame( 0 ),
        frameTime( DefaultTimer ),
        bmpFlags( 0 )
{
}

void Sprite::SetFrames( const Bounds* frames, int yOffset )
{
    for ( int i = 0; i < 2; i++ )
    {
        this->frames[i].X = frames[i].X;
        this->frames[i].Y = frames[i].Y;
        this->frames[i].Y += yOffset;
        this->frames[i].Width = frames[i].Width;
        this->frames[i].Height = frames[i].Height;
    }

    frameTime = DefaultTimer;
    frame = 0;
}

void Sprite::SetFrames( const Bounds16* frames, int yOffset )
{
    memcpy( this->frames, frames, sizeof this->frames );

    for ( int i = 0; i < 2; i++ )
    {
        this->frames[i].Y += yOffset;
    }

    frameTime = DefaultTimer;
    frame = 0;
}

void Sprite::SetBitmapFlags( int bmpFlags )
{
    this->bmpFlags = bmpFlags;
}

void Sprite::SetTimer( int timer )
{
    frameTime = timer;
}

void Sprite::SetX( int x )
{
    this->x = x;
}

void Sprite::SetY( int y )
{
    this->y = y;
}

int Sprite::GetX()
{
    return x;
}

int Sprite::GetY()
{
    return y;
}

int Sprite::GetFlags()
{
    return bmpFlags;
}

void Sprite::Flash( bool enable )
{
    flash = enable;
}

void Sprite::Update()
{
    if ( frameTime == 0 )
    {
        frame = (frame + 1) % 2;
        frameTime = DefaultTimer;
    }

    frameTime--;
}

void Sprite::Draw()
{
    DrawAt( x, y );
}

void Sprite::DrawAt( int screenX, int screenY )
{
    if ( flash && frameTime < 4 )
        return;

    al_draw_bitmap_region(
        bmp,
        frames[frame].X,
        frames[frame].Y,
        frames[frame].Width,
        frames[frame].Height,
        screenX,
        screenY,
        bmpFlags );
}
