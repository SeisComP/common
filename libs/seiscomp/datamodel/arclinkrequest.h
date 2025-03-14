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


#ifndef SEISCOMP_DATAMODEL_ARCLINKREQUEST_H
#define SEISCOMP_DATAMODEL_ARCLINKREQUEST_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/arclinkrequestsummary.h>
#include <vector>
#include <seiscomp/datamodel/arclinkstatusline.h>
#include <seiscomp/datamodel/arclinkrequestline.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkRequest);
DEFINE_SMARTPOINTER(ArclinkStatusLine);
DEFINE_SMARTPOINTER(ArclinkRequestLine);

class ArclinkLog;


class SC_SYSTEM_CORE_API ArclinkRequestIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestIndex();
		ArclinkRequestIndex(Seiscomp::Core::Time created,
		                    const std::string& requestID,
		                    const std::string& userID);

		//! Copy constructor
		ArclinkRequestIndex(const ArclinkRequestIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkRequestIndex&) const;
		bool operator!=(const ArclinkRequestIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time created;
		std::string requestID;
		std::string userID;
};


class SC_SYSTEM_CORE_API ArclinkRequest : public PublicObject {
	DECLARE_SC_CLASS(ArclinkRequest)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ArclinkRequest();

	public:
		//! Copy constructor
		ArclinkRequest(const ArclinkRequest &other);

		//! Constructor with publicID
		ArclinkRequest(const std::string& publicID);

		//! Destructor
		~ArclinkRequest() override;


	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ArclinkRequest *Create();
		static ArclinkRequest *Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ArclinkRequest *Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ArclinkRequest &operator=(const ArclinkRequest &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ArclinkRequest &other) const;
		bool operator!=(const ArclinkRequest &other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkRequest &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setRequestID(const std::string& requestID);
		const std::string& requestID() const;

		void setUserID(const std::string& userID);
		const std::string& userID() const;

		void setUserIP(const std::string& userIP);
		const std::string& userIP() const;

		void setClientID(const std::string& clientID);
		const std::string& clientID() const;

		void setClientIP(const std::string& clientIP);
		const std::string& clientIP() const;

		void setType(const std::string& type);
		const std::string& type() const;

		void setCreated(Seiscomp::Core::Time created);
		Seiscomp::Core::Time created() const;

		void setStatus(const std::string& status);
		const std::string& status() const;

		void setMessage(const std::string& message);
		const std::string& message() const;

		void setLabel(const std::string& label);
		const std::string& label() const;

		void setHeader(const std::string& header);
		const std::string& header() const;

		void setSummary(const OPT(ArclinkRequestSummary)& summary);
		ArclinkRequestSummary& summary();
		const ArclinkRequestSummary& summary() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkRequestIndex &index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkRequest *lhs) const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(ArclinkStatusLine *obj);
		bool add(ArclinkRequestLine *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(ArclinkStatusLine *obj);
		bool remove(ArclinkRequestLine *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeArclinkStatusLine(size_t i);
		bool removeArclinkStatusLine(const ArclinkStatusLineIndex &i);
		bool removeArclinkRequestLine(size_t i);
		bool removeArclinkRequestLine(const ArclinkRequestLineIndex &i);

		//! Retrieve the number of objects of a particular class
		size_t arclinkStatusLineCount() const;
		size_t arclinkRequestLineCount() const;

		//! Index access
		//! @return The object at index i
		ArclinkStatusLine *arclinkStatusLine(size_t i) const;
		ArclinkStatusLine *arclinkStatusLine(const ArclinkStatusLineIndex &i) const;

		ArclinkRequestLine *arclinkRequestLine(size_t i) const;
		ArclinkRequestLine *arclinkRequestLine(const ArclinkRequestLineIndex &i) const;

		//! Find an object by its unique attribute(s)

		ArclinkLog *arclinkLog() const;

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
		ArclinkRequestIndex _index;

		// Attributes
		std::string _userIP;
		std::string _clientID;
		std::string _clientIP;
		std::string _type;
		std::string _status;
		std::string _message;
		std::string _label;
		std::string _header;
		OPT(ArclinkRequestSummary) _summary;

		// Aggregations
		std::vector<ArclinkStatusLinePtr> _arclinkStatusLines;
		std::vector<ArclinkRequestLinePtr> _arclinkRequestLines;

	DECLARE_SC_CLASSFACTORY_FRIEND(ArclinkRequest);
};


}
}


#endif
