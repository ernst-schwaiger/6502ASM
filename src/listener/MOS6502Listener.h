/*
 * MOS6502Listener.h
 *
 *  Created on: 19.08.2018
 *      Author: Ernst
 */

#ifndef MOS6502LISTENER_H_
#define MOS6502LISTENER_H_

#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <iomanip>
#include <string>
#include <optional>

#include "MOS6502BaseListener.h"
#include "SymbolTable.h"
#include "CodeLine.h"
#include "SemanticError.h"
#include "MemBlocks.h"

namespace asm6502
{

typedef std::optional<uint32_t> TOptExprValue;

constexpr uint32_t ADDR_INVALID = 0xffffffffU;

class IExpression
{
public:
    virtual ~IExpression(){};
    virtual auto eval(SymbolTable const &symbolTable) const -> TOptExprValue = 0;
    virtual auto getText() const -> std::string = 0;
    [[nodiscard]] virtual auto getLine() const -> size_t = 0;
    [[nodiscard]] virtual auto getColumn() const -> size_t = 0;
};

// Implements a deferred expression evaluation for commands that use
// absolute, indirect, indexed commands where the base address may be defined
// after the statement, i.e. is not yet known
class DeferredExpressionEval
{
public:
    DeferredExpressionEval(uint8_t opCode_, uint8_t opNrBytes_, std::shared_ptr<IExpression> expr_, uint32_t address_, size_t srcLine_, size_t srcCol_) :
        expr{expr_},
        srcLine{srcLine_},
        srcCol{srcCol_},
        address{address_},
        opCode{opCode_},
        opNrBytes{opNrBytes_}
    {}

    std::shared_ptr<IExpression> expr;
    size_t srcLine;
    size_t srcCol;
    uint32_t address;
    uint8_t opCode; 
    uint8_t opNrBytes;
};

class MOS6502Listener : public MOS6502BaseListener
{
public:
    MOS6502Listener(char const *pFileName);
    virtual ~MOS6502Listener() = default;

    void exitOrg_directive(MOS6502Parser::Org_directiveContext * /*ctx*/) override;
    void exitByte_directive(MOS6502Parser::Byte_directiveContext * /*ctx*/) override;
    void exitWord_directive(MOS6502Parser::Word_directiveContext * /*ctx*/) override;
    void exitDbyte_directive(MOS6502Parser::Dbyte_directiveContext * /*ctx*/) override;

    void exitLabel(MOS6502Parser::LabelContext * /*ctx*/) override;
    void exitAss_directive(MOS6502Parser::Ass_directiveContext * /*ctx*/) override;

    void exitDir_statement(MOS6502Parser::Dir_statementContext * /*ctx*/) override;
    void exitImm_statement(MOS6502Parser::Imm_statementContext * /*ctx*/) override;
    void exitRel_statement(MOS6502Parser::Rel_statementContext * /*ctx*/) override;
    void exitIdx_x_statement(MOS6502Parser::Idx_x_statementContext * /*ctx*/) override;
    void exitIdx_y_statement(MOS6502Parser::Idx_y_statementContext * /*ctx*/) override;
    void exitIdx_abs_statement(MOS6502Parser::Idx_abs_statementContext * /*ctx*/) override;
    void exitIdr_statement(MOS6502Parser::Idr_statementContext * /*ctx*/) override;
    void exitIdx_idr_statement(MOS6502Parser::Idx_idr_statementContext * /*ctx*/) override;
    void exitIdr_idx_statement(MOS6502Parser::Idr_idx_statementContext * /*ctx*/) override;


    void exitExpression(MOS6502Parser::ExpressionContext * ctx) override;
    void exitSymbol(MOS6502Parser::SymbolContext * /*ctx*/) override;
    void exitDec8(MOS6502Parser::Dec8Context * ctx) override;
    void exitDec(MOS6502Parser::DecContext * ctx) override;
    void exitHex16(MOS6502Parser::Hex16Context * ctx) override;
    void exitHex8(MOS6502Parser::Hex8Context * ctx) override;
    void exitBin8(MOS6502Parser::Bin8Context * ctx) override;
    void exitChar8(MOS6502Parser::Char8Context * ctx) override;
    void exitData_string(MOS6502Parser::Data_stringContext * /*ctx*/) override;

    void exitLine(MOS6502Parser::LineContext * /*ctx*/) override;

    void resolveBranchTargets();
    void resolveDeferredExpressions();
    bool detectedErrors() const { return ((semanticErrors.size() + parseErrors.size()) > 0); }

    MemBlocks getAssembledMemBlocks() const;
    std::vector<asm6502::SemanticError> getSemanticErrors() const { return semanticErrors; }
    std::vector<std::string> getParseErrors() const { return parseErrors; }

    void addParseError(std::string const &errorMsg) {parseErrors.push_back(errorMsg); }

private:

    static uint32_t convertDec(std::string const &dec);
    static uint32_t convertHex(std::string const &hex);
    static uint32_t convertBin(std::string const &bin);

    TOptExprValue popExpression();
    TOptExprValue peekExpression();
    std::shared_ptr<IExpression> popNonEvalExpression();

    std::vector<TOptExprValue> popAllExpressions();

    void appendIdxOrZpgCmd(uint8_t opcode, uint8_t opcode_zpg, antlr4::ParserRuleContext const *ctx);
    void appendIdxIdrOrIdrIdxOrImmCmd(uint8_t opcode, antlr4::ParserRuleContext const *ctx);

    void appendByteToPayload(uint8_t byte);
    void appendByteToPayload(std::optional<uint8_t> optByte);
    void addWordToPayload(uint16_t word);
    void addWordToPayload(std::optional<uint16_t> optWord);
    void addDByteToPayload(uint16_t dbyte);
    void addDByteToPayload(std::optional<uint16_t> optDbyte);

    void addSymbolCheckAlreadyDefined(std::string const &symName, uint32_t symVal, antlr4::ParserRuleContext *ctx);
    void addMissingSymbolError(std::string const &symName, size_t line, size_t col);
    void addUnresolvedBranchTargetError(IExpression const &branchTargetExpression); // for failed branch target resolution
    void addBranchTargetTooFarError(IExpression const &branchTargetExpression, uint32_t branch, uint32_t target); // if branch and target are too far away, out of byte offset [-128 .. 127]
    void addDuplicateSymbolError(std::string const &symName, Sym const &duplicate, antlr4::ParserRuleContext const *ctx);
    void addValueOutOfRangeError(uint32_t value, uint32_t min, uint32_t max, antlr4::ParserRuleContext const *ctx);
    void addOperandTooLargeError(uint32_t operand, size_t line, size_t col);
    void addInternalError(size_t line, size_t col);

    size_t line(antlr4::ParserRuleContext const *ctx) { return ctx->getStart()->getLine(); }
    size_t col(antlr4::ParserRuleContext const *ctx) { return ctx->getStart()->getCharPositionInLine(); }

    void makeDeferredExpression(uint8_t opcode, uint8_t opNrBytes, std::shared_ptr<IExpression> pExpression, uint32_t currentAddress, size_t line, size_t col );


    std::string fileName;
    uint32_t currentAddress;
    uint32_t addressOfLine;
    std::vector<std::pair<uint32_t, std::shared_ptr<IExpression const>>> branchTargets; // branch tgt addresses to labels
    std::vector<DeferredExpressionEval> deferredExpressionStatements;
    SymbolTable symbolTable;
    std::vector<std::shared_ptr<IExpression>> expressionStack; // expression stack for one code line, reset after each code line
    std::map<uint32_t, uint8_t> payload;
    std::vector<CodeLine> codeLines;
    std::vector<asm6502::SemanticError> semanticErrors;
    std::vector<std::string> parseErrors;
};

} /* namespace asm6502 */

#endif /* MOS6502LISTENER_H_ */
