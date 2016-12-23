#pragma once

#include <iostream>
#include <fstream>

#include "Histogram.hpp"
#include "autocorrelation.hpp"

bool has_suffix(const std::string &str, const std::string &suffix);

void write_out(std::string file, std::string text);
std::string getNthWord(const std::string &line, int n);

/** Get the next line from an input stream.
 *
 *  \tparam T           type of the input stram
 *  \param instream     reference to the input stream to read from
 */
template<class T>
std::string getNextLine(T &instream)
{
    std::string line;
    while(instream.good())
    {
        std::getline(instream, line);
        if(line.empty() || line[0] == '#')
            continue;
        break;
    }
    return line;
}

/** Create a histogram from an input stream (of string).
 *
 *  \tparam T           type of the input stram
 *  \param instream     reference to the input stream to read from
 *  \param num_bins     how many bins should the histogram have
 *  \param lower        lower border of the histogram
 *  \param upper        upper border of the histogram
 *  \param column       in which column of the input stream is the data
 *  \param skip         skip the first lines of the input stream
 */
template<class T>
Histogram histogramFromStream(T &instream, int num_bins, double lower, double upper, int column=0, int skip=0, int step=1)
{
    Histogram h(num_bins, lower, upper);

    int ctr = 0;
    while(instream.good())
    {
        std::string line = getNextLine(instream);
        if(line.empty() || line[0] == '#')
            continue;
        if(ctr++ < skip)
            continue;
        if((ctr-skip) % step)
            continue;
        std::string word = getNthWord(line, column);

        double number = std::stod(word);
        h.add(number);
    }
    return h;
}

/** Obtain the largest and smallest values from an input stream (of string).
 *
 *  \tparam         T            type of the input stram
 *  \param          instream     reference to the input stream to read from
 *  \param[in,out]  lower        smallest value of the input stream or given value
 *  \param[in,out]  upper        largest value of the input stream or given value
 *  \param          column       in which column of the input stream is the data
 *  \param          skip         skip the first lines of the input stream
 */
template<class T>
void bordersFromStream(T &instream, double &lower, double &upper, int column=0, int skip=0, int step=1)
{
    int ctr = 0;
    while(instream.good())
    {
        std::string line = getNextLine(instream);
        if(line.empty() || line[0] == '#')
            continue;
        if(ctr++ < skip)
            continue;
        if((ctr-skip) % step)
            continue;
        std::string word = getNthWord(line, column);

        double number = std::stod(word);

        if(number < lower)
            lower = number;
        if(number > upper)
            upper = number;
    }
    lower -= 0.05*(upper-lower);
    upper += 0.05*(upper-lower);
}

template<class T>
double tauFromStream(T &instream, int column=0, int skip=0)
{
    int ctr = 0;
    std::vector<double> sample;
    // use this many samples to determine the autocorrelation time
    //~ int num_samples = 256;
    int num_samples = 1024;
    double tau;
    while(instream.good())
    {
        std::string line = getNextLine(instream);
        if(ctr++ < skip)
            continue;
        if(line.empty())
            continue;
        std::string word = getNthWord(line, column);

        double number = std::stod(word);
        sample.push_back(number);

        if(ctr-skip == num_samples)
        {
            tau = autocorrelationTime(sample);
            // if the autocorrelation time is of the same order of the
            // number of samples, try again with more samples
            if(tau > num_samples/10.)
                num_samples *= 2;
            else
                break;
        }
    }
    return tau;
}
