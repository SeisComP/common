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


#ifndef SEISCOMP_DATAMODEL_MOMENTTENSORSTATIONCONTRIBUTION_H
#define SEISCOMP_DATAMODEL_MOMENTTENSORSTATIONCONTRIBUTION_H


#include <seiscomp/datamodel/waveformstreamid.h>
#include <vector>
#include <seiscomp/datamodel/momenttensorcomponentcontribution.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensorStationContribution);
DEFINE_SMARTPOINTER(MomentTensorComponentContribution);

class MomentTensor;


class SC_SYSTEM_CORE_API MomentTensorStationContribution : public PublicObject {
	DECLARE_SC_CLASS(MomentTensorStationContribution)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		MomentTensorStationContribution();

	public:
		//! Copy constructor
		MomentTensorStationContribution(const MomentTensorStationContribution& other);

		//! Constructor with publicID
		MomentTensorStationContribution(const std::string& publicID);

		//! Destructor
		~MomentTensorStationContribution() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static MomentTensorStationContribution* Create();
		static MomentTensorStationContribution* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static MomentTensorStationContribution* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		MomentTensorStationContribution& operator=(const MomentTensorStationContribution& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const MomentTensorStationContribution& other) const;
		bool operator!=(const MomentTensorStationContribution& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensorStationContribution& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setActive(bool active);
		bool active() const;

		void setWaveformID(const OPT(WaveformStreamID)& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setWeight(const OPT(double)& weight);
		double weight() const;

		void setTimeShift(const OPT(double)& timeShift);
		double timeShift() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(MomentTensorComponentContribution* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(MomentTensorComponentContribution* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeMomentTensorComponentContribution(size_t i);
		bool removeMomentTensorComponentContribution(const MomentTensorComponentContributionIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t momentTensorComponentContributionCount() const;

		//! Index access
		//! @return The object at index i
		MomentTensorComponentContribution* momentTensorComponentContribution(size_t i) const;
		MomentTensorComponentContribution* momentTensorComponentContribution(const MomentTensorComponentContributionIndex& i) const;

		//! Find an object by its unique attribute(s)

		MomentTensor* momentTensor() const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		//! Implement PublicObject interface
		bool updateChild(Object *child) override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		bool _active;
		OPT(WaveformStreamID) _waveformID;
		OPT(double) _weight;
		OPT(double) _timeShift;

		// Aggregations
		std::vector<MomentTensorComponentContributionPtr> _momentTensorComponentContributions;

	DECLARE_SC_CLASSFACTORY_FRIEND(MomentTensorStationContribution);
};


}
}


#endif
