#ifndef MOS_6502_HELPER_H
#define MOS_6502_HELPER_H

#include "catch.hpp"
#include <MOS6502Lexer.h>
#include <ANTLRInputStream.h>
#include <CommonTokenStream.h>

#include "listener/MOS6502Listener.h"
#include "listener/MOS6502ErrorListener.h"
#include "listener/MemBlocks.h"
#include "listener/SemanticError.h"

namespace asm6502
{
typedef struct
{
    MemBlocks assembledProgram;
    std::vector<SemanticError> semanticErrors;
} ParseStatus;

ParseStatus parseStream(std::istream &stream, char const *fileName);
std::string memBlockAsString(MemBlock const &mb);
std::string getMemBlocksAsString(MemBlocks const &mbs);

void testAssembly(std::istream &prog, MemBlocks const &ref);

}

#endif