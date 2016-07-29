// OpenSEMBA
// Copyright (C) 2015 Salvador Gonzalez Garcia        (salva@ugr.es)
//                    Luis Manuel Diaz Angulo         (lmdiazangulo@semba.guru)
//                    Miguel David Ruiz-Cabello Nuñez (miguel@semba.guru)
//                    Daniel Mateos Romero            (damarro@semba.guru)
//                    Alejandro García Montoro        (alejandro.garciamontoro@gmail.com)
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

#include "gtest/gtest.h"
#include "VectorFitting.h"
#include "SpaceGenerator.h"

using namespace VectorFitting;
using namespace std;

class MathFittingVectorFittingTest : public ::testing::Test {

};

// Test first example of Bjorn Gustavsen's code
TEST_F(MathFittingVectorFittingTest, ex1) {
    // Define samples frequencies
    const size_t nS = 101;
    vector<VectorFitting::Sample> samples(nS);

    // Compute distribution of the frequencies
    vector<Real> sImag = logspace(pair<Real,Real>(0.0,4.0), nS);

    // Populate frequencies and responses
    for (size_t k = 0; k < nS; k++) {
        // Frequenciy
        const Complex sk = Complex(0.0, 2.0 * M_PI * sImag[k]);

        // Response
        vector<Complex> f(1);
        f[0] =  2.0 /(sk+5.0)
                + Complex(30.0,40.0)  / (sk-Complex(-100.0,500.0))
                + Complex(30.0,-40.0) / (sk-Complex(-100.0,-500.0))
                + 0.5;

        // Build sample
        samples[k].first = sk;
        samples[k].second = f;
    }

    // Define starting poles
    const size_t N = 3;
    vector<Complex> poles(N);

    // Compute distribution of the poles
    vector<Real> pReal = logspace(pair<Real,Real>(0.0,4.0), N);

    // Populate starting poles
    for (size_t i = 0; i < N; i++) {
        poles[i] = Complex(-2 * M_PI * pReal[i], 0.0);
    }

    // Model fitting
    VectorFitting::VectorFitting fitting(samples, poles, N);
    fitting.fit();

    // Error check
    EXPECT_NEAR(0.0, fitting.getRMSE(), 1e-3);
}

// // Test first example of Bjorn Gustavsen's code
// TEST_F(MathFittingVectorFittingTest, test) {
//     // Define samples frequencies
//     const size_t nS = 800;
//     vector<VectorFitting::Sample> samples(nS);
//
//     // Compute distribution of the frequencies
//     vector<Real> sImag = linspace(pair<Real,Real>(1,1e5), nS);
//
//     // Known poles
//     vector<Complex> knownPoles;
//     knownPoles.push_back(Complex(-4500, 0.0));
//     knownPoles.push_back(Complex(-41000, 0.0));
//     knownPoles.push_back(Complex(-100, +5000));
//     knownPoles.push_back(Complex(-100, -5000));
//     knownPoles.push_back(Complex(-120, +15000));
//     knownPoles.push_back(Complex(-120, -15000));
//     knownPoles.push_back(Complex(-3000, +35000));
//     knownPoles.push_back(Complex(-3000, -35000));
//
//     // Known residues
//     vector<Complex> knownResidues;
//     knownResidues.push_back(Complex(-3000, 0.0));
//     knownResidues.push_back(Complex(-83000, 0.0));
//     knownResidues.push_back(Complex(-5, +7000));
//     knownResidues.push_back(Complex(-5, -7000));
//     knownResidues.push_back(Complex(-20, +18000));
//     knownResidues.push_back(Complex(-20, -18000));
//     knownResidues.push_back(Complex(6000, +45000));
//     knownResidues.push_back(Complex(6000, -45000));
//
//     // Known parameters
//     Real knownD = 0.2;
//     Real knownH = 2e-5;
//
//     // Vector to store the fitted samples and loop variable
//     vector<Sample> knownResponses(nS);
//
//     for (size_t k = 0; k < nS; k++) {
//         // Independent variable s
//         Complex sk = Complex(0.0, sImag[k]);
//
//         // Computation of the dependent variable f(s) with the model (see (2))
//         vector<Complex> f(1);
//         f[0] = Complex(0,0);
//
//         for (size_t n = 0; n < knownPoles.size(); n++) {
//             Complex cn = knownResidues[n];
//             Complex an = knownPoles[n];
//
//             f[0] += cn / (sk - an);
//         }
//
//         f[0] += knownD + sk * knownH;
//
//         knownResponses[k] = Sample(sk, f);
//     }
//
//     // Define starting poles
//     const size_t N = 5;
//     vector<Complex> poles(N);
//
//     // Compute distribution of the poles
//     vector<Real> pReal = logspace(pair<Real,Real>(0.0,4.0), N);
//
//     // Populate starting poles
//     for (size_t i = 0; i < N; i++) {
//         poles[i] = Complex(-2 * M_PI * pReal[i], 0.0);
//     }
//
//     // Model fitting
//     VectorFitting::VectorFitting fitting(samples, poles, N);
//     fitting.fit();
//
//     // Error check
//     EXPECT_NEAR(0.0, fitting.getRMSE(), 1e-3);
// }
