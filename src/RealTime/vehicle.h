//
// Created by leonardorsg on 2/28/25.
//

#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
#include "Utils.h"



class Vehicle {

private:
    int route_id;
    int direction;
    int trip;
    std::string agency_id;
    Coordinates coordinates;


public:

    Vehicle() : route_id(0), direction(0), agency_id(""), coordinates{0,0}, trip(0) {}

    Vehicle(int route_id, int direction, std::string agency_id, std::string latitude, std::string longitude, int trip) {
        this->route_id = route_id;
        this->direction = direction;
        this->agency_id = agency_id;

        this->coordinates.Latitude = std::stod(latitude);
        this->coordinates.Longitude = std::stod(longitude);
        this->trip = trip;
    }

    //Getters
    int getRouteId() const { return route_id; }
    int getDirection() const { return direction; }
    std::string getAgencyId() const { return agency_id; }
    Coordinates getCoordinates() const { return coordinates; }
    int getTrip() const { return trip; }

    //Setters
    void setRouteId(int route_id) { this->route_id = route_id; }
    void setDirection(int direction) { this->direction = direction; }
    void setAgencyId(const std::string &agency_id) { this->agency_id = agency_id; }
    void setCoordinates(Coordinates coordinates) { this->coordinates = coordinates; }
    void setTrip(int trip) { this->trip = trip; }

};

#endif //VEHICLE_H
