/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Title.h"
#include "MainMenu.h"
#include "Text.h"
#include "Player.h"
#include "SceneStack.h"
#include "SaveLoadMenu.h"
#include "Sound.h"


const char* DefaultName = "????";


class NewGameMenu : public Menu
{
    int selCol;
    int selRow;
    Player::PlayerInfo players[Players];

public:
    NewGameMenu();

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

    void SetClass( int classId );
    void SetName( const char* name );
};


class ChooseClassMenu : public Menu
{
    NewGameMenu* parent;
    int selCol;
    int selRow;

public:
    ChooseClassMenu( NewGameMenu* parent );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    int GetSelectedClass();
};


class ChooseNameMenu : public Menu
{
    NewGameMenu* parent;
    int selCol;
    int selRow;
    int nameLength;
    char name[NameLength + 1];

public:
    ChooseNameMenu( NewGameMenu* parent, const char* name );

    virtual MenuAction Update( Menu*& nextMenu );
    virtual void Draw( MenuDrawState state );

private:
    void InsertChar();
    void DeleteChar();
};


//----------------------------------------------------------------------------
//  TitleScene
//----------------------------------------------------------------------------

const int TitleItemX = 96;
const int TitleItemY = 96;


TitleScene::TitleScene()
    :   selIndex( 0 )
{
    Sound::PlayTrack( Sound_Prelude, 0, true );
}

void TitleScene::Update()
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
            SceneStack::SwitchScene( SceneId_NewGame );
        else if ( selIndex == 1 )
            SceneStack::SwitchScene( SceneId_LoadGame );
    }
}

void TitleScene::Draw()
{
    al_clear_to_color( al_map_rgb( 0, 0, 0 ) );

    Text::DrawString( "NEW GAME", TitleItemX, TitleItemY );
    Text::DrawString( "LOAD GAME", TitleItemX, TitleItemY + 16 );

    int y = TitleItemY + selIndex * 16;

    Text::DrawCursor( TitleItemX - 16, y );
}

IPlayfield* TitleScene::AsPlayfield()
{
    return nullptr;
}

IModule* TitleScene::Make()
{
    TitleScene* screen = new TitleScene();
    return screen;
}


//----------------------------------------------------------------------------
//  NewGameScene
//----------------------------------------------------------------------------

NewGameScene::NewGameScene()
{
}

void NewGameScene::Init()
{
    NewGameMenu* newGameMenu = new NewGameMenu();

    mainMenu.InitHost( newGameMenu );
}

void NewGameScene::Update()
{
    mainMenu.Update();
}

void NewGameScene::Draw()
{
    mainMenu.Draw();
}

IPlayfield* NewGameScene::AsPlayfield()
{
    return nullptr;
}

IModule* NewGameScene::Make()
{
    NewGameScene* screen = new NewGameScene();
    screen->Init();
    return screen;
}


//----------------------------------------------------------------------------
//  NewGameMenu
//----------------------------------------------------------------------------

const int OptionBoxX = 128;
const int OptionBoxY = 24;


NewGameMenu::NewGameMenu()
    :   selCol( 0 ),
        selRow( 0 )
{
    players[0].Class = Class_Fighter;
    players[1].Class = Class_Thief;
    players[2].Class = Class_WhiteMage;
    players[3].Class = Class_BlackMage;

    for ( int i = 0; i < Players; i++ )
        strcpy_s( players[i].Name, DefaultName );
}

MenuAction NewGameMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selCol = (selCol + 1) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selCol = (selCol - 1 + 2) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selRow = (selRow + 1) % Players;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selRow = (selRow - 1 + Players) % Players;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();
        if ( selCol == 0 )
        {
            nextMenu = new ChooseClassMenu( this );
            return Menu_Push;
        }
        else if ( selCol == 1 )
        {
            nextMenu = new ChooseNameMenu( this, players[selRow].Name );
            return Menu_Push;
        }
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        SceneStack::SwitchScene( SceneId_Title );
    }
    else if ( Input::IsKeyPressing( MenuKey ) )
    {
        MainMenu::PlayConfirm();
        Player::InitFile( players );
        Point pos = Player::GetPlayerPos();
        SceneStack::SwitchToField( pos.X, pos.Y );
    }

    return Menu_None;
}

void NewGameMenu::Draw( MenuDrawState state )
{
    const int PicX = 16;
    const int NameX = 16 + 32;
    const int EntryY = 24+16;
    const int EntryHeight = 40;

    Text::DrawBox( 0, 0, 256, 24 );
    Text::DrawStringCenter( "New Game", 0, 8, 256 );

    Text::DrawBox( 0, 24, 128, 216 );

    for ( int i = 0; i < _countof( players ); i++ )
    {
        const char* className = Player::GetClassName( players[i].Class );
        int y = EntryY + EntryHeight * i;

        MainMenu::DrawClass( players[i].Class, PicX, y );
        Text::DrawString( players[i].Name, NameX, y );
        Text::DrawString( className, NameX, y + 16 );
    }

    Text::DrawBox( OptionBoxX, OptionBoxY, 128, 216 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = (selCol == 0 ? PicX : NameX) - 16;
        int y = EntryY + EntryHeight * selRow;

        Text::DrawCursor( x, y );
    }
}

void NewGameMenu::SetClass( int classId )
{
    players[selRow].Class = classId;
}

void NewGameMenu::SetName( const char* name )
{
    strcpy_s( players[selRow].Name, name );
}


//----------------------------------------------------------------------------
//  ChooseClassMenu
//----------------------------------------------------------------------------

ChooseClassMenu::ChooseClassMenu( NewGameMenu* parent )
    :   parent( parent ),
        selCol( 0 ),
        selRow( 0 )
{
}

MenuAction ChooseClassMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selCol = (selCol + 1) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selCol = (selCol - 1 + 2) % 2;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selRow = (selRow + 1) % 3;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selRow = (selRow - 1 + 3) % 3;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        int classId = GetSelectedClass();

        parent->SetClass( classId );
        MainMenu::PlayConfirm();

        return Menu_Pop;
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        return Menu_Pop;
    }

    return Menu_None;
}

void ChooseClassMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    const int EntryX = 128 + 16;
    const int EntryY = 24+16;
    const int EntryWidth = 48;
    const int EntryHeight = 56;

    for ( int r = 0; r < 3; r++ )
    {
        for ( int c = 0; c < 2; c++ )
        {
            int x = EntryX + EntryWidth * c;
            int y = EntryY + EntryHeight * r;
            int classId = r * 2 + c;

            MainMenu::DrawClass( classId, x, y + 16 );
        }
    }

    int selClassId = GetSelectedClass();
    const char* className = Player::GetClassName( selClassId );
    Text::DrawString( className, OptionBoxX + 16, OptionBoxY + 16 );

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = EntryX + EntryWidth * selCol - 16;
        int y = EntryY + EntryHeight * selRow + 16;

        Text::DrawCursor( x, y );
    }
}

int ChooseClassMenu::GetSelectedClass()
{
    return selRow * 2 + selCol;
}


//----------------------------------------------------------------------------
//  ChooseNameMenu
//----------------------------------------------------------------------------

const int LineCount = 8;
const int LineLength = 19;
const int LineCols = 10;

const char chars[LineCount][LineLength+1] = 
{
    "A B C D E F G H I J",
    "K L M N O P Q R S T",
    "U V W X Y Z        ",
    "a b c d e f g h i j",
    "k l m n o p q r s t",
    "u v w x y z        ",
    "0 1 2 3 4 5 6 7 8 9",
    "                END"
};


ChooseNameMenu::ChooseNameMenu( NewGameMenu* parent, const char* name )
    :   parent( parent ),
        selCol( 0 ),
        selRow( 0 ),
        nameLength( 0 )
{
    if ( strcmp( name, DefaultName ) == 0 )
    {
        this->name[0] = '\0';
    }
    else
    {
        strcpy_s( this->name, name );
        nameLength = strlen( name );
    }
}

MenuAction ChooseNameMenu::Update( Menu*& nextMenu )
{
    if ( Input::IsKeyPressing( ALLEGRO_KEY_RIGHT ) )
    {
        selCol = (selCol + 1) % LineCols;

        if ( selRow == LineCount - 1 && selCol == LineCols - 1 )
            selCol = 0;

        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_LEFT ) )
    {
        selCol = (selCol - 1 + LineCols) % LineCols;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_DOWN ) )
    {
        selRow = (selRow + 1) % LineCount;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ALLEGRO_KEY_UP ) )
    {
        selRow = (selRow - 1 + LineCount) % LineCount;
        MainMenu::PlayCursor();
    }
    else if ( Input::IsKeyPressing( ConfirmKey ) )
    {
        MainMenu::PlayConfirm();

        if ( selRow == LineCount - 1 && selCol == LineCols - 2 )
        {
            parent->SetName( name );
            return Menu_Pop;
        }

        InsertChar();
    }
    else if ( Input::IsKeyPressing( CancelKey ) )
    {
        if ( nameLength == 0 )
            return Menu_Pop;

        DeleteChar();
    }

    if ( selRow == LineCount - 1 && selCol == LineCols - 1 )
        selCol--;

    return Menu_None;
}

void ChooseNameMenu::InsertChar()
{
    int index = selCol * 2;
    char ch = chars[selRow][index];

    if ( nameLength < NameLength )
    {
        name[nameLength] = ch;
        nameLength++;
        name[nameLength] = '\0';
    }
    else
    {
        // replace the last char, if name is full
        name[nameLength - 1] = ch;
    }
}

void ChooseNameMenu::DeleteChar()
{
    if ( nameLength > 0 )
    {
        nameLength--;
        name[nameLength] = '\0';
    }
}

void ChooseNameMenu::Draw( MenuDrawState state )
{
    if ( prevMenu != nullptr )
        prevMenu->Draw( MenuDraw_Paused );

    const int BoxX = 32;
    const int BoxY = 24;
    const int NameX = BoxX + 16;
    const int NameY = BoxY + 16;
    const int CharsX = BoxX + 16;
    const int CharsY = BoxY + 48;

    Text::DrawBox( BoxX, BoxY, 192, 176 );
    Text::DrawString( name, NameX, NameY );

    for ( int i = 0; i < LineCount; i++ )
    {
        Text::DrawString( chars[i], CharsX, CharsY + i*16 );
    }

    if ( state == MenuDraw_Active || (GetFrameCounter() % 4) < 2 )
    {
        int x = CharsX + 16 * selCol - 16;
        int y = CharsY + 16 * selRow;

        Text::DrawCursor( x, y );
    }
}


//----------------------------------------------------------------------------
//  SaveLoadScene
//----------------------------------------------------------------------------

SaveLoadScene::SaveLoadScene()
{
}

void SaveLoadScene::InitSave()
{
    SaveLoadMenu* menu = new SaveLoadMenu( SaveLoadMenu::Save );

    mainMenu.InitHost( menu );
}

void SaveLoadScene::InitLoad()
{
    SaveLoadMenu* menu = new SaveLoadMenu( SaveLoadMenu::Load );

    mainMenu.InitHost( menu );
}

void SaveLoadScene::Update()
{
    mainMenu.Update();
}

void SaveLoadScene::Draw()
{
    mainMenu.Draw();
}

IPlayfield* SaveLoadScene::AsPlayfield()
{
    return nullptr;
}

IModule* SaveLoadScene::MakeSave()
{
    SaveLoadScene* screen = new SaveLoadScene();
    screen->InitSave();
    return screen;
}

IModule* SaveLoadScene::MakeLoad()
{
    SaveLoadScene* screen = new SaveLoadScene();
    screen->InitLoad();
    return screen;
}
