/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "MainMenu.h"
#include "SaveFolder.h"


class SaveLoadMenu : public Menu
{
public:
    enum Action
    {
        Save,
        Load,
    };

private:
    Action action;
    int selIndex;
    int topRow;
    SummarySet summaries;
    bool hasError;

public:
    SaveLoadMenu( Action action );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    MenuAction Close();
    void DrawFile( int slot, int boxY );
    void DrawErrorMessage();
};
