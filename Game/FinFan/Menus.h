/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "MainMenu.h"


class ItemMenu : public Menu
{
    int selIndex;

public:
    ItemMenu();

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


class ItemListMenuBase : public Menu
{
    int listLength;
    int startItemId;
    int rows;
    int topRow;
    int selCol;
    int selRow;
    bool drawAmounts;
    const char* itemText;

public:
    ItemListMenuBase( int listLength, int startItemId, bool drawAmounts );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    MenuAction UseItem( Menu*& nextMenu );
};


class UseItemMenu : public ItemListMenuBase
{
public:
    UseItemMenu();
};


class KeyItemMenu : public ItemListMenuBase
{
public:
    KeyItemMenu();
};


class ChooseItemTargetMenu : public Menu
{
    int itemId;
    int selIndex;
    int timer;

public:
    ChooseItemTargetMenu( int itemId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    void UseItem();
};


//----------------------------------------------------------------------------


class StatusMenu : public Menu
{
    int playerId;

public:
    StatusMenu( int playerId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


//----------------------------------------------------------------------------


class EquipMenu : public Menu
{
    int playerId;
    int selIndex;

public:
    EquipMenu( int playerId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


enum EquipAction
{
    Equip_Equip,
    Equip_Remove,
};


class ChooseEquipSlotMenu : public Menu
{
    int playerId;
    int selIndex;
    EquipAction action;

public:
    ChooseEquipSlotMenu( int playerId, EquipAction action );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    bool RemoveItem();
};


class ChooseEquipItemMenu : public Menu
{
    static const int VisibleRows = 6;
    static const int Weapons = 40;
    static const int Armors = 40;
    static const int MaxItems = Weapons > Armors ? Weapons : Armors;

    int playerId;
    int mergedSlot;
    int selRow;
    int topRow;
    int listLength;
    uint8_t items[MaxItems];

public:
    ChooseEquipItemMenu( int playerId, int mergedSlot );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    int GetListLength();

private:
    void FillWeapons();
    void FillArmors();
    void EquipSelectedItem();
};


//----------------------------------------------------------------------------


enum MagicAction
{
    Magic_Use,
    Magic_Discard,
};


class MagicMenu : public Menu
{
    int playerId;
    int selIndex;

public:
    MagicMenu( int playerId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


class ChooseMagicMenu : public Menu
{
    static const int Cols = 3;
    static const int Rows = 8;

    int playerId;
    MagicAction action;
    int selCol;
    int selRow;

public:
    ChooseMagicMenu( int playerId, MagicAction action );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    MenuAction CastSpell( Menu*& nextMenu );
};


class ChooseMagicTargetMenu : public Menu
{
    int casterId;
    int spellId;
    int selIndex;
    int timer;

public:
    ChooseMagicTargetMenu( int casterId, int spellId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    void CastSpell();
};


class ConfirmDiscardMenu : public Menu
{
    int playerId;
    int level;
    int col;
    int selIndex;

public:
    ConfirmDiscardMenu( int playerId, int level, int col );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    void DiscardMagic();
};


class CommandMenu : public Menu
{
    static CommandMenu* instance;

    int selIndex;
    bool showPics;

public:
    CommandMenu();

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    static void ShowPics( bool visible );
};


class ChoosePartyMenu : public Menu
{
    int step;
    int selIndex;
    int firstSelIndex;

public:
    ChoosePartyMenu();

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


class AnimateSwitchPartyMenu : public Menu
{
    int step;
    int playerId1;
    int playerId2;
    int xOffset;
    int yOffset;
    int targetYOffset;

public:
    AnimateSwitchPartyMenu( int playerId1, int playerId2 );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


class ChooseCommandTargetMenu : public Menu
{
    int cmdId;
    int selIndex;

public:
    ChooseCommandTargetMenu( int cmdId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};
