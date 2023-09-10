#include <iomanip>

#include "MemBlocks.h"

using namespace asm6502;

auto MemBlocks::getMemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<uint32_t, uint8_t> const &payload) -> std::vector<MemBlock>
{
    std::vector<MemBlock> memBlocks;
    std::vector<uint8_t> currMemBlockBytes;
    uint32_t currMemBlockAddress = 0;
    CodeLine const *pPrevCodeLine = nullptr;

    for (auto const &codeLine : codeLines)
    {
        if (!areAdjacent(pPrevCodeLine, &codeLine))
        {
            if (!currMemBlockBytes.empty())
            {
                memBlocks.emplace_back(MemBlock(currMemBlockAddress, currMemBlockBytes));
                currMemBlockBytes.clear();
            }

            currMemBlockAddress = codeLine.getStartAddress();
        }

        for (uint32_t idx = 0; idx < codeLine.getLengthBytes(); idx++)
        {
            currMemBlockBytes.push_back(payload.at(codeLine.getStartAddress() + idx));
        }

        pPrevCodeLine = &codeLine;
    }

    if (!currMemBlockBytes.empty())
    {
        memBlocks.emplace_back(MemBlock(currMemBlockAddress, currMemBlockBytes));
        currMemBlockBytes.clear();
    }

    // sort the various blocks by starting address
    std::sort(begin(memBlocks), end(memBlocks));

    return memBlocks;
}

auto MemBlocks::areAdjacent(asm6502::CodeLine const *prevCodeLine, asm6502::CodeLine const *currCodeLine) -> bool
{
    return (prevCodeLine != nullptr) && (prevCodeLine->getStartAddress() + prevCodeLine->getLengthBytes() == currCodeLine->getStartAddress());
}

auto MemBlocks::getBasicMemBlockInitializerListing() const -> std::string 
{
    std::stringstream strm;

    strm << "100 read nb" << std::endl;
    strm << "110 for bi = 1 to nb" << std::endl;
    strm << "120 read addr" << std::endl;
    strm << "130 read nby" << std::endl;
    strm << "140 for byi = 1 to nby" << std::endl;
    strm << "150 read byvl" << std::endl;
    strm << "160 poke addr+byi-1, byvl" << std::endl;
    strm << "170 next byi" << std::endl;
    strm << "180 next bi" << std::endl;
    strm << "190 end" << std::endl;

    uint32_t lineNr = 200;
    strm << lineNr << " rem number of mem blocks" << std::endl;
    lineNr += 10;
    strm << lineNr << " data " << getNumMemBlocks() << std::endl;
    lineNr += 10;

    for (uint32_t memBlockIdx = 0; memBlockIdx < getNumMemBlocks(); memBlockIdx++)
    {
        MemBlock const &memBlock = getMemBlockAt(memBlockIdx);
        strm << lineNr << " rem block start number bytes" << std::endl;
        lineNr += 10;
        strm << lineNr << " data " << memBlock.getStartAddress() << ", " << memBlock.getLengthBytes();
        lineNr += 10;

        for (uint32_t byteIdx = 0; byteIdx < memBlock.getLengthBytes(); byteIdx++)
        {
            if (byteIdx % 4 == 0)
            {
                strm << std::endl << lineNr << " data ";
                lineNr += 10;
            }

            strm << std::setw(3) << static_cast<uint32_t>(memBlock.getByteAt(byteIdx));

            if ((byteIdx % 4 != 3) && (byteIdx + 1 < memBlock.getLengthBytes()))
            {
                strm << ",";
            }
        }

        strm << std::endl;
    }

    return strm.str();
}

auto MemBlocks::getByteAt(uint32_t address) const -> uint8_t
{
    uint8_t ret = 0xff;
    for (auto const &mb : memBlocks)
    {
        if ((mb.getStartAddress() <= address) && (mb.getStartAddress() + mb.getLengthBytes() > address))
        {
            ret = mb.getByteAt(address - mb.getStartAddress());
            break;
        }
    }

    return ret;
}

auto MemBlocks::getMachineCode(bool includeAssembly) const -> std::string 
{
    std::stringstream strm;

    for (auto const &codeLine : codeLines)
    {
        strm << codeLine.get(*this, includeAssembly);
    }
    return strm.str();
}

// streams the list of mem blocks into a binary .prg file, which starts
// with a 16 bit start address (Little Endian), followed by the byte stream
// followed by a zero byte as terminator
auto asm6502::operator << (std::ostream &os, asm6502::MemBlocks const &memBlocks) -> std::ostream & 
{  
    if (memBlocks.getNumMemBlocks() > 0)
    {
        auto itMemBlock = begin(memBlocks.memBlocks);
        uint32_t startAddress = itMemBlock->getStartAddress();
        os << static_cast<uint8_t>(startAddress & 0xffU); // LSB of start address
        os << static_cast<uint8_t>((startAddress >> 8U) & 0xffU); // MSB of start address

        while (itMemBlock != end(memBlocks.memBlocks))
        {
            os << *itMemBlock;
            auto itNextMemBlock = itMemBlock + 1 ;

            if (itNextMemBlock != end(memBlocks.memBlocks))
            {
                // should be safe, start addresses and lengths are in fact uint32_t (16 bit)
                auto numPadding = itNextMemBlock->getStartAddress() - (itMemBlock->getStartAddress() + itMemBlock->getLengthBytes());

                for (uint32_t padIdx = 0; padIdx < numPadding; padIdx++)
                {
                    os << static_cast<uint8_t>(0xff);
                }
            }

            itMemBlock = itNextMemBlock;
        }
    }

    // zero-byte termination: Added this since converting a binary file without it would yield a truncated
    // file (last actual byte was missing) when writing it into a .d64 image file. Not yet sure whether
    // this is aproblem in the file or in the .d64 conversion
    // is it perhaps that .PRG files should only contain an *even* number of bytes?
    // in that case, pad only to make the byte length even
    os << static_cast<uint8_t>(0x00);

    return os;
}

auto asm6502::operator << (std::ostream &os, asm6502::MemBlock const &memBlock) -> std::ostream &
{
    for (auto byte : memBlock.bytes) { os << byte; }
    return os;
}


