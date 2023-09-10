#include <iomanip>
#include <iostream>

#include "CodeLine.h"
#include "MemBlocks.h"

using namespace asm6502;

// private helper class to get all terminal nodes of a given production rule
class TerminalNodeVisitor : public antlr4::tree::ParseTreeVisitor
{
public:
    static auto getTerminalNodes(antlr4::RuleContext *ctx) -> std::vector<antlr4::tree::ParseTree *>
    {
        TerminalNodeVisitor visitor;
        // ctx may be invalid if ANTLR detected a parse error
        if (ctx != nullptr)
        {
            ctx->accept(&visitor);
        }
        return visitor.terminalNodes;
    }
private:

    TerminalNodeVisitor() = default; // shall only be created by the static method

    auto visit(antlr4::tree::ParseTree *tree) -> std::any override { return 0; }
    auto visitTerminal(antlr4::tree::TerminalNode *node) -> std::any override { return 0; }
    auto visitErrorNode(antlr4::tree::ErrorNode *node) -> std::any override { return 0; }

    auto visitChildren(antlr4::tree::ParseTree *node) -> std::any override
    {
        if (node->children.empty())
        {
            terminalNodes.push_back(node);
        }
        else
        {
            for (antlr4::tree::ParseTree *child : node->children)
            {
                visitChildren(child);
            }
        }

        return 0;
    }

    std::vector<antlr4::tree::ParseTree *> terminalNodes;
};

auto CodeLine::get(asm6502::MemBlocks const &mb, bool addAssembly) const -> std::string
{
    std::stringstream strm;

    std::string pl = getMachineCode(mb);
    strm << pl;

    if (addAssembly)
    {
        auto column = pl.length();
        while (column < 24)
        {
            strm << " ";
            column++;
        }

        strm << label;
        column += label.length();

        while (column < 36)
        {
            strm << " ";
            column++;
        }

        strm << assembly;
    }

    strm << std::endl;
    return strm.str();
}


auto CodeLine::getMachineCode(MemBlocks const &mb) const -> std::string
{
    std::stringstream strm;

    if (lengthBytes > 0)
    {
        strm << "0x" << std::hex << std::setw(4) << std::setfill('0') << startAddress << ":";

        uint32_t lastByteAddr = startAddress + (lengthBytes - 1);

        for (uint32_t addr = startAddress; addr < lastByteAddr; addr++)
        {
            strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(mb.getByteAt(addr)) << ",";
        }

        strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(mb.getByteAt(lastByteAddr));
        strm << " ";
    }

    return strm.str();
}

auto CodeLine::getLabel(MOS6502Parser::LineContext *ctx) -> std::string
{
    MOS6502Parser::LabelContext *labelCtx = ctx->label();
    return labelCtx != nullptr ? labelCtx->getText() : "";
}

auto CodeLine::getAssembly(MOS6502Parser::LineContext *ctx) -> std::string
{
    antlr4::RuleContext *dirOrStatementCtx = (ctx->directive() != nullptr) ? 
        static_cast<antlr4::RuleContext *>(ctx->directive()) : 
        static_cast<antlr4::RuleContext *>(ctx->statement());

    return prettyPrintDirOrStatement(dirOrStatementCtx);
}

auto CodeLine::prettyPrintDirOrStatement(antlr4::RuleContext *dirOrStatementCtx) -> std::string
{
    std::stringstream strm;
    std::vector<antlr4::tree::ParseTree *> terminalNodes = TerminalNodeVisitor::getTerminalNodes(dirOrStatementCtx);

    antlr4::tree::ParseTree *prevTerminalNode = nullptr;

    for (antlr4::tree::ParseTree *terminalNode : terminalNodes)
    {
        strm << getWhitespaceBetweenTokens(terminalNode, prevTerminalNode) << terminalNode->getText();
        prevTerminalNode = terminalNode;
    }
    return strm.str();
}

auto CodeLine::getWhitespaceBetweenTokens(antlr4::tree::ParseTree *terminalNode, antlr4::tree::ParseTree *prevTerminalNode) -> std::string
{
    bool skipWS = ((prevTerminalNode == nullptr) ||
                   (prevTerminalNode->getText() == ".") ||
                   (prevTerminalNode->getText() == "#") ||
                   ((!terminalNode->getText().empty()) && (terminalNode->getText().substr(0,1) == std::string(",")))
                  );

    return skipWS ? "" : " ";
}