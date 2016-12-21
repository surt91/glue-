#include <random>
#include <vector>
#include <algorithm>

#include <fstream>
#include "gzstream/gzstream.h"

#include "Cmd.hpp"
#include "fileOp.hpp"
#include "glue.hpp"


int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    // if no borders are given, determine the borders from the first
    // and the last of the given data files
    if(o.border_path_vector.empty() && o.lowerBound < 0 && o.upperBound < 0)
    {
        o.border_path_vector.push_back(o.data_path_vector.front());
        if(o.data_path_vector.size() > 1)
            o.border_path_vector.push_back(o.data_path_vector.back());
    }

    // TODO: read the files in parallel with openMP
    if(!o.border_path_vector.empty())
    {
        LOG(LOG_INFO) << "determine borders from files (" << o.border_path_vector.size() << " given)";
        o.lowerBound = 1e300;
        o.upperBound = -1e300;
        // unzip, if it ends on .gz
        for(auto &file : o.border_path_vector)
        {
            LOG(LOG_INFO) << "read: " << file;
            if(has_suffix(file, ".gz"))
            {
                igzstream is(file.c_str());
                bordersFromStream(is, o.lowerBound, o.upperBound, o.column);
            }
            // otherwise try to read it as a normal file
            else
            {
                std::ifstream is(file.c_str());
                bordersFromStream(is, o.lowerBound, o.upperBound, o.column);
            }
        }
        LOG(LOG_INFO) << "use range [" << o.lowerBound << ", " << o.upperBound<< "]";
    }

    // TODO: read the files in parallel with openMP
    std::vector<Histogram> histograms;
    for(auto &file : o.data_path_vector)
    {
        // unzip, if it ends on .gz
        if(has_suffix(file, ".gz"))
        {
            igzstream is(file.c_str());
            histograms.emplace_back(histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound, o.column));
        }
        // otherwise try to read it as a normal file
        else
        {
            std::ifstream is(file.c_str());
            histograms.emplace_back(histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound, o.column));
        }
    }

    if(o.data_path_vector.size() > 1)
    {
        Histogram h = glueHistograms(histograms, o.thetas, 100);
        std::cout << h.ascii_table();
    }
    else
    {
        for(auto &h : histograms)
            std::cout << h.ascii_table();
    }
}
