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


#ifndef SEISCOMP_IO_RECORDS_SACRECORD_H
#define SEISCOMP_IO_RECORDS_SACRECORD_H


#include <seiscomp/core/record.h>
#include <seiscomp/core/array.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(SACRecord);


class SC_SYSTEM_CORE_API SACRecord : public Record {
	public:
		//! Initializing Constructor
		SACRecord(const std::string &net = "AB", const std::string &sta = "12345",
		          const std::string &loc = "", const std::string &cha = "XYZ",
		          Core::Time stime = Core::Time(), double fsamp=0., int tqual=-1,
		          Array::DataType dt = Array::DOUBLE, Hint h = DATA_ONLY);

		//! Copy Constructor
		SACRecord(const SACRecord &rec);
		SACRecord(const Record &rec);

		//! Destructor
		virtual ~SACRecord();


	public:
		//! Assignment operator
		SACRecord &operator=(const SACRecord &other);

		//! Returns the data samples if the data is available; otherwise 0
		Array* data();

		//! Returns the data samples if the data is available; otherwise 0
		const Array* data() const;

		const Array* raw() const;

		//! Sets the data sample array. The ownership goes over to the record.
		void setData(Array* data);

		//! Sets the data sample array.
		void setData(int size, const void *data, Array::DataType datatype);

		//! Returns a deep copy of the calling object.
		SACRecord *copy() const;

		void saveSpace() const;

		void read(std::istream &in);
		void write(std::ostream &out);


	private:
		mutable ArrayPtr _data;

};


}
}

#endif
