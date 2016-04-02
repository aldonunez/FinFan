/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "SceneStack.h"
#include "Module.h"
#include "MainMenu.h"
#include "BattleMod.h"
#include "Overworld.h"
#include "Level.h"
#include "Title.h"
#include "StoryScenes.h"
#include <allegro5\allegro_primitives.h>


enum SceneAction
{
    Scene_None,
    Scene_SwitchScene,
    Scene_SwitchToField,
    Scene_PushLevel,
    Scene_PopLevel,
    Scene_PopAllLevels,
};

struct SceneChange
{
    SceneAction Action;
    int Col;
    int Row;
    int Level;
};

struct SceneFrame
{
    int8_t  Level;
    uint8_t Col;
    uint8_t Row;
    uint8_t InRoom;
};

typedef IModule* (*SceneMaker)();


SceneChange pendingScene;

SceneFrame stack[100];
int stackLength;
int curLevelId;
bool inShop;

IModule* curScene;
IModule* curOverlay;

// make sure this list matches SceneId enum

SceneMaker sceneMakers[] = 
{
    IntroScene::Make,
    TitleScene::Make,
    NewGameScene::Make,
    SaveLoadScene::MakeLoad,
    EndingScene::Make,
};

bool fade;
int fadeTimer;
int fadeFrames;
ALLEGRO_COLOR startColor;
ALLEGRO_COLOR endColor;
SceneStack::FadeEndProc fadeEndProc;


void SceneStack::ShowShop( int id )
{
    if ( curOverlay == nullptr )
    {
        MainMenu* mainMenu = new MainMenu();
        mainMenu->InitShop( id );
        inShop = true;
        curOverlay = mainMenu;
    }
}

void SceneStack::ShowMenu()
{
    if ( curOverlay == nullptr )
    {
        MainMenu* mainMenu = new MainMenu();
        mainMenu->InitMain();
        inShop = false;
        curOverlay = mainMenu;
    }
}

void SceneStack::HideMenu()
{
    delete curOverlay;
    curOverlay = nullptr;

    if ( curScene != nullptr )
    {
        IPlayfield* playfield = curScene->AsPlayfield();

        if ( playfield != nullptr && inShop )
            playfield->HandleShopClosed();
    }
}

void SceneStack::ShowOpening()
{
    if ( curOverlay == nullptr )
    {
        curOverlay = OpeningScene::Make();
    }
}

void SceneStack::HideOpening()
{
    delete curOverlay;
    curOverlay = nullptr;

    if ( curScene != nullptr )
    {
        IPlayfield* playfield = curScene->AsPlayfield();

        if ( playfield != nullptr )
            playfield->HandleOpeningEnded();
    }
}

void SceneStack::EnterBattle( int formationId, int backdropId )
{
    if ( curOverlay == nullptr )
    {
        BattleMod* battle = new BattleMod();
        battle->Init( formationId, backdropId );
        curOverlay = battle;
    }
}

void SceneStack::LeaveBattle()
{
    delete curOverlay;
    curOverlay = nullptr;

    if ( curScene != nullptr )
    {
        IPlayfield* playfield = curScene->AsPlayfield();

        if ( playfield != nullptr )
            playfield->HandleFightEnded();
    }
}

void SceneStack::PushLevel( int levelId, int startCol, int startRow )
{
    pendingScene.Action = Scene_PushLevel;
    pendingScene.Level = levelId;
    pendingScene.Col = startCol;
    pendingScene.Row = startRow;
}

void SceneStack::PopLevel()
{
    pendingScene.Action = Scene_PopLevel;
}

void SceneStack::PopAllLevels()
{
    pendingScene.Action = Scene_PopAllLevels;
}

void SceneStack::SwitchToField( int startCol, int startRow )
{
    pendingScene.Action = Scene_SwitchToField;
    pendingScene.Col = startCol;
    pendingScene.Row = startRow;
}

void SceneStack::SwitchScene( SceneId id )
{
    pendingScene.Action = Scene_SwitchScene;
    pendingScene.Level = id;
}

static void PushLevel()
{
    int inRoom = 0;
    Point pos = { 0 };

    if ( curScene != nullptr && stackLength < _countof( stack ) )
    {
        IPlayfield* playfield = curScene->AsPlayfield();

        if ( playfield != nullptr )
        {
            inRoom = playfield->GetInRoom();
            pos = playfield->GetCurrentPos();

            stack[stackLength].Level = curLevelId;
            stack[stackLength].Col = pos.X;
            stack[stackLength].Row = pos.Y;
            stack[stackLength].InRoom = inRoom;
            stackLength++;
        }
    }

    delete curOverlay;
    curOverlay = nullptr;

    delete curScene;
    curScene = nullptr;

    Level* level = new Level();
    level->Init( pendingScene.Level, pendingScene.Col, pendingScene.Row, inRoom );

    curScene = level;
    curLevelId = pendingScene.Level;
}

static void PopLevel()
{
    if ( stackLength <= 0 )
        return;

    delete curOverlay;
    curOverlay = nullptr;

    delete curScene;
    curScene = nullptr;

    stackLength--;
    SceneFrame& frame = stack[stackLength];

    if ( frame.Level == -1 )
    {
        Overworld* field = new Overworld();
        field->Init( frame.Col, frame.Row );

        curScene = field;
        curLevelId = -1;
    }
    else
    {
        Level* level = new Level();
        level->Init( 
            frame.Level, 
            frame.Col, 
            frame.Row, 
            frame.InRoom );

        curScene = level;
        curLevelId = frame.Level;
    }
}

static void PopAllLevels()
{
    if ( stackLength > 1 )
        stackLength = 1;

    PopLevel();
}

static void SwitchToField()
{
    delete curOverlay;
    curOverlay = nullptr;

    delete curScene;
    curScene = nullptr;

    Overworld* field = new Overworld();
    field->Init( pendingScene.Col, pendingScene.Row );

    curScene = field;
    curLevelId = -1;
    stackLength = 0;
}

static IModule* MakeScene( int id )
{
    if ( id < 0 || id >= _countof( sceneMakers ) )
    {
        return TitleScene::Make();
    }

    SceneMaker maker = sceneMakers[id];

    return maker();
}

static void SwitchScene()
{
    delete curOverlay;
    curOverlay = nullptr;

    delete curScene;
    curScene = nullptr;

    IModule* scene = MakeScene( pendingScene.Level );

    curScene = scene;
    curLevelId = -1;
    stackLength = 0;
}

void SceneStack::PerformSceneChange()
{
    if ( pendingScene.Action != Scene_None )
    {
        // scene changes override fades
        fade = false;
    }

    switch ( pendingScene.Action )
    {
    case Scene_SwitchScene:
        ::SwitchScene();
        break;

    case Scene_SwitchToField:
        ::SwitchToField();
        break;

    case Scene_PushLevel:
        ::PushLevel();
        break;

    case Scene_PopLevel:
        ::PopLevel();
        break;

    case Scene_PopAllLevels:
        ::PopAllLevels();
        break;
    }

    pendingScene.Action = Scene_None;
}

static void UpdateFade()
{
    if ( !fade )
        return;

    if ( fadeTimer < fadeFrames )
        fadeTimer++;

    if ( SceneStack::AtFadeEnd() )
    {
        if ( fadeEndProc )
        {
            SceneStack::EndFade();
            fadeEndProc();
            SceneStack::PerformSceneChange();
        }
    }
}

static void DrawFade()
{
    if ( !fade )
        return;

    ALLEGRO_COLOR fadeColor;

    float factor = fadeTimer / (float) fadeFrames;
    fadeColor = al_map_rgba_f( 
        startColor.r + (endColor.r - startColor.r) * factor, 
        startColor.g + (endColor.g - startColor.g) * factor, 
        startColor.b + (endColor.b - startColor.b) * factor, 
        startColor.a + (endColor.a - startColor.a) * factor );

    al_draw_filled_rectangle( 0, 0, StdViewWidth, StdViewHeight, fadeColor );
}

void SceneStack::BeginFade( int frames, ALLEGRO_COLOR startColor, ALLEGRO_COLOR endColor, FadeEndProc p )
{
    fade = true;
    fadeTimer = 0;
    fadeFrames = frames;
    ::startColor = startColor;
    ::endColor = endColor;
    fadeEndProc = p;
}

void SceneStack::EndFade()
{
    fade = false;
}

bool SceneStack::IsFading()
{
    return fade;
}

bool SceneStack::AtFadeEnd()
{
    return fadeTimer >= fadeFrames;
}

void SceneStack::Update()
{
    if ( curOverlay != nullptr )
        curOverlay->Update();
    else
        curScene->Update();

    PerformSceneChange();
    UpdateFade();
}

void SceneStack::Draw()
{
    if ( curOverlay != nullptr )
        curOverlay->Draw();
    else
        curScene->Draw();

    DrawFade();
}
