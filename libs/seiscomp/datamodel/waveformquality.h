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


#ifndef SEISCOMP_DATAMODEL_WAVEFORMQUALITY_H
#define SEISCOMP_DATAMODEL_WAVEFORMQUALITY_H


#include <seiscomp/datamodel/waveformstreamid.h>
#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(WaveformQuality);

class QualityControl;


class SC_SYSTEM_CORE_API WaveformQualityIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		WaveformQualityIndex();
		WaveformQualityIndex(Seiscomp::Core::Time start,
		                     const WaveformStreamID& waveformID,
		                     const std::string& type,
		                     const std::string& parameter);

		//! Copy constructor
		WaveformQualityIndex(const WaveformQualityIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const WaveformQualityIndex&) const;
		bool operator!=(const WaveformQualityIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
		WaveformStreamID waveformID;
		std::string type;
		std::string parameter;
};


class SC_SYSTEM_CORE_API WaveformQuality : public Object {
	DECLARE_SC_CLASS(WaveformQuality)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		WaveformQuality();

		//! Copy constructor
		WaveformQuality(const WaveformQuality& other);

		//! Destructor
		~WaveformQuality() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		WaveformQuality& operator=(const WaveformQuality& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const WaveformQuality& other) const;
		bool operator!=(const WaveformQuality& other) const;

		//! Wrapper that calls operator==
		bool equal(const WaveformQuality& other) const;


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

		void setType(const std::string& type);
		const std::string& type() const;

		void setParameter(const std::string& parameter);
		const std::string& parameter() const;

		void setValue(double value);
		double value() const;

		void setLowerUncertainty(const OPT(double)& lowerUncertainty);
		double lowerUncertainty() const;

		void setUpperUncertainty(const OPT(double)& upperUncertainty);
		double upperUncertainty() const;

		void setWindowLength(const OPT(double)& windowLength);
		double windowLength() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const WaveformQualityIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const WaveformQuality* lhs) const;

	
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
		WaveformQualityIndex _index;

		// Attributes
		std::string _creatorID;
		Seiscomp::Core::Time _created;
		OPT(Seiscomp::Core::Time) _end;
		double _value;
		OPT(double) _lowerUncertainty;
		OPT(double) _upperUncertainty;
		OPT(double) _windowLength;
};


}
}


#endif
