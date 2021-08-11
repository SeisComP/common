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


#ifndef SEISCOMP_DATAMODEL_ARCLINKREQUESTSUMMARY_H
#define SEISCOMP_DATAMODEL_ARCLINKREQUESTSUMMARY_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkRequestSummary);


class SC_SYSTEM_CORE_API ArclinkRequestSummary : public Core::BaseObject {
	DECLARE_SC_CLASS(ArclinkRequestSummary)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestSummary();

		//! Copy constructor
		ArclinkRequestSummary(const ArclinkRequestSummary& other);

		//! Destructor
		~ArclinkRequestSummary() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ArclinkRequestSummary& operator=(const ArclinkRequestSummary& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ArclinkRequestSummary& other) const;
		bool operator!=(const ArclinkRequestSummary& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkRequestSummary& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setOkLineCount(int okLineCount);
		int okLineCount() const;

		void setTotalLineCount(int totalLineCount);
		int totalLineCount() const;

		void setAverageTimeWindow(int averageTimeWindow);
		int averageTimeWindow() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		int _okLineCount;
		int _totalLineCount;
		int _averageTimeWindow;
};


}
}


#endif
