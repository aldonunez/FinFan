/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "MainMenu.h"


enum ShopTransaction
{
    Shop_Buy,
    Shop_Sell
};


class ItemShopMenu : public Menu
{
    static ItemShopMenu* instance;

    int shopId;
    int selIndex;
    const char* msg;

public:
    ItemShopMenu( int shopId );
    ~ItemShopMenu();

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    static Menu* Make( int shopId );

    static void SetMessage( int shopMsgId );
};


class BuyItemMenu : public Menu
{
    static const int MaxItems = 5;

    int shopId;
    int selIndex;
    int listLength;
    uint8_t items[MaxItems];

public:
    BuyItemMenu( int shopId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    void DrawEquipUI();
};


class AmountMenu : public Menu
{
    int itemId;
    ShopTransaction transaction;
    int amount;
    int maxAmount;
    int price;

public:
    AmountMenu( int itemId, ShopTransaction transaction );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


class SellItemMenu : public Menu
{
    int topRow;
    int selCol;
    int selRow;
    int rows;

public:
    SellItemMenu( int shopId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


//----------------------------------------------------------------------------


class MagicShopMenu : public Menu
{
    static MagicShopMenu* instance;

    int shopId;
    int selIndex;
    const char* msg;
    int listLength;
    uint8_t spells[SpellsInLevel];

public:
    MagicShopMenu( int shopId );
    ~MagicShopMenu();

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    static Menu* Make( int shopId );

    static void SetMessage( int shopMsgId );
};


class BuySpellMenu : public Menu
{
    int selIndex;
    int listLength;
    const uint8_t* spells;

public:
    BuySpellMenu( const uint8_t* spells, int listLength );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );
};


class LearnerMenu : public Menu
{
    int spellId;
    int selIndex;

public:
    LearnerMenu( int spellId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    bool LearnSpell();
};


//----------------------------------------------------------------------------


class ChurchShopMenu : public Menu
{
    int shopId;
    int price;
    int selIndex;
    int step;
    int playerCount;
    int players[Players];

    char question[256];
    const char* msg;

public:
    ChurchShopMenu( int shopId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    static Menu* Make( int shopId );

private:
    void RevivePlayer();
};


//----------------------------------------------------------------------------


class InnShopMenu : public Menu
{
    int shopId;
    int price;
    int selIndex;
    int step;

    char question[256];
    const char* msg;

public:
    InnShopMenu( int shopId );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    static Menu* Make( int shopId );

private:
    void RestorePlayers();
};
