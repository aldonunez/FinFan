/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Text.h"
#include <allegro5\allegro_primitives.h>


namespace Text
{
    struct Point
    {
        uint8_t X;
        uint8_t Y;
    };


    const ALLEGRO_COLOR White = al_map_rgb_f( 1, 1, 1 );


    ALLEGRO_BITMAP* font;
    ALLEGRO_BITMAP* fontB;
    Point fontChars[128];


    bool LoadDataFile( const char* path, void* buffer, size_t length )
    {
        FILE* file = nullptr;
        size_t countRead = 0;
        errno_t err = 0;

        err = fopen_s( &file, path, "rb" );
        if ( err != 0 )
            return false;

        countRead = fread( buffer, length, 1, file );
        if ( countRead < 1 )
            return false;

        fclose( file );

        return true;
    }

    bool Init()
    {
        font = al_load_bitmap( "font.png" );
        if ( font == nullptr )
            return false;

        fontB = al_load_bitmap( "fontB.png" );
        if ( fontB == nullptr )
            return false;

        if ( !LoadDataFile( "main.mfont", fontChars, sizeof fontChars ) )
            return false;

        return true;
    }

    void Uninit()
    {
        if ( font != nullptr )
        {
            al_destroy_bitmap( font );
            font = nullptr;
        }

        if ( fontB != nullptr )
        {
            al_destroy_bitmap( fontB );
            fontB = nullptr;
        }
    }

    void DrawString( const char* str, int fontId, int x, int y, ALLEGRO_COLOR tint )
    {
        ALLEGRO_BITMAP* f = nullptr;
        const char* s = str;
        int origX = x;

        if ( fontId == 1 )
            f = fontB;
        else
            f = font;

        al_hold_bitmap_drawing( true );

        while ( *s != '\0' )
        {
            if ( *s == '\n' )
            {
                x = origX;
                y += 8;
                s++;
            }
            else
            {
                al_draw_tinted_bitmap_region(
                    f,
                    tint,
                    fontChars[*s].X,
                    fontChars[*s].Y,
                    8,
                    8,
                    x,
                    y,
                    0 );
                x += 8;
                s++;
            }
        }

        al_hold_bitmap_drawing( false );
    }

    void DrawString( const char* str, int x, int y )
    {
        DrawString( str, 0, x, y, White );
    }

    void DrawDialogString( const char* str, int x, int y, const char* itemName, ALLEGRO_COLOR tint )
    {
        const char* s = str;

        al_hold_bitmap_drawing( true );

        while ( *s != '\0' && *s != '\n' )
        {
            if ( *s == 2 && itemName != nullptr )
            {
                const char* val = itemName;

                while ( *val != '\0' && *val != '\n' )
                {
                    DrawChar( *val, x, y, tint );
                    x += 8;
                    val++;
                }
            }
            else
            {
                DrawChar( *s, x, y, tint );
            }

            x += 8;
            s++;
        }

        al_hold_bitmap_drawing( false );
    }

    void DrawDialogString( const char* str, int x, int y, const char* itemName )
    {
        DrawDialogString( str, x, y, itemName, White );
    }

    void DrawStringRight( const char* str, int fontId, ALLEGRO_COLOR tint, int x, int y )
    {
        ALLEGRO_BITMAP* f = nullptr;
        const char* s = str;

        while ( *s != '\0' )
            s++;

        if ( fontId == 1 )
            f = fontB;
        else
            f = font;

        for ( s--; s >= str; s-- )
        {
            x -= 8;
            al_draw_tinted_bitmap_region(
                f,
                tint,
                fontChars[*s].X,
                fontChars[*s].Y,
                8,
                8,
                x,
                y,
                0 );
        }
    }

    void DrawStringRight( const char* str, int x, int y )
    {
        DrawStringRight( str, 0, White, x, y );
    }

    void DrawStringCenter( const char* str, int x, int y, int width )
    {
        int len = strlen( str );
        int strWidth = len * 8;
        int xOffset = (width - strWidth) / 2;

        DrawString( str, x + xOffset, y );
    }

    void DrawChar( char c, int x, int y, ALLEGRO_COLOR tint )
    {
        al_draw_tinted_bitmap_region(
            font,
            tint,
            fontChars[c].X,
            fontChars[c].Y,
            8,
            8,
            x,
            y,
            0 );
    }

    void DrawChar( char c, int count, int x, int y )
    {
        for ( int i = 0; i < count; i++ )
        {
            DrawChar( c, x, y, White );
            x += 8;
        }
    }

    void DrawBox( int x, int y, int width, int height )
    {
        ALLEGRO_COLOR backColor = al_map_rgb( 0, 0, 252 );
        int middleHCount = width / 8 - 2;
        int middleVCount = height / 8 - 2;
        int lastRowY = y + height - 8;
        int lastColX = x + width - 8;

        al_draw_filled_rectangle( x + 4, y + 4, x + width - 4, y + height - 4, backColor );

        al_hold_bitmap_drawing( true );

        DrawChar( '\4', 1, x, y );
        DrawChar( '\5', middleHCount, x + 8, y );
        DrawChar( '\6', 1, lastColX, y );

        for ( int i = 0; i < middleVCount; i++ )
        {
            DrawChar( '\7', 1, x, y + 8 + i * 8 );
            DrawChar( '\10', 1, lastColX, y + 8 + i * 8 );
        }

        DrawChar( '\11', 1, x, lastRowY );
        DrawChar( '\12', middleHCount, x + 8, lastRowY );
        DrawChar( '\13', 1, lastColX, lastRowY );

        al_hold_bitmap_drawing( false );
    }

    void DrawBoxPart( int x, int y, int width, int height, int visHeight )
    {
        if ( visHeight == 0 )
            return;

        ALLEGRO_COLOR backColor = al_map_rgb( 0, 0, 252 );
        int middleHCount = width / 8 - 2;
        int middleVCount = visHeight / 8 - 1;
        int lastRowY = y + height - 8;
        int lastColX = x + width - 8;
        int fillHeight = visHeight - 4;

        if ( visHeight == height )
        {
            middleVCount--;
            fillHeight -= 4;
        }

        al_draw_filled_rectangle( x + 4, y + 4, x + width - 4, y + 4 + fillHeight, backColor );

        al_hold_bitmap_drawing( true );

        if ( visHeight >= 8 )
        {
            DrawChar( '\4', 1, x, y );
            DrawChar( '\5', middleHCount, x + 8, y );
            DrawChar( '\6', 1, lastColX, y );
        }

        for ( int i = 0; i < middleVCount; i++ )
        {
            DrawChar( '\7', 1, x, y + 8 + i * 8 );
            DrawChar( '\10', 1, lastColX, y + 8 + i * 8 );
        }

        if ( visHeight == height )
        {
            DrawChar( '\11', 1, x, lastRowY );
            DrawChar( '\12', middleHCount, x + 8, lastRowY );
            DrawChar( '\13', 1, lastColX, lastRowY );
        }

        al_hold_bitmap_drawing( false );
    }

    void DrawCursor( int x, int y )
    {
        int sy = al_get_bitmap_height( font ) - 16;

        al_draw_bitmap_region( font, 0, sy, 16, 16, x, y, 0 );
    }
}
