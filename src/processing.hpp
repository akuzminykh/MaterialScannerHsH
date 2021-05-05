#pragma once

#include "Mat.hpp"
#include <vector>
using std::vector;

double distanceBetweenPixelPair(const size_t& length);
double calcSizeRatio(const size_t& height, const size_t& width);
vector<Mat> correctionMatricesX(const size_t& height, const double scaledCorrectionFactor);
vector<Mat> correctionMatricesY(const size_t& width, const double correctionFactor);

