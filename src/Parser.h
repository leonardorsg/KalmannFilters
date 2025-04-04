//
// Created by leona on 20/02/2025.
//

#ifndef DA_TSP_PROJ2_PARSER_H
#define DA_TSP_PROJ2_PARSER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iterator>
#include <cctype>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "GTFS/agency.h"
#include "GTFS/calendar.h"
#include "GTFS/route.h"
#include "GTFS/stops.h"
#include "GTFS/trips.h"
#include "RealTime/vehicle.h"


class Parser {
    private:
        bool parsed = false;
        std::string agency_file;
        std::string calendar_file;
        std::string routes_file;
        std::string stops_times_file;
        std::string stops_file;
        std::string trips_file;
        std::string shapes_file;


        std::list<Agency> agencyList;
        std::list<Calendar> calendarList;

        std::vector<Vehicle> vehicles;
        std::unordered_map<int, Route> routeHashTable; // Key: route_id, Value: Route object
        std::unordered_map<std::string, Stops> stopsHashTable; // Key: stop_id, Value: Stops object
        std::unordered_map<std::string, Trips> tripsHashTable; // Key: trip_id, Value: Trips object
        std::map<std::string, std::vector<Coordinates>> shapes; //to calculate the shapes.txt



    public:

           // Methods for Route hash table
        void insertRoute(int route_id, const Route &route) {
            routeHashTable[route_id] = route;
        }

        void deleteRoute(int route_id) {
            routeHashTable.erase(route_id);
        }

        Route *lookupRoute(int route_id) {
            auto it = routeHashTable.find(route_id);
            if (it != routeHashTable.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void printAllRoutes() {
            for (auto &[key, route]: routeHashTable) {
                std::cout << "Route ID: " << key << " Route Data: " << route.toString() << std::endl;
            }
        }

        // Methods for Stops hash table
        void insertStop(const std::string& stop_id, const Stops &stop) {
            stopsHashTable[stop_id] = stop;
        }

        void deleteStop(const std::string& stop_id) {
            stopsHashTable.erase(stop_id);
        }

        Stops *lookupStop(const std::string& stop_id) {
            auto it = stopsHashTable.find(stop_id);
            if (it != stopsHashTable.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void printAllStops() {
            for (auto &[key, stop]: stopsHashTable) {
                stop.toString();
            }
        }

    void insertTrip(const std::string& trip_id, const Trips &trip) {
            // Validate trip_id isn't empty
            if (trip_id.empty()) {
                throw std::invalid_argument("Trip ID cannot be empty");
            }

            // Validate the trip object has valid data
            if (trip.getTripId().empty()) {
                throw std::invalid_argument("Trip object has empty trip_id");
            }

            // Make a copy with validated strings
            Trips safe_copy(
                trip.getTripId().empty() ? "" : trip.getTripId(),
                trip.getRouteId().empty() ? "" : trip.getRouteId(),
                trip.getDirectionId(),
                trip.getServiceId().empty() ? "" : trip.getServiceId(),
                trip.getTripHeadsign().empty() ? "" : trip.getTripHeadsign(),
                trip.getShapeId().empty() ? "" : trip.getShapeId()
            );

            tripsHashTable[trip_id] = safe_copy;
        }

        void deleteTrip(const std::string& trip_id) {
            tripsHashTable.erase(trip_id);
        }

        Trips *lookupTrip(std::string trip_id) {
            auto it = tripsHashTable.find(trip_id);
            if (it != tripsHashTable.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void printAllTrips() {
            for (auto &[key, trip]: tripsHashTable) {
                std::cout << trip.toString();
            }
        }

        /**
         * @brief Prints all Agency objects in the hash table.
         */
        void printAllAgencies() {
            for (auto& agency : agencyList) {
                std::cout << agency.getAgencyId() << " " << agency.getAgencyName() << " " << agency.getAgencyUrl() << " " << agency.getAgencyTimezone() << std::endl;
            }
        }

        void printAllCalendars() {
            for (auto& calendar : calendarList) {
                std::cout << calendar.toString() << std::endl;
            }
        }


        Parser(std::string agency_file, std::string calendar_file, std::string routes_file, std::string stops_times_file, std::string stops_file, std::string trips_file, std::string shapes_file) {
            this->agency_file = agency_file;
            this->calendar_file = calendar_file;
            this->routes_file = routes_file;
            this->stops_times_file = stops_times_file;
            this->stops_file = stops_file;
            this->trips_file = trips_file;
            this->shapes_file = shapes_file;

        }

    /**
     * @brief Helper function for data parsing
     * @details Helps check if a given csv contains headers.
     * @return -1 if there are no matching headers
     * @return n > 0 where n is the number of headers
     * */
    int countHeaders(const std::string& header) {
            std::stringstream ss(header);
            std::string word;
            int count = 0;
            while (std::getline(ss, word, ',')) {
                for (char& c : word) if (isdigit(c)) return -1;
                if (count == -1) break;
                count++;
            }

            if (std::getline(ss, word, '\r')) {
                for (char& c : word) if (isdigit(c)) return -1;

                count++;
            }
            return count;
        }

    int parseAgency() {
            std::ifstream file(agency_file);
            if (!file.is_open()) {
                std::cout << "Error opening file" << std::endl;
                return -1;
            }

            auto stringToLower = [&](std::string& label){for (char& c : label) c = (char)tolower(c);};
            std::string line;
            std::getline(file, line);
            stringToLower(line);
            if (countHeaders(line) < 0) return -1;

            while (std::getline(file, line)) {
                std::istringstream s(line);
                if (line.empty()) continue;
                std::string agency_id_str, agency_name, agency_url, agency_timezone, unused;
                
               if (!(std::getline(s, agency_id_str, ',') && std::getline(s, agency_name, ',') &&
                     std::getline(s, agency_url, ',') && std::getline(s, agency_timezone, ','))) {
                    std::cerr << "Error parsing agency file" << std::endl;
                    return -1;
                }
                std::getline(s, unused); // Read everything after the timezone into unused

                stringToLower(agency_name);
                stringToLower(agency_url);
                stringToLower(agency_timezone);
                Agency agency(agency_id_str, agency_name, agency_url, agency_timezone);
                std::cout << agency_name;
                agencyList.push_back(agency);
            }
            file.close();
            return 0;

        }
    int parseCalendar() {
            std::ifstream file(calendar_file);
            if (!file.is_open()) {
                std::cout << "Error opening file" << std::endl;
                return -1;
            }
            auto stringToLower = [&](std::string& label){for (char& c : label) c = (char)tolower(c);};
            auto stringToBool = [&](std::string& label){if (label == "1") return true; else return false;};
            std::string line;
            std::getline(file, line);
            stringToLower(line);
            if (countHeaders(line) < 0) return -1;

            while (std::getline(file, line)) {
                std::istringstream s(line);
                std::string service_id, monday, tuesday, wednesday, thursday, friday, saturday, sunday, start_date, end_date, unused;

                if (!(std::getline(s, service_id, ',') && std::getline(s, monday, ',') && std::getline(s, tuesday, ',') &&
                    std::getline(s, wednesday, ',') && std::getline(s, thursday, ',') && std::getline(s, friday, ',') &&
                    std::getline(s, saturday, ',') && std::getline(s, sunday, ',') && std::getline(s, start_date, ',') && std::getline(s, end_date, '\r'))) {
                    std::cerr << "Error parsing agency file" << std::endl;
                    return -1;
                      }
                std::getline(s, unused); // Read everything after the timezone into unused


                calendarList.emplace_back(service_id, stringToBool(monday), stringToBool(tuesday), stringToBool(wednesday), stringToBool(thursday), stringToBool(friday), stringToBool(saturday), stringToBool(sunday), start_date, end_date);
            }
            file.close();
            return 0;
        }
    int parseRoutes() {
            std::ifstream file(routes_file);
            if (!file.is_open()) {
                std::cout << "Error opening file" << std::endl;
                return -1;
            }
            auto stringToLower = [&](std::string& label){for (char& c : label) c = (char)tolower(c);};
            std::string line;
            std::getline(file, line);
            stringToLower(line);
            if (countHeaders(line) < 0) return -1;

            while (std::getline(file, line)) {
                std::istringstream s(line);
                std::string route_id, route_short_name, route_long_name, route_type, route_url, unused;
                if (!(std::getline(s, route_id, ',') && std::getline(s, route_short_name, ',') && std::getline(s, route_long_name, ',') &&
                    (std::getline(s, unused, ',')) && (std::getline(s, route_type, ',')) && (std::getline(s, route_url, '\r')))) {
                    std::cerr << "Error parsing agency file" << std::endl;
                    return -1;
                    }
                std::getline(s, unused); // Read everything after the timezone into unused

                stringToLower(route_short_name);
                stringToLower(route_long_name);
                stringToLower(route_url);
                Route route(std::stoi(route_id), route_short_name, route_long_name, std::stoi(route_type), route_url);
                insertRoute(std::stoi(route_id), route);
                   

            }
            file.close();
            return 0;
        }
    int parseStops() {
            std::ifstream file(stops_file);
            if (!file.is_open()) {
                std::cout << "Error opening file" << std::endl;
                return -1;
            }
            auto stringToLower = [&](std::string& label){for (char& c : label) c = (char)tolower(c);};
            std::string line;
            std::getline(file, line);
            stringToLower(line);
            if (countHeaders(line) < 0) return -1;

            while (std::getline(file, line)) {
                std::istringstream s(line);
                std::string stop_id, stop_code,stop_name, stop_lat, stop_lon, zone_id, stop_url, unused;
                if (!(std::getline(s, stop_id, ',') && std::getline(s, stop_code, ',') && (std::getline(s, stop_name, ','))&& std::getline(s, stop_lat, ',') &&
                    (std::getline(s, stop_lon, ',')) && (std::getline(s, zone_id, ',')) && (std::getline(s, stop_url, '\r')))) {
                    std::cerr << "Error parsing agency file" << std::endl;
                    return -1;
                    }
                std::getline(s, unused); // Read everything after the timezone into unused

                stringToLower(stop_id);
                stringToLower(stop_code);
                stringToLower(stop_name);
                stringToLower(stop_lat);
                stringToLower(stop_lon);
                stringToLower(zone_id);
                stringToLower(stop_url);
                Stops stop(stop_id, stop_code, stop_name, stop_lat, stop_lon, zone_id, stop_url);

                insertStop(stop_id, stop);
            }
            file.close();
            return 0;
        }
    int parseStopsTimes() {
            std::ifstream file(stops_times_file);
            if (!file.is_open()) {
                std::cout << "Error opening file" << std::endl;
                return -1;
            }
            auto stringToLower = [&](std::string& label){for (char& c : label) c = (char)tolower(c);};
            std::string line;
            std::getline(file, line);

            while (std::getline(file, line)) {
                std::istringstream s(line);
                std::string trip_id, arrival, departure, stop_id, stop_sequence, unused;
                if (!(std::getline(s, trip_id, ',') && std::getline(s, arrival, ',') && std::getline(s, departure, ',') &&
                    std::getline(s, stop_id, ',') && std::getline(s, stop_sequence, ','))) {
                    std::cerr << "Error parsing agency file" << std::endl;
                    return -1;
                    }
                std::getline(s, unused); // Read everything after the timezone into unused

                stringToLower(trip_id);
                stringToLower(stop_id);
                stringToLower(arrival);
                stringToLower(departure);
                Stops_times stop_times(trip_id, arrival, departure, stop_id, stop_sequence);
                Stops *stop = lookupStop(stop_id);
                if (stop == nullptr) {
                    std::cerr << "Stop not found" << std::endl;
                    return -1;
                }
                stop->addStopTime(trip_id,stop_times);
            }
            file.close();
            return 0;


        }
    int parseTrips() {
            std::ifstream file(trips_file);
            if (!file.is_open()) {
                std::cout << "Error opening file" << std::endl;
                return -1;
            }
            auto stringToLower = [&](std::string& label) {
                if (!label.empty()) {  // Check if string is not empty
                    for (char& c : label) c = (char)tolower(c);
                }
            };
            std::string line;
            std::getline(file, line);
            stringToLower(line);
            if (countHeaders(line) < 0) return -1;

            while (std::getline(file, line)) {
                if (line.empty()) continue;

                if (line.back() == '\r') {
                    line.pop_back();
                }

                std::istringstream s(line);
                std::string route_id,direction_id,service_id, trip_id,trip_headsign, unused, shape_id, wheelchair;
                // Read all fields with proper error checking
                if (!std::getline(s, route_id, ',') || route_id.empty() ||
                    !std::getline(s, direction_id, ',') || direction_id.empty() ||
                    !std::getline(s, service_id, ',') || service_id.empty() ||
                    !std::getline(s, trip_id, ',') || trip_id.empty() ||
                    !std::getline(s, trip_headsign, ',') ||
                    !std::getline(s, unused, ',') ||  // wheelchair_accessible
                    !std::getline(s, unused, ',') ||  // block_id
                    !std::getline(s, shape_id)) {     // shape_id
                    std::cerr << "Error parsing line: " << line << std::endl;
                    continue; // Skip malformed line but continue with others
                    }


                stringToLower(route_id);
                stringToLower(service_id);
                stringToLower(trip_headsign);
                stringToLower(shape_id);
                stringToLower(trip_id);

                try {
                    Trips trip(trip_id, route_id, std::stoi(direction_id), service_id, trip_headsign, shape_id);
                    insertTrip(trip_id, trip);
                } catch (const std::exception& e) {
                    std::cerr << "Error creating trip from line: " << line << "\nError: " << e.what() << std::endl;
                    continue;
                }

            }
            file.close();
            return 0;
        }



    void parseVehicles(const std::string &jsonFile) {
    std::ifstream file(jsonFile);
    if (!file) {
        throw std::runtime_error("Unable to open the JSON file: " + jsonFile);
    }

    // Parse JSON file using nlohmann::json
    nlohmann::json jsonData;
    file >> jsonData;

    for (const auto &busEntry : jsonData) {
        Vehicle vehicle;

        // Parse annotations for route, trip, and direction
        const auto annotations = busEntry["annotations"]["value"];
        for (const auto &annotation : annotations) {
            std::string annotationStr = annotation.get<std::string>();

            if (annotationStr.find("stcp%3Aroute%3A") != std::string::npos) {
                size_t pos = annotationStr.find_last_of("%3A");
                if (pos != std::string::npos) {
                    std::string routeIdStr = annotationStr.substr(pos + 1);
                    try {
                        vehicle.setRouteId(std::stoi(routeIdStr));
                    } catch (const std::invalid_argument &e) {
                        std::cerr << "Invalid route ID: " << routeIdStr << std::endl;
                    } catch (const std::out_of_range &e) {
                        std::cerr << "Route ID out of range: " << routeIdStr << std::endl;
                    }
                }
            } else if (annotationStr.find("stcp%3Anr_viagem%3A") != std::string::npos) {
                size_t pos = annotationStr.find_last_of("%3A");
                if (pos != std::string::npos) {
                    std::string tripStr = annotationStr.substr(pos + 1);
                    try {
                        vehicle.setTrip(std::stoi(tripStr));
                    } catch (const std::invalid_argument &e) {
                        std::cerr << "Invalid trip number: " << tripStr << std::endl;
                    } catch (const std::out_of_range &e) {
                        std::cerr << "Trip number out of range: " << tripStr << std::endl;
                    }
                }
            } else if (annotationStr.find("stcp%3Asentido%3A") != std::string::npos) {
                size_t pos = annotationStr.find_last_of("%3A");
                if (pos != std::string::npos) {
                    std::string directionStr = annotationStr.substr(pos + 1);
                    try {
                        vehicle.setDirection(std::stoi(directionStr));
                    } catch (const std::invalid_argument &e) {
                        std::cerr << "Invalid direction: " << directionStr << std::endl;
                    } catch (const std::out_of_range &e) {
                        std::cerr << "Direction out of range: " << directionStr << std::endl;
                    }
                }
            }
        }

        // Parse agency (data provider)
        if (busEntry.contains("dataProvider")) {
            vehicle.setAgencyId(busEntry["dataProvider"]["value"]);
        }

        // Parse location (latitude and longitude)
        if (busEntry.contains("location")) {
            const auto location = busEntry["location"]["value"];
            const auto coordinates = location["coordinates"];
            Coordinates newcoordinates = {coordinates[1].get<double>(),coordinates[0].get<double>()};
            vehicle.setCoordinates(newcoordinates);
        }

        // Add vehicle to the collection
        vehicles.push_back(vehicle);
    }
}

    // Read CSV and group coordinates by shape_id
    int parseShapes() {
            std::ifstream file(this->shapes_file);
            std::string line, shape_id;
            double lat, lon;

            // Skip header
            std::getline(file, line);

            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string temp;

                std::getline(ss, shape_id, ',');   // shape_id
                std::getline(ss, temp, ',');       // lat
                lat = std::stod(temp);
                std::getline(ss, temp, ',');       // lon
                lon = std::stod(temp);

                shapes[shape_id].push_back({lat, lon});
            }
            return 0;
        }

    std::string run() {
        if (parseAgency() < 0) return "Agency file is invalid";
        if (parseCalendar() < 0) return "Calendar file is invalid";
        if (parseRoutes() < 0) return "Routes file is invalid";
        if (parseStops() < 0) return "Stops file is invalid";
        if (parseStopsTimes() < 0) return "Stops times file is invalid";
        if (parseTrips() < 0) return "Trips file is invalid";
        if (parseShapes() < 0) return "Shapes file is invalid";
            return "Success";
    }



    // Getters
    //Retrive the list of trips
    const std::map<std::string, std::vector<Coordinates>>& getShapes() {
            return shapes;
        }

    // Retrieve the list of vehicles
    const std::vector<Vehicle>& getVehicles() const {
            return vehicles;
        }

    std::list<Agency> getAgencies () {return agencyList;}
    std::list<Calendar> getCalendars () {return calendarList;}
    std::vector<Vehicle> getVehicles () {return vehicles;}
    std::unordered_map<int, Route> getRoutes () {return routeHashTable;}
    std::unordered_map<std::string, Stops> getStops () {return stopsHashTable;}
    std::unordered_map<std::string, Trips> getTrips () {return tripsHashTable;}

    //Print Fields

    std::string printAgencies() {
            std::ostringstream os;
            os << "AGENCIES: \n";
            for (auto agency : agencyList) {
                os << "ID: " << agency.getAgencyId() << "\n";
                os << "Name: " << agency.getAgencyName() << "\n";
                os << "Timezone: " << agency.getAgencyTimezone() << "\n";
                os << "URL: " << agency.getAgencyUrl() << "\n";
            }
            return os.str();
        }

    std::string printCalendar() {
            std::ostringstream os;
            os << "CALENDAR: \n";
            for (auto calendar : calendarList) {
                os << "Start Date: " << calendar.getStartDate() << "\n";
                os << "End  Date: " << calendar.getEndDate() << "\n";
                os << "Service ID: "<< calendar.getServiceId() << "\n";
            }
            return os.str();
        }

    std::string printRoutes() {
            std::ostringstream os;
            os << "ROUTES: \n";
            for (auto route : routeHashTable) {
                os << route.second.toString();
            }
            return os.str();
        }

    std::string printStops() {
            std::ostringstream os;
            os << "STOPS: \n";
            for (auto stop : stopsHashTable) {
                os << stop.second.toString();
            }
            return os.str();
        }

    std::string printTrips() {
            std::ostringstream os;
            os << "TRIPS: \n";
            for (auto trip : tripsHashTable) {
                os << trip.second.toString();

            }
            return os.str();
        }
};

#endif //DA_TSP_PROJ2_PARSER_H
