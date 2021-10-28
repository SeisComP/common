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


#ifndef SEISCOMP_DATAMODEL_ACCESS_H
#define SEISCOMP_DATAMODEL_ACCESS_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Access);

class Routing;


class SC_SYSTEM_CORE_API AccessIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AccessIndex();
		AccessIndex(const std::string& networkCode,
		            const std::string& stationCode,
		            const std::string& locationCode,
		            const std::string& streamCode,
		            const std::string& user,
		            Seiscomp::Core::Time start);

		//! Copy constructor
		AccessIndex(const AccessIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AccessIndex&) const;
		bool operator!=(const AccessIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string networkCode;
		std::string stationCode;
		std::string locationCode;
		std::string streamCode;
		std::string user;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes an ArcLink access rule
 */
class SC_SYSTEM_CORE_API Access : public Object {
	DECLARE_SC_CLASS(Access)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Access();

		//! Copy constructor
		Access(const Access& other);

		//! Destructor
		~Access() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Access& operator=(const Access& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Access& other) const;
		bool operator!=(const Access& other) const;

		//! Wrapper that calls operator==
		bool equal(const Access& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Network code
		void setNetworkCode(const std::string& networkCode);
		const std::string& networkCode() const;

		//! Station code (empty for any station)
		void setStationCode(const std::string& stationCode);
		const std::string& stationCode() const;

		//! Location code (empty for any location)
		void setLocationCode(const std::string& locationCode);
		const std::string& locationCode() const;

		//! Stream (Channel) code (empty for any stream)
		void setStreamCode(const std::string& streamCode);
		const std::string& streamCode() const;

		//! Username (e-mail) or part of it (must match the end)
		void setUser(const std::string& user);
		const std::string& user() const;

		//! Start of validity
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of validity
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AccessIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Access* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Routing* routing() const;

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
		AccessIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
};


}
}


#endif
