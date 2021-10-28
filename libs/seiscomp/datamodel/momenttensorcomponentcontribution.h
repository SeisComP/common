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


#ifndef SEISCOMP_DATAMODEL_MOMENTTENSORCOMPONENTCONTRIBUTION_H
#define SEISCOMP_DATAMODEL_MOMENTTENSORCOMPONENTCONTRIBUTION_H


#include <string>
#include <vector>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensorComponentContribution);

class MomentTensorStationContribution;


class SC_SYSTEM_CORE_API MomentTensorComponentContributionIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorComponentContributionIndex();
		MomentTensorComponentContributionIndex(const std::string& phaseCode,
		                                       int component);

		//! Copy constructor
		MomentTensorComponentContributionIndex(const MomentTensorComponentContributionIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const MomentTensorComponentContributionIndex&) const;
		bool operator!=(const MomentTensorComponentContributionIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string phaseCode;
		int component;
};


class SC_SYSTEM_CORE_API MomentTensorComponentContribution : public Object {
	DECLARE_SC_CLASS(MomentTensorComponentContribution)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorComponentContribution();

		//! Copy constructor
		MomentTensorComponentContribution(const MomentTensorComponentContribution& other);

		//! Custom constructor
		MomentTensorComponentContribution(const std::string& phaseCode);
		MomentTensorComponentContribution(const std::string& phaseCode,
		                                  int component,
		                                  bool active,
		                                  double weight,
		                                  double timeShift,
		                                  double dataTimeWindow,
		                                  const OPT(double)& misfit = Seiscomp::Core::None,
		                                  const OPT(double)& snr = Seiscomp::Core::None);

		//! Destructor
		~MomentTensorComponentContribution() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		MomentTensorComponentContribution& operator=(const MomentTensorComponentContribution& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const MomentTensorComponentContribution& other) const;
		bool operator!=(const MomentTensorComponentContribution& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensorComponentContribution& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setPhaseCode(const std::string& phaseCode);
		const std::string& phaseCode() const;

		void setComponent(int component);
		int component() const;

		void setActive(bool active);
		bool active() const;

		void setWeight(double weight);
		double weight() const;

		void setTimeShift(double timeShift);
		double timeShift() const;

		void setDataTimeWindow(const std::vector< double >&);
		const std::vector< double >& dataTimeWindow() const;
		std::vector< double >& dataTimeWindow();

		void setMisfit(const OPT(double)& misfit);
		double misfit() const;

		void setSnr(const OPT(double)& snr);
		double snr() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const MomentTensorComponentContributionIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const MomentTensorComponentContribution* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		MomentTensorStationContribution* momentTensorStationContribution() const;

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
		MomentTensorComponentContributionIndex _index;

		// Attributes
		bool _active;
		double _weight;
		double _timeShift;
		std::vector< double > _dataTimeWindow;
		OPT(double) _misfit;
		OPT(double) _snr;
};


}
}


#endif
