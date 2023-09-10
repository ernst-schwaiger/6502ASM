#ifndef MOS_6502_HELPER_H
#define MOS_6502_HELPER_H

#include "ASM6502.h"

namespace asm6502
{

auto parseStream(std::istream &stream, char const *fileName) -> AssemblyStatus;
auto memBlockAsString(MemBlock const &mb) -> std::string;
auto getMemBlocksAsString(MemBlocks const &mbs) -> std::string;

auto testAssembly(std::istream &prog, MemBlocks const &ref) -> void;
auto testErrors(std::istream &prog, std::vector<size_t> expectedErrorLinesSorted = {}) -> void;
}

#endif