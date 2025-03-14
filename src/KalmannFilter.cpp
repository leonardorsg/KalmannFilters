//
// Created by leonardorsg on 3/7/25.
//


#include "KalmannFilter.h"

using namespace std;
using namespace Eigen;

//g++ -I/usr/include/eigen3 -o my_program my_program.cpp

KalmannFilter::KalmannFilter(const MatrixXd A, const MatrixXd B, MatrixXd C,
        MatrixXd Q, MatrixXd R, MatrixXd P0, MatrixXd x0, unsigned int maxSamples) {
    this->k = 0;    this->A = A;    this->B = B;
    this->C = C;    this->Q = Q;    this->R = R;
    this->P0 = P0;  this->x0 = x0;

    this->n = A.rows();     this->m=B.cols();       this->r=C.rows();

    //posteiori estimates column-wise
    this->estimatesAposteriori.resize(n, maxSamples);
    this->estimatesAposteriori.setZero();
    this->estimatesAposteriori.col(0) = x0; //

    //a priori estimates column-wise
    this->estimatesApriori.resize(n, maxSamples);
    this->estimatesApriori.col(0) = x0;

    this->covarianceAposteriori.resize(n, n*maxSamples);
    this->covarianceAposteriori.setZero();
    this->covarianceAposteriori(all, seq(0,n-1))=P0; //store covariance matrices next to each other

    this->covarianceApriori.resize(n, n*maxSamples);
    this->covarianceApriori.setZero();

    this->kalmannGain.resize(n, r*maxSamples);
    this->kalmannGain.setZero();

    this->errors.resize(n, maxSamples);
    this->errors.setZero();
}


//In the prediction stage, the algorithm predicts a state estimate and the error covariance
void KalmannFilter::predictEstimate(MatrixXd input) {
    this->estimatesApriori.col(this->k)=A*estimatesApriori.col(this->k)+this->B*input;
    this->covarianceApriori(all, seq(this->k*this->n,(this->k+1)*this->n-1))=this->A*this->covarianceAposteriori(all, seq(k*n,(k+1)*n-1))*(this->A.transpose())+this->Q;
    this->k++;
}

/* When there is a new measurement for the predicted variables, the algorithm updates, the
 * state estimate, combining the measured values with the predicted state estimate.
 * Measurement = Z(k) (observation of the true state X(k)) - C(k) * estimatesApriori(k-1)
 */
MatrixXd KalmannFilter::updateEstimate(MatrixXd measurement) {
    //Calculation of last bit of Kalmann Gain Matrix calculation
    MatrixXd initialGain;
    initialGain.resize(this->n,this->n);
    initialGain = this->R+this->C*this->covarianceApriori(all, seq((k-1)*n,k*n-1))*(this->C.transpose());
    initialGain = initialGain.inverse();

    //Kalmann Gain calculation
    this->kalmannGain(all,seq((k-1)*this->r,this->k*this->r-1)) = this->covarianceApriori(all,seq((k-1)*this->n,this->k*this->n-1))*(this->C.transpose())*initialGain;
    //update error matrix
    this->errors.col(this->k-1) = measurement - this->C * this->estimatesApriori.col(this->k-1);
    //Posteiori state estimate calculation
    this->estimatesAposteriori.col(k) = this->estimatesApriori.col(k-1)+this->kalmannGain(all,seq((k-1)*r,k*r-1))*errors.col(k-1);

    //Calculation of the posteriori covariance matrix
    MatrixXd identity = MatrixXd::Identity(n,n);
    MatrixXd calcSnippet;   calcSnippet.resize(n,n);
    calcSnippet = identity - this->kalmannGain(all, seq((k-1)*this->r,this->k*this->r-1)) * this->C;

    this->covarianceAposteriori(all, seq(k*n,(k+1)*n-1)) = calcSnippet * this->covarianceApriori(all,seq((k-1)*n,k*n-1)) * (calcSnippet.transpose())
                                                                   + this->kalmannGain(all, seq((k-1)*r,k*r-1)) * this->R * (this->kalmannGain(all,seq((k-1)*r,k*r-1)).transpose());

    // 🔴 ATUALIZAÇÃO DO ESTADO APRIORI PARA O PRÓXIMO PASSO 🔴
    this->estimatesApriori.col(k) = this->estimatesAposteriori.col(k);
    this->covarianceApriori.block(0, k*n, n, n) = this->covarianceAposteriori.block(0, k*n, n, n);


    return this->estimatesAposteriori.col(k);
}

