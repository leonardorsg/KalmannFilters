
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
            clearScreen();
            runKalmannFilter(bus_line, stop_id, direction);
            break;
        }
        case 2:
            cout << "Thank you very much and Bye-Bye.\n";
            delay(4000);
            exit(0);
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
         << "3. GTFS Data.\n"
         << "Select: ";
    cin >> dataChoice;
    clearScreen();

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
    double dt = 60.0;
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
        bool firstMeasurement = true;

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
            
            if (closestVehicle) {

                // Inicializa o estado do filtro na primeira medição real
                if (firstMeasurement) {
                    Eigen::Vector2d x0;
                    x0 << minRelativeDistance, 6.0;
                    kf.setState(x0);

                    Eigen::Matrix2d P0 = Eigen::Matrix2d::Identity() * 300;
                    kf.setCovariance(P0);

                    cout << "Inicializando filtro com distancia: " << minRelativeDistance << "m"
                        << " e velocidade estimada: " << 6.0 << " m/s\n";

                    firstMeasurement = false;
                }

                

                busFound = true;

                kf.predict();
                Eigen::Matrix<double,1,1> z;
                z << minRelativeDistance;
                kf.update(z);

                Eigen::Vector2d state = kf.state();
                double distance = state(0);
                double velocity = std::clamp(state(1), 0.1, 30.0);
                ETA = distance / velocity + eta_delay_from_api;

                cout << "\n--- Real-time Update ---\n";
                cout << "Current Position: " << minRelativeDistance << "m from stop\n";
                cout << "Filtered Position: " << distance << "m\n";
                cout << "Estimated Velocity: " << velocity << " m/s\n";
                cout << "ETA (corrigido): " << ETA << " seconds (" << ETA / 60.0 << " minutes)\n";

                cout << "Vehicle ID: " << closestVehicle->getRouteId() << "\n";
                cout << "Trip ID: " << closestVehicle->getTrip() << "\n";


                if (ETA <= 0) busComes++;
                if (busComes == 2) {
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
            this_thread::sleep_for(seconds(60));
        }
    } else if (dataChoice == 3){
        std::cout << "\nUsing GTFS data to estimate upcoming arrivals at stop " << stop_id << "...\n";

        std::time_t now = std::time(nullptr);
        std::tm* local_time = std::localtime(&now);
        int current_minutes = local_time->tm_hour * 60 + local_time->tm_min;

        const int timeWindowBefore = 0;     // do agora até +15 min
        const int timeWindowAfter = 15;     // minutos no futuro

        std::vector<std::pair<std::string, int>> upcomingTrips; // trip_id, eta

        for (const auto& [trip_id, trip] : trips) {
            if (trip.getRouteId() != bus_line || trip.getDirectionId() != direction) continue;

            const auto& stopTimesMap = stop.getStopTimesMap();
            auto stopTimeIt = stopTimesMap.find(trip_id);
            if (stopTimeIt == stopTimesMap.end()) continue;

            auto stopTime = stopTimeIt->second;
            const std::string& arrival = stopTime.getArrivalTime();

            int h, m, s;
            if (sscanf(arrival.c_str(), "%d:%d:%d", &h, &m, &s) != 3) continue;

            int arrival_minutes = h * 60 + m;
            int diff = arrival_minutes - current_minutes;

            if (diff >= timeWindowBefore && diff <= timeWindowAfter) {
                upcomingTrips.emplace_back(trip_id, diff);
            }
        }

        if (upcomingTrips.empty()) {
            std::cout << "No trips scheduled to arrive at this stop in the next 15 minutes.\n";
            std::cout << "Please try again later or check the schedule.\n";
            showGoBackMenu(0, "Check upcoming trips");
            return;
        } 
        
        int minEta = std::numeric_limits<int>::max();
        int maxEta = std::numeric_limits<int>::min();
        for (const auto& [trip_id, eta] : upcomingTrips) {
            minEta = std::min(minEta, eta);
            maxEta = std::max(maxEta, eta);
        }

        // --- Fetch contextual delay metrics from API ---
        double delay_mean = 0.0;
        double delay_std = 0.0;

        try {
            auto metrics = getContextualDelayMetrics(bus_line, stop_id, direction); 
            delay_mean = metrics.first;     // avg_delay in seconds
            delay_std = metrics.second;     // std_deviation in seconds
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not load API delay metrics. Using GTFS only.\n";
        }

        // --- Adjust ETA range using delay metrics ---
        int corrected_min_eta_sec = minEta * 60 + delay_mean;
        int corrected_max_eta_sec = maxEta * 60 + delay_mean;

        int eta_low_bound = static_cast<int>(std::max(0.0, corrected_min_eta_sec - delay_std));
        int eta_high_bound = static_cast<int>(corrected_max_eta_sec + delay_std);

        std::cout << "\nAdjusted ETA window (GTFS + real delay context):\n";
        std::cout << "Estimated arrival between "
          << eta_low_bound / 60 << " and "
          << eta_high_bound / 60 << " minutes\n";

        // Give a probability of x% that the bus will arrive within the next 10 minutes
        int eta_10_min = 10 * 60; // 10 minutes in seconds
        double probability = 0.0;
        if (eta_low_bound <= eta_10_min && eta_high_bound >= eta_10_min && delay_std > 0.0) {
            double mean = (corrected_min_eta_sec + corrected_max_eta_sec) / 2.0;
            double z = (eta_10_min - mean) / delay_std;
            probability = 0.5 * (1.0 + std::erf(z / std::sqrt(2.0))); // CDF normal
        }

        std::cout << "Probability of arrival within 10 minutes: " << probability * 100 << "%\n";

        std::cout << "\nPress any key to return to the main menu...\n";
        std::cin.ignore();
        std::cin.get();
        showMainMenu();
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
