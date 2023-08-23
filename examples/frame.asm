            ;
            ; changes frame color for a short moment
            ;
            .ORG $C000
            LDY #0
label2:     STY $D020
            INY
            BNE label2
            RTS
.END
