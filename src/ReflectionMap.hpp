#pragma once

#include "Mat.hpp"
#include "Vec.hpp"
#include "util.hpp"

#include <vector>
#include <cassert>


// Represents an image taken with the material-scanner
// with one lamp turned on. The attributes azimuthalAngle
// and polarAngle describe the direction to the lamp from
// the center of the ground.
struct ReflectionMap {
	// All intensities are within the interval [0, 1].
	const std::vector<double> intensities;

	const std::size_t width;
	const std::size_t height;

	// Search for "spherical coordinate system".
	const double azimuthalAngle;
	const double polarAngle;

	ReflectionMap(
		const std::size_t width,
		const std::size_t height,
		const std::vector<double>& intensities,
		const double azimuthalAngle,
		const double polarAngle)
		:
		width(width),
		height(height),
		intensities(intensities),
		azimuthalAngle(azimuthalAngle),
		polarAngle(polarAngle)
	{
		assert(intensities.size() == width * height);
		assert(allSmallerEqualTo(intensities, 1.0));
	}


	// If you want to iterate all, then rather use a pointer/iterator.
	double at(const int x, const int y) const {
		assert(x < width && x >= 0);
		assert(y < height && y >= 0);
		return intensities[y * width + x];
	}


	// Returns the direction to the light-source.
	Vec incidentIlluminationDirection() const;
};