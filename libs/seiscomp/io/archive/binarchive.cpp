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


#define SEISCOMP_COMPONENT BinaryArchive
#include <seiscomp/logging/log.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/core/exceptions.h>

#include <iostream>
#include <fstream>
#include <string.h>


namespace {


bool IsBigEndian() {
	short word = 0x1234;
	return (*(char*)&word) == 0x12;
}

#define Is64Bit (sizeof(time_t) == 8)

//typedef int64_t dateint;
typedef int32_t dateint;


template<int N, typename T>
struct Swapper {
	static T Take(const T& value) {
		T ret;
		register int i = 0;
		register int j = N;
		const unsigned char* in = reinterpret_cast<const unsigned char*>(&value);
		unsigned char* out = reinterpret_cast<unsigned char*>(&ret);
		while ( --j ) {
			out[j] = in[i];
			++i;
		}
	
		return ret;
	}
};

template<typename T>
struct Swapper<1,T> {
	static T Take(const T& value) {
		return value;
	}
};

template<typename T>
struct Swapper<2,T> {
	static T Take(const T& value) {
		return (static_cast<const unsigned short&>(value) >> 0x08) |
		       (static_cast<const unsigned short&>(value) << 0x08);
	}
};

template<typename T>
struct Swapper<4,T> {
	static T Take(const T& value) {
		return  (static_cast<const unsigned long&>(value) << 24) |
		       ((static_cast<const unsigned long&>(value) << 8) & 0x00FF0000) |
		       ((static_cast<const unsigned long&>(value) >> 8) & 0x0000FF00) |
		        (static_cast<const unsigned long&>(value) >> 24);
	}
};

template <typename T>
T Convert(T value) {
	return IsBigEndian()?Swapper<sizeof(T),T>::Take(value):value;
}


char MAGIC[] = "SCBA";


}


namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BinaryArchive::BinaryArchive() : _buf(nullptr), _deleteOnClose(false) {
	_sequenceSize = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BinaryArchive::BinaryArchive(std::streambuf* buf, bool isReading) {
	_buf = buf;
	_isReading = isReading;
	_deleteOnClose = false;
	_sequenceSize = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BinaryArchive::~BinaryArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BinaryArchive::open(const char* file) {
	close();

	if ( !strcmp(file, "-") ) {
		_buf = std::cin.rdbuf();
		_deleteOnClose = false;
	}
	else {
		std::filebuf* fb = new std::filebuf;
		if ( fb->open(file, std::ios::in | std::ios::binary) == nullptr ) {
			delete fb;
			return false;
		}

		_buf = fb;
		_deleteOnClose = true;
	}

	return Seiscomp::Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BinaryArchive::open(std::streambuf* buf) {
	close();
	_buf = buf;
	_deleteOnClose = false;

	return Seiscomp::Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BinaryArchive::create(const char* file) {
	close();

	if ( !strcmp(file, "-") ) {
		_buf = std::cout.rdbuf();
		_deleteOnClose = false;
	}
	else {
		std::filebuf* fb = new std::filebuf;

		if ( fb->open(file, std::ios::out | std::ios::binary) == nullptr ) {
			delete fb;
			return false;
		}

		_buf = fb;
		_deleteOnClose = true;
	}

	return Seiscomp::Core::Archive::create(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BinaryArchive::create(std::streambuf* buf) {
	close();

	_buf = buf;
	_deleteOnClose = false;

	return Seiscomp::Core::Archive::create(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::close() {
	if ( _deleteOnClose && _buf )
		delete _buf;

	_classes.clear();
	_sequenceSize = -1;

	_buf = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BinaryArchive::readInt(T& value) {
	int size = _buf?_buf->sgetn((char*)&value, sizeof(T)):0;
	if ( size != sizeof(T) ) {
		SEISCOMP_ERROR("read(int): expected %d bytes from stream, got %d", (int)sizeof(T), size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(int8_t& value) {
	readInt<int8_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(int16_t& value) {
	readInt<int16_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(int32_t& value) {
	readInt<int32_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(int64_t& value) {
	readInt<int64_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(float& value) {
	int size = _buf?_buf->sgetn((char*)&value, sizeof(float)):0;
	if ( size != sizeof(float) ) {
		SEISCOMP_ERROR("read(float): expected %d bytes from stream, got %d", (int)sizeof(float), size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(double& value) {
	int size = _buf?_buf->sgetn((char*)&value, sizeof(double)):0;
	if ( size != sizeof(double) ) {
		SEISCOMP_ERROR("read(double): expected %d bytes from stream, got %d", (int)sizeof(double), size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<char>& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	vsize = vsize * sizeof(char);
	size = _buf->sgetn((char*)value.data(), vsize);
	if ( size != vsize ) {
		SEISCOMP_ERROR("read(char*): expected %d bytes from stream, got %d", vsize, size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BinaryArchive::readIntVector(std::vector<T>& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	vsize = vsize * sizeof(int);
	size = _buf->sgetn((char*)value.data(), vsize);
	if ( size != vsize ) {
		SEISCOMP_ERROR("read(int*): expected %d bytes from stream, got %d", vsize, size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<int8_t>& value) {
	readIntVector<int8_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<int16_t>& value) {
	readIntVector<int16_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<int32_t>& value) {
	readIntVector<int32_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<int64_t>& value) {
	readIntVector<int64_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<float>& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	vsize = vsize * sizeof(float);
	size = _buf->sgetn((char*)value.data(), vsize);
	if ( size != vsize ) {
		SEISCOMP_ERROR("read(float*): expected %d bytes from stream, got %d", vsize, size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<double>& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	vsize = vsize * sizeof(double);
	size = _buf->sgetn((char*)value.data(), vsize);
	if ( size != vsize ) {
		SEISCOMP_ERROR("read(double*): expected %d bytes from stream, got %d", vsize, size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<std::string>& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	for ( size_t i = 0; i < value.size(); ++i ) {
		read(value[i]);
		if ( !success() ) return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<Core::Time>& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	for ( size_t i = 0; i < value.size(); ++i ) {
		read(value[i]);
		if ( !success() ) return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::complex<float>& value) {
	int size = _buf?_buf->sgetn((char*)&value, sizeof(std::complex<float>)):0;
	if ( size != sizeof(std::complex<float>) ) {
		SEISCOMP_ERROR("read(complex<float>): expected %d bytes from stream, got %d", (int)sizeof(std::complex<float>), size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::complex<double>& value) {
	int size = _buf?_buf->sgetn((char*)&value, sizeof(std::complex<double>)):0;
	if ( size != sizeof(std::complex<double>) ) {
		SEISCOMP_ERROR("read(complex<double>): expected %d bytes from stream, got %d", (int)sizeof(std::complex<double>), size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(bool& value) {
	char tmp;
	int size = _buf?_buf->sgetn(&tmp, sizeof(char)):0;
	if ( size != sizeof(char) ) {
		SEISCOMP_ERROR("read(bool): expected %d bytes from stream, got %d", (int)sizeof(char), size);
		setValidity(false);
	}
	else
		value = (bool)tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::vector<std::complex<double> >& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int vsize;
	int size = _buf->sgetn((char*)&vsize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(array.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	value.resize(vsize);
	vsize = vsize * sizeof(std::complex<double>);
	size = _buf->sgetn((char*)value.data(), vsize);

	if ( size != vsize ) {
		SEISCOMP_ERROR("read(complex<double>*): expected %d bytes from stream, got %d", vsize, size);
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(std::string& value) {
	if ( !_buf ) {
		setValidity(false);
		return;
	}

	int ssize;
	int size = _buf->sgetn((char*)&ssize, sizeof(int));
	if ( size != sizeof(int) ) {
		SEISCOMP_ERROR("read(string.len): expected %d bytes from stream, got %d", (int)sizeof(int), size);
		setValidity(false);
		return;
	}

	if ( !ssize )
		value.clear();
	else {
		value.resize(ssize);
		ssize = ssize * sizeof(std::string::value_type);
		size = _buf->sgetn((char*)value.data(), ssize);
		if ( size != ssize ) {
			SEISCOMP_ERROR("read(string): expected %d bytes from stream, got %d", ssize, size);
			setValidity(false);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::read(Seiscomp::Core::Time& value) {
	dateint tmpSeconds, tmpUSeconds;
	int size = _buf?_buf->sgetn((char*)&tmpSeconds, sizeof(tmpSeconds)):0;
	size += _buf?_buf->sgetn((char*)&tmpUSeconds, sizeof(tmpUSeconds)):0;
	if ( size != sizeof(tmpSeconds) + sizeof(tmpUSeconds) ) {
		SEISCOMP_ERROR("read(datetime): expected %d bytes from stream, got %d", int(sizeof(tmpSeconds) + sizeof(tmpUSeconds)), size);
		setValidity(false);
	}
	value = Seiscomp::Core::Time(tmpSeconds, tmpUSeconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline int BinaryArchive::writeBytes(const void* buf, int size) {
	return _buf->sputn((const char*)buf, size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(int8_t value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(int8_t));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(int16_t value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(int16_t));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(int32_t value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(int32_t));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(int64_t value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(int64_t));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(float value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(float));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(double value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(double));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BinaryArchive::writeVector(std::vector<T>& value) {
	if ( !_buf ) return;
	int32_t vsize = value.size();
	writeBytes(&vsize, sizeof(int32_t));
	vsize *= sizeof(T);
	writeBytes(value.data(), vsize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<char>& value) {
	writeVector<char>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<int8_t>& value) {
	writeVector<int8_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<int16_t>& value) {
	writeVector<int16_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<int32_t>& value) {
	writeVector<int32_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<int64_t>& value) {
	writeVector<int64_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<float>& value) {
	writeVector<float>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<double>& value) {
	writeVector<double>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<std::string>& value) {
	if ( !_buf ) return;

	int vsize = value.size();
	writeBytes(&vsize, sizeof(int));
	for ( size_t i = 0; i < value.size(); ++i )
		write(value[i]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<Core::Time>& value) {
	if ( !_buf ) return;

	int vsize = value.size();
	writeBytes(&vsize, sizeof(int));
	for ( size_t i = 0; i < value.size(); ++i )
		write(value[i]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::complex<float>& value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(std::complex<float>));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::complex<double>& value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(std::complex<double>));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(bool value) {
	if ( !_buf ) return;
	writeBytes(&value, sizeof(char));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::vector<std::complex<double> >& value) {
	if ( !_buf ) return;
	int vsize = value.size();
	writeBytes(&vsize, sizeof(int));
	vsize *= sizeof(std::complex<double>);
	writeBytes(value.data(), vsize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(std::string& value) {
	if ( !_buf ) return;
	int ssize = value.size();
	writeBytes(&ssize, sizeof(int));
	ssize *= sizeof(std::string::value_type);
	writeBytes(value.data(), ssize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::write(Seiscomp::Core::Time& value) {
	if ( !_buf ) return;
	dateint tmpSeconds = (dateint)value.seconds();
	dateint tmpUSeconds = (dateint)value.microseconds();
	writeBytes(&tmpSeconds, sizeof(tmpSeconds));
	writeBytes(&tmpUSeconds, sizeof(tmpUSeconds));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BinaryArchive::locateObjectByName(const char* name,
                                       const char* targetClass,
                                       bool nullable) {
	if ( !_buf ) return false;

	if ( isReading() ) {
		// if the first object of a sequence has to be read and
		// the sequence size is 0, skip this sequence
		if ( _sequenceSize == 0 ) {
			_sequenceSize = -1;
			return false;
		}

		if ( _sequenceSize > 0 ) {
			--_sequenceSize;
		}

		if ( targetClass ) {
			int class_id;
			read(class_id);
			if ( class_id == -1 ) {
				read(_classname);
				//std::cout << "read raw classname " << _classname << std::endl;
				_classes.push_back(_classname);
			}
			else {
				if ( class_id >= 0 && class_id < (int)_classes.size() )
					_classname = _classes[class_id];
				else
					throw Seiscomp::Core::StreamException("unknown class id");
			}

			if ( !Seiscomp::Core::ClassFactory::IsTypeOf(targetClass, _classname.c_str()) ) {
				throw Seiscomp::Core::StreamException(std::string("expected exact or derived from ")
				                                      + targetClass + ", found " + _classname);
				return false;
			}
		}

		if ( nullable ) {
			bool used;
			read(used);
			if ( used == 0 )
				return false;
			else if ( used != 1 )
				throw Seiscomp::Core::StreamException("wrong 'used' token (expected 0 or 1)");
		}
	}
	else {
		_nullable = nullable;
		_usedObject = true;

		if ( targetClass )
			_classname = targetClass;
		else if ( _nullable )
			write(_usedObject);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BinaryArchive::locateNextObjectByName(const char* name,
                                           const char* targetClass) {
	return locateObjectByName(name, targetClass, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::locateNullObjectByName(const char* name,
                                           const char* targetClass,
                                           bool first) {
	if ( !_buf ) return;

	_nullable = true;
	_usedObject = false;

	if ( targetClass )
		setClassName(targetClass);
	else
		write(_usedObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string BinaryArchive::determineClassName() {
	return _classname;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::setClassName(const char* className) {
	if ( !_buf ) return;

	if ( className )
		_classname = className;

	if ( _classname.empty() ) return;
	
	int class_id = classId(_classname);
	write(class_id);
	if ( class_id < 0 )
		write(_classname);
	_classname.clear();

	if ( _nullable )
		write(_usedObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int BinaryArchive::classId(const std::string& classname) {
	for ( size_t i = 0; i < _classes.size(); ++i )
		if ( _classes[i] == classname )
			return i;

	_classes.push_back(classname);

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::readSequence() {
	_sequenceSize = 0;
	read(_sequenceSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::writeSequence(int size) {
	_sequenceSize = size;
	write(_sequenceSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::serialize(RootType* object) {
	int tmpSeqSize = _sequenceSize;
	_sequenceSize = -1;
	Seiscomp::Core::Archive::serialize(object);
	_sequenceSize = tmpSeqSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BinaryArchive::serialize(SerializeDispatcher& disp) {
	int tmpSeqSize = _sequenceSize;
	_sequenceSize = -1;
	Seiscomp::Core::Archive::serialize(disp);
	_sequenceSize = tmpSeqSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VBinaryArchive::VBinaryArchive(int forceWriteVersion)
: _forceWriteVersion(forceWriteVersion) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VBinaryArchive::VBinaryArchive(std::streambuf* buf, bool isReading,
                               int forceWriteVersion)
: BinaryArchive(buf, isReading),  _forceWriteVersion(forceWriteVersion) {
	if ( isReading ) {
		if ( !readHeader() ) throw Core::StreamException(errorMsg());
	}
	else
		writeHeader();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void VBinaryArchive::setWriteVersion(int version) {
	_forceWriteVersion = version;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool VBinaryArchive::open(const char* file) {
	_error = "";

	if ( !BinaryArchive::open(file) ) return false;

	if ( !readHeader() ) {
		close();
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool VBinaryArchive::open(std::streambuf *buf) {
	_error = "";

	if ( !BinaryArchive::open(buf) ) return false;

	if ( !readHeader() ) {
		close();
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool VBinaryArchive::create(const char* file) {
	_error = "";
	if ( !BinaryArchive::create(file) ) return false;
	writeHeader();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool VBinaryArchive::create(std::streambuf *buf) {
	_error = "";
	if ( !BinaryArchive::create(buf) ) return false;
	writeHeader();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void VBinaryArchive::close() {
	BinaryArchive::close();
	_error = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *VBinaryArchive::errorMsg() const {
	return _error.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void VBinaryArchive::writeHeader() {
	if ( _forceWriteVersion == 0 ) {
		setVersion(Core::Version(0,0));
		return;
	}

	if ( _forceWriteVersion == -1 )
		setVersion(Core::Version(DataModel::Version::Major,DataModel::Version::Minor));
	else
		_version = _forceWriteVersion;

	writeBytes(MAGIC, strlen(MAGIC));

	uint32_t versionTag = _version.majorTag() << 0x10 | _version.minorTag();
	write((int)versionTag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool VBinaryArchive::readHeader() {
	const char *magic = MAGIC;
	while ( *magic != '\0' ) {
		if ( _buf->sgetc() == *magic ) {
			++magic;
			_buf->snextc();
			continue;
		}
		else {
			_error = "invalid header, expected ";
			_error += *magic;
			break;
		}
	}

	if ( !_error.empty() ) {
		while ( magic > MAGIC ) {
			_buf->sungetc();
			--magic;
		}
		_version = 0;
		SEISCOMP_DEBUG("reading unversioned binary");
		return true;
	}

	int v;
	read(v);

	uint32_t versionTag = uint32_t(v);
	setVersion(Core::Version(versionTag >> 0x10, versionTag & 0xffff));

	if ( v <= 0 || versionMajor() < 0 || versionMinor() < 0 ) {
		_error = "invalid version";
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
