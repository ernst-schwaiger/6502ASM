            ; symbols before references in code
            TOP_OF_STACK=$100
            ;
            ; data segment before references in code
            ;
zeroPage:   .ORG $0000
byteZero:   .BYTE $FF
byteOne:    .BYTE $FE
addr:       .WORD $1234

            .ORG $1000
wordZero:   .WORD $5678
wordOne:    .WORD $9ABC

            .ORG $2000
            ; zero-paged, absolute
            LDX byteZero
            ; zero-paged, indexed
            LDA byteOne,x
            ; indirect indexed
            LDA [addr],y
            ; indexed indirect
            LDA [addr,x]
            ; FIXME LDA [addr],x causes a parse error, then a crash
            ; expression using address represented by label
            LDA #(wordOne % 256) ; LSB of address
            LDY #(wordOne / 256) ; MSB of address
            ; expression
            LDX #(1 + 3)
            ; usage of symbol
            LDA TOP_OF_STACK,x
            JSR subroutine
            RTS

subroutine: LDX #12
            RTS

            ;
            ; now referencing symbols *that follow*
            ;
            JMP skip
            NOP
            ; absolute
skip:       LDY lastByte
            ; indexed
            LDA lastWord,y
            ; expression using address represented by label
            ; FIXME this does not work currently
            LDA (lastWord % 256) ; LSB of address
            LDY (lastWord / 256) ; MSB of address
            LDA BOTTOM_OF_STACK,x
            JSR subroutine

            ;
            ; data segment before references in code
            ;
            .ORG $8000
lastWord:   .WORD $DEAD
lastByte:   .BYTE $12

            ; symbols after references in code
            BOTTOM_OF_STACK=$1FF

.END
