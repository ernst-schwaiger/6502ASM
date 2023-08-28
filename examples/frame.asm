            ;
            ; changes frame color for a short moment
            ;
            .ORG $C000
            LDY #0
label:      STY $D020
            INY
            BNE label
            RTS
.END
