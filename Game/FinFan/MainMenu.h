/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Module.h"


enum MenuAction
{
    Menu_None,
    Menu_Push,
    Menu_Pop,
    Menu_SwitchTop,
};

enum MenuDrawState
{
    MenuDraw_Active,
    MenuDraw_Paused,
};

enum ShopType
{
    ShopType_Weapon,
    ShopType_Armor,
    ShopType_WhiteMagic,
    ShopType_BlackMagic,
    ShopType_Church,
    ShopType_Inn,
    ShopType_Item,
    ShopType_Caravan,
};


const int PlayerPicX = 24;
const int PlayerPicY = 16;
const int PlayerEntryHeight = 56;


class Menu
{
public:
    Menu* prevMenu;

    virtual MenuAction Update( Menu*& nextMenu ) = 0;
    virtual void Draw( MenuDrawState state ) = 0;
};


class MainMenu : public IModule
{
    static const int MenuStrings = 64;
    static const int Shops = 71;
    static const int ShopStrings = 38;

    static MainMenu* instance;

    uint8_t itemTarget[(256 + 7) / 8];
    uint8_t magicTarget[(64 + 7) / 8];
    uint8_t shopTypes[Shops];
    ALLEGRO_BITMAP* playerBmp;
    ALLEGRO_BITMAP* menuBmp;
    Table<char, MenuStrings> menuText;
    Table<uint8_t, Shops> shopStock;
    Table<char, ShopStrings> shopText;

    Menu* activeMenu;

public:
    MainMenu();
    ~MainMenu();

    void InitMain();
    void InitShop( int shopId );
    void InitHost( Menu* firstMenu );

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();

    static void DrawMainPlayerInfo();
    static void DrawMainPlayerText();
    static void DrawMainPlayerPic();
    static void DrawPlayer( int playerId, int x, int y );
    static void DrawClass( int classId, int x, int y );
    static void DrawPlayerEquipableAnim( int playerId, int x, int y );
    static void DrawOrb( int index, bool lit, int x, int y );

    static bool ItemTargetsPlayer( int itemId );
    static bool SpellTargetsPlayer( int spellId );
    static const char* GetMenuText( int index );
    static const char* GetShopText( int index );
    static ShopType GetShopType( int shopId );
    static int GetShopItems( int shopId, uint8_t* items, int maxItems );
    static int GetShopPrice( int shopId );

    static void PlayError();
    static void PlayCursor();
    static void PlayConfirm();
    static void PlayFanfare();
    static void PlayPotion();

private:
    void Init();

    void Push( Menu* nextMenu );
    void Pop();
    void PopAll();
};
