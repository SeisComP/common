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


#ifndef SEISCOMP_DATAMODEL_ARCLINKREQUESTLINE_H
#define SEISCOMP_DATAMODEL_ARCLINKREQUESTLINE_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/waveformstreamid.h>
#include <string>
#include <seiscomp/datamodel/arclinkstatusline.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkRequestLine);

class ArclinkRequest;


class SC_SYSTEM_CORE_API ArclinkRequestLineIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestLineIndex();
		ArclinkRequestLineIndex(Seiscomp::Core::Time start,
		                        Seiscomp::Core::Time end,
		                        const WaveformStreamID& streamID);

		//! Copy constructor
		ArclinkRequestLineIndex(const ArclinkRequestLineIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkRequestLineIndex&) const;
		bool operator!=(const ArclinkRequestLineIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
		Seiscomp::Core::Time end;
		WaveformStreamID streamID;
};


class SC_SYSTEM_CORE_API ArclinkRequestLine : public Object {
	DECLARE_SC_CLASS(ArclinkRequestLine)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestLine();

		//! Copy constructor
		ArclinkRequestLine(const ArclinkRequestLine& other);

		//! Destructor
		~ArclinkRequestLine() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ArclinkRequestLine& operator=(const ArclinkRequestLine& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ArclinkRequestLine& other) const;
		bool operator!=(const ArclinkRequestLine& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkRequestLine& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		void setStreamID(const WaveformStreamID& streamID);
		WaveformStreamID& streamID();
		const WaveformStreamID& streamID() const;

		void setRestricted(const OPT(bool)& restricted);
		bool restricted() const;

		void setShared(const OPT(bool)& shared);
		bool shared() const;

		void setNetClass(const std::string& netClass);
		const std::string& netClass() const;

		void setConstraints(const std::string& constraints);
		const std::string& constraints() const;

		void setStatus(const ArclinkStatusLine& status);
		ArclinkStatusLine& status();
		ArclinkStatusLine status() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkRequestLineIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkRequestLine* lhs) const;

	
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
		ArclinkRequestLineIndex _index;

		// Attributes
		OPT(bool) _restricted;
		OPT(bool) _shared;
		std::string _netClass;
		std::string _constraints;
		ArclinkStatusLine _status;
};


}
}


#endif
