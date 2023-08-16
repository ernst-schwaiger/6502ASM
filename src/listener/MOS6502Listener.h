/*
 * MOS6502Listener.h
 *
 *  Created on: 19.08.2018
 *      Author: Ernst
 */

#ifndef MOS6502LISTENER_H_
#define MOS6502LISTENER_H_

#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <iomanip>


#include "MOS6502BaseListener.h"

namespace asm6502
{

class ISymbolTable
{
public:
	virtual ~ISymbolTable() {};
	virtual unsigned int resolveSymbol(std::string const &symbol) const = 0;
	virtual void addSymbol(std::string const &symbol, unsigned int val) = 0;
};

class IExpression
{
public:
	virtual ~IExpression() {};
	virtual unsigned int eval(ISymbolTable const *symbolTable) const = 0;
};

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

	std::string get(std::map<unsigned int, unsigned char> const &payload)
	{
		std::stringstream strm;
		int column = 0;

		std::string pl = getPayload(payload);

		strm << pl;
		column += pl.length();

		while (column < 24)
		{
			strm << " ";
			column++;
		}

		MOS6502Parser::LabelContext *labelCtx = ctx->label();

		if (labelCtx)
		{
			strm << labelCtx->getText();
			column += labelCtx->getText().length();
		}

		while (column < 36)
		{
			strm << " ";
			column++;
		}
		MOS6502Parser::DirectiveContext *dirCtx = ctx->directive();
		MOS6502Parser::StatementContext *stmntCtx = ctx->statement();

		strm << (dirCtx ? dirCtx->getText() : stmntCtx->getText()) << std::endl;

		return strm.str();
	}

	std::string getPayload(std::map<unsigned int, unsigned char> const &payload)
	{
		std::stringstream strm;

		if (lengthBytes > 0)
		{
			strm << "0x" << std::hex << std::setw(4) << std::setfill('0') << startAddress << ":";

			unsigned int lastByteAddr = startAddress + (lengthBytes - 1);

			for (unsigned int addr = startAddress; addr < lastByteAddr; addr++)
			{
				strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(payload.at(addr)) << ",";
			}

			strm << "0x" << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(payload.at(lastByteAddr));
			strm << " ";
		}

		return strm.str();
	}

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

private:

	std::vector<MemBlock> getMemBlocks(std::vector<asm6502::CodeLine> const &codeLines, std::map<unsigned int, unsigned char> const &payload)
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

	bool areAdjacent(asm6502::CodeLine const *prevCodeLine, asm6502::CodeLine const *currCodeLine) const
	{
		return (prevCodeLine != nullptr) && (prevCodeLine->getStartAddress() + prevCodeLine->getLengthBytes() == currCodeLine->getStartAddress());
	}

	std::vector<MemBlock> _memBlocks;
};



class MOS6502Listener : public MOS6502BaseListener
{
public:
	MOS6502Listener();
	virtual ~MOS6502Listener();

	void exitOrg_directive(MOS6502Parser::Org_directiveContext * /*ctx*/) override;
	void exitByte_directive(MOS6502Parser::Byte_directiveContext * /*ctx*/) override;
	void exitWord_directive(MOS6502Parser::Word_directiveContext * /*ctx*/) override;
	void exitDbyte_directive(MOS6502Parser::Dbyte_directiveContext * /*ctx*/) override;

	void exitLabel(MOS6502Parser::LabelContext * /*ctx*/) override;
	void exitAss_directive(MOS6502Parser::Ass_directiveContext * /*ctx*/) override;

	void exitDir_statement(MOS6502Parser::Dir_statementContext * /*ctx*/) override;
	void exitImm_statement(MOS6502Parser::Imm_statementContext * /*ctx*/) override;
	void exitRel_statement(MOS6502Parser::Rel_statementContext * /*ctx*/) override;
	void exitIdx_x_statement(MOS6502Parser::Idx_x_statementContext * /*ctx*/) override;
	void exitIdx_y_statement(MOS6502Parser::Idx_y_statementContext * /*ctx*/) override;
	void exitIdx_abs_statement(MOS6502Parser::Idx_abs_statementContext * /*ctx*/) override;
	void exitIdr_statement(MOS6502Parser::Idr_statementContext * /*ctx*/) override;
	void exitIdx_idr_statement(MOS6502Parser::Idx_idr_statementContext * /*ctx*/) override;
	void exitIdr_idx_statement(MOS6502Parser::Idr_idx_statementContext * /*ctx*/) override;


	void exitExpression(MOS6502Parser::ExpressionContext * ctx) override;
	void exitSymbol(MOS6502Parser::SymbolContext * /*ctx*/) override;
	void exitDec8(MOS6502Parser::Dec8Context * ctx) override;
	void exitDec(MOS6502Parser::DecContext * ctx) override;
	void exitHex16(MOS6502Parser::Hex16Context * ctx) override;
	void exitHex8(MOS6502Parser::Hex8Context * ctx) override;
	void exitBin8(MOS6502Parser::Bin8Context * ctx) override;
	void exitChar8(MOS6502Parser::Char8Context * ctx) override;
	void exitData_string(MOS6502Parser::Data_stringContext * /*ctx*/) override;

	void exitLine(MOS6502Parser::LineContext * /*ctx*/) override;

	void resolveBranchTargets();
	void outputPayload();

private:

	unsigned int convertDec(std::string const &dec) const;
	unsigned int convertHex(std::string const &hex) const;
	unsigned int convertBin(std::string const &bin) const;

	unsigned int popExpression();
	std::vector<unsigned int> popAllExpressions();

	void appendIdxOrZpgCmd(unsigned char opcode, unsigned char opcode_zpg, unsigned int operand);

	void appendByteToPayload(unsigned char byte);
	void addWordToPayload(unsigned short word);
	void addDByteToPayload(unsigned short dbyte);

	unsigned int currentAddress;
	int addressOfLine;
	std::vector<std::pair<unsigned int, std::shared_ptr<IExpression const>>> branchTargets; // branch tgt addresses to labels
	std::unique_ptr<ISymbolTable> symbolTable;
	std::vector<std::shared_ptr<IExpression>> expressionStack;
	std::map<unsigned int, unsigned char> payload;
	std::vector<CodeLine> codeLines;
};

} /* namespace asm6502 */

#endif /* MOS6502LISTENER_H_ */
