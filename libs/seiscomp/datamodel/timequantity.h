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


#ifndef SEISCOMP_DATAMODEL_TIMEQUANTITY_H
#define SEISCOMP_DATAMODEL_TIMEQUANTITY_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/timepdf1d.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(TimeQuantity);


/**
 * \brief This type describes a point in time, given in ISO 8601
 * \brief format, with
 * \brief optional symmetric or asymmetric uncertainties given in
 * \brief seconds. The
 * \brief time has to be specified in UTC.
 */
class SC_SYSTEM_CORE_API TimeQuantity : public Core::BaseObject {
	DECLARE_SC_CLASS(TimeQuantity)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		TimeQuantity();

		//! Copy constructor
		TimeQuantity(const TimeQuantity& other);

		//! Custom constructor
		TimeQuantity(Seiscomp::Core::Time value,
		             const OPT(double)& uncertainty = Seiscomp::Core::None,
		             const OPT(double)& lowerUncertainty = Seiscomp::Core::None,
		             const OPT(double)& upperUncertainty = Seiscomp::Core::None,
		             const OPT(double)& confidenceLevel = Seiscomp::Core::None,
		             const OPT(TimePDF1D)& pdf = Seiscomp::Core::None);

		//! Destructor
		~TimeQuantity() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator Seiscomp::Core::Time&();
		operator Seiscomp::Core::Time() const;

		//! Copies the metadata of other to this
		TimeQuantity& operator=(const TimeQuantity& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const TimeQuantity& other) const;
		bool operator!=(const TimeQuantity& other) const;

		//! Wrapper that calls operator==
		bool equal(const TimeQuantity& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Point in time (UTC), given in ISO 8601 format.
		void setValue(Seiscomp::Core::Time value);
		Seiscomp::Core::Time value() const;

		//! Symmetric uncertainty of point in time in seconds.
		void setUncertainty(const OPT(double)& uncertainty);
		double uncertainty() const;

		//! Lower uncertainty of point in time in seconds.
		void setLowerUncertainty(const OPT(double)& lowerUncertainty);
		double lowerUncertainty() const;

		//! Upper uncertainty of point in time in seconds.
		void setUpperUncertainty(const OPT(double)& upperUncertainty);
		double upperUncertainty() const;

		//! Confidence level of the uncertainty, given in percent.
		void setConfidenceLevel(const OPT(double)& confidenceLevel);
		double confidenceLevel() const;

		//! Probability density function of the quantity.
		void setPdf(const OPT(TimePDF1D)& pdf);
		TimePDF1D& pdf();
		const TimePDF1D& pdf() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		Seiscomp::Core::Time _value;
		OPT(double) _uncertainty;
		OPT(double) _lowerUncertainty;
		OPT(double) _upperUncertainty;
		OPT(double) _confidenceLevel;
		OPT(TimePDF1D) _pdf;
};


}
}


#endif
