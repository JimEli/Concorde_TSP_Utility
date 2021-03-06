// File input/ouput utilities.
#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>

#include "kml.h"

// Attempt to open a file for input.
bool openFile(const std::string fn, const std::string ext, std::ifstream& fs)
{
    std::string in = removeExt(fn);
    in.append(ext);

    fs.open(in, std::ifstream::in);
    if (!fs)
        std::cerr << "Unable to read input file \"" << fn << "\"\n";

    return fs.is_open();
}

// Read csv file of coordinates.
size_t readCSVFile(const std::string fn, std::vector<std::array<double, 2>>& table)
{
    std::ifstream ifs;
    size_t n = 0;

    if (openFile(fn, ".csv", ifs))
    {
        std::string sRow;

        while (getline(ifs, sRow))
        {
            std::array<double, 2> dRow;
            std::string value;
            std::stringstream ss(sRow);
            char separator = ',';
            int i = 0;

            while (getline(ss, value, separator))
                dRow[i++] = (std::stod(value));
            table.push_back(dRow);
            n++;
        }

        ifs.close();
    }

    return n;
}

// Read Concorde cycle file.
size_t readCycleFile(const std::string fn, std::vector<int>& tour)
{
    size_t n = 0;
    std::ifstream ifs;

    if (openFile(fn, ".cyc", ifs))
    {
        std::string sRow;

        while (getline(ifs, sRow))
        {
            int i;
            std::string value;
            std::stringstream ss(sRow);
            char separator = ' ';

            while (getline(ss, value, separator))
                i = (std::stoi(value));
            tour.push_back(i);
            n++;
        }

        ifs.close();
    }

    return n;
}

// Write tsp file.
bool writeTSPFile(const std::string fn, size_t ncount, std::vector<std::array<double, 2>>& pts)
{
    std::ofstream ofs;
    std::string out = removeExt(fn);
    std::string tspName(out);
    out.append(".tsp");
    ofs.open(out.c_str(), std::ios::out);
    if (!ofs.is_open())
        return false;
    
    // Remove any leading path.
    size_t pos = out.find_last_of("\\/.");
    if (pos != std::string::npos && out[pos] == '.')
        tspName = out.substr(0, pos);

    // Output file data.
    ofs << "NAME: " << tspName << ncount << std::endl;
    ofs << "TYPE: TSP\n";
    ofs << "COMMENT: Generated by CtoK writeTSPFile\n";
    ofs << "DIMENSION: " << ncount << std::endl;
    ofs << "EDGE_WEIGHT_TYPE: EUC_2D\n";
    ofs << "NODE_COORD_SECTION\n";
    for (size_t i = 0; i < ncount; i++)
        ofs << i + 1 << " " << pts[i][LONGITUDE] << " " << pts[i][LATITUDE] << std::endl;

    ofs.close();
    return true;
}

// Write basic kml file.
void writeFile(const std::string fn, const std::vector<std::array<double, 2>>& pts, const std::vector<int>& tour, bool outputPoints)
{
    KML kml;

    // Start file.
    if (kml.open(fn))
    {
        kml.tagOpen("Folder");
        kml.tagStart("Placemark"); kml.write("id="); kml.tagEnd("\"TOUR\"");

        // Change line style.
        kml.tagOpen("Style");
        kml.tagOpen("LineStyle");
        //kml.tagLine("color", "FF0000FF");  // Red.
        kml.tagLine("width", "3.0");
        kml.tagClose(); // linestyle.
        kml.tagClose(); // style.

        // Tour coordinates.
        kml.tagOpen("LineString");
        kml.tagOpen("coordinates");
        for (auto n : tour)
            kml.write(std::to_string(pts[n][LONGITUDE]) + "," + std::to_string(pts[n][LATITUDE]) + "\n", true);
        kml.write(std::to_string(pts[tour[0]][LONGITUDE]) + "," + std::to_string(pts[tour[0]][LATITUDE]) + "\n", true);
        kml.tagClose(); // coordinates.
        kml.tagClose(); // linestring.
        kml.tagClose(); // placemark.

        if (outputPoints)
        {
            // Add individual points.
            for (size_t i = 0; i < pts.size(); i++)
            {
                kml.tagOpen("Placemark");
                kml.tagLine("name", std::to_string(i + 1));
                kml.tagOpen("Point");
                kml.tagOpen("coordinates");
                kml.write(std::to_string(pts[i][LONGITUDE]) + "," + std::to_string(pts[i][LATITUDE]) + "\n", true);
                kml.tagClose(); // coordinates.
                kml.tagClose(); // point.
                kml.tagClose(); // placemark.
            }
        }
        // Finish file.
        kml.tagClose(); // folder.
    }
    else
        std::cerr << "Error opening output file.\n";
}

#endif
