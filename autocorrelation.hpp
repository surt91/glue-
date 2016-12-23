#pragma once

#include <vector>
#include <cmath>

#include "Logging.hpp"
#include "kiss_fft.h"
#include "stat.hpp"

double autocorrelationTime(const std::vector<double> &timeseries);
