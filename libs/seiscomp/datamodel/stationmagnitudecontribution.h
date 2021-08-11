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


#ifndef SEISCOMP_DATAMODEL_STATIONMAGNITUDECONTRIBUTION_H
#define SEISCOMP_DATAMODEL_STATIONMAGNITUDECONTRIBUTION_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(StationMagnitudeContribution);

class Magnitude;


class SC_SYSTEM_CORE_API StationMagnitudeContributionIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationMagnitudeContributionIndex();
		StationMagnitudeContributionIndex(const std::string& stationMagnitudeID);

		//! Copy constructor
		StationMagnitudeContributionIndex(const StationMagnitudeContributionIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const StationMagnitudeContributionIndex&) const;
		bool operator!=(const StationMagnitudeContributionIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string stationMagnitudeID;
};


/**
 * \brief This class describes the weighting of magnitude values from
 * \brief several StationMagnitude objects for computing a
 * \brief network magnitude estimation.
 */
class SC_SYSTEM_CORE_API StationMagnitudeContribution : public Object {
	DECLARE_SC_CLASS(StationMagnitudeContribution)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationMagnitudeContribution();

		//! Copy constructor
		StationMagnitudeContribution(const StationMagnitudeContribution& other);

		//! Custom constructor
		StationMagnitudeContribution(const std::string& stationMagnitudeID,
		                             const OPT(double)& residual = Seiscomp::Core::None,
		                             const OPT(double)& weight = Seiscomp::Core::None);

		//! Destructor
		~StationMagnitudeContribution() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		StationMagnitudeContribution& operator=(const StationMagnitudeContribution& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const StationMagnitudeContribution& other) const;
		bool operator!=(const StationMagnitudeContribution& other) const;

		//! Wrapper that calls operator==
		bool equal(const StationMagnitudeContribution& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Refers to the publicID of a StationMagnitude object.
		void setStationMagnitudeID(const std::string& stationMagnitudeID);
		const std::string& stationMagnitudeID() const;

		//! Residual of magnitude computation.
		void setResidual(const OPT(double)& residual);
		double residual() const;

		//! Weight of the magnitude value from class StationMagnitude
		//! for computing
		//! the magnitude value in class Magnitude. Note that there is
		//! no rule
		//! for the sum of the weights of all station magnitude
		//! contributions
		//! to a specific network magnitude. In particular, the weights
		//! are not
		//! required to sum up to unity.
		void setWeight(const OPT(double)& weight);
		double weight() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const StationMagnitudeContributionIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const StationMagnitudeContribution* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Magnitude* magnitude() const;

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
		StationMagnitudeContributionIndex _index;

		// Attributes
		OPT(double) _residual;
		OPT(double) _weight;
};


}
}


#endif
