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
using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

// width and height of the image are scaled into [-1, 1].
// that means the first pixel in x-axis / y-axis is always -1; 
double distanceBetweenPixelPair(const size_t& length) {

	// Pixel at Position 0
	const double firstPixelPos = -1;

	// Pixel at Position 1 
	const double secondPixelPos = ((static_cast<double>(1) / (length - 1)) * 2 - 1);
	const double distanceBetweenPixels = secondPixelPos - firstPixelPos;

	return distanceBetweenPixels;
}

vector<Mat> correctionMatricesX(const size_t& height, const double scaledCorrectionFactor) {

	vector<Mat> correctionMatricesX;
	const double distanceBetweenPixelsX = distanceBetweenPixelPair(height);
	double correctionIntensityX = -1;
	for (int y = 0; y < height; y++) {
		correctionMatricesX.push_back(Mat::rotationX(scaledCorrectionFactor * correctionIntensityX));
		correctionIntensityX += distanceBetweenPixelsX;
	}

	return correctionMatricesX;

}

vector<Mat> correctionMatricesY(const size_t& width, const double correctionFactor) {

	vector<Mat> correctionMatricesY;

	// we determine the distance between 2 pixels
	// the distance information is used to determine the correction intensity for each pixel
	const double distanceBetweenPixelsY = distanceBetweenPixelPair(width);

	// -1 and 1 have the same correction intensity, whereas the mathematical sign influences the 'intensity's direction'
	// intensity value of -1 and 1 have the strongest intensity and a value of 0 has no intensity
	// -> the further the pixel is away from the center, the higher the correction intensity is 
	double correctionIntensityY = -1;
	for (int x = 0; x < width; x++) {
		correctionMatricesY.push_back(Mat::rotationY(correctionFactor * correctionIntensityY));
		correctionIntensityY += distanceBetweenPixelsY;
	}

	return correctionMatricesY;

}

double calcSizeRatio(const size_t& height, const size_t& width) {
	//check if height and width are even numbers
	const size_t heightCalc = height % 2 != 0 ? height - 1 : height;
	const size_t widthCalc = width % 2 != 0 ? width - 1 : width;

	const double sizeRatio = static_cast<double>(heightCalc) / widthCalc;
	return sizeRatio;
}

NormalMap photometricStereo(const vector<ReflectionMap>& dataset, const double correctionFactor) {

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

	
	vector<Mat> rotY = correctionMatricesY(width, correctionFactor);
	vector<Mat> rotX = correctionMatricesX(height, correctionFactor * calcSizeRatio(height, width));

	for (int y = 0; y < height; ++y) {

		future<vector<double>> future = pool.enqueue(
			[y, width, nImages, &dataset, &L_inverseTransposed, &rotX, &rotY] {
				
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
					
					//orientation correction
					const Vec normal = (rotX[y] * (rotY[x] * n)).normalize();
				
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
	const double correctionRadians = degreesToRadians(std::stoi(argv[3]));

	try {
		dataset = readDataset(datasetDirectory);
	}
	catch (invalid_argument e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	
	steady_clock::time_point begin = steady_clock::now();

	const NormalMap nmap = photometricStereo(dataset, correctionRadians);

	steady_clock::time_point end = steady_clock::now();
	cout << "Calculation Time Normalmap (sec) = " << (duration_cast<microseconds>(end - begin).count()) / 1000000.0 << std::endl;

	try {
		writeNormalMap(nmap, outNormalMap);
	}
	catch (invalid_argument e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}