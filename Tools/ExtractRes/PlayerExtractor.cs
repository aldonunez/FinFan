/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.Text;
using System.IO;

namespace ExtractRes
{
    class PlayerExtractor
    {
        internal static void Extract( Options options )
        {
            WriteChargeFile( @"E:\temp\chargeBoost.dat" );
        }

        static byte[] MakeEvenSpellCharge( int maxLevel )
        {
            byte[] buf = new byte[50];
            int chargeBoostBits = 0;

            chargeBoostBits = (1 << maxLevel) - 1;

            for ( int i = 1; i <= 50; i++ )
            {
                if ( i >= 15 )
                    buf[i - 1] = (byte) chargeBoostBits;
                // else, leave the element 0
            }

            return buf;
        }

        static byte[] ReadSpellCharges( byte[][] maxCharges )
        {
            byte[] buf = new byte[50];
            MemoryStream stream = new MemoryStream( buf );
            byte[] lastCharges = new byte[8];

            for ( int l = 0; l < 50; l++ )
            {
                byte[] charges = maxCharges[l];
                int chargeBoostBits = 0;

                for ( int i = 0; i < 8; i++ )
                {
                    if ( charges[i] > lastCharges[i] )
                        chargeBoostBits |= (1 << i);
                }

                stream.WriteByte( (byte) chargeBoostBits );
                lastCharges = charges;
            }

            return buf;
        }

        // Fighter      Knight
        // Thief        Ninja
        // BlackBelt    Master
        // RedMage      RedWizard
        // WhiteMage    WhiteWizard
        // BlackMage    BlackWizard

        static void WriteChargeFile( string path )
        {

            byte[] red = ReadSpellCharges( RedMaxCharges );
            byte[] bw = ReadSpellCharges( BlackWhiteMaxCharges );
            byte[] knight = MakeEvenSpellCharge( 3 );
            byte[] ninja = MakeEvenSpellCharge( 4 );
            byte[] none = MakeEvenSpellCharge( 0 );

            using ( FileStream stream = File.OpenWrite( path ) )
            {
                stream.SetLength( 0 );

                stream.Write( none, 0, none.Length );
                stream.Write( none, 0, none.Length );
                stream.Write( none, 0, none.Length );
                stream.Write( red, 0, red.Length );
                stream.Write( bw, 0, bw.Length );
                stream.Write( bw, 0, bw.Length );

                stream.Write( knight, 0, knight.Length );
                stream.Write( ninja, 0, ninja.Length );
                stream.Write( none, 0, none.Length );
                stream.Write( red, 0, red.Length );
                stream.Write( bw, 0, bw.Length );
                stream.Write( bw, 0, bw.Length );
            }
        }

        static byte[][] BlackWhiteMaxCharges = 
        {
            new byte[] { 2, 0, 0, 0, 0, 0, 0, 0 },
            new byte[] { 3, 1, 0, 0, 0, 0, 0, 0 },
            new byte[] { 3, 2, 0, 0, 0, 0, 0, 0 },
            new byte[] { 4, 2, 0, 0, 0, 0, 0, 0 },
            new byte[] { 4, 3, 1, 0, 0, 0, 0, 0 },
            new byte[] { 4, 3, 2, 0, 0, 0, 0, 0 },
            new byte[] { 5, 3, 2, 0, 0, 0, 0, 0 },
            new byte[] { 5, 3, 3, 1, 0, 0, 0, 0 },
            new byte[] { 5, 4, 3, 2, 0, 0, 0, 0 },
            new byte[] { 6, 4, 3, 2, 0, 0, 0, 0 },
            new byte[] { 6, 4, 4, 3, 0, 0, 0, 0 },
            new byte[] { 6, 4, 4, 3, 1, 0, 0, 0 },
            new byte[] { 6, 5, 4, 3, 2, 0, 0, 0 },
            new byte[] { 7, 5, 4, 4, 2, 0, 0, 0 },
            new byte[] { 7, 5, 5, 4, 3, 0, 0, 0 },
            new byte[] { 7, 5, 5, 4, 3, 1, 0, 0 },
            new byte[] { 7, 6, 5, 4, 3, 2, 0, 0 },
            new byte[] { 7, 6, 5, 5, 4, 2, 0, 0 },
            new byte[] { 8, 6, 5, 5, 4, 3, 0, 0 },
            new byte[] { 8, 6, 6, 5, 4, 3, 1, 0 },
            new byte[] { 8, 6, 6, 5, 4, 3, 2, 0 },
            new byte[] { 8, 6, 6, 5, 5, 4, 2, 0 },
            new byte[] { 8, 6, 6, 6, 5, 4, 3, 0 },
            new byte[] { 9, 7, 6, 6, 5, 4, 3, 0 },
            new byte[] { 9, 7, 6, 6, 5, 5, 3, 1 },
            new byte[] { 9, 7, 6, 6, 5, 5, 4, 2 },
            new byte[] { 9, 7, 7, 6, 6, 5, 4, 2 },
            new byte[] { 9, 8, 7, 6, 6, 5, 4, 3 },
            new byte[] { 9, 8, 7, 7, 6, 5, 5, 3 },
            new byte[] { 9, 8, 7, 7, 6, 6, 5, 4 },
            new byte[] { 9, 8, 8, 7, 6, 6, 5, 4 },
            new byte[] { 9, 8, 8, 7, 7, 6, 5, 4 },
            new byte[] { 9, 8, 8, 7, 7, 6, 5, 5 },
            new byte[] { 9, 8, 8, 7, 7, 6, 6, 5 },
            new byte[] { 9, 8, 8, 8, 7, 6, 6, 5 },
            new byte[] { 9, 8, 8, 8, 7, 7, 6, 5 },
            new byte[] { 9, 8, 8, 8, 7, 7, 6, 6 },
            new byte[] { 9, 9, 8, 8, 7, 7, 6, 6 },
            new byte[] { 9, 9, 8, 8, 7, 7, 7, 6 },
            new byte[] { 9, 9, 8, 8, 8, 7, 7, 6 },
            new byte[] { 9, 9, 8, 8, 8, 7, 7, 7 },
            new byte[] { 9, 9, 8, 8, 8, 8, 7, 7 },
            new byte[] { 9, 9, 9, 8, 8, 8, 7, 7 },
            new byte[] { 9, 9, 9, 8, 8, 8, 8, 7 },
            new byte[] { 9, 9, 9, 8, 8, 8, 8, 8 },
            new byte[] { 9, 9, 9, 9, 8, 8, 8, 8 },
            new byte[] { 9, 9, 9, 9, 9, 8, 8, 8 },
            new byte[] { 9, 9, 9, 9, 9, 9, 8, 8 },
            new byte[] { 9, 9, 9, 9, 9, 9, 9, 8 },
            new byte[] { 9, 9, 9, 9, 9, 9, 9, 9 },
        };

        static byte[][] RedMaxCharges = 
        {
            new byte[] { 2, 0, 0, 0, 0, 0, 0, 0 },
            new byte[] { 3, 1, 0, 0, 0, 0, 0, 0 },
            new byte[] { 3, 2, 0, 0, 0, 0, 0, 0 },
            new byte[] { 4, 2, 0, 0, 0, 0, 0, 0 },
            new byte[] { 4, 3, 0, 0, 0, 0, 0, 0 },
            new byte[] { 4, 3, 1, 0, 0, 0, 0, 0 },
            new byte[] { 5, 3, 2, 0, 0, 0, 0, 0 },
            new byte[] { 5, 4, 2, 0, 0, 0, 0, 0 },
            new byte[] { 5, 4, 3, 0, 0, 0, 0, 0 },
            new byte[] { 6, 4, 3, 1, 0, 0, 0, 0 },
            new byte[] { 6, 4, 3, 2, 0, 0, 0, 0 },
            new byte[] { 6, 4, 4, 2, 0, 0, 0, 0 },
            new byte[] { 6, 5, 4, 3, 0, 0, 0, 0 },
            new byte[] { 7, 5, 4, 3, 0, 0, 0, 0 },
            new byte[] { 7, 5, 4, 3, 1, 0, 0, 0 },
            new byte[] { 7, 5, 5, 4, 2, 0, 0, 0 },
            new byte[] { 7, 6, 5, 4, 2, 0, 0, 0 },
            new byte[] { 7, 6, 5, 4, 3, 0, 0, 0 },
            new byte[] { 7, 6, 5, 5, 3, 0, 0, 0 },
            new byte[] { 7, 6, 6, 5, 3, 1, 0, 0 },
            new byte[] { 8, 6, 6, 5, 4, 2, 0, 0 },
            new byte[] { 9, 6, 6, 5, 4, 2, 0, 0 },
            new byte[] { 9, 6, 6, 5, 4, 3, 0, 0 },
            new byte[] { 9, 7, 6, 6, 4, 3, 0, 0 },
            new byte[] { 9, 7, 6, 6, 5, 3, 0, 0 },
            new byte[] { 9, 7, 6, 6, 5, 4, 1, 0 },
            new byte[] { 9, 7, 7, 6, 5, 4, 2, 0 },
            new byte[] { 9, 8, 7, 6, 5, 4, 2, 0 },
            new byte[] { 9, 8, 7, 6, 5, 5, 3, 0 },
            new byte[] { 9, 8, 7, 7, 6, 5, 3, 0 },
            new byte[] { 9, 8, 7, 7, 6, 5, 3, 1 },
            new byte[] { 9, 8, 8, 7, 6, 5, 4, 2 },
            new byte[] { 9, 8, 8, 7, 6, 6, 4, 2 },
            new byte[] { 9, 8, 8, 7, 6, 6, 4, 3 },
            new byte[] { 9, 8, 8, 7, 7, 6, 5, 3 },
            new byte[] { 9, 8, 8, 7, 7, 6, 5, 4 },
            new byte[] { 9, 8, 8, 8, 7, 6, 5, 4 },
            new byte[] { 9, 8, 8, 8, 7, 6, 6, 4 },
            new byte[] { 9, 8, 8, 8, 7, 6, 6, 5 },
            new byte[] { 9, 8, 8, 8, 7, 7, 6, 5 },
            new byte[] { 9, 9, 8, 8, 7, 7, 6, 5 },
            new byte[] { 9, 9, 8, 8, 8, 7, 6, 5 },
            new byte[] { 9, 9, 8, 8, 8, 7, 6, 6 },
            new byte[] { 9, 9, 8, 8, 8, 7, 7, 6 },
            new byte[] { 9, 9, 9, 8, 8, 7, 7, 6 },
            new byte[] { 9, 9, 9, 8, 8, 8, 7, 6 },
            new byte[] { 9, 9, 9, 8, 8, 8, 7, 7 },
            new byte[] { 9, 9, 9, 9, 8, 8, 7, 7 },
            new byte[] { 9, 9, 9, 9, 8, 8, 8, 7 },
            new byte[] { 9, 9, 9, 9, 9, 8, 8, 7 },
        };
    }
}
