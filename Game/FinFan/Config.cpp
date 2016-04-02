/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Config.h"


const char* ConfigFileName = "ff1.ini";


static ALLEGRO_CONFIG* config;


static ALLEGRO_PATH* GetConfigPath()
{
    ALLEGRO_PATH* path = nullptr;

    path = al_get_standard_path( ALLEGRO_USER_DATA_PATH );
    if ( path == nullptr )
        return nullptr;

    al_set_path_filename( path, ConfigFileName );

    return path;
}


bool Config::LoadConfig()
{
    // already loaded?
    if ( config != nullptr )
        return true;

    ALLEGRO_PATH* path = GetConfigPath();

    if ( path != nullptr )
    {
        const char* pathStr = al_path_cstr( path, ALLEGRO_NATIVE_PATH_SEP );
        config = al_load_config_file( pathStr );
        al_destroy_path( path );
    }

    if ( config == nullptr )
        return false;

    return true;
}

bool Config::GetBool( const char* name, bool& value )
{
    if ( config == nullptr )
        return false;

    const char* rawVal = al_get_config_value( config, nullptr, name );

    if ( rawVal == nullptr )
        return false;

    // keep in mind this allows things that aren't numbers
    value = atoi( rawVal ) != 0;
    return true;
}
