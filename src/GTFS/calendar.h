//
// Created by leonardorsg on 2/28/25.
//

#ifndef CALENDAR_H
#define CALENDAR_H

#include <list>
#include <map>
#include <string>
#include <utility>

class Calendar {
private:
    std::string service_id;
    bool monday;
    bool tuesday;
    bool wednesday;
    bool thursday;
    bool friday;
    bool saturday;
    bool sunday;
    std::string start_date;
    std::string end_date;
    std::list<std::map<std::string, int> > exception_date;

public:
    Calendar(std::string service_id, bool monday, bool tuesday, bool wednesday, bool thursday, bool friday,
             bool saturday, bool sunday, std::string start_date, std::string end_date) {
        this->service_id = std::move(service_id);
        this->monday = monday;
        this->tuesday = tuesday;
        this->wednesday = wednesday;
        this->thursday = thursday;
        this->friday = friday;
        this->saturday = saturday;
        this->sunday = sunday;
        this->start_date = std::move(start_date);
        this->end_date = std::move(end_date);
    }

    // Getters
    std::string getServiceId() const { return service_id; }
    bool isMonday() const { return monday; }
    bool isTuesday() const { return tuesday; }
    bool isWednesday() const { return wednesday; }
    bool isThursday() const { return thursday; }
    bool isFriday() const { return friday; }
    bool isSaturday() const { return saturday; }
    bool isSunday() const { return sunday; }
    std::string getStartDate() const { return start_date; }
    std::string getEndDate() const { return end_date; }

    // Setters
    void setServiceId(const std::string &id) { service_id = id; }
    void setMonday(bool value) { monday = value; }
    void setTuesday(bool value) { tuesday = value; }
    void setWednesday(bool value) { wednesday = value; }
    void setThursday(bool value) { thursday = value; }
    void setFriday(bool value) { friday = value; }
    void setSaturday(bool value) { saturday = value; }
    void setSunday(bool value) { sunday = value; }
    void setStartDate(const std::string &date) { start_date = date; }
    void setEndDate(const std::string &date) { end_date = date; }

    //Methods
    void addExceptionDate(std::string date, int exception_type) {
        std::map<std::string, int> exception;
        exception[date] = exception_type;
        exception_date.push_back(exception);
    }

    std::string toString() {
        std::stringstream ss;
        ss << "Service ID: " << service_id << "\n"
                << "Service Days:\n"
                << "  Monday: " << (monday ? "Yes" : "No") << "\n"
                << "  Tuesday: " << (tuesday ? "Yes" : "No") << "\n"
                << "  Wednesday: " << (wednesday ? "Yes" : "No") << "\n"
                << "  Thursday: " << (thursday ? "Yes" : "No") << "\n"
                << "  Friday: " << (friday ? "Yes" : "No") << "\n"
                << "  Saturday: " << (saturday ? "Yes" : "No") << "\n"
                << "  Sunday: " << (sunday ? "Yes" : "No") << "\n"
                << "Start Date: " << start_date << "\n"
                << "End Date: " << end_date << "\n";

        ss << "Exception Dates:\n";
        for (const auto &exception: exception_date) {
            for (const auto &entry: exception) {
                ss << "  Date: " << entry.first << ", Exception Type: " << entry.second << "\n";
            }
        }

        return ss.str();
    }
};


#endif //CALENDAR_H
