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


#ifndef SEISCOMP_IO_RECORDS_MSEED_HEADER_H
#define SEISCOMP_IO_RECORDS_MSEED_HEADER_H


#include <seiscomp/core/endianess.h>

#include <cstddef>
#include <cstdint>


namespace Seiscomp::IO::MSEED {


constexpr const size_t MaximumRecordLength = 10485760;
constexpr const size_t NSTModulus          = 1000000000;


// SEED data encoding types
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


enum ByteOrderFlags {
	SwapHeader         = 0x01,  //!< Header needs byte swapping
	SwapPayload        = 0x02,  //!< Payload needs byte swapping
	TargetLittleEndian = 0x04
};


template <typename T, size_t OFFSET>
struct Attribute {
	static constexpr const T * Get(const void *ptr) {
		return reinterpret_cast<const T*>(static_cast<const uint8_t*>(ptr) + OFFSET);
	}

	static constexpr T * Get(void *ptr) {
		return reinterpret_cast<T*>(static_cast<uint8_t*>(ptr) + OFFSET);
	}
};


template <typename T>
inline constexpr T &swap(T *ptr, bool swapflag) {
	if ( swapflag ) {
		Core::Endianess::swap(ptr);
	}
	return *ptr;
}

template <typename T>
inline constexpr T swap(T value, bool swapflag) {
	if ( swapflag ) {
		Core::Endianess::swap(&value);
	}
	return value;
}


namespace V2 {
/***************************************************************************
 * miniSEED 2.4 Fixed Section of Data Header
 * 48 bytes total
 *
 * FIELD               TYPE       OFFSET
 * sequence_number     char[6]       0
 * dataquality         char          6
 * reserved            char          7
 * station             char[5]       8
 * location            char[2]      13
 * channel             char[3]      15
 * network             char[2]      18
 * year                uint16_t     20
 * day                 uint16_t     22
 * hour                uint8_t      24
 * min                 uint8_t      25
 * sec                 uint8_t      26
 * unused              uint8_t      27
 * fract               uint16_t     28
 * numsamples          uint16_t     30
 * samprate_fact       int16_t      32
 * samprate_mult       int16_t      34
 * act_flags           uint8_t      36
 * io_flags            uint8_t      37
 * dq_flags            uint8_t      38
 * numblockettes       uint8_t      39
 * time_correct        int32_t      40
 * data_offset         uint16_t     44
 * blockette_offset    uint16_t     46
 ***************************************************************************/
constexpr size_t HeaderLength = 48;

using SequenceNumber  = Attribute<char, 0>;
using DataQuality     = Attribute<char, 6>;
using Reserved        = Attribute<char, 7>;
using Station         = Attribute<char, 8>;
using Location        = Attribute<char, 13>;
using Channel         = Attribute<char, 15>;
using Network         = Attribute<char, 18>;
using Year            = Attribute<uint16_t, 20>;
using YDay            = Attribute<uint16_t, 22>;
using Hour            = Attribute<uint8_t, 24>;
using Minute          = Attribute<uint8_t, 25>;
using Second          = Attribute<uint8_t, 26>;
using Unused          = Attribute<uint8_t, 27>;
using FSecond         = Attribute<uint16_t, 28>;
using SampleCount     = Attribute<uint16_t, 30>;
using SamplingRateF   = Attribute<int16_t, 32>;
using SamplingRateM   = Attribute<int16_t, 34>;
using ActivityFlags   = Attribute<uint8_t, 36>;
using IOFlags         = Attribute<uint8_t, 37>;
using DQFlags         = Attribute<uint8_t, 38>;
using BlocketteCount  = Attribute<uint8_t, 39>;
using TimeCorrection  = Attribute<int32_t, 40>;
using DataOffset      = Attribute<uint16_t, 44>;
using BlocketteOffset = Attribute<uint16_t, 46>;


inline constexpr void swapHeader(void *ptr) {
	Core::Endianess::swap(Year::Get(ptr));
	Core::Endianess::swap(YDay::Get(ptr));
	Core::Endianess::swap(FSecond::Get(ptr));
	Core::Endianess::swap(SampleCount::Get(ptr));
	Core::Endianess::swap(SamplingRateF::Get(ptr));
	Core::Endianess::swap(SamplingRateM::Get(ptr));
	Core::Endianess::swap(TimeCorrection::Get(ptr));
	Core::Endianess::swap(DataOffset::Get(ptr));
	Core::Endianess::swap(BlocketteOffset::Get(ptr));
}


constexpr const size_t BHeadLength = 4;
using BlocketteType = Attribute<uint16_t, 0>;
using BlocketteNext = Attribute<uint16_t, 2>;


inline constexpr void swapBlocketteHeader(void *ptr) {
	Core::Endianess::swap(BlocketteType::Get(ptr));
	Core::Endianess::swap(BlocketteNext::Get(ptr));
}


/***************************************************************************
 * miniSEED 2.4 Blockette 1000 - data only SEED (miniSEED)
 *
 * FIELD               TYPE       OFFSET
 * type                uint16_t      0
 * next offset         uint16_t      2
 * encoding            uint8_t       4
 * byteorder           uint8_t       5
 * reclen              uint8_t       6
 * reserved            uint8_t       7
 ***************************************************************************/
constexpr const size_t B1000Length = 4;
using B1000Encoding  = Attribute<uint8_t, 4>;
using B1000ByteOrder = Attribute<uint8_t, 5>;
using B1000RecLength = Attribute<uint8_t, 6>;
using B1000Reserved  = Attribute<uint8_t, 7>;


/***************************************************************************
 * miniSEED 2.4 Blockette 1001 - data extension
 *
 * FIELD               TYPE       OFFSET
 * type                uint16_t      0
 * next offset         uint16_t      2
 * timing quality      uint8_t       4
 * microsecond         int8_t        5
 * reserved            uint8_t       6
 * frame count         uint8_t       7
 ***************************************************************************/
constexpr const size_t B1001Length = 4;
using B1001TimingQuality = Attribute<uint8_t, 4>;
using B1001MicroSecond   = Attribute<int8_t, 5>;
using B1001Reserved      = Attribute<uint8_t, 6>;
using B1001FrameCount    = Attribute<uint8_t, 7>;


/***************************************************************************
 * miniSEED 2.4 Blockette 2000 - opaque data
 *
 * FIELD               TYPE       OFFSET
 * type                uint16_t      0
 * next offset         uint16_t      2
 * length              uint16_t      4
 * data offset         uint16_t      6
 * recnum              uint32_t      8
 * byteorder           uint8_t      12
 * flags               uint8_t      13
 * numheaders          uint8_t      14
 * payload             char[1]      15
 ***************************************************************************/
using B2000Length         = Attribute<uint16_t, 4>;
using B2000DataOffset     = Attribute<uint16_t, 6>;
using B2000RecorderNumber = Attribute<uint32_t, 8>;
using B2000ByteOrder      = Attribute<uint8_t, 12>;
using B2000Flags          = Attribute<uint8_t, 13>;
using B2000HeaderCount    = Attribute<uint8_t, 14>;
using B2000Payload        = Attribute<char, 15>;


}


namespace V3 {
/***************************************************************************
 * miniSEED 3.0 Fixed Section of Data Header
 * 40 bytes, plus length of identifier, plus length of extra headers
 *
 * #  FIELD                   TYPE       OFFSET
 * 1  record indicator        char[2]       0
 * 2  format version          uint8_t       2
 * 3  flags                   uint8_t       3
 * 4a nanosecond              uint32_t      4
 * 4b year                    uint16_t      8
 * 4c day                     uint16_t     10
 * 4d hour                    uint8_t      12
 * 4e min                     uint8_t      13
 * 4f sec                     uint8_t      14
 * 5  data encoding           uint8_t      15
 * 6  sample rate/period      float64      16
 * 7  number of samples       uint32_t     24
 * 8  CRC of record           uint32_t     28
 * 9  publication version     uint8_t      32
 * 10 length of identifer     uint8_t      33
 * 11 length of extra headers uint16_t     34
 * 12 length of data payload  uint32_t     36
 * 13 source identifier       char         40
 * 14 extra headers           char         40 + field 10
 * 15 data payload            encoded      40 + field 10 + field 11
 ***************************************************************************/
constexpr size_t HeaderLength = 40;

using Indicator          = Attribute<char,     0>;
using FormatVersion      = Attribute<uint8_t,  2>;
using Flags              = Attribute<uint8_t,  3>;
using Nanoseconds        = Attribute<uint32_t, 4>;
using Year               = Attribute<uint16_t, 8>;
using YDay               = Attribute<uint16_t, 10>;
using Hour               = Attribute<uint8_t, 12>;
using Minute             = Attribute<uint8_t, 13>;
using Second             = Attribute<uint8_t, 14>;
using Encoding           = Attribute<uint8_t, 15>;
using SamplingRate       = Attribute<double, 16>;
using SampleCount        = Attribute<uint32_t, 24>;
using CRC                = Attribute<uint32_t, 28>;
using PublicationVersion = Attribute<uint8_t, 32>;
using SIDLength          = Attribute<uint8_t, 33>;
using ExtraLength        = Attribute<uint16_t, 34>;
using DataLength         = Attribute<uint32_t, 36>;
using SID                = Attribute<char, 40>;


}
}


#endif
