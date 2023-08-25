
;            .ORG $FB ; zero page
            ;
            ; data segment
            ;
;vx          .BYTE $20
;vy          .BYTE $21
;result      .WORD $0000

            .ORG $1300

            .ORG $2B6
            ; first usr routine, started from BASIC via USR(x)
            ; copies first screen line + color info to area below ROM
            LDX #$27
loop        LDA $0400,x
            LDY $D800,x
            STA $E000,x
            TYA
            STA $E028,x
            DEX
            BPL loop
            ; redirect LSB of USR vector to second function $2CE
            LDA #$CE
            STA $0311
            RTS

            .ORG $2CE
            ; second USR function, copies from area below ROM back to screen

            ; activate RAM in $E000 before reading
            LDA $01
            PHA
            LDA #$35
            STA $01

            LDX #$27
loop2       LDA $E000,x
            LDY $E028,x
            STA $0400,x
            TYA
            STA $D800,x
            DEX
            BPL loop2
            PLA
            STA $01
            ; redirect LSB of USR vector to first function $2B6
            LDA #$B6
            STA $0311
            RTS
.END