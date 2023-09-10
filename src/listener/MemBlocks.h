#ifndef MEM_BLOCKS_H
#define MEM_BLOCKS_H

#include <vector>
#include <iostream>

#include "CodeLine.h"

namespace asm6502
{

class MemBlock
{
public:
    MemBlock(uint32_t startAddress_, std::vector<uint8_t> const &bytes_) :
        startAddress(startAddress_),
        bytes(bytes_)
    {}

    auto operator == (MemBlock const &rhs) const -> bool
    {
        return ((startAddress == rhs.startAddress) && (bytes == rhs.bytes));
    }

    auto operator < (MemBlock const &rhs) const -> bool
    {
        return (startAddress < rhs.startAddress);
    }

    auto operator != (MemBlock const &rhs) const -> bool
    {
        return !(*this == rhs);
    }

    auto getStartAddress() const -> uint32_t { return startAddress; }
    auto getLengthBytes() const -> uint32_t { return bytes.size(); }
    auto getByteAt(uint32_t idx) const -> uint8_t { return bytes.at(idx); }

    friend auto operator << (std::ostream &os, asm6502::MemBlock const &memBlock) -> std::ostream &;

private:
    uint32_t startAddress;
    std::vector<uint8_t> bytes;
};

// Calculates the list of contiguous Memory Blocks that contain the generated assembly code and the data sections
// Generates a Commodore BASIC program which initializes the MemBlocks with their assembly and data content
class MemBlocks
{
public:
    MemBlocks() {}
    MemBlocks(std::vector<asm6502::CodeLine> const &codeLines_, std::map<uint32_t, uint8_t> const &payload) :
        codeLines { codeLines_ },
        memBlocks { getMemBlocks(codeLines_, payload) }
    {}

    MemBlocks(std::vector<asm6502::MemBlock> const &memBlocks_) : memBlocks(memBlocks_) {}

    auto operator == (MemBlocks const &rhs) const -> bool
    {
        return (memBlocks == rhs.memBlocks);
    }

    auto operator != (MemBlocks const &rhs) const -> bool
    {
        return !(memBlocks == rhs.memBlocks);
    }

    auto getNumMemBlocks() const -> uint32_t { return memBlocks.size(); }

    auto getMemBlockAt(uint32_t idx) const -> MemBlock
    {
        return memBlocks.at(idx);
    }

    auto getMachineCode(bool includeAssembly) const -> std::string;
    auto getBasicMemBlockInitializerListing() const -> std::string;
    auto getByteAt(uint32_t address) const -> uint8_t;

    friend auto operator<<(std::ostream& os, asm6502::MemBlocks const &memBlocks) -> std::ostream&;

private:

    static auto getMemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<uint32_t, uint8_t> const &payload) -> std::vector<MemBlock>;
    static auto areAdjacent(asm6502::CodeLine const *prevCodeLine, asm6502::CodeLine const *currCodeLine) -> bool;

    std::vector<MemBlock> memBlocks;
    std::vector<asm6502::CodeLine> codeLines;
};

auto operator << (std::ostream &os, asm6502::MemBlocks const &memBlocks) -> std::ostream &;
auto operator << (std::ostream &os, asm6502::MemBlock const &memBlock) -> std::ostream &;

} // namespace
#endif