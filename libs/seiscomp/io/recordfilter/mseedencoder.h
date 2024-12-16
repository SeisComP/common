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


#ifndef SEISCOMP_IO_RECORDFILTER_MSEEDENCODER_H
#define SEISCOMP_IO_RECORDFILTER_MSEEDENCODER_H


#include <seiscomp/core/genericrecord.h>
#include <seiscomp/io/recordfilter.h>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API MSeedEncoder : public RecordFilterInterface {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		MSeedEncoder() = default;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Sets the record size of the compressed MiniSEED record.
		 * @param size The size of the compressed record as an exponent of base 2.
		 *             The value range is from 7 to 20.
		 * @return Success flag.
		 */
		bool setRecordSize(int size);

		/**
		 * @brief Sets uncompressed encoding.
		 */
		void setIdentity();

		/**
		 * @brief Enables Steim1 compression.
		 */
		void setSteim1();

		/**
		 * @brief Enables Steim2 compression.
		 * The Steim2 compression is enabled by default.
		 */
		void setSteim2();

		void allowFloatingPointCompression(bool f);


	// ------------------------------------------------------------------
	//  RecordFilter interface
	// ------------------------------------------------------------------
	public:
		//! Applies the filter and returns a copy with a record of the
		//! requested datatype. The returned record instance is a GenericRecord.
		//! If no IIR filter is set a type converted copy is returned.
		Record *feed(const Record *rec) override;

		Record *flush() override;

		void reset() override;

		RecordFilterInterface *clone() const override;


	// ------------------------------------------------------------------
	//  Private methods
	// ------------------------------------------------------------------
	private:
		Record *pop();


	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		enum CompressionType {
			Identity,
			Steim1,
			Steim2
		};


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		double              _samplingFrequency{-1};
		int                 _recordSize{9};
		CompressionType     _compression{Steim2};
		Array::DataType     _dataType{Array::DT_QUANTITY};
		bool                _allowFloatingPointCompression{true};
		Core::BaseObjectPtr _encoder{nullptr};
};


}
}

#endif
