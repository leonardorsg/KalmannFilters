//
// Created by leonardorsg on 2/28/25.
//

#ifndef TRIPS_H
#define TRIPS_H
#include <string>

class Trips {
    private:
        int trip_id;
        std::string route_id;
        int direction_id;
        std::string service_id;
        std::string trip_headsign;
        std::string shape_id;


    public:
        Trips() : trip_id(0), route_id(""), direction_id(0), service_id(""), trip_headsign(""), shape_id("") {}

        Trips(int trip_id, std::string route_id, int direction_id, std::string service_id, std::string trip_headsign, std::string shape_id) {
            this->trip_id = trip_id;
            this->route_id = route_id;
            this->direction_id = direction_id;
            this->service_id = service_id;
            this->trip_headsign = trip_headsign;
            this->shape_id = shape_id;
        }

        //Getters
        int getTripId() const { return trip_id; }
        std::string getRouteId() const { return route_id; }
        int getDirectionId() const { return direction_id; }
        std::string getServiceId() const { return service_id; }
        std::string getTripHeadsign() const { return trip_headsign; }
        std::string getShapeId() const { return shape_id; }

        //Setters
        void setTripId(int trip_id) { this->trip_id = trip_id; }
        void setRouteId(const std::string &route_id) { this->route_id = route_id; }
        void setDirectionId(int direction_id) { this->direction_id = direction_id; }
        void setServiceId(const std::string &service_id) { this->service_id = service_id; }
        void setTripHeadsign(const std::string &trip_headsign) { this->trip_headsign = trip_headsign; }
        void setShapeId(const std::string &shape_id) {this->shape_id = shape_id; }

        std::string toString() {
            return "Trip ID: " + std::to_string(trip_id) + "\n" +
                   "Route ID: " + route_id + "\n" +
                   "Direction ID: " + std::to_string(direction_id) + "\n" +
                   "Service ID: " + service_id + "\n" +
                   "Trip Headsign: " + trip_headsign + "\n";
        }
    
};

#endif //TRIPS_H
