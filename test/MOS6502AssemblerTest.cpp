/*
 * MOS6502AssemblerTest.cpp
 *
 *  Created on: 19.08.2018
 *      Author: Ernst
 */
#include <catch2/catch_test_macros.hpp>

#include "MOS6502TestHelper.h"

using namespace antlr4;

namespace asm6502
{

//The output of an assembled program is a list of contiguous mem blocks
TEST_CASE( "compare single memory blocks", "MemBlocks" )
{
    asm6502::MemBlock mb1 {123, {1,2,3,4}};
    asm6502::MemBlock mb2 {124, {1,2,3,4}};
    asm6502::MemBlock mb3 {123, {1,2,3}};
    asm6502::MemBlock mb4 {123, {1,2,3,5}};
    asm6502::MemBlock mb6 {123, {1,2,3,4}};

    REQUIRE(mb1 == mb1);
    REQUIRE(mb1 != mb2);
    REQUIRE(mb1 != mb3);
    REQUIRE(mb1 != mb4);
    REQUIRE(mb1 == mb6);
}

TEST_CASE( "compare lists of memory blocks", "MemBlocks" )
{
    asm6502::MemBlock mb1 {123, {1,2,3,4}};
    asm6502::MemBlock mb2 {124, {1,2,3,4}};

    asm6502::MemBlocks mbs1({mb1, mb2});
    asm6502::MemBlocks mbs2({mb2, mb1});
    asm6502::MemBlocks mbs3({mb1});

    REQUIRE(mbs1 == mbs1);
    REQUIRE(mbs1 != mbs2);
    REQUIRE(mbs1 != mbs3);
}

TEST_CASE( "immediate and absolute addressing", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << ".ORG $1000" << std::endl
        << "LDA #$00" << std::endl
        << "STA $D020" << std::endl
        << "LDA #(2 - 1)" << std::endl
        << "STA ($D020 + $1)" << std::endl
        << "RTS" << std::endl;
    testAssembly(prog, 
        MemBlocks(
            {{0x1000, {0xA9,0x00,0x8D,0x20,0xD0,0xA9,0x01,0x8D,0x21,0xD0,0x60}}}
            )
        );
}

TEST_CASE( "zero page addressing", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << "            .ORG $FC  "
        << "firstByte:  .BYTE $00 "
        << "secondByte: .BYTE $01 "
        << "firstWord:  .WORD $2345 "
        << "            .ORG $1000        "
        << "            LDA firstByte "
        << "            LDX secondByte,Y "
        << "            STA [secondByte,X] "
        << "            STA [secondByte],Y "
        << "            RTS ";

    testAssembly(prog, 
        MemBlocks({
        {0x00FC, {0x00, 0x01, 0x45, 0x23}},
        {0x1000, {0xA5, 0xFC, 0xB6, 0xFD, 0x81, 0xFD, 0x91, 0xFD, 0x60}}
        }));
}

TEST_CASE( "resolve mem addresses", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << "            .ORG $FC "
        << "firstByte:  .BYTE $00 "
        << "secondByte: .BYTE $01 "
        << "firstWord:  .WORD $2345 "
        << "            .ORG $1000 "
        << "            LDA #firstByte "
        << "            LDX #(firstWord / 256) "
        << "            LDY #(firstWord % 256) "
        << "            RTS "
        ;
    testAssembly(prog, 
        MemBlocks({
                {0x00FC, {0x00, 0x01, 0x45, 0x23}},
                {0x1000, {0xA9, 0xFC, 0xA2, 0x00, 0xA0, 0xFE, 0x60}}
                })
        );
}

TEST_CASE( "resolve immediate values", "6502 Assembler" )
{
    // the lo/hi bytes of the address "irqhnd" can be read as immediate
    std::stringstream prog;
    prog 
        << "            IRQ_HND_VECTOR_LO = $0314 "
        << "            IRQ_HND_VECTOR_HI = $0315 "
        << "            FRAME_COLOR       = $D020 "
        << " "
        << "            .ORG $2000 "
        << "            LDA IRQ_HND_VECTOR_LO "
        << "            STA oldhnd "
        << "            LDA IRQ_HND_VECTOR_HI "
        << "            STA (oldhnd + 1) "
        << "            SEI "
        << "            LDA #(irqhnd % 256) "
        << "            STA IRQ_HND_VECTOR_LO "
        << "            LDA #(irqhnd / 256) "
        << "            STA IRQ_HND_VECTOR_HI "
        << "            CLI "
        << "oldhnd:     .WORD $0000 "
        << "irqhnd:     INC FRAME_COLOR "
        << "            JMP [oldhnd] "
        ;

    testAssembly(prog, 
        MemBlocks(
            {
                {
                    0x2000, 
                    { 
                        0xad, 0x14, 0x03, 
                        0x8d, 0x18, 0x20,
                        0xad, 0x15, 0x03, 
                        0x8d, 0x19, 0x20,
                        0x78, 
                        0xa9, 0x1a, 
                        0x8d, 0x14, 0x03, 
                        0xa9, 0x20, 
                        0x8d, 0x15, 0x03, 
                        0x58,
                        0x00, 0x00, 
                        0xee, 0x20, 0xd0, 
                        0x6c, 0x18, 0x20
                    }
                }
            })
        );        
}

TEST_CASE( "resolve indexed-indirect and indexed-indirect base addresses", "6502 Assembler" )
{
    std::stringstream prog;
    prog
        << "            .ORG $2000 " << std::endl
        << "            LDA #MY_OPERAND " << std::endl
        << "            LDA [MY_OPERAND],Y " << std::endl
        << "            LDA [MY_OPERAND,X] " << std::endl
        << "            RTS " << std::endl
        << "            MY_OPERAND = $FE " << std::endl
    ;

    testAssembly(prog, MemBlocks({{0x2000, { 0xa9,0xfe, 0xb1,0xfe, 0xa1,0xfe, 0x60 }}}));     
}

TEST_CASE( "branching and jumping", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << "            .ORG $1000 "
        << "            JMP skip"
        << "            NOP "
        << "skip:       LDY #10 "
        << "br_back:    DEY "
        << "            BNE br_back "
        << "            LDA $fe"
        << "            BNE br_forward "
        << "            LDA #$ee "
        << "            STA $fe "
        << "br_forward: NOP "
        << "            JMP skip "
        ;

    testAssembly(prog, 
        MemBlocks({
            {0x1000, { 0x4c, 0x04, 0x10, 0xea, 0xa0, 0x0a, 0x88, 0xd0, 
                        0xfd, 0xa5, 0xfe, 0xd0, 0x04, 0xa9, 0xee, 0x85, 
                        0xfe, 0xea, 0x4c, 0x04, 0x10}}
            })
        ); 
}

TEST_CASE( "calling subroutines", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << "            .ORG $1000 "
        << "routine1:   RTS "
        << "            .ORG $2000 "
        << "main:       JSR routine1"
        << "            JSR routine2"
        << "            RTS "
        << "            .ORG $3000 "
        << "routine2:   RTS "
        ;
    testAssembly(prog, 
        MemBlocks({
            {0x1000, { 0x60}},
            {0x2000, { 0x20, 0x00, 0x10, 0x20, 0x00, 0x30, 0x60}},
            {0x3000, { 0x60}}
            })
        ); 
}




} /* namespace asm6502 */
