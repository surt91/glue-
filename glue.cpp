#include "glue.hpp"

double correct_bias(double s, double theta, double p_s)
{
    // std::exp(s/theta) * p_s;, but calculate only logarithms
    return s/theta + std::log(p_s);
}

Histogram glueHistograms(std::vector<Histogram> hists, std::vector<double> thetas, int threshold)
{
    std::string histname = "hist.dat";
    std::string correctedname = "corrected.dat";
    std::string gluedname = "glued.dat";
    std::ofstream osHist(histname);
    std::ofstream osCorrected(correctedname);
    std::ofstream osGlued(gluedname);
    for(auto &h : hists)
        osHist << h.ascii_table();

    // centers of all histograms should be equal
    const auto &centers = hists[0].centers();

    std::vector<std::vector<double>> corrected_data;
    for(size_t i=0; i<hists.size(); ++i)
    {
        const auto &data = hists[i].get_data();
        const double &theta = thetas[i];

        std::vector<double> corrected;
        for(size_t j=0; j<data.size(); ++j)
            corrected.push_back(correct_bias(centers[j], theta, data[j]));

        corrected_data.push_back(corrected);

        for(size_t j=0; j<data.size(); ++j)
            osCorrected << centers[j] << " " << corrected[j] << "\n";
    }

    std::vector<double> Zs(hists.size(), 0);
    for(size_t i=1; i<hists.size(); ++i)
    {
        const auto &count1 = hists[i-1].get_data();
        const auto &count2 = hists[i].get_data();
        const auto &data1 = corrected_data[i-1];
        const auto &data2 = corrected_data[i];

        std::vector<double> Z;
        std::vector<double> weight; // how to weight the data, to get the mean of Z
        // get region of overlap
        // assumes temperatures are ordered
        for(size_t j=0; j<data1.size(); ++j)
        {
            if(count1[j] > threshold && count2[j] > threshold)
            {
                Z.push_back(data1[j]-data2[j]);
                // weight the Z: more weight, if both datasets have many entries
                weight.push_back(std::min(hists[i].get_data()[j], hists[i].get_data()[j]));
            }
        }
        double meanZ;
        if(Z.size())
            meanZ = weighted_mean(Z, weight);
        else
            meanZ = 0;

        Zs[i] = Zs[i-1] + meanZ;
    }
    // correct data
    for(size_t i=1; i<hists.size(); ++i)
        for(size_t j=0; j<corrected_data[i].size(); ++j)
        {
            corrected_data[i][j] += Zs[i];
            osGlued << centers[j] << " " << corrected_data[i][j] << "\n";
        }

    std::vector<double> unnormalized_data;
    // get data from all histograms and calculate weighted means
    for(size_t j=0; j<corrected_data[0].size(); ++j)
    {
        double total = 0;
        double total_weight = 0;
        for(size_t i=0; i<hists.size(); ++i)
        {
            double value = corrected_data[i][j];
            double weight = hists[i].get_data()[j];
            if(weight > threshold)
            {
                total += value * weight;
                total_weight += weight;
            }
        }

        unnormalized_data.push_back(total/total_weight);
    }

    std::vector<double> expData;
    std::vector<double> expCenters;
    // find largest element and normalize all to avoid numerical problems
    double m = 0;
    for(auto i : unnormalized_data)
        if(i>m)
            m = i;

    for(size_t i=0; i<unnormalized_data.size(); ++i)
    {
        // avoid nan, would result in a nan area
        // this should also work with -ffast-math
        if(unnormalized_data[i] > 1e-300 && unnormalized_data[i] < 1e300)
        {
            expData.push_back(std::exp(unnormalized_data[i]-m));
            expCenters.push_back(hists[0].centers()[i]);
        }
    }
    double logArea = m + std::log(trapz(expCenters, expData));
    LOG(LOG_DEBUG) << "area: " << logArea;

    Histogram out(hists[0].borders());
    for(size_t i=0; i<unnormalized_data.size(); ++i)
        out.at(i) = unnormalized_data[i] - logArea;

    return out;
}
