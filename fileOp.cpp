#include "fileOp.hpp"

Histogram histogramFromFile(std::string filename, int num_bins, double lower, double upper, int column)
{
    Histogram h(num_bins, lower, upper);
    igzstream is(filename.c_str());
    while(is.good())
    {
        std::string line;
        std::getline(is, line);
        if(line.empty() || line[0] == '#')
            continue;

        std::string word;
        std::stringstream ss;
        ss.str(line);
        int ctr = 0;
        do
        {
            std::getline(ss, word, ' ');
            ++ctr;
        } while(ctr <= column);

        double number = std::stod(word);
        h.add(number);
    }
    return h;
}
