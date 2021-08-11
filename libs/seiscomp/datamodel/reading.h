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


#ifndef SEISCOMP_DATAMODEL_READING_H
#define SEISCOMP_DATAMODEL_READING_H


#include <vector>
#include <seiscomp/datamodel/pickreference.h>
#include <seiscomp/datamodel/amplitudereference.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Reading);
DEFINE_SMARTPOINTER(PickReference);
DEFINE_SMARTPOINTER(AmplitudeReference);

class EventParameters;


/**
 * \brief This class groups Pick and Amplitude elements which are
 * \brief thought to belong
 * \brief to the same event, but for which the event identification
 * \brief is not known.
 */
class SC_SYSTEM_CORE_API Reading : public PublicObject {
	DECLARE_SC_CLASS(Reading)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Reading();

	public:
		//! Copy constructor
		Reading(const Reading& other);

		//! Constructor with publicID
		Reading(const std::string& publicID);

		//! Destructor
		~Reading() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Reading* Create();
		static Reading* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Reading* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Reading& operator=(const Reading& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Reading& other) const;
		bool operator!=(const Reading& other) const;

		//! Wrapper that calls operator==
		bool equal(const Reading& other) const;

	
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
		bool add(PickReference* obj);
		bool add(AmplitudeReference* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(PickReference* obj);
		bool remove(AmplitudeReference* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removePickReference(size_t i);
		bool removePickReference(const PickReferenceIndex& i);
		bool removeAmplitudeReference(size_t i);
		bool removeAmplitudeReference(const AmplitudeReferenceIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t pickReferenceCount() const;
		size_t amplitudeReferenceCount() const;

		//! Index access
		//! @return The object at index i
		PickReference* pickReference(size_t i) const;
		PickReference* pickReference(const PickReferenceIndex& i) const;

		AmplitudeReference* amplitudeReference(size_t i) const;
		AmplitudeReference* amplitudeReference(const AmplitudeReferenceIndex& i) const;

		//! Find an object by its unique attribute(s)

		EventParameters* eventParameters() const;

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
		// Aggregations
		std::vector<PickReferencePtr> _pickReferences;
		std::vector<AmplitudeReferencePtr> _amplitudeReferences;

	DECLARE_SC_CLASSFACTORY_FRIEND(Reading);
};


}
}


#endif
