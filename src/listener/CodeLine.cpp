#include "CodeLine.h"

#include <iostream>
#include <iomanip>
#include "MemBlocks.h"

using namespace asm6502;

// private helper class to get all terminal nodes of a given production rule
class TerminalNodeVisitor : public antlr4::tree::ParseTreeVisitor
{
public:
    static std::vector<antlr4::tree::ParseTree *> getTerminalNodes(antlr4::RuleContext *ctx)
    {
        TerminalNodeVisitor visitor;
        ctx->accept(&visitor);
        return visitor.terminalNodes;
    }
private:

    TerminalNodeVisitor() {}; // shall only be created by the static method

    virtual std::any visit(antlr4::tree::ParseTree *tree) { return 0; }
    virtual std::any visitTerminal(antlr4::tree::TerminalNode *node) { return 0; }
    virtual std::any visitErrorNode(antlr4::tree::ErrorNode *node) { return 0; }

    virtual std::any visitChildren(antlr4::tree::ParseTree *node)
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

std::string CodeLine::get(asm6502::MemBlocks const &mb, bool addAssembly) const
{
    std::stringstream strm;

    std::string pl = getMachineCode(mb);
    strm << pl;

    if (addAssembly)
    {
        int column = pl.length();
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


std::string CodeLine::getMachineCode(MemBlocks const &mb) const
{
    std::stringstream strm;

    if (lengthBytes > 0)
    {
        strm << "0x" << std::hex << std::setw(4) << std::setfill('0') << startAddress << ":";

        unsigned int lastByteAddr = startAddress + (lengthBytes - 1);

        for (unsigned int addr = startAddress; addr < lastByteAddr; addr++)
        {
            strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(mb.getByteAt(addr)) << ",";
        }

        strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(mb.getByteAt(lastByteAddr));
        strm << " ";
    }

    return strm.str();
}

std::string CodeLine::getLabel(MOS6502Parser::LineContext *ctx)
{
    MOS6502Parser::LabelContext *labelCtx = ctx->label();
    return labelCtx != nullptr ? labelCtx->getText() : "";
}

std::string CodeLine::getAssembly(MOS6502Parser::LineContext *ctx)
{
    antlr4::RuleContext *dirOrStatementCtx = (ctx->directive() != nullptr) ? 
        static_cast<antlr4::RuleContext *>(ctx->directive()) : 
        static_cast<antlr4::RuleContext *>(ctx->statement());

    return prettyPrintDirOrStatement(dirOrStatementCtx);
}

std::string CodeLine::prettyPrintDirOrStatement(antlr4::RuleContext *dirOrStatementCtx) const
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

std::string CodeLine::getWhitespaceBetweenTokens(antlr4::tree::ParseTree *terminalNode, antlr4::tree::ParseTree *prevTerminalNode) const
{
    std::string sTermNode = terminalNode->getText();
    std::string sPrevNode = (prevTerminalNode != nullptr) ?  prevTerminalNode->getText() : "";
    char const *strTermNode = sTermNode.c_str();
    char const *strPrevNode = sPrevNode.c_str();

    bool skipWS = ((prevTerminalNode == nullptr) ||
                   (prevTerminalNode->getText() == ".") ||
                   (prevTerminalNode->getText() == "#") ||
                   ((terminalNode->getText().size() >= 1) && (terminalNode->getText().substr(0,1) == std::string(",")))
                  );

    return skipWS ? "" : " ";
}