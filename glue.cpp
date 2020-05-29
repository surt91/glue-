#include "glue.hpp"

/** Correct the bias introduced by the temperature based Metropolis sampling.
 *
 *  Since all values are logarithms, this is numerically stable.
 *
 * \f[ \frac{1}{Z_\Theta} P(S) = e^{S/\Theta} P_{\Theta} \f]
 *
 * \param s         value of the observable
 * \param theta     temperature where s were sampled
 * \param p_theta   (unnormalized) probability to find s at theta
 */
double correct_bias(double s, double theta, double p_theta)
{
    // std::exp(s/theta) * p_theta;, but calculate only logarithms
    return s/theta + std::log(p_theta);
}

void write_to_stream(std::ofstream &oss, const std::vector<double> &centers, const std::vector<double> &data)
{
    for(size_t j=0; j<data.size(); ++j)
        if(std::isfinite(data[j]))
            oss << centers[j] << " " << data[j] << "\n";
    oss << "\n";
}


/** Determine the normalization constants Z
 *
 *  These can be used to correct the biased distributions by shifting according to
 *  \f[ \frac{1}{Z_\Theta} P(S) = e^{S/\Theta} P_{\Theta} \f]
 *
 * \param hists input histograms, used to determine weights
 * \param corrected_data right hand side of above equation for every histogram
 * \param threshold if a bin has fewer entries, ignore it
 * \param weighted should the means be weighted
 * \param thetas used temperatures, only for user output
 */
std::vector<double> determineZ(const std::vector<Histogram> &hists, const std::vector<std::vector<double>> &corrected_data, int threshold, bool weighted, const std::vector<double> &thetas)
{
    std::vector<double> Zs(hists.size(), 0);
    for(size_t i=1; i<hists.size(); ++i)
    {
        // TODO: do not only use successive histograms for glueing, but all 
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
                if(weighted)
                    weight.push_back(std::min(hists[i].get_data()[j], hists[i].get_data()[j]));
                else // equal weight, if data originates from WL
                    weight.push_back(1);
            }
        }
        double meanZ;
        if(Z.size())
        {
            meanZ = weighted_mean(Z, weight);
        }
        else if(weighted)
        {
            meanZ = 0;
            LOG(LOG_WARNING) << "no overlap between T = " << thetas[i-1] << " and T = " << thetas[i];
        }
        else
        {
            meanZ = 0;
            LOG(LOG_WARNING) << "no overlap between [" << (i-1)
                <<  "] and [" << i <<  "]";
        }

        Zs[i] = Zs[i-1] + meanZ;
    }
    return Zs;
}

/** Determine the normalization constants \f$ Z_\Theta \f$.
 *
 *  The histograms need to have the same borders.
 *
 *  \param hists        vector of histograms to glue
 *  \param thetas       temperatures for each histogram such that hists[i] is sampled at thetas[i]
 *  \param threshold    how many entries should a bin have to be considered for determination of \f$ Z_\Theta \f$
 */
Histogram glueHistograms(const std::vector<Histogram> &hists, const std::vector<double> thetas, int threshold, const GnuplotData gp)
{
    std::string histname = gp.hist_name;
    std::string correctedname = gp.corrected_name;
    std::string gluedname = gp.glued_name;
    std::string finishedname = gp.finished_name;
    std::ofstream osHist(histname);
    std::ofstream osCorrected(correctedname);
    std::ofstream osGlued(gluedname);
    std::ofstream osFinished(finishedname);
    for(auto &h : hists)
        osHist << h.ascii_table() << "\n";

    // perform weighting only for temperature based sheme
    const bool weighted = !thetas.empty();

    // centers of all histograms should be equal
    const auto &centers = hists[0].centers();

    std::vector<std::vector<double>> corrected_data;
    // if no temperatures are given, do just merge the histograms
    if(weighted)
    {
        for(size_t i=0; i<hists.size(); ++i)
        {
            const auto &data = hists[i].get_data();
            const double &theta = thetas[i];

            std::vector<double> corrected;
            for(size_t j=0; j<data.size(); ++j)
                corrected.push_back(correct_bias(centers[j], theta, data[j]));

            write_to_stream(osCorrected, centers, corrected);

            corrected_data.emplace_back(std::move(corrected));
        }
    }
    else
    {
        for(size_t i=0; i<hists.size(); ++i)
        {
            const auto &data = hists[i].get_data();

            std::vector<double> corrected;
            // if we get histograms, we assume that they originate from WL
            // we will replace zeros by nan for nicer plots
            // if we want to evaluate simple sampling, we need to pass
            // infinite theta
            for(size_t j=0; j<data.size(); ++j)
                if(data[j] <= 0)
                    corrected.push_back(std::nan(""));
                else
                    corrected.push_back(data[j]);

            write_to_stream(osCorrected, centers, corrected);

            corrected_data.emplace_back(std::move(corrected));
        }
    }

    std::vector<double> Zs = determineZ(hists, corrected_data, threshold, weighted, thetas);

    for(size_t i=1; i<hists.size(); ++i)
        for(size_t j=0; j<corrected_data[i].size(); ++j)
            corrected_data[i][j] += Zs[i];

    for(size_t i=0; i<hists.size(); ++i)
        write_to_stream(osGlued, centers, corrected_data[i]);

    std::vector<double> unnormalized_data;
    if(hists.size() > 1)
    {
        // get data from all histograms and calculate weighted means
        for(size_t j=0; j<corrected_data[0].size(); ++j)
        {
            double total = 0;
            double total_weight = 0;
            if(weighted)
            {
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
            }
            else // we have WL, we can not weight this
            {
                for(size_t i=0; i<hists.size(); ++i)
                    if(std::isfinite(corrected_data[i][j]))
                    {
                        total += corrected_data[i][j];
                        ++total_weight;
                    }
            }

            unnormalized_data.push_back(total/total_weight);
        }
    }
    else
    {
        // if we have only one histogram, we do not need to average
        unnormalized_data = corrected_data[0];
    }

    std::vector<double> expData;
    std::vector<double> expCenters;
    // find largest element and normalize all to avoid numerical problems
    double m = -1e300;
    for(auto i : unnormalized_data)
        if(i>m)
            m = i;

    LOG(LOG_DEBUG) << "max: " << m;

    for(size_t i=0; i<unnormalized_data.size(); ++i)
    {
        // avoid nan, would result in a nan area
        // this should also work with -ffast-math
        if(unnormalized_data[i] > -1e300 && unnormalized_data[i] < 1e300)
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

    osFinished << out.ascii_table();

    return out;
}

/** Takes a bootstrap sample of histograms, glues them and returns
 * a table with an error estimate obtained from bootstrapping.
 */
std::string bootstrapGlueing(const std::vector<std::vector<Histogram>> &histograms, const std::vector<double> thetas, int threshold)
{
    int num_bins = histograms[0][0].get_num_bins();
    int n_sample = histograms.size();
    std::vector<double> centers;
    std::vector<std::vector<double>> tmp(num_bins);

    centers = histograms[0][0].centers();
    for(int j=0; j<n_sample; ++j)
    {
        Histogram h = glueHistograms(histograms[j], thetas, threshold);
        for(int i=0; i<num_bins; ++i)
            tmp[i].push_back(h.at(i));
    }

    std::vector<double> values(num_bins);
    std::vector<double> errors(num_bins);
    for(int i=0; i<num_bins; ++i)
    {
        values[i] = mean(tmp[i]);
        errors[i] = sdev(tmp[i]);
    }

    std::stringstream table;
    table << "# centers count error\n";
    for(int i=0; i<num_bins; ++i)
    {
        table << centers[i] << " " << values[i] << " " << errors[i] << "\n";
    }

    return table.str();
}
