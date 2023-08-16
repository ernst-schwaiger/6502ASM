            ;
            ; data segment
            ;
            stack = $100

            .ORG $2000
x           .BYTE $20
y           .BYTE $21
tmp16       .WORD $0000

            ;
            ; code
            ;
            .ORG $C000

            ;
            ; mul 8bit, 8bit -> 16bit
            ;
            ; X   (SP + 4)
            ; Y   (SP + 3)
            ; RET (SP + 1)
            ; SP 
mul         TSX
            CLC
            LDY stack + 3,x     ; Y -> Y reg
            BEQ mul_zero_y
            LDA #$0
            STA stack + 3,x     ; clear high byte of result            
            LDA stack + 4,x     ; X -> Accumulator
            BEQ mul_zero_x
mul_loop    DEY
            BEQ mul_end
            ADC stack + 4,x
            BCC mul_loop
            STA stack,x         ; save low byte
            LDA stack + 3,x     ; increment high byte
            ADC #$0
            STA stack + 3,x
            LDA stack,x         ; reload low byte
            JMP mul_loop
mul_zero_y  LDA #$0             ; Y was zero -> zero X
mul_end     STA stack + 4,x     ; write back lower byte of result or zero
            RTS
mul_zero_x  LDA #$0             ; X was zero -> zero Y
            STA stack + 3,x     ; write back zero
            RTS
            ;
            ; mul2
            ;
mul2        TSX
            LDA stack + 4,x     ; X
            PHA
            LDA stack + 3,x     ; Y
            TAY
            LDA #$0             ; high byte of temp sum
            PHA            
            STA stack + 4,x     ; return value ->0
            STA stack + 3,x
            LDA stack,x         ; check any factor against 0
            BEQ mul2_end        ; -> we are finished
            TYA
            BEQ mul2_end
mul2_loop   LSR                 
            BCC mul2_shfac
            TAY                 ; carry set -> add current
            CLC                 ; factor to return value
            LDA stack,x
            ADC stack + 4,x
            STA stack + 4,x
            LDA stack - 1,x
            ADC stack + 3,x
            STA stack + 3,x
            TYA
mul2_shfac  BEQ mul2_end  
            ROL stack,x         ; setup next order bit factor
            ROL stack - 1,x
            JMP mul2_loop
mul2_end    TXS
            RTS
            
            ;
            ; main
            ;
            .ORG $400
            ; first arg
            LDA (x)
            PHA
            ; second arg
            LDA (y)
            PHA
            JSR mul
            PLA 
            STA tmp16 + 1       ; high byte of result
            PLA
            STA tmp16           ; low byte of result
end         RTS
.END
