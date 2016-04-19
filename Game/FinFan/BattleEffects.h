/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Battle.h"


namespace Battle
{

class SpecialEffect : public Effect
{
    int timer;
    Bounds bounds;
    bool visible[4];
    Point pos[4];
    int frameCount;
    int frame;
    int index;

public:
    SpecialEffect( const Bounds& bounds, int index, int frameCount );
    virtual void Update();
    virtual void Draw();
};

class StrikeEffect : public SpecialEffect
{
public:
    StrikeEffect( const Bounds& bounds, int index );
};

class SpellEffect : public SpecialEffect
{
public:
    SpellEffect( const Bounds& bounds, int index );
};

#if 0
const int NumbersPathY[] = 
{
    0,
    9,
    13,
    15,
    16,
    15,
    13,
    9,
    0,
    4,
    6,
    7,
    6,
    4,
    0
};
#endif

class NumbersEffect : public Effect
{
    int timer;
    Bounds bounds;
    char text[11];
    Point pos;
    ALLEGRO_COLOR color;

public:
    NumbersEffect( const Bounds& bounds, const char* text, ALLEGRO_COLOR color = Color::White() );
    virtual void Update();
    virtual void Draw();
};

}
