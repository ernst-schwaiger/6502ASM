#include <catch2/catch_test_macros.hpp>

#include "MOS6502TestHelper.h"

using namespace antlr4;

namespace asm6502
{
// ensure parser errors and semantic errors are detected


TEST_CASE( "unknown assembly command detected", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << ".ORG $1000" << std::endl
        << "RTE" << std::endl;
    testErrors(prog, {3}); // error on line 3
}

TEST_CASE( "duplicate jump labels detected", "6502 Assembler" )
{
    std::stringstream prog;
    prog 
        << ".ORG $1000 " << std::endl
        << "    duplicatedJumpLabel: LDA#$01 " << std::endl
        << "    duplicatedJumpLabel: LDA#$02 " << std::endl;
    testErrors(prog, {3});
}

TEST_CASE( "duplicate symbols detected", "6502 Assembler" )
{
    std::stringstream prog;
    prog
        << "    FOO=42" << std::endl
        << "    BAR=43" << std::endl
        << "    FOO=44" << std::endl
        << "    .ORG $1000 " << std::endl
        << "    LDA#$01 " << std::endl
        ;
    testErrors(prog, {3});
}

TEST_CASE( "data directives with out of range values detected", "6502 Assembler" )
{
    std::stringstream prog;
    prog
        << "        .ORG $1000 " << std::endl
        << "        .BYTE $01 " << std::endl
        << "        .BYTE $100 " << std::endl       // out of bounds
        << "        .WORD $FFFE " << std::endl
        << "        .WORD $FFFE1 " << std::endl     // out of bounds
        << "        .DBYTE $1234 " << std::endl
        << "        .WORD $12345 " << std::endl     // out of bounds
    ;
    testErrors(prog, {3, 5, 7});
}

TEST_CASE( "usage of undefined symbols", "6502 Assembler" )
{
    std::stringstream prog;
    prog
        << "        FOO=$12 " << std::endl
        << "        .ORG $1000 " << std::endl
        << "        LDA #FOO " << std::endl // immediate: OK
        << "        LDA FOO " << std::endl // absolute: OK as well
        << "        LDA #BAR " << std::endl // immediate: error symbol not available
        << "        LDA BAR " << std::endl // absolute: error symbol not available
        << "        JMP [FOO]" << std::endl // indirect: OK
        << "        JMP [BAR]" << std::endl // indirect: error symbol not available
        ;

    testErrors(prog, {5, 6, 8});
}

TEST_CASE( "forward jmp and branch targets", "6502 Assembler" )
{
    std::stringstream prog;
    prog
        << "        .ORG $1000 " << std::endl
        << "        INY " << std::endl
        << "        BMI skip_unresolved " << std::endl
        << "        LDY #$00 " << std::endl
        << "skip:   TYA " << std::endl
        << "        JMP skip2_unresolved " << std::endl
        << "        NOP " << std::endl
        << "skip2:  RTS " << std::endl
    ;

    testErrors(prog, {3, 6});
}

TEST_CASE( "branch target too far away", "6502 Assembler" )
{
    std::stringstream prog;
    prog
        << "            .ORG $1000 " << std::endl
        << "            INY " << std::endl
        << "            BMI far_away " << std::endl
        << "            .ORG $2000 " << std::endl
        << "far_away:   RTS " << std::endl
    ;

    testErrors(prog, {3});
}

TEST_CASE( "operands too large", "6502 Assembler" )
{

    std::stringstream prog;
    prog
        << "            MY_OPERAND = $100 " << std::endl
        << "            .ORG $1000 " << std::endl
        << "            LDA #MY_OPERAND " << std::endl
        << "            LDA [MY_OPERAND],Y " << std::endl
        << "            LDA [MY_OPERAND,X] " << std::endl
        << "            RTS " << std::endl
    ;

    testErrors(prog, {3, 4, 5});
}

TEST_CASE( "resolved operands too large", "6502 Assembler" )
{

    std::stringstream prog;
    prog
        << "            .ORG $1000 " << std::endl
        << "            LDA #MY_OPERAND " << std::endl
        << "            LDA [MY_OPERAND],Y " << std::endl
        << "            LDA [MY_OPERAND,X] " << std::endl
        << "            RTS " << std::endl
        << "            MY_OPERAND = $100 " << std::endl
    ;

    testErrors(prog, {2, 3, 4});
}

}