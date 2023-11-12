#include <cassert>
#include <cstdlib>
#include <iostream>

#include "ASM6502.h"
#include "getopt.hpp"

using namespace std;
using namespace asm6502;

static int const RET_OK = 0;
static int const RET_ERR = 1;

void usage(char const *argv0)
{
    cerr 
        << "Usage: " << endl
        << argv0 << " <asmfile> [-a] [-b] [-p <progfile>]" << endl
        << "    -a: output assembly and machine code bytes" << endl
        << "    -b: output C64 basic program that pokes machine code into RAM" << endl
        << "    -p <progfile>: write machine code into a progfile (C64 .PRG)" << endl;
}

auto main(int argc, char *argv[]) -> int
{
    int ret = RET_OK;

    bool assemblyOut = false;
    bool basicOut = false;
    bool prgFileOut = false;
    std::string pProgFilePath = "";
    std::string asmFilePath = "";

    auto options = get_opt::getopt(argc, argv, "abp:");
    for (auto const &option : options)
    {
        switch(option.opt)
        {
            case 'a':
                assemblyOut = true;
                break;
            case 'b':
                basicOut = true;
                break;
            case 'p':
                prgFileOut = true;
                pProgFilePath = option.optarg;
                break;
            case '!': // no preceding dash
                asmFilePath=option.optarg;
                break;
            case '?':
                usage(argv[0]);
                ret = RET_ERR;
                break;

            default:
                assert(0);
        }
    }

    // int opt;
    // while ((opt = getopt(argc, argv, "abp:")) != -1)
    // {
    //     switch(opt)
    //     {
    //         case 'a':
    //             assemblyOut = true;
    //             break;
    //         case 'b':
    //             basicOut = true;
    //             break;
    //         case 'p':
    //             prgFileOut = true;
    //             pProgFilePath = optarg;
    //             break;
    //         case '?':
    //             usage(argv[0]);
    //             ret = RET_ERR;
    //             break;
    //         default:
    //             assert(0);
    //     }
    // }

    // no parameters given -> default behavior: Output assembly and basic program
    if (!(assemblyOut ||  basicOut || prgFileOut))
    {
        assemblyOut = true;
        basicOut = true;
    }

    if (ret == RET_OK)
    {
        if (!asmFilePath.empty() ) // asmfile is one additional parameter w/o options
        {
            AssemblyStatus assemblyStatus = assembleFile(asmFilePath.c_str());

            if (assemblyStatus.errors.empty())
            {
                if (assemblyOut)
                {
                    cout << "--- 6502 Machine Code ---" << std::endl;
                    cout << assemblyStatus.assembledProgram.getMachineCode(true) << std::endl;
                }

                if (basicOut)
                {
                    cout << "--- Commodore Basic Initializer Listing ---" << std::endl;
                    cout << assemblyStatus.assembledProgram.getBasicMemBlockInitializerListing();
                }

                if (prgFileOut)
                {
                    writeProgFile(pProgFilePath.c_str(), assemblyStatus.assembledProgram);
                }
            }
            else
            {
                for (auto const &errMsg : assemblyStatus.errors)
                {
                    cerr << errMsg << std::endl;
                }
                ret = RET_ERR;
            }
        }
        else
        {
            usage(argv[0]);
            ret = RET_ERR;
        }
    }


    return ret;
}
