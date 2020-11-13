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


#ifndef SEISCOMP_MATH_MEAN_H
#define SEISCOMP_MATH_MEAN_H

#include<vector>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core.h>

namespace Seiscomp
{
namespace Math
{
namespace Statistics
{

SC_SYSTEM_CORE_API double mean(const DoubleArray &);
SC_SYSTEM_CORE_API double mean(const std::vector<double> &);
SC_SYSTEM_CORE_API double mean(int n, const double *);

SC_SYSTEM_CORE_API double median(const DoubleArray &);
SC_SYSTEM_CORE_API double median(const std::vector<double> &);
SC_SYSTEM_CORE_API double median(int n, const double *);

SC_SYSTEM_CORE_API double fractile(const DoubleArray &, double x);
SC_SYSTEM_CORE_API double fractile(const std::vector<double> &, double x);
SC_SYSTEM_CORE_API double fractile(int n, const double *, double x);

SC_SYSTEM_CORE_API bool computeTrimmedMean(int n, const double *f, double percent, double &value, double &stdev, double *weights = nullptr);
SC_SYSTEM_CORE_API bool computeTrimmedMean(int n, const double *f, double &value, double &stdev, double *weights = nullptr);
SC_SYSTEM_CORE_API bool computeTrimmedMean(const std::vector<double> &v, double percent, double &value, double &stdev, std::vector<double> *weights = nullptr);

SC_SYSTEM_CORE_API bool computeMedianTrimmedMean(int n, const double *f, double distance, double &value, double &stdev, double *weights = nullptr);
SC_SYSTEM_CORE_API bool computeMedianTrimmedMean(const std::vector<double> &v, double distance, double &value, double &stdev, std::vector<double> *weights = nullptr);

SC_SYSTEM_CORE_API bool computeMean(const std::vector<double> &v, double &value, double &stdev);

SC_SYSTEM_CORE_API bool average(int n, const double *values, const double *weights, double &value, double &stdev);
SC_SYSTEM_CORE_API bool average(const std::vector<double> &values, const std::vector<double> &weights, double &value, double &stdev);

SC_SYSTEM_CORE_API void computeLinearTrend(const std::vector<float> &data, double &m, double &n);
SC_SYSTEM_CORE_API void computeLinearTrend(const std::vector<double> &data, double &m, double &n);
SC_SYSTEM_CORE_API void computeLinearTrend(int cnt, const float *data, double &m, double &n);
SC_SYSTEM_CORE_API void computeLinearTrend(int cnt, const double *data, double &m, double &n);

SC_SYSTEM_CORE_API void detrend(std::vector<float> &data, double m, double n);
SC_SYSTEM_CORE_API void detrend(std::vector<double> &data, double m, double n);
SC_SYSTEM_CORE_API void detrend(int cnt, float *data, double m, double n);
SC_SYSTEM_CORE_API void detrend(int cnt, double *data, double m, double n);

// From Rosenberger, J.L., and Gasko, M. (1983). "Comparing Location
// Estimators: Trimmed Means, Medians, and Trimean", in Understanding
// Robust and Exploratory Data Analysis, ed. Hoaglin, D.C., Mosteller,
// F., and Tukey, J.W., p. 297-338, John Wiley, NY

// XXX deprecated XXX
SC_SYSTEM_CORE_API double trimmedMean(const DoubleArray &, double percent=25);
SC_SYSTEM_CORE_API double trimmedMean(const std::vector<double> &f, double percent=25);
SC_SYSTEM_CORE_API double trimmedMean(int n, const double *f, double percent=25);

} // namespace Statistics
} // namespace Math
} // namespace Seiscomp

#endif
