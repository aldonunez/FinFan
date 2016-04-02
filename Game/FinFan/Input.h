/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum KeyState
{
    KeyState_Lifted     = 0,
    KeyState_Pressing   = 1,
    KeyState_Releasing  = 2,
    KeyState_Held       = 3,
};


class Input
{
public:
    static bool IsKeyDown( int keyCode );
    static bool IsKeyPressing( int keyCode );

    static KeyState GetKey( int keyCode );
    static Direction GetInputDirection();

    static void ResetRepeat();

    static void Update();
};
