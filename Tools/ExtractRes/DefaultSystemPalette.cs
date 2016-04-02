/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System.Drawing;

namespace ExtractRes
{
    class DefaultSystemPalette
    {
        public static Color[] Colors = new Color[]
        {
            Color.FromArgb( 124, 124, 124 ),        // 00
            Color.FromArgb( 0, 0, 252 ),            // 01
            Color.FromArgb( 0, 0, 188 ),            // 02
            Color.FromArgb( 68, 40, 188 ),          // 03
            Color.FromArgb( 148, 0, 132 ),          // 04
            Color.FromArgb( 168, 0, 32 ),           // 05
            Color.FromArgb( 168, 16, 0 ),           // 06
            Color.FromArgb( 136, 20, 0 ),           // 07
            Color.FromArgb( 80, 48, 0 ),            // 08
            Color.FromArgb( 0, 120, 0 ),            // 09
            Color.FromArgb( 0, 104, 0 ),            // 0A
            Color.FromArgb( 0, 88, 0 ),             // 0B
            Color.FromArgb( 0, 64, 88 ),            // 0C
            Color.FromArgb( 0, 0, 0 ),              // 0D
            Color.FromArgb( 0, 0, 0 ),              // 0E (undefined)
            Color.FromArgb( 0, 0, 0 ),              // 0F (undefined)
            Color.FromArgb( 188, 188, 188 ),        // 10
            Color.FromArgb( 0, 120, 248 ),          // 11
            Color.FromArgb( 0, 88, 248 ),           // 12
            Color.FromArgb( 104, 68, 252 ),         // 13
            Color.FromArgb( 216, 0, 204 ),          // 14
            Color.FromArgb( 228, 0, 88 ),           // 15
            Color.FromArgb( 248, 56, 0 ),           // 16
            Color.FromArgb( 228, 92, 16 ),          // 17
            Color.FromArgb( 172, 124, 0 ),          // 18
            Color.FromArgb( 0, 184, 0 ),            // 19
            Color.FromArgb( 0, 168, 0 ),            // 1A
            Color.FromArgb( 0, 168, 68 ),           // 1B
            Color.FromArgb( 0, 136, 136 ),          // 1C
            Color.FromArgb( 0, 0, 0 ),              // 1D
            Color.FromArgb( 0, 0, 0 ),              // 1E (undefined)
            Color.FromArgb( 0, 0, 0 ),              // 1F (undefined)
            Color.FromArgb( 248, 248, 248 ),        // 20
            Color.FromArgb( 60, 188, 252 ),         // 21
            Color.FromArgb( 104, 136, 252 ),        // 22
            Color.FromArgb( 152, 120, 248 ),        // 23
            Color.FromArgb( 248, 120, 248 ),        // 24
            Color.FromArgb( 248, 88, 152 ),         // 25
            Color.FromArgb( 248, 120, 88 ),         // 26
            Color.FromArgb( 252, 160, 68 ),         // 27
            Color.FromArgb( 248, 184, 0 ),          // 28
            Color.FromArgb( 184, 248, 24 ),         // 29
            Color.FromArgb( 88, 216, 84 ),          // 2A
            Color.FromArgb( 88, 248, 152 ),         // 2B
            Color.FromArgb( 0, 232, 216 ),          // 2C
            Color.FromArgb( 120, 120, 120 ),        // 2D
            Color.FromArgb( 0, 0, 0 ),              // 2E (undefined)
            Color.FromArgb( 0, 0, 0 ),              // 2F (undefined)
            Color.FromArgb( 252, 252, 252 ),        // 30
            Color.FromArgb( 164, 228, 252 ),        // 31
            Color.FromArgb( 184, 184, 248 ),        // 32
            Color.FromArgb( 216, 184, 248 ),        // 33
            Color.FromArgb( 248, 184, 248 ),        // 34
            Color.FromArgb( 248, 164, 192 ),        // 35
            Color.FromArgb( 240, 208, 176 ),        // 36
            Color.FromArgb( 252, 224, 168 ),        // 37
            Color.FromArgb( 248, 216, 120 ),        // 38
            Color.FromArgb( 216, 248, 120 ),        // 39
            Color.FromArgb( 184, 248, 184 ),        // 3A
            Color.FromArgb( 184, 248, 216 ),        // 3B
            Color.FromArgb( 0, 252, 252 ),          // 3C
            Color.FromArgb( 216, 216, 216 ),        // 3D
            Color.FromArgb( 0, 0, 0 ),              // 3E (undefined)
            Color.FromArgb( 0, 0, 0 ),              // 3F (undefined)
        };
    }

    class DefaultTilePalette
    {
        public static Color[] Colors =
        {
            Color.DarkGreen,
            Color.LightGreen,
            Color.Magenta,
            Color.White
        };
    }
}
