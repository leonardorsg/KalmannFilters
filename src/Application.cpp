#include "Application.h"

void Application::run(int processedKey) {
    L1:
    clearScreen();

    switch (processedKey) {
        case 1: {
            std::string stop_id;
            std::cout << "Please enter the stop id: ";
            std::cin >> stop_id;
            std::cout << std::endl;


            std::string bus_line;
            std::cout << "Please enter the bus line: ";
            std::cin >> bus_line;
            std::cout << std::endl;

            int direction;
            std::cout << "0 - Outbound Travel\n";
            std::cout << "1 - Outbound Travel\n";
            std::cout << "Please enter the direction: ";

            std::cin >> direction;
            std::cout << std::endl;

            runKalmannFilter(bus_line, stop_id, direction);
            break;
        }
        case 2:
            std::cout << "Thank you very much and Bye-Bye.\n";
            delay(4);
            break;
        default:
            goto L1;
    }
}

/**
 * @brief clears (or simulates clearing) the screen, depending on the OS used
 */
void Application::clearScreen() {
    if (env == "win")
        L1:
        std::cout << "\n\n\n\n\n\n\n\n\n"
                     "\n\n\n\n\n\n\n\n\n"
                     "\n\n\n\n\n\n\n\n\n"
                     "\n\n\n\n\n\n\n\n\n";
    else if (env == "unix")
        if ( system("clear") == -1 ) goto L1;
}

/**
 * @brief Small delay for design purposes
 */
void Application::delay(long sleepMS) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMS));
}


/** @brief Handles the exception thrown by std::stoi. */
int Application::processKey(const std::string& option) {
    try {
        int intOPT = std::stoi(option);
        if (intOPT < 0 || option.size() > 1 || intOPT > 6) throw std::invalid_argument("NegativeNumber");
        return intOPT;
    } catch (std::invalid_argument& argument) {
        std::cout << "\n* Error while parsing option, please input a valid numeric option. *\n";
        delay(2000);
        return -1;
    }
}

/**
 * @brief Shows the main menu
 */
void Application::showMainMenu() {
    std::string opti;
    std::cout << "\nSelect an operation you would like to do:\n\n"
              << "1 - Execute algorithm.\n"
              << "2 - Exit.\n";

    std::cout << "Input: ";
    std::cin >> opti;
    std::cout << "\n";
    run(stoi(opti));
}

/**
 * @brief Shows the "Go back Menu" after each function
 */
void Application::showGoBackMenu(int functionNumber, const std::string& functionName) {
    L1:
    std::cout << "\nPress enter to continue";
    std::string a;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, a);
    clearScreen();
    std::cout << "\n\nWhat would you like to do next:\n"
              << "1 - Return to main menu.\n"
              << "2 - (again) " << functionName << "\n";

    std::string opt;
    std::cout << "Input: ";
    std::cin >> opt;
    std::cout << "\n";

    int processedKey = processKey(opt);

    switch (processedKey) {
        case -1:
            goto L1;
        case 1:
            throw std::invalid_argument("-1");
        default:
            std::cout << "\n* Error while parsing option, please input a valid numeric option. *\n";
            goto L1;
    }
}

