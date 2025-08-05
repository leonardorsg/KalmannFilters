#ifndef KALMANNFILTER_H
#define KALMANNFILTER_H

#include <Eigen/Dense>

/**
 * @brief Simple Kalman filter for estimating distance, velocity and acceleration.
 *
 * The filter assumes a constant acceleration model between samples.  The
 * system dynamics are given by the state transition matrix A and the
 * observation matrix C.  The process noise covariance Q and observation
 * noise covariance R should be chosen according to the expected variability
 * of the vehicle dynamics and the GPS accuracy, respectively.  See
 * Chapter 4 of Ana Filipa Pinto’s "Bus travel time estimation using mobile
 * devices" dissertation for details.
 */

class KalmanFilter {
public:

    /**
     * Construct a Kalman filter with explicit matrices.
     *
     * @param A State transition matrix (F in the thesis).
     * @param C Observation matrix (H in the thesis).
     * @param Q Process noise covariance.
     * @param R Measurement noise covariance.
     * @param P0 Initial estimate covariance.
     * @param x0 Initial state estimate.
     * @param maxSamples Maximum number of time steps to preallocate.
     */

    KalmanFilter(double dt,
                 const Eigen::Matrix2d& Q,
                 const Eigen::Matrix<double, 1, 1>& R,
                 const Eigen::Vector2d& x0,
                       const Eigen::Matrix2d& P0);

     /**
     * Perform the prediction step for the next time index.
     *
     * @param u Optional control input (unused in this model).
     */
    void predict();

    void setMeasurementNoise(const Eigen::Matrix<double, 1, 1>& R_new);
    void setState(const Eigen::Vector2d& x0);
    void setCovariance(const Eigen::Matrix2d& P0);

    /**
     * Perform the update step with an observation.
     *
     * @param z Measurement vector of dimension r×1.
     */
    void update(const Eigen::Matrix<double, 1, 1>& z);

    /**
     * Access the current posterior state estimate.
     */
    Eigen::Vector2d state() const;

    /**
     * Access the current posterior covariance.
     */
    Eigen::Matrix2d covariance() const;

private:
    double dt_;
    Eigen::Matrix2d F_;
    Eigen::RowVector2d H_;
    Eigen::Matrix2d Q_;
    Eigen::Matrix<double, 1, 1> R_;
    Eigen::Matrix2d P_;
    Eigen::Vector2d x_;
};

#endif // KALMANNFILTER_H