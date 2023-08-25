#ifndef MOS_6502_HELPER_H
#define MOS_6502_HELPER_H

#include "ASM6502.h"

namespace asm6502
{

AssemblyStatus parseStream(std::istream &stream, char const *fileName);
std::string memBlockAsString(MemBlock const &mb);
std::string getMemBlocksAsString(MemBlocks const &mbs);

void testAssembly(std::istream &prog, MemBlocks const &ref);
void testErrors(std::istream &prog, std::vector<size_t> expectedErrorLinesSorted = {});
}

#endif