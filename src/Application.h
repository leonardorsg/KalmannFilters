#ifndef APLICATION_H
#define APLICATION_H

#include <fstream>
#include <sstream>
#include <thread>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <regex>
#include <iomanip>
#include <map>
#include "Parser.h"
#include "KalmannFilter.h"
#include "Utils.h"
#include <chrono>


#define DATASET_PATHS_PORTO "../Dataset/Porto/"
#define DATASET_PATHS_BOSTON "../Dataset/Boston/"
#define JSON_FILE_PATH "../Dataset/Porto/bus_data.json"

using namespace std::chrono;

/**@brief Class that manages the menu.*/
class Application {
public:
    Application(std::string env) {
        this->env = std::move(env);
        restartConstructor:
        clearScreen();

        this->portoParser = nullptr;
        this->bostonParser = nullptr;

        std::string choiceString;
        int choice;
        std::cout << "Select a city:\n";
        std::cout << "1. Porto\n";
        std::cout << "2. Boston\n";
        std::cout << "Enter your choice: ";
        std::cin >> choiceString;

        // parse city based on choice
        if (choiceString == "1") {
            if (!portoParsed) {
                try {
                    this->portoParser = new Parser(std::string(DATASET_PATHS_PORTO) + "agency.txt",  std::string(DATASET_PATHS_PORTO) + "calendar.txt",  std::string(DATASET_PATHS_PORTO) + "routes.txt",  std::string(DATASET_PATHS_PORTO) + "stop_times.txt",  std::string(DATASET_PATHS_PORTO) + "stops.txt",  std::string(DATASET_PATHS_PORTO) + "trips.txt", std::string(DATASET_PATHS_PORTO) + "shapes.txt");
                    std::cout << this->portoParser->run() << "\n";
                    portoParsed = true;

                    //Print Data Parsed

                    //std::cout << portoParser->printAgencies() << portoParser->printCalendar() << portoParser->printRoutes() << portoParser->printStops(); //<< portoParser->printTrips();


                } catch (const std::exception &e) {
                    std::cerr << "Error1: " << e.what() << '\n';
                }
                try {

                    // Parse the JSON file to load vehicles
                    this->portoParser->parseVehicles(std::string(JSON_FILE_PATH));

                    // Access the vehicles
                    const auto &vehicles = portoParser->getVehicles();


                    // Print vehicle details
                    for (const auto &vehicle : vehicles) {
                        std::cout << "Route: " << vehicle.getRouteId() << "\n";
                        std::cout << "Trip: " << vehicle.getTrip() << "\n";
                        std::cout << "Direction: " << vehicle.getDirection() << "\n";
                        std::cout << "Agency ID: " << vehicle.getAgencyId() << "\n";
                        std::cout << "Location: (" << vehicle.getCoordinates().Latitude << ", " << vehicle.getCoordinates().Longitude << ")\n\n";

                    }

                } catch (const std::exception &e) {
                    std::cerr << "Error2: " << e.what() << '\n';
                }


            }
        } else if (choiceString == "2") {
            if (!bostonParsed) {
                //Not done yet
                //this->bostonParser = Parser("agency.txt", "calendar.txt", "routes.txt", "stop_times.txt", "stops.txt", "trips.txt");
                bostonParsed = true;
            }
        }

        //std::cout << "Press any key to quit: ";
        //std::cin >> choiceString;
        clearScreen();
        showMainMenu();
    }

    void clearScreen();

    void runKalmannFilter(int bus_line, std::string stop_id) {
        int dataChoice;

        std::cout << "Choose the type of data you want to use: " << std::endl;
        std::cout << "1. Virtual Data (Random)." << std::endl;
        std::cout << "2. GPS Measurements." << std::endl;
        std::cout << "Select: ";
        std::cin >> dataChoice;


        //time step
        double h = 0.1;

        Matrix<double,3,3> A {{1,h,0.5*(h*h)}, {0, 1 , h}, {0,0,1}};
        Matrix<double,3,1> B {{0}, {0}, {0}};
        Matrix<double,1,3> C {{1,0,0}};

        MatrixXd P0; P0.resize(3,3); P0 = MatrixXd::Identity(3,3);
        Matrix<double,1,1> R {{5}};
        MatrixXd Q; Q.resize(3,3); Q.setZero(3,3);
        Matrix<double,3,1> x0 {{0},{0},{0}};

        MatrixXd inputMeasurements, outputMeasurements;
        //Since we are only dealing with distances
        inputMeasurements.resize(1,1);
        outputMeasurements.resize(1,1);
        inputMeasurements.setZero();

        std::map<std::string, std::vector<Coordinates>> tripShape = this->portoParser->getShapes();
        auto tripDistances = Utils::calculateTripDistances(tripShape);
        auto trips = this->portoParser->getTrips();
        std::string shape_id = trips.at(bus_line).getShapeId();
        double totalDistance = tripDistances.at(shape_id);

        MatrixXd measurements = MatrixXd::Zero(1,3);
        std::vector<double> times;

        if (dataChoice == 1) {
            measurements = Utils::generateMeasurements(100,totalDistance);
            times = Utils::generateTimes(100);
        } //else{}
        unsigned int maxSamples = measurements.rows();

        KalmannFilter kf(A,B,C,Q,R,P0,x0,maxSamples);

        //start counting time
        auto start = high_resolution_clock::now();
        std::list<double> travelledMeasurements;

        //Kalmann Filter Loop
        for (int i = 0; i < maxSamples; i++) {
            outputMeasurements = measurements.row(i);
            kf.predictEstimate(inputMeasurements);
            inputMeasurements.col(i + 1) = kf.updateEstimate(outputMeasurements);
            //Insert last
            travelledMeasurements.push_back(inputMeasurements(0, i+1));
            double travelledDistance = Utils::getTotalDistance(travelledMeasurements);
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(now - start).count() /1e6;
            double ETA;
            if (dataChoice == 1) {
                ETA = (totalDistance - travelledDistance) / times[i];
            }else if (dataChoice == 2) ETA = (totalDistance - travelledDistance) / duration;

            std::cout << "ETA for bus" + std::to_string(bus_line) + ":" + std::to_string(ETA) + "\n";
            std::cout << "Travelled distance: " << travelledDistance << "\n";
        }

    }

    void run(int processedKey);
    static void delay(long sleepTime);
    static int processKey(const std::string &option);

private:

    /* Private functions */
    void showMainMenu();
    void showGoBackMenu(int option, const std::string& functionName);
    bool portoParsed = false;
    bool bostonParsed = false;
    Parser* portoParser;
    Parser* bostonParser;


    /* Attributes */
    std::string env;

};

#endif //APLICATION_H