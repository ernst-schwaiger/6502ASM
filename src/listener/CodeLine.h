#ifndef CODE_LINE_H
#define CODE_LINE_H

#include <MOS6502Parser.h>

namespace asm6502
{
// Models a complete Line statement, a start address and a byte length
// to associate assembler code and the bytes that have been generated
// for it

class MemBlocks;

class CodeLine
{
public:
    CodeLine(MOS6502Parser::LineContext *_ctx, unsigned int _startAddress, unsigned int _lengthBytes) :
        startAddress {_startAddress },
        lengthBytes {_lengthBytes},
        label { getLabel(_ctx) },
        assembly { getAssembly(_ctx) }
    {};

    std::string get(asm6502::MemBlocks const &mb, bool addAssembly) const;
    unsigned int getStartAddress() const { return startAddress; }
    unsigned int getLengthBytes() const { return lengthBytes; }

private:

    std::string getLabel(MOS6502Parser::LineContext *_ctx);
    std::string getAssembly(MOS6502Parser::LineContext *_ctx);

    std::string getMachineCode(asm6502::MemBlocks const &mb) const;
    std::string prettyPrintDirOrStatement(antlr4::RuleContext *dirOrStatementCtx) const;
    std::string getWhitespaceBetweenTokens(antlr4::tree::ParseTree *terminalNode, antlr4::tree::ParseTree *prevTerminalNode) const;

    unsigned int startAddress;
    unsigned int lengthBytes;
    std::string label;
    std::string assembly;
};

}

#endif