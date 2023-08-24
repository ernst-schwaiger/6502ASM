#include "MOS6502TestHelper.h"

using namespace antlr4;

namespace asm6502
{

ParseStatus parseStream(std::istream &stream, char const *fileName)
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

    return ParseStatus{listener.getAssembledMemBlocks(), listener.getSemanticErrors()};
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

// runs the passed prog thr
void testAssembly(std::istream &prog, MemBlocks const &ref)
{
    ParseStatus ps = parseStream(prog, "");

    if (!ps.semanticErrors.empty())
    {
        FAIL("Unexpected semantic errors were reported.");
    }

    if (ps.assembledProgram != ref)
    {
        std::stringstream strm;
        strm << "Built binary " << std::endl
            << getMemBlocksAsString(ps.assembledProgram)
            << "does not equal reference binary" << std::endl
            << getMemBlocksAsString(ref);

        FAIL(strm.str());
    }
}

} // namespace
