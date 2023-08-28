#ifndef ASM6502_H
#define ASM6502_H

#include <vector>
#include <iostream>
#include "listener/MemBlocks.h"

namespace asm6502
{
    typedef struct
    {
        std::vector<std::string> errors;
        MemBlocks assembledProgram;
    } AssemblyStatus;
    

    // API for clients passing an assemble file
    AssemblyStatus assembleFile(char const *fileName);
    // API for tests
    void assembleStream(std::istream &stream, char const *fileName, AssemblyStatus &ret);
    void writeProgFile(char const *pProgFilePath, MemBlocks const &memBlocks);
}

#endif