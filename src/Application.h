#ifndef APLICATION_H
#define APLICATION_H

/**
 * @file Application.h
 * @brief Declares the Application class which provides a user interface to run Kalman filtering on bus data.
 */

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

/**
 * @class Application
 * @brief Manages the lifecycle of the application, including parsing data, showing menus, and running the Kalman filter.
 */
class Application {
public:
    /**
     * @brief Constructs the Application object and initializes data parsing and user interaction.
     * @param env The operating system environment ("win" or "unix").
     */
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
    /**
     * @brief Clears the screen based on the OS.
     */
    void clearScreen();

    /**
     * @brief Runs the Kalman filter process for a specific bus line and stop.
     * @param bus_line The bus line identifier.
     * @param stop_id The stop identifier.
     * @param direction The direction of travel (0 for outbound, 1 for inbound).
     */
    void runKalmannFilter(std::string bus_line, std::string stop_id, int direction);

    /**
     * @brief Dispatches user actions based on menu input.
     * @param processedKey Numeric representation of the user menu choice.
     */
    void run(int processedKey);

    /**
     * @brief Introduces a delay in milliseconds.
     * @param sleepTime Time in milliseconds to pause.
     */
    static void delay(long sleepTime);

    /**
     * @brief Parses a menu option string into an integer command.
     * @param option The string input.
     * @return Parsed integer command or -1 if invalid.
     */
    static int processKey(const std::string &option);

private:

    /**
     * @brief Shows the main menu.
     */
    void showMainMenu();

    /**
     * @brief Shows a follow-up menu after an operation.
     * @param option Function index.
     * @param functionName Name of the function previously run.
     */
    void showGoBackMenu(int option, const std::string& functionName);
    bool portoParsed = false;
    bool bostonParsed = false;
    Parser* portoParser;
    Parser* bostonParser;


    /* Attributes */
    std::string env;

};

#endif //APLICATION_H