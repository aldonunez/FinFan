/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

#include "Module.h"


class BattleMod : public IModule
{
public:
    ~BattleMod();

    void Init( int formationId, int backdropId );

    virtual void Update();
    virtual void Draw();

    virtual IPlayfield* AsPlayfield();
};
