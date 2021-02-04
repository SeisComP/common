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


#include "anypacket.h"
#include "riff.h"
#include "utils.h"

#include <streambuf>
#include <iostream>
#include <cstring>


namespace Seiscomp {
namespace IO {
namespace CAPS {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AnyDataRecord::AnyHeader::put(std::streambuf &buf) const {
	Endianess::Writer put(buf);
	put(type, sizeof(type)-1);
	dataHeader.put(buf);

	put(endTime.year);
	put(endTime.yday);
	put(endTime.hour);
	put(endTime.minute);
	put(endTime.second);
	put(endTime.usec);

	return put.good;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnyDataRecord::AnyDataRecord() {
	strncpy(_header.type, "ANY", sizeof(_header.type));
	_header.dataHeader.samplingFrequencyDenominator = 0;
	_header.dataHeader.samplingFrequencyNumerator = 0;
	// Just a bunch of bytes
	_header.dataHeader.dataType = DT_INT8;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AnyDataRecord::setType(const char *type) {
	strncpy(_header.type, type, sizeof(_header.type));

	// Input clipped?
	if ( _header.type[sizeof(_header.type)-1] != '\0' ) {
		_header.type[sizeof(_header.type)-1] = '\0';
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *AnyDataRecord::type() const {
	return _header.type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnyDataRecord::setStartTime(const Time &time) {
	timeToTimestamp(_header.dataHeader.samplingTime, time);
	_startTime = time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnyDataRecord::setEndTime(const Time &time) {
	timeToTimestamp(_header.endTime, time);
	_endTime = time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnyDataRecord::setSamplingFrequency(uint16_t numerator, uint16_t denominator) {
	_header.dataHeader.samplingFrequencyNumerator = numerator;
	_header.dataHeader.samplingFrequencyDenominator = denominator;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *AnyDataRecord::formatName() const {
	return "ANY";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnyDataRecord::readMetaData(std::streambuf &buf, int size,
                                 Header &header,
                                 Time &startTime, Time &endTime) {
	// Read record type
	buf.sgetn(_header.type, 4);
	_header.type[sizeof(_header.type)-1] = '\0';

	size -= sizeof(_header.type)-1;

	if ( !header.get(buf) ) return;

	TimeStamp tmp;
	Endianess::Reader get(buf);

	get(tmp.year);
	get(tmp.yday);
	get(tmp.hour);
	get(tmp.minute);
	get(tmp.second);
	get(tmp.usec);

	startTime = timestampToTime(header.samplingTime);
	endTime = timestampToTime(tmp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataRecord::Header *AnyDataRecord::header() const {
	return &_header.dataHeader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time AnyDataRecord::startTime() const {
	return _startTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time AnyDataRecord::endTime() const {
	return _endTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t AnyDataRecord::dataSize(bool withHeader) const {
	if ( withHeader )
		return _data.size() + _header.dataSize();
	else
		return _data.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataRecord::ReadStatus AnyDataRecord::get(std::streambuf &buf, int size,
                                          const Time &start, const Time &end,
                                          int) {
	_data.clear();
	size -= _header.dataSize();
	if ( size < 0 ) return RS_Error;
	if ( !_header.get(buf) ) return RS_Error;

	_startTime = timestampToTime(_header.dataHeader.samplingTime);
	_endTime = timestampToTime(_header.endTime);

	if ( start.valid() ) {
		if ( _endTime < start || (_startTime < start && _endTime == start) )
			return RS_BeforeTimeWindow;
	}

	if ( end.valid() ) {
		if ( _startTime >= end ) return RS_AfterTimeWindow;
	}

	RIFF::VectorChunk<1,false> dataChunk(_data, 0, size);
	if ( !dataChunk.get(buf, size) ) return RS_Error;

	return RS_Complete;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnyDataRecord::setData(char *data, size_t size) {
	_data.resize(size);
	memcpy(_data.data(), data, size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}

