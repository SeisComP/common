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


#ifndef SEISCOMP_DATAMODEL_ROUTEARCLINK_H
#define SEISCOMP_DATAMODEL_ROUTEARCLINK_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(RouteArclink);

class Route;


class SC_SYSTEM_CORE_API RouteArclinkIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteArclinkIndex();
		RouteArclinkIndex(const std::string& address,
		                  Seiscomp::Core::Time start);

		//! Copy constructor
		RouteArclinkIndex(const RouteArclinkIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const RouteArclinkIndex&) const;
		bool operator!=(const RouteArclinkIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string address;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes an ArcLink route (data source)
 */
class SC_SYSTEM_CORE_API RouteArclink : public Object {
	DECLARE_SC_CLASS(RouteArclink)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteArclink();

		//! Copy constructor
		RouteArclink(const RouteArclink& other);

		//! Destructor
		~RouteArclink() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		RouteArclink& operator=(const RouteArclink& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const RouteArclink& other) const;
		bool operator!=(const RouteArclink& other) const;

		//! Wrapper that calls operator==
		bool equal(const RouteArclink& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Server address in ip:port format
		void setAddress(const std::string& address);
		const std::string& address() const;

		//! Start of data
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of data
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! priority (1 is highest)
		void setPriority(const OPT(int)& priority);
		int priority() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const RouteArclinkIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const RouteArclink* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Route* route() const;

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
		RouteArclinkIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(int) _priority;
};


}
}


#endif
