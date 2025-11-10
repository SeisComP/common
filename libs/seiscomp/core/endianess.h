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


#ifndef SEISCOMP_CORE_ENDIANESS_H
#define SEISCOMP_CORE_ENDIANESS_H


#include <iostream>
#include <cstdint>
#include <streambuf>


namespace Seiscomp::Core::Endianess {


template <typename T1, typename T2> inline const T1* lvalue(const T2 &value) {
	return reinterpret_cast<const T1*>(&value);
}


template <typename T, int swap, size_t size>
struct Swapper {
	constexpr static T Take(const T &v) {
		return v;
	}

	constexpr static void Take(T *ar, size_t len) {}
};


template <typename T>
struct Swapper<T,1,2> {
	constexpr static T Take(const T &v) {
		return *lvalue<T, uint16_t>((*reinterpret_cast<const uint16_t*>(&v) >> 0x08) |
		       (*reinterpret_cast<const uint16_t*>(&v) << 0x08));
	}

	constexpr static void Take(T *ar, size_t len) {
		for ( size_t i = 0; i < len; ++i ) {
			ar[i] = Take(ar[i]);
		}
	}
};


template <typename T>
struct Swapper<T,1,4> {
	constexpr static T Take(const T &v) {
		return *lvalue<T, uint32_t>(((*reinterpret_cast<const uint32_t*>(&v) << 24) & 0xFF000000) |
		       ((*reinterpret_cast<const uint32_t*>(&v) << 8)  & 0x00FF0000) |
		       ((*reinterpret_cast<const uint32_t*>(&v) >> 8)  & 0x0000FF00) |
		       ((*reinterpret_cast<const uint32_t*>(&v) >> 24) & 0x000000FF));
	}

	constexpr static void Take(T *ar, size_t len) {
		for ( size_t i = 0; i < len; ++i ) {
			ar[i] = Take(ar[i]);
		}
	}
};


template <typename T>
struct Swapper<T,1,8> {
	constexpr static T Take(const T &v) {
		return *lvalue<T, uint64_t>(((*reinterpret_cast<const uint64_t*>(&v) << 56) & 0xFF00000000000000LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) << 40) & 0x00FF000000000000LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) << 24) & 0x0000FF0000000000LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) << 8)  & 0x000000FF00000000LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) >> 8)  & 0x00000000FF000000LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) >> 24) & 0x0000000000FF0000LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) >> 40) & 0x000000000000FF00LL) |
		       ((*reinterpret_cast<const uint64_t*>(&v) >> 56) & 0x00000000000000FFLL));
	}

	constexpr static void Take(T *ar, size_t len) {
		for ( size_t i = 0; i < len; ++i ) {
			ar[i] = Take(ar[i]);
		}
	}
};


template <int size>
struct TypeMap {
	using ValueType = void;
};


template <>
struct TypeMap<1> {
	using ValueType = uint8_t;
};


template <>
struct TypeMap<2> {
	using ValueType = uint16_t;
};


template <>
struct TypeMap<4> {
	using ValueType = uint32_t;
};


template <>
struct TypeMap<8> {
	using ValueType = uint64_t;
};


template <int swap, size_t size>
struct ByteSwapper {
	constexpr static void Take(void *ar, size_t len) {}
};


template <size_t size>
struct ByteSwapper<1, size> {
	constexpr static void Take(void *ar, size_t len) {
		using T = typename TypeMap<size>::ValueType;
		Swapper<T,1,size>::Take(reinterpret_cast<T*>(ar), len);
	}
};


template <typename T>
constexpr void swap(T *ar, size_t len = 1) {
	Swapper<T, 1, sizeof(T)>::Take(ar, len);
}


struct Current {
	enum {
		LittleEndian = ((0x1234 >> 8) == 0x12?1:0),
		BigEndian = 1 - LittleEndian
	};
};


struct Converter {
	template <typename T>
	constexpr static T ToLittleEndian(const T &v) {
		return Swapper<T,Current::BigEndian,sizeof(T)>::Take(v);
	}

	template <typename T>
	constexpr static void ToLittleEndian(T *data, size_t len) {
		Swapper<T,Current::BigEndian,sizeof(T)>::Take(data, len);
	}

	template <typename T>
	constexpr static T FromLittleEndian(const T &v) {
		return Swapper<T,Current::BigEndian,sizeof(T)>::Take(v);
	}

	template <typename T>
	constexpr static T ToBigEndian(const T &v) {
		return Swapper<T,Current::LittleEndian,sizeof(T)>::Take(v);
	}

	template <typename T>
	constexpr static T FromBigEndian(const T &v) {
		return Swapper<T,Current::LittleEndian,sizeof(T)>::Take(v);
	}
};

struct Reader {
	Reader(std::streambuf &input) : stream(input), good(true) {}

	void operator()(void *data, int size) {
		good = stream.sgetn((char*)data, size) == size;
	}

	template <typename T>
	void operator()(T &v) {
		good = stream.sgetn((char*)&v, sizeof(T)) == sizeof(T);
		v = Converter::FromLittleEndian(v);
	}

	std::streambuf &stream;
	bool good;
};

struct Writer {
	Writer(std::streambuf &output) : stream(output), good(true) {}

	void operator()(const void *data, int size) {
		good = stream.sputn((char*)data, size) == size;
	}

	template <typename T>
	void operator()(T &v) {
		T tmp = Converter::ToLittleEndian(v);
		good = stream.sputn((char*)&tmp, sizeof(T)) == sizeof(T);
	}

	std::streambuf &stream;
	bool good;
};


}


#endif
