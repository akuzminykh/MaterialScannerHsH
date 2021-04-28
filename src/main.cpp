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

Mat correctiveRotationX(const double radians, const int y, const size_t height, const double addFactor) {

	const double radX = addFactor * radians;

	const double cos_radX = cos(radX);
	const double sin_radX = sin(radX);

	Mat rotX{
		1.0, 0.0,		0.0,
		0.0, cos_radX,	-sin_radX,
		0.0, sin_radX,	cos_radX
	};
	return rotX;
}

Mat correctiveRotationY(const double radians, const int x, const size_t width, const double addFactor) {

	const double radY = addFactor * radians;

	const double cos_radY = cos(radY);
	const double sin_radY = sin(radY);

	Mat rotY{
		cos_radY,	0.0, sin_radY,
		0.0,		1.0, 0.0,
		-sin_radY,	0.0, cos_radY
	};
	return rotY;
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

	//check if height and width are even numbers
	const size_t heightCalc = height % 2 != 0 ? height - 1 : height;
	const size_t widthCalc = width % 2 != 0 ? width - 1 : width;

	const size_t sizeRatio = static_cast<double>(heightCalc) / widthCalc;

	//width and height are scaled into [-1, 1].
	// Es werden die ersten beiden werte für die Faktoren berechnet. 
	// Mit dieser Rechnung finden wir heraus, wie die Differenz bzw. der Abtand zwischen zwei nebeneinanderliegenden Pixeln ist.  
	// Da die Differenz immer gleich bleibt, können wir durch eine Addition der Differenz pro Pixel bestimmen, 
	// wie sehr sich die Korrektur der Normalen auf jeden Pixel auswirkt.

	const double firstPixelCorrectionValueX = -1;
	const double secondPixelCorrectionValueX = ((static_cast<double>(1) / (height - 1)) * 2 - 1);
	const double factorX = secondPixelCorrectionValueX - firstPixelCorrectionValueX;

	const double firstPixelCorrectionValueY = -1;
	const double secondPixelCorrectionValueY = ((static_cast<double>(1) / (width - 1)) * 2 - 1);
	const double factorY = secondPixelCorrectionValueY - firstPixelCorrectionValueY;

	double addFactorX = -1;
	double addFactorY = -1;
	
	vector<Mat> rotY;
	for (int x = 0; x < width; x++) {
		rotY.push_back(correctiveRotationY(correctionFactor, x, width, addFactorY));
		addFactorY += factorY;
	}

	for (int y = 0; y < height; ++y) {

		const Mat rotX = correctiveRotationX(correctionFactor*sizeRatio, y, height, addFactorX);
		addFactorX += factorX;

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
					
					//orientation correction
					const Vec normal = (rotX * (rotY[x] * n)).normalize();
				
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

	try {
		writeNormalMap(nmap, outNormalMap);
	}
	catch (invalid_argument e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	
	steady_clock::time_point end = steady_clock::now();
	cout << "Calculation Time Normalmap (sec) = " << (duration_cast<microseconds>(end - begin).count()) / 1000000.0 << std::endl;
	
	return EXIT_SUCCESS;
}