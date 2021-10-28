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


#ifndef SEISCOMP_DATAMODEL_CREATIONINFO_H
#define SEISCOMP_DATAMODEL_CREATIONINFO_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(CreationInfo);


/**
 * \brief CreationInfo is used to describe creation metadata (author,
 * \brief version, and creation time) of a resource.
 */
class SC_SYSTEM_CORE_API CreationInfo : public Core::BaseObject {
	DECLARE_SC_CLASS(CreationInfo)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		CreationInfo();

		//! Copy constructor
		CreationInfo(const CreationInfo& other);

		//! Destructor
		~CreationInfo() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		CreationInfo& operator=(const CreationInfo& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const CreationInfo& other) const;
		bool operator!=(const CreationInfo& other) const;

		//! Wrapper that calls operator==
		bool equal(const CreationInfo& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Designation of agency that published a resource. The string
		//! has a maximum length of 64 characters.
		void setAgencyID(const std::string& agencyID);
		const std::string& agencyID() const;

		//! RI of the agency that published a resource.
		void setAgencyURI(const std::string& agencyURI);
		const std::string& agencyURI() const;

		//! Name describing the author of a resource. The string has a
		//! maximum length of 128 characters.
		void setAuthor(const std::string& author);
		const std::string& author() const;

		//! RI of the author of a resource.
		void setAuthorURI(const std::string& authorURI);
		const std::string& authorURI() const;

		//! Time of creation of a resource, in ISO 8601 format. It has
		//! to be given in UTC.
		void setCreationTime(const OPT(Seiscomp::Core::Time)& creationTime);
		Seiscomp::Core::Time creationTime() const;

		//! Time of last modification of a resource, in ISO 8601
		//! format. It has to be given in UTC.
		void setModificationTime(const OPT(Seiscomp::Core::Time)& modificationTime);
		Seiscomp::Core::Time modificationTime() const;

		//! Version string of a resource.
		void setVersion(const std::string& version);
		const std::string& version() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _agencyID;
		std::string _agencyURI;
		std::string _author;
		std::string _authorURI;
		OPT(Seiscomp::Core::Time) _creationTime;
		OPT(Seiscomp::Core::Time) _modificationTime;
		std::string _version;
};


}
}


#endif
