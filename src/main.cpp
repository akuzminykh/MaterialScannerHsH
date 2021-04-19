#include "io.hpp"
#include "util.hpp"

#include <iostream>
using std::cout;
using std::cerr;

#include <stdexcept>
using std::invalid_argument;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <cmath>
using std::sin;
using std::cos;

#include "../submodules/ThreadPool/ThreadPool.h"
using std::future;

#include <chrono>

Mat correctiveRotationX(const double factor, const int y, const size_t height, const size_t sizeRatio) {

	const double rotFacX = factor * sizeRatio;
	const double degX = (static_cast<double>(y) / (height-1) * 2.0 - 1.0) * rotFacX;
	const double radX = degreesToRadians(degX);

	const double cos_radX = cos(radX);
	const double sin_radX = sin(radX);

	Mat rotX{
		1.0, 0.0,		0.0,
		0.0, cos_radX,	-sin_radX,
		0.0, sin_radX,	cos_radX
	};
	return rotX;
}

Mat correctiveRotationY(const double factor, const int x, const size_t width) {

	const double rotFacY = factor;
	const double degY = (static_cast<double>(x) / (width - 1) * 2.0 - 1.0) * rotFacY;
	const double radY = degreesToRadians(degY);

	const double cos_radY = cos(radY);
	const double sin_radY = sin(radY);

	Mat rotY{
		cos_radY,	0.0, sin_radY,
		0.0,		1.0, 0.0,
		-sin_radY,	0.0, cos_radY
	};
	return rotY;
}


Vec orientationCorrection(const Vec& v, const int x, const Mat& rotX, const vector<Mat>& rotY)
{
	return rotX * (rotY[x] * v);
}


NormalMap photometricStereo(const vector<ReflectionMap>& dataset, const double correctionFactor) {

	for (int i = 0; i < dataset.size(); ++i) {
		if ((dataset[0].width != dataset[i].width) || (dataset[0].height != dataset[i].height)) {
			throw invalid_argument("Die Dateien haben nicht die gleiche Größe!");
		}
	}
	const size_t nImages = dataset.size();
	const size_t width = dataset[0].width;
	const size_t height = dataset[0].height;

	vector<Vec> lightDirs;
	lightDirs.reserve(dataset.size());

	for (int i = 0; i < nImages; ++i) {
		lightDirs.push_back(dataset[i].incidentIlluminationDirection());
	}

	vector<double> L_data;
	L_data.reserve(nImages * 3);

	for (int i = 0; i < nImages; ++i) {
		L_data.push_back(lightDirs[i][0]);
		L_data.push_back(lightDirs[i][1]);
		L_data.push_back(lightDirs[i][2]);
	}

	const Mat L{ nImages, 3, L_data };
	const Mat L_transposed = L.transpose();
	const Mat L_inverse = (L_transposed * L).inverse();
	const Mat L_inverseTransposed = L_inverse * L_transposed;
	
	
	vector<double> normalsData;
	normalsData.reserve(height * width * 3);

	const unsigned int parallelism = std::thread::hardware_concurrency();
	assert(parallelism > 0);
	ThreadPool pool{ parallelism };
	vector< future< vector<double> > > futures;
	futures.reserve(height);

	cout << "Calculating ... (" << parallelism << " threads)\n";

	//check if height and width are even numbers
	const size_t heightCalc = height % 2 != 0 ? height - 1 : height;
	const size_t widthCalc = width % 2 != 0 ? width - 1 : width;

	const size_t sizeRatio = static_cast<double>(heightCalc) / widthCalc;

	vector<Mat> rotY;
	for (int x = 0; x < width; x++) {
		rotY.push_back(correctiveRotationY(correctionFactor, x, width));
	}

	for (int y = 0; y < height; ++y) {

		const Mat rotX = correctiveRotationX(correctionFactor, y, height, sizeRatio);
		
		future<vector<double>> future = pool.enqueue(
			[y, width, nImages, &dataset, &L_inverseTransposed, rotX, &rotY] {
				
				vector<double> row;
				row.reserve(width * 3);

				int index = y * width;
				
				for (int x = 0; x < width; ++x) {

					vector<double> reflections;
					reflections.reserve(nImages);

					for (int k = 0; k < nImages; ++k) {
						reflections.push_back(dataset[k].intensities[index]);
					}
					++index;

					// This is what you saw in the paper by Woodham (1980).
					const Vec n = L_inverseTransposed * Vec{ reflections };

					const Vec normal = orientationCorrection(n, x, rotX, rotY).normalize();
					row.push_back(normal[0]);
					row.push_back(normal[1]);
					row.push_back(normal[2]);
				}

				return row;
			}
		);

		futures.push_back(std::move(future));
	}

	for (int i = 0; i < height; ++i) {
		const vector<double> row = futures[i].get();

		for (const double d : row) {
			normalsData.push_back(d);
		}
	}

	return NormalMap{ width, height, normalsData };
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "Pass a path to the dataset, path for the result and a factor for correction." << '\n';
		return EXIT_FAILURE;
	}

	vector<ReflectionMap> dataset;
	const string datasetDirectory{ argv[1] };
	const string outNormalMap{ argv[2] };
	const double correctionFactor = std::stoi(argv[3]);

	try {
		dataset = readDataset(datasetDirectory);
	}
	catch (invalid_argument e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	const NormalMap nmap = photometricStereo(dataset, correctionFactor);

	try {
		writeNormalMap(nmap, outNormalMap);
	}
	catch (invalid_argument e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference (sec) = " << (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) / 1000000.0 << std::endl;
	
	return EXIT_SUCCESS;
}