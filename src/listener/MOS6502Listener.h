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

typedef std::optional<unsigned int> TOptExprValue;

class IExpression
{
public:
    virtual ~IExpression(){};
    virtual TOptExprValue eval(SymbolTable const &symbolTable) const = 0;
    virtual std::string getText() const = 0;
    virtual size_t getLine() const = 0;
    virtual size_t getColumn() const = 0;
};

// Implements a deferred expression evaluation for commands that use
// absolute, indirect, indexed commands where the base address may be defined
// after the statement, i.e. is not yet known
class DeferredExpressionEval
{
public:
    DeferredExpressionEval(unsigned char opCode_, uint8_t opNrBytes_, std::shared_ptr<IExpression> expr_, unsigned int address_, size_t srcLine_, size_t srcCol_) :
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
    unsigned int address;
    unsigned char opCode; 
    unsigned char opNrBytes;
};

class MOS6502Listener : public MOS6502BaseListener
{
public:
    MOS6502Listener(char const *pFileName);
    virtual ~MOS6502Listener();

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

    unsigned int convertDec(std::string const &dec) const;
    unsigned int convertHex(std::string const &hex) const;
    unsigned int convertBin(std::string const &bin) const;

    TOptExprValue popExpression();
    TOptExprValue peekExpression();
    std::shared_ptr<IExpression> popNonEvalExpression();

    std::vector<TOptExprValue> popAllExpressions();

    void appendIdxOrZpgCmd(unsigned char opcode, unsigned char opcode_zpg, antlr4::ParserRuleContext const *ctx);
    void appendIdxIdrOrIdrIdxOrImmCmd(unsigned char opcode, antlr4::ParserRuleContext const *ctx);

    void appendByteToPayload(unsigned char byte);
    void appendByteToPayload(std::optional<unsigned char> optByte);
    void addWordToPayload(unsigned short word);
    void addWordToPayload(std::optional<unsigned short> optWord);
    void addDByteToPayload(unsigned short dbyte);
    void addDByteToPayload(std::optional<unsigned short> optDbyte);

    void addSymbolCheckAlreadyDefined(std::string symName, unsigned int symVal, antlr4::ParserRuleContext *ctx);
    void addMissingSymbolError(std::string const &symName, size_t line, size_t col);
    void addUnresolvedBranchTargetError(IExpression const &branchTargetExpression); // for failed branch target resolution
    void addBranchTargetTooFarError(IExpression const &branchTargetExpression, unsigned int branch, unsigned int target); // if branch and target are too far away, out of byte offset [-128 .. 127]
    void addDuplicateSymbolError(std::string const &symName, Sym const &duplicate, antlr4::ParserRuleContext const *ctx);
    void addValueOutOfRangeError(unsigned int value, unsigned int min, unsigned int max, antlr4::ParserRuleContext const *ctx);
    void addOperandTooLargeError(unsigned int operand, size_t line, size_t col);
    void addInternalError(size_t line, size_t col);

    size_t line(antlr4::ParserRuleContext const *ctx) { return ctx->getStart()->getLine(); }
    size_t col(antlr4::ParserRuleContext const *ctx) { return ctx->getStart()->getCharPositionInLine(); }

    void makeDeferredExpression(unsigned char opcode, uint8_t opNrBytes, std::shared_ptr<IExpression> pExpression, unsigned int currentAddress, size_t line, size_t col );


    std::string fileName;
    unsigned int currentAddress;
    int addressOfLine;
    std::vector<std::pair<unsigned int, std::shared_ptr<IExpression const>>> branchTargets; // branch tgt addresses to labels
    std::vector<DeferredExpressionEval> deferredExpressionStatements;
    SymbolTable symbolTable;
    std::vector<std::shared_ptr<IExpression>> expressionStack; // expression stack for one code line, reset after each code line
    std::map<unsigned int, unsigned char> payload;
    std::vector<CodeLine> codeLines;
    std::vector<asm6502::SemanticError> semanticErrors;
    std::vector<std::string> parseErrors;
};

} /* namespace asm6502 */

#endif /* MOS6502LISTENER_H_ */
