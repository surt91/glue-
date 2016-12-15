#pragma once

#include <iostream>
#include <fstream>

#include "gzstream/gzstream.h"
#include "Histogram.hpp"

Histogram histogramFromFile(std::string filename, int num_bins, double lower, double upper, int column=0);
