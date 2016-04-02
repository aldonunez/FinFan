/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class Config
{
public:
    // can only be called after Allegro is initialized
    static bool LoadConfig();

    static bool GetBool( const char* name, bool& value );
};
