/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/

#ifndef __LEASTSQUARES_SOLVER_H__
#define __LEASTSQUARES_SOLVER_H__

#include "lsmr.h"
#include "lsqr.h"
#include <stdexcept>
#include <string>

namespace {

/*
 * W G m = r W
 *
 * W: diagonal matrix to weight each equation.
 * G: matrix of the partial derivatives of the slowness vector with respect to
 *    event/station location and 1 in the last column corresponding to
 *    the source time correction term [sec/km]
 * m: vector containing the changes in hypocentral parameters we wish to
 *    determine (delta x, delta y, delta z and delta travel time)
 *    [km] and [sec]
 * r: residual vector [sec]
 */
struct System {
  const unsigned numColsG;
  const unsigned numRowsG;

  double *const W;
  double (*const G)[4];
  double m[4];
  double *const r;
  double L2NScaler[4];

  System(unsigned nObs)
      : numColsG(4), numRowsG(nObs), W(new double[numRowsG]),
        G(new double[numRowsG][4]), r(new double[numRowsG]) {}

  ~System() {
    delete[] W;
    delete[] G;
    delete[] r;
  }

  System(const System &other) = delete;
  System operator=(const System &other) = delete;
};

/**
 * Common System adapter for both LSQR and LSMR solvers
 * T can be `lsqrBase` or `lsmrBase`.
 */
template <typename T> class Adapter : public T {
private:
  System &_eq; // doesn't own the System

public:
  Adapter(System &eq) : _eq(eq) {
    if (_eq.numColsG != 4) {
      throw std::runtime_error("Solver: Internal logic error");
    }
    std::fill_n(_eq.L2NScaler, _eq.numColsG, 1.);
    for (unsigned int ob = 0; ob < _eq.numRowsG; ob++) {
      _eq.r[ob] *= _eq.W[ob];
    }
  }
  virtual ~Adapter() = default;

  void Solve() { T::Solve(_eq.numRowsG, _eq.numColsG, _eq.r, _eq.m); }

  /*
   * Scale G by normalizing the L2-norm of each column as suggested
   * by LSQR and LSMR solvers.
   */
  void L2normalize() {
    std::fill_n(_eq.L2NScaler, _eq.numColsG, 0.);

    for (unsigned int ob = 0; ob < _eq.numRowsG; ob++) {
      const double obsW = _eq.W[ob];

      _eq.L2NScaler[0] += (_eq.G[ob][0] * obsW) * (_eq.G[ob][0] * obsW);
      _eq.L2NScaler[1] += (_eq.G[ob][1] * obsW) * (_eq.G[ob][1] * obsW);
      _eq.L2NScaler[2] += (_eq.G[ob][2] * obsW) * (_eq.G[ob][2] * obsW);
      _eq.L2NScaler[3] += (_eq.G[ob][3] * obsW) * (_eq.G[ob][3] * obsW);
    }

    _eq.L2NScaler[0] = 1. / std::sqrt(_eq.L2NScaler[0]);
    _eq.L2NScaler[1] = 1. / std::sqrt(_eq.L2NScaler[1]);
    _eq.L2NScaler[2] = 1. / std::sqrt(_eq.L2NScaler[2]);
    _eq.L2NScaler[3] = 1. / std::sqrt(_eq.L2NScaler[3]);
  }

  /*
   * Rescale m back to the initial scaling.
   */
  void L2DeNormalize() {
    _eq.m[0] *= _eq.L2NScaler[0];
    _eq.m[1] *= _eq.L2NScaler[1];
    _eq.m[2] *= _eq.L2NScaler[2];
    _eq.m[3] *= _eq.L2NScaler[3];
  }

  /**
   * Required by `lsqrBase` and `lsmrBase`:
   *
   * computes y = y + A*x without altering x,
   * where A is a matrix of dimensions A[m][n].
   * The size of the vector x is n.
   * The size of the vector y is m.
   */
  void Aprod1(unsigned int m, unsigned int n, const double *x,
              double *y) const {
    if (m != _eq.numRowsG || n != _eq.numColsG) {
      throw std::runtime_error("Solver: Internal logic error");
    }

    for (unsigned int ob = 0; ob < _eq.numRowsG; ob++) {

      double sum = 0;

      sum += _eq.G[ob][0] * _eq.L2NScaler[0] * x[0];
      sum += _eq.G[ob][1] * _eq.L2NScaler[1] * x[1];
      sum += _eq.G[ob][2] * _eq.L2NScaler[2] * x[2];
      sum += _eq.G[ob][3] * _eq.L2NScaler[3] * x[3];

      y[ob] += _eq.W[ob] * sum;
    }
  }

  /**
   * Required by `lsqrBase` and `lsmrBase`:
   *
   * computes x = x + A'*y without altering y,
   * where A is a matrix of dimensions A[m][n].
   * The size of the vector x is n.
   * The size of the vector y is m.
   */
  void Aprod2(unsigned int m, unsigned int n, double *x,
              const double *y) const {
    if (m != _eq.numRowsG || n != _eq.numColsG) {
      throw std::runtime_error("Solver: Internal logic error");
    }

    for (unsigned int ob = 0; ob < _eq.numRowsG; ob++) {
      x[0] += _eq.W[ob] * _eq.G[ob][0] * _eq.L2NScaler[0] * y[ob];
      x[1] += _eq.W[ob] * _eq.G[ob][1] * _eq.L2NScaler[1] * y[ob];
      x[2] += _eq.W[ob] * _eq.G[ob][2] * _eq.L2NScaler[2] * y[ob];
      x[3] += _eq.W[ob] * _eq.G[ob][3] * _eq.L2NScaler[3] * y[ob];
    }
  }
};

template <typename T>
Adapter<T> solve(System &eq, std::ostringstream *solverLogs = nullptr,
                 double dampingFactor = 0, unsigned numIterations = 0,
                 bool normalizeG = true) {
  Adapter<T> solver(eq); // keeps only a reference to eq, doesn't copy it!!!
  if (normalizeG) {
    solver.L2normalize();
  }
  solver.SetDamp(dampingFactor);
  solver.SetMaximumNumberOfIterations(numIterations ? numIterations
                                                    : eq.numColsG / 2);
  const double eps = std::numeric_limits<double>::epsilon();
  solver.SetEpsilon(eps);
  solver.SetToleranceA(1e-6); // we use [km] and [sec] in the eq system, so
  solver.SetToleranceB(1e-6); // this tolerance looks like enough (mm and usec)
  solver.SetUpperLimitOnConditional(1.0 / (10 * sqrt(eps)));

  // store logs
  if (solverLogs) {
    solver.SetOutputStream(*solverLogs);
  }

  solver.Solve();

  if (solver.GetStoppingReason() == 4) {
    std::string msg =
        "Solver: no solution found:" + solver.GetStoppingReasonMessage();
    throw std::runtime_error(msg);
  }

  if (normalizeG) {
    solver.L2DeNormalize();
  }

  return solver;
}

} // namespace

#endif
