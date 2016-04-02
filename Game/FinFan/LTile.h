/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


class LTile
{
public:
    enum TeleportType
    {
        TT_None,
        TT_Warp,
        TT_Swap,
        TT_Exit,
    };

    enum Specials
    {
        S_OpenDoor  = 2,
        S_Locked    = 4,
        S_CloseDoor = 6,
        S_Treasure  = 8,
        S_Battle    = 0xa,
        S_Damage    = 0xc,
        S_Crown     = 0xe,
        S_Cube      = 0x10,
        S_4Orbs     = 0x12,
        S_UseRod    = 0x14,
        S_UseLute   = 0x16,
        S_EarthOrb  = 0x18,
        S_FireOrb   = 0x1a,
        S_WaterOrb  = 0x1c,
        S_AirOrb    = 0x1e,
    };

private:
    enum Flags
    {
        None,
        NoWalk      = 1,

        TeleportType_Mask   = 3,
        TeleportType_Shift  = 6,

        Teleport_Mask   = 0x3f,
        Teleport_Shift  = 8,

        Special_Mask    = 0x1e,

        Chest_Mask      = 0xff,
        Chest_Shift     = 8,

        Message_Mask    = 0xff,
        Message_Shift   = 8,

        Shop_Mask       = 0xff,
        Shop_Shift      = 8,

        NoText_Mask     = 0xc2,

        Fight_Mask      = 0xff,
        Fight_Shift     = 8,
    };

public:
    static bool CanWalk( uint16_t attrs )
    {
        return (attrs & NoWalk) == 0;
    }

    static TeleportType GetTeleportType( uint16_t attrs )
    {
        return (TeleportType) ((attrs >> TeleportType_Shift) & TeleportType_Mask);
    }

    static int GetTeleport( uint16_t attrs )
    {
        return (attrs >> Teleport_Shift) & Teleport_Mask;
    }

    static int GetSpecial( uint16_t attrs )
    {
        return attrs & Special_Mask;
    }

    static int GetChest( uint16_t attrs )
    {
        return (attrs >> Chest_Shift) & Chest_Mask;
    }

    static bool HasMessage( uint16_t attrs )
    {
        return (attrs & NoText_Mask) == 0;
    }

    static int GetMessage( uint16_t attrs )
    {
        return (attrs >> Message_Shift) & Message_Mask;
    }

    static int GetShop( uint16_t attrs )
    {
        return (attrs >> Shop_Shift) & Shop_Mask;
    }

    static int GetFormation( uint16_t attrs )
    {
        return ((attrs >> Fight_Shift) & Fight_Mask);
    }

    static bool IsRandomBattle( int formationId )
    {
        return formationId >= 128;
    }
};
