//
// Created by leonardorsg on 2/28/25.
//

#ifndef AGENCY_H
#define AGENCY_H

#include <string>

class Agency {
    private:
      std::string agency_id;
      std::string agency_name;
      std::string agency_url;
      std::string agency_timezone;


    public:
      Agency(std::string agency_id, std::string agency_name, std::string agency_url, std::string agency_timezone){
          this->agency_id = agency_id;
          this->agency_name = agency_name;
          this->agency_url = agency_url;
          this->agency_timezone = agency_timezone;
      }

      std::string getAgencyId(){
        return this->agency_id;
      }

      void setAgencyId(std::string agency_id){
        this->agency_id = agency_id;
      }

      std::string getAgencyName(){
        return this->agency_name;
      }

      void setAgencyName(std::string agency_name){
        this->agency_name = agency_name;
      }

      std::string getAgencyUrl(){
        return this->agency_url;

      }

      void setAgencyUrl(std::string agency_url){
        this->agency_url = agency_url;
      }

      std::string getAgencyTimezone(){
        return this->agency_timezone;
      }

      void setAgencyTimezone(std::string agency_timezone){
        this->agency_timezone = agency_timezone;
      }


};


#endif //AGENCY_H
