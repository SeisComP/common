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


#ifndef SEISCOMP_IO_CAPS_RAWPACKET_H
#define SEISCOMP_IO_CAPS_RAWPACKET_H


#include "packet.h"
#include <vector>


namespace Seiscomp {
namespace IO {
namespace CAPS {


struct RawResponseHeader {
	int64_t timeSeconds;
	int32_t timeMicroSeconds;

	bool get(std::streambuf &buf) {
		Endianess::Reader get(buf);

		get(timeSeconds);
		get(timeMicroSeconds);

		return get.good;
	}

	int dataSize() const {
		return sizeof(timeSeconds) + sizeof(timeMicroSeconds);
	}
};


class RawDataRecord : public DataRecord {
	public:
		RawDataRecord();

		const char *formatName() const override;

		/**
		 * @brief Reads metadata from data record header
		 * @param The streambuf object
		 * @param The size of the data record in bytes
		 * @param The data record header object
		 * @param The startTime
		 * @param The endTime
		 */
		void readMetaData(std::streambuf &buf, int size,
		                  Header &header,
		                  Time &startTime, Time &endTime) override;

		void setHeader(const Header &header);

		/**
		 * @brief Returns the meta information of the data if supported
		 * @return The data record header
		 */
		const Header *header() const override;

		/**
		 * @brief Returns the start time of the record
		 * @return The start time
		 */
		Time startTime() const override;

		/**
		 * @brief Returns the end time of the record
		 * @return The end time
		 */
		Time endTime() const override;

		/**
		 * @brief Returns the data size in bytes if the current state would
		 * be written to a stream. Trimming has also to be taken into
		 * account while calculating the size.
		 * @param withHeader Take header into account
		 * @return Returns the data size in bytes
		 */
		size_t dataSize(bool withHeader) const override;

		/**
		 * @brief Reads the packet data including header from a streambuf
		 * and trims the data if possible to start and end.
		 * If maxBytes is greater than 0 then the record should
		 * not use more than this size of memory (in bytes).
		 *
		 * @param The streambuf object
		 * @param The buffer size
		 * @param The requested start time
		 * @param The requested end time
		 * @param The Max bytes to use
		 * @return If not the complete record has been read, RS_Partial
		 * must be returned, RS_Complete otherwise.
		 */
		ReadStatus get(std::streambuf &buf, int size,
		               const Time &start = Time(),
		               const Time &end = Time(),
		               int maxBytes = -1) override;

		/**
		 * @brief Reads the packet data without header from a streambuf
		 * and trims the data if possible to start and end.
		 * If maxBytes is greater than 0 then the record should
		 * not use more than this size of memory (in bytes).
		 *
		 * @param The streambuf object
		 * @param The buffer size
		 * @param The requested start time
		 * @param The requested end time
		 * @param The Max bytes to use
		 * @return If not the complete record has been read, RS_Partial
		 * must be returned, RS_Complete otherwise.
		 */
		ReadStatus getData(std::streambuf &buf, int size,
		                   const Time &start = Time(),
		                   const Time &end = Time(),
		                   int maxBytes = -1);

		/**
		 * @brief Returns the packet type
		 * @return The packet type
		 */
		PacketType packetType() const override { return RawDataPacket; }

		/**
		 * @brief Sets the start time of the record
		 * @param The start time
		 */
		void setStartTime(const Time &time);

		/**
		 * @brief Sets the sampling frequency of the record
		 * @param numerator The numerator
		 * @param denominator The denomintor
		 */
		void setSamplingFrequency(int numerator, int denominator);

		/**
		 * @brief Sets the data type of the record
		 * @param The datatype to use
		 */
		void setDataType(DataType dt);

		/**
		 * @brief Initializes the internal data vector from the given buffer
		 * @param The buffer to read the data from
		 * @param The buffer size
		 */
		void setBuffer(const void *data, size_t size);


	protected:
		Header  _header;

		mutable Header _currentHeader;
		mutable size_t _dataOfs;
		mutable size_t _dataSize;

		mutable Time   _startTime;
		mutable Time   _endTime;

		mutable bool   _dirty;
};


class FixedRawDataRecord : public RawDataRecord {
	public:
		virtual const char *formatName() const override;

		virtual ReadStatus get(std::streambuf &buf, int size,
		                       const Time &/*start*/, const Time &/*end*/,
		                       int maxBytes) override {
			return RawDataRecord::get(buf, size, Time(), Time(), maxBytes);
		}

		PacketType packetType() const override { return FixedRawDataPacket; }
};


}
}
}


#endif
