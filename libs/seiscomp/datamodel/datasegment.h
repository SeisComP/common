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


#ifndef SEISCOMP_DATAMODEL_DATASEGMENT_H
#define SEISCOMP_DATAMODEL_DATASEGMENT_H


#include <seiscomp/core/datetime.h>
#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataSegment);

class DataExtent;


class SC_SYSTEM_CORE_API DataSegmentIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataSegmentIndex();
		DataSegmentIndex(Seiscomp::Core::Time start);

		//! Copy constructor
		DataSegmentIndex(const DataSegmentIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DataSegmentIndex&) const;
		bool operator!=(const DataSegmentIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
};


class SC_SYSTEM_CORE_API DataSegment : public Object {
	DECLARE_SC_CLASS(DataSegment)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataSegment();

		//! Copy constructor
		DataSegment(const DataSegment& other);

		//! Destructor
		~DataSegment() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DataSegment& operator=(const DataSegment& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const DataSegment& other) const;
		bool operator!=(const DataSegment& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataSegment& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Time of first sample of data segment.
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! Time after last sample of data segment.
		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		//! The time of the last update or creation of this data
		//! segment.
		void setUpdated(Seiscomp::Core::Time updated);
		Seiscomp::Core::Time updated() const;

		//! Sample rate of the current data segment.
		void setSampleRate(double sampleRate);
		double sampleRate() const;

		//! Quality indicator of current data segment.
		void setQuality(const std::string& quality);
		const std::string& quality() const;

		//! Whether this segment is an out-of-order segment or not.
		void setOutOfOrder(bool outOfOrder);
		bool outOfOrder() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DataSegmentIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const DataSegment* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		DataExtent* dataExtent() const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		DataSegmentIndex _index;

		// Attributes
		Seiscomp::Core::Time _end;
		Seiscomp::Core::Time _updated;
		double _sampleRate;
		std::string _quality;
		bool _outOfOrder;
};


}
}


#endif
