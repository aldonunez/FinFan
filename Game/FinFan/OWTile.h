/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class OWTile
{
public:
    enum GeneralDomain
    {
        Land,
        River,
        Sea,
    };

    enum Specials
    {
        S_Chime     = 0x40,
        S_Caravan   = 0x80,
        S_Floater   = 0xc0,
    };

private:
    enum Flags
    {
        None,
        NoWalk      = 1,
        NoCanoe     = 2,
        NoShip      = 4,
        NoAirship   = 8,
        Forest      = 0x10,
        DockShip    = 0x20,
        Fight       = 0x4000,
        Teleport    = 0x8000,

        Fight_Mask  = 3,
        Fight_Shift = 8,

        Teleport_Mask   = 0x1f,
        Teleport_Shift  = 8,

        Special_Mask    = 0xc0,
    };

public:
    static bool IsForest( uint16_t attrs )
    {
        return (attrs & Forest) != 0;
    }

    static bool CanWalk( uint16_t attrs )
    {
        return (attrs & NoWalk) == 0;
    }

    static bool CanCanoe( uint16_t attrs )
    {
        return (attrs & NoCanoe) == 0;
    }

    static bool CanShip( uint16_t attrs )
    {
        return (attrs & NoShip) == 0;
    }

    static bool CanAirship( uint16_t attrs )
    {
        return (attrs & NoAirship) == 0;
    }

    static bool IsDock( uint16_t attrs )
    {
        return (attrs & DockShip) != 0;
    }

    static bool CanFight( uint16_t attrs )
    {
        return (attrs & Fight) != 0;
    }

    static GeneralDomain GetFightDomain( uint16_t attrs )
    {
        return (GeneralDomain) ((attrs >> Fight_Shift) & Fight_Mask);
    }

    static bool IsTeleport( uint16_t attrs )
    {
        return (attrs & Teleport) != 0;
    }

    static int GetTeleport( uint16_t attrs )
    {
        return (attrs >> Teleport_Shift) & Teleport_Mask;
    }

    static int GetSpecial( uint16_t attrs )
    {
        return attrs & Special_Mask;
    }
};
