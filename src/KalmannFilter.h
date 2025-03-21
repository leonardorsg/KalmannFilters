//
// Created by leonardorsg on 3/7/25.
//

#ifndef KALMANNFILTER_H
#define KALMANNFILTER_H
#include <iostream>
#include <string>
#include <Eigen/Dense>
#include "Utils.h"

using namespace Eigen;
class KalmannFilter {
private:
    //State variable
    unsigned int k;

    //input, state and output dimensions
    unsigned int m, n, r;

    //Eigen-type matrices
    /*        | 1   ∆t  (1/2)∆t² |
     * A(k) = | 0   1       ∆t   |
     *        | 0   0       1    |
     */
    MatrixXd A; // State Transition Function (Referred as F(k) in thesis)
    MatrixXd B; // Control Input Matrix (Non-existent in thesis)
    /*
     * C represents which variables are actually measured by our system.
     * Since the first version of this algorithm only measures distance
     * C = [ 1 0 0 ]
     */
    MatrixXd C; // Observation Model (Referred as H(k) is thesis)
    MatrixXd Q; // Process noise covariance
    /*
     * R represents the accuracy of the GPS values. We will assume that
     * the base accuracy is of 5 meters, so R = [5].
     */
    MatrixXd R; // Observation noise
    MatrixXd P0; //P0-> initial estimated noise

    //X(k-1) = [d(k-1)  v(k-1)  a(k-1)]
    MatrixXd x0; // initial estimated state

    //posteriori estimates matrix X(k)(+)
    //X(k) = [d(k)  v(k)  a(k)]
    MatrixXd estimatesAposteriori;

    //apriori estimates matrix X(k)(-)
    /* The estimatesApriori variable represents a matrix with estimates for
     * the distance travelled by a bus on a given line since the start of that line.
     *
     */
    MatrixXd estimatesApriori;

    //covariance a posteriori matrix P0(k)(+)
    MatrixXd covarianceAposteriori;

    //covariance a priori matrix P0(k)(-)
    MatrixXd covarianceApriori;

    //Kalmann gain matrix
    MatrixXd kalmannGain;

    //Error predictions matrix
    MatrixXd errors; // = y(k) - C(k)X(k)(-)

public:
    KalmannFilter();

    KalmannFilter(const MatrixXd A, MatrixXd C,
        MatrixXd Q, MatrixXd R, MatrixXd P0, MatrixXd x0, unsigned int maxSamples);

    MatrixXd updateEstimate(MatrixXd measurement);

    void predictEstimate(MatrixXd input);

    void printMatrices();
};

#endif //KALMANNFILTER_H
