/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "MainMenu.h"
#include "Menus.h"
#include "ShopMenus.h"
#include "Text.h"
#include "Player.h"
#include "SceneStack.h"
#include "Sound.h"


typedef Menu* (*MenuMaker)( int shopId );


MenuMaker makers[] = 
{
    ItemShopMenu::Make,
    ItemShopMenu::Make,
    MagicShopMenu::Make,
    MagicShopMenu::Make,
    ChurchShopMenu::Make,
    InnShopMenu::Make,
    ItemShopMenu::Make,
    ItemShopMenu::Make,
};

MainMenu* MainMenu::instance;


MainMenu::MainMenu()
    :   activeMenu( nullptr ),
        playerBmp( nullptr ),
        menuBmp( nullptr )
{
    instance = this;
}

MainMenu::~MainMenu()
{
    instance = nullptr;

    PopAll();

    al_destroy_bitmap( menuBmp );
    al_destroy_bitmap( playerBmp );
}

void MainMenu::Init()
{
    if ( !LoadList( "itemTarget.dat", itemTarget, _countof( itemTarget ) ) )
        return;

    if ( !LoadList( "magicTarget.dat", magicTarget, _countof( magicTarget ) ) )
        return;

    if ( !LoadResource( "menuText.tab", &menuText ) )
        return;

    if ( !LoadList( "shopTypes.dat", shopTypes, _countof( shopTypes ) ) )
        return;

    if ( !LoadResource( "shopStock.tab", &shopStock ) )
        return;

    if ( !LoadResource( "shopText.tab", &shopText ) )
        return;

    playerBmp = al_load_bitmap( "playerSprites.png" );
    if ( playerBmp == nullptr )
        return;

    menuBmp = al_load_bitmap( "menu.png" );
    if ( menuBmp == nullptr )
        return;
}

void MainMenu::InitMain()
{
    Init();

    activeMenu = new CommandMenu();
    activeMenu->prevMenu = nullptr;
}

void MainMenu::InitShop( int shopId )
{
    Init();

    if ( shopId < 0 || shopId >= Shops )
        return;

    int shopType = shopTypes[shopId];
    MenuMaker maker = makers[shopType];

    activeMenu = maker( shopId );
    activeMenu->prevMenu = nullptr;

    Sound::PlayTrack( Sound_Shop, 0, true );
}

void MainMenu::InitHost( Menu* firstMenu )
{
    Init();

    // MainMenu takes ownership of this menu

    activeMenu = firstMenu;
    activeMenu->prevMenu = nullptr;
}

IPlayfield* MainMenu::AsPlayfield()
{
    return nullptr;
}

void MainMenu::Push( Menu* nextMenu )
{
    if ( nextMenu != nullptr )
    {
        nextMenu->prevMenu = activeMenu;
        activeMenu = nextMenu;
    }
}

void MainMenu::Pop()
{
    Menu* menu = activeMenu;
    activeMenu = activeMenu->prevMenu;
    delete menu;
}

void MainMenu::PopAll()
{
    while ( activeMenu != nullptr )
    {
        Pop();
    }
}

void MainMenu::Update()
{
    MenuAction action = Menu_None;
    Menu* nextMenu = nullptr;

    action = activeMenu->Update( nextMenu );

    switch ( action )
    {
    case Menu_Pop:
        Pop();
        if ( activeMenu == nullptr )
            SceneStack::HideMenu();
        break;

    case Menu_Push:
        Push( nextMenu );
        break;

    case Menu_SwitchTop:
        PopAll();
        Push( nextMenu );
        break;
    }
}

void MainMenu::Draw()
{
    al_clear_to_color( al_map_rgb( 0, 0, 0 ) );

    activeMenu->Draw( MenuDraw_Active );
}

void MainMenu::DrawMainPlayerInfo()
{
    DrawMainPlayerText();
    DrawMainPlayerPic();
}

void MainMenu::DrawMainPlayerText()
{
    const int TextX = PlayerPicX + 32;
    char str[32];

    Text::DrawBox( 0, 0, 176, 240 );

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Player::Character& player = Player::Party[i];
        int y = PlayerPicY + PlayerEntryHeight * i;

        Text::DrawString( player.name, PlayerPicX + 32, y );

        if ( (player.status & Status_Death) != 0 )
            strcpy_s( str, "Swoon" );
        else if ( (player.status & Status_Stone) != 0 )
            strcpy_s( str, "Stone" );
        else if ( (player.status & Status_Poison) != 0 )
            strcpy_s( str, "Poison" );
        else
            sprintf_s( str, "Lv %2d", player.level );

        Text::DrawString( str, TextX + 7*8, y + 0 );

        sprintf_s( str, "HP %3d/%3d", player.hp, player.maxHp );

        Text::DrawString( str, TextX + 2*8, y + 16 );

        sprintf_s( str, "MP %d/%d/%d/%d", 
            player.spellCharge[0],
            player.spellCharge[1],
            player.spellCharge[2],
            player.spellCharge[3] );

        Text::DrawString( str, TextX + 2*8, y + 32 );

        sprintf_s( str, "   %d/%d/%d/%d",
            player.spellCharge[4],
            player.spellCharge[5],
            player.spellCharge[6],
            player.spellCharge[7] );

        Text::DrawString( str, TextX + 2*8, y + 40 );
    }
}

void MainMenu::DrawMainPlayerPic()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        int y = PlayerPicY + PlayerEntryHeight * i;

        DrawPlayer( i, PlayerPicX, y );
    }
}

void MainMenu::DrawPlayer( int id, int x, int y )
{
    Player::Character& player = Player::Party[id];
    int sy = player._class * 24;

    al_draw_bitmap_region( instance->playerBmp, 0, sy, 16, 24, x, y, 0 );
}

void MainMenu::DrawClass( int classId, int x, int y )
{
    int sy = classId * 24;

    al_draw_bitmap_region( instance->playerBmp, 0, sy, 16, 24, x, y, 0 );
}

void MainMenu::DrawOrb( int index, bool lit, int x, int y )
{
    int sx = 0;

    if ( !lit )
        sx = 5 * 16;
    else
        sx = 16 + index * 16;

    al_draw_bitmap_region( instance->menuBmp, sx, 0, 16, 16, x, y, 0 );
}

bool MainMenu::ItemTargetsPlayer( int itemId )
{
    return (instance->itemTarget[itemId / 8] & (1 << (itemId % 8))) != 0;
}

bool MainMenu::SpellTargetsPlayer( int spellId )
{
    return (instance->magicTarget[spellId / 8] & (1 << (spellId % 8))) != 0;
}

const char* MainMenu::GetMenuText( int index )
{
    return instance->menuText.GetItem( index );
}

const char* MainMenu::GetShopText( int index )
{
    return instance->shopText.GetItem( index );
}

ShopType MainMenu::GetShopType( int shopId )
{
    return (ShopType) instance->shopTypes[shopId];
}

int MainMenu::GetShopItems( int shopId, uint8_t* items, int maxItems )
{
    if ( shopId < 0 || shopId >= Shops )
        return 0;

    const uint8_t* shopItems = instance->shopStock.GetItem( shopId );

    for ( int i = 0; i < maxItems; i++ )
    {
        if ( shopItems[i] == 0 )
            return i;

        items[i] = shopItems[i];
    }

    return maxItems;
}

int MainMenu::GetShopPrice( int shopId )
{
    uint32_t price = 0;
    uint8_t* asItems = (uint8_t*) &price;

    GetShopItems( shopId, asItems, 3 );

    return price & 0xffffff;
}

void MainMenu::PlayError()
{
    Sound::PlayEffect( SEffect_Error );
}

void MainMenu::PlayCursor()
{
    Sound::PlayEffect( SEffect_Cursor );
}

void MainMenu::PlayConfirm()
{
    Sound::PlayEffect( SEffect_Confirm );
}

void MainMenu::PlayFanfare()
{
    Sound::PushTrack( Sound_Fanfare, 0 );
}

void MainMenu::PlayPotion()
{
    Sound::PushTrack( Sound_Potion, 0 );
}
