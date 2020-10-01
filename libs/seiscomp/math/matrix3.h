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


#ifndef SEISCOMP_MATH_MATRIX3_H
#define SEISCOMP_MATH_MATRIX3_H


#include <seiscomp/math/vector3.h>
#include <string.h>


namespace Seiscomp {
namespace Math {


template <typename T>
struct Matrix3 {
	Matrix3() {}
	Matrix3(const Matrix3<T> &other) {
		memcpy(d, other.d, sizeof(d));
	}

	Matrix3<T> &identity();

	Matrix3<T> &setRow(int row, const Vector3<T> &v);
	Matrix3<T> &setColumn(int col, const Vector3<T> &v);

	//! Returns a copy of the r-th row
	Vector3<T> row(int r) const;

	//! Returns a copy of the c-th column
	Vector3<T> column(int c) const;

	Matrix3<T> &loadRotateX(T theta);
	Matrix3<T> &loadRotateY(T theta);
	Matrix3<T> &loadRotateZ(T theta);

	Matrix3<T> &mult(const Matrix3<T> &a, const Matrix3<T> &b);

	Vector3<T> &transform(Vector3<T> &dst, const Vector3<T> &v) const;
	Vector3<T> &invTransform(Vector3<T> &dst, const Vector3<T> &v) const;

	operator T *() { return (T*)this; }
	operator const T *() const { return (const T*)this; }

	Vector3<T> operator*(const Vector3<T> &v) const;

	// Coefficients
	union {
		T d[3][3];

		struct {
			T _11, _12, _13;
			T _21, _22, _23;
			T _31, _32, _33;
		} c;
	};
};


typedef Matrix3<float>  Matrix3f;
typedef Matrix3<double> Matrix3d;


#include <seiscomp/math/matrix3.ipp>


} // namespace Math
} // namespace Seiscomp

#endif
