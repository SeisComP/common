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


#ifndef SEISCOMP_DATAMODEL_MOMENTTENSORPHASESETTING_H
#define SEISCOMP_DATAMODEL_MOMENTTENSORPHASESETTING_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensorPhaseSetting);

class MomentTensor;


class SC_SYSTEM_CORE_API MomentTensorPhaseSettingIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorPhaseSettingIndex();
		MomentTensorPhaseSettingIndex(const std::string& code);

		//! Copy constructor
		MomentTensorPhaseSettingIndex(const MomentTensorPhaseSettingIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const MomentTensorPhaseSettingIndex&) const;
		bool operator!=(const MomentTensorPhaseSettingIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
};


class SC_SYSTEM_CORE_API MomentTensorPhaseSetting : public Object {
	DECLARE_SC_CLASS(MomentTensorPhaseSetting)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorPhaseSetting();

		//! Copy constructor
		MomentTensorPhaseSetting(const MomentTensorPhaseSetting& other);

		//! Custom constructor
		MomentTensorPhaseSetting(const std::string& code);
		MomentTensorPhaseSetting(const std::string& code,
		                         double lowerPeriod,
		                         double upperPeriod,
		                         const OPT(double)& minimumSNR = Seiscomp::Core::None,
		                         const OPT(double)& maximumTimeShift = Seiscomp::Core::None);

		//! Destructor
		~MomentTensorPhaseSetting() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		MomentTensorPhaseSetting& operator=(const MomentTensorPhaseSetting& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const MomentTensorPhaseSetting& other) const;
		bool operator!=(const MomentTensorPhaseSetting& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensorPhaseSetting& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setCode(const std::string& code);
		const std::string& code() const;

		void setLowerPeriod(double lowerPeriod);
		double lowerPeriod() const;

		void setUpperPeriod(double upperPeriod);
		double upperPeriod() const;

		void setMinimumSNR(const OPT(double)& minimumSNR);
		double minimumSNR() const;

		void setMaximumTimeShift(const OPT(double)& maximumTimeShift);
		double maximumTimeShift() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const MomentTensorPhaseSettingIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const MomentTensorPhaseSetting* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		MomentTensor* momentTensor() const;

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
		MomentTensorPhaseSettingIndex _index;

		// Attributes
		double _lowerPeriod;
		double _upperPeriod;
		OPT(double) _minimumSNR;
		OPT(double) _maximumTimeShift;
};


}
}


#endif
