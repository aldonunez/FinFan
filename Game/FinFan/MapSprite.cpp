/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "MapSprite.h"


const int DefaultTimer = 7;

const Bounds walkLeftFrames[2] = 
{
    { 2 * 16, 0, 16, 16 },
    { 3 * 16, 0, 16, 16 },
};


MapSprite::MapSprite( ALLEGRO_BITMAP* bitmap )
    :   bmp( bitmap ),
        dir( Dir_Down ),
        x( 0 ),
        y( 0 ),
        frameOffsetY( 0 ),
        frame( 0 ),
        frameTime( 0 ),
        animating( false ),
        showBottom( true )
{
}

void MapSprite::SetFrames( int yOffset )
{
    frameOffsetY = yOffset;
    frameTime = 0;
    frame = 0;
}

void MapSprite::ShowBottom( bool value )
{
    showBottom = value;
}

void MapSprite::SetDirection( Direction direction )
{
    dir = direction;
}

void MapSprite::SetX( int x )
{
    this->x = x;
}

void MapSprite::SetY( int y )
{
    this->y = y;
}

int MapSprite::GetX()
{
    return x;
}

int MapSprite::GetY()
{
    return y;
}

Direction MapSprite::GetDirection()
{
    return dir;
}

void MapSprite::Start()
{
    animating = true;
}

void MapSprite::Stop()
{
    animating = false;
}

void MapSprite::Update()
{
    if ( animating )
    {
        if ( frameTime == 0 )
        {
            frame = (frame + 1) % 2;
            frameTime = DefaultTimer;
        }
        else
        {
            frameTime--;
        }
    }
}

void MapSprite::DrawAt( int screenX, int screenY )
{
    switch ( dir )
    {
    case Dir_Right:
    case Dir_Left:
        DrawHorizontalAt( screenX, screenY );
        break;

    case Dir_Down:
    case Dir_Up:
        DrawVerticalAt( screenX, screenY );
        break;
    }
}

void MapSprite::DrawHorizontalAt( int screenX, int screenY )
{
    int flags = (dir == Dir_Right) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    int height = showBottom ? 16 : 8;

    al_draw_bitmap_region(
        bmp,
        walkLeftFrames[frame].X,
        walkLeftFrames[frame].Y + frameOffsetY,
        walkLeftFrames[frame].Width,
        height,
        screenX,
        screenY,
        flags );
}

void MapSprite::DrawVerticalAt( int screenX, int screenY )
{
    int srcX = (dir == Dir_Up) ? 16 : 0;
    int bottomFlags = (frame == 0) ? 0 : ALLEGRO_FLIP_HORIZONTAL;

    al_draw_bitmap_region( 
        bmp, 
        srcX, 
        frameOffsetY + 0, 
        16, 
        8, 
        screenX,
        screenY,
        0 );

    if ( showBottom )
    {
        al_draw_bitmap_region( 
            bmp, 
            srcX, 
            frameOffsetY + 8, 
            16, 
            8, 
            screenX, 
            screenY + 8, 
            bottomFlags );
    }
}
