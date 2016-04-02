/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Dialog.h"
#include "Text.h"


const int TextLeftCol = 2;
const int TextTopRow = 2;
const int BoxLeft = 16;
const int BoxTop = 16;
const int BoxWidth = 224;
const int BoxHeight = 11 * 8;
const int ScrollFrames = 10;
const int LineFrames = 1;


Dialog::Dialog()
    :   isClosed( true ),
        itemName( nullptr ),
        visibleBoxLines( 0 ),
        frameTimer( 0 ),
        curUpdate( &Dialog::UpdateClose )
{
}

void Dialog::Reinit( const char* text, const char* itemName )
{
    SplitText( text );

    this->itemName = itemName;
    isClosed = false;
    visibleBoxLines = 0;
    frameTimer = ScrollFrames;
    curUpdate = &Dialog::UpdateOpen;
}

bool Dialog::IsClosed()
{
    return isClosed;
}

void Dialog::SplitText( const char* text )
{
    const char* nextLine = text;
    int i = 0;

    for ( i = 0; i < TextLines; i++ )
    {
        lines[i] = nextLine;

        const char* s = nextLine;

        while ( *s != '\0' && *s != '\n' )
            s++;

        if ( *s == '\0' )
            break;

        nextLine = s + 1;
    }

    for ( i++ ; i < TextLines; i++ )
    {
        lines[i] = "";
    }
}

void Dialog::Draw()
{
    Text::DrawBoxPart( BoxLeft, BoxTop, BoxWidth, BoxHeight, visibleBoxLines * 8 );

    int linesToDraw = visibleBoxLines - TextTopRow;

    if ( linesToDraw > TextLines )
        linesToDraw = TextLines;

    for ( int i = 0; i < linesToDraw; i++ )
    {
        int x = BoxLeft + TextLeftCol * 8;
        int y = BoxTop + (TextTopRow + i) * 8;

        Text::DrawDialogString( lines[i], x, y, itemName );
    }
}

void Dialog::Update()
{
    (this->*curUpdate)();
}

void Dialog::UpdateOpen()
{
    if ( frameTimer == 0 )
    {
        visibleBoxLines = BoxLines;

        curUpdate = &Dialog::UpdateIdle;
    }
    else
    {
        frameTimer--;

        visibleBoxLines = (ScrollFrames - frameTimer) / LineFrames;
    }
}

void Dialog::UpdateIdle()
{
    if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        frameTimer = ScrollFrames;

        curUpdate = &Dialog::UpdateClose;
    }
}

void Dialog::UpdateClose()
{
    if ( frameTimer == 0 )
    {
        isClosed = true;
    }
    else
    {
        frameTimer--;

        visibleBoxLines = frameTimer / LineFrames;
    }
}
