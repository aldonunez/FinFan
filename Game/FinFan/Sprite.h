/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


struct ALLEGRO_BITMAP;


class Sprite
{
    int x;
    int y;
    ALLEGRO_BITMAP* bmp;
    Bounds16 frames[2];
    int bmpFlags;
    bool flash;
    int frame;
    int frameTime;

public:
    Sprite( ALLEGRO_BITMAP* bitmap );

    int GetX();
    int GetY();
    int GetFlags();

    void SetFrames( const Bounds* frames, int yOffset );
    void SetFrames( const Bounds16* frames, int yOffset );
    void SetBitmapFlags( int bmpFlags );
    void SetTimer( int timer );

    void SetX( int x );
    void SetY( int y );

    void Flash( bool enable );

    void Update();
    void Draw();
    void DrawAt( int screenX, int screenY );
};
