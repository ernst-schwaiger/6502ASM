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
    CodeLine(MOS6502Parser::LineContext *_ctx, uint32_t _startAddress, uint32_t _lengthBytes) :
        startAddress {_startAddress },
        lengthBytes {_lengthBytes},
        label { getLabel(_ctx) },
        assembly { getAssembly(_ctx) }
    {};

    std::string get(asm6502::MemBlocks const &mb, bool addAssembly) const;
    uint32_t getStartAddress() const { return startAddress; }
    uint32_t getLengthBytes() const { return lengthBytes; }

private:
    auto getMachineCode(asm6502::MemBlocks const &mb) const -> std::string;

    static auto getLabel(MOS6502Parser::LineContext *_ctx) -> std::string;
    static auto getAssembly(MOS6502Parser::LineContext *_ctx) -> std::string;
    static auto prettyPrintDirOrStatement(antlr4::RuleContext *dirOrStatementCtx) -> std::string;
    static auto getWhitespaceBetweenTokens(antlr4::tree::ParseTree *terminalNode, antlr4::tree::ParseTree *prevTerminalNode) -> std::string;

    uint32_t startAddress;
    uint32_t lengthBytes;
    std::string label;
    std::string assembly;
};

}

#endif