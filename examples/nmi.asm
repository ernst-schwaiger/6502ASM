
            ; symbols before references in code
            REG_13_NMI_CIA  =   $DD0D
            NMI_CIA_HANDLER =   $FE72
            NMI_HANDLER     =   $FEBC
            FRAME_COLOR     =   $D020
            CUST_NMI_HND    =   $6000
            NMI_VECTOR      =   $0318

            ; NMI Handler
            .ORG CUST_NMI_HND
            PHA
            TXA
            PHA
            TYA
            PHA
            ; stop NMI CIA from generating interrupts
            LDA #$7F
            STA REG_13_NMI_CIA
            ; check whether NMI CIA caused the interrupt
            LDA REG_13_NMI_CIA
            BMI h_nmi_cia       ; handle cia nmi
            LDA FRAME_COL
            ADC #$01
            STA FRAME_COL
            JMP NMI_HANDLER
h_nmi_cia:  JMP NMI_CIA_HANDLER
            ; reset interrupt handler
            .ORG (CUST_NMI_HND + $20)
            LDA (CUST_NMI_HND / 256)
            LDX (CUST_NMI_HND % 256)
            STX NMI_VECTOR
            STA (NMI_VECTOR + 1)
            RET
.END