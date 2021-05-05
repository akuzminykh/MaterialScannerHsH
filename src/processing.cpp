#include "processing.hpp"


// width and height of the image are scaled into [-1, 1].
// that means the first pixel in x-axis / y-axis is always -1; 
double distanceBetweenPixelPair(const size_t& length) {

	// Pixel at Position 0
	const double firstPixelPos = -1;

	// Pixel at Position 1
	// TODO: Lenght cannot be one, because of zero division
	const double secondPixelPos = ((static_cast<double>(1) / (length - 1)) * 2 - 1);
	const double distanceBetweenPixels = secondPixelPos - firstPixelPos;

	return distanceBetweenPixels;
}

double calcSizeRatio(const size_t& height, const size_t& width) {
	//check if height and width are even numbers
	const size_t heightCalc = height % 2 != 0 ? height - 1 : height;
	const size_t widthCalc = width % 2 != 0 ? width - 1 : width;

	const double sizeRatio = static_cast<double>(heightCalc) / widthCalc;
	return sizeRatio;
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