#pragma once

#include "Vec.hpp"

#include <vector>
#include <cassert>


struct Mat {
	const std::size_t M;
	const std::size_t N;
	const std::vector<double> data;


	Mat(std::size_t M, std::size_t N, const std::vector<double> & data)
		: M(M), N(N), data(data)
	{
		assert(data.size() == M * N);
	}


	Mat(const double x_11, const double x_12, const double x_13,
		const double x_21, const double x_22, const double x_23,
		const double x_31, const double x_32, const double x_33)
		:
		Mat(3, 3, std::vector<double>{x_11, x_12, x_13, x_21, x_22, x_23, x_31, x_32, x_33}) {}


	double at(const int m, const int n) const {
		assert(m < M && m >= 0);
		assert(n < N && n >= 0);
		return data[m * N + n];
	}


	Mat transpose() const;
	Mat inverse() const;

	static Mat rotationX(const double radX);
	static Mat rotationY(const double radY);

	bool operator==(const Mat& other) const;
	Mat operator*(const Mat& other) const;
	Vec operator*(const Vec& v) const;
};