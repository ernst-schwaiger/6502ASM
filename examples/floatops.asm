

            ; data
            .ORG 2200
pi          .BYTE "12345"

            ; redirect USR vector to our routine
            .ORG $2000
            LDA #$00
            STA $0311
            LDA #$21
            STA $0312
            RTS

            ; called via USR(f)
            .ORG $2100            
            ; call MOVAF FAC -> ARG
            JSR $BC0C
            ; call MOVMF FAC -> dest in 5 bytes packed format
            LDX #$00
            LDY #$22
            JSR $BBD4
            RTS

.END