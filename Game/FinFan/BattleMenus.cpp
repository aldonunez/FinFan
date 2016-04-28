/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "BattleMenus.h"
#include "Text.h"
#include "Player.h"
#include "Sound.h"


namespace Battle
{

void PlayError()
{
    Sound::PlayEffect( SEffect_Error );
}

void PlayCursor()
{
    Sound::PlayEffect( SEffect_Cursor );
}

void PlayConfirm()
{
    Sound::PlayEffect( SEffect_Confirm );
}


//----------------------------------------------------------------------------
//  BattleMenu
//----------------------------------------------------------------------------

BattleMenu::BattleMenu()
    :   selIndex( 0 )
{
}

MenuAction BattleMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selIndex = (selIndex + 1) % 4;
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selIndex = (selIndex - 1 + 4) % 4;
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        PlayConfirm();
        return AcceptAction( nextMenu );
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

MenuAction BattleMenu::AcceptAction( Menu*& nextMenu )
{
    if ( selIndex == 0 )
    {
        GetCommandBuilder().action = Action_Fight;
        nextMenu = new ChooseOneEnemyTargetMenu();
        return Menu_Push;
    }
    else if ( selIndex == 1 )
    {
        GetCommandBuilder().action = Action_Magic;
        nextMenu = new MagicMenu();
        return Menu_Push;
    }
    else if ( selIndex == 2 )
    {
        GetCommandBuilder().action = Action_Item;
        nextMenu = new ItemMenu();
        return Menu_Push;
    }
    else if ( selIndex == 3 )
    {
        Command& builder = GetCommandBuilder();
        builder.action = Action_Run;
        AddCommand( builder );
        return Menu_PopAll;
    }

    return Menu_None;
}

void BattleMenu::Draw()
{
    Text::DrawBox( BoxX, BoxY, 72, 88 );
    Text::DrawString( "FIGHT", TextX, TextY );
    Text::DrawString( "MAGIC", TextX, TextY + 1 * 16 );
    Text::DrawString( "ITEM", TextX, TextY + 2 * 16 );
    Text::DrawString( "RUN", TextX, TextY + 3 * 16 );
}

void BattleMenu::DrawCursor()
{
    Text::DrawCursor( TextX - 16, TextY + selIndex * LineHeight );
}


//----------------------------------------------------------------------------
//  MagicMenu
//----------------------------------------------------------------------------

MagicMenu::MagicMenu()
    :   selRow( 0 ),
        selCol( 0 ),
        topRow( 0 )
{
}

MenuAction MagicMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        if ( selRow == 7 )
        {
            selRow = 0;
            topRow = 0;
        }
        else
        {
            selRow++;
            if ( selRow > topRow + 3 )
                topRow = selRow - 3;
        }
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        if ( selRow == 0 )
        {
            selRow = 7;
            topRow = 7 - 3;
        }
        else
        {
            selRow--;
            if ( selRow < topRow )
                topRow = selRow;
        }
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selCol = (selCol + 1) % 3;
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selCol = (selCol - 1 + 3) % 3;
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        return AcceptAction( nextMenu );
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

MenuAction MakeTargetMenu( Target target, Menu*& nextMenu )
{
    switch ( target )
    {
    case Target_AllEnemies:
        nextMenu = new ChooseAllEnemiesTargetMenu();
        break;

    case Target_AllPlayers:
        nextMenu = new ChooseAllPlayersTargetMenu();
        break;

    case Target_OneEnemy:
        nextMenu = new ChooseOneEnemyTargetMenu();
        break;

    case Target_OnePlayer:
        nextMenu = new ChooseOnePlayerTargetMenu();
        break;

    case Target_Self:
        nextMenu = new ChooseSelfTargetMenu();
        break;

    default:
        return Menu_None;
    }

    if ( nextMenu == nullptr )
        return Menu_None;

    return Menu_Push;
}

MenuAction MagicMenu::AcceptAction( Menu*& nextMenu )
{
    Command& cmd = GetCommandBuilder();
    int playerId = cmd.actorIndex;
    Player::Character& player = Player::Party[playerId];
    int level = selRow;

    if ( player.spellCharge[level] > 0 )
    {
        int spell = player.spells[level][selCol];

        if ( spell != NoMagic )
        {
            int spellIndex = (level * 8) + (spell - 1);
            Player::MagicAttr& attr = Player::magicAttrs[spellIndex];

            cmd.actionId = spellIndex;

            PlayConfirm();
            return MakeTargetMenu( (Target) attr.Target, nextMenu );
        }
    }

    PlayError();

    return Menu_None;
}

void MagicMenu::Draw()
{
    int playerId = GetCommandBuilder().actorIndex;
    Player::Character& player = Player::Party[playerId];
    char str[16] = "";

    Text::DrawBox( BoxX, BoxY, 248, 88 );

    for ( int r = 0; r < 4; r++ )
    {
        int level = topRow + r;
        int y = TextY + r * LineHeight;

        sprintf_s( str, "Lv%d %2d-%2d", 
            level + 1, 
            player.spellCharge[level], 
            player.spellMaxCharge[level] );

        Text::DrawString( str, TextX, y );

        for ( int c = 0; c < 3; c++ )
        {
            uint8_t spell = player.spells[level][c];

            if ( spell != NoMagic )
            {
                int x = ItemX + c * ItemWidth;
                const char* name = Player::GetMagicName( level, spell );

                Text::DrawString( name, x, y );
            }
        }
    }
}

void MagicMenu::DrawCursor()
{
    int x = ItemX - 16 + selCol * ItemWidth;
    int y = TextY + (selRow - topRow) * LineHeight;

    Text::DrawCursor( x, y );
}


//----------------------------------------------------------------------------
//  ItemMenu
//----------------------------------------------------------------------------

ItemMenu::ItemMenu()
    :   selRow( 0 ),
        selCol( 0 ),
        topRow( 0 )
{
    // tally up all items the party actually has, 
    // so they're all shown in a list without holes

    int j = 0;

    for ( int i = Player::UsableItemsBaseId; i < UsableItems; i++ )
    {
        int itemId = i;

        if ( Player::Items[itemId] > 0 )
        {
            itemIds[j] = itemId;
            j++;
        }
    }

    for ( ; j < UsableItems; j++ )
    {
        itemIds[j] = NoItem;
    }
}

MenuAction ItemMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        if ( selRow < Rows - 1 )
        {
            selRow++;
            if ( selRow > topRow + 3 )
                topRow = selRow - 3;
            PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        if ( selRow > 0 )
        {
            selRow--;
            if ( selRow < topRow )
                topRow = selRow;
            PlayCursor();
        }
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selCol = (selCol + 1) % 2;
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selCol = (selCol - 1 + 2) % 2;
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        return AcceptAction( nextMenu );
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

MenuAction ItemMenu::AcceptAction( Menu*& nextMenu )
{
    Command& cmd = GetCommandBuilder();
    int index = selRow * 2 + selCol;
    int itemId = NoItem;

    if ( index < _countof( itemIds ) )
        itemId = itemIds[index];

    if ( itemId != NoItem && Player::Items[itemId] > 0 )
    {
        cmd.actionId = itemId;

        int spellId = Player::GetSpellForItem( itemId );

        if ( spellId != 0 )
        {
            Player::MagicAttr& magicAttr = Player::magicAttrs[spellId - 1];

            PlayConfirm();
            return MakeTargetMenu( (Target) magicAttr.Target, nextMenu );
        }
    }

    PlayError();

    return Menu_None;
}

void ItemMenu::Draw()
{
    char str[16] = "";

    Text::DrawBox( BoxX, BoxY, 248, 88 );

    for ( int r = 0; r < 4; r++ )
    {
        int row = topRow + r;
        int y = TextY + r * LineHeight;

        for ( int c = 0; c < 2; c++ )
        {
            int index = row * 2 + c;
            int x = TextX + c * ItemWidth;

            if ( index >= _countof( itemIds ) || itemIds[index] == NoItem )
                continue;

            int itemId = itemIds[index];
            const char* name = Player::GetItemName( itemId );

            sprintf_s( str, "%-8s %2d", 
                name, 
                Player::Items[itemId] );

            Text::DrawString( str, x, y );
        }
    }
}

void ItemMenu::DrawCursor()
{
    int x = ItemX - 16 + selCol * ItemWidth;
    int y = TextY + (selRow - topRow) * LineHeight;

    Text::DrawCursor( x, y );
}


//----------------------------------------------------------------------------
//  ChooseOneEnemyTargetMenu
//----------------------------------------------------------------------------

const int EnemyMapCells = EnemyMapCols * EnemyMapRows;


ChooseOneEnemyTargetMenu::ChooseOneEnemyTargetMenu()
    :   map( nullptr ),
        cursorX( 0 ),
        cursorY( 0 )
{
    map = GetEnemyMap();
    cell.X = EnemyMapCols - 1;
    cell.Y = EnemyMapRows - 1;
    SelectDown();
}

void ChooseOneEnemyTargetMenu::Select( Point cell )
{
    int index = map->Indexes[cell.Y][cell.X];
    cursorX = GetEnemies()[index].Bounds.X - 16;
    cursorY = GetEnemies()[index].Bounds.Y;
}

static Point MoveDown( Point cell )
{
    cell.Y++;
    if ( cell.Y >= EnemyMapRows )
    {
        cell.Y = 0;
        cell.X = (cell.X + 1) % EnemyMapCols;
    }
    return cell;
}

static Point MoveUp( Point cell )
{
    cell.Y--;
    if ( cell.Y >= EnemyMapRows )       // co-ordinates in Point are unsigned
    {
        cell.Y = EnemyMapRows - 1;
        cell.X = (cell.X - 1 + EnemyMapCols) % EnemyMapCols;
    }
    return cell;
}

static Point MoveRight( Point cell )
{
    cell.X++;
    if ( cell.X >= EnemyMapCols )
    {
        cell.X = 0;
        cell.Y = (cell.Y + 1) % EnemyMapRows;
    }
    return cell;
}

static Point MoveLeft( Point cell )
{
    cell.X--;
    if ( cell.X >= EnemyMapCols )       // co-ordinates in Point are unsigned
    {
        cell.X = EnemyMapCols - 1;
        cell.Y = (cell.Y - 1 + EnemyMapRows) % EnemyMapRows;
    }
    return cell;
}

static Point Find( Point start, const EnemyMap* map, Point (*moveProc)( Point start ) )
{
    Point p = start;

    for ( int i = 0; i < EnemyMapCells - 1; i++ )
    {
        p = moveProc( p );
        int index = map->Indexes[p.Y][p.X];

        if ( index < 0 )
            continue;
        if ( GetEnemies()[index].Type != InvalidEnemyType && GetEnemies()[index].Hp > 0 )
            return p;
    }

    return start;
}

void ChooseOneEnemyTargetMenu::SelectDown()
{
    cell = Find( cell, map, MoveDown );
    Select( cell );
}

void ChooseOneEnemyTargetMenu::SelectUp()
{
    cell = Find( cell, map, MoveUp );
    Select( cell );
}

void ChooseOneEnemyTargetMenu::SelectRight()
{
    cell = Find( cell, map, MoveRight );
    Select( cell );
}

void ChooseOneEnemyTargetMenu::SelectLeft()
{
    cell = Find( cell, map, MoveLeft );
    Select( cell );
}

MenuAction ChooseOneEnemyTargetMenu::Update( Menu*& nextMenu )
{
    int index = map->Indexes[cell.Y][cell.X];

    // The enemy could have run away or died while the cursor was over it.
    if ( GetEnemies()[index].Type == InvalidEnemyType || GetEnemies()[index].Hp == 0 )
    {
        return Menu_Pop;
    }

    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        SelectDown();
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        SelectUp();
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        SelectRight();
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        SelectLeft();
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Command& builder = GetCommandBuilder();
        builder.target = Target_OneEnemy;
        builder.targetIndex = index;
        AddCommand( builder );
        PlayConfirm();
        return Menu_PopAll;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseOneEnemyTargetMenu::Draw()
{
    if ( prevMenu != nullptr )
        prevMenu->Draw();
}

void ChooseOneEnemyTargetMenu::DrawCursor()
{
    Text::DrawCursor( cursorX, cursorY );
}


//----------------------------------------------------------------------------
//  ChooseOnePlayerTargetMenu
//----------------------------------------------------------------------------

ChooseOnePlayerTargetMenu::ChooseOnePlayerTargetMenu()
    :   index( 0 ),
        cursorX( 0 ),
        cursorY( 0 )
{
    index = FindNextPlayer( -1 );
    Select( index );
}

void ChooseOnePlayerTargetMenu::Select( int index )
{
    Command& cmd = GetCommandBuilder();

    cursorX = PartyX - 16;
    cursorY = PartyY + index * PlayerSpriteHeight;

    if ( index == cmd.actorIndex )
        cursorX -= 16;
}

void ChooseOnePlayerTargetMenu::SelectNext()
{
    index = FindNextPlayer( index );
    Select( index );
}

void ChooseOnePlayerTargetMenu::SelectPrev()
{
    index = FindPrevPlayer( index );
    Select( index );
}

int ChooseOnePlayerTargetMenu::FindNextPlayer( int index )
{
    return (index + 1) % Player::PartySize;
}

int ChooseOnePlayerTargetMenu::FindPrevPlayer( int index )
{
    return (index - 1 + Player::PartySize) % Player::PartySize;
}

MenuAction ChooseOnePlayerTargetMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        SelectNext();
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        SelectPrev();
        PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Command& builder = GetCommandBuilder();
        builder.target = Target_OnePlayer;
        builder.targetIndex = index;
        AddCommand( builder );
        PlayConfirm();
        return Menu_PopAll;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseOnePlayerTargetMenu::Draw()
{
    if ( prevMenu != nullptr )
        prevMenu->Draw();
}

void ChooseOnePlayerTargetMenu::DrawCursor()
{
    Text::DrawCursor( cursorX, cursorY );
}


//----------------------------------------------------------------------------
//  ChooseSelfTargetMenu
//----------------------------------------------------------------------------

ChooseSelfTargetMenu::ChooseSelfTargetMenu()
    :   cursorX( 0 ),
        cursorY( 0 )
{
    Command& cmd = GetCommandBuilder();
    int index = cmd.actorIndex;

    cursorX = PartyX - 16 - 16;
    cursorY = PartyY + index * PlayerSpriteHeight;
}

MenuAction ChooseSelfTargetMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Command& builder = GetCommandBuilder();
        builder.target = Target_Self;
        builder.targetIndex = builder.actorIndex;
        AddCommand( builder );
        PlayConfirm();
        return Menu_PopAll;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseSelfTargetMenu::Draw()
{
    if ( prevMenu != nullptr )
        prevMenu->Draw();
}

void ChooseSelfTargetMenu::DrawCursor()
{
    Text::DrawCursor( cursorX, cursorY );
}


//----------------------------------------------------------------------------
//  ChooseAllPlayersTargetMenu
//----------------------------------------------------------------------------

ChooseAllPlayersTargetMenu::ChooseAllPlayersTargetMenu()
    :   timer( 0 )
{
}

MenuAction ChooseAllPlayersTargetMenu::Update( Menu*& nextMenu )
{
    timer = (timer + 1) % 4;

    if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Command& builder = GetCommandBuilder();
        builder.target = Target_AllPlayers;
        AddCommand( builder );
        PlayConfirm();
        return Menu_PopAll;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseAllPlayersTargetMenu::Draw()
{
    if ( prevMenu != nullptr )
        prevMenu->Draw();
}

void ChooseAllPlayersTargetMenu::DrawCursor()
{
    Command& cmd = GetCommandBuilder();

    if ( timer < 2 )
    {
        for ( int i = 0; i < Player::PartySize; i++ )
        {
            int x = PartyX - 16;

            if ( i == cmd.actorIndex )
                x -= 16;

            Text::DrawCursor( x, PartyY + i * PlayerSpriteHeight );
        }
    }
}


//----------------------------------------------------------------------------
//  ChooseAllEnemiesTargetMenu
//----------------------------------------------------------------------------

ChooseAllEnemiesTargetMenu::ChooseAllEnemiesTargetMenu()
    :   timer( 0 )
{
}

MenuAction ChooseAllEnemiesTargetMenu::Update( Menu*& nextMenu )
{
    timer = (timer + 1) % 4;

    if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Command& builder = GetCommandBuilder();
        builder.target = Target_AllEnemies;
        AddCommand( builder );
        PlayConfirm();
        return Menu_PopAll;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseAllEnemiesTargetMenu::Draw()
{
    if ( prevMenu != nullptr )
        prevMenu->Draw();
}

void ChooseAllEnemiesTargetMenu::DrawCursor()
{
    if ( timer < 2 )
    {
        Enemy* enemies = GetEnemies();

        for ( int i = 0; i < MaxEnemies; i++ )
        {
            if ( enemies[i].Type != InvalidEnemyType && enemies[i].Hp > 0 )
                Text::DrawCursor( enemies[i].Bounds.X - 16, enemies[i].Bounds.Y );
        }
    }
}

}
