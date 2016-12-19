#include "fileOp.hpp"

std::string getNthWord(const std::string &line, int n)
{
    std::string word;
    std::stringstream ss;
    ss.str(line);
    int ctr = 0;
    do
    {
        std::getline(ss, word, ' ');
        ++ctr;
    } while(ctr <= n);
    return word;
}

// http://stackoverflow.com/a/20446239/1698412
bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}
