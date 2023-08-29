
            USRISR      = $2028
            IRQVECLO    = $0314
            IRQVECHI    = $0315

            RASTERIRQ   = $D012
            RASTERIRQHI = $D011

            IRQ_ENABLE  = $D01A
            IRQ_REQUEST = $D019
            FRAME_COLOR = $D020
            VIC_HANDLER = $EA81

            CIA_REQUEST = $DC0D
            CIA_HANDLER = $EA31

            RASTER_START= $32
            RASTER_END  = $F8
            LINEWIDTH   = $04
            LINEWITH_M  = $02 ; we store the linewidth here


            .ORG $2000 
            ; redirecting IRQ vector to our routine
            SEI
            LDA #(USRISR % 256)
            STA IRQVECLO
            LDA #(USRISR / 256)
            STA IRQVECHI
            ; trigger IRQ if raster ray reaches line RASTER_END
            LDA #RASTER_END
            STA RASTERIRQ
            LDA RASTERIRQHI
            AND #$7F
            STA RASTERIRQHI
            ; enable IRQs triggered by raster ray
            LDA #$81
            STA IRQ_ENABLE

            ; frame set to black, init line width for ISR
            LDA #$00
            STA FRAME_COLOR
            LDA #LINEWIDTH
            STA LINEWITH_M
            CLI
            RTS

            .ORG USRISR ; our custom VIC ISR
            ; read from request register, determine if source is VIC
            LDA IRQ_REQUEST
            ; setting the bits clears the register
            STA IRQ_REQUEST
            ; bit 7 is set -> video interrupt
            BMI handle_vic
            ; prepare handling CIA interrupt
            LDA CIA_REQUEST
            CLI
            JMP CIA_HANDLER
handle_vic: LDA RASTERIRQ   ; get raster number that triggered the IRQ
            CMP #RASTER_END ; raster line beyond RASTER_END ?
            BCS skip_color
            CLC
            ADC LINEWITH_M
            STA RASTERIRQ   ; next irq trigger
            LDY #$03        ; busy waiting (?)
loop:       DEY
            BNE loop
            INC FRAME_COLOR
            JMP VIC_HANDLER
skip_color: LDA #$00            ; frame color is black
            STA FRAME_COLOR
            LDA #RASTER_START   ; trigger next irq at top
            STA RASTERIRQ
            JMP VIC_HANDLER

            .ORG $2100          ; reset IRQ vector
            SEI
            LDA (VIC_HANDLER % 256)
            STA IRQVECLO
            LDA (VIC_HANDLER / 256)
            STA IRQVECHI
            CLI
            RTS
