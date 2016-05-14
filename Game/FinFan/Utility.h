/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


template <typename T>
void ShuffleArray( T* array, int length )
{
    for ( int i = length - 1; i >= 1; i-- )
    {
        int r = GetNextRandom( i + 1 );
        T orig = array[i];
        array[i] = array[r];
        array[r] = orig;
    }
}

