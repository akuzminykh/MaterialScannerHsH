#pragma once

#include <vector>
#include <string>
#include "ReflectionMap.hpp"
#include "NormalMap.hpp"


std::vector<std::string> listItems(const std::string& dir);
std::vector<ReflectionMap> readDataset(const std::string& dir);
ReflectionMap readIntensities(const std::string& file);
NormalMap photometricStereo(const std::vector<ReflectionMap>& dataset, const double correctionFactor);
Mat matRotX(const double factor, const int y, const size_t height, const size_t width);
Mat matRotY(const double factor, const int x, const size_t width);
void writeNormalMap(const NormalMap& normalMap, const std::string& file);