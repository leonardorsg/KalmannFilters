//
// Created by leonardorsg on 3/7/25.
//

#ifndef UTILS_H
#define UTILS_H

#include <Eigen/Dense>
#include "KalmannFilter.h"
#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <bits/random.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace Eigen;

struct Coordinates {
    double Latitude;
    double Longitude;
};

#define MAJOR_AXIS 6348137.0
#define FLATTENING 0.003352810665
#define MINOR_AXIS 6325852.899

class Utils {
    public:
    Utils(){};

    //Function to calculate the distance between two coordinates
    static double vincentyFormula(Coordinates coordinate1, Coordinates coordinate2) {
        // Convert degrees to radians
        double lat1 = coordinate1.Latitude;
        double lat2 = coordinate2.Latitude;
        double lon1 = coordinate1.Longitude;
        double lon2 = coordinate2.Longitude;
        double U1 = atan((1 - FLATTENING) * tan(lat1 * M_PI / 180.0));
        double U2 = atan((1 - FLATTENING) * tan(lat2 * M_PI / 180.0));
        double L = (lon2 - lon1) * M_PI / 180.0;
        double lambda = L;

        double sinU1 = sin(U1), cosU1 = cos(U1);
        double sinU2 = sin(U2), cosU2 = cos(U2);

        double sinLambda, cosLambda, sinSigma, cosSigma, sigma, sinAlpha, cos2Alpha, cos2SigmaM;
        double lambdaPrev;

        int iterations = 0;
        do {
            sinLambda = sin(lambda);
            cosLambda = cos(lambda);
            sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) +
                        (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) * (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
            if (sinSigma == 0) {
                return 0; // Coincident points
            }
            cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
            sigma = atan2(sinSigma, cosSigma);
            sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
            cos2Alpha = 1 - sinAlpha * sinAlpha;
            cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cos2Alpha;
            double C = FLATTENING / 16 * cos2Alpha * (4 + FLATTENING * (4 - 3 * cos2Alpha));
            lambdaPrev = lambda;
            lambda = L + (1 - C) * FLATTENING * sinAlpha *
                         (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
        } while (fabs(lambda - lambdaPrev) > 1e-12 && ++iterations < 100);

        if (iterations >= 100) {
            return -1; // Formula failed to converge
        }

        double uSquared = cos2Alpha * (MAJOR_AXIS * MAJOR_AXIS - MINOR_AXIS * MINOR_AXIS) / (MINOR_AXIS * MINOR_AXIS);
        double A = 1 + uSquared / 16384 * (4096 + uSquared * (-768 + uSquared * (320 - 175 * uSquared)));
        double B = uSquared / 1024 * (256 + uSquared * (-128 + uSquared * (74 - 47 * uSquared)));
        double deltaSigma = B * sinSigma * (cos2SigmaM + B / 4 * (cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM) -
                                 B / 6 * cos2SigmaM * (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));

        double s = MINOR_AXIS * A * (sigma - deltaSigma);
        return s; // Distance in meters
    }

    static double getTotalDistance(std::list<double> measurements) {
        double totalDistance = 0;

        for (auto distance : measurements) {
            totalDistance += distance;
        }

        return totalDistance;
    }

    static std::vector<double> generateTimes(int num_values) {
        // Seed the random number generator
        std::srand(static_cast<unsigned>(std::time(0)));

        // Vector to hold the random values
        std::vector<double> values;

        // Generate random values between 0 and 3000
        for (int i = 0; i < num_values; ++i) {
            double random_value = static_cast<double>(std::rand()) / RAND_MAX * 3000.0;
            values.push_back(random_value);
        }

        // Sort the values in ascending order
        std::sort(values.begin(), values.end());

        return values;
    }

    static MatrixXd generateMeasurements(unsigned int maxSamples, double totalDistance) {
        MatrixXd measurements(maxSamples, 3); // [d, v, a]
        measurements.setZero();
        // Generate evenly spaced distances between 0 and totalDistance
        double stepSize = totalDistance / (maxSamples - 1); // Step size to ensure maxSamples distances
        for (unsigned int i = 0; i < maxSamples; i++) {
            measurements(i, 0) = i * stepSize;  // Distance (d)
            measurements(i, 1) = 0;              // Velocity (v)
            measurements(i, 2) = 0;              // Acceleration (a)
        }

        return measurements;
    }

    // Calculate total distance for each trip
    static std::map<std::string, double> calculateTripDistances(const std::map<std::string, std::vector<Coordinates>> &trips) {
        std::map<std::string, double> tripDistances;

        for (const auto &[shape_id, coordinates] : trips) {
            double totalDistance = 0.0;
            for (size_t i = 1; i < coordinates.size(); i++) {
                totalDistance += vincentyFormula(coordinates[i - 1], coordinates[i]);
            }
            tripDistances[shape_id] = totalDistance;
        }

        return tripDistances;
    }

    static MatrixXd convertData();
    static void outputData();
};


#endif //UTILS_H
