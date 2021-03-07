#include "ReflectionMap.hpp"

using std::sin;
using std::cos;


Vec ReflectionMap::incidentIlluminationDirection() const {
	const double sin_polarAngle = sin(polarAngle);
	return Vec{
		sin_polarAngle	* cos(azimuthalAngle),
		sin_polarAngle	* sin(azimuthalAngle),
		cos(polarAngle)
	};
}