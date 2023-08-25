#include "MemBlocks.h"
#include <iomanip>
using namespace asm6502;

std::vector<MemBlock> MemBlocks::getMemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<unsigned int, unsigned char> const &payload)
{
    std::vector<MemBlock> memBlocks;
    std::vector<unsigned char> currMemBlockBytes;
    unsigned int currMemBlockAddress = 0;
    CodeLine const *pPrevCodeLine = nullptr;

    for (auto &codeLine : codeLines)
    {
        if (!areAdjacent(pPrevCodeLine, &codeLine))
        {
            if (currMemBlockBytes.size() > 0)
            {
                memBlocks.push_back(MemBlock(currMemBlockAddress, currMemBlockBytes));
                currMemBlockBytes.clear();
            }

            currMemBlockAddress = codeLine.getStartAddress();
        }

        for (unsigned int idx = 0; idx < codeLine.getLengthBytes(); idx++)
        {
            currMemBlockBytes.push_back(payload.at(codeLine.getStartAddress() + idx));
        }

        pPrevCodeLine = &codeLine;
    }

    if (currMemBlockBytes.size() > 0)
    {
        memBlocks.push_back(MemBlock(currMemBlockAddress, currMemBlockBytes));
        currMemBlockBytes.clear();
    }

    return memBlocks;
}

bool MemBlocks::areAdjacent(asm6502::CodeLine const *prevCodeLine, asm6502::CodeLine const *currCodeLine) const
{
    return (prevCodeLine != nullptr) && (prevCodeLine->getStartAddress() + prevCodeLine->getLengthBytes() == currCodeLine->getStartAddress());
}

std::string MemBlocks::getBasicMemBlockInitializerListing() const
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

    unsigned int lineNr = 200;
    strm << lineNr << " rem number of mem blocks" << std::endl;
    lineNr += 10;
    strm << lineNr << " data " << getNumMemBlocks() << std::endl;
    lineNr += 10;

    for (unsigned int memBlockIdx = 0; memBlockIdx < getNumMemBlocks(); memBlockIdx++)
    {
        MemBlock const &memBlock = getMemBlockAt(memBlockIdx);
        strm << lineNr << " rem block start number bytes" << std::endl;
        lineNr += 10;
        strm << lineNr << " data " << memBlock.getStartAddress() << ", " << memBlock.getLengthBytes();
        lineNr += 10;

        for (unsigned int byteIdx = 0; byteIdx < memBlock.getLengthBytes(); byteIdx++)
        {
            if (byteIdx % 4 == 0)
            {
                strm << std::endl << lineNr << " data ";
                lineNr += 10;
            }

            strm << std::setw(3) << static_cast<unsigned int>(memBlock.getByteAt(byteIdx));

            if ((byteIdx % 4 != 3) && (byteIdx + 1 < memBlock.getLengthBytes()))
            {
                strm << ",";
            }
        }

        strm << std::endl;
    }

    return strm.str();
}

unsigned char MemBlocks::getByteAt(unsigned int address) const
{
    unsigned char ret = 0xff;
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

std::string MemBlocks::getMachineCode(bool includeAssembly) const
{
    std::stringstream strm;

    for (auto const &codeLine : codeLines)
    {
        strm << codeLine.get(*this, includeAssembly);
    }
    return strm.str();
}
