#ifndef ASM6502_H
#define ASM6502_H

#include <vector>
#include "listener/MemBlocks.h"

namespace asm6502
{
    typedef struct
    {
        std::vector<std::string> errors;
        MemBlocks assembledProgram;
    } AssemblyStatus;
    

    AssemblyStatus assembleFile(char const *fileName);

}

#endif