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


#ifndef SEISCOMP_IO_RECORDFILTER_IIRFILTER_H
#define SEISCOMP_IO_RECORDFILTER_IIRFILTER_H

#include <seiscomp/core/genericrecord.h>
#include <seiscomp/io/recordfilter.h>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace IO {


/**
 * \brief RecordInplaceFilter is a record filter that applies a
 * \brief Math::InplaceFilter to each passed record. Type conversion
 * \brief and gap/overlap handling (causing a filter reset) are part of it.
 *
 * RecordIIRFilter does not distinguish between different channels. All
 * records fed into this class are assumed to be of the same stream/channel.
 */
template <typename T>
class SC_SYSTEM_CORE_API RecordIIRFilter : public RecordFilterInterface {
	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef Math::Filtering::InPlaceFilter<T> InplaceFilterType;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructs a record filter with an optional inplace filter.
		//! The passed instance is managed by the record filter.
		RecordIIRFilter(Seiscomp::Math::Filtering::InPlaceFilter<T> *filter = nullptr);
		~RecordIIRFilter();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Note: the ownership goes to the record filter
		RecordIIRFilter<T> &operator=(Seiscomp::Math::Filtering::InPlaceFilter<T> *f);

		//! Note: the ownership goes to the record filter
		void setIIR(Seiscomp::Math::Filtering::InPlaceFilter<T> *f);

		Seiscomp::Math::Filtering::InPlaceFilter<T> *filter() { return _filter; }
		const Seiscomp::Math::Filtering::InPlaceFilter<T> *filter() const { return _filter; }

		//! Applies the IIR filter on the input data. The data type of the
		//! input record must match the requested data type (template
		//! parameter)!
		//! @returns True, if apply was successfull, false otherwise
		bool apply(GenericRecord *rec);

		//! The bool operator returns if an IIR filter is set or not
		operator bool() const { return _filter != nullptr; }

		//! Returns the last error in case feed or apply returned nullptr or false.
		const std::string &lastError() const;


	// ------------------------------------------------------------------
	//  RecordFilter interface
	// ------------------------------------------------------------------
	public:
		//! Applies the filter and returns a copy with a record of the
		//! requested datatype. The returned record instance is a GenericRecord.
		//! If no IIR filter is set a type converted copy is returned.
		virtual Record *feed(const Record *rec);

		virtual Record *flush();

		virtual void reset();

		RecordFilterInterface *clone() const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		InplaceFilterType *_filter;
		Core::Time         _lastEndTime;
		double             _samplingFrequency;
		std::string        _lastError;
};


template <typename T>
inline const std::string &RecordIIRFilter<T>::lastError() const {
	return _lastError;
}


}
}

#endif
