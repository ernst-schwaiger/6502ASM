#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <string>
#include <map>
#include <optional>

namespace asm6502
{

class Sym
{
public:
    Sym(size_t line_, size_t col_, unsigned int val_) : 
        line{line_},
        col{col_},
        val{val_}
    {}

    Sym() : 
        line{0},
        col{0},
        val{0}
    {}

    size_t line;
    size_t col;
    unsigned int val;
};

class SymbolTable
{
public:

    std::optional<Sym> resolveSymbol(std::string const &symbol) const
    {
        std::optional<Sym> ret = std::nullopt;
        auto pos = symbols.find(symbol);

        if (pos != end(symbols))
        {
            ret = std::optional<Sym>(pos->second);
        }

        return ret;
    }

    void addSymbol(std::string const &symbol, size_t line, size_t col, unsigned int val)
    {
        // if a symbol aready existed, we overwrite it here
        // symbol clashes must be covered by the caller
        symbols[symbol] = {line, col, val};
    }

private:
    std::map<std::string, Sym> symbols;
};
} // namespace
#endif
