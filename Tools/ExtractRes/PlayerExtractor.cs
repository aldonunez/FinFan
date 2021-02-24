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
        const int LevelUpData = 0x2d0a4;

        static readonly byte[] InitCharges =
        {
            0, 0, 0, 2, 2, 2,
        };

        internal static void Extract( Options options )
        {
            ExtractLevelUpCharges( options );
            ExtractLevelUpAttrs( options );
        }

        internal static void ExtractLevelUpCharges( Options options )
        {
            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                string chargeFile = options.MakeOutPath( @"chargeBoost.dat" );
                reader.BaseStream.Position = LevelUpData;

                // Reading charges, which are in the second byte of each record.

                byte[][] chargesByClass = new byte[6][];

                for ( int i = 0; i < 6; i++ )
                {
                    byte[] classCharges = new byte[49];

                    for ( int j = 0; j < 49; j++ )
                    {
                        // Skip the first byte of a record.
                        reader.ReadByte();
                        classCharges[j] = reader.ReadByte();
                    }

                    chargesByClass[i] = classCharges;
                }

                using ( FileStream stream = File.OpenWrite( chargeFile ) )
                {
                    stream.SetLength( 0 );

                    for ( int i = 0; i < 2; i++ )
                    {
                        for ( int j = 0; j < 6; j++ )
                        {
                            var classCharges = chargesByClass[j];
                            stream.WriteByte( InitCharges[j] );
                            stream.Write( classCharges, 0, classCharges.Length );
                        }
                    }
                }
            }
        }

        internal static void ExtractLevelUpAttrs( Options options )
        {
            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            {
                string attrFile = options.MakeOutPath( @"levelUpAttrs.dat" );
                reader.BaseStream.Position = LevelUpData;

                // Reading attrs, which are in the first byte of each record.

                byte[][] attrsByClass = new byte[6][];

                for ( int i = 0; i < 6; i++ )
                {
                    byte[] classAttrs = new byte[49];

                    for ( int j = 0; j < 49; j++ )
                    {
                        classAttrs[j] = reader.ReadByte();
                        // Skip the second byte of a record, which specifies level-up charges.
                        reader.ReadByte();
                    }

                    attrsByClass[i] = classAttrs;
                }

                using ( FileStream stream = File.OpenWrite( attrFile ) )
                {
                    stream.SetLength( 0 );

                    for ( int i = 0; i < 2; i++ )
                    {
                        for ( int j = 0; j < 6; j++ )
                        {
                            var classAttrs = attrsByClass[j];
                            stream.WriteByte( 0 );
                            stream.Write( classAttrs, 0, classAttrs.Length );
                        }
                    }
                }
            }
        }
    }
}
