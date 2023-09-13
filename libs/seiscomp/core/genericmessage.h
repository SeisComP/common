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


#ifndef SEISCOMP_CORE_GENERICMESSAGE_H
#define SEISCOMP_CORE_GENERICMESSAGE_H


#include <seiscomp/core/message.h>
#include <algorithm>
#include <list>


namespace Seiscomp {
namespace Core {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
class GenericMessage : public ::Seiscomp::Core::Message {
	// ----------------------------------------------------------------------
	//  Public Types
	// ----------------------------------------------------------------------
	public:
		typedef T AttachementType;
		typedef typename std::list<typename Seiscomp::Core::SmartPointer<T>::Impl> AttachementList;
		typedef typename AttachementList::iterator iterator;
		typedef typename AttachementList::const_iterator const_iterator;

	DECLARE_SERIALIZATION;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		GenericMessage();

		//! Destructor
		~GenericMessage();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * Attaches an object to the message
		 * @param  attachment A pointer to the object
		 * @retval true The operation was successfull and the object has been attached properly
		 * @retval false The object is nullptr or the object has been attached already
		 */
		bool attach(AttachementType* attachment);
		bool attach(typename Seiscomp::Core::SmartPointer<AttachementType>::Impl& attachment);

		/**
		 * Detaches an already attached object from the message
		 * @param  object Pointer to an object in the messagebody
		 * @retval true The object has been detached successfully
		 * @retval false The object has not been attached before
		 */
		bool detach(AttachementType* attachment);
		bool detach(typename Seiscomp::Core::SmartPointer<AttachementType>::Impl& attachment);

		/**
		 * Detaches an object from the message
		 * @param it The iterator pointing to the object
		 * @retval true The object has been detached successfully
		 * @retval false The iterator is invalid
		 */
		iterator detach(iterator it);

		//! Removes all attachments from the message
		void clear();

		//! Returns the iterators for begin and end of
		//! the attachment list
		iterator begin();
		const_iterator begin() const;

		iterator end();
		const_iterator end() const;

		//! Implemented from baseclass
		bool empty() const;

		/**
		 * @return Returns the number of objects attached to a message
		 */
		int size() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		MessageIterator::Impl* iterImpl() const;

	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	protected:
		AttachementList _attachments;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DEFINE_MESSAGE_FOR(CLASS, TYPENAME, APIDef) \
	class APIDef TYPENAME : public ::Seiscomp::Core::GenericMessage<CLASS> { \
		DECLARE_SC_CLASS(TYPENAME); \
	}; \
	typedef ::Seiscomp::Core::SmartPointer<TYPENAME>::Impl TYPENAME##Ptr

#define IMPLEMENT_MESSAGE_FOR(CLASS, TYPENAME, NAME) \
	IMPLEMENT_SC_CLASS_DERIVED(TYPENAME, ::Seiscomp::Core::Message, NAME)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include <seiscomp/core/genericmessage.ipp>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
