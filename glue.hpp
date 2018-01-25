#pragma once

#include <vector>
#include <cmath>

#include "Histogram.hpp"
#include "stat.hpp"
#include "gnuplot.hpp"

/**
 * Glues multiple histograms together.
 *
 * The bins of all histograms need to be the same.
 */
Histogram glueHistograms(const std::vector<Histogram> &hists, const std::vector<double> thetas=std::vector<double>(), int threshold=0, const GnuplotData=GnuplotData());
std::string bootstrapGlueing(const std::vector<std::vector<Histogram>> &histograms, const std::vector<double> thetas=std::vector<double>(), int threshold=0);
