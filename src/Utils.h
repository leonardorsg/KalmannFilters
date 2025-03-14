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
    double vincentyFormula(Coordinates coordinate1, Coordinates coordinate2) {
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

    static MatrixXd convertData();
    static void outputData();
};


#endif //UTILS_H
