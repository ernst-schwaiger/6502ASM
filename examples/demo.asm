            ;
            ; data segment
            ;
            .ORG $200
myString    .BYTE "dies ist ein test", 0
            .ORG $220
toBeFilled  .BYTE "                 " ; // here the sting will be copied
            .ORG $80
src         .WORD myString
dest        .WORD toBeFilled
            ;
            ; code
            ;
            .ORG $400
            LDY #0
strcpy      LDA [src],y
            STA [dest],y
            INY
            BNE strcpy
end_strcpy  BRK
            .ORG $500 ; just testing rubbish
label SBC 42 ;comment
label2 ADC #12
;comment again
ADC #0
ADC #255
ADC #249
ADC #99
ADC #20
ADC #'_'
ADC #'#'
;depp ADC #'a'
ADC #%00000000
ADC #%11111111
ADC #%00011100
ADC #%10101
ADC #$0
ADC #$f
ADC #$00
ADC #$ff
ADC #$ef
ADC #$12
ADC #$0a
ADC #$ab
ADC #12;
ADC #12;
ADC #0;
ADC 1234,x;
ADC $1234,y
ADC 7834,x
ADC 256
ADC %010,y
ADC [depp,x]
ADC [depp],y
ADC [$fecb],y
BRK;
JMP (12345)
BNE depp

ADC (DEPP - 3) / FOO
.BYTE 12345, 23, %00001111, $ff, 3 + 4, 'f', orsch
.BYTE "orsch2", 'a', depp

SBC A + B / C
SBC (A + B) / C

.END
