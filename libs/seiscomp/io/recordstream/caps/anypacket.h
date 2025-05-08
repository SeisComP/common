/***************************************************************************
 * Copyright (C) 2012 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#ifndef SEISCOMP_IO_CAPS_ANYPACKET_H
#define SEISCOMP_IO_CAPS_ANYPACKET_H


#include "packet.h"
#include "endianess.h"

#include <vector>


namespace Seiscomp {
namespace IO {
namespace CAPS {


class AnyDataRecord : public DataRecord {
	public:
		typedef std::vector<char> Buffer;

		struct AnyHeader {
			char      type[5];
			Header    dataHeader;
			TimeStamp endTime;

			bool get(std::streambuf &buf) {
				Endianess::Reader get(buf);
				get(type, sizeof(type)-1);
				type[sizeof(type)-1] = '\0';
				dataHeader.get(buf);

				get(endTime.year);
				get(endTime.yday);
				get(endTime.hour);
				get(endTime.minute);
				get(endTime.second);
				get(endTime.usec);

				return get.good;
			}

			bool put(std::streambuf &buf) const;

			// 4 additional bytes (type) with respect to the original
			// data header
			int dataSize() const {
				return sizeof(type)-1 +
				       dataHeader.dataSize() +
				       sizeof(endTime.year) +
				       sizeof(endTime.yday) +
				       sizeof(endTime.hour) +
				       sizeof(endTime.minute) +
				       sizeof(endTime.second) +
				       sizeof(endTime.usec);
			}
		};

		AnyDataRecord();

		//! Sets the format of the any record. The format can be
		//! anything that fits into 4 characters. If more than
		//! 4 characters are given, false is returned.
		bool setType(const char *type);
		const char *type() const;

		const char *formatName() const override;

		void readMetaData(std::streambuf &buf, int size,
		                  Header &header,
		                  Time &startTime,
		                  Time &endTime) override;

		const Header *header() const override;
		Time startTime() const override;
		Time endTime() const override;

		size_t dataSize(bool withHeader) const override;

		ReadStatus get(std::streambuf &buf, int size,
		               const Time &start = Time(),
		               const Time &end = Time(),
		               int maxSize = -1) override;

		/**
		 * @brief Returns the packet type
		 * @return The packet type
		 */
		PacketType packetType() const override { return ANYPacket; }

		/**
		 * @brief Sets the start time of the record
		 * @param The start time
		 */
		void setStartTime(const Time &time);

		/**
		 * @brief Sets the end time of the record
		 * @param The end time
		 */
		void setEndTime(const Time &time);

		/**
		 * @brief Sets the sampling frequency of the record
		 * @param numerator The numerator
		 * @param denominator The denomintor
		 */
		void setSamplingFrequency(uint16_t numerator, uint16_t denominator);

		/**
		 * @brief Returns the data vector to be filled by the caller
		 * @return The pointer to the internal buffer
		 */
		Buffer *data() override { return &_data; }

		/**
		 * @brief Initializes the internal data vector from the given buffer
		 * @param The buffer to read the data from
		 * @param The buffer size
		 */
		virtual void setData(char *data, size_t size);

	protected:
		AnyHeader      _header;
		Buffer         _data;

		Time          _startTime;
		Time          _endTime;
};


}
}
}


#endif
