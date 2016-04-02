/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "MapSprite.h"
#include "Sprite.h"


class FlyerSprite : public ICustomMapSprite
{
    Sprite sprite;
    Direction direction;
    int frameOffsetY;

public:
    FlyerSprite( ALLEGRO_BITMAP* bitmap );

    int GetX();
    int GetY();

    void SetX( int x );
    void SetY( int y );

    Direction GetDirection();
    void SetDirection( Direction direction );

    void SetFrames( int yOffset );

    void Start();
    void Stop();

    void Update();
    void DrawAt( int screenX, int screenY );

private:
    void SetFrames();
};
