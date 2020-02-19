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

#define _mat3_elem(row,column) d[row][column]

template <typename T>
inline Vector3<T> &Matrix3<T>::transform(Vector3<T> &dst, const Vector3<T> &v) const {
	for ( int r = 0; r < 3; ++r )
		dst[r] = _mat3_elem(r,0)*v[0] +
		         _mat3_elem(r,1)*v[1] +
		         _mat3_elem(r,2)*v[2];

	return dst;
}


template <typename T>
inline Vector3<T> &Matrix3<T>::invTransform(Vector3<T> &dst, const Vector3<T> &v) const {
	for ( int r = 0; r < 3; ++r )
		dst[r] = _mat3_elem(0,r)*v[0] +
		         _mat3_elem(1,r)*v[1] +
		         _mat3_elem(2,r)*v[2];

	return dst;
}


template <typename T>
inline Vector3<T> Matrix3<T>::operator*(const Vector3<T> &v) const {
	Vector3<T> r;
	transform(r, v);
	return r;
}
