/***************************************************************************
 * Copyright (C) Preparatory Commission for the Comprehensive              *
 * Nuclear-Test-Ban Treaty Organization (CTBTO).                           *
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


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_IDCUTILS_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_IDCUTILS_H


#include <cstddef>


namespace Seiscomp {
namespace Processing {
namespace Utils {
namespace IDC {


bool runningAverage(const double *data, const size_t *state,
                    const size_t numPoints, const size_t averageWindowLength,
                    const size_t threshold, double (*function)(double x),
                    double *runningAverage,
                    size_t *runningAverageState);


bool recursiveAverage(const double *data, const size_t *state,
                      const size_t numPoints,
                      const size_t recursionLookbackLength,
                      const size_t averageWindowLength,
                      const size_t threshold, double (*function)(double x),
                      double *recursiveAverage,
                      size_t *recursiveAverageState);


double samex(double x);


} // namespace IDC
} // namespace Utils
} // namespace Processing
} // namespace Seiscomp


#endif // SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_IDCUTILS_H
