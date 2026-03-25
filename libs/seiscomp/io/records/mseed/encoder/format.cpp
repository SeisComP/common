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


#include <seiscomp/core/endianess.h>

#include <algorithm>
#include <climits>

#include "format.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::IO::MSEED {

namespace {


// SEED binary time (10 bytes)
using BTimeYear   = Attribute<uint16_t, 0>;
using BTimeYDay   = Attribute<uint16_t, 2>;
using BTimeHour   = Attribute<uint8_t,  4>;
using BTimeMinute = Attribute<uint8_t,  5>;
using BTimeSecond = Attribute<uint8_t,  6>;
using BTimeUnused = Attribute<uint8_t,  7>;
using BTimeFract  = Attribute<uint16_t, 8>;


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool getFraction(int &num, int &den, double value, double epsilon, int maxIterations) {
	int64_t overflow = INT_MAX;
	double r0 = value;
	int64_t a0 = (int64_t)r0;
	if ( std::abs(a0) > overflow ) {
		return false;
	}

	// check for (almost) integer arguments, which should not go
	// to iterations.
	if ( std::abs(a0 - value) < epsilon) {
		num = (int)a0;
		den = 1;
		return true;
	}

	int64_t p0 = 1;
	int64_t q0 = 0;
	int64_t p1 = a0;
	int64_t q1 = 1;

	int64_t p2 = 0;
	int64_t q2 = 1;

	int n = 0;
	bool stop = false;

	do {
		++n;
		double r1 = 1.0 / (r0 - a0);
		int64_t a1 = (int64_t)r1;
		p2 = (a1 * p1) + p0;
		q2 = (a1 * q1) + q0;
		if ( (std::abs(p2) > overflow) || (std::abs(q2) > overflow) ) {
			return false;
		}

		double convergent = (double)p2 / (double)q2;
		if ( (n < maxIterations)
		  && (std::abs(convergent - value) > epsilon)
		  && (q2 < INT_MAX) ) {
			p0 = p1;
			p1 = p2;
			q0 = q1;
			q1 = q2;
			a0 = a1;
			r0 = r1;
		}
		else {
			stop = true;
		}
	}
	while (!stop);

	if ( n >= maxIterations ) {
		return false;
	}

	if ( q2 < INT_MAX ) {
		num = (int) p2;
		den = (int) q2;
	}
	else {
		num = (int) p1;
		den = (int) q1;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Format::Format(const std::string &net, const std::string &sta,
               const std::string &loc, const std::string &cha)
: networkCode(net), stationCode(sta)
, locationCode(loc), channelCode(cha) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace V2 {


Format::Format(const std::string &net, const std::string &sta,
               const std::string &loc, const std::string &cha,
               unsigned short freqn, unsigned short freqd)
: Seiscomp::IO::MSEED::Format(net, sta, loc, cha) {
	if ( freqn == 0 || freqd == 0 ) {
		sampleRateFactor = 0;
		sampleRateMultiplier = 0;
	}
	else if ( !(freqn % freqd) ) {
		sampleRateFactor = freqn / freqd;
		sampleRateMultiplier = 1;
	}
	else if ( !(freqd % freqn) ) {
		sampleRateFactor = -freqd / freqn;
		sampleRateMultiplier = 1;
	}
	else {
		sampleRateFactor = -freqd;
		sampleRateMultiplier = freqn;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Format::populateHeader(const Core::Time &time, int timingQuality,
                            char *&ptr, int startFrames) {
	int n;

	*DataQuality::Get(ptr) = 'D';
	*Reserved::Get(ptr) = ' ';

	memcpy(Station::Get(ptr), stationCode.data(), std::min(stationCode.length(), size_t(5)));
	if ( (n = stationCode.length()) < 5 ) {
		memset(Station::Get(ptr) + n, 32, 5 - n);
	}
	memcpy(Location::Get(ptr), locationCode.data(), std::min(locationCode.length(), size_t(2)));
	if ( (n = locationCode.length()) < 2 ) {
		memset(Location::Get(ptr) + n, 32, 2 - n);
	}
	memcpy(Channel::Get(ptr), channelCode.data(), std::min(channelCode.length(), size_t(3)));
	if ( (n = channelCode.length()) < 3 ) {
		memset(Channel::Get(ptr) + n, 32, 3 - n);
	}
	memcpy(Network::Get(ptr), networkCode.data(), std::min(networkCode.length(), size_t(2)));
	if ( (n = networkCode.length()) < 2 ) {
		memset(Network::Get(ptr) + n, 32, 2 - n);
	}

	int year, doy, hour, minute, second, usec;
	time.get2(&year, &doy, &hour, &minute, &second, &usec);

	div_t d_tms = div(usec, 100);

	auto header = ptr;

	*Year::Get(header) = year;
	*YDay::Get(header) = doy + 1;
	*Hour::Get(header) = hour;
	*Minute::Get(header) = minute;
	*Second::Get(header) = second;
	*FSecond::Get(header) = d_tms.quot;
	*SamplingRateF::Get(header) = sampleRateFactor;
	*SamplingRateM::Get(header) = sampleRateMultiplier;
	*BlocketteCount::Get(header) = 1;
	*TimeCorrection::Get(header) = 0;
	*DataOffset::Get(header) = startFrames;
	*BlocketteOffset::Get(header) = HeaderLength;

	if ( bigEndian != Core::Endianess::Current::BigEndian ) {
		swapHeader(header);
	}

	ptr += HeaderLength;

	auto b1000 = ptr;
	*BlocketteType::Get(b1000) = 1000;
	*BlocketteNext::Get(b1000) = 0;
	*B1000Encoding::Get(b1000) = static_cast<uint8_t>(packType);
	*B1000ByteOrder::Get(b1000) = bigEndian ? 1 : 0;
	*B1000RecLength::Get(b1000) = recordSize;
	*B1000Reserved::Get(b1000) = 0;

	ptr += BHeadLength + B1000Length;

	if ( (timingQuality >= 0) || (d_tms.rem > 0) ) {
		*BlocketteNext::Get(b1000) = HeaderLength + BHeadLength + B1000Length;
		++*BlocketteCount::Get(header);

		auto b1001 = ptr;

		*BlocketteType::Get(b1001) = 1001; // Data Extension Blockette
		*BlocketteNext::Get(b1001) = 0; // Data Extension Blockette
		*B1001TimingQuality::Get(b1001) = timingQuality < 0 ? 0 : (timingQuality > 100 ? 100 : timingQuality);
		*B1001MicroSecond::Get(b1001) = d_tms.rem;
		*B1001Reserved::Get(b1001) = 0;
		*B1001FrameCount::Get(b1001) = 0;

		if ( bigEndian != Core::Endianess::Current::BigEndian ) {
			swapBlocketteHeader(b1001);
		}

		ptr += BHeadLength + B1000Length;
	}

	if ( bigEndian != Core::Endianess::Current::BigEndian ) {
		swapBlocketteHeader(b1000);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardFormat::StandardFormat(const std::string &net, const std::string &sta,
                               const std::string &loc, const std::string &cha,
                               unsigned short freqn, unsigned short freqd)
: Format(net, sta, loc, cha, freqn, freqd) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CharArrayPtr StandardFormat::getBuffer(const Core::Time &time, int timingQuality,
                                       void **dataptr, int *datalen) {
	int len = 1 << recordSize;
	CharArrayPtr record = new CharArray(len);
	record->fill(0);

	char *buf = record->typedData();

	const int startFrames = (HeaderLength + BHeadLength + B1000Length +
	                        // align to 64 bytes
	                        BHeadLength + B1001Length + 63) & 0xffffffc0;

	if ( dataptr ) {
		*dataptr = (void *)(buf + startFrames);
	}
	if ( datalen ) {
		*datalen = len - startFrames;
	}

	populateHeader(time, timingQuality, buf, startFrames);

	return record;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardFormat::updateBuffer(CharArray *rec, int samples, int frames) {
	auto ptr = rec->typedData();
	memset(SequenceNumber::Get(ptr), '0', 6);
	*DataQuality::Get(ptr) = 'D';
	*SampleCount::Get(ptr) = bigEndian ?
		Core::Endianess::Converter::ToBigEndian<uint16_t>(samples)
		:
		Core::Endianess::Converter::ToLittleEndian<uint16_t>(samples)
	;

	auto b1000 = ptr + HeaderLength;

	auto next = bigEndian ?
		Core::Endianess::Converter::FromBigEndian<uint16_t>(*BlocketteNext::Get(b1000))
		:
		Core::Endianess::Converter::FromLittleEndian<uint16_t>(*BlocketteNext::Get(b1000))
	;
	if ( next ) {
		auto b1001 = ptr + next;
		*B1001FrameCount::Get(b1001) = frames;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
