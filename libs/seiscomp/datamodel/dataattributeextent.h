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


#ifndef SEISCOMP_DATAMODEL_DATAATTRIBUTEEXTENT_H
#define SEISCOMP_DATAMODEL_DATAATTRIBUTEEXTENT_H


#include <seiscomp/core/datetime.h>
#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataAttributeExtent);

class DataExtent;


class SC_SYSTEM_CORE_API DataAttributeExtentIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAttributeExtentIndex();
		DataAttributeExtentIndex(double sampleRate,
		                         const std::string& quality);

		//! Copy constructor
		DataAttributeExtentIndex(const DataAttributeExtentIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DataAttributeExtentIndex&) const;
		bool operator!=(const DataAttributeExtentIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		double sampleRate;
		std::string quality;
};


class SC_SYSTEM_CORE_API DataAttributeExtent : public Object {
	DECLARE_SC_CLASS(DataAttributeExtent)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAttributeExtent();

		//! Copy constructor
		DataAttributeExtent(const DataAttributeExtent& other);

		//! Destructor
		~DataAttributeExtent() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DataAttributeExtent& operator=(const DataAttributeExtent& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const DataAttributeExtent& other) const;
		bool operator!=(const DataAttributeExtent& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataAttributeExtent& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Time of first sample of data attribute extent.
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! Time after last sample of data attribute extent.
		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		//! Sample rate of the current data attribute extent.
		void setSampleRate(double sampleRate);
		double sampleRate() const;

		//! Quality indicator of current data attribute extent.
		void setQuality(const std::string& quality);
		const std::string& quality() const;

		//! The time of the last update or creation of this data
		//! attribute extent.
		void setUpdated(Seiscomp::Core::Time updated);
		Seiscomp::Core::Time updated() const;

		//! Number of data segments covered by this data attribute
		//! extent.
		void setSegmentCount(int segmentCount);
		int segmentCount() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DataAttributeExtentIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const DataAttributeExtent* lhs) const;

	
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
		DataAttributeExtentIndex _index;

		// Attributes
		Seiscomp::Core::Time _start;
		Seiscomp::Core::Time _end;
		Seiscomp::Core::Time _updated;
		int _segmentCount;
};


}
}


#endif
