            ;
            ; changes frame color for a short moment
            ;
            .ORG $2000
            LDY #0
label:      STY $D020
            INY
            BNE label
            NOP
            RTS
.END
