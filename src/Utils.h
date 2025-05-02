//
// Created by leonardorsg on 3/7/25.
//

#ifndef UTILS_H
#define UTILS_H

/**
 * @file Utils.h
 * @brief Utility class for geographic calculations, bus movement simulations, and measurement generation.
 *
 * This file contains a `Utils` class with methods for:
 * - Calculating distances between geographic coordinates using the Vincenty formula.
 * - Generating random times and bus movement data.
 * - Calculating bus distances and trip distances.
 */

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

/**
 * @struct Coordinates
 * @brief Structure representing geographic coordinates (latitude and longitude).
 */
struct Coordinates {
    double Latitude;
    double Longitude;
};
/**
 * @def MAJOR_AXIS
 * @brief Semi-major axis of the ellipsoid (in meters).
 */
#define MAJOR_AXIS 6348137.0
/**
 * @def FLATTENING
 * @brief Flattening constant for the ellipsoid.
 */
#define FLATTENING 0.003352810665
/**
 * @def MINOR_AXIS
 * @brief Semi-minor axis of the ellipsoid (in meters).
 */
#define MINOR_AXIS 6325852.899

/**
 * @class Utils
 * @brief Utility class that provides various static methods for geographic calculations, bus movements, and data generation.
 */
class Utils {
    public:
    Utils(){};

    /**
     * @brief Calculates the distance between two geographic coordinates using the Vincenty formula.
     *
     * The Vincenty formula calculates the geodesic distance between two points on an ellipsoid.
     *
     * @param coordinate1 The first coordinate.
     * @param coordinate2 The second coordinate.
     * @return The distance between the two coordinates in meters.
     * @note Returns -1 if the formula fails to converge.
     */
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

    /**
     * @brief Calculates the total distance from a list of measurements.
     *
     * This method sums up all the distances in the list and returns the total.
     *
     * @param measurements A list of distance measurements.
     * @return The total distance in the list.
     */
    static double getTotalDistance(std::list<double> measurements) {
        double totalDistance = 0;

        for (auto distance : measurements) {
            totalDistance += distance;
        }

        return totalDistance;
    }

    /**
     * @brief Generates a list of random times.
     *
     * The method generates `num_values` random times between 0 and 3000, sorts them, and returns the sorted list.
     *
     * @param num_values The number of random values to generate.
     * @return A vector containing the sorted random times.
     */
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

    /**
     * @brief Generates a matrix of bus movement measurements.
     *
     * This method generates bus movement data based on a simple model, where the bus accelerates and decelerates.
     * It also adds noise to the measurements to simulate realistic bus behavior.
     *
     * @param maxSamples The maximum number of samples to generate.
     * @param totalDistance The total distance to cover during the simulation.
     * @return A matrix containing the generated bus movement measurements.
     */
    static MatrixXd generateMeasurements(unsigned int maxSamples, double totalDistance) {
        MatrixXd measurements(maxSamples, 3);
        measurements.setZero();

        // Generate realistic bus movement profile
        double step = totalDistance / (maxSamples-1);
        double current_vel = 0;
        double max_vel = 15.0; // ~54 km/h

        for (unsigned int i = 0; i < maxSamples; i++) {
            // Simulate bus acceleration then deceleration
            if (i < maxSamples/2) {
                current_vel = std::min(max_vel, current_vel + 0.1);
            } else {
                current_vel = std::max(0.0, current_vel - 0.1);
            }

            // Add realistic noise (5m max error)
            double noise = (rand() % 1000)/100.0 - 5.0;

            measurements(i, 0) = i * step + noise;
            measurements(i, 1) = current_vel + noise/10.0;
            measurements(i, 2) = 0; // Near-zero acceleration
        }

        return measurements;
    }

    /**
     * @brief Calculates the distance from a bus stop to a trip shape's coordinates.
     *
     * This method calculates the distance between a given bus stop's coordinates and the closest point on the bus trip's shape.
     *
     * @param tripShape A map of trip shapes, where each shape is associated with a list of coordinates.
     * @param shape_id The identifier of the bus trip shape.
     * @param stop_coordinates The coordinates of the bus stop.
     * @return The distance to the bus stop from the trip's shape, in meters.
     * @throws std::runtime_error if the shape ID is not found or the coordinates are invalid.
     */
    static double calculateBusDistance(const std::map<std::string,std::vector<Coordinates>>& tripShape,
                                  const std::string& shape_id,
                                  const Coordinates& stop_coordinates) {
        try {
            double bus_distance = 0;
            double distance = 0;
            double shortest_distance = 500000;
            auto shape_it = tripShape.find(shape_id);
            if (shape_it == tripShape.end()) {
                throw std::runtime_error("Shape ID not found");
            }

            const auto& coordinates = shape_it->second;
            if (coordinates.empty()) {
                throw std::runtime_error("No coordinates for shape");
            }

            for (size_t i = 1; i < coordinates.size(); i++) {
                bus_distance += vincentyFormula(coordinates[i - 1], coordinates[i]);
                distance = vincentyFormula(coordinates[i], stop_coordinates);
                if (distance < shortest_distance) { shortest_distance = distance; }
                //Calculates distance up until 200m of the bus stop
                //std::cout << "Distance: " << distance << std::endl;
            }

            if (distance > 0) return distance;
            std::ostringstream error_msg;
            error_msg << "Stop not found within route " << shortest_distance;
            throw std::runtime_error(error_msg.str());
        } catch (const std::exception& e) {
            std::cerr << "Error in calculateBusDistance: " << e.what() << std::endl;
            return -1.0;
        }
    }

    /**
     * @brief Calculates the total distance for each trip in a set of trips.
     *
     * This method computes the total distance for each trip by summing the distances between consecutive coordinates.
     *
     * @param trips A map of trips, where each trip is associated with a list of coordinates.
     * @return A map of total distances for each trip.
     */
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

    /**
     * @brief Placeholder method to convert data.
     *
     * This method is a placeholder for data conversion logic, which can be implemented as needed.
     */
    static MatrixXd convertData();
    /**
     * @brief Placeholder method to output data.
     *
     * This method is a placeholder for logic to output data, which can be implemented as needed.
     */
    static void outputData();
};


#endif //UTILS_H
