/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace ExtractRes
{
    class SoundExtractor
    {
        const ushort ImageBase = 0x8000;
        const uint ImageSize = 0x8000;

        struct NsfTrack
        {
            public ushort Init;
            public ushort Play;
        }

        public static void MakeSfxNsf( Options options )
        {
            ushort initAddr = 0;
            ushort playAddr = 0;
            ushort customPtr = 0x0200;
            string nsfPath = options.NsfPath;
            List<NsfTrack> tracks = new List<NsfTrack>();

            using ( BinaryReader reader = new BinaryReader( File.OpenRead( options.RomPath ) ) )
            using ( BinaryWriter writer = new BinaryWriter( File.OpenWrite( nsfPath ), Encoding.ASCII ) )
            {
                const ushort InitTableAddr = 0x8100;
                const ushort PlayTableAddr = 0x8130;

                byte[] image = new byte[ImageSize];

                // When a track doesn't have an address, it goes here.
                image[0] = 0x60;    // RTS

                ushort ptr = 1;
                initAddr = (ushort) (ptr + ImageBase);
                // STA $E0
                image[ptr++] = 0x85;
                image[ptr++] = 0xE0;
                // ASL
                image[ptr++] = 0x0A;
                // TAY
                image[ptr++] = 0xA8;
                // LDA $8100, Y
                image[ptr++] = 0xB9;
                image[ptr++] = InitTableAddr & 0xFF;
                image[ptr++] = InitTableAddr >> 8;
                // STA $00
                image[ptr++] = 0x85;
                image[ptr++] = 0;
                // LDA $8101, Y
                image[ptr++] = 0xB9;
                image[ptr++] = (InitTableAddr + 1) & 0xFF;
                image[ptr++] = (InitTableAddr + 1) >> 8;
                // STA $01
                image[ptr++] = 0x85;
                image[ptr++] = 1;
                // JMP ($0000)
                image[ptr++] = 0x6C;
                image[ptr++] = 0;
                image[ptr++] = 0;

                playAddr = (ushort) (ptr + ImageBase);
                // LDA $E0
                image[ptr++] = 0xA5;
                image[ptr++] = 0xE0;
                // ASL
                image[ptr++] = 0x0A;
                // TAY
                image[ptr++] = 0xA8;
                // LDA $8130, Y
                image[ptr++] = 0xB9;
                image[ptr++] = PlayTableAddr & 0xFF;
                image[ptr++] = PlayTableAddr >> 8;
                // STA $00
                image[ptr++] = 0x85;
                image[ptr++] = 0;
                // LDA $8131, Y
                image[ptr++] = 0xB9;
                image[ptr++] = (PlayTableAddr + 1) & 0xFF;
                image[ptr++] = (PlayTableAddr + 1) >> 8;
                // STA $01
                image[ptr++] = 0x85;
                image[ptr++] = 1;
                // JMP ($0000)
                image[ptr++] = 0x6C;
                image[ptr++] = 0;
                image[ptr++] = 0;

                tracks.Add( ExtractMenuConfirmSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractMenuCursorSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractMinigameErrorSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractAirshipSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractShipSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractLavaDamageSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractPoisonSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractDoorSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractDialogOpenSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractDialogCloseSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractWipeOpenSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractWipeCloseSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractFightSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractAltarSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractErrorSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractAirshipLiftSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractAirshipLandSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractMagicSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractHurtSfx( image, reader, ref customPtr ) );
                tracks.Add( ExtractStrikeSfx( image, reader, ref customPtr ) );

                ushort initTablePtr = InitTableAddr - ImageBase;
                ushort playTablePtr = PlayTableAddr - ImageBase;

                foreach ( NsfTrack track in tracks )
                {
                    ptr = track.Init;
                    if ( ptr == 0 )
                        ptr = 0x8000;
                    image[initTablePtr] = (byte) (ptr & 0xFF);
                    image[initTablePtr + 1] = (byte) (ptr >> 8);
                    initTablePtr += 2;

                    ptr = track.Play;
                    if ( ptr == 0 )
                        ptr = 0x8000;
                    image[playTablePtr] = (byte) (ptr & 0xFF);
                    image[playTablePtr + 1] = (byte) (ptr >> 8);
                    playTablePtr += 2;
                }

                writer.Write( "NESM".ToCharArray() );
                writer.Write( (byte) 0x1A );
                writer.Write( (byte) 1 );           // NSF version
                writer.Write( (byte) tracks.Count );
                writer.Write( (byte) 1 );           // first song
                writer.Write( (ushort) 0 );         // load
                writer.Write( (ushort) initAddr );  // init
                writer.Write( (ushort) playAddr );  // play
                // name
                writer.Write( "Final Fantasy".ToCharArray() );
                for ( int i = 0; i < 19; i++ )
                    writer.Write( (byte) 0 );
                // artist
                for ( int i = 0; i < 32; i++ )
                    writer.Write( (byte) 0 );
                // copyright
                writer.Write( "1987 Square".ToCharArray() );
                for ( int i = 0; i < 21; i++ )
                    writer.Write( (byte) 0 );
                // NTSC play speed (default)
                writer.Write( (ushort) 0 );
                // bankswitch values
                for ( int i = 0; i < 8; i++ )
                    writer.Write( (byte) 0 );
                // PAL play speed (default)
                writer.Write( (ushort) 0 );
                // PAL/NTSC, sound chips, reserved
                for ( int i = 0; i < 6; i++ )
                    writer.Write( (byte) 0 );
                writer.Write( image );
            }
        }

        private static NsfTrack ExtractMenuConfirmSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3AD94;       // E
            reader.Read( image, 0xAD84 - ImageBase, 0x19 );
            return new NsfTrack { Init = 0xAD84, Play = 0 };
        }

        private static NsfTrack ExtractMenuCursorSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3ADAD;       // E
            reader.Read( image, 0xAD9D - ImageBase, 0x16 );
            return new NsfTrack { Init = 0xAD9D, Play = 0 };
        }

        private static NsfTrack ExtractMinigameErrorSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x36F3C;       // D
            reader.Read( image, 0xAF2C - ImageBase, 0x18 );
            image[0xAF2C - ImageBase + 0x18] = 0x60;    // Replace JMP with RTS
            return new NsfTrack { Init = 0xAF2C, Play = 0 };
        }

        private static NsfTrack ExtractAirshipSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3C129;       // F
            reader.Read( image, 0xC119 - ImageBase, 0x24 );
            image[0xC119 - ImageBase + 0x24] = 0x60;    // Replace JMP with RTS
            ushort playAddr = (ushort) (customPtr + ImageBase);
            // INC $F0
            image[customPtr++] = 0xE6;
            image[customPtr++] = 0xF0;
            // JMP $C119
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0x19;
            image[customPtr++] = 0xC1;
            return new NsfTrack { Init = 0, Play = playAddr };
        }

        private static NsfTrack ExtractShipSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3C129;       // F
            reader.Read( image, 0xC119 - ImageBase, 0x24 );
            image[0xC119 - ImageBase + 0x24] = 0x60;    // Replace JMP with RTS
            ushort playAddr = (ushort) (customPtr + ImageBase);
            // INC $F0
            image[customPtr++] = 0xE6;
            image[customPtr++] = 0xF0;
            // JMP $C124
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0x24;
            image[customPtr++] = 0xC1;
            return new NsfTrack { Init = 0, Play = playAddr };
        }

        private static NsfTrack ExtractLavaDamageSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3C7F7;       // F
            reader.Read( image, 0xC7E7 - ImageBase, 0xF );
            image[0xC7E7 - ImageBase + 0xF] = 0x60;     // Replace { LDA; BEQ; RTS } with RTS
            return new NsfTrack { Init = 0xC7E7, Play = 0 };
        }

        private static NsfTrack ExtractPoisonSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3C828;       // F
            reader.Read( image, 0xC818 - ImageBase, 0x16 );
            image[0xC818 - ImageBase + 0x16] = 0x60;    // Replace { LDA; BEQ; RTS } with RTS
            return new NsfTrack { Init = 0xC818, Play = 0 };
        }

        private static NsfTrack ExtractDoorSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3CF2E;       // F
            reader.Read( image, 0xCF1E - ImageBase, 0x10 );
            return new NsfTrack { Init = 0xCF1E, Play = 0 };
        }

        private static NsfTrack ExtractDialogOpenSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3D6D7;       // F
            reader.Read( image, 0xD6C7 - ImageBase, 0x15 );
            ushort initAddr = (ushort) (customPtr + ImageBase);
            // LDA #$8E
            image[customPtr++] = 0xA9;
            image[customPtr++] = 0x8E;
            // JMP $D6C7
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0xC7;
            image[customPtr++] = 0xD6;
            return new NsfTrack { Init = initAddr, Play = 0 };
        }

        private static NsfTrack ExtractDialogCloseSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3D6D7;       // F
            reader.Read( image, 0xD6C7 - ImageBase, 0x15 );
            ushort initAddr = (ushort) (customPtr + ImageBase);
            // LDA #$95
            image[customPtr++] = 0xA9;
            image[customPtr++] = 0x95;
            // JMP $D6C7
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0xC7;
            image[customPtr++] = 0xD6;
            return new NsfTrack { Init = initAddr, Play = 0 };
        }

        private static NsfTrack ExtractWipeOpenSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            // ScreenWipe_Open
            reader.BaseStream.Position = 0x3D6EC;       // F
            reader.Read( image, customPtr, 11 );
            image[customPtr + 11] = 0x60;       // RTS
            image[customPtr + 1] = 0xAF;        // Jump into the middle of StartScreenWipe
            image[customPtr + 2] = 0xD7;
            customPtr += 12;

            // Loop
            reader.BaseStream.Position = 0x3D6F7;       // F
            reader.Read( image, 0xD6E7 - ImageBase, 72 );
            image[0xD6E7 - ImageBase + 20] = (byte) (image[0xD6E7 - ImageBase + 20] - 1);
            image[0xD6E8 - ImageBase] = 0x6C;   // Jump into the middle of ScreenWipeFrame
            image[0xD6E9 - ImageBase] = 0xD7;

            image[0xD6E0 - ImageBase] = 0xA5;   // LDA tmp+5
            image[0xD6E1 - ImageBase] = 0x15;
            image[0xD6E2 - ImageBase] = 0xC9;   // CMP #224
            image[0xD6E3 - ImageBase] = 0xE0;
            image[0xD6E4 - ImageBase] = 0x90;   // BCC :+1
            image[0xD6E5 - ImageBase] = 0x01;
            image[0xD6E6 - ImageBase] = 0x60;   // RTS

            // ScreenWipeFrame
            reader.BaseStream.Position = 0x3D77C;       // F
            reader.Read( image, 0xD76C - ImageBase, 28 );

            // StartScreenWipe
            reader.BaseStream.Position = 0x3D7BF;       // F
            reader.Read( image, 0xD7AF - ImageBase, 16 );

            return new NsfTrack { Init = initAddr, Play = 0xD6E0 };
        }

        private static NsfTrack ExtractWipeCloseSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            // ScreenWipe_Close
            reader.BaseStream.Position = 0x3D711;       // F
            reader.Read( image, customPtr, 11 );
            image[customPtr + 11] = 0x60;       // RTS
            image[customPtr + 1] = 0xAF;        // Jump into the middle of StartScreenWipe
            image[customPtr + 2] = 0xD7;
            customPtr += 12;

            // Loop
            reader.BaseStream.Position = 0x3D71C;       // F
            reader.Read( image, 0xD70C - ImageBase, 35 );
            image[0xD70C - ImageBase + 20] = (byte) (image[0xD70C - ImageBase + 20] - 1);
            image[0xD70D - ImageBase] = 0x6C;   // Jump into the middle of ScreenWipeFrame
            image[0xD70E - ImageBase] = 0xD7;

            image[0xD705 - ImageBase] = 0xA5;  // LDA tmp+5
            image[0xD706 - ImageBase] = 0x15;
            image[0xD707 - ImageBase] = 0xC9;  // CMP #1
            image[0xD708 - ImageBase] = 0x01;
            image[0xD709 - ImageBase] = 0xD0;  // BNE :+1
            image[0xD70A - ImageBase] = 0x01;
            image[0xD70B - ImageBase] = 0x60;  // RTS

            // ScreenWipeFrame
            reader.BaseStream.Position = 0x3D77C;       // F
            reader.Read( image, 0xD76C - ImageBase, 28 );

            // StartScreenWipe
            reader.BaseStream.Position = 0x3D7BF;       // F
            reader.Read( image, 0xD7AF - ImageBase, 16 );

            return new NsfTrack { Init = initAddr, Play = 0xD705 };
        }

        private static NsfTrack ExtractFightSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3D8DD;       // F
            reader.Read( image, 0xD8CD - ImageBase, 0x48 );
            image[0xD8CD - ImageBase + 9] = 0x60;
            image[0xD8CD - ImageBase + 0x48] = 0x60;

            image[0xD8E0 - ImageBase] = 0xA5;   // LDA tmp+12
            image[0xD8E1 - ImageBase] = 0x1C;
            image[0xD8E2 - ImageBase] = 0xC9;   // CMP #$41
            image[0xD8E3 - ImageBase] = 0x41;
            image[0xD8E4 - ImageBase] = 0x90;   // BCC :+1
            image[0xD8E5 - ImageBase] = 0x01;
            image[0xD8E6 - ImageBase] = 0x60;   // RTS
            return new NsfTrack { Init = 0xD8CD, Play = 0xD8E7 };
        }

        private static NsfTrack ExtractAltarSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3DA5E;       // F
            reader.Read( image, 0xDA4E - ImageBase, 0x9D );
            image[0xDA4E - ImageBase + 0x3A] = 0x60;
            image[0xDA4E - ImageBase + 0x9D] = 0x60;
            image[0xDA90 - ImageBase] = 0xEA;   // Replace call with NOP
            image[0xDA91 - ImageBase] = 0xEA;
            image[0xDA92 - ImageBase] = 0xEA;
            image[0xDA98 - ImageBase] = 0xEA;   // Replace call with NOP
            image[0xDA99 - ImageBase] = 0xEA;
            image[0xDA9A - ImageBase] = 0xEA;
            image[0xDAB3 - ImageBase] = (byte) (image[0xDAB3 - ImageBase] - 3);
            image[0xDAB9 - ImageBase] = (byte) (image[0xDAB9 - ImageBase] - 3);
            image[0xDAC5 - ImageBase] = (byte) (image[0xDAC5 - ImageBase] - 3);

            // if [tmp+2] = 1 and [tmp+1] < 8, stop effect
            image[0xDA4E - ImageBase + 0x9E] = 0xA5;    // LDA tmp+2
            image[0xDA4E - ImageBase + 0x9F] = 0x12;
            image[0xDA4E - ImageBase + 0xA0] = 0xC9;    // CMP #1
            image[0xDA4E - ImageBase + 0xA1] = 0x01;
            image[0xDA4E - ImageBase + 0xA2] = 0xD0;    // BNE :+7
            image[0xDA4E - ImageBase + 0xA3] = 0x07;
            image[0xDA4E - ImageBase + 0xA4] = 0xA5;    // LDA tmp+1
            image[0xDA4E - ImageBase + 0xA5] = 0x11;
            image[0xDA4E - ImageBase + 0xA6] = 0xC9;    // CMP #8
            image[0xDA4E - ImageBase + 0xA7] = 0x08;
            image[0xDA4E - ImageBase + 0xA8] = 0xB0;    // BCS :+1
            image[0xDA4E - ImageBase + 0xA9] = 0x01;
            image[0xDA4E - ImageBase + 0xAA] = 0x60;    // RTS
            image[0xDA4E - ImageBase + 0xAB] = 0x4C;    // JMP $DA8B
            image[0xDA4E - ImageBase + 0xAC] = 0x8B;
            image[0xDA4E - ImageBase + 0xAD] = 0xDA;

            return new NsfTrack { Init = 0xDA4E, Play = 0xDAEC };
        }

        private static NsfTrack ExtractErrorSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            reader.BaseStream.Position = 0x3DB36;       // F
            reader.Read( image, 0xDB26 - ImageBase, 17 );
            // STY $E1
            // RTS
            image[0xDB26 - ImageBase + 17] = 0x84;
            image[0xDB26 - ImageBase + 18] = 0xE1;
            image[0xDB26 - ImageBase + 19] = 0x60;

            reader.BaseStream.Position = 0x3DB4D;       // F
            reader.Read( image, 0xDB3D - ImageBase, 11 );

            reader.BaseStream.Position = 0x3DB5B;       // F
            reader.Read( image, 0xDB4B - ImageBase, 20 );

            // Replace RTS with:
            image[0xDB4B - ImageBase + 20] = 0xC6;  // DEC $E1
            image[0xDB4B - ImageBase + 21] = 0xE1;
            image[0xDB4B - ImageBase + 22] = 0x30;  // BMI :+1
            image[0xDB4B - ImageBase + 23] = 0x01;
            image[0xDB4B - ImageBase + 24] = 0x60;  // RTS
            image[0xDB4B - ImageBase + 25] = 0x4C;  // JMP $DB3D
            image[0xDB4B - ImageBase + 26] = 0x3D;
            image[0xDB4B - ImageBase + 27] = 0xDB;

            // DB67:
            image[0xDB4B - ImageBase + 28] = 0xA5;  // LDA $E1
            image[0xDB4B - ImageBase + 29] = 0xE1;
            image[0xDB4B - ImageBase + 30] = 0x10;  // BPL :+1
            image[0xDB4B - ImageBase + 31] = 0x01;
            image[0xDB4B - ImageBase + 32] = 0x60;  // RTS
            image[0xDB4B - ImageBase + 33] = 0x4C;  // JMP $DB4B
            image[0xDB4B - ImageBase + 34] = 0x4B;
            image[0xDB4B - ImageBase + 35] = 0xDB;

            return new NsfTrack { Init = 0xDB26, Play = 0xDB67 };
        }

        private static NsfTrack ExtractAirshipLiftSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            reader.BaseStream.Position = 0x3E1B8;
            reader.Read( image, customPtr, 4 );
            image[customPtr + 4] = 0x60;
            customPtr += 5;

            reader.BaseStream.Position = 0x3E1BC;
            reader.Read( image, 0xE1AC - ImageBase, 22 );
            image[0xE1AC - ImageBase + 8] = 12;     // Change relative targets
            image[0xE1AC - ImageBase + 16] = 4;
            image[0xE1AC - ImageBase + 1] = 0xE9;   // Change absolute target
            image[0xE1AC - ImageBase + 2] = 0xE1;

            reader.BaseStream.Position = 0x3E1F9;       // F
            reader.Read( image, 0xE1E9 - ImageBase, 7 );
            reader.BaseStream.Position = 0x3E225;       // F
            reader.Read( image, 0xE1F0 - ImageBase, 16 );

            ushort playAddr = (ushort) (customPtr + ImageBase);
            image[customPtr++] = 0xA5;  // LDA tmp+10
            image[customPtr++] = 0x1A;
            image[customPtr++] = 0xC9;  // CMP #$4F
            image[customPtr++] = 0x4F;
            image[customPtr++] = 0xB0;  // BCS :+1
            image[customPtr++] = 0x01;
            image[customPtr++] = 0x60;  // RTS
            image[customPtr++] = 0x4C;  // JMP $E1AC
            image[customPtr++] = 0xAC;
            image[customPtr++] = 0xE1;

            return new NsfTrack { Init = initAddr, Play = playAddr };
        }

        private static NsfTrack ExtractAirshipLandSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            reader.BaseStream.Position = 0x3E1D2;
            reader.Read( image, customPtr, 4 );
            image[customPtr + 4] = 0x60;
            customPtr += 5;

            reader.BaseStream.Position = 0x3E1D6;
            reader.Read( image, 0xE1C6 - ImageBase, 27 );
            image[0xE1C6 - ImageBase + 8] = 17;
            image[0xE1C6 - ImageBase + 16] = 9;
            image[0xE1C6 - ImageBase + 1] = 0xE9;
            image[0xE1C6 - ImageBase + 2] = 0xE1;

            reader.BaseStream.Position = 0x3E1F9;       // F
            reader.Read( image, 0xE1E9 - ImageBase, 7 );
            reader.BaseStream.Position = 0x3E225;       // F
            reader.Read( image, 0xE1F0 - ImageBase, 16 );

            ushort playAddr = (ushort) (customPtr + ImageBase);
            image[customPtr++] = 0xA5;  // LDA tmp+10
            image[customPtr++] = 0x1A;
            image[customPtr++] = 0xC9;  // CMP #$70
            image[customPtr++] = 0x70;
            image[customPtr++] = 0x90;  // BCC :+1
            image[customPtr++] = 0x01;
            image[customPtr++] = 0x60;  // RTS
            image[customPtr++] = 0x4C;  // JMP $E1C6
            image[customPtr++] = 0xC6;
            image[customPtr++] = 0xE1;

            return new NsfTrack { Init = initAddr, Play = playAddr };
        }

        private static NsfTrack ExtractMagicSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            reader.BaseStream.Position = 0x33EC8;       // C
            reader.Read( image, 0xBEB8 - ImageBase, 0x143 + 5 );
            // LDA #0
            image[customPtr++] = 0xA9;
            image[customPtr++] = 0;
            // JMP $BEB8
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0xB8;
            image[customPtr++] = 0xBE;
            return new NsfTrack { Init = initAddr, Play = 0xBF13 };
        }

        private static NsfTrack ExtractHurtSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            reader.BaseStream.Position = 0x33EC8;       // C
            reader.Read( image, 0xBEB8 - ImageBase, 0x143 + 5 );
            // LDA #1
            image[customPtr++] = 0xA9;
            image[customPtr++] = 1;
            // JMP $BEB8
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0xB8;
            image[customPtr++] = 0xBE;
            return new NsfTrack { Init = initAddr, Play = 0xBF13 };
        }

        private static NsfTrack ExtractStrikeSfx( byte[] image, BinaryReader reader, ref ushort customPtr )
        {
            ushort initAddr = (ushort) (customPtr + ImageBase);
            reader.BaseStream.Position = 0x33EC8;       // C
            reader.Read( image, 0xBEB8 - ImageBase, 0x143 + 5 );
            // LDA #2
            image[customPtr++] = 0xA9;
            image[customPtr++] = 2;
            // JMP $BEB8
            image[customPtr++] = 0x4C;
            image[customPtr++] = 0xB8;
            image[customPtr++] = 0xBE;
            return new NsfTrack { Init = initAddr, Play = 0xBF13 };
        }
    }
}
