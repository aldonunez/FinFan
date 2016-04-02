/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "MapSprite.h"
#include "Sprite.h"


class VehicleSprite : public IMapSprite
{
    Sprite sprite;
    bool moving;
    Direction direction;
    int frameOffsetY;

public:
    VehicleSprite( ALLEGRO_BITMAP* bmp, int frameOffsetY );

    virtual int GetX();
    virtual int GetY();

    virtual void SetX( int x );
    virtual void SetY( int y );

    virtual Direction GetDirection();
    virtual void SetDirection( Direction direction );

    virtual void Start();
    virtual void Stop();

    virtual void Update();
    virtual void DrawAt( int screenX, int screenY );

private:
    void SetFrames();
};


class CanoeSprite : public VehicleSprite
{
public:
    CanoeSprite( ALLEGRO_BITMAP* bmp );
};


class ShipSprite : public VehicleSprite
{
public:
    ShipSprite( ALLEGRO_BITMAP* bmp );

    virtual void Start();
    virtual void Stop();
};


class AirshipSprite : public IMapSprite
{
public:
    enum State
    {
        Lifting,
        Landing,
    };

private:
    static const int FrameOffsetY = 2 * 16;
    static const int LiftHeight = 2 * 16;
    static const int LiftSpeed = 1;

    Sprite sprite;
    Direction direction;
    ALLEGRO_BITMAP* bitmap;

    int x;
    int y;
    int liftY;
    State state;

public:
    AirshipSprite( ALLEGRO_BITMAP* bmp );

    virtual int GetX();
    virtual int GetY();

    virtual void SetX( int x );
    virtual void SetY( int y );

    virtual Direction GetDirection();
    virtual void SetDirection( Direction direction );

    virtual void Start();
    virtual void Stop();

    virtual void Update();
    virtual void DrawAt( int screenX, int screenY );

    void SetState( State state );
    bool FinishedLiftLand();

private:
    void SetFrames();
};
