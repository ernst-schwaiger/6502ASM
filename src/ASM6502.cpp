
#include <iostream>
#include <string>

#include <ANTLRInputStream.h>
#include <MOS6502Lexer.h>
#include <MOS6502Parser.h>

#include "ASM6502.h"
#include "listener/MOS6502ErrorListener.h"
#include "listener/MOS6502Listener.h"

using namespace std;
using namespace antlr4;

namespace asm6502
{

void assembleStream(std::istream &stream, char const *fileName, AssemblyStatus &ret)
{
    ANTLRInputStream input(stream);
    MOS6502Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    MOS6502Parser parser(&tokens);
    asm6502::MOS6502Listener listener(fileName);

    parser.addParseListener(&listener);

    parser.removeErrorListeners();
    asm6502::MOS6502ErrorListener errorListener(fileName, &listener);
    parser.addErrorListener(&errorListener);

    parser.r();
    listener.resolveDeferredExpressions();
    listener.resolveBranchTargets();

    bool errorsDetected = listener.detectedErrors();

    if (errorsDetected)
    {
        for (auto const &pe : listener.getParseErrors())
        {
            ret.errors.push_back(pe);
        }
        for (auto const &se : listener.getSemanticErrors())
        {
            ret.errors.push_back(se.getErrorMessage());
        }
    }
    else
    {
        ret.assembledProgram = listener.getAssembledMemBlocks();
    }
}

auto assembleFile(char const *fileName) -> AssemblyStatus
{
    AssemblyStatus ret;

    std::ifstream stream;
    stream.open(fileName);

    if (!stream.fail())
    {
        try
        {
            assembleStream(stream, fileName, ret);
        }
        catch (logic_error const &e)
        {
            std::stringstream strm;
            strm << "Error: " << e.what();
            ret.errors.push_back(strm.str());
        }
        catch (RecognitionException &e)
        {
            std::stringstream strm;
            strm << "Error: " << e.what();
            ret.errors.push_back(strm.str());
        }
        catch (...)
        {
            std::stringstream strm;
            strm << "Unknown error occurred." << std::endl;
            ret.errors.push_back(strm.str());
        }
    }
    else
    {
        std::stringstream strm;
        strm << "Could not open file: " << fileName << endl;
        ret.errors.push_back(strm.str());        
    }

    return ret;
}

void writeProgFile(char const *pProgFilePath, MemBlocks const &memBlocks)
{
    std::ofstream progFile(pProgFilePath, std::ios::binary | std::ios::out | std::ios::trunc);
    progFile << memBlocks;
    progFile.close();
}

}
