
            ; symbols before references in code
            IRQ_HANDLER     =   $EA31
            FRAME_COLOR     =   $D020
            CUST_IRQ_HND    =   $6000
            IRQ_VECTOR      =   $0314

            ; NMI Handler
            .ORG CUST_IRQ_HND
            INC FRAME_COLOR
            JMP IRQ_HANDLER
            ; reset irq vector
            .ORG (CUST_IRQ_HND + $20)
            SEI
            LDA (CUST_IRQ_HND / 256)
            LDX (CUST_IRQ_HND % 256)
            STX IRQ_VECTOR
            STA (IRQ_VECTOR + 1)
            CLI
            RTS
.END