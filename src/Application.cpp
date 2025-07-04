/**
* @file Application.cpp
 * @brief Contains the implementation of the Application class which manages the Kalman filter execution and menu system.
 */
#include "Application.h"

#include <set>

#include "Utils.h"

/**
 * @brief Main dispatcher that handles user interaction based on menu options.
 * @param processedKey The numeric option entered by the user.
 */
void Application::run(int processedKey) {
    L1:
    clearScreen();

    switch (processedKey) {
        case 1: {
            std::string stop_id;
            std::cout << "Please enter the stop id: ";
            std::cin >> stop_id;
            std::cout << std::endl;


            std::string bus_line;
            std::cout << "Please enter the bus line: ";
            std::cin >> bus_line;
            std::cout << std::endl;

            int direction;
            std::cout << "0 - Outbound Travel\n";
            std::cout << "1 - Inbound Travel\n";
            std::cout << "Please enter the direction: ";

            std::cin >> direction;
            std::cout << std::endl;

            runKalmannFilter(bus_line, stop_id, direction);
            break;
        }
        case 2:
            std::cout << "Thank you very much and Bye-Bye.\n";
            delay(4);
            break;
        default:
            goto L1;
    }
}

/**
 * @brief clears (or simulates clearing) the screen, depending on the OS used
 */
void Application::clearScreen() {
    if (env == "win")
        L1:
        std::cout << "\n\n\n\n\n\n\n\n\n"
                     "\n\n\n\n\n\n\n\n\n"
                     "\n\n\n\n\n\n\n\n\n"
                     "\n\n\n\n\n\n\n\n\n";
    else if (env == "unix")
        if ( system("clear") == -1 ) goto L1;
}

/**
 * @brief Small delay for design purposes
 */
void Application::delay(long sleepMS) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMS));
}


/** @brief Handles the exception thrown by std::stoi. */
int Application::processKey(const std::string& option) {
    try {
        int intOPT = std::stoi(option);
        if (intOPT < 0 || option.size() > 1 || intOPT > 6) throw std::invalid_argument("NegativeNumber");
        return intOPT;
    } catch (std::invalid_argument& argument) {
        std::cout << "\n* Error while parsing option, please input a valid numeric option. *\n";
        delay(2000);
        return -1;
    }
}

/**
 * @brief Shows the main menu
 */
void Application::showMainMenu() {
    std::string opti;
    std::cout << "\nSelect an operation you would like to do:\n\n"
              << "1 - Execute algorithm.\n"
              << "2 - Exit.\n";

    std::cout << "Input: ";
    std::cin >> opti;
    std::cout << "\n";
    run(stoi(opti));
}

/**
 * @brief Shows the "Go back Menu" after each function
 */
void Application::showGoBackMenu(int functionNumber, const std::string& functionName) {
    L1:
    std::cout << "\nPress enter to continue";
    std::string a;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, a);
    clearScreen();
    std::cout << "\n\nWhat would you like to do next:\n"
              << "1 - Return to main menu.\n"
              << "2 - (again) " << functionName << "\n";

    std::string opt;
    std::cout << "Input: ";
    std::cin >> opt;
    std::cout << "\n";

    int processedKey = processKey(opt);

    switch (processedKey) {
        case -1:
            goto L1;
        case 1:
            throw std::invalid_argument("-1");
        default:
            std::cout << "\n* Error while parsing option, please input a valid numeric option. *\n";
            goto L1;
    }
}

/**
 * @brief Executes the Kalman Filter prediction and estimation for a specific bus line and stop.
 * @param bus_line Bus line ID as string.
 * @param stop_id Stop identifier.
 * @param direction Travel direction (0 = outbound, 1 = inbound).
 */

void Application::runKalmannFilter(std::string bus_line, std::string stop_id, int direction) {

        int dataChoice;
        std::cout << "Choose the type of data you want to use: " << std::endl;
        std::cout << "1. Virtual Data (Random)." << std::endl;
        std::cout << "2. GPS Measurements." << std::endl;
        std::cout << "Select: ";
        std::cin >> dataChoice;

        // Take out the letter M, in cases of bus line 2M, 3M etc..
        if (!bus_line.empty() && (bus_line.back() == 'm' || bus_line.back() == 'M') ) {
            bus_line.pop_back();
        }

        const auto& trips = this->portoParser->getTrips();
        const auto& stops = this->portoParser->getStops();
        const auto& shapes = this->portoParser->getShapes();



        std::string shape_id = "";
        // Safe map access
        if (!stops.count(stop_id)) {
            throw std::runtime_error("Invalid stop ID");
        }

        for (auto trip : trips) {
            if (trip.second.getRouteId() == bus_line && trip.second.getDirectionId() == direction) {
                shape_id = trip.second.getShapeId();
            }
        }

        if (shape_id == "") {
            throw std::runtime_error("Invalid bus line or direction");
        }


        const auto& stop = stops.at(stop_id);

        // Safe coordinate conversion
        Coordinates stop_coordinates;
        try {
            stop_coordinates = {std::stod(stop.getStopLat()), std::stod(stop.getStopLon())};
        } catch (...) {
            throw std::runtime_error("Invalid stop coordinates");
        }

        //Calculate the distance between the beginning of the trip and the bus stop
        double totalDistance = Utils::calculateBusDistance(shapes, shape_id, stop_coordinates);
        if (totalDistance < 0) {
            throw std::runtime_error("Could not calculate route distance");
        }

        std::cout << "Total Distance: " << totalDistance << '\n';

        Utils utils;
        MatrixXd measurements = MatrixXd::Zero(1,3);
        std::vector<double> times;

        //time step
        double h = 0.1;

        MatrixXd A{{1, h, 0.5 * (h * h)}, {0, 1, h}, {0, 0, 1}};
        Matrix<double,1,3> C {{1,0,0}};

        MatrixXd P0; P0.resize(3,3); P0 = MatrixXd::Identity(3,3);
        Matrix<double,1,1> R {10}; // valor calibrado, mais desconfiança em dados GPS
        Matrix<double,3,1> x0 {{0},{0},{0}};
        //valor calibrado
        Matrix<double,3,3> Q {
            {0.1, 0, 0},
            {0, 0.01, 0},
            {0, 0, 0.01}
        };

        MatrixXd inputMeasurements, outputMeasurements;
        //Since we are only dealing with distances
        inputMeasurements.resize(3,1);
        outputMeasurements.resize(3,1);
        outputMeasurements.setZero();
        inputMeasurements.setZero();
        double ETA = -1;


        if (dataChoice == 1) {
            measurements = Utils::generateMeasurements(100,totalDistance);
            times = Utils::generateTimes(100);

            unsigned int maxSamples = measurements.rows();

            std::cout << "Max samples: " << maxSamples << std::endl;

            KalmannFilter kf(A,C,Q,R,P0,x0,maxSamples);

            //start counting time
            auto start = high_resolution_clock::now();


            //Kalmann Filter Loop
            for (int i = 0; i < maxSamples-1; i++) {
                try {
                    // Predict step
                    kf.predictEstimate(MatrixXd::Zero(3,1)); // No control input

                    // Update step with measurement
                    outputMeasurements(0,0) = measurements(i,0);
                    MatrixXd state = kf.updateEstimate(outputMeasurements);

                    // Get current estimates
                    double distance = state(0,0);
                    double velocity = state(1,0);
                    double acceleration = state(2,0);

                    // Sanity checks
                    velocity = std::max(0.0, std::min(velocity, 30.0)); // Constrain to 0-30 m/s (108 km/h)

                    // Calculate ETA (only if we've moved and have reasonable velocity)
                    if (distance > 10 && velocity > 0.1) {
                        ETA = (totalDistance - distance) / velocity;
                        std::cout << "ETA: " << ETA
                                  << " Travelled: " << distance
                                  << " Velocity: " << velocity << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error at iteration " << i << ": " << e.what() << std::endl;
                    break;
                }
            }


            kf.finalize();
        }
        else if (dataChoice == 2) {
            KalmannFilter kf(A, C, Q, R, P0, x0, 3000); // 0 = unlimited samples
            auto lastUpdateTime = high_resolution_clock::now();
            double lastDistance = 0;
            int busComes = 0;
            std::set<int> goneBuses;
            bool busFound = false;

            while (true) {

                //Refresh GPS data
                try {
                    this->portoParser->parseVehicles(std::string(JSON_FILE_PATH));
                    std::string vehicleDataFile = "vehicle_data.txt";
                    this->portoParser->saveVehiclesToFile(vehicleDataFile);
                }
                catch (const std::exception &e) {
                    std::cerr << "Error updating vehicle data: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(30));
                    continue;
                }

                const auto& vehicles = this->portoParser->getVehicles();
                const Vehicle* closestVehicle = nullptr;
                double minDistance = std::numeric_limits<double>::max();

                // 1. Find the closest vehicle matching the criteria
                for (const auto& vehicle : vehicles) {
                    if (vehicle.getRouteId() == std::stoi(bus_line)
                        && vehicle.getDirection() == direction
                        && goneBuses.find(vehicle.getTrip()) == goneBuses.end()) {

                        double currentDistance = Utils::vincentyFormula(stop.getLocation(),vehicle.getCoordinates());
                        if (currentDistance < minDistance) {
                            minDistance = currentDistance;
                            closestVehicle = &vehicle;
                        }
                        }
                }

                // 2. If found, update Kalman filter with that vehicle
                if (closestVehicle) {
                    busFound = true;



                    try {
                        kf.predictEstimate(MatrixXd::Zero(3,1));

                        Matrix<double,3,1> measurementVector;
                        measurementVector << minDistance, 0, 0;


                        MatrixXd state = kf.updateEstimate(measurementVector);

                        double distance = state(0,0);
                        double velocity = state(1,0);

                        ETA = (totalDistance - distance) / velocity;
                        std::cout << "\n--- Real-time Update ---\n";
                        std::cout << "Current Position: " << minDistance << "m from start\n";
                        std::cout << "Filtered Position: " << distance << "m from start\n";
                        std::cout << "Estimated Velocity: " << velocity << " m/s\n";

                        if (ETA < 0) busComes++;
                        if (busComes == 3) {
                            goneBuses.insert(closestVehicle->getTrip());
                            busComes = 0;
                        }



                        std::cout << "ETA: " << ETA << " seconds (" << ETA/60 << " minutes)\n";
                        utils.storeResults(stop_id, bus_line, direction, ETA);
                    } catch (std::exception &e) {
                        std::cerr << "Error updating Kalman filter: " << e.what() << std::endl;
                    }
                }

                if (!busFound) {
                    std::cout << "Bus not found in current data. Retrying..." << std::endl;
                }
                this->portoParser->destroyVehicles();
                std::this_thread::sleep_for(std::chrono::seconds(30));
            }
        }
    END:

    //clearScreen();
    std::cout << "Your bus has arrived!\n";
    std::cout << "\n\nWhat would you like to do next:\n"
              << "1 - Return to main menu.\n"
              << "2 - Exit " << "\n";

    std::string opt;
    std::cout << "Input: ";
    std::cin >> opt;
    std::cout << "\n";

    int processedKey = processKey(opt);

    switch (processedKey) {
        case 1:
            showMainMenu();
        case 2:
            std::cout << "Thank you very much and Bye-Bye.\n";
            delay(4);
            break;
        default:
            std::cout << "\n* Error while parsing option, please input a valid numeric option. *\n";
            showMainMenu();
    }

    }

