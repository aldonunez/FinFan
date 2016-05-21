/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Module.h"


typedef Table<char, 30> StoryTable;


class IntroScene : public IModule
{
    static const int TextLines = 13;

    StoryTable storyText;

    const char* lines[TextLines];
    int visibleLines;
    int timer;

public:
    IntroScene();

    void Init();

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static IModule* Make();

private:
    void SplitText( const char* text );
};


class StoryBox
{
    StoryTable storyText;

    int lastPage;
    int page;
    int timer;

public:
    StoryBox( int firstPage, int pageCount );

    void Init();

    bool IsDone();
    void SkipPage();

    void Update();
    void Draw();
};


class OpeningScene : public IModule
{
    StoryBox storyBox;

    ALLEGRO_BITMAP* backPic;

public:
    OpeningScene();

    void Init();

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static IModule* Make();
};


class TheEndAnim
{
    enum
    {
        TheEndProgSize  = 0x400,
        TheEndMaskSize  = 0x50,
        TheEndStartX    = 0xC,
        TheEndStartY    = 0,
    };

    enum State
    {
        Outline,
        Trace,
        FillLine,
        Done,
    };

    ALLEGRO_BITMAP* theEndPic;
    State state;
    int x;
    int y;
    int xfill;
    uint8_t* progPtr;
    uint8_t theEndProg[TheEndProgSize];
    uint8_t theEndMask[TheEndMaskSize];

public:
    TheEndAnim();
    ~TheEndAnim();

    void Init();
    void Update();
    void Draw();

private:
    void MovePen();
    void DrawPixel( int x, int y );

    void UpdateFillLine();
};


class EndingScene : public IModule
{
    StoryBox storyBox;
    TheEndAnim theEnd;

    ALLEGRO_BITMAP* backPic;
    int timer;

public:
    EndingScene();

    void Init();

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static IModule* Make();
};
