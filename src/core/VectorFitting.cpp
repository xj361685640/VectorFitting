// OpenSEMBA
// Copyright (C) 2015 Salvador Gonzalez Garcia        (salva@ugr.es)
//                    Luis Manuel Diaz Angulo         (lmdiazangulo@semba.guru)
//                    Miguel David Ruiz-Cabello Nuñez (miguel@semba.guru)
//                    Daniel Mateos Romero            (damarro@semba.guru)
//
// This file is part of OpenSEMBA.
//
// OpenSEMBA is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// OpenSEMBA is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with OpenSEMBA. If not, see <http://www.gnu.org/licenses/>.

#include "VectorFitting.h"
#include "SpaceGenerator.h"

#include <iostream>

namespace VectorFitting {

VectorFitting::VectorFitting(
        const vector<Sample>& samples,
        size_t order) {

    samples_ = samples;
    order_ = order;

    // Define starting poles as a vector of complex conjugates -a + bi with
    // the imaginary part linearly distributed over the frequency range of
    // interest; i.e., for each pair of complex conjugates (see eqs 9 and 10):
    //      1. imagParts = linspace(range(samples), number_of_poles)
    //      2. realParts = imagParts / 100

    // Generate the imaginary parts of the initial poles from a linear
    // distribution covering the range in the samples.
    // TODO: We are assuming the samples are ordered depending on their
    // imaginary parts; order them before doing this!
    pair<Real,Real> range(  samples_.front().first.imag(),
                            samples_.back().first.imag());

    // This can also be done with a logarithmic distribution (sometimes
    // faster convergence -see Userguide, p.8-)
    vector<Real> imagParts = linspace(range, order_/2);

    // Generate all the starting poles
    for (size_t i = 0; i < order_; i+=2) {
        Real imag = imagParts[i];
        Real real = - imag / (Real) 100.0;
        poles_.push_back(Complex(real, imag));
        poles_.push_back(Complex(real, -imag));
    }
}

void VectorFitting::fit() {
    // ########################################################################
    // ##################### STAGE 1: Pole identification #####################
    // ########################################################################

    // Define matrix A following equation (A.3), where:
    //      s_k = sample[k].first()
    //      f(s_k) = sample[k].second()
    //      a_i = starting poles
    // If a_i, a_{i+1} is a complex conjugate (they always come in pairs;
    // otherwise, there is an error), the elements have to follow equation
    // (A.6)
    // Define column vector B following equation (A.4), where:
    //      f(s_k) = sample.second()


    // Number of rows = number of samples
    // Number of columns = 2* order of approximation + 2
    MatrixXcd A(samples_.size(), 2*order_ + 2);
    MatrixXcd B(samples_.size(), 1);

    // TODO: We are dealing only with the first element in f(s_k), so this
    // is not yet a *vector* fitting.
    for (size_t k = 0; k < A.rows()/2; k++) {
        Complex s_k = samples_[k].first;
        Complex f_k = samples_[k].second[0];

        B(k,0) = f_k;

        // TODO: We are considering all the poles are complex, is it ok?
        for (size_t i = 0; i < order_; i+=2) {
            Complex a_i = poles_[i];
            A(k,i) = 1.0 / (s_k - a_i) + 1.0 / (s_k - conj(a_i));
            A(k,i+1) = Complex(0.0,1.0) * A(k,i);

            A(k,2 + 2*i) = -A(k,i) * f_k;
            A(k,2 + 2*i + 1) = -A(k,i+1) * f_k;
        }
        A(k, order_) = Complex(1.0,0.0);
        A(k, order_+1) = s_k;
    }

    // Solve AX = B (¿by singular value decomposition? Eigen!), whose elements
    // are:
    //      X[:N] = f residues
    //      X[N] = d
    //      X[N+1] = h
    //      X[N+2:] = sigma residues <- these values are the important ones
    ArrayXcd X = A.colPivHouseholderQr().solve(B);

    ArrayXcd fResidues = X.head(order_);
    Complex d = X[order_];
    Complex h = X[order_ + 1];
    ArrayXcd sigmaResidues = X.tail(X.size() - (order_ + 2));

    // Debug check
    assert(sigmaResidues.size() == poles_.size());

    // cout << X.block(0,0,20,1) << endl << "_________________________" << endl;
    // cout << X.block(19,0,1,1) << endl << "_________________________" << endl;
    // cout << X.block(20,0,1,1) << endl << "_________________________" << endl;
    // cout << X.block(21,0,20,1) << endl << "_________________________" << endl;

    // Define a diagonal matrix containing the starting poles, A, (see B.2)
    // as follows:
    //      If a_i is real => A[i,i] = a_i
    //      If a_i, a_{i+1} is a complex conjugate =>
    //          A[i,i] = A[i+1, i+1] = real(a_i)
    //          A[i,i+1] = imag(a_i)
    //          A[i+1,i] = -imag(a_i)
    // Define a column vector, B, as follows (see B.2):
    //      If a_i is real => B[i] = 1
    //      If a_i, a_{i+1} is a complex conjugate => B[i] = 2; B[i+1] = 0
    // Define a row vector, C, as follows:
    //      If a_i is real => C[i] = sigma_residues[i]
    //      If a_i, a_{i+1} is a complex conjugate =>
    //          C[i] = real(sigma_residues[i])
    //          C[i+1] = imag(sigma_residues[i])

    MatrixXd A_(poles_.size(), poles_.size());
    ColumnVectorXd B_(poles_.size());
    RowVectorXd C_(poles_.size());

    // Populate matrices A_, B_ and C_
    for (size_t i = 0; i < A_.rows(); i+=2) {
        Real poleReal = poles_[i].real();
        Real poleImag = poles_[i].imag();

        A_(i, i) = A_(i+1, i+1) = poleReal;

        A_(i, i+1) = poleImag;
        A_(i+1, i) = -poleImag;

        B(i) = 2;
        B(i+1) = 0;

        Real sigmaReal = sigmaResidues[i].real();
        Real sigmaImag = sigmaResidues[i].imag();

        C_(i) = sigmaReal;
        C_(i+1) = sigmaImag;
    }

    // Compute the eigen values of H = A - BC
    MatrixXd = A_ - B
}

vector<VectorFitting::Sample> VectorFitting::getFittedSamples(
        const vector<Complex >& frequencies) const {

}

vector<complex<Real>> VectorFitting::getPoles() {
    // assert(computed_ == true);
    return poles_;
}

vector<complex<Real>> VectorFitting::getResidues() {
    // assert(computed_ == true);
    return residues_;
}

Real VectorFitting::getRMS() {
}

} /* namespace VectorFitting */
