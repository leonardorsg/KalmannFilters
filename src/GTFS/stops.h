//
// Created by leonardorsg on 2/28/25.
//

#ifndef STOPS_H
#define STOPS_H
#include <string>
#include <unordered_map>

class Stops_times {
    private:
        std::string trip_id;
        std::string arrival_time;
        std::string departure_time;
        std::string stop_id;
        std::string stop_sequence;

    public:

    Stops_times()
    : trip_id(""), stop_id(""), arrival_time(""), departure_time(""),
      stop_sequence(""){}



        Stops_times(std::string trip_id, std::string arrival_time, std::string departure_time, std::string stop_id, std::string stop_sequence) {
            this->stop_id = stop_id;
            this->stop_sequence = stop_sequence;
            this->trip_id = trip_id;
            this->arrival_time = arrival_time;
            this->departure_time = departure_time;

        }

        // Getters
        std::string getTripId() const { return trip_id; }
        std::string getArrivalTime() const { return arrival_time; }
        std::string getDepartureTime() const { return departure_time; }
        std::string getStopId() const { return stop_id; }
        std::string getStopSequence() const { return stop_sequence; }

        // Setters
        void setTripId(const std::string &trip_id) { this->trip_id = trip_id; }
        void setArrivalTime(const std::string &arrival_time) { this->arrival_time = arrival_time; }
        void setDepartureTime(const std::string &departure_time) { this->departure_time = departure_time; }
        void setStopId(const std::string &stop_id) { this->stop_id = stop_id; }
        void setStopSequence(const std::string &stop_sequence) { this->stop_sequence = stop_sequence; }
};

class Stops {
    private:
        std::string stop_id;
        std::string stop_code;
        std::string stop_name;
        std::string stop_lat;
        std::string stop_lon;
        std::string zone_id;
        std::string stop_url;
        std::string location_type;
        //hash table to store Stops_times objects
        std::unordered_map<std::string, Stops_times> stop_times_map;

    public:
    Stops()
    : stop_id(""), stop_code(""), stop_name(""), stop_lat(""),
      stop_lon(""), zone_id(""), stop_url("") {}


        Stops(std::string stop_id, std::string stop_code, std::string stop_name, std::string stop_lat, std::string stop_lon, std::string zone_id, std::string stop_url) {
            this->stop_id = stop_id;
            this->stop_code = stop_code;
            this->stop_name = stop_name;
            this->stop_lat = stop_lat;
            this->stop_lon = stop_lon;
            this->zone_id = zone_id;
            this->stop_url = stop_url;

        }


        //Getters
        std::string getStopId() const { return stop_id; }
        std::string getStopCode() const { return stop_code; }
        std::string getStopName() const { return stop_name; }
        std::string getStopLat() const { return stop_lat; }
        std::string getStopLon() const { return stop_lon; }
        std::string getZoneId() const { return zone_id; }
        std::string getStopUrl() const { return stop_url; }

        //Setters
        void setStopId(const std::string &stop_id) { this->stop_id = stop_id; }
        void setStopCode(const std::string &stop_code) { this->stop_code = stop_code; }
        void setStopName(const std::string &stop_name) { this->stop_name = stop_name; }
        void setStopLat(const std::string &stop_lat) { this->stop_lat = stop_lat; }
        void setStopLon(const std::string &stop_lon) { this->stop_lon = stop_lon; }
        void setZoneId(const std::string &zone_id) { this->zone_id = zone_id; }
        void setStopUrl(const std::string &stop_url) { this->stop_url = stop_url; }

        std::string toString() {
            std::string result = "Stop Details:\n";
            result += "Stop ID: " + stop_id + "\n";
            result += "Stop Code: " + stop_code + "\n";
            result += "Stop Name: " + stop_name + "\n";
            result += "Stop Latitude: " + stop_lat + "\n";
            result += "Stop Longitude: " + stop_lon + "\n";
            result += "Zone ID: " + zone_id + "\n";
            result += "Stop URL: " + stop_url + "\n";

            result += "\nStop Times:\n";
            for (const auto &entry: stop_times_map) {
                const std::string &trip_id = entry.first;
                const Stops_times &stop_time = entry.second;

                result += "Trip ID: " + trip_id + "\n";
                result += "  Arrival Time: " + stop_time.getArrivalTime() + "\n";
                result += "  Departure Time: " + stop_time.getDepartureTime() + "\n";
                result += "  Stop ID: " + stop_time.getStopId() + "\n";
                result += "  Stop Sequence: " + stop_time.getStopSequence() + "\n";
            }

            return result;
        }



        // Getter for accessing the hash table
        std::unordered_map<std::string, Stops_times> &getStopTimesMap() {
            return stop_times_map;
        }

        // Method to add a Stops_times object to the hash table
        void addStopTime(const std::string &trip_id, const Stops_times &stop_time) {
            stop_times_map[trip_id] = stop_time;
        }

        // Method to retrieve a Stops_times object by trip_id
        Stops_times *getStopTime(const std::string &trip_id) {
            auto it = stop_times_map.find(trip_id);
            if (it != stop_times_map.end()) {
                return &(it->second);
            }
            return nullptr; // Return nullptr if trip_id is not found
        }

        // Method to check if a specific trip_id exists in the hash table
        bool hasStopTime(const std::string &trip_id) const {
            return stop_times_map.find(trip_id) != stop_times_map.end();
        }




};

#endif //STOPS_H
