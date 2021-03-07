#include "io.hpp"
using std::vector;
using std::string;
using std::stod;

#include "NormalMap.hpp"
#include "util.hpp"

#include <stdexcept>
using std::invalid_argument;

#include <OpenImageIO/imageio.h>

#include <iostream>
using std::cout;

using std::size_t;

#include <cassert>

#include <filesystem>
using std::filesystem::path;
using std::filesystem::directory_iterator;
using std::filesystem::is_directory;

#include <algorithm>


vector<string> listItems(const string& dir) {
	const path p{ dir };
	vector<string> items;
	if (is_directory(p)) {
		for (const auto & item : directory_iterator{ p }) {
			items.push_back(item.path().string());
		}
	}
	else {
		throw invalid_argument{ "Not a directory: " + dir };
	}
	return items;
}


vector<ReflectionMap> readDataset(const string& dir) {

	const vector<string> items = listItems(dir);

	if (items.size() != 8) {
		throw invalid_argument{ "Currently only for 8 images." };
	}

	vector<ReflectionMap> dataset;
	dataset.reserve(8);
	
	for (auto & item : items) {
		dataset.push_back(readIntensities(item));
	}

	return dataset;
}


ReflectionMap readIntensities(const string & file) {
	cout << "Reading image.\n";

	vector<string> imageParams = splitBy(path{ file }.stem().string(), '_');

	if (imageParams.size() != 3) {
		throw invalid_argument("File expected in the form \"name_azimuthalAngle_polarAngle.ext\": " + file);
	}

	const double azimuthalDegrees = stod(imageParams[1]);
	const double polarDegress = stod(imageParams[2]);

	if (azimuthalDegrees < 0.0 || 360.0 <= azimuthalDegrees) {
		throw invalid_argument("Illegal azimuthal-angle: " + file);
	}
	if (polarDegress < 0.0 || 90.0 < polarDegress) {
		throw invalid_argument("Illegal polar-angle: " + file);
	}

	const OIIO::ImageInput::unique_ptr in = OIIO::ImageInput::open(file);
	if (!in) throw invalid_argument{ "Cannot open file: " + file };

	const OIIO::ImageSpec& inSpec{ in->spec() };
	const int numChannels = inSpec.nchannels;
	if (numChannels == 3) {
		for (int i = 0; i < numChannels; ++i) {
			if (!(inSpec.channelformat(i) == OIIO::TypeDesc::UINT8)) {
				in->close();
				throw invalid_argument("Only accepting 8-bit-formats currently: " + file);
			}
		}
	}
	else {
		in->close();
		throw invalid_argument("Only accepting RGB-formats (not RGBA) currently: " + file);
	}

	// Error-checking-stuff is done.

	const size_t width = inSpec.width;
	const size_t height = inSpec.height;
	const size_t nPixels = width * height;
	const size_t nChars = nPixels * numChannels;

	vector<unsigned char> data(nChars);
	in->read_image(OIIO::TypeDesc::UINT8, &data[0]);
	in->close();

	vector<double> values;
	values.reserve(nPixels);

	const unsigned char* r = &data[0];
	const unsigned char* g = r + 1;
	const unsigned char* b = g + 1;
	const unsigned char* end = r + (nChars);

	for (; r != end; r += 3, g += 3, b += 3) {
		const double gray = 0.299 * *r + 0.587 * *g + 0.114 * *b;
		values.push_back(gray / 255);
	}

	return ReflectionMap{
		width,
		height,
		values,
		degreesToRadians(azimuthalDegrees),
		degreesToRadians(polarDegress)
	};
}


void writeNormalMap(const NormalMap& normalMap, const string& file) {
	cout << "Writing image.\n";

	const std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(file);
	if (! out) throw invalid_argument{ "Cannot create file: " + file };
	const OIIO::ImageSpec spec(normalMap.width, normalMap.height, 3, OIIO::TypeDesc::UINT8);

	const size_t nNormals = normalMap.normalsData.size();

	vector<unsigned char> data;
	data.reserve(nNormals);

	const double* x_p = &normalMap.normalsData[0];
	const double* y_p = x_p + 1;
	const double* z_p = y_p + 1;
	const double* end = x_p + (nNormals);

	for (; x_p != end; x_p += 3, y_p += 3, z_p += 3) {
		// The normal can obviously have negative values.
		// We scale from [-1, 1] into [0, 1].
		const double x = (*x_p + 1) / 2;
		const double y = (*y_p + 1) / 2;
		const double z = (*z_p + 1) / 2;

		// If this ever throws, make it an if.
		// Or prove that it can never happen in our setup.
		// Negative z is actually impossible.
		assert(x >= 0.0);
		assert(y >= 0.0);
		//assert(z >= 0.0);

		data.push_back(static_cast<unsigned char>(x * 255));
		data.push_back(static_cast<unsigned char>(y * 255));
		data.push_back(static_cast<unsigned char>(z * 255));
	}

	out->open(file, spec);
	out->write_image(OIIO::TypeDesc::UINT8, &data[0]);
	out->close();
}