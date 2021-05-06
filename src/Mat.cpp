#include "Mat.hpp"
using std::vector;


Mat Mat::transpose() const {
	vector<double> v;
	v.reserve(M * N);
	for (int j = 0; j < N; ++j) { // iterate/add in column-major order ...
		for (int i = 0; i < M; ++i) {
			v.push_back(at(i, j));
		}
	}
	return Mat{ N, M, v }; // ... create in row-major order
}


// You don't need to read this, just believe in it.
// TODO proper documentation
Mat Mat::inverse() const {
	assert(M == N);
	// TODO check determinant

	const int dim = M; // or N

	// identity and unit-matrix for Gauss-Jordan
	vector<vector<double>> left(M);
	for (int i = 0; i < M; ++i) {
		left[i].resize(N);
		for (int j = 0; j < N; ++j) {
			left[i][j] = at(i, j);
		}
	}
	vector<vector<double>> right(dim);
	// create unit-matrix
	for (int i = 0; i < dim; ++i) {
		right[i].resize(dim, 0.0);
		right[i][i] = 1.0;
	}

	for (int i = 0; i < dim; ++i) { // i: rows from top to bottom
		double fac = left[i][i];

		int swapRow = i;
		while (fac == 0.0) { // find row to swap current with if necessary
			++swapRow;
			fac = left[swapRow][i];
		}
		for (int j = 0; j < dim; ++j) { // do the swap anyways (TODO optimize)
			double tempLeft = left[swapRow][j];
			left[swapRow][j] = left[i][j];
			left[i][j] = tempLeft;
			double tempRight = right[swapRow][j];
			right[swapRow][j] = right[i][j];
			right[i][j] = tempRight;
		}

		for (int j = 0; j < dim; ++j) { // normalize current row based on (i, i)
			left[i][j] = left[i][j] / fac;
			right[i][j] = right[i][j] / fac;
		}

		for (int k = i + 1; k < dim; ++k) { // k: rows from i to bottom
			double val = left[k][i]; // value to neutralize, i.e. making (k, i) zero
			for (int l = 0; l < dim; ++l) { // subtract i-row * val from k-row
				left[k][l] = left[k][l] - left[i][l] * val;
				right[k][l] = right[k][l] - right[i][l] * val;
			}
		}
	}

	// at this point we have a matrix with zeros under the diagonal
	// and 1s at the diagonal

	for (int i = dim - 1; i > 0; --i) { // i: rows from bottom to top
		for (int k = i - 1; k >= 0; --k) { // k: rows from i to top
			double val = left[k][i]; // value to neutralize, i.e. making (k, i) zero
			for (int l = 0; l < dim; ++l) { // subtract i-row * val from k-row
				left[k][l] = left[k][l] - left[i][l] * val;
				right[k][l] = right[k][l] - right[i][l] * val;
			}
		}
	}

	// at this point left became the unit-matrix and right the inverse

	vector<double> newValues;
	for (auto v : right) {
		for (double d : v) {
			newValues.push_back(d);
		}
	}

	return Mat{ M, N, newValues };
}

Mat Mat::rotationX(const double radX) {
	const double cos_radX = cos(radX);
	const double sin_radX = sin(radX);

	Mat rotX{
		1.0, 0.0,		0.0,
		0.0, cos_radX,	-sin_radX,
		0.0, sin_radX,	cos_radX
	};
	return rotX;
}

Mat Mat::rotationY(const double radY) {
	const double cos_radY = cos(radY);
	const double sin_radY = sin(radY);

	Mat rotY{
		cos_radY,	0.0, sin_radY,
		0.0,		1.0, 0.0,
		-sin_radY,	0.0, cos_radY
	};
	return rotY;
}


Mat Mat::operator*(const Mat& other) const {
	assert(N == other.M);
	vector<double> v;
	v.reserve(M * other.N);

	for (int i = 0; i < M; ++i) { // result-matrix's j
		for (int j = 0; j < other.N; ++j) { // result-matrix's i
			double sum = 0.0;
			for (int k = 0; k < N; ++k) { // k for summation
				sum += at(i, k) * other.at(k, j);
			}
			v.push_back(sum);
		}
	}
	return Mat{ M, other.N, v };
}


Vec Mat::operator*(const Vec& v) const {
	assert(N == v.n);

	vector<double> v_new;
	v_new.reserve(N);

	for (int i = 0; i < M; ++i) {
		double sum = 0.0;
		for (int k = 0; k < N; ++k) {
			sum += at(i, k) * v[k];
		}
		v_new.push_back(sum);
	}
	return Vec{ v_new };
}