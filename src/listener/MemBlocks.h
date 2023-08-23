#ifndef MEM_BLOCKS_H
#define MEM_BLOCKS_H

#include "CodeLine.h"
#include <vector>


namespace asm6502
{

class MemBlock
{
public:
	MemBlock(unsigned int startAddress, std::vector<unsigned char> const &bytes) :
		_startAddress(startAddress),
		_bytes(bytes)
	{}

	unsigned int getStartAddress() const { return _startAddress; }
	unsigned int getLengthBytes() const { return _bytes.size(); }
	unsigned char getByteAt(unsigned int idx) const { return _bytes.at(idx); }
private:
	unsigned int _startAddress;
	std::vector<unsigned char> _bytes;
};

// Calculates the list of contiguous Memory Blocks that contain the generated assembly code and the data sections
// Generates a Commodore BASIC program which initializes the MemBlocks with their assembly and data content
class MemBlocks
{
public:
	MemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<unsigned int, unsigned char> const &payload)
	{
		_memBlocks = getMemBlocks(codeLines, payload);
	}

	unsigned int getNumMemBlocks() const { return _memBlocks.size(); }

	MemBlock getMemBlockAt(unsigned int idx) const
	{
		return _memBlocks.at(idx);
	}

    std::string getBasicMemBlockInitializerListing() const;

private:

	std::vector<MemBlock> getMemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<unsigned int, unsigned char> const &payload);
	bool areAdjacent(asm6502::CodeLine const *prevCodeLine, asm6502::CodeLine const *currCodeLine) const;

	std::vector<MemBlock> _memBlocks;
};

} // namespace
#endif