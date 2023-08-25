            .ORG $FB ; zero page
            ;
            ; data segment for uint8 * uint8 -> uint16
            ;
x8:         .BYTE $20
y8:         .BYTE $21
result16:   .WORD $0000

            .ORG $4000
            ;
            ; data segment for uint16 * uint16 -> uint32
            ;
x16:        .WORD $1234
y16:        .WORD $5678
result32:   .BYTE 0,0,0,0

            .ORG $2000
            ;
            ; uint8 * uint8 -> uint16
            ;
            LDX #8              ; loop count
            LDA #0
            STA result16
            STA result16 + 1
startloop:  ASL result16
            ROL result16 + 1
            ASL y8
            BCC endloop
            CLC
            LDA x8
            ADC result16
            STA result16
            BCC endloop
            INC result16 + 1
endloop:    DEX
            BNE startloop
            RTS

            .ORG $2100
            ; uint16 * uint16 -> uint32
            LDX #16 ; counter
            LDA #$00
            STA result32
            STA result32 + 1
            STA result32 + 2
            STA result32 + 3
            JMP skip
startloop2: ASL result32
            ROL result32 + 1
            ROL result32 + 2
            ROL result32 + 3
skip:       ASL y16
            ROL y16 + 1
            BCC endloop2
            CLC
            LDA x16
            ADC result32
            STA result32
            LDA x16 + 1
            ADC result32 + 1
            STA result32 + 1
            BCC endloop2
            INC result32 + 2
            BCC endloop2
            INC result32 + 3
endloop2:   DEX
            BNE startloop2
            RTS

.END
