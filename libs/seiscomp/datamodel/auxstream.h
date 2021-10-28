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


#ifndef SEISCOMP_DATAMODEL_AUXSTREAM_H
#define SEISCOMP_DATAMODEL_AUXSTREAM_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(AuxStream);

class SensorLocation;


class SC_SYSTEM_CORE_API AuxStreamIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxStreamIndex();
		AuxStreamIndex(const std::string& code,
		               Seiscomp::Core::Time start);

		//! Copy constructor
		AuxStreamIndex(const AuxStreamIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AuxStreamIndex&) const;
		bool operator!=(const AuxStreamIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a stream (channel) without defined
 * \brief frequency response
 */
class SC_SYSTEM_CORE_API AuxStream : public Object {
	DECLARE_SC_CLASS(AuxStream)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxStream();

		//! Copy constructor
		AuxStream(const AuxStream& other);

		//! Destructor
		~AuxStream() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AuxStream& operator=(const AuxStream& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const AuxStream& other) const;
		bool operator!=(const AuxStream& other) const;

		//! Wrapper that calls operator==
		bool equal(const AuxStream& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Stream code (52.04)
		void setCode(const std::string& code);
		const std::string& code() const;

		//! Start of epoch in ISO datetime format (52.22)
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of epoch (52.23)
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Reference to auxDevice/@publicID
		void setDevice(const std::string& device);
		const std::string& device() const;

		//! Serial number of device
		void setDeviceSerialNumber(const std::string& deviceSerialNumber);
		const std::string& deviceSerialNumber() const;

		//! Reference to auxSource/@name
		void setSource(const std::string& source);
		const std::string& source() const;

		//! Data format, eg.: "steim1", "steim2", "mseedN" (N =
		//! encoding format in blockette 1000)
		void setFormat(const std::string& format);
		const std::string& format() const;

		//! Channel flags (52.21)
		void setFlags(const std::string& flags);
		const std::string& flags() const;

		//! Whether the stream is "restricted"
		void setRestricted(const OPT(bool)& restricted);
		bool restricted() const;

		//! Whether the metadata is synchronized with other datacenters
		void setShared(const OPT(bool)& shared);
		bool shared() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AuxStreamIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const AuxStream* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		SensorLocation* sensorLocation() const;

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
		AuxStreamIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		std::string _device;
		std::string _deviceSerialNumber;
		std::string _source;
		std::string _format;
		std::string _flags;
		OPT(bool) _restricted;
		OPT(bool) _shared;
};


}
}


#endif
