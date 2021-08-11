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


#ifndef SEISCOMP_DATAMODEL_TIMEWINDOW_H
#define SEISCOMP_DATAMODEL_TIMEWINDOW_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(TimeWindow);


/**
 * \brief Describes a time window for amplitude measurements, given
 * \brief by a central point in time, and points in time
 * \brief before and after this central point. Both points before and
 * \brief after may coincide with the central point.
 */
class SC_SYSTEM_CORE_API TimeWindow : public Core::BaseObject {
	DECLARE_SC_CLASS(TimeWindow)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		TimeWindow();

		//! Copy constructor
		TimeWindow(const TimeWindow& other);

		//! Custom constructor
		TimeWindow(Seiscomp::Core::Time reference);
		TimeWindow(Seiscomp::Core::Time reference,
		           double begin,
		           double end);

		//! Destructor
		~TimeWindow() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator Seiscomp::Core::Time&();
		operator Seiscomp::Core::Time() const;

		//! Copies the metadata of other to this
		TimeWindow& operator=(const TimeWindow& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const TimeWindow& other) const;
		bool operator!=(const TimeWindow& other) const;

		//! Wrapper that calls operator==
		bool equal(const TimeWindow& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Reference point in time ("central" point), in ISO 8601
		//! format. It
		//! has to be given in UTC.
		void setReference(Seiscomp::Core::Time reference);
		Seiscomp::Core::Time reference() const;

		//! Absolute value of duration of time interval before
		//! reference point
		//! in time window. The value may be zero, but not negative in
		//! seconds.
		void setBegin(double begin);
		double begin() const;

		//! Absolute value of duration of time interval after reference
		//! point in
		//! time window. The value may be zero, but not negative in
		//! seconds.
		void setEnd(double end);
		double end() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		Seiscomp::Core::Time _reference;
		double _begin;
		double _end;
};


}
}


#endif
