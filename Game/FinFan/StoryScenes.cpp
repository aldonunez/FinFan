/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "StoryScenes.h"
#include "SceneStack.h"
#include "Sound.h"
#include "Text.h"


//----------------------------------------------------------------------------
//  IntroScene
//----------------------------------------------------------------------------

const int LineFrames = 60;
const int LinesX = 8;
const int LinesY = 24;


IntroScene::IntroScene()
    :   visibleLines( 0 ),
        timer( LineFrames )
{
    SplitText( "" );
}

void IntroScene::Init()
{
    if ( !LoadResource( "storyText.tab", &storyText ) )
        return;

    const char* introText = storyText.GetItem( 29 );

    SplitText( introText );

    Sound::PlayTrack( Sound_Prelude, 0, true );
}

void IntroScene::Update()
{
    if ( visibleLines < TextLines )
    {
        if ( timer == 0 )
        {
            visibleLines++;
            timer = LineFrames;
        }
        else
        {
            timer--;
        }
    }

    if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        SceneStack::SwitchScene( SceneId_Title );
    }
}

void IntroScene::Draw()
{
    ALLEGRO_COLOR backColor = al_map_rgb( 0, 0, 255 );

    al_clear_to_color( backColor );

    for ( int i = 0; i < visibleLines; i++ )
    {
        Text::DrawDialogString( lines[i], LinesX, LinesY + i * 16, "" );
    }

    if ( visibleLines < TextLines )
    {
        int j = visibleLines;
        int alpha = (LineFrames - timer) * 255 / LineFrames;
        ALLEGRO_COLOR color = al_map_rgba( alpha, alpha, alpha, alpha );

        Text::DrawDialogString( lines[j], LinesX, LinesY + j * 16, "", color );
    }
}

IPlayfield* IntroScene::AsPlayfield()
{
    return nullptr;
}

IModule* IntroScene::Make()
{
    IntroScene* mod = new IntroScene();
    mod->Init();
    return mod;
}

void IntroScene::SplitText( const char* text )
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


//----------------------------------------------------------------------------
//  StoryBox
//----------------------------------------------------------------------------

const int PageFrames = 60 * 10;


StoryBox::StoryBox( int firstPage, int pageCount )
    :   page( firstPage ),
        lastPage( firstPage + pageCount ),
        timer( PageFrames )
{
}

void StoryBox::Init()
{
    if ( !LoadResource( "storyText.tab", &storyText ) )
        return;
}

bool StoryBox::IsDone()
{
    return page == lastPage;
}

void StoryBox::SkipPage()
{
    timer = 0;
}

void StoryBox::Update()
{
    if ( timer == 0 )
    {
        if ( page < lastPage )
        {
            page++;
            timer = PageFrames;
        }
    }
    else
    {
        timer--;
    }
}

void StoryBox::Draw()
{
    if ( page == lastPage )
        return;

    const char* pageText = storyText.GetItem( page );
    int alpha = 255;
    ALLEGRO_COLOR color;

    if ( timer > PageFrames - LineFrames )
    {
        int value = (PageFrames - timer);
        alpha = value * 255 / LineFrames;
    }

    color = al_map_rgba( alpha, alpha, alpha, alpha );

    Text::DrawString( pageText, Text::FontA, 40, 48, color );
}


//----------------------------------------------------------------------------
//  OpeningScene
//----------------------------------------------------------------------------

const int OpeningMessages = 8;


OpeningScene::OpeningScene()
    :   backPic( nullptr ),
        storyBox( 0, OpeningMessages )
{
}

void OpeningScene::Init()
{
    storyBox.Init();

    backPic = al_load_bitmap( "opening.png" );
    if ( backPic == nullptr )
        return;

    Sound::PlayTrack( Sound_Opening, 0, true );

    SceneStack::BeginFade( 60, Color::White(), Color::Transparent(), [] {} );
}

void OpeningScene::Update()
{
    if ( SceneStack::IsFading() )
        return;

    storyBox.Update();

    if ( Input::IsKeyPressing( ConfirmKey ) )
        storyBox.SkipPage();

    if ( storyBox.IsDone() )
        SceneStack::BeginFade( 60, Color::Transparent(), Color::White(), 
            [] { SceneStack::HideOpening(); } );
}

void OpeningScene::Draw()
{
    al_draw_bitmap( backPic, 0, 0, 0 );

    storyBox.Draw();
}

IPlayfield* OpeningScene::AsPlayfield()
{
    return nullptr;
}

IModule* OpeningScene::Make()
{
    OpeningScene* mod = new OpeningScene();
    mod->Init();
    return mod;
}


//----------------------------------------------------------------------------
//  TheEndAnim
//----------------------------------------------------------------------------

TheEndAnim::TheEndAnim()
    :   theEndPic( nullptr ),
        state( Outline ),
        x( TheEndStartX ),
        y( TheEndStartY ),
        xfill( 0 ),
        progPtr( theEndProg )
{
}

TheEndAnim::~TheEndAnim()
{
    if ( theEndPic != nullptr )
    {
        al_destroy_bitmap( theEndPic );
        theEndPic = nullptr;
    }
}

void TheEndAnim::Init()
{
    if ( !LoadList( "theEndProg.dat", theEndProg, TheEndProgSize ) )
        return;

    if ( !LoadList( "theEndMask.dat", theEndMask, TheEndMaskSize ) )
        return;

    theEndPic = al_create_bitmap( 80, 80 );
    if ( theEndPic == nullptr )
        return;
}

void TheEndAnim::Update()
{
    ALLEGRO_BITMAP* origBmp = al_get_target_bitmap();
    al_set_target_bitmap( theEndPic );

    if ( state == Outline )
    {
        if ( *progPtr == 0 )
        {
            state = Trace;
            progPtr = theEndProg;
            x = TheEndStartX;
            y = TheEndStartY;
        }
        else
        {
            MovePen();
            DrawPixel( x, y );
        }
    }
    else if ( state == Trace )
    {
        if ( *progPtr == 0 )
        {
            state = Done;
        }
        else
        {
            int b = *progPtr;

            if ( (theEndMask[y] != 0)
                && (b == 2 || b == 3 || b == 7) )
            {
                xfill = x;
                state = FillLine;
            }
            else
            {
                MovePen();
            }
        }
    }
    else if ( state == FillLine )
    {
        xfill--;

        if ( xfill < 0 || al_get_pixel( theEndPic, xfill, y ).a != 0 )
        {
            state = Trace;
            MovePen();
        }
        else
        {
            DrawPixel( xfill, y );
        }
    }

    al_set_target_bitmap( origBmp );
}

void TheEndAnim::Draw()
{
    al_draw_bitmap( theEndPic, 64, 40, 0 );
}

void TheEndAnim::MovePen()
{
    int b = *progPtr;

    if ( (b & 8) != 0 )
        y--;
    else if ( (b & 2) != 0 )
        y++;

    if ( (b & 4) != 0 )
        x--;
    else if ( (b & 1) != 0 )
        x++;

    progPtr++;
}

void TheEndAnim::DrawPixel( int x, int y )
{
    al_put_pixel( x + 0.5, y + 0.5, Color::White() );
}


//----------------------------------------------------------------------------
//  EndingScene
//----------------------------------------------------------------------------

const int EndingMessages = 21;


EndingScene::EndingScene()
    :   backPic( nullptr ),
        storyBox( 8, EndingMessages ),
        timer( 120 )
{
}

void EndingScene::Init()
{
    storyBox.Init();
    theEnd.Init();

    backPic = al_load_bitmap( "ending.png" );
    if ( backPic == nullptr )
        return;

    Sound::PlayTrack( Sound_Ending, 0, false );
}

void EndingScene::Update()
{
    if ( timer != 0 )
    {
        timer--;
        return;
    }

    storyBox.Update();

    if ( storyBox.IsDone() )
        theEnd.Update();
}

void EndingScene::Draw()
{
    if ( timer != 0 )
    {
        al_clear_to_color( Color::Black() );
        return;
    }

    al_draw_bitmap( backPic, 0, 0, 0 );

    storyBox.Draw();

    if ( storyBox.IsDone() )
        theEnd.Draw();
}

IPlayfield* EndingScene::AsPlayfield()
{
    return nullptr;
}

IModule* EndingScene::Make()
{
    EndingScene* mod = new EndingScene();
    mod->Init();
    return mod;
}
