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


// Rotates v around Y by factor * someValue. The values x and width
// are use to calculate someValue: x that is between 0 and width is
// scaled into [-1, 1]. Then a similar rotation around X happens.
Vec orientationCorrection(const Vec& v, const double factor,
	const int x, const int y, const size_t width, const size_t height)
{
	const double rotFacY = factor;
	const double degY = (static_cast<double>(x) / width * 2.0 - 1.0) * rotFacY;
	const double radY = degreesToRadians(degY);

	const double rotFacX = factor * (static_cast<double>(height) / width);
	const double degX = (static_cast<double>(y) / height * 2.0 - 1.0) * rotFacX;
	const double radX = degreesToRadians(degX);

	// Calculating these matrices is super expensive, especially for each pixel.
	// Need to optimize that. TODO: Cache already known values.

	const double cos_radY = cos(radY);
	const double sin_radY = sin(radY);

	Mat rotY{
		cos_radY,	0.0, sin_radY,
		0.0,		1.0, 0.0,
		-sin_radY,	0.0, cos_radY
	};

	const double cos_radX = cos(radX);
	const double sin_radX = sin(radX);

	Mat rotX{
		1.0, 0.0,		0.0,
		0.0, cos_radX,	-sin_radX,
		0.0, sin_radX,	cos_radX
	};

	return rotX * (rotY * v);
}


NormalMap photometricStereo(const vector<ReflectionMap>& dataset, const double correctionFactor) {

	// TODO: check if it's the same for all
	const size_t nImages = dataset.size();
	const size_t width = dataset[0].width;
	const size_t height = dataset[0].height;

	cout << "Dataset Size: (" << dataset.size() << " )\n";

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

	vector<double> normalsData;
	normalsData.reserve(height * width * 3);

	const unsigned int parallelism = std::thread::hardware_concurrency();
	assert(parallelism > 0);
	ThreadPool pool{ parallelism };
	vector< future< vector<double> > > futures;
	futures.reserve(height);

	cout << "Calculating ... (" << parallelism << " threads)\n";

	cout << "Width: (" << width << " )\n";

	vector<Vec> correctedNormals;
	correctedNormals.reserve(width);
	cout << "Correctede Normals size: " << correctedNormals.size() << "\n";


	for (int y = 0; y < height; ++y) {

		future<vector<double>> future = pool.enqueue(
			[y, width, height, nImages, correctionFactor, &dataset, &L_transposed, &L_inverse, &correctedNormals] {
				
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
					const Vec L_transposeI = L_transposed * Vec{ reflections };
					const Vec n = L_inverse * L_transposeI;


					
					if (y == 0) {
						correctedNormals.push_back(orientationCorrection(n, correctionFactor, x, y, width, height).normalize());
						// cout << "Corrected Normals in Spalte" << y << ": (" << correctedNormals[x][0] << "," << correctedNormals[x][1]  << "," << correctedNormals[x][2]  <<"\n";

					}
					
					const Vec normal = orientationCorrection(n, correctionFactor, x, y, width, height).normalize();

					row.push_back(normal[0]);
					row.push_back(normal[1]);
					row.push_back(normal[2]);
				}

				if (y == 0) {
					cout << "Size: " << correctedNormals.size() << "\n";
					correctedNormals.push_back(Vec{ 1,2,3 });
					cout << "Size: " << correctedNormals.size() << "\n";
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

	const NormalMap nmap = photometricStereo(dataset, correctionFactor);

	try {
		writeNormalMap(nmap, outNormalMap);
	}
	catch (invalid_argument e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	

	return EXIT_SUCCESS;
}