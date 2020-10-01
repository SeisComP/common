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


#ifndef SEISCOMP_CORE_ARRAY_H
#define SEISCOMP_CORE_ARRAY_H


#include <seiscomp/core/baseobject.h>
#include <string>


namespace Seiscomp {

DEFINE_SMARTPOINTER(Array);

/**
 * Generic abstract base class of certain array types.
 */
class SC_SYSTEM_CORE_API Array : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(Array);

	public:
		//! Specifies the supported array data types.
		enum DataType {
			CHAR,
			INT,
			FLOAT,
			DOUBLE,
			DATETIME,
			STRING,
			COMPLEX_FLOAT,
			COMPLEX_DOUBLE,
			DT_QUANTITY
		};
	

	protected:
		//! Initializing Constructor
		Array(DataType dt);

	public:
		//! Destructor
		virtual ~Array();
	
		//! Returns the data type of the array
		DataType dataType() const { return _datatype; }
	
		//! Returns a clone of the array
		Array* clone() const;

		//! Returns a copy of the array of the specified data type.
		virtual Array* copy(DataType dt) const = 0;
	
		//! Returns the data address pointer.
		virtual const void *data() const = 0;
	
		//! Returns the size of the array.
		virtual int size() const = 0;

		//! Resizes the array
		virtual void resize(int size) = 0;

		//! Drops all elements.
		virtual void clear() = 0;
	
		//! Returns the number of bytes of an array element.
		virtual int elementSize() const = 0;

		//! Appends the given array to this array.
		virtual void append(const Array*) = 0;

		//! Concatenates the given array to this array.
		//		virtual void concatenate(Array*) = 0;

		//! Returns the slice m...n-1 of the array
		virtual Array* slice(int m, int n) const = 0;

		//! Converts the array into a binary stream of
		//! chars
		std::string str() const;

	private:
		DataType _datatype;
};

}

#endif
