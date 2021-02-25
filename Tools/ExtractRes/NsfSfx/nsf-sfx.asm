.SEGMENT "HEADER"

.BYTE "NESM", $1A                               ; 00 Format
.BYTE $01                                       ; 05 Version
.BYTE $15                                       ; 06 Total songs
.BYTE $01                                       ; 07 Starting song

.ADDR Load
.ADDR Init
.ADDR Play

.BYTE "Final Fantasy"                           ; 0E Name
.BYTE $00, $00, $00
.BYTE $00, $00, $00, $00, $00, $00, $00, $00
.BYTE $00, $00, $00, $00, $00, $00, $00, $00

.BYTE "Nobuo Uematsu"                           ; 2E Artist
.BYTE $00, $00, $00
.BYTE $00, $00, $00, $00, $00, $00, $00, $00
.BYTE $00, $00, $00, $00, $00, $00, $00, $00

.BYTE "1987 Square"                             ; 4E Copyright
.BYTE $00, $00, $00, $00, $00
.BYTE $00, $00, $00, $00, $00, $00, $00, $00
.BYTE $00, $00, $00, $00, $00, $00, $00, $00

.BYTE $00, $00                                  ; 6E Play speed, NSTC
.BYTE $06, $06, $00, $01, $02, $03, $04, $05    ; 70 Bankswitch init
.BYTE $00, $00                                  ; 78 Play speed, PAL
.BYTE $00                                       ; 7A PAL/NSTC bits
.BYTE $00                                       ; 7B Extra sound chip support
.BYTE $00                                       ; 7C Reserved for NSF2
.BYTE $00, $00, $00                             ; 7D Program length


;=============================================================================

.SEGMENT "ENTRY"

Nsf_InitMenuConfirm     := $AD84    ; bank E
Nsf_InitMenuCursor      := $AD9D    ; bank E
Nsf_InitMinigameError   := $AF2C    ; bank D
Nsf_InitTile            := $C7E7
Nsf_InitPoison          := $C818
Nsf_InitDoor            := $CF1E
Nsf_InitFight           := $D8CD
Nsf_InitAltar           := $DA4E
Nsf_PlayFight           := $D8E7
UpdateBattleSFX         := $BF13    ; bank C
Nsf_PlayBattle          := UpdateBattleSFX
ChaosDeath_FadeNoise    := $A03D    ; bank B
PlaySFX_Error           := $DB26


Load:

InitAddrs:
    .ADDR Nsf_InitMenuConfirm
    .ADDR Nsf_InitMenuCursor
    .ADDR Nsf_InitMinigameError
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nsf_InitTile
    .ADDR Nsf_InitPoison
    .ADDR Nsf_InitDoor
    .ADDR Nsf_InitDialogOpen
    .ADDR Nsf_InitDialogClose
    .ADDR Nsf_InitWipeOpen
    .ADDR Nsf_InitWipeClose
    .ADDR Nsf_InitFight
    .ADDR Nsf_InitAltar
    .ADDR Nsf_InitError
    .ADDR Nsf_InitAirshipLift
    .ADDR Nsf_InitAirshipLand
    .ADDR Nsf_InitMagic
    .ADDR Nsf_InitHurt
    .ADDR Nsf_InitStrike
    .ADDR Nsf_InitChaos

PlayAddrs:
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nsf_PlayAirship
    .ADDR Nsf_PlayShip
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nothing
    .ADDR Nsf_PlayWipeOpen
    .ADDR Nsf_PlayWipeClose
    .ADDR Nsf_PlayFight
    .ADDR Nsf_PlayAltar
    .ADDR Nsf_PlayError
    .ADDR Nsf_PlayAirshipLift
    .ADDR Nsf_PlayAirshipLand
    .ADDR Nsf_PlayBattle
    .ADDR Nsf_PlayBattle
    .ADDR Nsf_PlayBattle
    .ADDR Nsf_PlayChaos

Nothing:
	RTS

Init:
	STA $E0
	ASL
	TAY
	LDA InitAddrs, Y
	STA $00
	LDA InitAddrs+1, Y
	STA $01
	JMP ($0000)

Play:
	LDA $E0
	ASL
	TAY
	LDA PlayAddrs, Y
	STA $00
	LDA PlayAddrs+1, Y
	STA $01
	JMP ($0000)


;=============================================================================

.SEGMENT "CUSTOM"

; There is no centralized sound effect driver, like the one in bank 0 of Zelda.
; Instead, sound effects are interspersed throughout the gameplay code in banks D-F.
; This means that each sound effect has to be tracked down and instrumented
; individually.

; Furthermore, because the game is not event driven, care must be taken to find
; the points where frames are crossed in order to jump to these points from the
; NSF's playback entrypoint.

; I've used four strategies:
; 1. Write new custom code that jumps to the original code.
; 2. Adapt code based on the original, that jumps to the original code.
; 3. Jump into the middle of an original procedure to avoid problematic parts.
; 4. Overwrite problematic original code. (1 instance, see WaitForVBlank_L)

; Certain labels of data and code are based on Disch's disassembly.

tmp := $10


Nsf_PlayAirship:
    INC $F0
    JMP $C119

Nsf_PlayShip:
    INC $F0
    JMP $C124


Nsf_InitDialogOpen:
    LDA #$8E
    JMP $D6C7

Nsf_InitDialogClose:
    LDA #$95
    JMP $D6C7


Nsf_InitWipeOpen:
    JSR $D7AF
    JMP $D6DF

Nsf_InitWipeClose:
    JSR $D7AF
    JMP $D704

Nsf_PlayWipeOpen:
    LDA tmp+5
    CMP #224
    BCC :+
    RTS
:
    JSR $D76C
    JMP $D6EA

Nsf_PlayWipeClose:
    LDA tmp+5
    CMP #1
    BNE :+
    RTS
:
    JSR $D76C
    JMP $D70F


Nsf_InitAirshipLift:
    LDA #$6F
    STA tmp+10
    RTS

Nsf_PlayAirshipLift:
    LDA tmp+10
    CMP #$4F
    BCS :+
    ; Silence noise here, because AnimateAirshipTakeoff doesn't even though AnimateAirshipLanding does.
    LDA #0
    STA $400C
    RTS
:
    INC $F0
    JSR $E215
    JMP $E1AF


Nsf_InitAirshipLand:
    LDA #$4F
    STA tmp+10
    RTS

Nsf_PlayAirshipLand:
    LDA tmp+10
    CMP #$70
    BCC :+
    RTS
:
    INC $F0
    JSR $E215
    JMP $E1C9


Nsf_InitMagic:
    LDA #0
    JMP $BEB8           ; bank C

Nsf_InitHurt:
    LDA #1
    JMP $BEB8           ; bank C

Nsf_InitStrike:
    LDA #2
    JMP $BEB8           ; bank C


Nsf_InitChaos:
        @outerctr       = $9A
    LDA #8
    STA @outerctr
    JMP ChaosDeath_FadeNoise

Nsf_PlayChaos:
        @outerctr       = $9A
        @innerctr       = $9B
    LDA @outerctr
    BNE @Fade
    LDY $9E
    BEQ @DoNothing
    DEY
    BNE @DecWait
    LDA #0
    STA $4015
@DecWait:
    STY $9E
@DoNothing:
    RTS

@Fade:
    LDA @innerctr
    CMP #$80
    BNE :+
    JSR ChaosDeath_FadeNoise
:
    DEC @innerctr
    BNE @Done
    JSR ChaosDeath_FadeNoise
    DEC @outerctr
    BNE @OuterLoop
    LDA #120
    STA $9E
    RTS
@OuterLoop:
    LDA #0
    STA @innerctr
@Done:
    RTS


Nsf_PlayAltar:
    LDA tmp+2
    CMP #1
    BNE :+
    LDA tmp+1
    CMP #8
    BCS :+
    RTS
:
    ; From beginning of AltarFrame
    LDA tmp+1
    LSR A
    LSR A
    LSR A
    ORA #$30
    STA $400C
    STA $4004
    ; Jump into the middle of the MainLoop of DoAltarEffect, after the call to AltarFrame.
    JMP $DA8E


Nsf_InitError:
    LDY #$0F
    STY $E1
    ; Won't return.
    JSR PlaySFX_Error
    RTS

Nsf_PlayError:
    LDA $E1
    BPL :+
    RTS
:
    JSR $DB4B
    DEC $E1
    BMI :+
    RTS
:
    JMP $DB3D


;=============================================================================

.SEGMENT "EXIT"
; $FEA8

; Instead of responding to an NMI by running the game for a frame; the game runs
; continuously, but calls the original procedure at this address whenever it needs
; to wait for the next frame.

; NSF uses the NMI trigger model, which calls the playback entrypoint once a frame.

; This procedure overwrites the original one so that the game breaks out and lets
; the NSF player wait for the next frame.

WaitForVBlank_L:
    LDX #$FD        ; Pop everything but the first address pushed
    TXS             ; by setting the stack pointer to $01FD.
    RTS             ; Now we can return from the game to the player.
