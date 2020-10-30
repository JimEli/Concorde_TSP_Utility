/*************************************************************************
* Title: CtoK
* Author: James Eli
* Date: 10/30/2020
*
* 1. Converts a csv file of decimal degree lat/long coordinates to
*    a tsp file suitable for input to the Concorde TSP solver. Removes
*    any duplicate coordinates found in file.

* 2. Converts above csv file and Concorde tour cycle (.cyc) file
*    into a kml file showing the generated tour. Optionally, can
*    produce file with/without waypoint depiction (thumbtack).
* 
* 3. Minimal error checking performed.
* 
* Notes:
*  (1) Compiled with MS Visual Studio 2019 Community (v141).
*************************************************************************
* Change Log:
*   10/30/2020: Initial release. JME
*************************************************************************/
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#define _USE_MATH_DEFINES
#include <math.h>

const size_t MIN_COORDINATES{ 4 };

#define LATITUDE 0
#define LONGITUDE 1

// Conversion factors.
// Kilometers.
constexpr double KM_PER_NM{ 1.852 };
constexpr double KM_PER_SM{ 1.609347 };
// Statute miles.
constexpr double SM_PER_KM{ (1.0 / 1.609347) };
constexpr double SM_PER_NM{ 1.150778974 };
// Nautical miles.
constexpr double NM_PER_KM{ (1.0 / 1.852) };
constexpr double NM_PER_SM{ (1.0 / 1.150778974) };

#include "file.h"

// Visual Leak Detector.
#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"

// Remove duplicate elements in an unsorted vector.
template <typename FwdIterator>
FwdIterator removeDuplicates(FwdIterator first, FwdIterator last)
{
	auto newLast = first;

	for (auto current = first; current != last; ++current)
		if (std::find(first, newLast, *current) == newLast)
		{
			if (newLast != current)
				*newLast = *current;
			++newLast;
		}

	return newLast;
}

// Convert degree to radian.
double DegToRad(double degree) { return (degree / 180.0) * M_PI; }
// Convert radian to degree.
double RadToDeg(double radian) { return (radian / M_PI) * 180.0; }

// Mod function.
double Mod(double y, double x)
{
	if (y >= 0.0)
		return (y - x * floor(y / x));
	else
		return y + x * (floor(-(y / x)) + 1.0);
}

// Calculate rhumbline distance.
double rhumbline(const double& p1lat, const double& p1long, const double& p2lat, const double& p2long)
{
	double d = 0.;

	// Calculate true course.
	double tc = Mod(atan2(DegToRad(p1long - p2long), log(tan(DegToRad(p2lat) / 2.0 + M_PI / 4.0) / tan(DegToRad(p1lat) / 2.0 + M_PI / 4.0))), (M_PI * 2.0));

	// Calculate distance (nm).
	if ((tc > 1.570795 && tc < 1.570797) || (tc > 4.71238 && tc < 4.71239)) // 90 or 270.
		d = 60.0 * fabs(p2long - p1long) * cos(DegToRad(p1lat));
	else
		d = 60.0 * ((p2lat - p1lat) / cos(tc));

	//tc = RadToDeg(tc);

	// Convert to km.
	return d * KM_PER_NM;
}

// Distance equation scaling factor.
const double scaleFactor = 10.0;

// Calculate tour cost.
int calcCost(std::vector<int>& tour, const std::vector<std::array<double, 2>>& pts)
{
	int d = (int)(rhumbline(pts[tour[0]][LATITUDE], pts[tour[0]][LONGITUDE], pts[tour[tour.size() - 1]][LATITUDE], pts[tour[tour.size() - 1]][LONGITUDE]) * scaleFactor);
	
	for (auto i = tour.begin(); i < (tour.end() - 1); ++i)
		// Rhumbline distance.
		d += (int)(rhumbline(pts[*i][LATITUDE], pts[*i][LONGITUDE], pts[*(i + 1)][LATITUDE], pts[*(i + 1)][LONGITUDE]) * scaleFactor);

	return d;
}

int main(int argc, char* argv[])
{
	// Program options.
	bool outputPoints = true;
	bool outputTSPFile = false;

	// Commandline argument?
	if (argc < 2)
	{
		std::cout << "Usage: CtoK input\n";
		std::cout << "Input is a comma delimited file of decimal degree latitude/longitude coordinates and a Concorde produced cycle file.\n";
		std::cout << "Output is a kml file of the optimized route.\nOptions:\n";
		std::cout << " -n kml file omits points.\n -o outputs a Concorde TSP input file created from csv input file.\n";
		return EXIT_FAILURE;
	}

	// Parse commandline.
	while (argc > 2)
	{
		std::string option = std::string(argv[argc - 1]);

		if ((option == "-n") || (option == "-N"))
			outputPoints = false;

		if ((option == "-o") || (option == "-O"))
			outputTSPFile = true;
		
		argc--;
	}

	// Attempt input from csv file.
	std::vector<std::array<double, 2>> coordinates;
	size_t n = readCSVFile(argv[1], coordinates);
	if (n < MIN_COORDINATES)
	{
		std::cout << "Insufficeint number of coordinates: " << n << std::endl;
		return EXIT_FAILURE;
	}

	// Remove any duplicate coordinates.
	coordinates.erase(removeDuplicates(coordinates.begin(), coordinates.end()), coordinates.end());
	size_t numCoords = coordinates.size();
	if (n != numCoords)
		std::cout << (n - numCoords) << " duplicate coordinates removed.\n";

	if (outputTSPFile)
	{
		// Display stats.
		std::cout << "Number of coordinates: " << numCoords << std::endl;
		// Output tsp file.
		writeTSPFile(argv[1], numCoords, coordinates);
	}
	else
	{
		// Attempt input from file.
		std::vector<int> tour;
		size_t numEdges = readCycleFile(argv[1], tour);
		if (numEdges != numCoords)
		{
			std::cout << "Number of csv file coordinates ("<< numCoords << ") doesn't match cycle file (" << numEdges << ").\n";
			return EXIT_FAILURE;
		}

		// Calc tour cost.
		int cost = calcCost(tour, coordinates);

		// Display stats.
		std::cout << "Number of coordinates: " << numCoords << std::endl;
		std::cout << std::fixed << std::setprecision(1) << "Total distance: ";
		std::cout << ((cost / scaleFactor) * NM_PER_KM) << "nm \nTour path: ";
		for (int i = 0; i < (int)tour.size(); i++)
			std::cout << tour[i] + 1 << " ";
		std::cout << tour[0] + 1 << std::endl;

		// Output kml file.
		writeFile(argv[1], coordinates, tour, outputPoints);
	}

	return EXIT_SUCCESS;
}
