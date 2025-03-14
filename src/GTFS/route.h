//
// Created by leonardorsg on 2/28/25.
//

#ifndef ROUTE_H
#define ROUTE_H
#include <string>

class Route {
    private:
        int route_id;
        std::string route_short_name;
        std::string route_long_name;
        int route_type;
        std::string route_url;
    public:

        Route() : route_id(0), route_short_name(""), route_long_name(""), route_type(0), route_url("") {}

        Route(int route_id, std::string route_short_name, std::string route_long_name, int route_type, std::string route_url) {
            this->route_id = route_id;
            this->route_short_name = route_short_name;
            this->route_long_name = route_long_name;
            this->route_type = route_type;
            this->route_url = route_url;
        }

        //Getters
        int getRouteId() const { return route_id; }
        std::string getRouteShortName() const { return route_short_name; }
        std::string getRouteLongName() const { return route_long_name; }
        int getRouteType() const { return route_type; }
        std::string getRouteUrl() const { return route_url; }


        //Setters
        void setRouteId(int route_id) { this->route_id = route_id; }
        void setRouteShortName(const std::string &route_short_name) { this->route_short_name = route_short_name; }
        void setRouteLongName(const std::string &route_long_name) { this->route_long_name = route_long_name; }
        void setRouteType(int route_type) { this->route_type = route_type; }
        void setRouteUrl(const std::string &route_url) { this->route_url = route_url; }

        std::string toString() {
            return "Short name: " + route_short_name + "\n" + " Long name: " + route_long_name + "\n"+ " Type: " + std::to_string(route_type) + "\n" + " Url: " + route_url;
        }
};
#endif //ROUTE_H
