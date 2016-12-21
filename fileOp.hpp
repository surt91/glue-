#pragma once

#include <iostream>
#include <fstream>

#include "Histogram.hpp"

void write_out(std::string file, std::string text);

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

std::string getNthWord(const std::string &line, int n);

template<class T>
Histogram histogramFromStream(T &instream, int num_bins, double lower, double upper, int column=0, int skip=0)
{
    Histogram h(num_bins, lower, upper);

    int ctr = 0;
    while(instream.good())
    {
        std::string line = getNextLine(instream);
        if(ctr++ < skip)
            continue;
        if(line.empty())
            continue;
        std::string word = getNthWord(line, column);

        double number = std::stod(word);
        h.add(number);
    }
    return h;
}

template<class T>
void bordersFromStream(T &instream, double &lower, double &upper, int column=0, int skip=0)
{
    int ctr = 0;
    while(instream.good())
    {
        std::string line = getNextLine(instream);
        if(ctr++ < skip)
            continue;
        if(line.empty())
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

bool has_suffix(const std::string &str, const std::string &suffix);
