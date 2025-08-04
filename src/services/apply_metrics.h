#ifndef APPLY_METRICS_H
#define APPLY_METRICS_H

#include <string>
#include "KalmannFilter.h"

void applyContextualMetricsToKalman(KalmanFilter& kf,
                                    const std::string& route_id,
                                    const std::string& stop_id,
                                    int direction,
                                    double& eta_adjustment_seconds);

#endif // APPLY_METRICS_H
