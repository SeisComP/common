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


#ifndef SEISCOMP_IO_RECORDS_MSEEDRECORD_H
#define SEISCOMP_IO_RECORDS_MSEEDRECORD_H


#include <seiscomp/core/record.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core.h>

#include <string>
#include <cstdint>


namespace Seiscomp::IO {


DEFINE_SMARTPOINTER(MSeedRecord);


class SC_SYSTEM_CORE_API LibmseedException : public Core::StreamException {
	public:
		LibmseedException() : Core::StreamException("libmseed error") {}
		LibmseedException(std::string what) : Core::StreamException(what) {}
};


/**
 * Uses seiscomp error logging as component MSEEDRECORD.
 **/
class SC_SYSTEM_CORE_API MSeedRecord: public Record {
	DECLARE_SC_CLASS(MSeedRecord)

	public:
		enum Format {
			V2,
			V3
		};


	public:
		//! Initializing Constructor
		MSeedRecord(Array::DataType dt = Array::DOUBLE, Hint h = SAVE_RAW);

		//! Copy Constructor
		MSeedRecord(const MSeedRecord &ms);

		MSeedRecord(const Record &rec, int reclen = 512);


	public:
		/**
		 * @brief Tries to detect the record length from a given memory chunk.
		 * @param data The memory address.
		 * @param len The length in bytes of the memory block.
		 * @param format The optional return format.
		 * @return The record length in bytes or -1.
		 */
		static int64_t Detect(const void *data, size_t len, Format *format = nullptr);

		//! Assignment Operator
		MSeedRecord &operator=(const MSeedRecord &ms);

		void setNetworkCode(std::string net) override;

		//! Sets the station code
		void setStationCode(std::string sta) override;

		//! Sets the location code
		void setLocationCode(std::string loc) override;

		//! Sets the channel code
		void setChannelCode(std::string cha) override;

		//! Sets the start time
		void setStartTime(const Core::Time& time) override;

		//! Returns the data quality
		char dataQuality() const { return _quality; }

		//! Sets the data quality
		void setDataQuality(char qual);

		//! Returns the byteorder
		int8_t byteOrder() const { return _byteOrder; }

		//! Returns the encoding code
		int8_t encoding() const { return _encoding; }

		//! Returns the end time of data samples
		const Seiscomp::Core::Time& endTime() const { return _endTime; }

		//! Returns the length of a Mini SEED record
		int recordLength() const { return _recordLength; }

		//! Returns a nonmutable pointer to the data samples if the data is available; otherwise 0
		//! (the data type is independent from the original one and was given by the DataType flag in the constructor)
		const Array* data() const override;

		const Array* raw() const override;

		//! Frees the memory occupied by the decoded data samples.
		//! ! Use it with the hint SAVE_RAW only otherwise the data samples cannot be redecoded!
		void saveSpace() const override;

		//! Returns a deep copy of the calling object.
		Record *copy() const override;

		//! Sets flag specifying the encoding type of the write routine.
		//! true(default) -> use the encoding of the original record; false -> use the type of the data
		void useEncoding(bool flag);

		//! Sets the encoding flag to little-endian
		void setLittleEndian(bool flag);

		//! Sets the record length used for the output
		void setOutputRecordLength(int reclen);

		//! Extract the packed MSeedRecord attributes from the given stream
		void read(std::istream &in) override;

		//! Encode the record into the given stream
		void write(std::ostream &out) override;


	private:
		void updateAuthentication(const char *rec, size_t reclen, size_t offset);
		void unpackData(const char *rec, size_t reclen) const;


	private:
		Format               _format;
		CharArray            _raw;
		mutable ArrayPtr     _data;
		char                 _quality{'D'};
		int8_t               _byteOrder{0};
		int16_t              _encoding{-1};
		uint32_t             _recordLength{0};
		Seiscomp::Core::Time _endTime;
		bool                 _encodingFlag{false};
};


}


#endif
