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


#ifndef SEISCOMP_DATAMODEL_OUTAGE_H
#define SEISCOMP_DATAMODEL_OUTAGE_H


#include <seiscomp/datamodel/waveformstreamid.h>
#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Outage);

class QualityControl;


class SC_SYSTEM_CORE_API OutageIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		OutageIndex();
		OutageIndex(const WaveformStreamID& waveformID,
		            Seiscomp::Core::Time start);

		//! Copy constructor
		OutageIndex(const OutageIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const OutageIndex&) const;
		bool operator!=(const OutageIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		WaveformStreamID waveformID;
		Seiscomp::Core::Time start;
};


class SC_SYSTEM_CORE_API Outage : public Object {
	DECLARE_SC_CLASS(Outage)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Outage();

		//! Copy constructor
		Outage(const Outage& other);

		//! Destructor
		~Outage() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Outage& operator=(const Outage& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Outage& other) const;
		bool operator!=(const Outage& other) const;

		//! Wrapper that calls operator==
		bool equal(const Outage& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setCreatorID(const std::string& creatorID);
		const std::string& creatorID() const;

		void setCreated(Seiscomp::Core::Time created);
		Seiscomp::Core::Time created() const;

		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const OutageIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Outage* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QualityControl* qualityControl() const;

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
		OutageIndex _index;

		// Attributes
		std::string _creatorID;
		Seiscomp::Core::Time _created;
		OPT(Seiscomp::Core::Time) _end;
};


}
}


#endif
