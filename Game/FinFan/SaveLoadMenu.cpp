/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "SaveLoadMenu.h"
#include "Text.h"
#include "SaveFolder.h"
#include "SceneStack.h"
#include "Player.h"


const int BoxY = 16;
const int BoxHeight = 56;
const int TopLineYOff = 8;
const int MidLineYOff = 24;
const int BottomLineYOff = 40;


//----------------------------------------------------------------------------
//  SaveLoadMenu
//----------------------------------------------------------------------------

SaveLoadMenu::SaveLoadMenu( Action action )
    :   action( action ),
        selIndex( 0 ),
        topRow( 0 ),
        hasError( false )
{
    memset( &summaries, 0, sizeof summaries );
    hasError = !SaveFolder::ReadSummaries( summaries );
}

MenuAction SaveLoadMenu::Update( Menu*& nextMenu )
{
    if ( hasError )
    {
        if ( Input::IsKeyPressing( ConfirmKey ) || Input::IsKeyPressing( CancelKey ) )
            return Close();

        return Menu_None;
    }

    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        if ( selIndex < Slots - 1 )
        {
            selIndex++;
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        if ( selIndex > 0 )
        {
            selIndex--;
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        if ( action == Save )
        {
            MainMenu::PlayConfirm();

            hasError = !SaveFolder::SaveFile( selIndex );
            if ( !hasError )
                return Menu_Pop;
        }
        else if ( action == Load )
        {
            if ( summaries.Summaries[selIndex].State == 1 )
            {
                MainMenu::PlayConfirm();

                hasError = !SaveFolder::LoadFile( selIndex );
                if ( !hasError )
                {
                    Point pos = Player::GetPlayerPos();
                    SceneStack::SwitchToField( pos.X, pos.Y );
                }
            }
            else
                MainMenu::PlayError();
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Close();
    }

    if ( selIndex < topRow )
    {
        topRow = selIndex;
    }
    else if ( selIndex >= topRow + 4 )
    {
        topRow = selIndex - 3;
    }

    return Menu_None;
}

MenuAction SaveLoadMenu::Close()
{
    if ( action == Save )
    {
        return Menu_Pop;
    }
    else if ( action == Load )
    {
        SceneStack::SwitchScene( SceneId_Title );
    }

    return Menu_None;
}

void SaveLoadMenu::Draw( MenuDrawState state )
{
    const char* title = (action == Save) ? "SAVE" : "LOAD";

    for ( int i = 0; i < 4; i++ )
    {
        int index = topRow + i;
        int boxY = BoxY + BoxHeight * i;

        DrawFile( index, boxY );
    }

    Text::DrawBox( 176, 0, 80, 24 );
    Text::DrawString( title, 192, 8 );

    if ( hasError )
        DrawErrorMessage();

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = BoxY + MidLineYOff + BoxHeight * (selIndex - topRow);

        Text::DrawCursor( 0, y );
    }
}

void SaveLoadMenu::DrawFile( int slot, int boxY )
{
    char str[32];
    auto& summary = summaries.Summaries[slot];

    Text::DrawBox( 0, boxY, 256, BoxHeight );

    sprintf_s( str, "%d", slot + 1 );
    Text::DrawString( str, 16, boxY + MidLineYOff );

    if ( summary.State == 0 )
    {
        Text::DrawString( "EMPTY", 48, boxY + MidLineYOff );
        return;
    }

    Text::DrawString( summary.Name, 40, boxY + TopLineYOff );

    for ( int j = 0; j < Players; j++ )
    {
        MainMenu::DrawClass( summary.Classes[j], 96 + 24 * j, boxY + TopLineYOff );
    }

    for ( int r = 0; r < 2; r++ )
    {
        for ( int c = 0; c < 2; c++ )
        {
            int index = r * 2 + c;
            int x = 192 + c * 16;
            int y = boxY + TopLineYOff + r * 16;

            MainMenu::DrawOrb( index, summary.Orbs[index], x, y );
        }
    }

    uint32_t hours = 0;
    uint32_t minutes = 0;
    Global::GetHoursMinutes( summary.TimeMillis, hours, minutes );

    sprintf_s( str, "%02u:%02u", hours, minutes );
    Text::DrawString( str, 40, boxY + BottomLineYOff );

    sprintf_s( str, "%6u G", summary.Gil );
    Text::DrawString( str, 40 + 48, boxY + BottomLineYOff );

    sprintf_s( str, "Lv %u", summary.Level );
    Text::DrawString( str, 40, boxY + MidLineYOff );
}

void SaveLoadMenu::DrawErrorMessage()
{
    const int BoxX = 40;
    const int BoxY = 64;
    const char* Message = "There was an error \nreading or writing \nthe save file.";

    Text::DrawBox( BoxX, BoxY, 176, 112 );
    Text::DrawString( Message, BoxX + 16, BoxY + 32 );
}
