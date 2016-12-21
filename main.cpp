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

    if(!o.border_path_vector.empty())
    {
        LOG(LOG_INFO) << "determine borders from files (" << o.border_path_vector.size() << " given)";

        double lower = 1e300;
        double upper = 1e-300;

        #pragma omp parallel for reduction(min:lower), reduction(max:upper)
        for(size_t i=0; i<o.border_path_vector.size(); ++i)
        {
            auto &file = o.border_path_vector[i];
            LOG(LOG_INFO) << "read: " << file;
            // unzip, if it ends on .gz
            if(has_suffix(file, ".gz"))
            {
                igzstream is(file.c_str());
                bordersFromStream(is, lower, upper, o.column);
            }
            // otherwise try to read it as a normal file
            else
            {
                std::ifstream is(file.c_str());
                bordersFromStream(is, lower, upper, o.column);
            }
        }
        o.lowerBound = lower;
        o.upperBound = upper;
        LOG(LOG_INFO) << "use range [" << o.lowerBound << ", " << o.upperBound<< "]";
    }

    std::vector<Histogram> histograms(o.data_path_vector.size());

    #pragma omp parallel for
    for(size_t i=0; i<o.data_path_vector.size(); ++i)
    {
        auto &file = o.data_path_vector[i];
        // unzip, if it ends on .gz
        if(has_suffix(file, ".gz"))
        {
            igzstream is(file.c_str());
            histograms[i] = histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound, o.column);
        }
        // otherwise try to read it as a normal file
        else
        {
            std::ifstream is(file.c_str());
            histograms[i] = histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound, o.column);
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
