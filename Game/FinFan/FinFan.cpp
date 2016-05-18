/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include <allegro5\allegro_audio.h>
#include <allegro5\allegro_acodec.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_primitives.h>
#include "Text.h"
#include "Player.h"
#include "Module.h"
#include "SceneStack.h"
#include "Sound.h"
#include "Config.h"


const double FrameTime = 1 / 60.0;


static ALLEGRO_EVENT_QUEUE* eventQ;
static ALLEGRO_DISPLAY* display;
static int frameCounter;
static int screenScale = 1;


void InitPlayer()
{
    Player::Party[0]._class = 0;
    Player::Party[0].hp = 33;
    Player::Party[0].maxHp = 33;
    Player::Party[0].level = 1;
    Player::Party[0].weaponId = 23;
    Player::Party[0].basicStats[Player::Stat_Strength] = 20;
    Player::Party[0].damage = 10;
    Player::Party[0].hitRate = 10;
    strcpy_s( Player::Party[0].name, "Crono" );

    Player::Party[1]._class = 1;
    Player::Party[1].hp = 135;
    Player::Party[1].maxHp = 135;
    Player::Party[1].level = 1;
    Player::Party[1].weaponId = 23;
    Player::Party[1].basicStats[Player::Stat_Strength] = 5;
    Player::Party[1].damage = 2;
    Player::Party[1].hitRate = 5;
    strcpy_s( Player::Party[1].name, "Verci" );

    Player::Party[2]._class = 2;
    Player::Party[2].hp = 2;
    Player::Party[2].maxHp = 2;
    Player::Party[2].level = 1;
    Player::Party[2].weaponId = 23;
    Player::Party[2].basicStats[Player::Stat_Strength] = 5;
    Player::Party[2].damage = 2;
    Player::Party[2].hitRate = 5;
    strcpy_s( Player::Party[2].name, "Hamil" );

    Player::Party[3]._class = 3;
    Player::Party[3].hp = 457;
    Player::Party[3].maxHp = 457;
    Player::Party[3].level = 1;
    Player::Party[3].weaponId = 23;
    Player::Party[3].basicStats[Player::Stat_Strength] = 10;
    Player::Party[3].damage = 5;
    Player::Party[3].hitRate = 7;
    strcpy_s( Player::Party[3].name, "Aurel" );

    for ( int i = 0; i < 4; i++ )
    {
        for ( int j = 0; j < Player::Armor_Max; j++ )
            Player::Party[i].armorIds[j] = NoArmor;
        Player::CalcDerivedStats( i );
        for ( int j = 0; j < 8; j++ )
        {
            for ( int k = 0; k < 3; k++ )
                Player::Party[i].spells[j][k] = k + 1 + 1;
            Player::Party[i].spellCharge[j] = 3;
            Player::Party[i].spellMaxCharge[j] = 9;
        }
        Player::Party[i].hitMultiplier = 1;
    }

    for ( int i = 0; i < Player::ItemTypes; i++ )
        Player::Items[i] = 0;

    Player::Items[25] = 3;
    Player::Items[26] = 1;
    Player::Items[27] = 2;
    Player::Items[25 + 26] = 1;

    Player::SetVehicles( (Vehicle) (Vehicle_Canoe | Vehicle_Ship | Vehicle_Airship) );
    Player::SetActiveVehicle( Vehicle_Foot );

    Point shipPos = { 152, 169 };
    Player::SetShipRowCol( shipPos );
    Point airshipPos = { 148, 168 };
    Player::SetAirshipRowCol( airshipPos );

    Player::ClearGameFlags();


#if 0
    ALLEGRO_PATH* path = al_get_standard_path( ALLEGRO_USER_DATA_PATH );

    const char* pathStr = al_path_cstr( path, '\\' );
    al_destroy_path( path );

    FILE* file = fopen( "E:\\temp\\save.1", "wb" );
    Player::SaveFile( file );
    fclose( file );

    size_t sChar = sizeof( Player::Character );
#endif
}

int GetFrameCounter()
{
    return frameCounter;
}

int GetScreenScale()
{
    return screenScale;
}

IModule::~IModule()
{
}

static void ResizeView( int screenWidth, int screenHeight )
{
    float viewAspect = StdViewWidth / (float) StdViewHeight;
    float screenAspect = screenWidth / (float) screenHeight;
    // only allow whole number scaling
    int scale = 1;

    if ( viewAspect > screenAspect )
    {
        scale = screenWidth / StdViewWidth;
    }
    else
    {
        scale = screenHeight / StdViewHeight;
    }

    if ( scale <= 0 )
        scale = 1;

    screenScale = scale;

    int viewWidth = StdViewWidth * scale;
    int viewHeight = StdViewHeight * scale;

    // it looks better when the offsets are whole numbers
    int offsetX = (screenWidth - viewWidth) / 2;
    int offsetY = (screenHeight - viewHeight) / 2;

    al_set_clipping_rectangle( offsetX, offsetY, viewWidth, viewHeight );

    ALLEGRO_TRANSFORM t;

    al_identity_transform( &t );
    al_scale_transform( &t, scale, scale );
    al_translate_transform( &t, offsetX, offsetY );
    al_use_transform( &t );
}

static void Run()
{
    bool done = false;
    ALLEGRO_EVENT event = { 0 };
    ALLEGRO_EVENT_SOURCE* keyboardSource = al_get_keyboard_event_source();
    ALLEGRO_EVENT_SOURCE* displaySource = al_get_display_event_source( display );
    double startTime = al_get_time();
    double waitSpan = 0;

    if ( keyboardSource == nullptr )
        return;
    if ( displaySource == nullptr )
        return;

    al_register_event_source( eventQ, keyboardSource );
    al_register_event_source( eventQ, displaySource );

    Global::Init();
    Player::Init();

    SceneStack::SwitchScene( SceneId_Intro );
    SceneStack::PerformSceneChange();

    while ( !done )
    {
        while ( al_wait_for_event_timed( eventQ, &event, waitSpan ) )
        {
            waitSpan = 0;
            if ( event.any.type == ALLEGRO_EVENT_DISPLAY_CLOSE
                || (event.any.type == ALLEGRO_EVENT_KEY_DOWN 
                && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) )
            {
                done = true;
                break;
            }
            else if ( event.any.type == ALLEGRO_EVENT_DISPLAY_RESIZE )
            {
                al_acknowledge_resize( display );

                ResizeView( event.display.width, event.display.height );
            }
        }

        double now = al_get_time();
        bool updated = false;

        while ( (now - startTime) >= FrameTime )
        {
            frameCounter++;

            Input::Update();
            SceneStack::Update();
            Sound::Update();

            startTime += FrameTime;
            updated = true;
        }

        if ( updated )
        {
            SceneStack::Draw();
            al_flip_display();
        }

        double timeLeft = startTime + FrameTime - al_get_time();
        if ( timeLeft >= .002 )
            waitSpan = timeLeft - .001;
        else
            waitSpan = 0;
    }
}

void AdjustForDpi( int& width, int& height )
{
#if _WIN32
    HDC hDC = GetDC( NULL );
    if ( hDC != NULL )
    {
        int dpiX = GetDeviceCaps( hDC, LOGPIXELSX );
        int dpiY = GetDeviceCaps( hDC, LOGPIXELSY );
        ReleaseDC( NULL, hDC );
        width = MulDiv( width, dpiX, 96 );
        height = MulDiv( height, dpiY, 96 );
    }
#endif
}

static bool MakeDisplay()
{
    ALLEGRO_MONITOR_INFO monInfo = { 0 };
    int width = StdViewWidth * 2;
    int height = StdViewHeight * 2;
    int newFlags = ALLEGRO_RESIZABLE;
    bool fullScreen = false;
    bool vsync = false;
    
    Config::GetBool( "fullScreen", fullScreen );

    if ( fullScreen )
    {
        if ( !al_get_monitor_info( 0, &monInfo ) )
            fullScreen = false;
    }

    if ( fullScreen )
    {
        width = monInfo.x2 - monInfo.x1;
        height = monInfo.y2 - monInfo.y1;
        newFlags = ALLEGRO_FULLSCREEN;
    }
    else
    {
        AdjustForDpi( width, height );
    }

    Config::GetBool( "vsync", vsync );

    if ( vsync )
    {
        al_set_new_display_option( ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST );
    }

    al_set_new_display_flags( al_get_new_display_flags() | newFlags );

    display = al_create_display( width, height );
    if ( display == nullptr )
        return false;

    ResizeView( width, height );

    return true;
}

static bool InitAllegro()
{
    if ( !al_init() )
        return false;

    Config::LoadConfig();

    if ( !MakeDisplay() )
        return false;

    if ( !al_install_keyboard() )
        return false;
    if ( !al_install_audio() )
        return false;
    if ( !al_init_image_addon() )
        return false;
    if ( !al_init_acodec_addon() )
        return false;
    if ( !al_init_primitives_addon() )
        return false;

    eventQ = al_create_event_queue();
    if ( eventQ == nullptr )
        return false;

    if ( !Text::Init() )
        return false;
    if ( !Sound::Init() )
        return false;

    return true;
}

static void UninitAllegro()
{
    Sound::Uninit();
    Text::Uninit();

    if ( eventQ != nullptr )
        al_destroy_event_queue( eventQ );

    al_shutdown_primitives_addon();
    al_shutdown_image_addon();
    al_uninstall_audio();
    al_uninstall_keyboard();
    al_destroy_display( display );
    al_uninstall_system();
}

int main( int argc, char** argv )
{
    if ( InitAllegro() )
    {
        Run();
    }

    UninitAllegro();

    return 0;
}
