/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class IMapSprite
{
public:
    virtual int GetX() = 0;
    virtual int GetY() = 0;

    virtual void SetX( int x ) = 0;
    virtual void SetY( int y ) = 0;

    virtual Direction GetDirection() = 0;
    virtual void SetDirection( Direction direction ) = 0;

    virtual void Start() = 0;
    virtual void Stop() = 0;

    virtual void Update() = 0;
    virtual void DrawAt( int screenX, int screenY ) = 0;
};


class ICustomMapSprite : public IMapSprite
{
public:
    virtual void SetFrames( int yOffset ) = 0;
};


class MapSprite : public ICustomMapSprite
{
    ALLEGRO_BITMAP* bmp;
    Direction dir;
    int x;
    int y;
    int frameOffsetY;
    int frame;
    int frameTime;
    bool animating;
    bool showBottom;

public:
    MapSprite( ALLEGRO_BITMAP* bitmap );

    int GetX();
    int GetY();

    void SetX( int x );
    void SetY( int y );

    Direction GetDirection();
    void SetDirection( Direction direction );

    void SetFrames( int yOffset );
    void ShowBottom( bool value );

    void Start();
    void Stop();

    void Update();
    void DrawAt( int screenX, int screenY );

private:
    void DrawHorizontalAt( int screenX, int screenY );
    void DrawVerticalAt( int screenX, int screenY );
};
