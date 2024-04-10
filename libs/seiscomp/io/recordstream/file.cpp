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


#define SEISCOMP_COMPONENT RECORDFILE
#include "file.h"
#include <seiscomp/core/strings.h>
#include <seiscomp/core/system.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/system/environment.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::RecordStream;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_RECORDSTREAM(File, "file");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::File(string name) {
	File::setSource(name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::File(const File &f) {
	File::setSource(f.name());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File::~File() {
	File::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File &File::operator=(const File &f) {
	if ( this != &f ) {
		setSource(f.name());
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setSource(const string &name) {
	_name = name;
	_closeRequested = false;
	_current = &_fstream;

	if ( _fstream.is_open() ) {
		_fstream.close();
	}

	setRecordType("mseed");

	if ( _name == "-" ) {
		_current = &cin;
		return !cin.bad();
	}

	auto absPath = Environment::Instance()->absolutePath(_name);
	if ( !SC_FS_IS_REGULAR_FILE(SC_FS_PATH(absPath)) ) {
		SEISCOMP_ERROR("Source is not a regular file: %s", absPath.c_str());
		return false;
	}

	size_t pos = name.rfind('.');
	if ( pos != string::npos ) {
		string ext = name.substr(pos + 1);
		if ( ext == "xml" ) {
			setRecordType("xml");
		}
		else if ( ext == "bin" ) {
			setRecordType("binary");
		}
		else if ( ext == "ah" ) {
			setRecordType("ah");
		}
	}

	_fstream.open(absPath.c_str(), ifstream::in | ifstream::binary);
	if ( !_fstream.is_open() ) {
		SEISCOMP_ERROR("Could not open record file: %s", absPath.c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::addStream(const string &net, const string &sta,
                     const string &loc, const string &cha) {
	auto id = net + "." + sta + "." + loc + "." + cha;
	if ( id.find_first_of("*?") == std::string::npos ) {
		_filter.emplace(id, TimeWindowFilter());
	}
	else { // wildcards characters are present
		_reFilter.emplace_back(id, TimeWindowFilter());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::addStream(const string &net, const string &sta,
                     const string &loc, const string &cha,
                     const Seiscomp::Core::Time &startTime,
                     const Seiscomp::Core::Time &endTime) {
	auto id = net + "." + sta + "." + loc + "." + cha;
	if ( id.find_first_of("*?") == std::string::npos ) {
		_filter.emplace(id, TimeWindowFilter(startTime, endTime));
	}
	else { // wildcards characters are present
		_reFilter.emplace_back(id, TimeWindowFilter(startTime, endTime));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setStartTime(const Seiscomp::Core::Time &startTime) {
	_startTime = startTime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setEndTime(const Seiscomp::Core::Time &endTime) {
	_endTime = endTime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void File::close() {
	_closeRequested = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool File::setRecordType(const char *type) {
	auto *factory = RecordFactory::Find(type);
	if ( !factory ) {
		SEISCOMP_ERROR("Unknown record type '%s'", type);
		return false;
	}

	_factory = factory;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const File::TimeWindowFilter* File::findTimeWindowFilter(Record *rec) {
	const string streamID = rec->streamID();

	// First look for fully qualified stream id (no wildcards)
	const auto &it = _filter.find(streamID);
	if ( it != _filter.end() ) {
		return &it->second;
	}

	// then search the wildcarded filters
	for ( const auto &pair : _reFilter ) {
		const auto &wild = pair.first;
		const auto &twf = pair.second;
		if ( Core::wildcmp(wild, streamID) ) {
			// now add this stream to the fully qualified ones, so that
			// next record with the same stream will be resolved without
			// going through this loop
			_filter.emplace(streamID, twf);
			return &twf;
		}
	}

	// no matches
	return nullptr;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *File::next() {
	if ( _closeRequested ) {
		if ( _current == &_fstream ) {
			_fstream.close();
		}
		else {
			_current = &_fstream;
		}
		_filter.clear();
		_reFilter.clear();
		_closeRequested = false;
		return nullptr;
	}

	if ( !*_current || !_factory ) {
		return nullptr;
	}

	while ( !_closeRequested ) {
		auto *rec = _factory->create();
		if ( !rec ) {
			return nullptr;
		}

		setupRecord(rec);

		try {
			rec->read(*_current);
		}
		catch ( Core::EndOfStreamException & ) {
			delete rec;
			return nullptr;
		}
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("file read exception: %s", e.what());
			delete rec;
			continue;
		}

		if ( !_filter.empty() || !_reFilter.empty() ) {
			const auto *twf = findTimeWindowFilter(rec);
			// Not subscribed
			if ( !twf ) {
				delete rec;
				continue;
			}

			if ( twf->start.valid() ) {
				if ( rec->endTime() < twf->start ) {
					delete rec;
					continue;
				}
			}
			else if ( _startTime.valid() ) {
				if ( rec->endTime() < _startTime ) {
					delete rec;
					continue;
				}
			}

			if ( twf->end.valid() ) {
				if ( rec->startTime() >= twf->end ) {
					delete rec;
					continue;
				}
			}
			else if ( _endTime.valid() ) {
				if ( rec->startTime() >= _endTime ) {
					delete rec;
					continue;
				}
			}
		}
		else {
			if ( _startTime.valid() ) {
				if ( rec->endTime() < _startTime ) {
					delete rec;
					continue;
				}
			}

			if ( _endTime.valid() ) {
				if ( rec->startTime() >= _endTime ) {
					delete rec;
					continue;
				}
			}
		}

		return rec;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string File::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t File::tell() {
	return static_cast<size_t>(_fstream.tellg());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File &File::seek(size_t pos) {
	_fstream.seekg(static_cast<streampos>(pos));
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
File &File::seek(int off, SeekDir dir) {
	_fstream.seekg(static_cast<streamoff>(off),
	               static_cast<ios_base::seekdir>(dir));
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
