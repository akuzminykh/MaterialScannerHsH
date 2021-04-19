#pragma once

#include <vector>
#include <string>
#include "ReflectionMap.hpp"
#include "NormalMap.hpp"


std::vector<std::string> listItems(const std::string& dir);
std::vector<ReflectionMap> readDataset(const std::string& dir);
ReflectionMap readIntensities(const std::string& file);
void writeNormalMap(const NormalMap& normalMap, const std::string& file);