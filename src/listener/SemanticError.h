#ifndef SEMANTIC_ERROR_H
#define SEMANTIC_ERROR_H

#include <string>
#include <sstream>

namespace asm6502
{
class SemanticError
{
public:
    SemanticError(std::string const &_errmsg, std::string const &_filename, std::size_t _linenr, std::size_t _colnr) : 
        errmsg{_errmsg},
        filename{_filename},
        linenr{_linenr},
        colnr{_colnr}
    {}

    std::string getErrorMessage() const
    {
        std::stringstream strm;
        strm << filename << ":" << linenr << ":" << colnr << ": error: " << errmsg << std::endl;

        return strm.str();
    }
    
private:

    std::string errmsg;
    std::string filename;
    std::size_t linenr;
    std::size_t colnr;
};
}

#endif
