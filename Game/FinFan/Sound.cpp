/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Sound.h"
#include <allegro5\allegro_audio.h>


const int Streams = 4;
const int UserStreams = 2;
const int Songs = 24;


struct LoopPoints
{
    // the loop beginning and end points in frames (1/60 seconds)
    int16_t Begin;
    int16_t End;
};


static ALLEGRO_VOICE* defaultVoice;
static ALLEGRO_MIXER* defaultMixer;
static ALLEGRO_AUDIO_STREAM* streams[Streams];
static double savedPos[UserStreams];
static ALLEGRO_SAMPLE_INSTANCE* defaultInstance;
static ALLEGRO_SAMPLE* effectSamples[SEffect_Max];
static LoopPoints songLoops[Songs];

static const char* songFiles[] = 
{
    "01_prelude.wav",
    "02_opening.wav",
    "03_ending.wav",
    "04_field.wav",
    "05_ship.wav",
    "06_airship.wav",
    "07_town.wav",
    "08_castle.wav",
    "09_volcano.wav",
    "10_matoya.wav",
    "11_dungeon.wav",
    "12_temple.wav",
    "13_sky.wav",
    "14_sea_shrine.wav",
    "15_shop.wav",
    "16_battle.wav",
    "17_menu.wav",
    "18_dead.wav",
    "19_victory.wav",
    "20_fanfare.wav",
    "21_unknown.wav",
    "22_save.wav",
    "23_unknown.wav",
    "24_chaos_rumble.wav",
};

static const char* effectFiles[SEffect_Max] = 
{
    "ff1-sfx-confirm.wav",
    "ff1-sfx-cursor.wav",
    "ff1-sfx-door.wav",
    "ff1-sfx-error.wav",
    "ff1-sfx-fight.wav",
    "ff1-sfx-hurt.wav",
    "ff1-sfx-magic.wav",
    "ff1-sfx-potion.wav",
    "ff1-sfx-step.wav",
    "ff1-sfx-strike.wav",
    "ff1-sfx-ship.wav",
    "ff1-sfx-lift.wav",
    "ff1-sfx-land.wav",
    "ff1-sfx-airship.wav",
    "ff1-sfx-lava.wav",
};


static void PlayTrackInternal( int trackId, int streamId, bool loop, bool play )
{
    al_destroy_audio_stream( streams[streamId] );

    streams[streamId] = al_load_audio_stream( songFiles[trackId], 2, 2048 );
    if ( streams[streamId] == nullptr )
        return;

    if ( !al_attach_audio_stream_to_mixer( streams[streamId], defaultMixer ) )
    {
        al_destroy_audio_stream( streams[streamId] );
        streams[streamId] = nullptr;
        return;
    }

    ALLEGRO_PLAYMODE playMode = ALLEGRO_PLAYMODE_ONCE;

    if ( loop )
    {
        playMode = ALLEGRO_PLAYMODE_LOOP;

        if ( songLoops[trackId].End >= 0 )
        {
            double beginSecs = 0;
            double endSecs = songLoops[trackId].End * (1 / 60.0);

            if ( songLoops[trackId].Begin >= 0 )
                beginSecs = songLoops[trackId].Begin * (1 / 60.0);

            al_set_audio_stream_loop_secs( streams[streamId], beginSecs, endSecs );
        }
    }

    al_set_audio_stream_playmode( streams[streamId], playMode );
    al_set_audio_stream_playing( streams[streamId], play );
}

bool Sound::Init()
{
    defaultVoice = al_create_voice( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );
    if ( defaultVoice == nullptr )
        return false;

    defaultMixer = al_create_mixer( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );
    if ( defaultMixer == nullptr )
        return false;

    if ( !al_attach_mixer_to_voice( defaultMixer, defaultVoice ) )
        return false;

    defaultInstance = al_create_sample_instance( nullptr );
    if ( defaultInstance == nullptr )
        return false;

    if ( !al_attach_sample_instance_to_mixer( defaultInstance, defaultMixer ) )
        return false;

    for ( int i = 0; i < SEffect_Max; i++ )
    {
        effectSamples[i] = al_load_sample( effectFiles[i] );
        if ( effectSamples[i] == nullptr )
            return false;
    }

    if ( !LoadList( "loopPoints.dat", songLoops, Songs ) )
        return false;

    return true;
}

void Sound::Uninit()
{
    for ( int i = 0; i < SEffect_Max; i++ )
    {
        al_destroy_sample( effectSamples[i] );
    }

    al_destroy_sample_instance( defaultInstance );

    for ( int i = 0; i < Streams; i++ )
    {
        al_destroy_audio_stream( streams[i] );
    }

    al_destroy_mixer( defaultMixer );
    al_destroy_voice( defaultVoice );
}

void Sound::Update()
{
    for ( int i = UserStreams; i < Streams; i++ )
    {
        if ( streams[i] == nullptr
            || al_get_audio_stream_playing( streams[i] ) )
            continue;

        int loPriStream = i - UserStreams;

        al_destroy_audio_stream( streams[i] );
        streams[i] = nullptr;

        if ( streams[loPriStream] != nullptr )
        {
            al_seek_audio_stream_secs( streams[loPriStream], savedPos[loPriStream] );
            al_set_audio_stream_playing( streams[loPriStream], true ); 
        }
    }
}

void Sound::PlayTrack( int trackId, int streamId, bool loop )
{
    if ( streamId < 0 || streamId >= UserStreams )
        return;
    if ( trackId < 0 || trackId >= _countof( songFiles ) )
        return;

    int hiPriStream = UserStreams + streamId;

    if ( streams[hiPriStream] == nullptr || !al_get_audio_stream_playing( streams[hiPriStream] ) )
    {
        PlayTrackInternal( trackId, streamId, loop, true );
        return;
    }

    PlayTrackInternal( trackId, streamId, loop, false );
    savedPos[streamId] = 0;
}

void Sound::PushTrack( int trackId, int streamId )
{
    if ( streamId < 0 || streamId >= UserStreams )
        return;
    if ( trackId < 0 || trackId >= _countof( songFiles ) )
        return;

    if ( streams[streamId] != nullptr && al_get_audio_stream_playing( streams[streamId] ) )
    {
        savedPos[streamId] = al_get_audio_stream_position_secs( streams[streamId] );
        al_set_audio_stream_playing( streams[streamId], false );
    }

    int hiPriStream = UserStreams + streamId;

    PlayTrackInternal( trackId, hiPriStream, false, true );
}

void Sound::PlayEffect( int id, bool loop )
{
    if ( id < 0 || id >= _countof( effectSamples ) )
        return;

    al_set_sample( defaultInstance, effectSamples[id] );

    ALLEGRO_PLAYMODE playMode = ALLEGRO_PLAYMODE_ONCE;

    if ( loop )
        playMode = ALLEGRO_PLAYMODE_LOOP;

    al_set_sample_instance_playmode( defaultInstance, playMode );
    al_play_sample_instance( defaultInstance );
}

void Sound::StopEffect()
{
    al_stop_sample_instance( defaultInstance );
}
