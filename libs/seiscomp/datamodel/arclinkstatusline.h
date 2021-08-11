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


#ifndef SEISCOMP_DATAMODEL_ARCLINKSTATUSLINE_H
#define SEISCOMP_DATAMODEL_ARCLINKSTATUSLINE_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkStatusLine);

class ArclinkRequest;


class SC_SYSTEM_CORE_API ArclinkStatusLineIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkStatusLineIndex();
		ArclinkStatusLineIndex(const std::string& volumeID,
		                       const std::string& type,
		                       const std::string& status);

		//! Copy constructor
		ArclinkStatusLineIndex(const ArclinkStatusLineIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkStatusLineIndex&) const;
		bool operator!=(const ArclinkStatusLineIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string volumeID;
		std::string type;
		std::string status;
};


class SC_SYSTEM_CORE_API ArclinkStatusLine : public Object {
	DECLARE_SC_CLASS(ArclinkStatusLine)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkStatusLine();

		//! Copy constructor
		ArclinkStatusLine(const ArclinkStatusLine& other);

		//! Destructor
		~ArclinkStatusLine() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ArclinkStatusLine& operator=(const ArclinkStatusLine& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ArclinkStatusLine& other) const;
		bool operator!=(const ArclinkStatusLine& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkStatusLine& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setType(const std::string& type);
		const std::string& type() const;

		void setStatus(const std::string& status);
		const std::string& status() const;

		void setSize(const OPT(int)& size);
		int size() const;

		void setMessage(const std::string& message);
		const std::string& message() const;

		void setVolumeID(const std::string& volumeID);
		const std::string& volumeID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkStatusLineIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkStatusLine* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ArclinkRequest* arclinkRequest() const;

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
		ArclinkStatusLineIndex _index;

		// Attributes
		OPT(int) _size;
		std::string _message;
};


}
}


#endif
