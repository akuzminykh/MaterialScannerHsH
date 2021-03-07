#define _USE_MATH_DEFINES

#include "util.hpp"
using std::vector;
using std::string;
using std::getline;

#include <cmath>
using std::pow;
using std::abs;

#include <sstream>
using std::stringstream;


// This program most likely won't ever process images
// with a bit-depth > 16, so this is our limit.
const double DOUBLE_EQUALITY_EPSILON = 1.0 / pow(2, 16);


bool nearlyEqual(const double a, const double b) {
	return abs(a - b) < DOUBLE_EQUALITY_EPSILON;
}


double degreesToRadians(const double deg) {
	return deg * M_PI / 180;
}


bool allSmallerEqualTo(const vector<double>& data, const double lim) {
	for (const double d : data) {
		if (!(d <= lim)) {
			return false;
		}
	}
	return true;
}


vector<string> splitBy(const string& s, const char d) {
	stringstream ss{ s };
	string word;
	vector<string> words;
	while (getline(ss, word, d)) {
		words.push_back(word);
	}
	return words;
}