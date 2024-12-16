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


#ifndef SEISCOMP_IO_ENCODER_MSEED_FORMAT_H
#define SEISCOMP_IO_ENCODER_MSEED_FORMAT_H


#include <seiscomp/core/typedarray.h>

#include <cinttypes>
#include <string>


namespace Seiscomp {
namespace IO {
namespace MSEED {


/* SEED data encoding types */
enum class EncodingType {
	ASCII       = 0,
	INT16       = 1,
	INT32       = 3,
	FLOAT32     = 4,
	FLOAT64     = 5,
	STEIM1      = 10,
	STEIM2      = 11,
	GEOSCOPE24  = 12,
	GEOSCOPE163 = 13,
	GEOSCOPE164 = 14,
	CDSN        = 16,
	SRO         = 30,
	DWWSSN      = 32
};


template <typename T>
EncodingType getEncoding();

template <>
inline EncodingType getEncoding<char*>() {
	return EncodingType::ASCII;
}

template <>
inline EncodingType getEncoding<int16_t>() {
	return EncodingType::INT16;
}

template <>
inline EncodingType getEncoding<int32_t>() {
	return EncodingType::INT32;
}

template <>
inline EncodingType getEncoding<float>() {
	return EncodingType::FLOAT32;
}

template <>
inline EncodingType getEncoding<double>() {
	return EncodingType::FLOAT64;
}


class Format {
	public:
		Format(const std::string &net, const std::string &sta,
		       const std::string &loc, const std::string &cha,
		       unsigned short freqn, unsigned short freqd);
		virtual ~Format() = default;

	public:
		virtual CharArrayPtr getBuffer(const Core::Time &time, int timingQuality,
		                               void **dataptr = 0, int *datalen = 0) = 0;
		virtual void updateBuffer(CharArray *rec, int samples, int frames) = 0;

	protected:
		//! Returns the last blockette header and increases dataptr to the
		//! position after the last blockette.
		void populateHeader(const Core::Time &time, int timingQuality,
		                    char *&dataptr, int startFrames);

	public:
		std::string  networkCode;
		std::string  stationCode;
		std::string  locationCode;
		std::string  channelCode;
		int          sampleRateFactor;
		int          sampleRateMultiplier;
		int          recordSize{9};
		EncodingType packType;
		int          timingQuality;
};


class StandardFormat : public Format {
	public:
		StandardFormat(const std::string &net, const std::string &sta,
		               const std::string &loc, const std::string &cha,
		               unsigned short freqn, unsigned short freqd);

	public:
		CharArrayPtr getBuffer(const Core::Time &it, int timingQuality,
		                       void **dataptr = 0, int *datalen = 0) override;
		void updateBuffer(CharArray *rec, int samples, int frames) override;
};


}
}
}


#endif
