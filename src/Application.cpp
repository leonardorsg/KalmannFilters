
#include "Application.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <thread>
#include <vector>
#include <../services/apply_metrics.h>

using namespace std;
using namespace std::chrono;
using Eigen::MatrixXd;
using Eigen::VectorXd;

void Application::clearScreen() {
    if (env == "win") {
        // print many newlines on Windows
        cout << string(50, '\n');
    } else {
        // use ANSI escape sequence on unix
        system("clear");
    }
}

void Application::delay(long sleepMS) {
    this_thread::sleep_for(chrono::milliseconds(sleepMS));
}

int Application::processKey(const string &option) {
    try {
        int intOPT = stoi(option);
        if (intOPT < 0 || option.size() > 1 || intOPT > 6) throw invalid_argument("Invalid option");
        return intOPT;
    } catch (invalid_argument &) {
        cout << "\n* Error while parsing option, please input a valid numeric option. *\n";
        delay(2000);
        return -1;
    }
}

void Application::run(int processedKey) {
    clearScreen();
    switch (processedKey) {
        case 1: {
            string stop_id;
            cout << "Please enter the stop id: ";
            cin >> stop_id;
            cout << '\n';
            string bus_line;
            cout << "Please enter the bus line: ";
            cin >> bus_line;
            cout << '\n';
            int direction;
            cout << "0 - Outbound Travel\n";
            cout << "1 - Inbound Travel\n";
            cout << "Please enter the direction: ";
            cin >> direction;
            cout << '\n';
            runKalmannFilter(bus_line, stop_id, direction);
            break;
        }
        case 2:
            cout << "Thank you very much and Bye-Bye.\n";
            delay(4000);
            break;
        default:
            showMainMenu();
            break;
    }
}

void Application::showMainMenu() {
    string opt;
    cout << "\nSelect an operation you would like to do:\n\n"
         << "1 - Execute algorithm.\n"
         << "2 - Exit.\n"
         << "Input: ";
    cin >> opt;
    cout << "\n";
    int processedKey = processKey(opt);
    if (processedKey == -1) {
        showMainMenu();
    } else {
        run(processedKey);
    }
}

void Application::showGoBackMenu(int /*functionNumber*/, const string &functionName) {
    // Simple implementation: ask user to press enter and then return to main menu
    cout << "\nPress enter to continue";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
    clearScreen();
    cout << "\n\nWhat would you like to do next:\n"
         << "1 - Return to main menu.\n"
         << "2 - (again) " << functionName << "\n"
         << "Input: ";
    string opt;
    cin >> opt;
    cout << '\n';
    int processedKey = processKey(opt);
    if (processedKey == 1) {
        throw invalid_argument("Go back");
    }
    // otherwise the function repeats
}

void Application::runKalmannFilter(string bus_line, string stop_id, int direction) {
    int dataChoice;
    cout << "Choose the type of data you want to use:\n"
         << "1. Virtual Data (Random).\n"
         << "2. GPS Measurements.\n"
         << "Select: ";
    cin >> dataChoice;

    if (!bus_line.empty() && (bus_line.back() == 'm' || bus_line.back() == 'M')) {
        bus_line.pop_back();
    }

    const auto &trips  = portoParser->getTrips();
    const auto &stops  = portoParser->getStops();
    const auto &shapes = portoParser->getShapes();

    // Make stop_id lowercase
    transform(stop_id.begin(), stop_id.end(), stop_id.begin(), ::tolower);

    if (!stops.count(stop_id)) throw runtime_error("Invalid stop ID");

    string shape_id;
    for (const auto &tripPair : trips) {
        if (tripPair.second.getRouteId() == bus_line && tripPair.second.getDirectionId() == direction) {
            shape_id = tripPair.second.getShapeId();
            break;
        }
    }
    if (shape_id.empty()) throw runtime_error("Invalid bus line or direction");

    const auto &stop = stops.at(stop_id);
    Coordinates stop_coordinates = {stod(stop.getStopLat()), stod(stop.getStopLon())};

    double totalDistance = Utils::calculateBusDistance(shapes, shape_id, stop_coordinates);
    if (totalDistance < 0) throw runtime_error("Could not calculate route distance");
    cout << "Total Distance: " << totalDistance << '\n';

    // Kalman Filter (2 estados)
    double dt = 120.0;
    Eigen::Matrix2d Q;
    Q << 100.0, 0.0,
         0.0, 25.0;

    Eigen::Matrix<double,1,1> R;
    R << 25.0;

    Eigen::Vector2d x0;
    x0 << 0.0, 0.0;

    Eigen::Matrix2d P0 = Eigen::Matrix2d::Identity() * 15;

    KalmanFilter kf(dt, Q, R, x0, P0);

    // Aplicar métricas contextuais da API
    double eta_delay_from_api = 0.0;
    applyContextualMetricsToKalman(kf, bus_line, stop_id, direction, eta_delay_from_api);

    Utils utils;
    double ETA = -1.0;

    if (dataChoice == 1) {
        auto measurements = Utils::generateMeasurements(100, totalDistance);
        auto times = Utils::generateTimes(100);
        unsigned int maxSamples = static_cast<unsigned int>(measurements.rows());

        for (unsigned int i = 0; i < maxSamples; ++i) {
            kf.predict();
            Eigen::Matrix<double,1,1> z;
            z << measurements(i, 0);
            kf.update(z);

            Eigen::Vector2d state = kf.state();
            double distance = state(0);
            double velocity = std::clamp(state(1), 0.1, 30.0);

            if (distance > 10.0) {
                ETA = (totalDistance - distance) / velocity + eta_delay_from_api;
                cout << "ETA: " << ETA << "s"
                     << " | Travelled: " << distance
                     << " | Velocity: " << velocity << " m/s\n";
            }
        }

    } else if (dataChoice == 2) {
        int busComes = 0;
        set<int> goneBuses;
        bool busFound = false;

        while (true) {
            try {
                portoParser->parseVehicles(string(JSON_FILE_PATH));
                portoParser->saveVehiclesToFile("vehicle_data.txt");
            } catch (const exception &e) {
                cerr << "Error updating vehicle data: " << e.what() << endl;
                this_thread::sleep_for(seconds(30));
                continue;
            }

            /* Verifica todos os veículos ativos da linha e direção especificadas,
            // e seleciona aquele que está mais próximo da paragem ao longo do percurso (shape).
             A comparação é feita com base na distância acumulada desde o início do trajeto (calculateBusDistance),
             garantindo que apenas veículos que ainda não passaram pela paragem (ou seja, com distância relativa positiva)
             sejam considerados. O veículo selecionado é aquele cuja distância restante até a paragem é a menor.
            */

            const auto &vehicles = portoParser->getVehicles();
            const Vehicle* closestVehicle = nullptr;
            double stopProgress = Utils::calculateBusDistance(shapes, shape_id, stop.getLocation());
            double minRelativeDistance = std::numeric_limits<double>::max();

            for (const auto &vehicle : vehicles) {
                if (vehicle.getRouteId() == stoi(bus_line)
                    && vehicle.getDirection() == direction
                    && goneBuses.find(vehicle.getTrip()) == goneBuses.end()) {
                    
                    double vehicleProgress = Utils::calculateBusDistance(shapes, shape_id, vehicle.getCoordinates());

                    if (vehicleProgress < 0) continue; // invalid position

                    double relativeDistance = stopProgress - vehicleProgress;

                    if (relativeDistance > 0 && relativeDistance < minRelativeDistance) {
                        minRelativeDistance = relativeDistance;
                        closestVehicle = &vehicle;
                    }
                }
            }

            double vehicleProgress = closestVehicle ? Utils::calculateBusDistance(shapes, shape_id, closestVehicle->getCoordinates()) : -1;

            if (closestVehicle) {
                busFound = true;

                kf.predict();
                Eigen::Matrix<double,1,1> z;
                z << minRelativeDistance;
                kf.update(z);

                Eigen::Vector2d state = kf.state();
                double distance = state(0);
                double velocity = std::clamp(state(1), 0.1, 30.0);
                ETA = (totalDistance - distance) / velocity + eta_delay_from_api;

                cout << "\n--- Real-time Update ---\n";
                cout << "Current Position: " << minRelativeDistance << "m from stop\n";
                cout << "Filtered Position: " << distance << "m from start of route\n";
                cout << "Estimated Velocity: " << velocity << " m/s\n";
                cout << "ETA (corrigido): " << ETA << " seconds (" << ETA / 60.0 << " minutes)\n";

                cout << "Vehicle ID: " << closestVehicle->getRouteId() << "\n";
                cout << "Trip ID: " << closestVehicle->getTrip() << "\n";


                if (ETA <= 0) busComes++;
                if (busComes == 3) {
                    goneBuses.insert(closestVehicle->getTrip());
                    busComes = 0;
                    break;
                }

                utils.storeResults(stop_id, bus_line, direction, ETA);
            }

            if (!busFound) {
                cout << "The bus didn't begin its route yet. Retrying...\n";
            }

            portoParser->destroyVehicles();
            this_thread::sleep_for(seconds(120));
        }
    }

    cout << "Your bus has arrived!\n\nWhat would you like to do next:\n"
         << "1 - Return to main menu.\n"
         << "2 - Exit\nInput: ";
    string opt;
    cin >> opt;
    cout << '\n';

    int processedKey = processKey(opt);
    if (processedKey == 1) showMainMenu();
}
