/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "ShopMenus.h"
#include "Text.h"
#include "Player.h"
#include "Sound.h"


enum
{
    Msg_Welcome = 9,
    Msg_WhichItem,
    Msg_WhatToSell,
    Msg_CantAfford,
    Msg_CantCarryMore,
    Msg_BuyHowMany,
    Msg_ItemDontWant,
    Msg_ItemThanks,
    Msg_SellHowMany,
    Msg_WhichSpell,
    Msg_WhoWillLearn,
    Msg_SpellDontWant,
    Msg_CantLearn,
    Msg_AlreadyKnow,
    Msg_FullSpellLevel,
    Msg_SpellThanks,
    Msg_WhoToRevive_Price,
    Msg_NoneToRevive,
    Msg_ReturnToLife,
    Msg_Inn_Price,
    Msg_GoToSleep,
};


const int TopMenuBoxX = 0;
const int TopMenuBoxY = 40;
const int TopMenuWidth = 176;
const int TopMenuHeight = 24;

const int ItemBoxY = TopMenuBoxY + TopMenuHeight;
const int ItemBoxHeight = 120;

const int SpellListBoxX = 0;
const int SpellListBoxY = 64;
const int SpellListBoxWidth = 256;
const int SpellListBoxHeight = 120;

const int PartyX = 32;
const int PartyY = 200;
const int PlayerWidth = 48;


void DrawCommonShop( int shopId, const char* message, bool tallMsg )
{
    char str[32];
    int shopType = MainMenu::GetShopType( shopId );

    Text::DrawBox( 0, 0, 176, 40 );
    Text::DrawString( message, 16, 8 );

    Text::DrawBox( 176, 0, 80, 40 );
    Text::DrawString( MainMenu::GetShopText( shopType ), 192, 16 );

    sprintf_s( str, "%6d G", Player::GetG() );

    Text::DrawBox( 176, 40, 80, 24 );
    Text::DrawString( str, 184, 40+8 );
}

Point GetPlayerEquipabilityPos( int playerId )
{
    Point point = { PartyX, PartyY };
    point.X += playerId * PlayerWidth;
    return point;
}

void DrawPlayerEquipability( int playerId, bool canEquip )
{
    Point point = GetPlayerEquipabilityPos( playerId );

    if ( canEquip )
        MainMenu::DrawPlayerEquipableAnim( playerId, point.X, point.Y );
    else
        MainMenu::DrawPlayer( playerId, point.X, point.Y );
}


//----------------------------------------------------------------------------
//  ItemShopMenu
//----------------------------------------------------------------------------

ItemShopMenu* ItemShopMenu::instance;


ItemShopMenu::ItemShopMenu( int shopId )
    :   shopId( shopId ),
        selIndex( 0 ),
        msg( nullptr )
{
    instance = this;
    msg = MainMenu::GetShopText( Msg_Welcome );
}

ItemShopMenu::~ItemShopMenu()
{
    instance = nullptr;
}

MenuAction ItemShopMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selIndex = (selIndex + 1) % 3;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selIndex = (selIndex - 1 + 3) % 3;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();

        switch ( selIndex )
        {
        case 0:
            SetMessage( Msg_WhichItem );
            nextMenu = new BuyItemMenu( shopId );
            return Menu_Push;
        case 1:
            SetMessage( Msg_WhatToSell );
            nextMenu = new SellItemMenu( shopId );
            return Menu_Push;
        case 2:
            return Menu_Pop;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ItemShopMenu::Draw( MenuDrawState state )
{
    const int FirstItemX = TopMenuBoxX + 24;

    Text::DrawBox( TopMenuBoxX, TopMenuBoxY, TopMenuWidth, TopMenuHeight );
    Text::DrawString( "Buy", FirstItemX, TopMenuBoxY+8 );
    Text::DrawString( "Sell", TopMenuBoxX + 72, TopMenuBoxY+8 );
    Text::DrawString( "Exit", TopMenuBoxX + 120, TopMenuBoxY+8 );

    DrawCommonShop( shopId, msg, false );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = FirstItemX + selIndex * (72 - 24) - 16;

        Text::DrawCursor( x, TopMenuBoxY+8 );
    }
}

void ItemShopMenu::SetMessage( int shopMsgId )
{
    instance->msg = MainMenu::GetShopText( shopMsgId );
}

Menu* ItemShopMenu::Make( int shopId )
{
    return new ItemShopMenu( shopId );
}


//----------------------------------------------------------------------------
//  BuyItemMenu
//----------------------------------------------------------------------------

BuyItemMenu::BuyItemMenu( int shopId )
    :   shopId( shopId ),
        selIndex( 0 ),
        listLength( 0 )
{
    listLength = MainMenu::GetShopItems( shopId, items, MaxItems );
}

MenuAction BuyItemMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % listLength;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + listLength) % listLength;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        int itemId = items[selIndex];
        int price = Global::GetPrice( itemId );

        if ( price > Player::GetG() )
        {
            ItemShopMenu::SetMessage( Msg_CantAfford );
            MainMenu::PlayError();
        }
        else if ( Player::Items[itemId] == Player::MaxItems )
        {
            ItemShopMenu::SetMessage( Msg_CantCarryMore );
            MainMenu::PlayError();
        }
        else
        {
            MainMenu::PlayConfirm();
            ItemShopMenu::SetMessage( Msg_BuyHowMany );
            nextMenu = new AmountMenu( itemId, Shop_Buy );
            return Menu_Push;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        ItemShopMenu::SetMessage( Msg_ItemDontWant );
        return Menu_Pop;
    }

    return Menu_None;
}

void BuyItemMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    char str[32];

    Text::DrawBox( 0, ItemBoxY, 152, ItemBoxHeight );

    for ( int i = 0; i < listLength; i++ )
    {
        int itemId = items[i];
        const char* name = Player::GetItemName( itemId );
        int price = Global::GetPrice( itemId );

        sprintf_s( str, "%-8s %5u G", name, price );
        Text::DrawString( str, 16, ItemBoxY+16 + i*16 );
    }

    Text::DrawBox( 152, ItemBoxY, 128-24, ItemBoxHeight );

    int selItemId = items[selIndex];

    sprintf_s( str, "%2d", Player::Items[selItemId] );
    Text::DrawString( "Stock", 152 + 16, ItemBoxY + 16 );
    Text::DrawString( str, 152 + 80, ItemBoxY + 16 + 8 );

    DrawEquipUI();

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = ItemBoxY+16 + selIndex * 16;

        Text::DrawCursor( 0, y );
    }
}

void BuyItemMenu::DrawEquipUI()
{
    char str[32];
    int shopType = MainMenu::GetShopType( shopId );
    int selItemId = items[selIndex];
    int equipped = 0;

    if ( shopType != ShopType_Weapon && shopType != ShopType_Armor )
        return;

    if ( shopType == ShopType_Weapon )
    {
        for ( int i = 0; i < Player::PartySize; i++ )
        {
            if ( Player::Party[i].weaponId != NoWeapon )
            {
                int id = Player::Party[i].weaponId + Player::WeaponsBaseId;
                if ( id == selItemId )
                    equipped++;
            }
        }
    }
    else if ( shopType == ShopType_Armor )
    {
        for ( int i = 0; i < Player::PartySize; i++ )
        {
            for ( int j = 0; j < Player::Armor_Max; j++ )
            {
                if ( Player::Party[i].armorIds[j] != NoArmor )
                {
                    int id = Player::Party[i].armorIds[j] + Player::ArmorBaseId;
                    if ( id == selItemId )
                        equipped++;
                }
            }
        }
    }

    sprintf_s( str, "%2d", equipped );
    Text::DrawString( "Equipped", 152 + 16, ItemBoxY + 32 );
    Text::DrawString( str, 152 + 80, ItemBoxY + 32 + 8 );

    if (   shopType == ShopType_Weapon
        || shopType == ShopType_Armor )
    {
        for ( int i = 0; i < Players; i++ )
        {
            bool canEquip = false;

            if ( shopType == ShopType_Weapon )
                canEquip = Player::CanEquipWeapon( selItemId, Player::Party[i]._class );
            else if ( shopType == ShopType_Armor )
                canEquip = Player::CanEquipArmor( selItemId, Player::Party[i]._class );

            DrawPlayerEquipability( i, canEquip );
        }
    }
}


//----------------------------------------------------------------------------
//  AmountMenu
//----------------------------------------------------------------------------

AmountMenu::AmountMenu( int itemId, ShopTransaction transaction )
    :   itemId( itemId ),
        transaction( transaction ),
        amount( 1 ),
        maxAmount( 0 ),
        price( 0 )
{
    price = Global::GetPrice( itemId );

    if ( transaction == Shop_Buy )
    {
        maxAmount = Player::GetG() / price;
    }
    else
    {
        price /= 2;
        maxAmount = Player::Items[itemId];
    }
}

MenuAction AmountMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        if ( amount < maxAmount )
        {
            amount++;
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        if ( amount > 1 )
        {
            amount--;
            MainMenu::PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();

        if ( transaction == Shop_Buy )
        {
            Player::Items[itemId] += amount;
            Player::SubG( price * amount );
        }
        else
        {
            Player::Items[itemId] -= amount;
            Player::AddG( price * amount );
        }
        ItemShopMenu::SetMessage( Msg_ItemThanks );
        return Menu_Pop;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        ItemShopMenu::SetMessage( Msg_WhichItem );
        return Menu_Pop;
    }

    return Menu_None;
}

void AmountMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    const int BoxX = 0;
    const int BoxY = ItemBoxY;
    const int PromptX = BoxX + 16;
    const int PromptY = BoxY + 16;
    const int NumberX = PromptX + 9*8;

    char str[32];

    Text::DrawBox( BoxX, BoxY, 152, 136 );

    sprintf_s( str, "How many? %2u", amount );
    Text::DrawString( str, PromptX, PromptY );

    int total = amount * price;

    sprintf_s( str, "%5u G", total );
    Text::DrawString( str, NumberX, PromptY + 16 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        Text::DrawCursor( PromptX - 16, PromptY );
    }
}


//----------------------------------------------------------------------------
//  SellItemMenu
//----------------------------------------------------------------------------

const int UsableItems = Player::ItemTypes - Player::PotionItemsBaseId;
const int FirstUsableItem = Player::PotionItemsBaseId;

const int ListLength = UsableItems;
const int StartItemId = FirstUsableItem;
const int ItemVisibleRows = 10;

SellItemMenu::SellItemMenu( int shopId )
    :   topRow( 0 ),
        selCol( 0 ),
        selRow( 0 ),
        rows( (ListLength + 1) / 2 )
{
}

MenuAction SellItemMenu::Update( Menu*& nextMenu )
{
    int index = selRow * 2 + selCol;

    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        if ( index < ListLength - 1 )
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
        int itemId = index + FirstUsableItem;

        if ( Player::Items[itemId] != 0 )
        {
            MainMenu::PlayConfirm();
            ItemShopMenu::SetMessage( Msg_SellHowMany );
            nextMenu = new AmountMenu( itemId, Shop_Sell );
            return Menu_Push;
        }
        else
            MainMenu::PlayError();
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        ItemShopMenu::SetMessage( Msg_ItemDontWant );
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

    return Menu_None;
}

void SellItemMenu::Draw( MenuDrawState state )
{
    const int ItemY = ItemBoxY + 16;

    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    char str[32] = "";
    int firstIndex = topRow * 2;
    int lastIndex = firstIndex + ItemVisibleRows * 2;

    if ( lastIndex > ListLength )
        lastIndex = ListLength;

    Text::DrawBox( 0, ItemBoxY, 256, 176 );

    for ( int i = firstIndex; i < lastIndex; i++ )
    {
        int itemId = StartItemId + i;
        int col = i % 2;
        int row = i / 2 - topRow;

        if ( Player::Items[itemId] > 0 )
        {
            const char* name = Player::GetItemName( itemId );
            int x = 8 + 16 + col * 15 * 8;
            int y = ItemY + row * 16;

            sprintf_s( str, "%-10s %2d", name, Player::Items[itemId] );
            Text::DrawString( str, x, y );
        }
    }

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = 8 + selCol * 15 * 8;
        int y = ItemY + (selRow - topRow) * 16;

        Text::DrawCursor( x, y );
    }
}


//----------------------------------------------------------------------------
//  MagicShopMenu
//----------------------------------------------------------------------------

MagicShopMenu* MagicShopMenu::instance;


MagicShopMenu::MagicShopMenu( int shopId )
    :   shopId( shopId ),
        selIndex( 0 ),
        msg( nullptr ),
        listLength( 0 )
{
    instance = this;

    msg = MainMenu::GetShopText( Msg_Welcome );
    listLength = MainMenu::GetShopItems( shopId, spells, _countof( spells ) );
}

MagicShopMenu::~MagicShopMenu()
{
    instance = nullptr;
}

MenuAction MagicShopMenu::Update( Menu*& nextMenu )
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

        switch ( selIndex )
        {
        case 0:
            SetMessage( Msg_WhichSpell );
            nextMenu = new BuySpellMenu( spells, listLength );
            return Menu_Push;
        case 1:
            return Menu_Pop;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void MagicShopMenu::Draw( MenuDrawState state )
{
    const int FirstItemX = TopMenuBoxX + 32;

    char str[32];

    Text::DrawBox( TopMenuBoxX, TopMenuBoxY, TopMenuWidth, TopMenuHeight );
    Text::DrawString( "Buy", FirstItemX, TopMenuBoxY+8 );
    Text::DrawString( "Exit", TopMenuBoxX + 104, TopMenuBoxY+8 );

    DrawCommonShop( shopId, msg, false );

    Text::DrawBox( SpellListBoxX, SpellListBoxY, SpellListBoxWidth, SpellListBoxHeight );

    for ( int i = 0; i < listLength; i++ )
    {
        int itemId = spells[i];
        const char* name = Player::GetItemName( itemId );
        int price = Global::GetPrice( itemId );

        sprintf_s( str, "%-8s %5u G", name, price );
        Text::DrawString( str, SpellListBoxX+16, SpellListBoxY+16 + i*16 );
    }

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = FirstItemX + selIndex * (104 - 32) - 16;

        Text::DrawCursor( x, TopMenuBoxY+8 );
    }
}

void MagicShopMenu::SetMessage( int shopMsgId )
{
    instance->msg = MainMenu::GetShopText( shopMsgId );
}

Menu* MagicShopMenu::Make( int shopId )
{
    return new MagicShopMenu( shopId );
}


//----------------------------------------------------------------------------
//  BuySpellMenu
//----------------------------------------------------------------------------

BuySpellMenu::BuySpellMenu( const uint8_t* spells, int listLength )
    :   selIndex( 0 ),
        listLength( listLength ),
        spells( spells )
{
}

MenuAction BuySpellMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % listLength;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + listLength) % listLength;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        int spellItemId = spells[selIndex];

        if ( Player::GetG() >= Global::GetPrice( spellItemId ) )
        {
            MainMenu::PlayConfirm();
            MagicShopMenu::SetMessage( Msg_WhoWillLearn );
            nextMenu = new LearnerMenu( spellItemId );
            return Menu_Push;
        }
        else
        {
            MainMenu::PlayError();
            MagicShopMenu::SetMessage( Msg_CantAfford );
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        MagicShopMenu::SetMessage( Msg_SpellDontWant );
        return Menu_Pop;
    }

    return Menu_None;
}

void BuySpellMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = SpellListBoxY+16 + selIndex * 16;

        Text::DrawCursor( SpellListBoxX, y );
    }

    int spellItemId = spells[selIndex];

    for ( int i = 0; i < Players; i++ )
    {
        bool canLearn = Player::CanLearnSpell( spellItemId, Player::Party[i]._class );
        DrawPlayerEquipability( i, canLearn );
    }
}


//----------------------------------------------------------------------------
//  LearnerMenu
//----------------------------------------------------------------------------

LearnerMenu::LearnerMenu( int spellId )
    :   spellId( spellId ),
        selIndex( 0 )
{
}

MenuAction LearnerMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selIndex = (selIndex + 1) % Player::PartySize;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selIndex = (selIndex - 1 + Player::PartySize) % Player::PartySize;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        if ( LearnSpell() )
        {
            MainMenu::PlayConfirm();
            return Menu_Pop;
        }
        else
            MainMenu::PlayError();
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        MagicShopMenu::SetMessage( Msg_WhichSpell );
        return Menu_Pop;
    }

    return Menu_None;
}

bool LearnerMenu::LearnSpell()
{
    Player::Character& player = Player::Party[selIndex];
    int price = Global::GetPrice( spellId );

    if ( !player.IsAlive() 
        || !Player::CanLearnSpell( spellId, player._class ) )
    {
        MagicShopMenu::SetMessage( Msg_CantLearn );
        return false;
    }

    int level = 0;
    int sil = 0;
    int freeSlot = -1;

    Player::GetSpellId( spellId, level, sil );

    for ( int i = 0; i < SpellSlots; i++ )
    {
        if ( player.spells[level][i] == sil )
        {
            MagicShopMenu::SetMessage( Msg_AlreadyKnow );
            return false;
        }
        if ( player.spells[level][i] == NoMagic && freeSlot == -1 )
            freeSlot = i;
    }

    if ( freeSlot == -1 )
    {
        MagicShopMenu::SetMessage( Msg_FullSpellLevel );
        return false;
    }

    Player::SubG( price );
    player.spells[level][freeSlot] = sil;

    MagicShopMenu::SetMessage( Msg_SpellThanks );
    return true;
}

void LearnerMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        Point point = GetPlayerEquipabilityPos( selIndex );
        Text::DrawCursor( point.X - 16, point.Y );
    }
}


//----------------------------------------------------------------------------
//  ChurchShopMenu
//----------------------------------------------------------------------------

ChurchShopMenu::ChurchShopMenu( int shopId )
    :   shopId( shopId ),
        price( 0 ),
        selIndex( 0 ),
        step( 0 ),
        playerCount( 0 ),
        msg( nullptr )
{
    price = MainMenu::GetShopPrice( shopId );

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        if ( !Player::Party[i].IsAlive() )
        {
            players[playerCount] = i;
            playerCount++;
        }
    }

    if ( playerCount > 0 )
    {
        sprintf_s( question, MainMenu::GetShopText( Msg_WhoToRevive_Price ), price );
        msg = question;
    }
    else
    {
        msg = MainMenu::GetShopText( Msg_NoneToRevive );
        step = 1;
    }
}

MenuAction ChurchShopMenu::Update( Menu*& nextMenu )
{
    if ( step == 1 )
    {
        if ( Input::IsKeyPressing( ConfirmKey ) )
        {
            if ( playerCount == 0 )
                return Menu_Pop;

            sprintf_s( question, MainMenu::GetShopText( Msg_WhoToRevive_Price ), price );
            msg = question;
            step = 0;
        }

        return Menu_None;
    }

    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % playerCount;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + playerCount) % playerCount;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        step = 1;
        if ( price <= Player::GetG() )
        {
            Player::SubG( price );
            RevivePlayer();

            Sound::PlayEffect( SEffect_Potion );
            msg = MainMenu::GetShopText( Msg_ReturnToLife );
        }
        else
        {
            MainMenu::PlayError();
            msg = MainMenu::GetShopText( Msg_CantAfford );
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChurchShopMenu::RevivePlayer()
{
    int id = players[selIndex];

    Player::FullyRevive( id );

    for ( int i = selIndex; i < playerCount - 1; i++ )
    {
        players[i] = players[i + 1];
    }

    playerCount--;
    selIndex = 0;
}

void ChurchShopMenu::Draw( MenuDrawState state )
{
    DrawCommonShop( shopId, msg, true );

    if ( step == 0 )
    {
        Text::DrawBox( 0, 40, 80, 24 + playerCount*16 );

        for ( int i = 0; i < playerCount; i++ )
        {
            int id = players[i];
            Player::Character& player = Player::Party[id];

            Text::DrawString( player.name, 0+16, 40+16 + i*16 );
        }

        if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
        {
            int y = 40+16 + selIndex * 16;

            Text::DrawCursor( 0+16-16, y );
        }
    }
}

Menu* ChurchShopMenu::Make( int shopId )
{
    return new ChurchShopMenu( shopId );
}


//----------------------------------------------------------------------------
//  InnShopMenu
//----------------------------------------------------------------------------

InnShopMenu::InnShopMenu( int shopId )
    :   shopId( shopId ),
        price( 0 ),
        selIndex( 0 ),
        step( 0 ),
        msg( nullptr )
{
    price = MainMenu::GetShopPrice( shopId );

    sprintf_s( question, MainMenu::GetShopText( Msg_Inn_Price ), price );

    msg = question;
}

MenuAction InnShopMenu::Update( Menu*& nextMenu )
{
    if ( step == 1 )
    {
        if ( Input::IsKeyPressing( ConfirmKey ) )
        {
            return Menu_Pop;
        }

        return Menu_None;
    }

    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + 2) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        switch ( selIndex )
        {
        case 0:
            step = 1;
            if ( price <= Player::GetG() )
            {
                Player::SubG( price );
                RestorePlayers();

                Sound::PlayEffect( SEffect_Potion );
                msg = MainMenu::GetShopText( Msg_GoToSleep );
            }
            else
            {
                MainMenu::PlayError();
                msg = MainMenu::GetShopText( Msg_CantAfford );
            }
            break;
        case 1:
            return Menu_Pop;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void InnShopMenu::RestorePlayers()
{
    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Player::Character& player = Player::Party[i];

        if ( player.IsAlive() )
            Player::FullyRevive( i );
    }
}

void InnShopMenu::Draw( MenuDrawState state )
{
    DrawCommonShop( shopId, msg, true );

    if ( step == 0 )
    {
        Text::DrawBox( 0, 40, 80, 56 );
        Text::DrawString( "Yes", 0+16, 40+16 );
        Text::DrawString( "No", 0+16, 40+32 );

        if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
        {
            int y = 40+16 + selIndex * 16;

            Text::DrawCursor( 0+16-16, y );
        }
    }
}

Menu* InnShopMenu::Make( int shopId )
{
    return new InnShopMenu( shopId );
}
