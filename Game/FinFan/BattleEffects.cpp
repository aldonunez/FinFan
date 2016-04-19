/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "BattleEffects.h"
#include "Text.h"


namespace Battle
{

    SpecialEffect::SpecialEffect( const Bounds& bounds, int index, int frameCount )
        :   timer( 0 ),
            bounds( bounds ),
            frame( 0 ),
            index( index ),
            frameCount( frameCount )
    {
        memset( visible, 0, sizeof visible );
    }

    void SpecialEffect::Update()
    {
        int t = timer % 16;

        if ( timer % 4 == 0 )
        {
            int i = (timer % 16) / 4;

            visible[i] = true;
            pos[i].X = bounds.X + GetNextRandom( bounds.Width - 16 + 8 );
            pos[i].Y = bounds.Y + GetNextRandom( bounds.Height - 16 + 8 );
            frame = (frame + 1) % frameCount;
        }

        timer++;
    }

    void SpecialEffect::Draw()
    {
        for ( int i = 0; i < _countof( visible ); i++ )
        {
            if ( visible[i] )
            {
                al_draw_bitmap_region(
                    battleSprites,
                    48 + frame * 16,
                    index * 16,
                    16,
                    16,
                    pos[i].X,
                    pos[i].Y,
                    0 );
            }
        }
    }

    StrikeEffect::StrikeEffect( const Bounds& bounds, int index )
        :   SpecialEffect( bounds, index, 1 )
    {
    }

    SpellEffect::SpellEffect( const Bounds& bounds, int index )
        :   SpecialEffect( bounds, index, 3 )
    {
    }

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

    NumbersEffect::NumbersEffect( const Bounds& bounds, const char* text, ALLEGRO_COLOR color )
        :   timer( 0 ),
            bounds( bounds ),
            color( color )
    {
        strcpy_s( this->text, text );
        pos.X = bounds.X + bounds.Width;
    }

    void NumbersEffect::Update()
    {
        if ( timer % 2 == 0 )
        {
            int offset = 0;
            int i = timer / 2;

            if ( i < _countof( NumbersPathY ) )
                offset = NumbersPathY[i];

            pos.Y = bounds.Y + bounds.Height - 8 - offset;
        }

        timer++;
    }

    void NumbersEffect::Draw()
    {
        Text::DrawStringRight( text, Text::FontB, color, pos.X, pos.Y );
    }

}
