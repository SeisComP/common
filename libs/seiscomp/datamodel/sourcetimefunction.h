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


#ifndef SEISCOMP_DATAMODEL_SOURCETIMEFUNCTION_H
#define SEISCOMP_DATAMODEL_SOURCETIMEFUNCTION_H


#include <seiscomp/datamodel/types.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(SourceTimeFunction);


/**
 * \brief Source time function used in moment-tensor inversion.
 */
class SC_SYSTEM_CORE_API SourceTimeFunction : public Core::BaseObject {
	DECLARE_SC_CLASS(SourceTimeFunction)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SourceTimeFunction();

		//! Copy constructor
		SourceTimeFunction(const SourceTimeFunction& other);

		//! Destructor
		~SourceTimeFunction() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SourceTimeFunction& operator=(const SourceTimeFunction& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const SourceTimeFunction& other) const;
		bool operator!=(const SourceTimeFunction& other) const;

		//! Wrapper that calls operator==
		bool equal(const SourceTimeFunction& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Type of source time function. Values can be taken from the
		//! following:
		//! BOX_CAR, TRIANGLE, TRAPEZOID, UNKNOWN_FUNCTION.
		void setType(SourceTimeFunctionType type);
		SourceTimeFunctionType type() const;

		//! Source time function duration in seconds.
		void setDuration(double duration);
		double duration() const;

		//! Source time function rise time in seconds.
		void setRiseTime(const OPT(double)& riseTime);
		double riseTime() const;

		//! Source time function decay time in seconds.
		void setDecayTime(const OPT(double)& decayTime);
		double decayTime() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		SourceTimeFunctionType _type;
		double _duration;
		OPT(double) _riseTime;
		OPT(double) _decayTime;
};


}
}


#endif
