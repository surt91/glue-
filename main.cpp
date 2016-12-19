#include <random>
#include <vector>
#include <algorithm>

#include <fstream>
#include "gzstream/gzstream.h"

#include "Cmd.hpp"
#include "fileOp.hpp"


int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    if(o.border_path_vector.empty() && o.lowerBound < 0 && o.upperBound < 0)
    {
        o.border_path_vector = o.data_path_vector;
    }

    if(!o.border_path_vector.empty())
    {
        LOG(LOG_INFO) << "determine borders from files";
        o.lowerBound = 1e300;
        o.upperBound = -1e300;
        // unzip, if it ends on .gz
        for(auto file : o.border_path_vector)
        {
            if(has_suffix(file, ".gz"))
            {
                igzstream is(file.c_str());
                bordersFromStream(is, o.lowerBound, o.upperBound);
            }
            // otherwise try to read it as a normal file
            else
            {
                std::ifstream is(file.c_str());
                bordersFromStream(is, o.lowerBound, o.upperBound);
            }
            LOG(LOG_INFO) << "use range [" << o.lowerBound << ", " << o.upperBound<< "]";
        }
    }

    std::vector<Histogram> histograms;
    for(auto &file : o.data_path_vector)
    {
        // unzip, if it ends on .gz
        if(has_suffix(file, ".gz"))
        {
            igzstream is(file.c_str());
            histograms.emplace_back(histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound));
        }
        // otherwise try to read it as a normal file
        else
        {
            std::ifstream is(file.c_str());
            histograms.emplace_back(histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound));
        }
    }

    for(auto h : histograms)
        std::cout << h.ascii_table();
}
