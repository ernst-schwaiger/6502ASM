# 6502ASM
Assembler for the MOS 6502 processor

## Build

```
mkdir 6502ASM/build
cd 6502ASM/build
cmake ..
make -j
```

## Usage

``ASM6502 <asmfile> [-a] [-b] [-p <progfile>]``

``-a``: output assembly and machine code bytes"

``-b``: output C64 basic program that pokes machine code into RAM"

``-p <progfile>``: write machine code into a progfile (C64 .PRG)" << endl;


``ASM6502 <asmfile>`` produces machine code and BASIC program that copies the code into memory

``6502ASM examples/frame.asm`` produces

```
--- 6502 Machine Code ---
                                    .ORG $C000
0xc000:0xa0,0x00                    LDY #0
0xc002:0x8c,0x20,0xd0   label:      STY $D020
0xc005:0xc8                         INY
0xc006:0xd0,0xfa                    BNE label
0xc008:0x60                         RTS

--- Commodore Basic Initializer Listing ---
100 read nb
110 for bi = 1 to nb
120 read addr
130 read nby
140 for byi = 1 to nby
150 read byvl
160 poke addr+byi-1, byvl
170 next byi
180 next bi
190 end
200 rem number of mem blocks
210 data 1
220 rem block start number bytes
230 data 49152, 9
240 data 160,  0,140, 32
250 data 208,200,208,250
260 data  96
```

## ToDos
* Add support for deferred config also for immediate operands
* Add argument length 1/2 byte and info for potential zero page addressing also
* Add semantic checks: Indexed indirect, indirect indexed on non-zero base
* Add warning: zero-page defined after code that references it. zero-page opt cannot be made
(does non zero page with zero page operand actually work?)
* Test for all ASM commands including all addressing modes
