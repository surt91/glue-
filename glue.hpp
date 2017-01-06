#pragma once

#include <vector>
#include <cmath>

#include "Histogram.hpp"
#include "stat.hpp"

/**
 * Glues multiple histograms together.
 *
 * The bins of all histograms need to be the same.
 */
Histogram glueHistograms(const std::vector<Histogram> &hists, const std::vector<double> thetas=std::vector<double>(), int threshold=0);
std::string bootstrapGlueing(const std::vector<std::vector<Histogram>> &histograms, const std::vector<double> thetas=std::vector<double>(), int threshold=0);
