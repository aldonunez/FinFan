/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Input.h"


const int LongKeyTimer = 15;
const int ShortKeyTimer = 6;

static ALLEGRO_KEYBOARD_STATE oldKeyboardState;
static ALLEGRO_KEYBOARD_STATE keyboardState;

static bool repeating;
static int repeatingKey;
static int repeatTimer;
static bool signalRepeat;


bool Input::IsKeyDown( int keyCode )
{
    return al_key_down( &keyboardState, keyCode );
}

bool Input::IsKeyPressing( int keyCode )
{
    return GetKey( keyCode ) == KeyState_Pressing;
}

KeyState Input::GetKey( int keyCode )
{
    if ( signalRepeat && repeatingKey == keyCode )
        return KeyState_Pressing;

    int isDown = al_key_down( &keyboardState, keyCode ) ? 1 : 0;
    int wasDown = al_key_down( &oldKeyboardState, keyCode ) ? 1 : 0;

    return (KeyState) ((wasDown << 1) | isDown);
}

Direction Input::GetInputDirection()
{
    if ( IsKeyDown( ALLEGRO_KEY_LEFT ) )
    {
        return Dir_Left;
    }
    else if ( IsKeyDown( ALLEGRO_KEY_RIGHT ) )
    {
        return Dir_Right;
    }
    else if ( IsKeyDown( ALLEGRO_KEY_UP ) )
    {
        return Dir_Up;
    }
    else if ( IsKeyDown( ALLEGRO_KEY_DOWN ) )
    {
        return Dir_Down;
    }

    return Dir_None;
}

void Input::ResetRepeat()
{
    repeating = false;
}

static void Poll()
{
    memcpy( &oldKeyboardState, &keyboardState, sizeof keyboardState );
    al_get_keyboard_state( &keyboardState );
}

void UpdateRepeater()
{
    signalRepeat = false;

    if ( repeating )
    {
        if ( !Input::IsKeyDown( repeatingKey ) )
        {
            repeating = false;
        }
        else
        {
            repeatTimer--;
            if ( repeatTimer == 0 )
            {
                signalRepeat = true;
                repeatTimer = ShortKeyTimer;
            }
        }
    }

    if ( !repeating )
    {
        repeatingKey = 0;

        if ( Input::GetKey( ALLEGRO_KEY_LEFT ) == KeyState_Pressing )
        {
            repeatingKey = ALLEGRO_KEY_LEFT;
        }
        else if ( Input::GetKey( ALLEGRO_KEY_UP ) == KeyState_Pressing )
        {
            repeatingKey = ALLEGRO_KEY_UP;
        }
        else if ( Input::GetKey( ALLEGRO_KEY_RIGHT ) == KeyState_Pressing )
        {
            repeatingKey = ALLEGRO_KEY_RIGHT;
        }
        else if ( Input::GetKey( ALLEGRO_KEY_DOWN ) == KeyState_Pressing )
        {
            repeatingKey = ALLEGRO_KEY_DOWN;
        }

        if ( repeatingKey != 0 )
        {
            repeating = true;
            repeatTimer = LongKeyTimer;
        }
    }
}

void Input::Update()
{
    Poll();
    UpdateRepeater();
}
