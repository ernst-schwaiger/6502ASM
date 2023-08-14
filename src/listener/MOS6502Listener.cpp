/*
 * CMOS6502Listener.cpp
 *
 *  Created on: 19.08.2018
 *      Author: Ernst
 */

#include "MOS6502Listener.h"
#include <iostream>
#include <string>
#include <iterator>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace asm6502
{

static unsigned char const jmp_indir_opcode = 0x6C;

static map<string, unsigned char> dir_opcodes
{
	{"BRK", 0x00}, {"PHP", 0x08}, {"ASL", 0x0A}, {"CLC", 0x18}, {"PLP", 0x28}, {"ROL", 0x2A}, {"SEC", 0x38}, {"RTI", 0x40},
	{"PHA", 0x48}, {"LSR", 0x4A}, {"CLI", 0x58}, {"RTS", 0x60}, {"PLA", 0x68}, {"ROR", 0x6A}, {"SEI", 0x78}, {"DEY", 0x88},
	{"TXA", 0x8A}, {"TYA", 0x98}, {"TXS", 0x9A}, {"TAY", 0xA8}, {"TAX", 0xAA}, {"CLV", 0xB8}, {"TSX", 0xBA}, {"INY", 0xC8},
	{"DEX", 0xCA}, {"CLD", 0xD8}, {"INX", 0xE8}, {"NOP", 0xEA}, {"SED", 0xF8}
};

static map<string, unsigned char> imm_opcodes
{
	{"ORA", 0x09}, {"AND", 0x29}, {"EOR", 0x49}, {"ADC", 0x69}, {"LDY", 0xA0}, {"LDX", 0xA2}, {"LDA", 0xA9}, {"CPY", 0xC0},
	{"CMP", 0xC9}, {"CPX", 0xE0}, {"SBC", 0xE9}
};

static map<string, unsigned char> rel_opcodes
{
	{"BPL", 0x10}, {"BMI", 0x30}, {"BVC", 0x50}, {"BVS", 0x70}, {"BCC", 0x90}, {"BCS", 0xB0}, {"BNE", 0xD0}, {"BEQ", 0xF0}
};

static map<string, unsigned char> idx_x_opcodes
{
	{"ORA", 0x1D}, {"ASL", 0x1E}, {"AND", 0x3D}, {"ROL", 0x3E}, {"EOR", 0x5D}, {"LSR", 0x5E}, {"ADC", 0x7D}, {"ROR", 0x7E},
	{"STA", 0x9D}, {"LDY", 0xBC}, {"LDA", 0xBD}, {"CMP", 0xDD}, {"DEC", 0xDE}, {"SBC", 0xFD}, {"INC", 0xFE}
};

static map<string, unsigned char> idx_x_zpg_opcodes
{
	{"ORA", 0x15}, {"ASL", 0x16}, {"AND", 0x35}, {"ROL", 0x36}, {"EOR", 0x55}, {"LSR", 0x56}, {"ADC", 0x75}, {"ROR", 0x76},
	{"STY", 0x94}, {"STA", 0x95}, {"LDY", 0xB4}, {"LDA", 0xB5}, {"CMP", 0xD5}, {"DEC", 0xD6}, {"DBC", 0xF5}, {"INC", 0xF6}
};

static map<string, unsigned char> idx_y_opcodes
{
	{"ORA", 0x19}, {"AND", 0x39}, {"EOR", 0x59}, {"ADC", 0x79}, {"STA", 0x99}, {"LDA", 0xB9}, {"LDX", 0xBE}, {"CMP", 0xD9},
	{"SBC", 0xF9}
};

static map<string, unsigned char> idx_y_zpg_opcodes
{
	{"STX", 0x96}, {"LDX", 0xB6}
};

static map<string, unsigned char> abs_opcodes
{
	{"ORA", 0x0D}, {"ASL", 0x0E}, {"JSR", 0x20}, {"BIT", 0x2C}, {"AND", 0x2D}, {"ROL", 0x2E}, {"JMP", 0x4C}, {"JMP", 0x4C},
	{"EOR", 0x4D}, {"LSR", 0x4E}, {"ADC", 0x6D}, {"ROR", 0x6E}, {"STY", 0x8C}, {"STA", 0x8D}, {"STX", 0x8E}, {"LDY", 0xAC},
	{"LDA", 0xAD}, {"LDX", 0xAE}, {"CPY", 0xCC}, {"CMP", 0xCD}, {"DEC", 0xCE}, {"CPX", 0xEC}, {"SBC", 0xED}, {"INC", 0xEE}
};

static map<string, unsigned char> abs_zpg_opcodes
{
	{"ORA", 0x05}, {"ASL", 0x06}, {"BIT", 0x24}, {"AND", 0x25}, {"ROL", 0x26}, {"EOR", 0x45}, {"LSR", 0x46}, {"ADC", 0x65},
	{"ROR", 0x66}, {"STY", 0x84}, {"STA", 0x85}, {"STX", 0x86}, {"LDY", 0xA4}, {"LDA", 0xA5}, {"LDX", 0xA6}, {"CPY", 0xC4},
	{"CMP", 0xC5}, {"DEC", 0xC6}, {"CPX", 0xE4}, {"SBC", 0xE5}, {"INC", 0xE6}
};

static map<string, unsigned char> idx_idr_opcodes
{
	{"ORA", 0x01}, {"AND", 0x21}, {"EOR", 0x41}, {"ADC", 0x61}, {"STA", 0x81}, {"LDA", 0xA1}, {"CMP", 0xC1}, {"SBC", 0xE1}
};

static map<string, unsigned char> idr_idx_opcodes
{
	{"ORA", 0x11}, {"AND", 0x31}, {"EOR", 0x51}, {"ADC", 0x71}, {"STA", 0x91}, {"LDA", 0xB1}, {"CMP", 0xD1}, {"SBC", 0xF1}
};


unsigned char findOpCode(map<string, unsigned char> const &opcodeMap, string const &opcode)
{
	unsigned char ret = 0;
	auto pos = opcodeMap.find(opcode);

	if (pos != end(opcodeMap))
	{
		ret = pos->second;
	}

	return ret;
}

pair<unsigned char, unsigned char> findIdxZpgOpCodes(map<string, unsigned char> const &opcodeMap, map<string, unsigned char> const &zpgOpcodeMap, string const &opCode)
{
	unsigned char idxOpCode = findOpCode(opcodeMap, opCode);
	unsigned char zgpOpCode = findOpCode(zpgOpcodeMap, opCode);

	return {idxOpCode, zgpOpCode};
}


function<unsigned int(unsigned int, unsigned int)> add = [](unsigned int arg1, unsigned int arg2)
{
	return arg1 + arg2;
};

function<unsigned int(unsigned int, unsigned int)> sub = [](unsigned int arg1, unsigned int arg2)
{
	return arg1 - arg2;
};

function<unsigned int(unsigned int, unsigned int)> div = [](unsigned int arg1, unsigned int arg2)
{
	return arg1 / arg2;
};

function<unsigned int(unsigned int, unsigned int)> mul = [](unsigned int arg1, unsigned int arg2)
{
	return arg1 * arg2;
};

class DummySymbolTable : public ISymbolTable
{
public:
	virtual unsigned int resolveSymbol(string const &symbol) const override
	{
		unsigned int ret = 42;
		auto pos = symbols.find(symbol);

		if (pos != end(symbols))
		{
			ret = pos->second;
		}

		return ret;
	}

	virtual void addSymbol(string const &symbol, unsigned int val)
	{
		auto pos = symbols.find(symbol);

		if (pos == end(symbols))
		{
			// FIXME: Check for redefined symbols
			symbols[symbol] = val;
		} else
		{
			stringstream msg;
			msg << "Symbol \"" << symbol << "\" already defined as " << (*pos).second << ends;

			string errormsg = msg.str();
			logic_error e (errormsg);


			throw logic_error(e);
		}
	}

	virtual ~DummySymbolTable()
	{
		cout << "In dtor of symbol table" << endl;
	}

private:

	map<string, unsigned int> symbols;
};

class Numeric : public IExpression
{
public:
	Numeric(unsigned int _val) : val{_val}{}
	virtual unsigned int eval(ISymbolTable const *symbolTable) const override
	{
		return val;
	}

private:
	unsigned int const val;
};

class Symbol : public IExpression
{
public:
	Symbol(string const &_symbol) : symbol{_symbol} {}
	virtual unsigned int eval(ISymbolTable const *symbolTable) const override
	{
		return symbolTable->resolveSymbol(symbol);
	}

private:

	string const symbol;
};

class BinaryOperation : public IExpression
{
public:
	BinaryOperation(shared_ptr<IExpression> const _arg1, shared_ptr<IExpression> const _arg2, function<unsigned int(unsigned int, unsigned int)> const *_op) :
		arg1{_arg1},
		arg2{_arg2},
		op{_op}
		{};

		virtual unsigned int eval(ISymbolTable const *symbolTable) const override
		{
			return (*op)(arg1->eval(symbolTable), arg2->eval(symbolTable));
		}

private:
		shared_ptr<IExpression> const arg1;
		shared_ptr<IExpression> const arg2;
		function<unsigned int(unsigned int, unsigned int)> const *op;
};


CMOS6502Listener::CMOS6502Listener()  :
		currentAddress{0},
		addressOfLine{-1},
		branchTargets{},
		symbolTable{make_unique<DummySymbolTable>()},
		expressionStack{},
		payload{},
		codeLines{}
{
	// TODO Auto-generated constructor stub

}

CMOS6502Listener::~CMOS6502Listener()
{
	// TODO Auto-generated destructor stub
}

void CMOS6502Listener::exitOrg_directive(MOS6502Parser::Org_directiveContext * ctx)
{
	currentAddress = popExpression();
	addressOfLine = currentAddress;
}

void CMOS6502Listener::exitByte_directive(MOS6502Parser::Byte_directiveContext * ctx)
{
	for (auto byte : popAllExpressions())
	{
		// FIXME: Add a warning if byte is out of 0..255 bounds
		appendByteToPayload((unsigned char)byte);
	}
}

void CMOS6502Listener::exitWord_directive(MOS6502Parser::Word_directiveContext *ctx)
{
	for (auto word : popAllExpressions())
	{
		// FIXME: Add a warning if byte is out of 0..255 bounds
		addWordToPayload((unsigned short)word);
	}
}

void CMOS6502Listener::exitDbyte_directive(MOS6502Parser::Dbyte_directiveContext *ctx)
{
	for (auto dbyte : popAllExpressions())
	{
		// FIXME: Add a warning if byte is out of 0..255 bounds
		addWordToPayload((unsigned short)dbyte);
	}
}

void CMOS6502Listener::exitLabel(MOS6502Parser::LabelContext *ctx)
{
	symbolTable->addSymbol(ctx->ID()->getText(), currentAddress);
}

void CMOS6502Listener::exitAss_directive(MOS6502Parser::Ass_directiveContext *ctx)
{
	symbolTable->addSymbol(ctx->ID()->getText(), popExpression());
}

void CMOS6502Listener::exitDir_statement(MOS6502Parser::Dir_statementContext *ctx)
{
	appendByteToPayload(findOpCode(dir_opcodes, ctx->dir_opcode()->getText()));
}

void CMOS6502Listener::exitImm_statement(MOS6502Parser::Imm_statementContext *ctx)
{
	appendByteToPayload(findOpCode(imm_opcodes, ctx->imm_opcode()->getText()));
	appendByteToPayload(popExpression());
}

void CMOS6502Listener::exitRel_statement(MOS6502Parser::Rel_statementContext *ctx)
{
	appendByteToPayload(findOpCode(rel_opcodes, ctx->rel_opcode()->getText()));

	// the relative operand can only be resolved at the end of the assembler
	// run, since labels can be assigned here that have not yet been parsed
	auto label = make_shared<Symbol>(ctx->symbol()->getText());

	branchTargets.emplace_back(pair<unsigned int, shared_ptr<IExpression>>{currentAddress, label});
	++currentAddress;
}

void CMOS6502Listener::exitIdx_x_statement(MOS6502Parser::Idx_x_statementContext *ctx)
{
	auto opcodeStr = ctx->idx_opcode()->getText();
	auto opCodeZpgOpCode = findIdxZpgOpCodes(idx_x_opcodes, idx_x_zpg_opcodes, opcodeStr);
	unsigned int operand = popExpression();
	appendIdxOrZpgCmd(opCodeZpgOpCode.first, opCodeZpgOpCode.second, operand);
}

void CMOS6502Listener::exitIdx_y_statement(MOS6502Parser::Idx_y_statementContext *ctx)
{
	auto opcodeStr = ctx->idy_opcode()->getText();
	auto opCodeZpgOpCode = findIdxZpgOpCodes(idx_y_opcodes, idx_y_zpg_opcodes, opcodeStr);
	unsigned int operand = popExpression();
	appendIdxOrZpgCmd(opCodeZpgOpCode.first, opCodeZpgOpCode.second, operand);
}

void CMOS6502Listener::exitIdx_abs_statement(MOS6502Parser::Idx_abs_statementContext *ctx)
{
	auto opcodeStr = ctx->idabs_opcode()->getText();
	auto opCodeZpgOpCode = findIdxZpgOpCodes(abs_opcodes, abs_zpg_opcodes, opcodeStr);
	unsigned int operand = popExpression();
	appendIdxOrZpgCmd(opCodeZpgOpCode.first, opCodeZpgOpCode.second, operand);
}

void CMOS6502Listener::exitIdx_idr_statement(MOS6502Parser::Idx_idr_statementContext *ctx)
{
	auto opcodeStr = ctx->idx_idr_idx_opcode()->getText();
	auto opcode = findOpCode(idx_idr_opcodes, opcodeStr);
	unsigned int operand = popExpression();

	appendByteToPayload(opcode);
	appendByteToPayload(operand & 0xff);
}

void CMOS6502Listener::exitIdr_idx_statement(MOS6502Parser::Idr_idx_statementContext *ctx)
{
	auto opcodeStr = ctx->idx_idr_idx_opcode()->getText();
	auto opcode = findOpCode(idr_idx_opcodes, opcodeStr);
	unsigned int operand = popExpression();

	appendByteToPayload(opcode);
	appendByteToPayload(operand & 0xff);
}


void CMOS6502Listener::appendIdxOrZpgCmd(unsigned char opcode, unsigned char opcode_zpg, unsigned int operand)
{
	if (operand <= 0xff && opcode_zpg > 0)
	{
		appendByteToPayload(opcode_zpg);
		appendByteToPayload(operand & 0xff);
	}
	else
	{
		appendByteToPayload(opcode);
		appendByteToPayload(operand & 0xff);
		appendByteToPayload((operand >> 8) & 0xff);
	}
}

void CMOS6502Listener::exitIdr_statement(MOS6502Parser::Idr_statementContext *ctx)
{
	// there is only JMP: 0x6C
	appendByteToPayload(jmp_indir_opcode);
	unsigned int operand = popExpression();
	appendByteToPayload(operand & 0xff);
	appendByteToPayload((operand >> 8) & 0xff);
}


void CMOS6502Listener::exitExpression(MOS6502Parser::ExpressionContext * ctx)
{
	function<unsigned int(unsigned int, unsigned int)> const *op = nullptr;
	if (ctx->ADD() != nullptr)
	{
		op = &add;
	} else if (ctx->SUB() != nullptr)
	{
		op = &sub;
	} else if (ctx->MUL() != nullptr)
	{
		op = &mul;
	} else if (ctx->DIV() != nullptr)
	{
		op = &div;
	}

	if (op != nullptr)
	{
		auto arg2 = expressionStack.back();
		expressionStack.pop_back();
		auto arg1 = expressionStack.back();
		expressionStack.pop_back();

		expressionStack.emplace_back(make_shared<BinaryOperation>(arg1, arg2, op));
//			cout << "Expression: " << ctx->getText() << " evaluates to "
//				<< expressionStack.back().get()->eval(symbolTable.get()) << endl;
	}
}

void CMOS6502Listener::exitSymbol(MOS6502Parser::SymbolContext *ctx)
{
	unsigned int symbolVal = symbolTable.get()->resolveSymbol(ctx->ID()->getText());
	expressionStack.emplace_back(make_shared<Numeric>(symbolVal));
}

void CMOS6502Listener::exitDec8(MOS6502Parser::Dec8Context * ctx)
{
	int val = convertDec(ctx->getText());
	expressionStack.emplace_back(make_shared<Numeric>(val));

//	cout << "Dec8Val: " << val << endl;
}

void CMOS6502Listener::exitDec(MOS6502Parser::DecContext * ctx)
{
	int val = convertDec(ctx->getText());
	expressionStack.emplace_back(make_shared<Numeric>(val));
	//cout << "DecVal: " << val << endl;
}

void CMOS6502Listener::exitHex16(MOS6502Parser::Hex16Context * ctx)
{
	// w/o leading $ sign
	int val = convertHex(ctx->getText().substr(1));
	expressionStack.emplace_back(make_shared<Numeric>(val));
	//cout << "Hex16Val: " << val << endl;
}

void CMOS6502Listener::exitHex8(MOS6502Parser::Hex8Context * ctx)
{
	// w/o leading $ sign
	int val = convertHex(ctx->getText().substr(1));
	expressionStack.emplace_back(make_shared<Numeric>(val));
	//cout << "Hex8Val: " << val << endl;
}

void CMOS6502Listener::exitBin8(MOS6502Parser::Bin8Context * ctx)
{
	// w/o leading % sign
	int val = convertBin(ctx->getText().substr(1));
	expressionStack.emplace_back(make_shared<Numeric>(val));
	cout << "BinVal: " << val << endl;
}

void CMOS6502Listener::exitChar8(MOS6502Parser::Char8Context * ctx)
{
	// w/o leading/trailing apos
	int val = ctx->getText()[1];
	expressionStack.emplace_back(make_shared<Numeric>(val));
	//cout << "CharVal: " << val << endl;
}

void CMOS6502Listener::exitData_string(MOS6502Parser::Data_stringContext * ctx)
{
	auto stringWithQuotes = ctx->STRING()->getText();
	auto stringNoQuotes = stringWithQuotes.substr(1, stringWithQuotes.length() - 2);

	for (auto val : stringNoQuotes)
	{
		expressionStack.emplace_back(make_shared<Numeric>(val));
	}
}

unsigned int CMOS6502Listener::convertDec(string const &dec) const
{
	int ret = 0;
	stringstream ss;
	ss << dec;
	ss >> ret;

	return ret;
}

unsigned int CMOS6502Listener::convertHex(string const &hexa) const
{
	int ret = 0;
	stringstream ss;
	ss << hex << hexa;
	ss >> ret;

	return ret;
}

unsigned int CMOS6502Listener::convertBin(string const &bin) const
{
	int ret = 0;

	for (auto c : bin)
	{
		ret <<= 1;

		if (c == '1')
		{
			ret++;
		}
	}

	return ret;
}

void CMOS6502Listener::exitLine(MOS6502Parser::LineContext *ctx)
{
	unsigned int startAddress = 0;
	unsigned int numberOfBytes = 0;

	if (addressOfLine >= 0)
	{
		startAddress = addressOfLine;
		numberOfBytes = currentAddress - addressOfLine;
	}

	CodeLine codeLine(ctx, startAddress, numberOfBytes);
	codeLines.push_back(codeLine);
	addressOfLine = -1;
}

void CMOS6502Listener::resolveBranchTargets()
{
	for (auto foo : branchTargets)
	{
		unsigned int branchOperandAddress = foo.first;
		unsigned int destAddress = foo.second->eval(symbolTable.get());

		// relative address: destination address minus
		// address after branch statement (i.e. after operand address)
		int offset = destAddress - (branchOperandAddress + 1);

		payload[branchOperandAddress] = (offset & 0xff);
	}
}

void CMOS6502Listener::outputPayload()
{
	for (auto codeLine : codeLines)
	{
		cout << codeLine.get(payload);
	}


	MemBlocks memBlocks(codeLines, payload);

	cout << "--- mem blocks ---" << std::endl;

	cout << "100 read nb" << std::endl;
	cout << "110 for bi = 1 to nb" << std::endl;
	cout << "120 read addr" << std::endl;
	cout << "130 read nby" << std::endl;
	cout << "140 for byi = 1 to nby" << std::endl;
	cout << "150 read byvl" << std::endl;
	cout << "160 poke addr+byi-1, byvl" << std::endl;
	cout << "170 next byi" << std::endl;
	cout << "180 next bi" << std::endl;
	cout << "190 end" << std::endl;

	unsigned int lineNr = 200;
	cout << lineNr << " rem number of mem blocks" << std::endl;
	lineNr += 10;
	cout << lineNr << " data " << memBlocks.getNumMemBlocks() << std::endl;
	lineNr += 10;

	for (unsigned int memBlockIdx = 0; memBlockIdx < memBlocks.getNumMemBlocks(); memBlockIdx++)
	{
		MemBlock const &memBlock = memBlocks.getMemBlockAt(memBlockIdx);
		cout << lineNr << " rem block start number bytes" << std::endl;
		lineNr += 10;
		cout << lineNr << " data " << memBlock.getStartAddress() << ", " << memBlock.getLengthBytes();
		lineNr += 10;

		for (unsigned int byteIdx = 0; byteIdx < memBlock.getLengthBytes(); byteIdx++)
		{
			if (byteIdx % 4 == 0)
			{
				cout << std::endl << lineNr << " data ";
				lineNr += 10;
			}

			cout << std::setw(3) << static_cast<unsigned int>(memBlock.getByteAt(byteIdx));

			if ((byteIdx % 4 != 3) && (byteIdx + 1 < memBlock.getLengthBytes()))
			{
				cout << ",";
			}
		}

		cout << std::endl;

		//strm << "0x" << std::hex << std::setw(4) << std::setfill('0') << startAddress << ":";
		
	}
}

unsigned int CMOS6502Listener::popExpression()
{
	unsigned int ret = expressionStack.back()->eval(symbolTable.get());
	expressionStack.pop_back();
	return ret;
}

vector<unsigned int> CMOS6502Listener::popAllExpressions()
{
	vector<unsigned int> ret;

	// FIXME: Find a way to do that with lambdas
	for (auto e : expressionStack)
	{
		ret.push_back(e->eval(symbolTable.get()));
	}

	expressionStack.clear();

	return ret;
}

void CMOS6502Listener::appendByteToPayload(unsigned char byte)
{
	if (addressOfLine < 0)
	{
		addressOfLine = currentAddress;
	}

	payload[currentAddress++] = byte;
}


void CMOS6502Listener::addWordToPayload(unsigned short word)
{
	unsigned char lsb = word & 0xff;
	unsigned char msb = (word >> 8) & 0xff;

	// Little endian architecture
	appendByteToPayload(lsb);
	appendByteToPayload(msb);
}

void CMOS6502Listener::addDByteToPayload(unsigned short dbyte)
{
	unsigned char lsb = dbyte & 0xff;
	unsigned char msb = (dbyte >> 8) & 0xff;

	// write as two bytes, ignoring endianess
	appendByteToPayload(msb);
	appendByteToPayload(lsb);
}


} /* namespace asm6502 */
