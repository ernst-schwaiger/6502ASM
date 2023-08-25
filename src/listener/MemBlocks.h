#ifndef MEM_BLOCKS_H
#define MEM_BLOCKS_H

#include "CodeLine.h"
#include <vector>


namespace asm6502
{

class MemBlock
{
public:
    MemBlock(unsigned int startAddress_, std::vector<unsigned char> const &bytes_) :
        startAddress(startAddress_),
        bytes(bytes_)
    {}

    bool operator == (MemBlock const &rhs) const
    {
        return ((startAddress == rhs.startAddress) && (bytes == rhs.bytes));
    }

    bool operator != (MemBlock const &rhs) const
    {
        return !(*this == rhs);
    }

    unsigned int getStartAddress() const { return startAddress; }
    unsigned int getLengthBytes() const { return bytes.size(); }
    unsigned char getByteAt(unsigned int idx) const { return bytes.at(idx); }
private:
    unsigned int startAddress;
    std::vector<unsigned char> bytes;
};

// Calculates the list of contiguous Memory Blocks that contain the generated assembly code and the data sections
// Generates a Commodore BASIC program which initializes the MemBlocks with their assembly and data content
class MemBlocks
{
public:
    MemBlocks() {}
    MemBlocks(std::vector<asm6502::CodeLine> const &codeLines_, std::map<unsigned int, unsigned char> const &payload) :
        codeLines { codeLines_ },
        memBlocks { getMemBlocks(codeLines_, payload) }
    {}

    MemBlocks(std::vector<asm6502::MemBlock> const &memBlocks_) : memBlocks(memBlocks_) {}

    bool operator == (MemBlocks const &rhs) const
    {
        return (memBlocks == rhs.memBlocks);
    }

    bool operator != (MemBlocks const &rhs) const
    {
        return !(memBlocks == rhs.memBlocks);
    }

    unsigned int getNumMemBlocks() const { return memBlocks.size(); }

    MemBlock getMemBlockAt(unsigned int idx) const
    {
        return memBlocks.at(idx);
    }

    std::string getMachineCode(bool includeAssembly) const;
    std::string getBasicMemBlockInitializerListing() const;
    unsigned char getByteAt(unsigned int address) const;
private:

    std::vector<MemBlock> getMemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<unsigned int, unsigned char> const &payload);
    bool areAdjacent(asm6502::CodeLine const *prevCodeLine, asm6502::CodeLine const *currCodeLine) const;

    std::vector<MemBlock> memBlocks;
    std::vector<asm6502::CodeLine> codeLines;
};

} // namespace
#endif