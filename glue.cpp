#include "glue.hpp"

double correct_bias(double s, double theta, double p_s)
{
    return std::exp(s/theta) * p_s;
}

Histogram glueHistograms(std::vector<Histogram> hists, std::vector<double> thetas, int threshold=0)
{
    std::vector<std::vector<double>> corrected_data;
    for(size_t i=0; i<hists.size(); ++i)
    {
        const auto data = hists[i].get_data();
        const auto centers = hists[i].centers();
        const double theta = thetas[i];

        std::vector<double> corrected;
        for(size_t j=0; j<data.size(); ++j)
            corrected.push_back(correct_bias(centers[j], theta, data[j]));

        corrected_data.push_back(corrected);
    }

    for(size_t i=1; i<hists.size(); ++i)
    {
        const auto data1 = corrected_data[i-1];
        const auto data2 = corrected_data[i];
        const auto centers1 = hists[i-1].centers();
        const auto centers2 = hists[i].centers();

        std::vector<double> Z;
        // get region of overlap
        // assumes temperatures are ordered
        for(size_t j=0; j<data1.size(); ++j)
        {
            if(data1[j] != 0 && data2[j] != 0)
            {
                Z.push_back(data1[j]-data2[j]);
            }
        }
    }

    // FIXME: not ready
    if(threshold)
    {
    }
    return hists[0];
}
