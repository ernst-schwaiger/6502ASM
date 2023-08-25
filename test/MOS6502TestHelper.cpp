#include "MOS6502TestHelper.h"
#include <iomanip>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

using namespace antlr4;

namespace asm6502
{

AssemblyStatus parseStream(std::istream &stream, char const *fileName)
{
    AssemblyStatus ret;
    assembleStream(stream, fileName, ret);
    return ret;
}

std::string memBlockAsString(MemBlock const &mb)
{
    std::stringstream strm; 
    strm 
        << "{0x" << std::hex << std::setw(4) << std::setfill('0')
        << mb.getStartAddress() << ", { "
        << std::setw(2);

    for (size_t i = 0; i < mb.getLengthBytes(); i++)
    {
        strm << "0x" << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<unsigned int>(mb.getByteAt(i));
        if (i < mb.getLengthBytes() - 1)
        {
            strm << ", ";
        }
    }
    strm << "}}" << std::endl;
    return strm.str();
}

std::string getMemBlocksAsString(MemBlocks const &mbs)
{
    std::stringstream strm; 
    for (size_t i = 0; i < mbs.getNumMemBlocks(); i++)
    {
        strm << memBlockAsString(mbs.getMemBlockAt(i));
    }

    return strm.str();
}

// runs the passed prog through assembler
void testAssembly(std::istream &prog, MemBlocks const &ref)
{
    AssemblyStatus as = parseStream(prog, "");

    if (!as.errors.empty())
    {
        FAIL("Unexpected semantic errors were reported.");
    }

    if (as.assembledProgram != ref)
    {
        std::stringstream strm;
        strm << "Built binary " << std::endl
            << getMemBlocksAsString(as.assembledProgram)
            << "does not equal reference binary" << std::endl
            << getMemBlocksAsString(ref);

        FAIL(strm.str());
    }
}

} // namespace
