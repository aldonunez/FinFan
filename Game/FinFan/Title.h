/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Module.h"
#include "MainMenu.h"


class TitleScene : public IModule
{
    int selIndex;

public:
    TitleScene();

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static IModule* Make();
};


class NewGameScene : public IModule
{
    MainMenu mainMenu;

public:
    NewGameScene();

    void Init();

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static IModule* Make();
};


class SaveLoadScene : public IModule
{
    MainMenu mainMenu;

public:
    SaveLoadScene();

    void InitSave();
    void InitLoad();

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static IModule* MakeSave();
    static IModule* MakeLoad();
};
