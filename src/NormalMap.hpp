#pragma once

#include "util.hpp"

#include <vector>
#include <cassert>


bool allNormalized(const std::vector<double>& data);


struct NormalMap {
	// Stores all (x, y, z) one after another.
	// All values are within the interval [-1, 1].
	const std::vector<double> normalsData;

	const std::size_t width;
	const std::size_t height;

	NormalMap(
		const std::size_t width,
		const std::size_t height,
		const std::vector<double>& normalsData)
		:
		width(width), height(height), normalsData(normalsData)
	{
		assert(normalsData.size() == width * height * 3);
		assert(allNormalized(normalsData));
	}

};