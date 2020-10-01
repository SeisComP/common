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


#ifndef SEISCOMP_CORE_GENERICRECORD_H
#define SEISCOMP_CORE_GENERICRECORD_H

#include <string>
#include <iostream>
#include <seiscomp/core/record.h>
#include <seiscomp/core/array.h>
#include <seiscomp/core/bitset.h>


namespace Seiscomp {


DEFINE_SMARTPOINTER(GenericRecord);

class SC_SYSTEM_CORE_API GenericRecord : public Record {
	DECLARE_SC_CLASS(GenericRecord);
	DECLARE_SERIALIZATION;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! Default Constructor
		GenericRecord(Array::DataType dt = Array::DOUBLE, Hint h = DATA_ONLY);
	
		//! Initializing Constructor
		GenericRecord(std::string net, std::string sta,
		              std::string loc, std::string cha,
		              Core::Time stime, double fsamp, int tqual = -1,
		              Array::DataType dt = Array::DOUBLE,
		              Hint h = DATA_ONLY);
	
		//! Copy Constructor
		GenericRecord(const GenericRecord& rec);
	
		//! Another Constructor
		GenericRecord(const Record& rec);

		//! Destructor
		virtual ~GenericRecord();
	

	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		//! Assignment operator
		GenericRecord& operator=(const GenericRecord& rec);
	

	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the sample frequency
		void setSamplingFrequency(double freq);

		//! Returns the data samples if the data is available; otherwise 0
		Array* data();

		//! Returns the data samples if the data is available; otherwise 0
		const Array* data() const;

		//! Same as data()
		const Array* raw() const;

		//! Returns the clipmask. The size of the clipmask matches the size
		//! of the data array and each element (bit) is set to one if the
		//! sample at the same index is clipped. The returned pointer is
		//! managed by this instance and must not be deleted. But it is safe
		//! to store it in a smart pointer.
		const BitSet *clipMask() const;

		//! Sets the data sample array. The ownership goes over to the record.
		//! Note that this call will remove any clip mask set with previous
		//! calls.
		void setData(Array* data);

		//! Sets the data sample array.
		//! Note that this call will remove any clip mask set with previous
		//! calls.
		void setData(int size, const void *data, Array::DataType datatype);

		/**
		 * @brief Sets the clip mask.
		 * @param The bitset pointer which will be managed by this instance.
		 */
		void setClipMask(BitSet *clipMask);

		//! Updates internal parameters caused by data updates
		void dataUpdated();

		//! This method does nothing.
		void saveSpace() const;

		//! Returns a deep copy of the calling object.
		Record* copy() const;

		void read(std::istream &in);
		void write(std::ostream &out);
	

	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		ArrayPtr  _data;
		BitSetPtr _clipMask;
};
 
}

#endif
