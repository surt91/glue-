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
Histogram glueHistograms(std::vector<Histogram> hists, std::vector<double> thetas, int threshold=0);
