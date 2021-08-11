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


#ifndef SEISCOMP_DATAMODEL_AUXSOURCE_H
#define SEISCOMP_DATAMODEL_AUXSOURCE_H


#include <string>
#include <seiscomp/datamodel/blob.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(AuxSource);

class AuxDevice;


class SC_SYSTEM_CORE_API AuxSourceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxSourceIndex();
		AuxSourceIndex(const std::string& name);

		//! Copy constructor
		AuxSourceIndex(const AuxSourceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AuxSourceIndex&) const;
		bool operator!=(const AuxSourceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a channel of an auxilliary device
 */
class SC_SYSTEM_CORE_API AuxSource : public Object {
	DECLARE_SC_CLASS(AuxSource)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxSource();

		//! Copy constructor
		AuxSource(const AuxSource& other);

		//! Custom constructor
		AuxSource(const std::string& name);
		AuxSource(const std::string& name,
		          const std::string& description,
		          const std::string& unit,
		          const std::string& conversion,
		          const OPT(int)& sampleRateNumerator = Seiscomp::Core::None,
		          const OPT(int)& sampleRateDenominator = Seiscomp::Core::None,
		          const OPT(Blob)& remark = Seiscomp::Core::None);

		//! Destructor
		~AuxSource() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AuxSource& operator=(const AuxSource& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const AuxSource& other) const;
		bool operator!=(const AuxSource& other) const;

		//! Wrapper that calls operator==
		bool equal(const AuxSource& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Referred from network/station/auxStream/@source
		void setName(const std::string& name);
		const std::string& name() const;

		//! Description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! Unit of mesurement
		void setUnit(const std::string& unit);
		const std::string& unit() const;

		//! Conversion formula from counts to unit of measurement
		void setConversion(const std::string& conversion);
		const std::string& conversion() const;

		//! Output sample rate (numerator); referred from
		//! network/station/AuxStream/@sampleRateNumerator
		void setSampleRateNumerator(const OPT(int)& sampleRateNumerator);
		int sampleRateNumerator() const;

		//! Output sample rate (denominator); referred from
		//! network/station/AuxStream/@sampleRateDenominator
		void setSampleRateDenominator(const OPT(int)& sampleRateDenominator);
		int sampleRateDenominator() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AuxSourceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const AuxSource* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		AuxDevice* auxDevice() const;

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
		AuxSourceIndex _index;

		// Attributes
		std::string _description;
		std::string _unit;
		std::string _conversion;
		OPT(int) _sampleRateNumerator;
		OPT(int) _sampleRateDenominator;
		OPT(Blob) _remark;
};


}
}


#endif
