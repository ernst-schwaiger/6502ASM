#include "MOS6502TestHelper.h"
#include <iomanip>
#include <iostream>
#include <algorithm>

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

static size_t getLineNumberFromErrorMessage(std::string const &errMsg)
{
    size_t lineNr = 0;
    auto posFirstColon = errMsg.find(":");
    auto posSecondColon = errMsg.find(":", posFirstColon + 1);

    if (posFirstColon < posSecondColon)
    {
        std::string lineNrStr = errMsg.substr(posFirstColon + 1, posSecondColon - (posFirstColon + 1));
        std::stringstream ss;
        ss << lineNrStr;
        ss >> lineNr;
    }

    return lineNr;
}

static std::string getErrorLineNumbersAsString(std::vector<size_t> errLineNrs)
{
    std::stringstream strm;
    strm << "{";

    if (!errLineNrs.empty())
    {
        for (size_t i = 0; i < errLineNrs.size() - 1; i++)
        {
            strm << errLineNrs.at(i) << ", ";
        }
        
        strm << errLineNrs.at(errLineNrs.size() - 1);
    }

    strm << "}";

    return strm.str();
}

static std::vector<size_t> getErrorMsgLineNumbersSorted(std::vector<std::string> errMsgs)
{
    std::vector<size_t> ret;

    for (auto const &errMsg : errMsgs)
    {
        ret.push_back(getLineNumberFromErrorMessage(errMsg));
    }

    std::sort(begin(ret), end(ret));

    return ret; 
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

void testErrors(std::istream &prog, std::vector<size_t> expectedErrorLinesSorted)
{
    AssemblyStatus as = parseStream(prog, "");

    if (as.errors.empty())
    {
        FAIL("No errors were were detected");
    }

    if (as.assembledProgram.getNumMemBlocks() > 0)
    {
        FAIL("Unexpected assembly was generated while errors were detected");
    }

    if (!expectedErrorLinesSorted.empty())
    {
        std::vector<size_t> errMsgLinesSorted = getErrorMsgLineNumbersSorted(as.errors);

        if (errMsgLinesSorted != expectedErrorLinesSorted)
        {
            std::stringstream strm;
            strm
                << "Expected error message lines:" << std::endl
                << getErrorLineNumbersAsString(expectedErrorLinesSorted) << std::endl
                << "do not equal the actual detected error lines:"  << std::endl
                << getErrorLineNumbersAsString(errMsgLinesSorted) << std::endl;
            FAIL(strm.str());
        }
    }
}





} // namespace
