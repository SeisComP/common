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


#ifndef SEISCOMP_IO_RECORDS_MSEED_ENCODER_FORMAT_H
#define SEISCOMP_IO_RECORDS_MSEED_ENCODER_FORMAT_H


#include <seiscomp/core/typedarray.h>
#include <seiscomp/io/records/mseed/format.h>

#include <string>


namespace Seiscomp::IO::MSEED {


bool getFraction(int &num, int &den, double value, double epsilon = 1E-5, int maxIterations = 100);


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
		       const std::string &loc, const std::string &cha);
		virtual ~Format() = default;

	public:
		virtual CharArrayPtr getBuffer(const Core::Time &time, int timingQuality,
		                               void **dataptr = 0, int *datalen = 0) = 0;
		virtual void updateBuffer(CharArray *rec, int samples, int frames) = 0;

	public:
		std::string  networkCode;
		std::string  stationCode;
		std::string  locationCode;
		std::string  channelCode;
		EncodingType packType;
		bool         bigEndian{true};
		int          timingQuality;
};


namespace V2 {


class Format : public Seiscomp::IO::MSEED::Format {
	public:
		Format(const std::string &net, const std::string &sta,
		       const std::string &loc, const std::string &cha,
		       unsigned short freqn, unsigned short freqd);

	protected:
		//! Returns the last blockette header and increases dataptr to the
		//! position after the last blockette.
		void populateHeader(const Core::Time &time, int timingQuality,
		                    char *&dataptr, int startFrames);

	public:
		int  sampleRateFactor;
		int  sampleRateMultiplier;
		int  recordSize{9};
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


#endif
