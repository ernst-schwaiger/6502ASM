#ifndef MOS6502_ERROR_LISTENER_H
#define MOS6502_ERROR_LISTENER_H

#include <BaseErrorListener.h>
#include "MOS6502Listener.h"

namespace asm6502
{

class MOS6502Listener;

class MOS6502ErrorListener : public antlr4::BaseErrorListener
{
public:
    MOS6502ErrorListener(char const *pFileName, MOS6502Listener *pListener_) : 
        BaseErrorListener(), 
        fileName(pFileName),
        pListener(pListener_)
    {}

    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                             size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override
    {
        pListener->addParseError(getErrorMessage(line, charPositionInLine, msg));
    }

private:

    std::string getErrorMessage(size_t linenr, size_t colnr, std::string const &errmsg) const
    {
        std::stringstream strm;
        strm << fileName << ":" << linenr << ":" << colnr << ": error: " << errmsg << std::endl;
        return strm.str();
    }

    std::string fileName;
    MOS6502Listener *pListener;
};

}


#endif