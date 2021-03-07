#include "NormalMap.hpp"
using std::vector;
using std::size_t;

#include <cmath>
using std::sqrt;

#include <cassert>


bool allNormalized(const vector<double>& data) {
	const size_t n = data.size();
	assert(n % 3 == 0);

	const double* x_p = &data[0];
	const double* y_p = x_p + 1;
	const double* z_p = y_p + 1;
	const double* end = x_p + (n);

	for (; x_p != end; x_p += 3, y_p += 3, z_p += 3) {
		const double x = *x_p;
		const double y = *y_p;
		const double z = *z_p;
		const double length = sqrt(x * x + y * y + z * z);

		if (!nearlyEqual(1.0, length)) {
			return false;
		}
	}
	return true;
}