/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once

namespace Player
{
    struct MagicAttr;
}


typedef void (*CalcMagicFunc)( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result );


CalcMagicFunc GetMagicProc( int effect );
