/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Battle.h"


namespace Battle
{
    class BattleMenu : public Menu
    {
        static const int BoxX = 64;
        static const int BoxY = 152;
        static const int TextX = BoxX + 16;
        static const int TextY = BoxY + 16;
        static const int LineHeight = 16;

        int selIndex;

    public:
        BattleMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();

    private:
        MenuAction AcceptAction( Menu*& nextMenu );
    };

    class MagicMenu : public Menu
    {
        static const int BoxX = 8;
        static const int BoxY = 152;
        static const int TextX = BoxX + 16;
        static const int TextY = BoxY + 16;
        static const int ItemX = TextX + 11 * 8;
        static const int LineHeight = 16;
        static const int ItemWidth = 6 * 8;

        int selRow;
        int selCol;
        int topRow;

    public:
        MagicMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();

    private:
        MenuAction AcceptAction( Menu*& nextMenu );
    };

    class ItemMenu : public Menu
    {
        static const int BoxX = 8;
        static const int BoxY = 152;
        static const int TextX = BoxX + 16;
        static const int TextY = BoxY + 16;
        static const int ItemX = TextX;
        static const int LineHeight = 16;
        static const int ItemWidth = 13 * 8;
        static const int UsableItems = 108;
        static const int Rows = UsableItems / 2;

        int         selRow;
        int         selCol;
        int         topRow;
        uint16_t    itemIds[UsableItems];

    public:
        ItemMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();

    private:
        MenuAction AcceptAction( Menu*& nextMenu );
    };

    class ChooseOneEnemyTargetMenu : public Menu
    {
        const EnemyMap* map;
        Point cell;
        int cursorX;
        int cursorY;

    public:
        ChooseOneEnemyTargetMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();

    private:
        void Select( Point cell );
        void SelectDown();
        void SelectUp();
        void SelectRight();
        void SelectLeft();
    };

    class ChooseOnePlayerTargetMenu : public Menu
    {
        int index;
        int cursorX;
        int cursorY;

    public:
        ChooseOnePlayerTargetMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();

    private:
        void Select( int index );
        void SelectNext();
        void SelectPrev();
        int FindNextPlayer( int index );
        int FindPrevPlayer( int index );
    };

    class ChooseSelfTargetMenu : public Menu
    {
        int cursorX;
        int cursorY;

    public:
        ChooseSelfTargetMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();
    };

    class ChooseAllPlayersTargetMenu : public Menu
    {
        int timer;

    public:
        ChooseAllPlayersTargetMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();
    };

    class ChooseAllEnemiesTargetMenu : public Menu
    {
        int timer;

    public:
        ChooseAllEnemiesTargetMenu();
        virtual MenuAction Update( Menu*& nextMenu );
        virtual void Draw();
        virtual void DrawCursor();
    };
}
