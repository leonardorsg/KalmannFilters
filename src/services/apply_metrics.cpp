#include "apply_metrics.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

// Função auxiliar para requisições HTTP GET
static std::string httpGET(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
            +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                std::string& r = *reinterpret_cast<std::string*>(userdata);
                r.append(ptr, size * nmemb);
                return size;
            });

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return response;
}
void applyContextualMetricsToKalman(KalmanFilter& kf,
                                    const std::string& route_id,
                                    const std::string& stop_id,
                                    int direction,
                                    double& eta_adjustment_seconds) {
    try {
        // Monta URL do endpoint específico
        std::ostringstream url;
        url << "http://130.61.19.80:8000/metrics/" << route_id << "/" << stop_id << "/" << direction;

        std::string json_str = httpGET(url.str());
        if (json_str.empty()) {
            std::cerr << "API response vazia.\n";
            return;
        }

        json data = json::parse(json_str);
        if (!data.is_object()) {
            std::cerr << "Formato inesperado da resposta da API.\n";
            return;
        }

        // std_deviation → matriz R
        if (data.contains("std_deviation") && data["std_deviation"].is_number()) {
            double stddev = data["std_deviation"];
            Eigen::Matrix<double, 1, 1> R;
            R << stddev * stddev;
            kf.setMeasurementNoise(R);
        } else {
            kf.setMeasurementNoise(Eigen::Matrix<double, 1, 1>::Identity() * 25.0); // Valor padrão
        }

        // avg_delay → correção no ETA
        if (data.contains("avg_delay") && data["avg_delay"].is_number()) {
            eta_adjustment_seconds = data["avg_delay"];
        } else {
            eta_adjustment_seconds = 0.0; // Valor padrão
        }

    } catch (const std::exception& e) {
        std::cerr << "Erro ao aplicar métricas da API: " << e.what() << std::endl;
    }
}
