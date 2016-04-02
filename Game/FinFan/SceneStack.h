/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


enum SceneId
{
    SceneId_Intro,
    SceneId_Title,
    SceneId_NewGame,
    SceneId_LoadGame,
    SceneId_Ending,
};


class SceneStack
{
public:
    typedef std::function<void ()> FadeEndProc;

    static void ShowShop( int id );
    static void ShowMenu();
    static void HideMenu();

    static void ShowOpening();
    static void HideOpening();

    static void EnterBattle( int formationId, int backdropId );
    static void LeaveBattle();

    static void PushLevel( int levelId, int startCol, int startRow );
    static void PopLevel();
    static void PopAllLevels();
    static void SwitchToField( int startCol, int startRow );
    static void SwitchScene( SceneId id );

    static void PerformSceneChange();

    static void BeginFade( int frames, ALLEGRO_COLOR startColor, ALLEGRO_COLOR endColor, FadeEndProc p );
    static void EndFade();
    static bool IsFading();
    static bool AtFadeEnd();

    static void Update();
    static void Draw();
};
