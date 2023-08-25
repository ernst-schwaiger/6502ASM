
;            .ORG $FB ; zero page
            ;
            ; defines
            ;
            ; logic file number
LOGFNR       = #$04
            ; device number disk
DEVNR        = #$08
            .ORG $6000
myString    .BYTE "THIS IS A TEST ", 0
filename    .BYTE "myfile"
fileend     .BYTE 0


            .ORG $2000

            ; call SETNAM
            LDA #$00 ; no filename needed
            JSR $FFBD
            BCS error
            ; call SETLFS
            LDA LOGFNR ; logic file number
            LDX DEVNR ; device number
            LDY #$FF ; secondary device number -> off
            JSR $FFBA
            BCS error
            ; call OPEN
            JSR $FFC0
            BCS error
            ; call CHKOUT
            LDX LOGFNR
            JSR $FFC9
            BCS error

            LDY #$00
next_out    LDA myString,Y
            BEQ end_out
            ; CHROUT
            JSR $FFD2
            BCS error
            INY
            JMP next_out

            ; call CLRCHN
end_out     JSR $FFCC
            ; call CLOSE
            LDA LOGFNR ; logical file number
            JSR $FFC3
            RTS
error       BRK


.END