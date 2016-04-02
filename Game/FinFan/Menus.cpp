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
#include "SaveLoadMenu.h"
#include "Overworld.h"


StatusMenu::StatusMenu( int playerId )
    :   playerId( playerId )
{
}

MenuAction StatusMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( CancelKey ) )
    {
        nextMenu = new CommandMenu();
        return Menu_SwitchTop;
    }

    return Menu_None;
}

void StatusMenu::Draw( MenuDrawState state )
{
    const int Y = 24;
    const int HeadTextX = PlayerPicX + 32;
    const int ProfileTextX = HeadTextX + 2*8;
    const int StatBasicX = PlayerPicX;
    const int StatDerivedX = StatBasicX + 128;

    Player::Character& player = Player::Party[playerId];
    char str[32];

    Text::DrawBox( 0, 0, 256, 240 );

    Text::DrawBox( 176, 0, 80, 24 );
    Text::DrawString( "Status", 192, 8 );

    MainMenu::DrawPlayer( playerId, 24, Y );
    Text::DrawString( player.name, HeadTextX, Y );

    sprintf_s( str, "Lv %2d", player.level );

    Text::DrawString( str, HeadTextX + 7 * 8, Y );
    Text::DrawString( Player::GetClassName( player._class ), ProfileTextX, Y + 16 );

    sprintf_s( str, "HP %3d/%3d", player.hp, player.maxHp );

    Text::DrawString( str, ProfileTextX, Y + 32 );

    sprintf_s( str, "MP %d/%d/%d/%d", 
        player.spellCharge[0],
        player.spellCharge[1],
        player.spellCharge[2],
        player.spellCharge[3] );

    Text::DrawString( str, ProfileTextX, Y + 48 );

    sprintf_s( str, "   %d/%d/%d/%d",
        player.spellCharge[4],
        player.spellCharge[5],
        player.spellCharge[6],
        player.spellCharge[7] );

    Text::DrawString( str, ProfileTextX, Y + 56 );

    sprintf_s( str, "%s    %7d", "EXP.POINTS", player.xp );
    Text::DrawString( str, PlayerPicX, Y + 72 );

    sprintf_s( str, "%s    %7d", "FOR LEV UP", Player::levelXp[ player.level - 1 ] );
    Text::DrawString( str, PlayerPicX, Y + 88 );

    sprintf_s( str, "STR.    %3d", player.basicStats[Player::Stat_Strength] );
    Text::DrawString( str, StatBasicX, Y + 104 );

    sprintf_s( str, "AGL.    %3d", player.basicStats[Player::Stat_Agility] );
    Text::DrawString( str, StatBasicX, Y + 120 );

    sprintf_s( str, "INT.    %3d", player.basicStats[Player::Stat_Intelligence] );
    Text::DrawString( str, StatBasicX, Y + 136 );

    sprintf_s( str, "VIT.    %3d", player.basicStats[Player::Stat_Vitality] );
    Text::DrawString( str, StatBasicX, Y + 152 );

    sprintf_s( str, "LUCK    %3d", player.basicStats[Player::Stat_Luck] );
    Text::DrawString( str, StatBasicX, Y + 168 );

    sprintf_s( str, "DAMAGE   %3d", player.damage );
    Text::DrawString( str, StatDerivedX, Y + 104 );

    sprintf_s( str, "HIT %%    %3d", player.hitRate );
    Text::DrawString( str, StatDerivedX, Y + 120 );

    sprintf_s( str, "ABSORB   %3d", player.absorb );
    Text::DrawString( str, StatDerivedX, Y + 136 );

    sprintf_s( str, "EVADE %%  %3d", player.evadeRate );
    Text::DrawString( str, StatDerivedX, Y + 152 );
}


//----------------------------------------------------------------------------


// UI slot -> merged slot
const int uiSlotMap[] = 
{
    0,
    Player::Armor_Shield+1,
    Player::Armor_Head+1,
    Player::Armor_Body+1,
    Player::Armor_Hand+1,
};
const int EquipItemBoxY = 120;
const int EquipSlotY = 40;
const int EquipSlotX = 128 + 56;


EquipMenu::EquipMenu( int playerId )
    :   playerId( playerId ),
        selIndex( 0 )
{
}

MenuAction EquipMenu::Update( Menu*& nextMenu )
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
            nextMenu = new ChooseEquipSlotMenu( playerId, Equip_Equip );
        else
            nextMenu = new ChooseEquipSlotMenu( playerId, Equip_Remove );

        return Menu_Push;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        nextMenu = new CommandMenu();
        return Menu_SwitchTop;
    }

    return Menu_None;
}

void EquipMenu::Draw( MenuDrawState state )
{
    Player::Character& player = Player::Party[playerId];
    char str[32];

    Text::DrawBox( 0, 0, 176, 24 );
    Text::DrawString( "Equip", 32, 8 );
    Text::DrawString( "Remove", 104, 8 );

    Text::DrawBox( 176, 0, 80, 24 );
    Text::DrawString( "Equip", 192, 8 );

    Text::DrawBox( 0, 24, 256, 96 );
    Text::DrawString( "Weapon", 128, EquipSlotY );
    Text::DrawString( "Shield", 128, EquipSlotY +16 );
    Text::DrawString( "Helmet", 128, EquipSlotY +32 );
    Text::DrawString( "Armor",  128, EquipSlotY +48 );
    Text::DrawString( "Gloves", 128, EquipSlotY +64 );

    Text::DrawBox( 0, EquipItemBoxY, 128, 120 );
    Text::DrawBox( 128, 120, 128, 120 );

    const char* itemName = nullptr;

    if ( player.weaponId != NoWeapon )
    {
        int itemId = player.weaponId + Player::WeaponsBaseId;
        const char* itemName = Player::GetItemName( itemId );
        Text::DrawString( itemName, EquipSlotX, EquipSlotY );
    }

    for ( int i = 0; i < Player::Armor_Max; i++ )
    {
        int armorSlot = uiSlotMap[i + 1] - 1;
        if ( player.armorIds[armorSlot] != NoArmor )
        {
            int itemId = player.armorIds[armorSlot] + Player::ArmorBaseId;
            const char* itemName = Player::GetItemName( itemId );
            Text::DrawString( itemName, EquipSlotX, EquipSlotY + 16 + 16 * i );
        }
    }

    MainMenu::DrawPlayer( playerId, 16, 40 );
    Text::DrawString( player.name, 16+24, 40 );

    sprintf_s( str, "Lv %2d", player.level );

    Text::DrawString( str, 16+24, 40+16 );
    Text::DrawString( Player::GetClassName( player._class ), 16+24, 40+32 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = 16 + selIndex * (104-32);

        Text::DrawCursor( x, 8 );
    }
}


//----------------------------------------------------------------------------


ChooseEquipSlotMenu::ChooseEquipSlotMenu( int playerId, EquipAction action )
    :   playerId( playerId ),
        selIndex( 0 ),
        action( action )
{
}

MenuAction ChooseEquipSlotMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % 5;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + 5) % 5;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        if ( action == Equip_Equip )
        {
            int mergedSlot = uiSlotMap[selIndex];

            auto menu = new ChooseEquipItemMenu( playerId, mergedSlot );
            if ( menu->GetListLength() > 0 )
            {
                MainMenu::PlayConfirm();
                nextMenu = menu;
                return Menu_Push;
            }

            delete menu;
            MainMenu::PlayError();
        }
        else if ( action == Equip_Remove )
        {
            if ( RemoveItem() )
                MainMenu::PlayConfirm();
            else
                MainMenu::PlayError();
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseEquipSlotMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = EquipSlotY + selIndex * 16;

        Text::DrawCursor( EquipSlotX - 16, y );
    }
}

bool ChooseEquipSlotMenu::RemoveItem()
{
    Player::Character& player = Player::Party[playerId];
    int mergedSlot = uiSlotMap[selIndex];
    int itemId = 0;

    if ( mergedSlot == 0 )
    {
        if ( player.weaponId == NoWeapon )
            return false;

        itemId = player.weaponId + Player::WeaponsBaseId;
        player.weaponId = NoWeapon;
    }
    else
    {
        int armorSlot = mergedSlot - 1;

        if ( player.armorIds[armorSlot] == NoArmor )
            return false;

        itemId = player.armorIds[armorSlot] + Player::ArmorBaseId;
        player.armorIds[armorSlot] = NoArmor;
    }

    if ( Player::Items[itemId] < Player::MaxItems )
        Player::Items[itemId]++;

    Player::CalcDerivedStats( playerId );

    return true;
}


//----------------------------------------------------------------------------


ChooseEquipItemMenu::ChooseEquipItemMenu( int playerId, int mergedSlot )
    :   playerId( playerId ),
        mergedSlot( mergedSlot ),
        selRow( 0 ),
        topRow( 0 )
{
    if ( mergedSlot == 0 )
        FillWeapons();
    else
        FillArmors();
}

int ChooseEquipItemMenu::GetListLength()
{
    return listLength;
}

void ChooseEquipItemMenu::FillWeapons()
{
    int j = 0;
    int _class = Player::Party[playerId]._class;

    for ( int i = Player::WeaponsBaseId; i < Player::WeaponsBaseId + Weapons; i++ )
    {
        if ( Player::Items[i] > 0 
            && Player::CanEquipWeapon( i, _class ) )
        {
            items[j] = i;
            j++;
        }
    }

    listLength = j;
}

void ChooseEquipItemMenu::FillArmors()
{
    int j = 0;
    int _class = Player::Party[playerId]._class;
    int armorSlot = mergedSlot - 1;

    for ( int i = Player::ArmorBaseId; i < Player::ArmorBaseId + Armors; i++ )
    {
        if ( Player::Items[i] > 0 
            && Player::CanEquipArmor( i, _class ) 
            && Player::GetArmorType( i ) == armorSlot )
        {
            items[j] = i;
            j++;
        }
    }

    listLength = j;
}

MenuAction ChooseEquipItemMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        if ( selRow < listLength - 1 )
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
        MainMenu::PlayConfirm();
        EquipSelectedItem();
        return Menu_Pop;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    if ( selRow >= topRow + VisibleRows )
    {
        topRow = selRow - VisibleRows + 1;
    }
    else if ( selRow < topRow )
    {
        topRow = selRow;
    }

    return Menu_None;
}

void ChooseEquipItemMenu::Draw( MenuDrawState state )
{
    const int ItemY = EquipItemBoxY + 16;

    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    int firstIndex = topRow;
    int lastIndex = topRow + VisibleRows;

    if ( lastIndex > listLength )
        lastIndex = listLength;

    for ( int i = firstIndex; i < lastIndex; i++ )
    {
        int itemId = items[i];
        const char* name = Player::GetItemName( itemId );
        int y = ItemY + (i - firstIndex) * 16;
        char str[4];

        Text::DrawString( name, 16, y );

        sprintf_s( str, "%2d", Player::Items[itemId] );

        Text::DrawString( str, 16 + 80, y );
    }

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = ItemY + (selRow - topRow) * 16;

        Text::DrawCursor( 0, y );
    }
}

void ChooseEquipItemMenu::EquipSelectedItem()
{
    Player::Character& player = Player::Party[playerId];

    int oldItemId = 0;
    int newItemId = items[selRow];

    if ( mergedSlot == 0 )
    {
        if ( player.weaponId != NoWeapon )
            oldItemId = player.weaponId + Player::WeaponsBaseId;
        player.weaponId = newItemId - Player::WeaponsBaseId;
    }
    else
    {
        int armorSlot = mergedSlot - 1;

        if ( player.armorIds[armorSlot] != NoArmor )
            oldItemId = player.armorIds[armorSlot] + Player::ArmorBaseId;
        player.armorIds[armorSlot] = newItemId - Player::ArmorBaseId;
    }

    if ( oldItemId > 0 && Player::Items[oldItemId] < Player::MaxItems )
        Player::Items[oldItemId]++;

    Player::Items[newItemId]--;
    Player::CalcDerivedStats( playerId );
}


//----------------------------------------------------------------------------


CommandMenu* CommandMenu::instance;


CommandMenu::CommandMenu()
    :   selIndex( 0 ),
        showPics( true )
{
    instance = this;
}

MenuAction CommandMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % 6;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + 6) % 6;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        switch ( selIndex )
        {
        case 0:
            MainMenu::PlayConfirm();
            nextMenu = new ItemMenu();
            return Menu_SwitchTop;

        case 1:
        case 2:
        case 3:
            MainMenu::PlayConfirm();
            nextMenu = new ChooseCommandTargetMenu( selIndex );
            return Menu_Push;

        case 4:
            MainMenu::PlayConfirm();
            nextMenu = new ChoosePartyMenu();
            return Menu_Push;

        case 5:
            if ( Overworld::IsActive()
                && Player::GetActiveVehicle() == Vehicle_Foot )
            {
                MainMenu::PlayConfirm();
                nextMenu = new SaveLoadMenu( SaveLoadMenu::Save );
                return Menu_Push;
            }
            MainMenu::PlayError();
            break;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void CommandMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    MainMenu::DrawMainPlayerText();
    if ( showPics )
        MainMenu::DrawMainPlayerPic();

    const int CmdBoxX = 176;
    const int CmdBoxY = 0;
    const int CmdBoxWidth = 80;
    const int CmdBoxHeight = 120;
    const int CmdX = CmdBoxX + 16;

    Text::DrawBox( CmdBoxX, CmdBoxY, CmdBoxWidth, CmdBoxHeight );

    Text::DrawString( "ITEM", CmdX, CmdBoxY + 16 );
    Text::DrawString( "EQUIP", CmdX, CmdBoxY + 32 );
    Text::DrawString( "MAGIC", CmdX, CmdBoxY + 48 );
    Text::DrawString( "STATUS", CmdX, CmdBoxY + 64 );
    Text::DrawString( "PARTY", CmdX, CmdBoxY + 80 );
    Text::DrawString( "SAVE", CmdX, CmdBoxY + 96 );

    const int OrbBoxX = CmdBoxX;
    const int OrbBoxY = CmdBoxHeight;
    const int OrbBoxWidth = CmdBoxWidth;
    const int OrbBoxHeight = 64;

    Text::DrawBox( OrbBoxX, OrbBoxY, OrbBoxWidth, OrbBoxHeight );

    MainMenu::DrawOrb( 0, Player::Items[Item_OrbFire] > 0, OrbBoxX + 16, OrbBoxY + 16 );
    MainMenu::DrawOrb( 1, Player::Items[Item_OrbWater] > 0, OrbBoxX + 32, OrbBoxY + 16 );
    MainMenu::DrawOrb( 2, Player::Items[Item_OrbAir] > 0, OrbBoxX + 16, OrbBoxY + 32 );
    MainMenu::DrawOrb( 3, Player::Items[Item_OrbEarth] > 0, OrbBoxX + 32, OrbBoxY + 32 );

    const int GilBoxX = CmdBoxX;
    const int GilBoxY = OrbBoxY + OrbBoxHeight;

    Text::DrawBox( CmdBoxX, GilBoxY, OrbBoxWidth, 56 );

    char str[32];

    uint32_t hours = 0;
    uint32_t minutes = 0;
    Global::GetHoursMinutes( Global::GetTime(), hours, minutes );

    sprintf_s( str, "%02u:%02u", hours, minutes );
    Text::DrawString( str, GilBoxX + 16, GilBoxY + 16 );

    sprintf_s( str, "%6d G", Player::GetG() );
    Text::DrawString( str, GilBoxX + 8, GilBoxY + 32 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = CmdBoxY + 16 + selIndex * 16;

        Text::DrawCursor( CmdX - 16, y );
    }
}

void CommandMenu::ShowPics( bool visible )
{
    if ( instance != nullptr )
        instance->showPics = visible;
}


//----------------------------------------------------------------------------


ChoosePartyMenu::ChoosePartyMenu()
    :   step( 0 ),
        selIndex( 0 ),
        firstSelIndex( 0 )
{
}

MenuAction ChoosePartyMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % Player::PartySize;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + Player::PartySize) % Player::PartySize;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();

        if ( step == 0 )
        {
            firstSelIndex = selIndex;
            step = 1;
        }
        else
        {
            step = 0;
            nextMenu = new AnimateSwitchPartyMenu( firstSelIndex, selIndex );
            return Menu_Push;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        if ( step == 0 )
            return Menu_Pop;

        step = 0;
    }

    return Menu_None;
}

void ChoosePartyMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    int y = 0;

    if ( step == 1 && (GetFrameCounter() % 4) < 2 )
    {
        y = PlayerPicY + firstSelIndex * PlayerEntryHeight;

        Text::DrawCursor( PlayerPicX - 16 + 4, y - 4 );
    }

    y = PlayerPicY + selIndex * PlayerEntryHeight;

    Text::DrawCursor( PlayerPicX - 16, y );
}


//----------------------------------------------------------------------------


AnimateSwitchPartyMenu::AnimateSwitchPartyMenu( int player1, int player2 )
    :   step( 0 ),
        playerId1( player1 ),
        playerId2( player2 ),
        xOffset( 0 ),
        yOffset( 0 ),
        targetYOffset( 0 )
{
    // the animation depends on player1 being before player2 - swap if needed
    if ( playerId1 > playerId2 )
    {
        int p = playerId1;
        playerId1 = playerId2;
        playerId2 = p;
    }

    targetYOffset = PlayerEntryHeight * (playerId2 - playerId1);

    CommandMenu::ShowPics( false );
}

MenuAction AnimateSwitchPartyMenu::Update( Menu*& nextMenu )
{
    switch ( step )
    {
    case 0:
        if ( xOffset <= -PlayerPicX )
            step = 1;
        else
            xOffset -= 2;
        break;

    case 1:
        if ( yOffset >= targetYOffset )
            step = 2;
        else
            yOffset += 2;
        break;

    case 2:
        if ( xOffset >= 0 )
        {
            Player::SwitchPlayers( playerId1, playerId2 );
            CommandMenu::ShowPics( true );
            return Menu_Pop;
        }
        else
            xOffset += 2;
        break;
    }

    return Menu_None;
}

void AnimateSwitchPartyMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    int xOffsets[Player::PartySize] = { 0 };
    int yOffsets[Player::PartySize] = { 0 };

    xOffsets[playerId1] = xOffset;
    xOffsets[playerId2] = -xOffset;

    yOffsets[playerId1] = yOffset;
    yOffsets[playerId2] = -yOffset;

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        MainMenu::DrawPlayer( 
            i, 
            PlayerPicX + xOffsets[i], 
            PlayerPicY + PlayerEntryHeight * i + yOffsets[i] );
    }
}


//----------------------------------------------------------------------------


ChooseCommandTargetMenu::ChooseCommandTargetMenu( int cmdId )
    :   cmdId( cmdId ),
        selIndex( 0 )
{
}

MenuAction ChooseCommandTargetMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % Player::PartySize;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + Player::PartySize) % Player::PartySize;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();
        switch ( cmdId )
        {
        case 1:
            nextMenu = new EquipMenu( selIndex );
            return Menu_SwitchTop;
        case 2:
            nextMenu = new MagicMenu( selIndex );
            return Menu_SwitchTop;
        case 3:
            nextMenu = new StatusMenu( selIndex );
            return Menu_SwitchTop;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseCommandTargetMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = PlayerPicY + selIndex * PlayerEntryHeight;

        Text::DrawCursor( PlayerPicX - 16, y );
    }
}
