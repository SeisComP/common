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


#ifndef SEISCOMP_PROCESSING_OPERATOR_L2NORM_H
#define SEISCOMP_PROCESSING_OPERATOR_L2NORM_H


namespace Seiscomp {
namespace Processing {
namespace Operator {


template <typename T, int N>
class L2Norm {
	L2Norm();

	// Process N traces in place of length n
	void operator()(const Record *, T *data[N], int n, const Core::Time &stime, double sfreq) const;

	// publishs a processed component
	bool publish(int c) const;
};


template <typename T>
struct L2Norm<T,2> {
	bool publish(int c) const { return c == 0; }

	void operator()(const Record *, T *data[2], int n, const Core::Time &stime, double sfreq) const {
		for ( int i = 0; i < n; ++i )
			data[0][i] = sqrt(data[0][i] * data[0][i] +
			                  data[1][i] * data[1][i]);
	}
};


template <typename T>
struct L2Norm<T,3> {
	bool publish(int c) const { return c == 0; }

	void operator()(const Record *, T *data[3], int n, const Core::Time &stime, double sfreq) const {
		for ( int i = 0; i < n; ++i )
			data[0][i] = sqrt(data[0][i] * data[0][i] +
			                  data[1][i] * data[1][i]);
	}
};


}
}
}


#endif
