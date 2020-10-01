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


#ifndef SEISCOMP_PROCESSING_OPERATOR_TRANSFORMATION_H
#define SEISCOMP_PROCESSING_OPERATOR_TRANSFORMATION_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/math/matrix3.h>


namespace Seiscomp {
namespace Processing {
namespace Operator {


template <typename T, int N>
struct Transformation {
	Transformation(const Math::Matrix3<T> &m);

	// Process N traces in place of length n
	void operator()(const Record *, T *data[N], int n, const Core::Time &, double) const;

	// publishs a processed component
	bool publish(int c) const;
};


template <typename T>
struct Transformation<T,2> {
	Transformation(const Math::Matrix3<T> &m) : matrix(m) {}

	bool publish(int c) const { return true; }

	void operator()(const Record *, T *data[2], int n, const Core::Time &, double) const {
		for ( int i = 0; i < n; ++i ) {
			Math::Vector3<T> v = matrix*Math::Vector3<T>(*data[0], *data[1], 0);
			*data[0] = v.x;
			*data[1] = v.y;
			++data[0]; ++data[1];
		}
	}

	Math::Matrix3<T> matrix;
};


template <typename T>
struct Transformation<T,3> {
	Transformation(const Math::Matrix3<T> &m) : matrix(m) {}

	bool publish(int c) const { return true; }

	void operator()(const Record *, T *data[3], int n, const Core::Time &, double) const {
		for ( int i = 0; i < n; ++i ) {
			Math::Vector3<T> v = matrix*Math::Vector3<T>(*data[0], *data[1], *data[2]);
			*data[0] = v.x;
			*data[1] = v.y;
			*data[2] = v.z;
			++data[0]; ++data[1]; ++data[2];
		}
	}

	Math::Matrix3<T> matrix;
};


}
}
}


#endif
