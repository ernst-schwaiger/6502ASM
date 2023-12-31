/*
 * MOS6502Listener.cpp
 *
 *  Created on: 19.08.2018
 *      Author: Ernst
 */
#include <functional>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <iterator>
#include <string>

#include "MOS6502Listener.h"

using namespace std;

namespace asm6502
{

static uint8_t const jmp_indir_opcode = 0x6C;

static map<string, uint8_t> const dir_opcodes
{
    {"BRK", 0x00}, {"PHP", 0x08}, {"ASL", 0x0A}, {"CLC", 0x18}, {"PLP", 0x28}, {"ROL", 0x2A}, {"SEC", 0x38}, {"RTI", 0x40},
    {"PHA", 0x48}, {"LSR", 0x4A}, {"CLI", 0x58}, {"RTS", 0x60}, {"PLA", 0x68}, {"ROR", 0x6A}, {"SEI", 0x78}, {"DEY", 0x88},
    {"TXA", 0x8A}, {"TYA", 0x98}, {"TXS", 0x9A}, {"TAY", 0xA8}, {"TAX", 0xAA}, {"CLV", 0xB8}, {"TSX", 0xBA}, {"INY", 0xC8},
    {"DEX", 0xCA}, {"CLD", 0xD8}, {"INX", 0xE8}, {"NOP", 0xEA}, {"SED", 0xF8}
};

static map<string, uint8_t> const imm_opcodes
{
    {"ORA", 0x09}, {"AND", 0x29}, {"EOR", 0x49}, {"ADC", 0x69}, {"LDY", 0xA0}, {"LDX", 0xA2}, {"LDA", 0xA9}, {"CPY", 0xC0},
    {"CMP", 0xC9}, {"CPX", 0xE0}, {"SBC", 0xE9}
};

static map<string, uint8_t> const rel_opcodes
{
    {"BPL", 0x10}, {"BMI", 0x30}, {"BVC", 0x50}, {"BVS", 0x70}, {"BCC", 0x90}, {"BCS", 0xB0}, {"BNE", 0xD0}, {"BEQ", 0xF0}
};

static map<string, uint8_t> const idx_x_opcodes
{
    {"ORA", 0x1D}, {"ASL", 0x1E}, {"AND", 0x3D}, {"ROL", 0x3E}, {"EOR", 0x5D}, {"LSR", 0x5E}, {"ADC", 0x7D}, {"ROR", 0x7E},
    {"STA", 0x9D}, {"LDY", 0xBC}, {"LDA", 0xBD}, {"CMP", 0xDD}, {"DEC", 0xDE}, {"SBC", 0xFD}, {"INC", 0xFE}
};

static map<string, uint8_t> const idx_x_zpg_opcodes
{
    {"ORA", 0x15}, {"ASL", 0x16}, {"AND", 0x35}, {"ROL", 0x36}, {"EOR", 0x55}, {"LSR", 0x56}, {"ADC", 0x75}, {"ROR", 0x76},
    {"STY", 0x94}, {"STA", 0x95}, {"LDY", 0xB4}, {"LDA", 0xB5}, {"CMP", 0xD5}, {"DEC", 0xD6}, {"DBC", 0xF5}, {"INC", 0xF6}
};

static map<string, uint8_t> const idx_y_opcodes
{
    {"ORA", 0x19}, {"AND", 0x39}, {"EOR", 0x59}, {"ADC", 0x79}, {"STA", 0x99}, {"LDA", 0xB9}, {"LDX", 0xBE}, {"CMP", 0xD9},
    {"SBC", 0xF9}
};

static map<string, uint8_t> const idx_y_zpg_opcodes
{
    {"STX", 0x96}, {"LDX", 0xB6}
};

static map<string, uint8_t> const abs_opcodes
{
    {"ORA", 0x0D}, {"ASL", 0x0E}, {"JSR", 0x20}, {"BIT", 0x2C}, {"AND", 0x2D}, {"ROL", 0x2E}, {"JMP", 0x4C},
    {"EOR", 0x4D}, {"LSR", 0x4E}, {"ADC", 0x6D}, {"ROR", 0x6E}, {"STY", 0x8C}, {"STA", 0x8D}, {"STX", 0x8E}, {"LDY", 0xAC},
    {"LDA", 0xAD}, {"LDX", 0xAE}, {"CPY", 0xCC}, {"CMP", 0xCD}, {"DEC", 0xCE}, {"CPX", 0xEC}, {"SBC", 0xED}, {"INC", 0xEE}
};

static map<string, uint8_t> const abs_zpg_opcodes
{
    {"ORA", 0x05}, {"ASL", 0x06}, {"BIT", 0x24}, {"AND", 0x25}, {"ROL", 0x26}, {"EOR", 0x45}, {"LSR", 0x46}, {"ADC", 0x65},
    {"ROR", 0x66}, {"STY", 0x84}, {"STA", 0x85}, {"STX", 0x86}, {"LDY", 0xA4}, {"LDA", 0xA5}, {"LDX", 0xA6}, {"CPY", 0xC4},
    {"CMP", 0xC5}, {"DEC", 0xC6}, {"CPX", 0xE4}, {"SBC", 0xE5}, {"INC", 0xE6}
};

static map<string, uint8_t> const idx_idr_opcodes
{
    {"ORA", 0x01}, {"AND", 0x21}, {"EOR", 0x41}, {"ADC", 0x61}, {"STA", 0x81}, {"LDA", 0xA1}, {"CMP", 0xC1}, {"SBC", 0xE1}
};

static map<string, uint8_t> const idr_idx_opcodes
{
    {"ORA", 0x11}, {"AND", 0x31}, {"EOR", 0x51}, {"ADC", 0x71}, {"STA", 0x91}, {"LDA", 0xB1}, {"CMP", 0xD1}, {"SBC", 0xF1}
};


auto findOpCode(map<string, uint8_t> const &opcodeMap, string const &opcode) -> uint8_t
{
    uint8_t ret = 0;
    auto pos = opcodeMap.find(opcode);

    if (pos != end(opcodeMap))
    {
        ret = pos->second;
    }

    return ret;
}

auto findIdxZpgOpCodes(map<string, uint8_t> const &opcodeMap, map<string, uint8_t> const &zpgOpcodeMap, string const &opCode) -> pair<uint8_t, uint8_t> 
{
    uint8_t idxOpCode = findOpCode(opcodeMap, opCode);
    uint8_t zgpOpCode = findOpCode(zpgOpcodeMap, opCode);

    return {idxOpCode, zgpOpCode};
}


static function<TOptExprValue(TOptExprValue, TOptExprValue)> const add = [](TOptExprValue arg1, TOptExprValue arg2)
{
    TOptExprValue ret = std::nullopt;

    if ((arg1 != std::nullopt) && (arg2 != std::nullopt))
    {
        ret = TOptExprValue(arg1.value() + arg2.value());
    }

    return ret;
};

static function<TOptExprValue(TOptExprValue, TOptExprValue)> const sub = [](TOptExprValue arg1, TOptExprValue arg2)
{
    TOptExprValue ret = std::nullopt;

    if ((arg1 != std::nullopt) && (arg2 != std::nullopt))
    {
        ret = TOptExprValue(arg1.value() - arg2.value());
    }

    return ret;
};

static function<TOptExprValue(TOptExprValue, TOptExprValue)> const div = [](TOptExprValue arg1, TOptExprValue arg2)
{
    TOptExprValue ret = std::nullopt;

    if ((arg1 != std::nullopt) && (arg2 != std::nullopt))
    {
        ret = TOptExprValue(arg1.value() / arg2.value());
    }

    return ret;
};

static function<TOptExprValue(TOptExprValue, TOptExprValue)> const mul = [](TOptExprValue arg1, TOptExprValue arg2)
{
    TOptExprValue ret = std::nullopt;

    if ((arg1 != std::nullopt) && (arg2 != std::nullopt))
    {
        ret = TOptExprValue(arg1.value() * arg2.value());
    }

    return ret;
};

static function<TOptExprValue(TOptExprValue, TOptExprValue)> const mod = [](TOptExprValue arg1, TOptExprValue arg2)
{
    TOptExprValue ret = std::nullopt;

    if ((arg1 != std::nullopt) && (arg2 != std::nullopt))
    {
        ret = TOptExprValue(arg1.value() % arg2.value());
    }

    return ret;
};

class ExpressionBase : public IExpression
{
public:
    ExpressionBase(size_t line_, size_t column_) : line{line_}, column{column_} {}
    [[nodiscard]] auto getLine() const -> size_t override { return line; }
    [[nodiscard]] auto getColumn() const -> size_t override { return column; }
private:
    size_t line;
    size_t column;
};

class Numeric : public ExpressionBase
{
public:
    Numeric(uint32_t _val, size_t line_, size_t col_) : ExpressionBase {line_, col_}, val{_val} {}
    [[nodiscard]] auto eval(SymbolTable const &symbolTable) const -> TOptExprValue override 
    {
        return {val};
    }

    [[nodiscard]] auto getText() const -> std::string override
    {
        std::stringstream strm;
        strm << val;
        return strm.str();
    }

private:
    uint32_t val;
};

class Symbol : public ExpressionBase
{
public:
    Symbol(string const _symbol, size_t line_, size_t col_) : ExpressionBase {line_, col_}, symbol{_symbol} {}
    [[nodiscard]] auto eval(SymbolTable const &symbolTable) const -> TOptExprValue override
    {
        TOptExprValue ret = std::nullopt;

        std::optional<Sym> optSym = symbolTable.resolveSymbol(symbol);

        if (optSym != std::nullopt)
        {
            ret = TOptExprValue(optSym.value().val);
        }

        return ret;
    }

    [[nodiscard]] auto getText() const -> std::string override { return symbol; }

private:
    string symbol;
};

class BinaryOperation : public ExpressionBase
{
public:
    BinaryOperation(
        shared_ptr<IExpression> _arg1, 
        shared_ptr<IExpression> _arg2, 
        function<TOptExprValue(TOptExprValue, TOptExprValue)> const *_op,
        size_t line_,
        size_t col_) :
        ExpressionBase {line_, col_},
        arg1{_arg1},
        arg2{_arg2},
        op{_op}
        {};

    [[nodiscard]] auto eval(SymbolTable const &symbolTable) const -> TOptExprValue override
    {
        return (*op)(arg1->eval(symbolTable), arg2->eval(symbolTable));
    }

    [[nodiscard]] auto getText() const -> std::string override { return "<<expression>>"; }

private:
    shared_ptr<IExpression> const arg1;
    shared_ptr<IExpression> const arg2;
    function<TOptExprValue(TOptExprValue, TOptExprValue)> const *op;
};


MOS6502Listener::MOS6502Listener(char const *pFileName) :
        fileName{pFileName},
        currentAddress{0},
        addressOfLine{ADDR_INVALID}
{
}

void MOS6502Listener::exitOrg_directive(MOS6502Parser::Org_directiveContext *ctx)
{
    TOptExprValue optCurrAddr = popExpression();
    if (optCurrAddr != std::nullopt)
    {
        addressOfLine = optCurrAddr.value();
        currentAddress = optCurrAddr.value();
    } 
}

void MOS6502Listener::exitByte_directive(MOS6502Parser::Byte_directiveContext *ctx)
{
    for (auto optByte : popAllExpressions())
    {
        if (optByte != std::nullopt)
        {
            auto byteVal = optByte.value();

            if (byteVal > 255)
            {
                addValueOutOfRangeError(byteVal, 0, 255, ctx);
            }
            else
            {
                appendByteToPayload(byteVal);
            }
        }
    }
}

void MOS6502Listener::exitWord_directive(MOS6502Parser::Word_directiveContext *ctx)
{
    for (auto optWord : popAllExpressions())
    {
        if (optWord != std::nullopt)
        {
            auto wordVal = optWord.value();
            if (wordVal > 65535)
            {
                addValueOutOfRangeError(wordVal, 0, 65535, ctx);
            }
            else
            {
                addWordToPayload(wordVal);
            }
        }
    }
}

void MOS6502Listener::exitDbyte_directive(MOS6502Parser::Dbyte_directiveContext *ctx)
{
    for (auto optDbyte : popAllExpressions())
    {
        if (optDbyte != std::nullopt)
        {
            auto dByteVal = optDbyte.value();
            if (dByteVal > 65535)
            {
                addValueOutOfRangeError(dByteVal, 0, 65535, ctx);
            }
            else
            {
                addDByteToPayload(dByteVal);
            }
        }
    }
}

void MOS6502Listener::exitLabel(MOS6502Parser::LabelContext *ctx)
{
    string symName = ctx->ID()->getText();
    addSymbolCheckAlreadyDefined(symName, currentAddress, ctx);
}

void MOS6502Listener::exitAss_directive(MOS6502Parser::Ass_directiveContext *ctx)
{
    string symName = ctx->ID()->getText();
    TOptExprValue optExprVal = popExpression();
    if (optExprVal != std::nullopt)
    {
        addSymbolCheckAlreadyDefined(symName, optExprVal.value(), ctx);
    }
    else
    {
        addMissingSymbolError(symName, line(ctx), col(ctx));
    }
}

void MOS6502Listener::addSymbolCheckAlreadyDefined(string const &symName, uint32_t symVal, antlr4::ParserRuleContext *ctx)
{
    std::optional<Sym> optSym = symbolTable.resolveSymbol(symName);
    if (optSym == std::nullopt)
    {
        symbolTable.addSymbol(symName, line(ctx), col(ctx), symVal);
    }
    else
    {
        addDuplicateSymbolError(symName, optSym.value(), ctx);
    }
}

void MOS6502Listener::exitDir_statement(MOS6502Parser::Dir_statementContext *ctx)
{
    appendByteToPayload(findOpCode(dir_opcodes, ctx->dir_opcode()->getText()));
}

void MOS6502Listener::exitImm_statement(MOS6502Parser::Imm_statementContext *ctx)
{
    auto opCode = findOpCode(imm_opcodes, ctx->imm_opcode()->getText());
    appendIdxIdrOrIdrIdxOrImmCmd(opCode, ctx);
}

void MOS6502Listener::exitRel_statement(MOS6502Parser::Rel_statementContext *ctx)
{
    appendByteToPayload(findOpCode(rel_opcodes, ctx->rel_opcode()->getText()));

    // the relative operand can only be resolved at the end of the assembler
    // run, since labels can be assigned here that have not yet been parsed
    auto label = make_shared<Symbol>(ctx->symbol()->getText(), line(ctx), col(ctx));

    branchTargets.emplace_back(pair<uint32_t, shared_ptr<IExpression>>{currentAddress, label});
    ++currentAddress;
}

void MOS6502Listener::exitIdx_x_statement(MOS6502Parser::Idx_x_statementContext *ctx)
{
    auto opcodeStr = ctx->idx_opcode()->getText();
    auto opCodeZpgOpCode = findIdxZpgOpCodes(idx_x_opcodes, idx_x_zpg_opcodes, opcodeStr);
    appendIdxOrZpgCmd(opCodeZpgOpCode.first, opCodeZpgOpCode.second, ctx);
}

void MOS6502Listener::exitIdx_y_statement(MOS6502Parser::Idx_y_statementContext *ctx)
{
    auto opcodeStr = ctx->idy_opcode()->getText();
    auto opCodeZpgOpCode = findIdxZpgOpCodes(idx_y_opcodes, idx_y_zpg_opcodes, opcodeStr);
    appendIdxOrZpgCmd(opCodeZpgOpCode.first, opCodeZpgOpCode.second, ctx);
}

void MOS6502Listener::exitIdx_abs_statement(MOS6502Parser::Idx_abs_statementContext *ctx)
{
    auto opcodeStr = ctx->idabs_opcode()->getText();
    auto opCodeZpgOpCode = findIdxZpgOpCodes(abs_opcodes, abs_zpg_opcodes, opcodeStr);
    appendIdxOrZpgCmd(opCodeZpgOpCode.first, opCodeZpgOpCode.second, ctx);
}

void MOS6502Listener::exitIdx_idr_statement(MOS6502Parser::Idx_idr_statementContext *ctx)
{
    auto opcode = findOpCode(idx_idr_opcodes, ctx->idx_idr_idx_opcode()->getText());
    appendIdxIdrOrIdrIdxOrImmCmd(opcode, ctx);
}

void MOS6502Listener::exitIdr_idx_statement(MOS6502Parser::Idr_idx_statementContext *ctx)
{
    auto opcode = findOpCode(idr_idx_opcodes, ctx->idx_idr_idx_opcode()->getText());
    appendIdxIdrOrIdrIdxOrImmCmd(opcode, ctx);
}

// opcode here is always implied zero-page
void MOS6502Listener::appendIdxIdrOrIdrIdxOrImmCmd(uint8_t opcode, antlr4::ParserRuleContext const *ctx)
{
    shared_ptr<IExpression> pExpression = popNonEvalExpression();

    if (pExpression != nullptr)
    {
        TOptExprValue optOperand = pExpression->eval(symbolTable);

        if (optOperand != std::nullopt)
        {
            // We could evaluate the expression, write the code immediately
            uint32_t operand = optOperand.value();

            if (operand <= 0xffU)
            {
                appendByteToPayload(opcode);
                appendByteToPayload(operand & 0xffU);
            }
            else
            {
                addOperandTooLargeError(operand, line(ctx), col(ctx));
            }
        }
        else
        {
            // The expression could not be evaluated due to a missing symbol we don't know yet
            // Since we now have to reserve payload for the statement, we reserve 2 bytes here
            // one for the opcode, one for the zero-based address
            makeDeferredExpression(opcode, 2, pExpression, currentAddress, line(ctx), col(ctx));
        }
    }
    else
    {
        // Something went wrong. We do not have an expression for our command at all!
        addInternalError(line(ctx), col(ctx));
    }

}


void MOS6502Listener::appendIdxOrZpgCmd(uint8_t opcode, uint8_t opcode_zpg, antlr4::ParserRuleContext const *ctx)
{
    shared_ptr<IExpression> pExpression = popNonEvalExpression();

    if (pExpression != nullptr)
    {
        TOptExprValue optOperand = pExpression->eval(symbolTable);

        if (optOperand != std::nullopt)
        {
            // We could evaluate the expression, write the code immediately
            uint32_t operand = optOperand.value();

            if (operand <= 0xff && opcode_zpg > 0)
            {
                appendByteToPayload(opcode_zpg);
                appendByteToPayload(operand & 0xffU);
            }
            else
            {
                appendByteToPayload(opcode);
                appendByteToPayload(operand & 0xffU);
                appendByteToPayload((operand >> 8U) & 0xffU);
            }
        }
        else
        {
            // The expression could not be evaluated due to a missing symbol we don't know yet
            // Since we now have to reserve payload for the statement, we reserve 3 bytes here
            // one for the opcode, two for the potential 16 bit address
            makeDeferredExpression(opcode, 3, pExpression, currentAddress, line(ctx), col(ctx));
        }
    }
    else
    {
        // Something went wrong. We do not have an expression for our command at all!
        addInternalError(line(ctx), col(ctx));
    }
}

void MOS6502Listener::makeDeferredExpression(uint8_t opcode, uint8_t opNrBytes, shared_ptr<IExpression> pExpression, uint32_t currentAddress, size_t line, size_t col )
{
    deferredExpressionStatements.emplace_back(DeferredExpressionEval(opcode, opNrBytes, pExpression, currentAddress, line, col));
    do { appendByteToPayload(0xff); } while (--opNrBytes > 0);
}


void MOS6502Listener::exitIdr_statement(MOS6502Parser::Idr_statementContext *ctx)
{
    // there is only one indirect op, JMP: 0x6C
    appendIdxOrZpgCmd(jmp_indir_opcode, 0, ctx);
}


void MOS6502Listener::exitExpression(MOS6502Parser::ExpressionContext * ctx)
{
    function<TOptExprValue(TOptExprValue, TOptExprValue)> const *op = nullptr;
    if (ctx->ADD() != nullptr)
    {
        op = &add;
    } else if (ctx->SUB() != nullptr)
    {
        op = &sub;
    } else if (ctx->MUL() != nullptr)
    {
        op = &mul;
    } else if (ctx->DIV() != nullptr)
    {
        op = &div;
    } else if (ctx->PERCENT() != nullptr)
    {
        op = &mod;
    }

    if (op != nullptr)
    {
        auto arg2 = expressionStack.back();
        expressionStack.pop_back();
        auto arg1 = expressionStack.back();
        expressionStack.pop_back();

        expressionStack.emplace_back(make_shared<BinaryOperation>(arg1, arg2, op, line(ctx), col(ctx)));
    }
    else
    {
        // Non operation expressions: labels, symbols, ... processed elsewhere
    }
}

void MOS6502Listener::exitSymbol(MOS6502Parser::SymbolContext *ctx)
{
    uint32_t resolvedSymVal = 0xffffffff; // this is what is put into our expression if the symbol could not be resolved
    string symName = ctx->ID()->getText();
    optional<Sym> optSymbolVal = symbolTable.resolveSymbol(symName);

    if (optSymbolVal == std::nullopt)
    {
        // if the symbol cannot be evaluated for now, add it as an unresolved symbol
        expressionStack.emplace_back(make_shared<Symbol>(symName, line(ctx), col(ctx)));
    }
    else
    {
        resolvedSymVal = optSymbolVal.value().val;
        expressionStack.emplace_back(make_shared<Numeric>(resolvedSymVal, line(ctx), col(ctx)));
    }    
}

void MOS6502Listener::exitDec8(MOS6502Parser::Dec8Context * ctx)
{
    auto val = convertDec(ctx->getText());
    expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
}

void MOS6502Listener::exitDec(MOS6502Parser::DecContext * ctx)
{
    auto val = convertDec(ctx->getText());
    expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
}

void MOS6502Listener::exitHex16(MOS6502Parser::Hex16Context * ctx)
{
    // w/o leading $ sign
    auto val = convertHex(ctx->getText().substr(1));
    expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
}

void MOS6502Listener::exitHex8(MOS6502Parser::Hex8Context * ctx)
{
    // w/o leading $ sign
    auto val = convertHex(ctx->getText().substr(1));
    expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
}

void MOS6502Listener::exitBin8(MOS6502Parser::Bin8Context * ctx)
{
    // w/o leading % sign
    auto val = convertBin(ctx->getText().substr(1));
    expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
}

void MOS6502Listener::exitChar8(MOS6502Parser::Char8Context * ctx)
{
    // w/o leading/trailing apos
    auto val = ctx->getText()[1];
    expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
}

void MOS6502Listener::exitData_string(MOS6502Parser::Data_stringContext * ctx)
{
    auto stringWithQuotes = ctx->STRING()->getText();
    auto stringNoQuotes = stringWithQuotes.substr(1, stringWithQuotes.length() - 2);

    for (auto val : stringNoQuotes)
    {
        expressionStack.emplace_back(make_shared<Numeric>(val, line(ctx), col(ctx)));
    }
}

auto MOS6502Listener::convertDec(string const &dec) -> uint32_t
{
    uint32_t ret = 0;
    stringstream ss;
    ss << dec;
    ss >> ret;

    return ret;
}

auto MOS6502Listener::convertHex(string const &hexa) -> uint32_t
{
    uint32_t ret = 0;
    stringstream ss;
    ss << hex << hexa;
    ss >> ret;

    return ret;
}

auto MOS6502Listener::convertBin(string const &bin) -> uint32_t
{
    uint32_t ret = 0;

    for (auto c : bin)
    {
        ret <<= 1U;

        if (c == '1')
        {
            ret++;
        }
    }

    return ret;
}

void MOS6502Listener::exitLine(MOS6502Parser::LineContext *ctx)
{
    uint32_t startAddress = 0;
    uint32_t numberOfBytes = 0;

    if (addressOfLine != ADDR_INVALID)
    {
        startAddress = addressOfLine;
        numberOfBytes = currentAddress - addressOfLine;
    }

    CodeLine codeLine(ctx, startAddress, numberOfBytes);
    codeLines.push_back(codeLine);

    // the expressions parsed in this codeline are not used any more
    // clean up the list for the next code line
    expressionStack.clear();
    addressOfLine = ADDR_INVALID;
}

void MOS6502Listener::resolveBranchTargets()
{
    for (auto const &bt : branchTargets)
    {
        uint32_t branchOperandAddress = bt.first;
        TOptExprValue destAddress = bt.second->eval(symbolTable);

        if (destAddress != std::nullopt)
        {
            // relative address: destination address minus
            // address after branch statement (i.e. after operand address)
            auto offset = static_cast<int32_t>(destAddress.value() - (branchOperandAddress + 1));

            if (offset >= -128 && offset <= 127)
            {
                payload[branchOperandAddress] = static_cast<uint8_t>(offset & 0xffU);
            }
            else
            {
                addBranchTargetTooFarError(*bt.second, branchOperandAddress + 1, destAddress.value());
            }
        }
        else
        {
            addUnresolvedBranchTargetError(*bt.second);
        }
    }
}

void MOS6502Listener::resolveDeferredExpressions()
{
    for (auto const &defExprStmnt : deferredExpressionStatements)
    {
        TOptExprValue eval = defExprStmnt.expr->eval(this->symbolTable);
        if (eval != std::nullopt)
        {
            uint32_t operand = eval.value();
            if ((operand > 255) && (defExprStmnt.opNrBytes < 3))
            {
                addOperandTooLargeError(operand, defExprStmnt.srcLine, defExprStmnt.srcCol);
            }
            else
            {
                payload[defExprStmnt.address] = defExprStmnt.opCode;
                payload[defExprStmnt.address + 1] = static_cast<uint8_t>(operand & 0xffU);

                if (defExprStmnt.opNrBytes == 3)
                {
                    payload[defExprStmnt.address + 2] = static_cast<uint8_t>((operand >> 8U) & 0xffU);
                }
            }
        }
        else
        {
            addMissingSymbolError(defExprStmnt.expr->getText(), defExprStmnt.srcLine, defExprStmnt.srcCol);
        }
    }
}

auto MOS6502Listener::getAssembledMemBlocks() const -> MemBlocks
{
    return { codeLines, payload };
}

auto MOS6502Listener::popExpression() -> TOptExprValue
{
    TOptExprValue ret = std::nullopt;
    if (!expressionStack.empty())
    {
        ret = expressionStack.back()->eval(symbolTable);
        expressionStack.pop_back();
    }
    return ret;
}

auto MOS6502Listener::popNonEvalExpression() -> shared_ptr<IExpression>
{
    shared_ptr<IExpression> ret = nullptr;
    if (!expressionStack.empty())
    {
        ret = expressionStack.back();
        expressionStack.pop_back();
    }
    return ret;
}

auto MOS6502Listener::peekExpression() -> TOptExprValue
{
    TOptExprValue ret = std::nullopt;
    if (!expressionStack.empty())
    {
        ret = expressionStack.back()->eval(symbolTable);
    }
    return ret;
}

auto MOS6502Listener::popAllExpressions() -> vector<TOptExprValue>
{
    vector<TOptExprValue> ret;

    for (auto const &e : expressionStack)
    {
        ret.push_back(e->eval(symbolTable));
    }

    expressionStack.clear();

    return ret;
}

void MOS6502Listener::appendByteToPayload(uint8_t byte)
{
    if (addressOfLine == ADDR_INVALID)
    {
        addressOfLine = currentAddress;
    }

    payload[currentAddress++] = byte;
}

void MOS6502Listener::appendByteToPayload(optional<uint8_t> optByte)
{
    if (optByte != std::nullopt)
    {
        appendByteToPayload(optByte.value());
    }
}


void MOS6502Listener::addWordToPayload(uint16_t word)
{
    uint8_t lsb = word & 0xffU;
    uint8_t msb = static_cast<uint8_t>(word >> 8U) & 0xffU;

    // Little endian architecture
    appendByteToPayload(lsb);
    appendByteToPayload(msb);
}

void MOS6502Listener::addWordToPayload(optional<uint16_t> optWord)
{
    if (optWord != std::nullopt)
    {
        addWordToPayload(optWord.value());
    }
}

void MOS6502Listener::addDByteToPayload(uint16_t dbyte)
{
    uint8_t lsb = dbyte & 0xffU;
    uint8_t msb = static_cast<uint8_t>(dbyte >> 8U) & 0xffU;

    // write as two bytes, ignoring endianess
    appendByteToPayload(msb);
    appendByteToPayload(lsb);
}

void MOS6502Listener::addDByteToPayload(optional<uint16_t> optDbyte)
{
    if (optDbyte != std::nullopt)
    {
        addDByteToPayload(optDbyte.value());
    }
}

void MOS6502Listener::addMissingSymbolError(std::string const &symName, size_t line, size_t col)
{
    std::stringstream strm;
    strm << "Symbol or expression \"" << symName << "\" could not be resolved.";
    semanticErrors.emplace_back(SemanticError{strm.str(), fileName, line, col});
}

void MOS6502Listener::addUnresolvedBranchTargetError(IExpression const &branchTargetExpression)
{
    std::stringstream strm;
    strm << "Symbol or expression \"" << branchTargetExpression.getText() << "\" could not be resolved";
    semanticErrors.emplace_back(SemanticError{strm.str(), fileName, branchTargetExpression.getLine(), branchTargetExpression.getColumn()});
}

void MOS6502Listener::addBranchTargetTooFarError(IExpression const &branchTargetExpression, uint32_t branch, uint32_t target)
{
    std::stringstream strm;
    strm 
        << "Branch at address 0x" 
        << std::hex << std::setfill('0') << branch
        << " is too far away from the branch target \"" << branchTargetExpression.getText() << "\" at address 0x"
        << std::hex << std::setfill('0') << target << ".";
    
    semanticErrors.emplace_back(SemanticError{strm.str(), fileName, branchTargetExpression.getLine(), branchTargetExpression.getColumn()});
}

void MOS6502Listener::addDuplicateSymbolError(std::string const &symName, Sym const &duplicate, antlr4::ParserRuleContext const *ctx)
{
    std::stringstream strm;
    strm << "Redefinition of Symbol \"" << symName << "\" detected. " << std::endl
         << "See previous definition at " << fileName << ":" << duplicate.line << ":" << duplicate.col << std::endl;

    semanticErrors.emplace_back(SemanticError{strm.str(), fileName, line(ctx), col(ctx)});
}

void MOS6502Listener::addValueOutOfRangeError(uint32_t value, uint32_t min, uint32_t max, antlr4::ParserRuleContext const *ctx)
{
    std::stringstream strm;
    strm << "Value \"" << value << "\" is out of its supported value range: [" << min << "," << max << "]."<< std::endl;
    semanticErrors.emplace_back(SemanticError{strm.str(), fileName, line(ctx), col(ctx)});
}

void MOS6502Listener::addOperandTooLargeError(uint32_t operand, size_t line, size_t col)
{
    std::stringstream strm;
    strm 
        << "The operation requires a one byte operand operand, but its value 0x" 
        << std::hex << std::setw(4) << std::setfill('0') << operand << " does not fit into one byte."<< std::endl;
    semanticErrors.emplace_back(SemanticError{strm.str(), fileName, line, col});

}

// These errors should not happen. Likely cause by programming bug
void MOS6502Listener::addInternalError(size_t line, size_t col)
{
    semanticErrors.emplace_back(SemanticError{"Internal error.", fileName, line, col});
}

} /* namespace asm6502 */
