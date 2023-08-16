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

using namespace std;
using namespace antlr4;

static int const RET_OK = 0;
static int const RET_ERR = -1;


void parseStreamOutputPayload(std::ifstream &stream)
{
	ANTLRInputStream input(stream);
	MOS6502Lexer lexer(&input);
	CommonTokenStream tokens(&lexer);
	MOS6502Parser parser(&tokens);
	asm6502::MOS6502Listener listener;

	parser.addParseListener(&listener);

	parser.r();
	listener.resolveBranchTargets();
	listener.outputPayload();
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
				parseStreamOutputPayload(stream);
			}
			catch (logic_error const &e)
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

