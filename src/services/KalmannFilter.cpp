//
// Implementation of the corrected Kalman filter for the KalmannFilters project.
//
// The implementation follows the standard discrete-time Kalman filter
// equations: a prediction step that advances the state estimate with the
// system dynamics and adds process noise, and an update step that
// incorporates a new measurement using the Kalman gain.  See the header
// KalmannFilter.h for more information.

#include "KalmannFilter.h"

#include <iostream>

KalmanFilter::KalmanFilter(double dt,
                                       const Eigen::Matrix2d& Q,
                                       const Eigen::Matrix<double, 1, 1>& R,
                                       const Eigen::Vector2d& x0,
                                       const Eigen::Matrix2d& P0)
    : dt_(dt), Q_(Q), R_(R), x_(x0), P_(P0)
{
    F_ << 1.0, -dt_,
          0.0, 1.0;

    H_ << 1.0, 0.0;
}

void KalmanFilter::setMeasurementNoise(const Eigen::Matrix<double, 1, 1>& R_new) {
    R_ = R_new;
}

void KalmanFilter::setState(const Eigen::Vector2d& x0) {
    x_ = x0;
}

void KalmanFilter::setCovariance(const Eigen::Matrix2d& P0) {
    P_ = P0;
}

void KalmanFilter::setDt(double dt) {
    dt_ = dt;
    F_ << 1.0, -dt_,
          0.0, 1.0;
}

void KalmanFilter::predict() {
    x_ = F_ * x_;
    P_ = F_ * P_ * F_.transpose() + Q_;
}

void KalmanFilter::update(const Eigen::Matrix<double, 1, 1>& z) {
    auto y = z - H_ * x_;
    auto S = H_ * P_ * H_.transpose() + R_;
    auto K = P_ * H_.transpose() * S.inverse();

    x_ = x_ + K * y;
    P_ = (Eigen::Matrix2d::Identity() - K * H_) * P_;
}

Eigen::Vector2d KalmanFilter::state() const {
    return x_;
}

Eigen::Matrix2d KalmanFilter::covariance() const {
    return P_;
}