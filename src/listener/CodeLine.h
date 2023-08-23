#ifndef CODE_LINE_H
#define CODE_LINE_H

#include <MOS6502Parser.h>

namespace asm6502
{
// Models a complete Line statement, a start address and a byte length
// to associate assembler code and the bytes that have been generated
// for it
class CodeLine
{
public:
	CodeLine(MOS6502Parser::LineContext *_ctx, unsigned int _startAddress, unsigned int _lengthBytes) :
		ctx {_ctx},
		startAddress {_startAddress },
		lengthBytes {_lengthBytes}
		{};

	std::string get(std::map<unsigned int, unsigned char> const &payload) const;
	std::string getPayload(std::map<unsigned int, unsigned char> const &payload) const;

	unsigned int getStartAddress() const
	{
		return startAddress;
	}

	unsigned int getLengthBytes() const
	{
		return lengthBytes;
	}

private:
	MOS6502Parser::LineContext *ctx;
	unsigned int const startAddress;
	unsigned int const lengthBytes;
};

}

#endif