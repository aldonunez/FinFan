/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


namespace Text
{
    enum
    {
        FontA,
        FontB,
    };

    bool Init();
    void Uninit();

    void DrawString( const char* str, int x, int y );
    void DrawStringRight( const char* str, int x, int y );
    void DrawStringCenter( const char* str, int x, int y, int width );
    void DrawChar( char c, int count, int x, int y );
    void DrawChar( char c, int x, int y, ALLEGRO_COLOR tint );

    void DrawString( const char* str, int fontId, int x, int y, ALLEGRO_COLOR tint );
    void DrawStringRight( const char* str, int fontId, ALLEGRO_COLOR tint, int x, int y );

    void DrawDialogString( const char* str, int x, int y, const char* itemName );
    void DrawDialogString( const char* str, int x, int y, const char* itemName, ALLEGRO_COLOR tint );

    void DrawBox( int x, int y, int width, int height );
    void DrawBoxPart( int x, int y, int width, int height, int visHeight );

    void DrawCursor( int x, int y );
}
