#pragma once

#include <cmath>
#include <vector>


struct Vec {
	const std::vector<double> data;
	const std::size_t n;


	Vec(const std::vector<double>& data, const std::size_t n) : data(data), n(n) {}


	Vec(const std::vector<double>& data) : Vec(data, data.size()) {}


	Vec(const double x, const double y, const double z) : Vec(std::vector<double>{x, y, z}, 3) {}


	double operator[](const int i) const { return data[i]; }


	double length() const {
		double sum = 0.0;
		for (const double d : data) {
			sum += d * d;
		}
		return std::sqrt(sum);
	}


	Vec normalize() const {
		const double len = length();

		std::vector<double> newData;
		newData.reserve(n);

		for (const double d : data) {
			newData.push_back(d / len);
		}

		return Vec{ newData };
	}

};