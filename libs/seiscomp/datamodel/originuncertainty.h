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


#ifndef SEISCOMP_DATAMODEL_ORIGINUNCERTAINTY_H
#define SEISCOMP_DATAMODEL_ORIGINUNCERTAINTY_H


#include <seiscomp/datamodel/confidenceellipsoid.h>
#include <seiscomp/datamodel/types.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(OriginUncertainty);


/**
 * \brief This class describes the location uncertainties of an
 * \brief origin. The uncertainty
 * \brief can be described either as a simple circular horizontal
 * \brief uncertainty, an
 * \brief uncertainty ellipse according to IMS1.0, or a confidence
 * \brief ellipsoid. If
 * \brief multiple uncertainty models are given, the preferred
 * \brief variant can be
 * \brief specified in the attribute preferredDescription.
 */
class SC_SYSTEM_CORE_API OriginUncertainty : public Core::BaseObject {
	DECLARE_SC_CLASS(OriginUncertainty)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		OriginUncertainty();

		//! Copy constructor
		OriginUncertainty(const OriginUncertainty& other);

		//! Destructor
		~OriginUncertainty() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		OriginUncertainty& operator=(const OriginUncertainty& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const OriginUncertainty& other) const;
		bool operator!=(const OriginUncertainty& other) const;

		//! Wrapper that calls operator==
		bool equal(const OriginUncertainty& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Circular confidence region, given by single value of
		//! horizontal uncertainty in km.
		void setHorizontalUncertainty(const OPT(double)& horizontalUncertainty);
		double horizontalUncertainty() const;

		//! Semi-minor axis of confidence ellipse in km.
		void setMinHorizontalUncertainty(const OPT(double)& minHorizontalUncertainty);
		double minHorizontalUncertainty() const;

		//! Semi-major axis of confidence ellipse in km.
		void setMaxHorizontalUncertainty(const OPT(double)& maxHorizontalUncertainty);
		double maxHorizontalUncertainty() const;

		//! Azimuth of major axis of confidence ellipse. Measured
		//! clockwise from
		//! South-North direction at epicenter in degrees.
		void setAzimuthMaxHorizontalUncertainty(const OPT(double)& azimuthMaxHorizontalUncertainty);
		double azimuthMaxHorizontalUncertainty() const;

		//! Confidence ellipsoid.
		void setConfidenceEllipsoid(const OPT(ConfidenceEllipsoid)& confidenceEllipsoid);
		ConfidenceEllipsoid& confidenceEllipsoid();
		const ConfidenceEllipsoid& confidenceEllipsoid() const;

		//! Preferred uncertainty description.
		void setPreferredDescription(const OPT(OriginUncertaintyDescription)& preferredDescription);
		OriginUncertaintyDescription preferredDescription() const;

		//! Confidence level of the uncertainty, given in percent.
		void setConfidenceLevel(const OPT(double)& confidenceLevel);
		double confidenceLevel() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(double) _horizontalUncertainty;
		OPT(double) _minHorizontalUncertainty;
		OPT(double) _maxHorizontalUncertainty;
		OPT(double) _azimuthMaxHorizontalUncertainty;
		OPT(ConfidenceEllipsoid) _confidenceEllipsoid;
		OPT(OriginUncertaintyDescription) _preferredDescription;
		OPT(double) _confidenceLevel;
};


}
}


#endif
