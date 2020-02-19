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


namespace Seiscomp {
namespace Util {


template <class T, class A>
T join(const A &begin, const A &end, const T &glue) {
	T result;
	bool first = true;
	for ( A it = begin; it != end; ++it ) {
		if ( first )
			first = false;
		else
			result += glue;
		result += *it;
	}
	return result;
}


template <typename T>
void toHex(std::string &out, T v) {
	const unsigned char *bytes = reinterpret_cast<const unsigned char*>(&v);
	for ( int i = sizeof(v)-1; i >= 0; --i )
		toHex(out, bytes[i]);
}


template <>
inline void toHex<unsigned char>(std::string &out, unsigned char v) {
	out += HEXCHARS[v >> 4];
	out += HEXCHARS[v & 0x0F];
}


template <>
inline void toHex<char>(std::string &out, char v) {
	toHex(out, (unsigned char)v);
}


}
}
