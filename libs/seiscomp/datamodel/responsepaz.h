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


#ifndef SEISCOMP_DATAMODEL_RESPONSEPAZ_H
#define SEISCOMP_DATAMODEL_RESPONSEPAZ_H


#include <string>
#include <seiscomp/datamodel/complexarray.h>
#include <seiscomp/datamodel/blob.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ResponsePAZ);

class Inventory;


class SC_SYSTEM_CORE_API ResponsePAZIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponsePAZIndex();
		ResponsePAZIndex(const std::string& name);

		//! Copy constructor
		ResponsePAZIndex(const ResponsePAZIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ResponsePAZIndex&) const;
		bool operator!=(const ResponsePAZIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a sensor response using poles and zeros
 */
class SC_SYSTEM_CORE_API ResponsePAZ : public PublicObject {
	DECLARE_SC_CLASS(ResponsePAZ)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ResponsePAZ();

	public:
		//! Copy constructor
		ResponsePAZ(const ResponsePAZ& other);

		//! Constructor with publicID
		ResponsePAZ(const std::string& publicID);

		//! Destructor
		~ResponsePAZ() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ResponsePAZ* Create();
		static ResponsePAZ* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ResponsePAZ* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ResponsePAZ& operator=(const ResponsePAZ& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ResponsePAZ& other) const;
		bool operator!=(const ResponsePAZ& other) const;

		//! Wrapper that calls operator==
		bool equal(const ResponsePAZ& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique response name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Response type (43.05/53.03): A - Laplace transform analog
		//! response in rad/sec, B - Analog response in Hz, C -
		//! Composite (currently undefined), D - Digital (Z - transform)
		void setType(const std::string& type);
		const std::string& type() const;

		//! Gain of response (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

		//! A0 normalization factor (43.08/53.07)
		void setNormalizationFactor(const OPT(double)& normalizationFactor);
		double normalizationFactor() const;

		//! Normalization frequency (43.09/53.08)
		void setNormalizationFrequency(const OPT(double)& normalizationFrequency);
		double normalizationFrequency() const;

		//! Number of zeros (43.10/53.09)
		void setNumberOfZeros(const OPT(int)& numberOfZeros);
		int numberOfZeros() const;

		//! Number of poles (43.15/53.14)
		void setNumberOfPoles(const OPT(int)& numberOfPoles);
		int numberOfPoles() const;

		//! Zeros (43.16-19/53.10-13)
		void setZeros(const OPT(ComplexArray)& zeros);
		ComplexArray& zeros();
		const ComplexArray& zeros() const;

		//! Poles (43.11-14/53.15-18)
		void setPoles(const OPT(ComplexArray)& poles);
		ComplexArray& poles();
		const ComplexArray& poles() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;

		//! Decimation factor (47.06/57.05)
		void setDecimationFactor(const OPT(int)& decimationFactor);
		int decimationFactor() const;

		//! Estimated delay (47.08/57.07)
		void setDelay(const OPT(double)& delay);
		double delay() const;

		//! Applied correction (47.09/57.08)
		void setCorrection(const OPT(double)& correction);
		double correction() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ResponsePAZIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ResponsePAZ* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Inventory* inventory() const;

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
		// Index
		ResponsePAZIndex _index;

		// Attributes
		std::string _type;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(double) _normalizationFactor;
		OPT(double) _normalizationFrequency;
		OPT(int) _numberOfZeros;
		OPT(int) _numberOfPoles;
		OPT(ComplexArray) _zeros;
		OPT(ComplexArray) _poles;
		OPT(Blob) _remark;
		OPT(int) _decimationFactor;
		OPT(double) _delay;
		OPT(double) _correction;

	DECLARE_SC_CLASSFACTORY_FRIEND(ResponsePAZ);
};


}
}


#endif
