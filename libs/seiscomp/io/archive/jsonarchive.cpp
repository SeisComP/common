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


#define SEISCOMP_COMPONENT JSON

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/io/archive/jsonarchive.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/math/math.h>

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdint.h>
#include <limits>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;


#define DATE_FORMAT "%FT%T.%fZ"
#define DATE_FORMAT_SIMPLE "%FT%TZ"


namespace Seiscomp {
namespace IO {

namespace {


struct jsontime {
	jsontime(const Core::Time &r) : ref(r) {}
	const Core::Time &ref;
};

struct jsonstring {
	jsonstring(const string &s) : str(s) {}
	const string &str;
};


ostream &operator<<(ostream &os, const jsontime &js) {
	os << js.ref.toString("\"" DATE_FORMAT "\"");
	return os;
}

ostream &operator<<(ostream &os, const jsonstring &js) {
	// Keep it simple and forward utf-8 unescaped bytes
	size_t l = js.str.length();
	for ( size_t i = 0; i < l; ++i ) {
		char c = js.str[i];
		switch ( c ) {
			case '"':
			case '\\':
				os.write("\\", 1);
				os.write((const char*)&c, 1);
				break;
			case '\b':
				os.write("\\b", 2);
				break;
			case '\f':
				os.write("\\f", 2);
				break;
			case '\n':
				os.write("\\n", 2);
				break;
			case '\r':
				os.write("\\r", 2);
				break;
			case '\t':
				os.write("\\t", 2);
				break;
			default:
				os.write((const char*)&c, 1);
				break;
		}
	}

	return os;
}


struct InputStream {
	typedef streambuf::char_type Ch;

	InputStream(streambuf *buf) : _buf(buf), _pos(0) {}

	Ch Peek() const {
		streambuf::int_type c = _buf->sgetc();
		return c == streambuf::traits_type::eof()?'\0':streambuf::traits_type::to_char_type(c);
	}

	Ch Take() {
		++_pos;
		streambuf::int_type c = _buf->sbumpc();
		return c == streambuf::traits_type::eof()?'\0':streambuf::traits_type::to_char_type(c);
	}

	size_t Tell() const { return _pos; }

	void Put(Ch) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); }
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

	streambuf *_buf;
	size_t _pos;
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
JSONArchive::JSONArchive()
: _forceWriteVersion(-1)
, _deleteBufOnClose(false)
, _deleteStreamOnClose(false)
, _rootObject(true)
, _indent(0)
, _buf(nullptr) {
	_sequenceSize = -1;
	_isSequence = false;
	_attribIndex = 0;
	_os = nullptr;
	_document = nullptr;
	_objectLocation = nullptr;
	_current = nullptr;
	_formattedOutput = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
JSONArchive::JSONArchive(std::streambuf* buf, bool isReading,
                         int forceWriteVersion)
: _forceWriteVersion(forceWriteVersion)
, _deleteBufOnClose(false)
, _deleteStreamOnClose(false)
, _rootObject(true)
, _indent(0)
, _buf(nullptr) {
	_sequenceSize = -1;
	_isSequence = false;
	_attribIndex = 0;
	_os = nullptr;
	_document = nullptr;
	_objectLocation = nullptr;
	_current = nullptr;
	_formattedOutput = false;

	if ( isReading )
		open(buf);
	else
		create(buf);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
JSONArchive::~JSONArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
bool JSONArchive::open(const char* file) {
	close();

	_deleteStreamOnClose = false;

	if ( !strcmp(file, "-") ) {
		_buf = std::cin.rdbuf();
		_deleteBufOnClose = false;
	}
	else {
		std::filebuf *fb = new std::filebuf;
		if ( fb->open(file, std::ios::in | std::ios::binary) == nullptr ) {
			delete fb;
			return false;
		}

		_buf = fb;
		_deleteBufOnClose = true;
	}

	_document = new Document;
	InputStream is(_buf);
	_document->ParseStream(is);
	if ( _document->HasParseError() ) {
		SEISCOMP_ERROR("%s", rapidjson::GetParseError_En(_document->GetParseError()));
		close();
		return false;
	}

	_current = _document;
	parseVersion();

	return Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::open(std::streambuf *buf) {
	close();

	_buf = buf;
	_deleteBufOnClose = false;
	_deleteStreamOnClose = false;

	_document = new Document;
	InputStream is(_buf);
	_document->ParseStream(is);
	if ( _document->HasParseError() ) {
		SEISCOMP_ERROR("%s", rapidjson::GetParseError_En(_document->GetParseError()));
		close();
		return false;
	}

	_current = _document;
	parseVersion();

	return Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::from(const Value *v) {
	close();

	_current = v;
	parseVersion();

	return Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::from(const char *str) {
	close();
	_document = new Document;
	_document->Parse(str);
	if ( _document->HasParseError() ) {
		SEISCOMP_ERROR("%s", rapidjson::GetParseError_En(_document->GetParseError()));
		close();
		return false;
	}
	_current = _document;
	parseVersion();

	return Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::from(char *str, bool insitu) {
	close();
	_document = new Document;
	if ( insitu )
		_document->ParseInsitu(str);
	else
		_document->Parse(str);
	if ( _document->HasParseError() ) {
		SEISCOMP_ERROR("%s", rapidjson::GetParseError_En(_document->GetParseError()));
		close();
		return false;
	}
	_current = _document;
	parseVersion();

	return Core::Archive::open(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::parseVersion() {
	for ( ConstItr itr = _current->MemberBegin(); itr != _current->MemberEnd(); ++itr ) {
		if ( itr->value.IsString() && !strcmp("version", itr->name.GetString()) ) {
			std::string version = itr->value.GetString();
			size_t pos = version.find(".");

			if ( pos != std::string::npos ) {
				int major;
				int minor;

				if ( Core::fromString(major, version.substr(0, pos) ) &&
				     Core::fromString(minor, version.substr(pos + 1, std::string::npos)) )
					setVersion(Core::Version(major, minor));
				else
					setVersion(Core::Version(0,0));
			}
			else {
				int major;

				if (Core::fromString(major, version.substr(0, pos)) )
					setVersion(Core::Version(major ,0));
				else
					setVersion(Core::Version(0,0));
			}

			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::create(const char* file, bool writeVersion) {
	close();

	_indent = 0;

	if ( !strcmp(file, "-") ) {
		_buf = std::cout.rdbuf();
		_os = &std::cout;
		_deleteBufOnClose = false;
		_deleteStreamOnClose = false;
	}
	else {
		std::filebuf* fb = new std::filebuf;

		if ( fb->open(file, std::ios::out | std::ios::binary) == nullptr ) {
			delete fb;
			return false;
		}

		_buf = fb;
		_os = new std::ostream(_buf);

		_deleteBufOnClose = true;
		_deleteStreamOnClose = true;
	}

	createDocument(writeVersion);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::create(std::streambuf *buf, bool writeVersion) {
	close();

	_buf = buf;
	_os = new std::ostream(_buf);

	_deleteBufOnClose = false;
	_deleteStreamOnClose = true;

	createDocument(writeVersion);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::create(std::ostream *os, bool writeVersion) {
	close();

	_buf = os->rdbuf();
	_os = os;

	_deleteBufOnClose = false;
	_deleteStreamOnClose = false;

	createDocument(writeVersion);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::close() {
	_sequenceSize = -1;
	_isSequence = false;
	_attribIndex = 0;

	if ( _buf != nullptr && !_isReading && _rootObject ) {
		if ( _formattedOutput )
			_buf->sputn("\n",1);
		_buf->sputn("}",1);
		if ( _formattedOutput )
			_buf->sputn("\n",1);
	}

	if ( _deleteStreamOnClose && _os )
		delete _os;

	if ( _deleteBufOnClose && _buf )
		delete _buf;

	_buf = nullptr;
	_os = nullptr;

	if ( _document ) {
		delete _document;
		_document = nullptr;
	}

	_objectLocation = nullptr;
	_current = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::createDocument(bool writeVersion) {
	Core::Archive::create(nullptr);

	if ( writeVersion ) {
		if ( _forceWriteVersion >= 0 )
			setVersion(Core::Version(_forceWriteVersion));
		else if ( versionMajor() == 0 && versionMinor() == 0 )
			setVersion(Core::Version(DataModel::Version::Major, DataModel::Version::Minor));
	}
	else
		setVersion(Core::Version(0,0));

	if ( _rootObject ) {
		_buf->sputn("{", 1);
		if ( _formattedOutput ) {
			_buf->sputn("\n", 1);
			++_indent;
		}

		if ( writeVersion ) {
			if ( _formattedOutput )
				_buf->sputn("\t", 1);
			_buf->sputn("\"version\":\"", 11);
			string v = Core::toString(versionMajor()) + "." + Core::toString(versionMinor());
			_buf->sputn(v.c_str(), v.size());
			_attribIndex = 1;
			_buf->sputn("\"", 1);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::setFormattedOutput(bool f) {
	_formattedOutput = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::setRootObject(bool f) {
	_rootObject = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(int8_t &value) {
	if ( !_objectLocation->IsInt() ) {
		SEISCOMP_ERROR("integer expected");
		setValidity(false);
		return;
	}

	int tmp = _objectLocation->GetInt();

	if ( tmp < std::numeric_limits<int8_t>::min() ||
	     tmp > std::numeric_limits<int8_t>::max() ) {
		SEISCOMP_ERROR("int8 expected");
		setValidity(false);
		return;
	}

	value = tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(int16_t &value) {
	if ( !_objectLocation->IsInt() ) {
		SEISCOMP_ERROR("integer expected");
		setValidity(false);
		return;
	}

	int tmp = _objectLocation->GetInt();

	if ( tmp < std::numeric_limits<int16_t>::min() ||
	     tmp > std::numeric_limits<int16_t>::max() ) {
		SEISCOMP_ERROR("int16 expected");
		setValidity(false);
		return;
	}

	value = tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(int32_t &value) {
	if ( !_objectLocation->IsInt() ) {
		SEISCOMP_ERROR("integer expected");
		setValidity(false);
		return;
	}

	value = _objectLocation->GetInt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(int64_t &value) {
	if ( !_objectLocation->IsInt64() ) {
		SEISCOMP_ERROR("integer expected");
		setValidity(false);
		return;
	}

	value = _objectLocation->GetInt64();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(float &value) {
	if ( !_objectLocation->IsNumber() ) {
		if ( _objectLocation->IsString() ) {
			if ( !strcmp(_objectLocation->GetString(), "NaN") ) {
				value = std::numeric_limits<float>::quiet_NaN();
				return;
			}
		}

		SEISCOMP_ERROR("number expected");
		setValidity(false);
		return;
	}

	value = (float)_objectLocation->GetDouble();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(double &value) {
	if ( !_objectLocation->IsNumber() ) {
		if ( _objectLocation->IsString() ) {
			if ( !strcmp(_objectLocation->GetString(), "NaN") ) {
				value = std::numeric_limits<float>::quiet_NaN();
				return;
			}
		}

		SEISCOMP_ERROR("number expected");
		setValidity(false);
		return;
	}

	value = _objectLocation->GetDouble();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<char> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected char array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsInt() ) {
			SEISCOMP_ERROR("integer expected");
			setValidity(false);
			return;
		}

		value.push_back((char)(*_objectLocation)[i].GetInt());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void JSONArchive::readIntVector(std::vector<T> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected int array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsInt() ) {
			SEISCOMP_ERROR("integer expected");
			setValidity(false);
			return;
		}

		value.push_back((*_objectLocation)[i].GetInt());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<int8_t> &value) {
	readIntVector<int8_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<int16_t> &value) {
	readIntVector<int16_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<int32_t> &value) {
	readIntVector<int32_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<int64_t> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected int array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsInt64() ) {
			SEISCOMP_ERROR("integer expected");
			setValidity(false);
			return;
		}

		value.push_back((*_objectLocation)[i].GetInt64());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<float> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected float array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsNumber() ) {
			SEISCOMP_ERROR("number expected");
			setValidity(false);
			return;
		}

		value.push_back((float)(*_objectLocation)[i].GetDouble());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<double> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected double array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsNumber() ) {
			SEISCOMP_ERROR("number expected");
			setValidity(false);
			return;
		}

		value.push_back((*_objectLocation)[i].GetDouble());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<std::string> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected string array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsString() ) {
			SEISCOMP_ERROR("string expected");
			setValidity(false);
			return;
		}

		value.push_back((*_objectLocation)[i].GetString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<Core::Time> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected string array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		if ( !(*_objectLocation)[i].IsString() ) {
			SEISCOMP_ERROR("string expected");
			setValidity(false);
			return;
		}

		Core::Time t;
		if ( !Core::fromString(t, (*_objectLocation)[i].GetString()) ) {
			SEISCOMP_ERROR("invalid time at index %d: %s",
			               i, (*_objectLocation)[i].GetString());
			setValidity(false);
			return;
		}

		value.push_back(t);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::complex<float> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("invalid complex number, expected array notation");
		setValidity(false);
		return;
	}

	if ( _objectLocation->Size() != 2 ) {
		SEISCOMP_ERROR("invalid complex number, expected array notation with 2 components");
		setValidity(false);
		return;
	}

	if ( !(*_objectLocation)[0].IsNumber() || !(*_objectLocation)[1].IsNumber() ) {
		SEISCOMP_ERROR("two numbers expected");
		setValidity(false);
		return;
	}

	value = std::complex<float>((float)(*_objectLocation)[0].GetDouble(),
	                            (float)(*_objectLocation)[1].GetDouble());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::complex<double> &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("invalid complex number, expected array notation");
		setValidity(false);
		return;
	}

	if ( _objectLocation->Size() != 2 ) {
		SEISCOMP_ERROR("invalid complex number, expected array notation with 2 components");
		setValidity(false);
		return;
	}

	if ( !(*_objectLocation)[0].IsNumber() || !(*_objectLocation)[1].IsNumber() ) {
		SEISCOMP_ERROR("two numbers expected");
		setValidity(false);
		return;
	}

	value = std::complex<double>((*_objectLocation)[0].GetDouble(),
	                             (*_objectLocation)[1].GetDouble());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(bool &value) {
	if ( !_objectLocation->IsBool() ) {
		SEISCOMP_ERROR("boolean expected");
		setValidity(false);
		return;
	}

	value = _objectLocation->GetBool();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::vector<std::complex<double> > &value) {
	if ( !_objectLocation->IsArray() ) {
		SEISCOMP_ERROR("expected complex array");
		setValidity(false);
		return;
	}

	Size l = _objectLocation->Size();
	for ( Size i = 0; i < l; ++i ) {
		const Value &item = (*_objectLocation)[i];
		if ( !item.IsArray() ) {
			SEISCOMP_ERROR("invalid complex number, expected array notation");
			setValidity(false);
			return;
		}

		if ( item.Size() != 2 ) {
			SEISCOMP_ERROR("invalid complex number, expected array notation with 2 components");
			setValidity(false);
			return;
		}

		if ( !item[0].IsNumber() || !item[1].IsNumber() ) {
			SEISCOMP_ERROR("two numbers expected");
			setValidity(false);
			return;
		}

		value.push_back(std::complex<double>(item[0].GetDouble(),
		                                     item[1].GetDouble()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(std::string &value) {
	if ( !_objectLocation->IsString() ) {
		SEISCOMP_ERROR("string expected");
		setValidity(false);
		return;
	}

	value = _objectLocation->GetString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::read(Core::Time &value) {
	if ( !_objectLocation->IsString() ) {
		if ( _objectLocation->IsNull() )
			value = Core::Time::Null;
		else {
			SEISCOMP_ERROR("iso string expected, got type %d", _objectLocation->GetType());
			setValidity(false);
		}

		return;
	}

	/*
	double t = _objectLocation->GetDouble()*1E-3;
	long frac = (long)round((t-floor(t))*1E6);
	((TimeSpan&)value).set((long)t);
	value.setUSecs(frac);
	*/
	if ( !value.fromString(_objectLocation->GetString(), DATE_FORMAT) ) {
		if ( !value.fromString(_objectLocation->GetString(), DATE_FORMAT_SIMPLE) ) {
			SEISCOMP_ERROR("invalid iso date string: %s", _objectLocation->GetString());
			setValidity(false);
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void JSONArchive::preAttrib() {
	if ( _attribIndex > 0 ) {
		_buf->sputn(",", 1);
		if ( _formattedOutput ) {
			_buf->sputn("\n",1);
		}
	}
	else if ( _formattedOutput )
		_buf->sputn("\n",1);

	if ( _formattedOutput ) {
		for ( int i = 0; i < _indent; ++i )
			_buf->sputn("\t",1);
	}

	_buf->sputn("\"",1);
	_buf->sputn(_tagname.data(), _tagname.size());
	_buf->sputn("\":",2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void JSONArchive::postAttrib() {
	++_attribIndex;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(int8_t value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << value;
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(int16_t value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << value;
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(int32_t value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << value;
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(int64_t value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << value;
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(float value) {
	if ( !_buf ) return;
	preAttrib();
	if ( Math::isNaN(value) )
		*_os << "\"NaN\"";
	else
		*_os << boost::lexical_cast<std::string>(value);
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(double value) {
	if ( !_buf ) return;
	preAttrib();
	if ( Math::isNaN(value) )
		*_os << "\"NaN\"";
	else
		*_os << boost::lexical_cast<std::string>(value);
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void JSONArchive::writeVector(std::vector<T> &value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << "[";
	for ( size_t i = 0; i < value.size(); ++i ) {
		if ( i ) *_os << ",";
		*_os << value[i];
	}
	*_os << "]";
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<char> &value) {
	writeVector<char>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<int8_t> &value) {
	writeVector<int8_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<int16_t> &value) {
	writeVector<int16_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<int32_t> &value) {
	writeVector<int32_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<int64_t> &value) {
	writeVector<int64_t>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<float> &value) {
	writeVector<float>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<double> &value) {
	writeVector<double>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<std::string> &value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << "[";
	for ( size_t i = 0; i < value.size(); ++i ) {
		if ( i ) *_os << ",";
		*_os << "\"" << jsonstring(value[i]) << "\"";
	}
	*_os << "]";
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<Core::Time> &value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << "[";
	for ( size_t i = 0; i < value.size(); ++i ) {
		if ( i ) *_os << ",";
		*_os << "\"" << Core::toString(value[i]) << "\"";
	}
	*_os << "]";
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::complex<float> &value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << "[" << value.real() << "," << value.imag() << "]";
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::complex<double> &value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << "[" << value.real() << "," << value.imag() << "]";
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(bool value) {
	if ( !_buf ) return;
	preAttrib();
	if ( value )
		_buf->sputn("true", 4);
	else
		_buf->sputn("false", 5);
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::vector<std::complex<double> > &value) {
	if ( !_buf ) return;
	preAttrib();
	*_os << "[";
	for ( size_t i = 0; i < value.size(); ++i ) {
		if ( i ) *_os << ",";
		*_os << "[" << value[i].real() << "," << value[i].imag() << "]";
	}
	*_os << "]";
	postAttrib();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(std::string &value) {
	if ( !_buf ) return;

	if ( (hint() & XML_MANDATORY) || !value.empty() ) {
		preAttrib();
		*_os << "\"" << jsonstring(value) << "\"";
		postAttrib();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::write(Core::Time &value) {
	if ( !_buf ) return;
	if ( (hint() & XML_MANDATORY) || value.valid() ) {
		preAttrib();
		*_os << jsontime(value);
		postAttrib();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::locateObjectByName(const char* name,
                                     const char* targetClass,
                                     bool nullable) {
	if ( isReading() ) {
		if ( _current == nullptr )
			return false;

		if ( targetClass ) {
			if ( (hint() & STATIC_TYPE) == 0 ) {
				_objectLocation = findTag(_current, _currentIndex, name, targetClass);
				return _objectLocation != nullptr;
			}
			else {
				_objectLocation = findTag(_current, _currentIndex, name, nullptr);
				return _objectLocation != nullptr;
			}
		}
		else {
			if ( name && *name ) {
				_objectLocation = findAttrib(_current, name);
				return _objectLocation != nullptr;
			}
			else {
				RAPIDJSON_ASSERT(false);
			}
		}

		return false;
	}
	else {
		_nullable = nullable;

		if ( !targetClass ) {
			_isClass = false;
			if ( name )
				_tagname = name;
			else
				_tagname.clear();
		}
		else {
			_isClass = true;
			if ( name )
				_tagname = name;
			else
				_tagname = targetClass;
		}

		if ( _isSequence ) {
			if ( _attribIndex > 0 ) {
				_buf->sputn(",",1);
			}
			if ( _formattedOutput ) {
				_buf->sputn("\n", 1);
				for ( int i = 0; i < _indent; ++i ) {
					_buf->sputn("\t", 1);
				}
			}
			_buf->sputn("\"", 1);
			_buf->sputn(_tagname.data(), _tagname.size());
			_buf->sputn("\":", 2);

			if ( _formattedOutput ) {
				_buf->sputn("\n", 1);
				for ( int i = 0; i < _indent; ++i ) {
					_buf->sputn("\t", 1);
				}
			}

			_buf->sputn("[", 1);

			if ( _formattedOutput ) {
				_buf->sputn("\n", 1);
			}

			++_indent;
			++_attribIndex;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool JSONArchive::locateNextObjectByName(const char* name,
                                         const char* targetClass) {
	if ( isReading() ) {
		if ( _current == nullptr ) return false;

		if ( targetClass != nullptr ) {
			if ( (hint() & STATIC_TYPE) == 0 ) {
				_objectLocation = findNextTag(_currentArray, _currentIndex, name, targetClass);
				return _objectLocation != nullptr;
			}
			else {
				_objectLocation = findNextTag(_currentArray, _currentIndex, name, nullptr);
				return _objectLocation != nullptr;
			}
		}
		else {
			if ( name && *name ) {
				_objectLocation = findAttrib(_current, name);
				return _objectLocation != nullptr;
			}
			else {
				RAPIDJSON_ASSERT(false);
			}
		}

		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::locateNullObjectByName(const char* name,
                                         const char* targetClass,
                                         bool first) {
	_nullable = true;

	if ( targetClass )
		setClassName(targetClass);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string JSONArchive::determineClassName() {
	if ( _current == nullptr || _objectLocation == nullptr )
		return "";

	if ( !_current->IsObject() )
		return "";

	for ( Value::ConstMemberIterator itr = _current->MemberBegin();
	      itr != _current->MemberEnd(); ++itr ) {
		if ( &itr->value != _objectLocation ) continue;
		return itr->name.GetString();
	}

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::setClassName(const char *className) {
	if ( !_buf ) return;

	if ( className ) {
		_tagname = className;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::readSequence() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::writeSequence(int size) {
	if ( !size ) {
		_sequenceSize = -1;
		_isSequence = false;
		return;
	}

	_isSequence = true;
	_sequenceSize = size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline void JSONArchive::_serialize(T &target) {
	if ( isReading() ) {
		const Value *backupCurrent = _current;
		const Value *backupCurrentArray = _currentArray;
		const Value *backupLocation = _objectLocation;
		int backupIdx = _currentIndex;

		_current = _objectLocation;

		Core::Archive::serialize(target);

		_currentIndex = backupIdx;
		_current = backupCurrent;
		_currentArray = backupCurrentArray;
		_objectLocation = backupLocation;
	}
	else {
		if ( _sequenceSize > 0 ) --_sequenceSize;
		if ( _attribIndex > 0 && !_isSequence ) {
			_buf->sputn(",",1);
		}

		if ( _formattedOutput && _isClass ) {
			if ( !_isSequence ) {
				_buf->sputn("\n", 1);
			}
			for ( int i = 0; i < _indent; ++i ) {
				_buf->sputn("\t", 1);
			}
		}

		bool unwrappedObject = _isSequence && (hint() & STATIC_TYPE);

		if ( _isClass ) {
			if ( unwrappedObject ) {
				//
			}
			else {
				++_attribIndex;
				if ( _isSequence ) {
					++_indent;
					_buf->sputn("{",1);
					if ( _formattedOutput ) {
						_buf->sputn("\n", 1);
						for ( int i = 0; i < _indent; ++i ) {
							_buf->sputn("\t", 1);
						}
					}
				}

				_buf->sputn("\"",1);
				_buf->sputn(_tagname.data(), _tagname.size());
				_buf->sputn("\":",2);

				if ( _formattedOutput ) {
					_buf->sputn("\n", 1);
				}
			}
		}
		else {
			++_attribIndex;
		}

		if ( _formattedOutput && _isClass && !unwrappedObject ) {
			for ( int i = 0; i < _indent; ++i )
				_buf->sputn("\t", 1);
		}

		if ( _isClass ) {
			_buf->sputn("{",1);

			if ( _formattedOutput ) {
				//_buf->sputn("\n", 1);
				++_indent;
			}
		}

		int tmpSeqSize = _sequenceSize;
		bool tmpIsSequence = _isSequence;
		int tmpAttribIndex = _attribIndex;
		bool tmpIsClass = _isClass;
		_sequenceSize = -1;
		_isSequence = false;
		_attribIndex = 0;

		Core::Archive::serialize(target);
		_sequenceSize = tmpSeqSize;
		_isSequence = tmpIsSequence;
		_attribIndex = tmpAttribIndex;
		_isClass = tmpIsClass;

		if ( _isClass ) {
			if ( _formattedOutput ) {
				--_indent;
				_buf->sputn("\n", 1);
				for ( int i = 0; i < _indent; ++i )
					_buf->sputn("\t", 1);
			}

			_buf->sputn("}", 1);

			if ( _isSequence && !unwrappedObject ) {
				--_indent;
				if ( _formattedOutput ) {
					_buf->sputn("\n",1);
					for ( int i = 0; i < _indent; ++i ) {
						_buf->sputn("\t", 1);
					}
				}

				_buf->sputn("}", 1);
			}
		}

		if ( _sequenceSize > 0 ) {
			_buf->sputn(",", 1);
			if ( _formattedOutput )
				_buf->sputn("\n",1);
		}

		if ( !_sequenceSize ) {
			--_indent;
			if ( _formattedOutput ) {
				if ( _formattedOutput ) {
					_buf->sputn("\n", 1);
				}
				for ( int i = 0; i < _indent; ++i )
					_buf->sputn("\t", 1);
			}
			_buf->sputn("]", 1);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::serialize(RootType *object) {
	_serialize(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void JSONArchive::serialize(SerializeDispatcher &disp) {
	_serialize(disp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const JSONArchive::Value *JSONArchive::findAttrib(const Value *node, const char* name) {
	// find the child node with name = param name
	if ( !node->IsObject() )
		return nullptr;

	for ( Value::ConstMemberIterator itr = node->MemberBegin(); itr != node->MemberEnd(); ++itr ) {
		// Tag matches requested name?
		if ( !strcmp(itr->name.GetString(), name) )
			return &itr->value;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const JSONArchive::Value *
JSONArchive::findTag(const Value *node, Size &index,
                     const char* name, const char* targetClass) {
	// find the child node with name = param name
	if ( !node->IsObject() )
		return nullptr;

	for ( ConstItr itr = node->MemberBegin(); itr != node->MemberEnd(); ++itr ) {
		if ( !itr->value.IsObject() && !itr->value.IsArray() ) continue;

		// Tag does not match the requested name?
		if ( itr->value.IsArray() ) {
			if ( strcmp(itr->name.GetString(), name) ) {
				if ( !Core::ClassFactory::IsTypeOf(name, itr->name.GetString()) )
					continue;
			}

			_currentArray = &itr->value;
			index = 0;

			if ( _currentArray->Size() <= index )
				return nullptr;

			if ( targetClass ) {
				// Dynamic classes are wrapped
				auto wrapper = &(*_currentArray)[index];
				if ( wrapper->MemberBegin() != wrapper->MemberEnd() ) {
					_current = wrapper;
					return &_current->MemberBegin()->value;
				}

				return nullptr;
			}
			else {
				return &(*_currentArray)[index];
			}
		}
		else {
			if ( name && *name ) {
				if ( strcmp(itr->name.GetString(), name) ) {
					if ( !Core::ClassFactory::IsTypeOf(name, itr->name.GetString()) ) {
						continue;
					}
				}
			}
			else if ( targetClass ) {
				if ( !Core::ClassFactory::IsTypeOf(targetClass, itr->name.GetString()) ) {
					continue;
				}
			}
			else {
				continue;
			}

			index = -1;
		}

		return &itr->value;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const JSONArchive::Value *
JSONArchive::findNextTag(const Value *node, Size &index,
                         const char* name, const char* targetClass) {
	if ( _currentIndex < 0 ) return nullptr;
	++_currentIndex;
	if ( _currentIndex >= node->Size() )
		return nullptr;

	if ( targetClass ) {
		// Dynamic classes are wrapped
		auto wrapper = &(*node)[_currentIndex];
		if ( wrapper->MemberBegin() != wrapper->MemberEnd() ) {
			_current = wrapper;
			return &_current->MemberBegin()->value;
		}

		return nullptr;
	}
	else {
		return &(*node)[_currentIndex];
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
