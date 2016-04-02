/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "SaveFolder.h"
#include "Player.h"
#include "Ids.h"
#include "Overworld.h"


enum OpenMode
{
    Open_Read,
    Open_Write,
};


const int AlignedFileSize = ((Player::FileSize + 15) / 16) * 16;
const int FullSize = sizeof( SummarySet ) + AlignedFileSize * Slots;

const char* SaveFileName = "ff1.sav";


// The save should probably have a version and checksum
// for compatibility and integrity testing

static errno_t OpenFile( FILE** file, OpenMode mode )
{
    ALLEGRO_PATH* path = nullptr;
    const char* pathStr = nullptr;
    errno_t err = 0;

    path = al_get_standard_path( ALLEGRO_USER_DATA_PATH );
    if ( path == nullptr )
        return EPERM;

    pathStr = al_path_cstr( path, ALLEGRO_NATIVE_PATH_SEP );
    _mkdir( pathStr );

    al_set_path_filename( path, SaveFileName );

    pathStr = al_path_cstr( path, ALLEGRO_NATIVE_PATH_SEP );

    if ( mode == Open_Write )
    {
        err = fopen_s( file, pathStr, "r+b" );
        if ( err == ENOENT )
            err = fopen_s( file, pathStr, "wb" );
    }
    else
    {
        err = fopen_s( file, pathStr, "rb" );
    }

    al_destroy_path( path );

    return err;
}

static bool GetFileSize( FILE* file, int& size )
{
    int err = 0;
    
    err = fseek( file, 0, SEEK_END );
    if ( err != 0 )
        return false;

    size = ftell( file );
    if ( size == -1 )
        return false;

    err = fseek( file, 0, SEEK_SET );
    if ( err != 0 )
        return false;

    return true;
}

static bool InitFile( FILE* file )
{
    int err = 0;

    err = fseek( file, FullSize - 1, SEEK_SET );
    if ( err != 0 )
        return false;

    if ( fputc( 0, file ) == EOF )
        return false;

    err = fseek( file, 0, SEEK_SET );
    if ( err != 0 )
        return false;

    return true;
}

static bool CheckInitFile( FILE* file )
{
    int fileSize = 0;

    if ( !GetFileSize( file, fileSize ) )
        return false;

    if ( fileSize == 0 )
    {
        if ( !InitFile( file ) )
            return false;
    }
    else if ( fileSize != FullSize )
        return false;

    return true;
}

static bool CheckFile( FILE* file )
{
    int fileSize = 0;

    if ( !GetFileSize( file, fileSize ) )
        return false;

    if ( fileSize != FullSize )
        return false;

    return true;
}

static void SetSummary( FileSummary& summary )
{
    const Player::Character& firstPlayer = Player::Party[0];

    summary.State = 1;

    strcpy_s( summary.Name, firstPlayer.name );
    summary.TimeMillis = Global::GetTime();
    summary.Gil = Player::GetG();
    summary.Level = firstPlayer.level;
    summary.MaxHp = firstPlayer.maxHp;

    for ( int i = 0; i < 4; i++ )
        summary.Orbs[i] = Player::Items[Item_First_Orb + i];

    for ( int i = 0; i < Players; i++ )
        summary.Classes[i] = Player::Party[i]._class;
}

static bool SaveOpenFile( int slot, FILE* file )
{
    if ( !CheckInitFile( file ) )
        return false;

    int offset = 0;
    int ret = 0;
    FileSummary summary = { 0 };
        
    SetSummary( summary );

    Point pos = Overworld::GetPlayerPos();
    Player::SetPlayerPos( pos );

    offset = sizeof( FileSummary ) * slot;
    ret = fseek( file, offset, SEEK_SET );
    if ( ret != 0 )
        return false;

    ret = fwrite( &summary, sizeof summary, 1, file );
    if ( ret != 1 )
        return false;

    offset = sizeof( SummarySet ) + (AlignedFileSize * slot);
    ret = fseek( file, offset, SEEK_SET );
    if ( ret != 0 )
        return false;

    Player::SaveFile( file );

    ret = ferror( file );
    if ( ret != 0 )
        return false;

    return true;
}

bool SaveFolder::SaveFile( int slot )
{
    if ( slot < 0 || slot >= Slots )
        return false;

    FILE* file = nullptr;
    errno_t err = 0;
    bool ret = false;
    
    err = OpenFile( &file, Open_Write );
    if ( err != 0 )
        return false;

    ret = SaveOpenFile( slot, file );
    fclose( file );

    return ret;
}

static bool LoadOpenFile( int slot, FILE* file )
{
    if ( !CheckFile( file ) )
        return false;

    int ret = 0;
    int offset = sizeof( SummarySet ) + (AlignedFileSize * slot);

    ret = fseek( file, offset, SEEK_SET );
    if ( ret != 0 )
        return false;

    Player::LoadFile( file );

    ret = ferror( file );
    if ( ret != 0 )
        return false;

    return true;
}

bool SaveFolder::LoadFile( int slot )
{
    if ( slot < 0 || slot >= Slots )
        return false;

    FILE* file = nullptr;
    errno_t err = 0;
    bool ret = false;
    
    err = OpenFile( &file, Open_Read );
    if ( err != 0 )
        return false;

    ret = LoadOpenFile( slot, file );
    fclose( file );

    return ret;
}

bool SaveFolder::ReadSummaries( SummarySet& set )
{
    FILE* file = nullptr;
    errno_t err = 0;
    size_t lenRead = 0;
    
    err = OpenFile( &file, Open_Read );
    if ( err == ENOENT )
    {
        memset( &set, 0, sizeof set );
        return true;
    }

    if ( err != 0 )
        return false;

    lenRead = fread( &set, sizeof set, 1, file );
    if ( lenRead < 1 )
        return false;

    fclose( file );

    return true;
}
