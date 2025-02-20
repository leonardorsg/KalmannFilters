#ifndef APLICATION_H
#define APLICATION_H

#include <fstream>
#include <sstream>
#include <thread>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <regex>
#include <iomanip>
#include <map>

#include "Graph.h"

#define DATASET_PATHS "../dataset/"

/**@brief Class that manages the menu.*/
class Application {
public:
    Application(std::string env) {
        this->env = std::move(env);
        restartConstructor:
        clearScreen();

        std::string choiceString;
        int choice;
        std::cout << "Select a city:\n";
        std::cout << "1. Porto\n";
        std::cout << "2. Boston\n";
        std::cout << "Enter your choice: ";
        std::cin >> choiceString;

        // parse city based on choice
        clearScreen();
    }

    void clearScreen();
    void run(int processedKey);
    static void delay(long sleepTime);
    static int processKey(const std::string &option);

private:

    /* Private functions */
    std::string showMainMenu();
    void showGoBackMenu(int option, const std::string& functionName);


    /* Attributes */
    std::string env;

};

#endif //APLICATION_H