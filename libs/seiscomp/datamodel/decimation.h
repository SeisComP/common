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


#ifndef SEISCOMP_DATAMODEL_DECIMATION_H
#define SEISCOMP_DATAMODEL_DECIMATION_H


#include <seiscomp/datamodel/blob.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Decimation);

class Datalogger;


class SC_SYSTEM_CORE_API DecimationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DecimationIndex();
		DecimationIndex(int sampleRateNumerator,
		                int sampleRateDenominator);

		//! Copy constructor
		DecimationIndex(const DecimationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DecimationIndex&) const;
		bool operator!=(const DecimationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		int sampleRateNumerator;
		int sampleRateDenominator;
};


/**
 * \brief This type describes a decimation to a certain sample rate
 */
class SC_SYSTEM_CORE_API Decimation : public Object {
	DECLARE_SC_CLASS(Decimation)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Decimation();

		//! Copy constructor
		Decimation(const Decimation& other);

		//! Destructor
		~Decimation() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Decimation& operator=(const Decimation& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Decimation& other) const;
		bool operator!=(const Decimation& other) const;

		//! Wrapper that calls operator==
		bool equal(const Decimation& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Output sample rate (numerator); referred from
		//! network/station/Stream/@sampleRateNumerator
		void setSampleRateNumerator(int sampleRateNumerator);
		int sampleRateNumerator() const;

		//! Output sample rate (denominator); referred from
		//! network/station/Stream/@sampleRateDenominator
		void setSampleRateDenominator(int sampleRateDenominator);
		int sampleRateDenominator() const;

		//! Specifies analogue filters between seismometer and
		//! digitizer. Each element (separated by space) references
		//! responsePAZ/@publicID
		void setAnalogueFilterChain(const OPT(Blob)& analogueFilterChain);
		Blob& analogueFilterChain();
		const Blob& analogueFilterChain() const;

		//! Specifies digital filters (decimation, gain removal). Each
		//! element (separated by space) references
		//! responsePAZ@publicID or responseFIR/@publicID
		void setDigitalFilterChain(const OPT(Blob)& digitalFilterChain);
		Blob& digitalFilterChain();
		const Blob& digitalFilterChain() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DecimationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Decimation* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Datalogger* datalogger() const;

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
		DecimationIndex _index;

		// Attributes
		OPT(Blob) _analogueFilterChain;
		OPT(Blob) _digitalFilterChain;
};


}
}


#endif
