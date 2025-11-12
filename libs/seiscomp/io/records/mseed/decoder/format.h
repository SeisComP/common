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


#ifndef SEISCOMP_IO_RECORDS_MSEED_DECODER_HEADER_H
#define SEISCOMP_IO_RECORDS_MSEED_DECODER_HEADER_H


#include <seiscomp/io/records/mseed/format.h>
#include <seiscomp/core/strings.h>

#include <string_view>


namespace Seiscomp::IO::MSEED {


inline bool isValidYearDay(int year, int yday) {
	return (year >= 1900) && (year <= 2100) && (yday >= 1) && (yday <= 366);
}


bool sid2nslc(std::string_view sid, std::string &net, std::string &sta,
              std::string &loc, std::string &cha);


namespace V2 {


inline constexpr bool isValidHeader(const void *ptr) {
	return ((SequenceNumber::Get(ptr)[0] >= '0' && SequenceNumber::Get(ptr)[0] <= '9') || (SequenceNumber::Get(ptr)[0] == ' ') || !SequenceNumber::Get(ptr)[0]) &&
	       ((SequenceNumber::Get(ptr)[1] >= '0' && SequenceNumber::Get(ptr)[1] <= '9') || (SequenceNumber::Get(ptr)[1] == ' ') || !SequenceNumber::Get(ptr)[1]) &&
	       ((SequenceNumber::Get(ptr)[2] >= '0' && SequenceNumber::Get(ptr)[2] <= '9') || (SequenceNumber::Get(ptr)[2] == ' ') || !SequenceNumber::Get(ptr)[2]) &&
	       ((SequenceNumber::Get(ptr)[3] >= '0' && SequenceNumber::Get(ptr)[3] <= '9') || (SequenceNumber::Get(ptr)[3] == ' ') || !SequenceNumber::Get(ptr)[3]) &&
	       ((SequenceNumber::Get(ptr)[4] >= '0' && SequenceNumber::Get(ptr)[4] <= '9') || (SequenceNumber::Get(ptr)[4] == ' ') || !SequenceNumber::Get(ptr)[4]) &&
	       ((SequenceNumber::Get(ptr)[5] >= '0' && SequenceNumber::Get(ptr)[5] <= '9') || (SequenceNumber::Get(ptr)[5] == ' ') || !SequenceNumber::Get(ptr)[5]) &&
	       ((*DataQuality::Get(ptr) == 'D') || (*DataQuality::Get(ptr) == 'R') || (*DataQuality::Get(ptr) == 'Q') || (*DataQuality::Get(ptr) == 'M')) &&
	       ((*Reserved::Get(ptr) == ' ') || (*Reserved::Get(ptr) == '\0')) &&
	       (*Hour::Get(ptr) >= 0) && (*Hour::Get(ptr) <= 23) &&
	       (*Minute::Get(ptr) >= 0) && (*Minute::Get(ptr) <= 59) &&
	       (*Second::Get(ptr) >= 0) && (*Second::Get(ptr) <= 60);
}


uint16_t blocketteLength(uint16_t type, const void *ptrBlockette, bool swapflag);


}


namespace V3 {


inline constexpr bool isValidHeader(const void *ptr) {
	return (Indicator::Get(ptr)[0] == 'M') && (Indicator::Get(ptr)[1] == 'S') &&
	       (*FormatVersion::Get(ptr) == 3) &&
	       (*Hour::Get(ptr) >= 0) && (*Hour::Get(ptr) <= 23) &&
	       (*Minute::Get(ptr) >= 0) && (*Minute::Get(ptr) <= 59) &&
	       (*Second::Get(ptr) >= 0) && (*Second::Get(ptr) <= 60);
}


}


// Sample decoding functions.
int64_t decodeSteim1(int32_t *input, size_t inputLength, size_t sampleCount,
                     int32_t *output, size_t outputLength, bool swapflag);
int64_t decodeSteim2(int32_t *input, size_t inputLength, size_t sampleCount,
                     int32_t *output, size_t outputLength, bool swapflag);
int64_t decodeGEOSCOPE(char *input, size_t sampleCount, float *output, size_t outputLength,
                       EncodingType encoding, bool swapflag);
int64_t decodeCDSN(int16_t *input, size_t sampleCount, int32_t *output, size_t outputLength,
                   bool swapflag);
int64_t decodeSRO(int16_t *input, size_t sampleCount, int32_t *output, size_t outputLength,
                  bool swapflag);
int64_t decodeDWWSSN(int16_t *input, size_t sampleCount, int32_t *output, size_t outputLength,
                     bool swapflag);


}


#endif
