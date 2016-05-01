/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


namespace Battle
{
    void GotoFirstState();
    void UpdateState();
    int GetResultCount();

#if defined( ATB )
    enum
    {
        ReadyTime       = 0xFFFF,
    };

    struct AtbActor;

    AtbActor* GetAtbPlayer( int index );
#endif
}
