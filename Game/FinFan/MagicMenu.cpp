/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Menus.h"
#include "Text.h"
#include "Player.h"
#include "SceneStack.h"


typedef void (*SpellRoutine)( int casterId, int targetId, int spellId );


static void CastNone( int casterId, int targetId, int spellId )
{
    MainMenu::PlayError();
}

static void CastCure( int casterId, int targetId, int spellId, int hp )
{
    Player::Character& caster = Player::Party[casterId];
    Player::Character& target = Player::Party[targetId];

    if ( target.IsAlive() && target.hp < target.maxHp )
    {
        int level = spellId / 8;

        target.AddHp( hp );
        caster.spellCharge[level]--;

        MainMenu::PlayPotion();
    }
    else
    {
        MainMenu::PlayError();
    }
}

static void CastCure1( int casterId, int targetId, int spellId )
{
    int r = GetNextRandom( 16 );

    CastCure( casterId, targetId, spellId, r + 16 );
}

static void CastCure2( int casterId, int targetId, int spellId )
{
    int r = GetNextRandom( 32 );

    CastCure( casterId, targetId, spellId, r + 32 );
}

static void CastCure3( int casterId, int targetId, int spellId )
{
    int r = GetNextRandom( 64 );

    CastCure( casterId, targetId, spellId, r + 64 );
}

static void CastCure4( int casterId, int targetId, int spellId )
{
    Player::Character& caster = Player::Party[casterId];
    Player::Character& target = Player::Party[targetId];

    if ( target.IsAlive() )
    {
        int level = spellId / 8;

        target.SetHp( target.GetMaxHp() );
        target.RemoveStatus( Status_AllLiving );
        caster.spellCharge[level]--;

        MainMenu::PlayPotion();
    }
    else
    {
        MainMenu::PlayError();
    }
}

static void CastHeal( int casterId, int spellId, int hp )
{
    Player::Character& caster = Player::Party[casterId];

    for ( int i = 0; i < Player::PartySize; i++ )
    {
        Player::Character& player = Player::Party[i];

        if ( player.IsAlive() )
        {
            player.AddHp( hp );
        }
    }

    int level = spellId / 8;

    caster.spellCharge[level]--;

    MainMenu::PlayPotion();
}

static void CastHeal1( int casterId, int targetId, int spellId )
{
    int r = GetNextRandom( 8 );

    CastHeal( casterId, spellId, 16 + r );
}

static void CastHeal2( int casterId, int targetId, int spellId )
{
    int r = GetNextRandom( 16 );

    CastHeal( casterId, spellId, 32 + r );
}

static void CastHeal3( int casterId, int targetId, int spellId )
{
    int r = GetNextRandom( 32 );

    CastHeal( casterId, spellId, 64 + r );
}

static bool CastRemoveStatus( int casterId, int targetId, int spellId, Status status )
{
    Player::Character& caster = Player::Party[casterId];
    Player::Character& target = Player::Party[targetId];

    if ( (target.status & status) != 0 )
    {
        int level = spellId / 8;

        target.RemoveStatus( status );
        caster.spellCharge[level]--;

        MainMenu::PlayPotion();
        return true;
    }
    else
    {
        MainMenu::PlayError();
        return false;
    }
}

static void CastPure( int casterId, int targetId, int spellId )
{
    CastRemoveStatus( casterId, targetId, spellId, Status_Poison );
}

static void CastSoft( int casterId, int targetId, int spellId )
{
    CastRemoveStatus( casterId, targetId, spellId, Status_Stone );
}

static void CastLife1( int casterId, int targetId, int spellId )
{
    if ( CastRemoveStatus( casterId, targetId, spellId, Status_Death ) )
    {
        Player::Character& target = Player::Party[targetId];

        target.hp = 1;
    }
}

static void CastLife2( int casterId, int targetId, int spellId )
{
    if ( CastRemoveStatus( casterId, targetId, spellId, Status_Death ) )
    {
        Player::Character& target = Player::Party[targetId];

        target.hp = target.maxHp;
    }
}

static void CastWarp( int casterId, int targetId, int spellId )
{
    Player::Character& caster = Player::Party[casterId];
    int level = spellId / 8;

    SceneStack::PopLevel();

    caster.spellCharge[level]--;
    MainMenu::PlayPotion();
}

static void CastExit( int casterId, int targetId, int spellId )
{
    Player::Character& caster = Player::Party[casterId];
    int level = spellId / 8;

    SceneStack::PopAllLevels();

    caster.spellCharge[level]--;
    MainMenu::PlayPotion();
}


SpellRoutine spellRoutines[] = 
{
/* 00 */    CastCure1,
/* 01 */    CastNone,
/* 02 */    CastNone,
/* 03 */    CastNone,
/* 04 */    CastNone,
/* 05 */    CastNone,
/* 06 */    CastNone,
/* 07 */    CastNone,
/* 08 */    CastNone,
/* 09 */    CastNone,
/* 0A */    CastNone,
/* 0B */    CastNone,
/* 0C */    CastNone,
/* 0D */    CastNone,
/* 0E */    CastNone,
/* 0F */    CastNone,
/* 10 */    CastCure2,
/* 11 */    CastNone,
/* 12 */    CastNone,
/* 13 */    CastHeal1,
/* 14 */    CastNone,
/* 15 */    CastNone,
/* 16 */    CastNone,
/* 17 */    CastNone,
/* 18 */    CastPure,
/* 19 */    CastNone,
/* 1A */    CastNone,
/* 1B */    CastNone,
/* 1C */    CastNone,
/* 1D */    CastNone,
/* 1E */    CastNone,
/* 1F */    CastNone,
/* 20 */    CastCure3,
/* 21 */    CastLife1,
/* 22 */    CastNone,
/* 23 */    CastHeal2,
/* 24 */    CastNone,
/* 25 */    CastNone,
/* 26 */    CastWarp,
/* 27 */    CastNone,
/* 28 */    CastSoft,
/* 29 */    CastExit,
/* 2A */    CastNone,
/* 2B */    CastNone,
/* 2C */    CastNone,
/* 2D */    CastNone,
/* 2E */    CastNone,
/* 2F */    CastNone,
/* 30 */    CastCure4,
/* 31 */    CastNone,
/* 32 */    CastNone,
/* 33 */    CastHeal3,
/* 34 */    CastNone,
/* 35 */    CastNone,
/* 36 */    CastNone,
/* 37 */    CastNone,
/* 38 */    CastLife2,
/* 39 */    CastNone,
/* 3A */    CastNone,
/* 3B */    CastNone,
/* 3C */    CastNone,
/* 3D */    CastNone,
/* 3E */    CastNone,
/* 3F */    CastNone,
};


static SpellRoutine GetSpellRoutine( int spellId )
{
    if ( spellId < 0 || spellId >= _countof( spellRoutines ) )
        return CastNone;

    return spellRoutines[spellId];
}


//----------------------------------------------------------------------------

const int SpellX = 96;
const int SpellY = 48;
const int SpellWidth = 7 * 8;


MagicMenu::MagicMenu( int playerId )
    :   playerId( playerId ),
        selIndex( 0 )
{
}

MenuAction MagicMenu::Update( Menu*& nextMenu )
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
            nextMenu = new ChooseMagicMenu( playerId, Magic_Use );
        else
            nextMenu = new ChooseMagicMenu( playerId, Magic_Discard );

        return Menu_Push;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        nextMenu = new CommandMenu();
        return Menu_SwitchTop;
    }

    return Menu_None;
}

void MagicMenu::Draw( MenuDrawState state )
{
    Text::DrawBox( 0, 0, 176, 24 );
    Text::DrawString( "Use", 32, 8 );
    Text::DrawString( "Discard", 104, 8 );

    Text::DrawBox( 176, 0, 80, 24 );
    Text::DrawString( "Magic", 192, 8 );

    Text::DrawBox( 0, 24, 256, 176 );
    Text::DrawBox( 0, 176+24, 256, 40 );

    Player::Character& player = Player::Party[playerId];
    char str[16] = "";

    for ( int i = 0; i < 8; i++ )
    {
        int level = i;
        int y = SpellY + i * 16;

        sprintf_s( str, "Lv%d %2d/%2d", 
            level + 1, 
            player.spellCharge[level], 
            player.spellMaxCharge[level] );

        Text::DrawString( str, 16, y );

        for ( int c = 0; c < 3; c++ )
        {
            uint8_t spell = player.spells[level][c];

            if ( spell != NoMagic )
            {
                int x = SpellX + c * SpellWidth;
                const char* name = Player::GetMagicName( level, spell );

                Text::DrawString( name, x, y );
            }
        }
    }

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = 16 + selIndex * (104-32);

        Text::DrawCursor( x, 8 );
    }
}


//----------------------------------------------------------------------------


ChooseMagicMenu::ChooseMagicMenu( int playerId, MagicAction action )
    :   playerId( playerId ),
        action( action ),
        selCol( 0 ),
        selRow( 0 )
{
}

MenuAction ChooseMagicMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selCol = (selCol + 1) % Cols;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selCol = (selCol - 1 + Cols) % Cols;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selRow = (selRow + 1) % Rows;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selRow = (selRow - 1 + Rows) % Rows;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        Player::Character& player = Player::Party[playerId];
        uint8_t sil = player.spells[selRow][selCol];

        if ( sil != NoMagic )
        {
            if ( action == Magic_Use )
            {
                return CastSpell( nextMenu );
            }
            else
            {
                MainMenu::PlayConfirm();
                nextMenu = new ConfirmDiscardMenu( playerId, selRow, selCol );
                return Menu_Push;
            }
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

MenuAction ChooseMagicMenu::CastSpell( Menu*& nextMenu )
{
    Player::Character& player = Player::Party[playerId];
    uint8_t sil = player.spells[selRow][selCol];

    if ( player.spellCharge[selRow] > 0 )
    {
        int spellId = selRow * 8 + sil - 1;

        if ( MainMenu::SpellTargetsPlayer( spellId ) )
        {
            nextMenu = new ChooseMagicTargetMenu( playerId, spellId );
            return Menu_Push;
        }
        else
        {
            SpellRoutine routine = GetSpellRoutine( spellId );

            routine( playerId, -1, spellId );
        }
    }
    else
    {
        MainMenu::PlayError();
    }

    return Menu_None;
}

void ChooseMagicMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = SpellX + selCol * SpellWidth - 16;
        int y = SpellY + selRow * 16;

        Text::DrawCursor( x, y );
    }
}


//----------------------------------------------------------------------------


ConfirmDiscardMenu::ConfirmDiscardMenu( int playerId, int level, int col )
    :   playerId( playerId ),
        level( level ),
        col( col ),
        selIndex( 1 )
{
}

MenuAction ConfirmDiscardMenu::Update( Menu*& nextMenu )
{
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
        MainMenu::PlayConfirm();

        if ( selIndex == 0 )
        {
            DiscardMagic();
        }

        return Menu_Pop;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ConfirmDiscardMenu::Draw( MenuDrawState state )
{
    const int BoxX = 48;
    const int BoxY = 64;
    const int ChoiceX = BoxX + 32;
    const int ChoiceY = BoxY + 32;

    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    Text::DrawBox( BoxX, BoxY, 128+32, 128 );

    Player::Character& player = Player::Party[playerId];
    uint8_t spell = player.spells[level][col];
    char str[32];
    const char* name = Player::GetMagicName( level, spell );

    sprintf_s( str, "Discard %s?", name );

    Text::DrawString( str, BoxX+16, BoxY+16 );
    Text::DrawString( "Yes", ChoiceX, ChoiceY );
    Text::DrawString( "No", ChoiceX, ChoiceY + 16 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = ChoiceX - 16;
        int y = ChoiceY + selIndex * 16;

        Text::DrawCursor( x, y );
    }
}

void ConfirmDiscardMenu::DiscardMagic()
{
    Player::Character& player = Player::Party[playerId];
    player.spells[level][col] = NoMagic;
}


//----------------------------------------------------------------------------


ChooseMagicTargetMenu::ChooseMagicTargetMenu( int casterId, int spellId )
    :   casterId( casterId ),
        spellId( spellId ),
        selIndex( 0 ),
        timer( 0 )
{
}

MenuAction ChooseMagicTargetMenu::Update( Menu*& nextMenu )
{
    if ( timer > 0 )
    {
        timer--;
        return Menu_None;
    }

    int level = spellId / 8;

    if ( Player::Party[casterId].spellCharge[level] == 0 )
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
        CastSpell();

        timer = 15;

        return Menu_None;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseMagicTargetMenu::CastSpell()
{
    SpellRoutine routine = GetSpellRoutine( spellId );

    routine( casterId, selIndex, spellId );
}

void ChooseMagicTargetMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    MainMenu::DrawMainPlayerInfo();

    int level = spellId / 8;
    int sil = spellId % 8 + 1;

    char str[32];
    const char* spellName = Player::GetMagicName( level, sil );

    Text::DrawBox( 176, 0, 80, 32+3*8 );

    Text::DrawString( spellName, 176 + 8, 16 );

    sprintf_s( str, "Total %2d", Player::Party[casterId].spellCharge[level] );

    Text::DrawString( str, 176 + 8, 32 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int y = PlayerPicY + selIndex * PlayerEntryHeight;

        Text::DrawCursor( PlayerPicX - 16, y );
    }
}
