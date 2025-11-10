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

#include "format.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace IO {
namespace MSEED {


namespace {


#if defined (__GNUC__)
	#define ATTR_PACKED __attribute__ ((packed))
#else
	#define ATTR_PACKED
#endif

/* SEED binary time (10 bytes) */
struct SEEDBTime {
	uint16_t  year;
	uint16_t  day;
	uint8_t   hour;
	uint8_t   min;
	uint8_t   sec;
	uint8_t   unused;
	uint16_t  fract;
} ATTR_PACKED;


/* Fixed section data of header (48 bytes) */
struct SEEDFixedHeader {
	char      sequenceNumber[6];
	char      dhqIndicator;
	char      reserved;
	char      station[5];
	char      location[2];
	char      channel[3];
	char      network[2];
	SEEDBTime startTime;
	uint16_t  numberOfSamples;
	int16_t   sampleRateFactor;
	int16_t   sampleRateMultiplier;
	uint8_t   activityFlags;
	uint8_t   ioAndClockFlags;
	uint8_t   dataQualityFlags;
	uint8_t   numberOfBlockettes;
	int32_t   timeCorrection;
	uint16_t  dataOffset;
	uint16_t  blocketteOffset;
} ATTR_PACKED;


/* Generic struct for head of blockettes */
struct SEEDBlocketteHeader {
	uint16_t  type;
	uint16_t  nextOffset;
} ATTR_PACKED;


/* 1000 Blockette (8 bytes) */
struct SEEDBlockette1000 : SEEDBlocketteHeader {
	uint8_t   encoding;
	uint8_t   wordSwap;
	uint8_t   recordLength;
	uint8_t   reserved;
} ATTR_PACKED;

/* 1001 Blockette (8 bytes) */
struct SEEDBlockette1001 : SEEDBlocketteHeader {
	int8_t    timingQuality;
	int8_t    usec;
	uint8_t   reserved;
	int8_t    numberOfFrames;
} ATTR_PACKED;


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Format::Format(const std::string &net, const std::string &sta,
               const std::string &loc, const std::string &cha,
               unsigned short freqn, unsigned short freqd)
: networkCode(net), stationCode(sta)
, locationCode(loc), channelCode(cha)
{
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
                            char *&dataptr, int startFrames) {
	auto fsdh = reinterpret_cast<SEEDFixedHeader*>(dataptr);
	int n;

	fsdh->dhqIndicator = 'D';
	fsdh->reserved = ' ';

	memcpy(fsdh->station, stationCode.data(), std::min(stationCode.length(), size_t(5)));
	if ( (n = stationCode.length()) < 5 ) {
		memset(fsdh->station + n, 32, 5 - n);
	}
	memcpy(fsdh->location, locationCode.data(), std::min(locationCode.length(), size_t(2)));
	if ( (n = locationCode.length()) < 2 ) {
		memset(fsdh->location + n, 32, 2 - n);
	}
	memcpy(fsdh->channel, channelCode.data(), std::min(channelCode.length(), size_t(3)));
	if ( (n = channelCode.length()) < 3 ) {
		memset(fsdh->channel + n, 32, 3 - n);
	}
	memcpy(fsdh->network, networkCode.data(), std::min(networkCode.length(), size_t(2)));
	if ( (n = networkCode.length()) < 2 ) {
		memset(fsdh->network + n, 32, 2 - n);
	}

#ifdef MSEED_ROUND_TENTH_MILLISEC
	EXT_TIME et = int_to_ext(add_dtime(it, 50));
#else
	int year, doy, hour, minute, second, usec;
	time.get2(&year, &doy, &hour, &minute, &second, &usec);
#endif

	div_t d_tms = div(usec, 100);

	fsdh->startTime.year = Core::Endianess::Converter::ToBigEndian<uint16_t>(year);
	fsdh->startTime.day = Core::Endianess::Converter::ToBigEndian<uint16_t>(doy + 1);
	fsdh->startTime.hour = hour;
	fsdh->startTime.min = minute;
	fsdh->startTime.sec = second;
	fsdh->startTime.fract = Core::Endianess::Converter::ToBigEndian<uint16_t>(d_tms.quot);
	fsdh->sampleRateFactor = (int16_t)Core::Endianess::Converter::ToBigEndian<uint16_t>(sampleRateFactor);
	fsdh->sampleRateMultiplier = (int16_t)Core::Endianess::Converter::ToBigEndian<uint16_t>(sampleRateMultiplier);
	fsdh->numberOfBlockettes = 1;
	fsdh->timeCorrection = 0;
	fsdh->dataOffset = Core::Endianess::Converter::ToBigEndian<uint16_t>(startFrames);
	fsdh->blocketteOffset = Core::Endianess::Converter::ToBigEndian<uint16_t>(sizeof(SEEDFixedHeader));

	dataptr += sizeof(SEEDFixedHeader);

	auto b1000 = reinterpret_cast<SEEDBlockette1000*>(reinterpret_cast<char*>(fsdh) + sizeof(SEEDFixedHeader));
	b1000->type = Core::Endianess::Converter::ToBigEndian<uint16_t>(1000);          // Data Only SEED Blockette

	b1000->encoding = static_cast<uint8_t>(packType);
	b1000->wordSwap = 1;
	b1000->recordLength = recordSize;

	dataptr += sizeof(SEEDBlockette1000);

	if ( timingQuality >= 0 ) {
		b1000->nextOffset = Core::Endianess::Converter::ToBigEndian<uint16_t>(sizeof(SEEDFixedHeader) + sizeof(SEEDBlockette1000));
		++fsdh->numberOfBlockettes;

		dataptr += sizeof(SEEDBlockette1001);

		auto b1001 = reinterpret_cast<SEEDBlockette1001*>(reinterpret_cast<char*>(fsdh) + sizeof(SEEDFixedHeader) + sizeof(SEEDBlockette1000));

		b1001->type = Core::Endianess::Converter::ToBigEndian<uint16_t>(1001);      // Data Extension Blockette
		b1001->timingQuality = timingQuality;

		b1001->usec = 0;
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

	const int startFrames = (sizeof(SEEDFixedHeader) + sizeof(SEEDBlockette1000) +
	                        // align to 64 bytes
	                        sizeof(SEEDBlockette1001) + 63) & 0xffffffc0;

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
	auto fsdh = reinterpret_cast<SEEDFixedHeader*>(rec->typedData());
	char temp[7];

	sprintf(temp, "%06d", (int)0);
	memcpy(fsdh->sequenceNumber,temp,6);
	fsdh->dhqIndicator = 'D';
	fsdh->numberOfSamples = Core::Endianess::Converter::ToBigEndian<uint16_t>(samples);

	auto b1000 = reinterpret_cast<SEEDBlockette1000*>(reinterpret_cast<char *>(fsdh) + sizeof(SEEDFixedHeader));

	if ( Core::Endianess::Converter::ToBigEndian<uint16_t>(b1000->nextOffset) ) {
		auto b1001 = reinterpret_cast<SEEDBlockette1001*>(
			reinterpret_cast<char *>(fsdh) + sizeof(SEEDFixedHeader) + sizeof(SEEDBlockette1000)
		);

		b1001->numberOfFrames = frames;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
