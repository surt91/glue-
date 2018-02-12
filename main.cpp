#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

#include <fstream>
#include "gzstream/gzstream.h"

#include "Cmd.hpp"
#include "fileOp.hpp"
#include "glue.hpp"
#include "autocorrelation.hpp"
#include "gnuplot.hpp"

/**
 * \mainpage glue++
 *
 *
 */

/** If the borders have their default values ([0, 0]), obtain
 * tight borders from the files
 */
void updateBorders(Cmd &o)
{
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

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
            const auto &file = o.border_path_vector[i];
            LOG(LOG_DEBUG) << "read: " << file;

            if(isHistogramFile(file))
            {
                Histogram h(file);
                lower = std::min(lower, h.borders().front());
                upper = std::max(upper, h.borders().back());
                o.num_bins = h.get_num_bins();
            }
            else
            {
                // igzstream can also read plain files
                igzstream is(file.c_str());
                bordersFromStream(is, lower, upper, o.column, o.skip);
            }
        }
        o.lowerBound = lower;
        o.upperBound = upper;
        LOG(LOG_INFO) << "use range [" << o.lowerBound << ", " << o.upperBound<< "]";
    }

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    LOG(LOG_TIMING) << "determining borders " << time_span.count() << "s";
}

/** test if all borders are the same
 *
 * This is important, since the glueing needs them to be the same.
 * If we load finished histograms, we do not have control over
 * the borders.
 */
bool sameBorders(std::vector<Histogram> histograms)
{
    const std::vector<double> &reference = histograms[0].borders();
    for(size_t i=1; i<histograms.size(); ++i)
    {
        const std::vector<double> &cmp = histograms[i].borders();
        if(reference.size() != cmp.size())
            return false;
        for(size_t j=0; j<reference.size(); ++j)
            if(reference[j] != cmp[j])
                return false;

    }
    return true;
}

/** Create Histograms from the specified files.
 *
 * If the files are already histograms or if there is an already
 * calculated cached histogram, use those, otherwise generate the
 * histograms from raw data.
 */
std::vector<Histogram> createHistograms(const Cmd &o)
{
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    std::vector<Histogram> histograms(o.data_path_vector.size());

    #pragma omp parallel for schedule(dynamic,1)
    for(size_t i=0; i<o.data_path_vector.size(); ++i)
    {
        int step = o.step;
        const auto &file = o.data_path_vector[i];
        LOG(LOG_DEBUG) << "read: " << file;

        // first test, if the file already contains a histogram (is it shorter than 5 lines)
        // next test, if there is a file with the same name but ending .hist
        // then test, if o.lower/upper and num bins are the same, if not discard
        // else evaluate the datafile

        Histogram tmp_hist;
        if(isHistogramFile(file) && !o.force)
            tmp_hist = Histogram(file);
        else if(fileReadable(file+".hist") && !o.force)
            tmp_hist = Histogram(file+".hist");

        if(!o.force
        && tmp_hist.get_num_bins() == o.num_bins
        && std::abs(o.lowerBound - tmp_hist.borders().front()) < 1e-1
        && std::abs(o.upperBound - tmp_hist.borders().back()) < 1e-1
        )
        {
            histograms[i] = std::move(tmp_hist);
            LOG(LOG_DEBUG) << "load histogram for " << file;
        }
        else
        {
            LOG(LOG_DEBUG) << "calculate histogram for " << file;

            if(!o.step)
            {
                // igzstream can also read plain files
                igzstream is(file.c_str());
                double tau = tauFromStream(is, o.column, o.skip);
                step = std::ceil(2*tau);
            }

            // I can not reset the igzstream somehow
            igzstream is(file.c_str());
            LOG(LOG_DEBUG) << file << ": t_eq = " << o.skip << ", tau = " << step;
            histograms[i] = histogramFromStream(is, o.num_bins, o.lowerBound, o.upperBound, o.column, o.skip, step);

            // save histogram to load it the next time ~ cache
            histograms[i].writeToFile(file + ".hist");
        }
    }

    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    LOG(LOG_TIMING) << "reading files and creating histograms " << time_span.count() << "s";

    return histograms;
}

std::vector<std::vector<Histogram>> bootstrapHistograms(Cmd &o, int n_sample, int seed=0)
{
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<Histogram>> histograms(n_sample);
    for(int i=0; i<n_sample; ++i)
        histograms[i].resize(o.data_path_vector.size());

    #pragma omp parallel for schedule(dynamic,1)
    for(size_t i=0; i<o.data_path_vector.size(); ++i)
    {
        // this is dumb, but will generate the same random numbers
        // independent of parallelism and it is good enough for bootstrapping
        std::mt19937 rng(seed+i);

        const auto &file = o.data_path_vector[i];
        LOG(LOG_DEBUG) << "read: " << file;

        if(!o.step)
        {
            // igzstream can also read plain files
            igzstream is(file.c_str());
            double tau = tauFromStream(is, o.column, o.skip);
            o.step = std::ceil(2*tau);
        }

        // I can not reset the igzstream somehow
        igzstream is(file.c_str());
        LOG(LOG_DEBUG) << file << ": t_eq = " << o.skip << ", tau = " << o.step;
        std::vector<double> numbers = vectorFromStream(is, o.column, o.skip, o.step);
        size_t num_numbers = numbers.size();
        std::uniform_int_distribution<int> uniform(0, num_numbers-1);

        for(int j=0; j<n_sample; ++j)
        {
            Histogram h(o.num_bins, o.lowerBound, o.upperBound);
            for(size_t k=0; k<num_numbers; ++k)
            {
                h.add(numbers[uniform(rng)]);
            }
            histograms[j][i] = h;
        }
    }

    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    LOG(LOG_TIMING) << "reading files and bootstrapping histograms " << time_span.count() << "s";

    return histograms;
}

int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    updateBorders(o);
    GnuplotData gp(o);

    if(!o.bootstrap)
    {
        std::vector<Histogram> histograms = createHistograms(o);

        std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();

        Histogram h = glueHistograms(histograms, o.thetas, o.threshold, gp);
        write_out(o.output, h.ascii_table());

        std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t4 - t3);
        LOG(LOG_TIMING) << "glueing histograms " << time_span.count() << "s";
    }
    else
    {
        std::vector<std::vector<Histogram>> histogramSamples = bootstrapHistograms(o, o.threshold);

        std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();

        std::string table = bootstrapGlueing(histogramSamples, o.thetas, o.threshold);
        write_out(o.output, table);

        std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t4 - t3);
        LOG(LOG_TIMING) << "glueing histograms " << time_span.count() << "s";
    }

    write_gnuplot_quality(gp);
}
