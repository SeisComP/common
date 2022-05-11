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


typedef struct MSRecord_s MSRecord;

namespace Seiscomp {
namespace IO {


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
		//! Initializing Constructor
		MSeedRecord(Array::DataType dt = Array::DOUBLE, Hint h = SAVE_RAW);

		//! Initializing Constructor
		MSeedRecord(MSRecord *msrec, Array::DataType dt = Array::DOUBLE, Hint h = SAVE_RAW);

		//! Copy Constructor
		MSeedRecord(const MSeedRecord &ms);

		//! Copy-from-Record  Constructor
		MSeedRecord(const Record &rec, int reclen=512);

		//! Destructor
		~MSeedRecord() override;


	public:
		//! Assignment Operator
		MSeedRecord& operator=(const MSeedRecord &ms);

		void setNetworkCode(std::string net) override;

		//! Sets the station code
		void setStationCode(std::string sta) override;

		//! Sets the location code
		void setLocationCode(std::string loc) override;

		//! Sets the channel code
		void setChannelCode(std::string cha) override;

		//! Sets the start time
		void setStartTime(const Core::Time& time) override;

		//! Returns the sequence number
		int sequenceNumber() const;

		//! Sets the sequence number
		void setSequenceNumber(int seqno);

		//! Returns the data quality
		char dataQuality() const;

		//! Sets the data quality
		void setDataQuality(char qual);

		//! Returns the sample rate factor
		int sampleRateFactor() const;

		//! Sets the sample rate factor
		void setSampleRateFactor(int srfact);

		//! Returns the sample rate multiplier
		int sampleRateMultiplier() const;

		//! Sets the sample rate multiplier
		void setSampleRateMultiplier(int srmult);

		//! Returns the byteorder
		int8_t byteOrder() const;

		//! Returns the encoding code
		int8_t encoding() const;

		//! Returns the sample rate numerator
		int sampleRateNumerator() const;

		//! Returns the sample rate denominator
		int sampleRateDenominator() const;

		//! Returns the number of data frames
		int frameNumber() const;

		//! Returns the end time of data samples
		const Seiscomp::Core::Time& endTime() const;

		//! Returns the length of a Mini SEED record
		int recordLength() const;

		//! Returns the leap seconds
		int leapSeconds() const;

		//! Returns a nonmutable pointer to the data samples if the data is available; otherwise 0
		//! (the data type is independent from the original one and was given by the DataType flag in the constructor)
		const Array* data() const override;

		const Array* raw() const override;

		//! Frees the memory occupied by the decoded data samples.
		//! ! Use it with the hint SAVE_RAW only otherwise the data samples cannot be redecoded!
		void saveSpace() const override;

		//! Returns a deep copy of the calling object.
		Record* copy() const;

		//! Sets flag specifying the encoding type of the write routine.
		//! true(default) -> use the encoding of the original record; false -> use the type of the data
		void useEncoding(bool flag);

		//! Sets the record length used for the output
		void setOutputRecordLength(int reclen);

		//! Extract the packed MSeedRecord attributes from the given stream
		void read(std::istream &in);

		//! Encode the record into the given stream
		void write(std::ostream& out);

	private:
		void _setDataAttributes(int reclen, char *data) const;

	private:
		CharArray            _raw;
		mutable ArrayPtr     _data;
		int                  _seqno;
		char                 _rectype;
		int                  _srfact;
		int                  _srmult;
		int8_t               _byteorder;
		int8_t               _encoding;
		int                  _srnum;
		int                  _srdenom;
		int                  _reclen;
		int                  _nframes;
		int                  _leap;
		Seiscomp::Core::Time _etime;
		bool                 _encodingFlag;
};

} // namespace IO
} // namespace Seiscomp

#endif
