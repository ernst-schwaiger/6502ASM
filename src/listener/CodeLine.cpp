#include "CodeLine.h"
#include <iostream>
#include <iomanip>

using namespace asm6502;

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
    MOS6502Parser::DirectiveContext *dirCtx = ctx->directive();
    MOS6502Parser::StatementContext *stmntCtx = ctx->statement();

    strm << (dirCtx ? dirCtx->getText() : stmntCtx->getText()) << std::endl;

    return strm.str();
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