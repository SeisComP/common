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


#ifndef SEISCOMP_IO_RECORDINPUT_H
#define SEISCOMP_IO_RECORDINPUT_H

#include <iterator>
#include <seiscomp/core.h>
#include <seiscomp/io/recordstream.h>

namespace Seiscomp {
namespace IO {


class RecordInput;

class SC_SYSTEM_CORE_API RecordIterator : public std::iterator<std::input_iterator_tag, Record *> {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		RecordIterator();
		//! Copy c'tor
		RecordIterator(const RecordIterator &iter);
		//! D'tor
		~RecordIterator();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		RecordIterator &operator=(const RecordIterator &iter);
		Record *operator*();
		RecordIterator &operator++();
		RecordIterator operator++(int);
		bool operator!=(const RecordIterator &iter) const;
		bool operator==(const RecordIterator &iter) const;
		

	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Returns the source used by the iterator
		 * @return A RecordInput pointer which must not be deleted
		 *         by the caller!
		 */
		RecordInput *source() const;

		/**
		 * Returns the current record read from the input stream.
		 * The record pointer is a raw pointer and has to be managed
		 * by the caller.
		 * @return The raw record pointer.
		 */
		Record *current() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		RecordIterator(RecordInput *from, Record *cur);
		RecordInput *_source;
		Record      *_current;

	friend class RecordInput;
};


DEFINE_SMARTPOINTER(RecordInput);

class SC_SYSTEM_CORE_API RecordInput : public Seiscomp::Core::BaseObject {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		RecordInput(RecordStream *in,
		            Array::DataType dt = Array::DOUBLE,
		            Record::Hint h = Record::SAVE_RAW);


	// ------------------------------------------------------------------
	//  Iteration
	// ------------------------------------------------------------------
	public:
		RecordIterator begin();
		RecordIterator end();
		Record *next();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		RecordStream *_in;
};


}
}


#endif
