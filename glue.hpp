#pragma once

#include <vector>

#include "Histogram.hpp"

/**
 * Glues multiple histograms together.
 *
 * The bins of all histograms need to be the same.
 */
Histogram glueHistograms(std::vector<Histogram> hists, int threshold=0);
