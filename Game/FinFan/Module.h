/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class IPlayfield
{
public:
    virtual Point GetCurrentPos() = 0;
    virtual int GetInRoom() = 0;

    virtual void HandleShopClosed() = 0;
    virtual void HandleFightEnded() = 0;
    virtual void HandleOpeningEnded() = 0;
};


class IModule
{
public:
    virtual ~IModule();

    virtual void Update() = 0;
    virtual void Draw() = 0;

    virtual IPlayfield* AsPlayfield() = 0;
};
