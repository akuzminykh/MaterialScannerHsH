#pragma once

#include <vector>
#include <string>


bool nearlyEqual(const double a, const double b);
double degreesToRadians(const double deg);
bool allSmallerEqualTo(const std::vector<double>& data, const double lim);
std::vector<std::string> splitBy(const std::string& s, const char d);