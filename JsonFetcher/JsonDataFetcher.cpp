#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

/**
 * @file JsonDataFetcher.cpp
 * @brief Periodically fetches JSON data from a URL and writes it to a file.
 *
 * This program uses libcurl to fetch data and nlohmann::json to parse it.
 * It is intended to collect real-time vehicle data (e.g., bus positions).
 *
 * Compile with:
 * g++ -std=c++14 -o JsonDataFetcher JsonDataFetcher.cpp -lcurl
 */

using json = nlohmann::json;

/**
 * @brief Callback function for libcurl to write received data into a string.
 *
 * @param contents Pointer to the data received.
 * @param size Size of a data unit.
 * @param nmemb Number of data units.
 * @param outString Pointer to the string where data will be appended.
 * @return size_t Total number of bytes processed.
 */

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* outString) {
    size_t totalSize = size * nmemb;
    outString->append((char*)contents, totalSize);
    return totalSize;
}

/**
 * @brief Fetches a JSON string from the specified URL using libcurl.
 *
 * @param url The URL to fetch JSON from.
 * @return std::string The raw JSON response as a string.
 */

std::string fetchJsonFromUrl(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Optional: Disable SSL verification (not recommended for production)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

/**
 * @brief Periodically fetches JSON data from a URL and writes it to a file.
 *
 * @param url The URL to fetch data from.
 * @param outputFilePath The file path to write the fetched JSON data to.
 * @param intervalSeconds The interval in seconds between each fetch.
 */

void fetchAndStorePeriodically(const std::string& url, const std::string& outputFilePath, int intervalSeconds) {
    while (true) {
        std::cout << "Fetching data from URL..." << std::endl;

        // Fetch the JSON data
        std::string jsonResponse = fetchJsonFromUrl(url);

        if (!jsonResponse.empty()) {
            try {
                // Parse JSON
                json jsonData = json::parse(jsonResponse);

                // Store JSON into a file
                std::ofstream outputFile(outputFilePath);
                if (outputFile.is_open()) {
                    outputFile << jsonData.dump(4); // Pretty print JSON with 4 spaces
                    outputFile.close();
                    std::cout << "Data successfully stored in " << outputFilePath << std::endl;
                } else {
                    std::cerr << "Could not open file for writing: " << outputFilePath << std::endl;
                }
            } catch (json::parse_error& e) {
                std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Failed to fetch data." << std::endl;
        }

        // Wait for the specified interval
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
    }
}

/**
 * @brief Entry point of the program. Starts periodic fetching of JSON data.
 *
 * @return int Exit code.
 */

int main() {
    const std::string url = "https://broker.fiware.urbanplatform.portodigital.pt/v2/entities?q=vehicleType==bus&limit=1000";
    const std::string outputFilePath = "../Dataset/Porto/bus_data.json";
    const int intervalSeconds = 30; // Fetch every 30 seconds

    fetchAndStorePeriodically(url, outputFilePath, intervalSeconds);
    return 0;
}