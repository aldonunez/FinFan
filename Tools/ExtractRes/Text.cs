/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.IO;

namespace ExtractRes
{
    class Text
    {
        static char[] charBuf = new char[1024];

        public static string[] ReadStringTable( 
            BinaryReader reader, 
            int pointer, 
            int baseRef, 
            int count )
        {
            reader.BaseStream.Position = pointer;

            ushort[] relRefs = new ushort[count];
            string[] table = new string[count];
            int minRef = int.MaxValue, maxRef = 0;
            int strLen = 0;
            int totalStrLen = 0;

            for ( int i = 0; i < count; i++ )
            {
                relRefs[i] = reader.ReadUInt16();
            }

            for ( int i = 0; i < count; i++ )
            {
                int @ref = baseRef + relRefs[i];
                string str = DecodeString( reader, @ref );
                totalStrLen += strLen;

                table[i] = str;

                if ( @ref < minRef )
                    minRef = @ref;
                if ( @ref > maxRef )
                    maxRef = @ref;
            }

            int tableSize = (count * 2) + ((maxRef + strLen + 1) - minRef);

            return table;
        }

        public static void WriteStringTable( BinaryWriter writer, string[] names )
        {
            int heapStart = names.Length * 2;
            ushort[] ptrs = new ushort[names.Length];
            ushort ptr = 0;

            writer.BaseStream.Position = heapStart;

            for ( int i = 0; i < names.Length; i++ )
            {
                ptrs[i] = ptr;
                writer.Write( names[i].ToCharArray() );
                writer.Write( (byte) 0 );
                ptr = (ushort) (writer.BaseStream.Position - heapStart);
            }

            writer.BaseStream.Position = 0;

            for ( int i = 0; i < names.Length; i++ )
            {
                writer.Write( ptrs[i] );
            }
        }

        public static string DecodeString( BinaryReader reader, int @ref )
        {
            int len = 0;
            byte b = 0;

            reader.BaseStream.Position = @ref;

            for ( b = reader.ReadByte(); b != 0; b = reader.ReadByte() )
            {
                if ( b >= 0xA4 && b <= 0xA4 + 25 )
                    charBuf[len] = (char) (b - 0xA4 + 'a');
                else if ( b >= 0x8A && b <= 0x8A + 25 )
                    charBuf[len] = (char) (b - 0x8A + 'A');
                else if ( b >= 0x80 && b <= 0x80 + 9 )
                    charBuf[len] = (char) (b - 0x80 + '0');
                else
                {
                    if ( b >= 0x1A && b <= 0x69 )
                    {
                        int index = b - 0x1A;
                        string str = dteTable[index];

                        charBuf[len++] = str[0];
                        charBuf[len] = str[1];
                    }
                    else
                    {
                        switch ( b )
                        {
                        case 0xBE: charBuf[len] = '\''; break;
                        case 0xBF: charBuf[len] = ','; break;
                        case 0xC0: charBuf[len] = '.'; break;
                        case 0xC2: charBuf[len] = '-'; break;
                        case 0xC3: charBuf[len] = '_'; break;   // really ellipsis
                        case 0xC4: charBuf[len] = '!'; break;
                        case 0xC5: charBuf[len] = '?'; break;
                        case 0x01: charBuf[len] = '\n'; break;
                        case 0x05: charBuf[len] = '\n'; break;
                        case 0xFF: charBuf[len] = ' '; break;
                        case 0xE0: charBuf[len] = '%'; break;
                        case 0xD4: charBuf[len] = '\u0010'; break;   // sword
                        case 0xD5: charBuf[len] = '\u0011'; break;   // hammer
                        case 0xD6: charBuf[len] = '\u0012'; break;   // knife
                        case 0xD7: charBuf[len] = '\u0013'; break;   // axe
                        case 0xD8: charBuf[len] = '\u0014'; break;   // staff
                        case 0xD9: charBuf[len] = '\u0015'; break;   // nunchuk
                        case 0xDA: charBuf[len] = '\u0016'; break;   // armor
                        case 0xDB: charBuf[len] = '\u0017'; break;   // shield
                        case 0xDC: charBuf[len] = '\u0018'; break;   // helmet
                        case 0xDD: charBuf[len] = '\u0019'; break;   // gloves
                        case 0xDE: charBuf[len] = '\u001A'; break;   // bracelet
                        case 0xDF: charBuf[len] = '\u001B'; break;   // shirt
                        case 0xE1: charBuf[len] = '\u001C'; break;   // potion?
                        case 0x02: charBuf[len] = '\u0002'; break;   // name of item
                        case 0: break;
                        default: charBuf[len] = '*'; break;
                        }
                    }
                }

                len++;

                if ( len > charBuf.Length )
                    break;
            }

            return new string( charBuf, 0, len );
        }

        static string[] dteTable = new string[]
            {
            "e ",
            " t",
            "th",
            "he",
            "s ",
            "in",
            " a",
            "t ",
            "an",
            "re",
            " s",
            "er",
            "ou",
            "d ",
            "to",
            "n ",
            "ng",
            "ea",
            "es",
            " i",
            "o ",
            "ar",
            "is",
            " b",
            "ve",
            " w",
            "me",
            "or",
            " o",
            "st",
            " c",
            "at",
            "en",
            "nd",
            "on",
            "hi",
            "se",
            "as",
            "ed",
            "ha",
            " m",
            " f",
            "r ",
            "le",
            "ow",
            "g ",
            "ce",
            "om",
            "GI",
            "y ",
            "of",
            "ro",
            "ll",
            " p",
            " y",
            "ca",
            "MA",
            "te",
            "f ",
            "ur",
            "at",
            "ti",
            "l ",
            " h",
            "ne",
            "it",
            "ri",
            "wa",
            "ac",
            "al",
            "we",
            "il",
            "be",
            "rs",
            "u ",
            " l",
            "ge",
            " d",
            "li",
            "__"
            };
    }
}
