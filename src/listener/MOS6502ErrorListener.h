#ifndef MOS6502_ERROR_LISTENER_H
#define MOS6502_ERROR_LISTENER_H

#include <BaseErrorListener.h>
#include <iostream>

namespace asm6502
{

class MOS6502ErrorListener : public antlr4::BaseErrorListener
{
public:
    MOS6502ErrorListener(char const *pFileName) : BaseErrorListener(), fileName(pFileName) {}


    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                             size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override
    {
        std::cerr << getErrorMessage(line, charPositionInLine, msg);
    }

private:

    std::string getErrorMessage(size_t linenr, size_t colnr, std::string const &errmsg) const
    {
        std::stringstream strm;
        strm << fileName << ":" << linenr << ":" << colnr << ": error: " << errmsg << std::endl;

        return strm.str();
    }

    std::string fileName;
};

}


#endif