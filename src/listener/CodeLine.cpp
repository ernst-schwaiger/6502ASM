#include "CodeLine.h"

#include <iostream>
#include <iomanip>

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

    TerminalNodeVisitor() {};

    virtual std::any visit(antlr4::tree::ParseTree *tree) { return 0; }
    virtual std::any visitTerminal(antlr4::tree::TerminalNode *node) { return 0; }
    virtual std::any visitErrorNode(antlr4::tree::ErrorNode *node) { return 0; }

    virtual std::any visitChildren(antlr4::tree::ParseTree *node)
    {
        collectTerminalNodes(node);
        return 0;
    }

    void collectTerminalNodes(antlr4::tree::ParseTree *node)
    {
        if (node->children.empty())
        {
            terminalNodes.push_back(node);
        }
        else
        {
            for (antlr4::tree::ParseTree *child : node->children)
            {
                collectTerminalNodes(child);
            }
        }
    }

    std::vector<antlr4::tree::ParseTree *> terminalNodes;
};

std::string CodeLine::get(std::map<unsigned int, unsigned char> const &payload) const
{
    std::stringstream strm;
    int column = 0;

    std::string pl = getPayload(payload);

    strm << pl;
    column += pl.length();

    while (column < 24)
    {
        strm << " ";
        column++;
    }

    MOS6502Parser::LabelContext *labelCtx = ctx->label();

    if (labelCtx)
    {
        strm << labelCtx->getText();
        column += labelCtx->getText().length();
    }

    while (column < 36)
    {
        strm << " ";
        column++;
    }

    antlr4::RuleContext *dirOrStatementCtx = (ctx->directive() != nullptr) ? 
        static_cast<antlr4::RuleContext *>(ctx->directive()) : 
        static_cast<antlr4::RuleContext *>(ctx->statement());

    strm << prettyPrintDirOrStatement(dirOrStatementCtx) << std::endl;

    return strm.str();
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


    // bool skipWS = (((prevTerminalNode != nullptr) && (prevTerminalNode->getText() == std::string("."))) ||
    //             ((terminalNode->getText().size() >= 1) && (terminalNode->getText().substr(0,1) == std::string(","))));

    return skipWS ? "" : " ";
}


std::string CodeLine::getPayload(std::map<unsigned int, unsigned char> const &payload) const
{
    std::stringstream strm;

    if (lengthBytes > 0)
    {
        strm << "0x" << std::hex << std::setw(4) << std::setfill('0') << startAddress << ":";

        unsigned int lastByteAddr = startAddress + (lengthBytes - 1);

        for (unsigned int addr = startAddress; addr < lastByteAddr; addr++)
        {
            strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(payload.at(addr)) << ",";
        }

        strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(payload.at(lastByteAddr));
        strm << " ";
    }

    return strm.str();
}