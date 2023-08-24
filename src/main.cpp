#include "ASM6502.h"

using namespace std;
using namespace asm6502;

static int const RET_OK = 0;
static int const RET_ERR = 1;

int main(int argc, char *argv[])
{
    int ret = RET_OK;

    if (argc == 2)
    {
        AssemblyStatus assemblyStatus = assembleFile(argv[1]);

		for (auto const &errMsg : assemblyStatus.errors)
		{
			cerr << errMsg << std::endl;
		}
    }
    else
    {
        cerr << "Usage: " << argv[0] << " <asmfile>" << endl;
        ret = RET_ERR;
    }

    return ret;
}
