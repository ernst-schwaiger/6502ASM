/*
 * 6502ASM.cpp
 *
 *  Created on: 19.08.2018
 *      Author: Ernst
 */

#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <ANTLRInputStream.h>

#include "MOS6502Lexer.h"
#include "MOS6502Parser.h"
#include "listener/MOS6502Listener.h"
#include "listener/MOS6502ErrorListener.h"


using namespace std;
using namespace antlr4;

static int const RET_OK = 0;
static int const RET_ERR = -1;


int parseStreamOutputPayload(std::ifstream &stream, char const *fileName)
{
	ANTLRInputStream input(stream);
	MOS6502Lexer lexer(&input);
	CommonTokenStream tokens(&lexer);
	MOS6502Parser parser(&tokens);
	asm6502::MOS6502Listener listener(fileName);

	parser.addParseListener(&listener);

	parser.removeErrorListeners();
	asm6502::MOS6502ErrorListener errorListener(fileName);
	parser.addErrorListener(&errorListener);

	parser.r();
	listener.resolveDeferredExpressions();
	listener.resolveBranchTargets();

	bool semanticErrs = listener.detectedSemanticErrors();

	if (semanticErrs)
	{
		listener.outputSemanticErrors();
	}
	else
	{
		listener.outputPayload();
	}

	return semanticErrs ? RET_ERR : RET_OK;

}

int main(int argc, char *argv[])
{
	int ret = RET_OK;

	if (argc == 2)
	{
		std::ifstream stream;
		stream.open(argv[1]);

		if (!stream.fail())
		{
			try
			{
				ret = parseStreamOutputPayload(stream, argv[1]);
			}
			catch (logic_error const &e)
			{
				cerr << "Error: " << e.what();
				ret = RET_ERR;
			}
			catch (RecognitionException &e)
			{
				cerr << "Error: " << e.what();
				ret = RET_ERR;
			}
			catch (...)
			{
				cerr << "Unknown error occurred.";
				ret = RET_ERR;
			}
		}
		else
		{
			cerr << "Could not open file: " << argv[1] << endl;
			ret = RET_ERR;
		}
	}
	else
	{
		cerr << "Usage: " << argv[0] << " <asmfile>" << endl;
		ret = RET_ERR;
	}

	return ret;
}

