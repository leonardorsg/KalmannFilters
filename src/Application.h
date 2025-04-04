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
                    /*
                    for (const auto &vehicle : vehicles) {
                        std::cout << "Route: " << vehicle.getRouteId() << "\n";
                        std::cout << "Trip: " << vehicle.getTrip() << "\n";
                        std::cout << "Direction: " << vehicle.getDirection() << "\n";
                        std::cout << "Agency ID: " << vehicle.getAgencyId() << "\n";
                        std::cout << "Location: (" << vehicle.getCoordinates().Latitude << ", " << vehicle.getCoordinates().Longitude << ")\n\n";

                    }
                    */

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

    void runKalmannFilter(std::string bus_line, std::string stop_id, int direction);

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