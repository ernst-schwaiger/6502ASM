#pragma once

#include <string>
#include <vector>
#include <algorithm>

namespace get_opt
{
    using Option = struct
    {
        char opt;
        std::string optarg;
    };

    namespace get_opt::internal
    {
        std::string::const_iterator findOpt(char opt, std::string const &opts)
        {
            return std::find(begin(opts), end(opts), opt);
        }

        bool isValid(std::string::const_iterator optIt, std::string const &opts)
        {
            return (optIt != end(opts));
        }

        bool hasArg(std::string::const_iterator optIt, std::string const &opts)
        {
            std::string::const_iterator argIt = ++optIt;
            return (isValid(argIt, opts) && (*argIt == ':'));
        }

        std::string getOptArg(std::string::const_iterator optIt, std::string const &opts, std::string const &arg, char const *pNextArg, bool &consumedNextArg)
        {
            std::string ret = "";
            consumedNextArg = false;
            if (hasArg(optIt, opts)) // according to the optstring, the option comes with a parameter
            {
                if (arg.length() > 2) // fallback, arg follows option, e.g. "-xarg"
                {
                    ret = arg.substr(2);
                }
                else if (pNextArg != nullptr && (pNextArg[0]) != '-') // arg is next string, e.g. "-x arg"
                {
                    ret = pNextArg;
                    consumedNextArg = true;
                }
            }

            return ret;
        }
    }    

    std::vector<Option> getopt(int argc, char **argv, std::string const &opts)
    {
        std::vector<Option> options;

        for (int idx = 1; idx < argc; idx++)
        {
            std::string arg(argv[idx]);

            if ((arg.length() >= 2) && arg.at(0) == '-')
            {
                auto optIt = get_opt::internal::findOpt(arg.at(1), opts);
                char opt = get_opt::internal::isValid(optIt, opts) ? *optIt: '?';

                char const *pNextArg = (idx + 1 < argc) ? argv[idx + 1] : nullptr;
                bool consumedNextArg = false;
                std::string optArg = get_opt::internal::getOptArg(optIt, opts, arg, pNextArg, consumedNextArg);                
                options.emplace_back(Option{*optIt, optArg});

                // we consumed the argument after the option
                if (consumedNextArg)
                {
                    idx++;
                }
            }
            else
            {
                options.emplace_back(Option{'!', arg});
            }
        }

        return options;
    }
}
