/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Menus.h"
#include "Text.h"
#include "Player.h"
#include "Ids.h"
#include "Overworld.h"
#include "Level.h"
#include "LTile.h"
#include "OWTile.h"


const int ItemVisibleRows = 10;
const int ItemY = 32;
const int MessageBoxY = 176 + 24;

const int UsableItems = Player::ItemTypes - Player::PotionItemsBaseId;
const int FirstUsableItem = Player::PotionItemsBaseId;

const int KeyItems = Player::PotionItemsBaseId - 1;
const int FirstKeyItem = 1;


typedef void (*ItemRoutine)( int playerId, const char*& itemText );


static void UseNone( int playerId, const char*& itemText )
{
    MainMenu::PlayError();
}

static void UseCrown( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 7 );
}

static void UseCrystal( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 8 );
}

static void UseHerb( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 9 );
}

static void UseMysticKey( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 10 );
}

static void UseTnt( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 11 );
}

static void UseAdamant( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 12 );
}

static void UseSlab( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 13 );
}

static void UseRuby( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 14 );
}

static void UseChime( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 19 );
}

static void UseTail( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 20 );
}

static void UseCube( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 21 );
}

static void UseOxyale( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 24 );
}

static void UseCanoe( int playerId, const char*& itemText )
{
    itemText = MainMenu::GetMenuText( 25 );
}

static void UseBottle( int playerId, const char*& itemText )
{
    if ( Player::GetObjVisible( Obj_Fairy ) )
    {
        itemText = MainMenu::GetMenuText( 0x17 );
    }
    else
    {
        Player::SetObjVisible( Obj_Fairy, true );
        itemText = MainMenu::GetMenuText( 0x16 );
        MainMenu::PlayFanfare();
    }
}

static void UseRod( int playerId, const char*& itemText )
{
    if ( Overworld::IsActive()
        || LTile::GetSpecial( Level::GetCurrentTileAttr() ) != LTile::S_UseRod
        || !Player::GetObjVisible( Obj_RodPlate ) )
    {
        itemText = MainMenu::GetMenuText( 0x10 );
    }
    else
    {
        Player::SetObjVisible( Obj_RodPlate, false );
        itemText = MainMenu::GetMenuText( 0xf );
        MainMenu::PlayFanfare();
    }
}

static void UseLute( int playerId, const char*& itemText )
{
    if ( Overworld::IsActive()
        || LTile::GetSpecial( Level::GetCurrentTileAttr() ) != LTile::S_UseLute
        || !Player::GetObjVisible( Obj_LutePlate ) )
    {
        itemText = MainMenu::GetMenuText( 0x6 );
    }
    else
    {
        Player::SetObjVisible( Obj_LutePlate, false );
        itemText = MainMenu::GetMenuText( 0x5 );
        MainMenu::PlayFanfare();
    }
}

static void UseFloater( int playerId, const char*& itemText )
{
    if ( Level::IsActive()
        || OWTile::GetSpecial(Overworld::GetCurrentTileAttr()) != OWTile::S_Floater
        || (Player::GetVehicles() & Vehicle_Airship) != 0 )
    {
        itemText = MainMenu::GetMenuText( 0x12 );
    }
    else
    {
        Player::SetVehicles( (Vehicle) (Player::GetVehicles() | Vehicle_Airship) );
        itemText = MainMenu::GetMenuText( 0x11 );
        MainMenu::PlayFanfare();
    }
}

static void AddPartyHp( int hp )
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Player::Party[i].AddHp( hp );
    }
}

static void UseTent( int playerId, const char*& itemText )
{
    AddPartyHp( 30 );
    MainMenu::PlayPotion();
}

static void UseCabin( int playerId, const char*& itemText )
{
    AddPartyHp( 60 );
    MainMenu::PlayPotion();
}

static void UseHouse( int playerId, const char*& itemText )
{
    AddPartyHp( 120 );
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        for ( int j = 0; j < 8; j++ )
            Player::Party[i].spellCharge[j] = Player::Party[i].spellMaxCharge[j];
    }

    MainMenu::PlayPotion();
}

static void UseHeal( int playerId, const char*& itemText )
{
    Player::Character& player = Player::Party[playerId];

    if ( (player.status & Status_Death) != 0
        || (player.status & Status_Stone) != 0
        || player.hp == player.maxHp )
    {
        MainMenu::PlayError();
    }
    else
    {
        player.AddHp( 30 );
        Player::Items[Item_Heal]--;
        MainMenu::PlayPotion();
    }
}

static void UsePure( int playerId, const char*& itemText )
{
    Player::Character& player = Player::Party[playerId];

    if ( (player.status & Status_Poison) == 0 )
    {
        MainMenu::PlayError();
    }
    else
    {
        player.RemoveStatus( Status_Poison );
        Player::Items[Item_Pure]--;
        MainMenu::PlayPotion();
    }
}

static void UseSoft( int playerId, const char*& itemText )
{
    Player::Character& player = Player::Party[playerId];

    if ( (player.status & Status_Stone) == 0 )
    {
        MainMenu::PlayError();
    }
    else
    {
        player.RemoveStatus( Status_Stone );
        Player::Items[Item_Soft]--;
        MainMenu::PlayPotion();
    }
}


ItemRoutine itemRoutines[] = 
{
    UseNone,        // 00
    UseLute,        // 01
    UseCrown,       // 02
    UseCrystal,     // 03
    UseHerb,        // 04
    UseMysticKey,   // 05
    UseTnt,         // 06
    UseAdamant,     // 07
    UseSlab,        // 08
    UseRuby,        // 09
    UseRod,         // 0A
    UseFloater,     // 0B
    UseChime,       // 0C
    UseTail,        // 0D
    UseCube,        // 0E
    UseBottle,      // 0F
    UseOxyale,      // 10
    UseCanoe,       // 11
    UseNone,        // 12
    UseNone,        // 13
    UseNone,        // 14
    UseNone,        // 15
    UseTent,        // 16
    UseCabin,       // 17
    UseHouse,       // 18
    UseHeal,        // 19
    UsePure,        // 1A
    UseSoft,        // 1B
};


ItemRoutine GetItemRoutine( int itemId )
{
    if ( itemId < 0 || itemId >= _countof( itemRoutines ) )
        return UseNone;

    return itemRoutines[itemId];
}


//----------------------------------------------------------------------------


ItemMenu::ItemMenu()
    :   selIndex( 0 )
{
}

MenuAction ItemMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selIndex = (selIndex + 1) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selIndex = (selIndex - 1 + 2) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();

        if ( selIndex == 0 )
            nextMenu = new UseItemMenu();
        else
            nextMenu = new KeyItemMenu();

        return Menu_Push;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        nextMenu = new CommandMenu();
        return Menu_SwitchTop;
    }

    return Menu_None;
}

void ItemMenu::Draw( MenuDrawState state )
{
    Text::DrawBox( 0, 0, 176, 24 );
    Text::DrawString( "Use", 32, 8 );
    Text::DrawString( "Key", 104, 8 );

    Text::DrawBox( 176, 0, 80, 24 );
    Text::DrawString( "Item", 192, 8 );

    Text::DrawBox( 0, 24, 256, 176 );
    Text::DrawBox( 0, MessageBoxY, 256, 40 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = 16 + selIndex * (104 - 32);

        Text::DrawCursor( x, 8 );
    }
}


//----------------------------------------------------------------------------


ItemListMenuBase::ItemListMenuBase( int listLength, int startItemId, bool drawAmounts )
    :   listLength( listLength ),
        startItemId( startItemId ),
        rows( (listLength + 1) / 2 ),
        topRow( 0 ),
        selCol( 0 ),
        selRow( 0 ),
        drawAmounts( drawAmounts ),
        itemText( "" )
{
}

MenuAction ItemListMenuBase::Update( Menu*& nextMenu )
{
    int index = selRow * 2 + selCol;

    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        if ( index < listLength - 1 )
        {
            selCol++;
            if ( selCol > 1 )
            {
                selCol = 0;
                selRow++;
            }
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        if ( index > 0 )
        {
            selCol--;
            if ( selCol < 0 )
            {
                selCol = 1;
                selRow--;
            }
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        if ( selRow < rows - 1 )
        {
            selRow++;
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        if ( selRow > 0 )
        {
            selRow--;
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        return UseItem( nextMenu );
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    if ( selRow >= topRow + ItemVisibleRows )
    {
        topRow = selRow - ItemVisibleRows + 1;
    }
    else if ( selRow < topRow )
    {
        topRow = selRow;
    }

    // reset the text, if moved the cursor
    if ( index != selRow * 2 + selCol )
        itemText = "";

    return Menu_None;
}

MenuAction ItemListMenuBase::UseItem( Menu*& nextMenu )
{
    int index = selRow * 2 + selCol;
    int itemId = startItemId + index;

    if ( Player::Items[itemId] > 0 )
    {
        if ( MainMenu::ItemTargetsPlayer( itemId ) )
        {
            nextMenu = new ChooseItemTargetMenu( itemId );
            return Menu_Push;
        }
        else
        {
            ItemRoutine routine = GetItemRoutine( itemId );
            const char* text = nullptr;

            routine( -1, text );

            if ( text == nullptr )
                text = "";

            itemText = text;
        }
    }

    return Menu_None;
}

void ItemListMenuBase::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    int firstIndex = topRow * 2;
    int lastIndex = firstIndex + ItemVisibleRows * 2;

    if ( lastIndex > listLength )
        lastIndex = listLength;

    for ( int i = firstIndex; i < lastIndex; i++ )
    {
        int itemId = startItemId + i;
        int col = i % 2;
        int row = i / 2 - topRow;

        if ( Player::Items[itemId] > 0 )
        {
            const char* name = Player::GetItemName( itemId );
            int x = 8 + 16 + col * 15 * 8;
            int y = ItemY + row * 16;

            Text::DrawString( name, x, y );

            if ( drawAmounts )
            {
                char amountStr[4] = "";

                sprintf_s( amountStr, " %2d", Player::Items[itemId] );

                Text::DrawString( amountStr, x + 10 * 8, y );
            }
        }
    }

    Text::DrawString( itemText, 16, MessageBoxY + 8 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = 8 + selCol * 15 * 8;
        int y = ItemY + (selRow - topRow) * 16;

        Text::DrawCursor( x, y );
    }
}


//----------------------------------------------------------------------------


UseItemMenu::UseItemMenu()
    :   ItemListMenuBase( UsableItems, FirstUsableItem, true )
{
}

//----------------------------------------------------------------------------


KeyItemMenu::KeyItemMenu()
    :   ItemListMenuBase( KeyItems, FirstKeyItem, false )
{
}


//----------------------------------------------------------------------------


ChooseItemTargetMenu::ChooseItemTargetMenu( int itemId )
    :   itemId( itemId ),
        selIndex( 0 ),
        timer( 0 )
{
}

MenuAction ChooseItemTargetMenu::Update( Menu*& nextMenu )
{
    if ( timer > 0 )
    {
        timer--;
        return Menu_None;
    }

    if ( Player::Items[itemId] == 0 )
        return Menu_Pop;

    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % Players;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + Players) % Players;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        UseItem();

        timer = 15;

        return Menu_None;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseItemTargetMenu::UseItem()
{
    ItemRoutine routine = GetItemRoutine( itemId );
    const char* text = nullptr;

    routine( selIndex, text );
}

void ChooseItemTargetMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    MainMenu::DrawMainPlayerInfo();

    char str[32];
    const char* itemName = Player::GetItemName( itemId );

    Text::DrawBox( 176, 0, 80, 32+3*8 );
    Text::DrawString( itemName, 176 + 8, 16 );

    sprintf_s( str, "Total %2d", Player::Items[itemId] );

    Text::DrawString( str, 176 + 8, 32 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = PlayerPicY + selIndex * PlayerEntryHeight;

        Text::DrawCursor( PlayerPicX - 16, y );
    }
}
