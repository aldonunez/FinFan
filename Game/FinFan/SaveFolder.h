/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


const int Slots = 20;


struct FileSummary
{
    uint8_t         State;
    uint8_t         Level;
    uint16_t        MaxHp;
    uint32_t        TimeMillis;
    uint32_t        Gil;
    char            Name[8];
    uint8_t         Classes[Players];
    uint8_t         Orbs[4];
    uint8_t         Reserved[4];
};

struct SummarySet
{
    FileSummary Summaries[Slots];
};


class SaveFolder
{
public:
    static bool SaveFile( int slot );
    static bool LoadFile( int slot );

    static bool ReadSummaries( SummarySet& set );
};
