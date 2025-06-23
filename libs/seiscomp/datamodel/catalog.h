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


#ifndef SEISCOMP_DATAMODEL_CATALOG_H
#define SEISCOMP_DATAMODEL_CATALOG_H


#include <string>
#include <seiscomp/datamodel/creationinfo.h>
#include <seiscomp/core/datetime.h>
#include <vector>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Catalog);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(Event);

class EventParameters;


class SC_SYSTEM_CORE_API Catalog : public PublicObject {
	DECLARE_SC_CLASS(Catalog)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Catalog();

	public:
		//! Copy constructor
		Catalog(const Catalog &other);

		//! Constructor with publicID
		Catalog(const std::string& publicID);

		//! Destructor
		~Catalog() override;


	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Catalog *Create();
		static Catalog *Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Catalog *Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Catalog &operator=(const Catalog &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Catalog &other) const;
		bool operator!=(const Catalog &other) const;

		//! Wrapper that calls operator==
		bool equal(const Catalog &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		//! Catalog description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! CreationInfo for the Catalog object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;

		//! Start of epoch in ISO datetime format
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of epoch (empty if the catalog epoch is open)
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		void setDynamic(bool dynamic);
		bool dynamic() const;


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
		bool add(Comment *obj);
		bool add(Event *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment *obj);
		bool remove(Event *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex &i);
		bool removeEvent(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t eventCount() const;

		//! Index access
		//! @return The object at index i
		Comment *comment(size_t i) const;
		Comment *comment(const CommentIndex &i) const;
		Event *event(size_t i) const;

		//! Find an object by its unique attribute(s)
		Event *findEvent(const std::string& publicID) const;

		EventParameters *eventParameters() const;

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
		// Attributes
		std::string _name;
		std::string _description;
		OPT(CreationInfo) _creationInfo;
		Seiscomp::Core::Time _start;
		OPT(Seiscomp::Core::Time) _end;
		bool _dynamic;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<EventPtr> _events;

	DECLARE_SC_CLASSFACTORY_FRIEND(Catalog);
};


}
}


#endif
