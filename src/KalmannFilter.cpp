//
// Created by leonardorsg on 3/7/25.
//


#include "KalmannFilter.h"

using namespace std;
using namespace Eigen;

//g++ -I/usr/include/eigen3 -o my_program my_program.cpp

KalmannFilter::KalmannFilter(const MatrixXd A, MatrixXd C,
        MatrixXd Q, MatrixXd R, MatrixXd P0, MatrixXd x0, unsigned int maxSamples) {
    this->k = 0;    this->A = A;
    this->C = C;    this->Q = Q;    this->R = R;
    this->P0 = P0;  this->x0 = x0;

    this->n = A.rows();       this->r=C.rows();

    //posteiori estimates column-wise
    this->estimatesAposteriori.resize(n, maxSamples);
    this->estimatesAposteriori.setZero();

    //a priori estimates column-wise
    this->estimatesApriori.resize(n, maxSamples);
    this->estimatesApriori.setZero();

    this->covarianceAposteriori.resize(n, n*maxSamples);
    this->covarianceAposteriori.setZero();
    this->covarianceAposteriori(all, seq(0,n-1))=P0; //store covariance matrices next to each other

    this->covarianceApriori.resize(n, n*maxSamples);
    this->covarianceApriori.setZero();

    this->kalmannGain.resize(n, r*maxSamples);
    this->kalmannGain.setZero();

    this->errors.resize(r, maxSamples);
    this->errors.setZero();
}


//In the prediction stage, the algorithm predicts a state estimate and the error covariance
void KalmannFilter::predictEstimate(MatrixXd input) {
    if (input.rows() != n || input.cols() != 1) {
        throw std::runtime_error("Error: Input matrix dimensions do not match state dimension.");
    }
    this->estimatesApriori.col(this->k) = A * this->estimatesAposteriori.col(this->k) + input;

    this->covarianceApriori(all,seq(k*n,(k+1)*n-1)) = A * this->covarianceAposteriori(all,seq(k*n,(k+1)*n-1)) * (A.transpose()) + Q;


    this->k++;
}


/* When there is a new measurement for the predicted variables, the algorithm updates, the
 * state estimate, combining the measured values with the predicted state estimate.
 * Measurement = Z(k) (observation of the true state X(k)) - C(k) * estimatesApriori(k-1)
 */
MatrixXd KalmannFilter::updateEstimate(MatrixXd measurement) {

    if (measurement.rows() != n || measurement.cols() != 1) {
        std::cerr << "Expected: " << C.rows() << "x" << 1 << ", Received: " << measurement.rows() << "x" << measurement.cols() << std::endl;
        throw std::runtime_error("Error: Measurement dimensions do not match expected observation matrix size.");
    }

    MatrixXd initialGain;
    initialGain.resize(r,r);
    initialGain = R + C*covarianceApriori(all,seq((k-1)*n,k*n-1)) * (C.transpose());
    initialGain = initialGain.inverse();


    this->kalmannGain(all, seq((k-1)*r,k*r-1)) = this->covarianceApriori(all,seq((k-1)*n,k*n-1)) * (C.transpose()) * initialGain;
    /*
    std::cout << "KalmannGain: " << std::endl;


    for (unsigned int i = 0; i < kalmannGain.rows(); i++) {
        std::cout << "|";
        for (unsigned int j = 0; j < kalmannGain.cols(); j++) {
            std::cout << kalmannGain(i,j) << ",";
        }
        std::cout << "|\n";
    }
    */
    measurement = (measurement.transpose());
    /*
    std::cout << "measurement: " << std::endl;


    for (unsigned int i = 0; i < measurement.rows(); i++) {
        std::cout << "|";
        for (unsigned int j = 0; j < measurement.cols(); j++) {
            std::cout << measurement(i,j) << ",";
        }
        std::cout << "|\n";
    }
*/
    auto result = C * this->estimatesApriori.col(this->k-1);
/*
    std::cout <<"result: "<< std::endl;
    for (unsigned int i = 0; i < result.rows(); i++) {
        std::cout << "|";
        for (unsigned int j = 0; j < result.cols(); j++) {
            std::cout << result(i,j) << ",";
        }
        std::cout << "|\n";
    }
*/


    //CHECAR ERRO AQUI
    this->errors.col(this->k-1) = abs(measurement.col(0) - result.col(0));

    this->estimatesAposteriori.col(this->k) = this->estimatesApriori.col(this->k-1) + kalmannGain(all,seq((k-1)*r,k*r-1))*errors.col(k-1);

    MatrixXd identity = MatrixXd::Identity(n, n);
    MatrixXd calcSnippet;
    calcSnippet.resize(n, n);
    calcSnippet = identity - this->kalmannGain(all, seq((k-1)*r,k*r-1)) * C;
    this->covarianceAposteriori(all, seq(k * n, (k + 1) * n - 1)) =
    identity * this->covarianceApriori(all, seq((k - 1) * n, k * n - 1)) * identity.transpose() +
    this->kalmannGain(all, seq((k - 1) * r, k * r - 1)) * R *
    (this->kalmannGain(all, seq((k - 1) * r, k * r - 1)).transpose());

    return this->estimatesAposteriori.col(k);
}

void KalmannFilter::printMatrices() {
    cout << "State Variable (k): " << k << endl;
    cout << "Dimensions - m: " << m << ", n: " << n << ", r: " << r << endl;

    cout << "\nMatrix A (State Transition Function):\n" << A << endl;
    cout << "\nMatrix B (Control Input Matrix):\n" << B << endl;
    cout << "\nMatrix C (Observation Model):\n" << C << endl;
    cout << "\nMatrix Q (Process Noise Covariance):\n" << Q << endl;
    cout << "\nMatrix R (Observation Noise Covariance):\n" << R << endl;
    cout << "\nMatrix P0 (Initial Estimated Noise):\n" << P0 << endl;
    cout << "\nMatrix x0 (Initial Estimated State):\n" << x0 << endl;
    cout << "\nMatrix estimatesAposteriori (Posteriori Estimates):\n" << estimatesAposteriori << endl;
    cout << "\nMatrix estimatesApriori (Apriori Estimates):\n" << estimatesApriori << endl;
    cout << "\nMatrix covarianceAposteriori (Posteriori Covariance):\n" << covarianceAposteriori << endl;
    cout << "\nMatrix covarianceApriori (Apriori Covariance):\n" << covarianceApriori << endl;
    cout << "\nMatrix kalmannGain (Kalman Gain):\n" << kalmannGain << endl;
    cout << "\nMatrix errors (Prediction Errors):\n" << errors << endl;
}

