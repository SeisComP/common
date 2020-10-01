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


#ifndef SEISCOMP_MATH_VECTOR3_H
#define SEISCOMP_MATH_VECTOR3_H


#include <seiscomp/math/math.h>


namespace Seiscomp {
namespace Math {


template <typename T>
struct Vector3 {
	Vector3() {}
	Vector3(const Vector3<T> &other) : x(other.x), y(other.y), z(other.z) {}
	Vector3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

	T length() const;
	T dot(const Vector3<T> &v) const;

	Vector3<T> &cross(const Vector3<T> &a, const Vector3<T> &b);
	Vector3<T> &normalize();

	Vector3<T> &operator=(const Vector3<T> &other);
	Vector3<T> operator*(T scale) const;
	Vector3<T> &operator*=(T scale);
	T operator*(const Vector3<T> &other) const;

	operator T *() { return (T*)this; }
	operator const T *() const { return (const T*)this; }

	Vector3<T> &operator+=(const Vector3<T> &other);
	Vector3<T> &operator-=(const Vector3<T> &other);

	Vector3<T> operator+(const Vector3<T> &other) const;
	Vector3<T> operator-(const Vector3<T> &other) const;

	Vector3<T> &fromAngles(T radAzimuth, T radDip);
	Vector3<T> &toAngles(T &radAzimuth, T &radDip);

	T  x,y,z;
};


typedef Vector3<float>  Vector3f;
typedef Vector3<double> Vector3d;


#include <seiscomp/math/vector3.ipp>


} // namespace Math
} // namespace Seiscomp

#endif
