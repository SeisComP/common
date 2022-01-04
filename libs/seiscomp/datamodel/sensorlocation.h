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


#ifndef SEISCOMP_DATAMODEL_SENSORLOCATION_H
#define SEISCOMP_DATAMODEL_SENSORLOCATION_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <vector>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/auxstream.h>
#include <seiscomp/datamodel/stream.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(SensorLocation);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(AuxStream);
DEFINE_SMARTPOINTER(Stream);

class Station;


class SC_SYSTEM_CORE_API SensorLocationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorLocationIndex();
		SensorLocationIndex(const std::string& code,
		                    Seiscomp::Core::Time start);

		//! Copy constructor
		SensorLocationIndex(const SensorLocationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SensorLocationIndex&) const;
		bool operator!=(const SensorLocationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a sensor location
 */
class SC_SYSTEM_CORE_API SensorLocation : public PublicObject {
	DECLARE_SC_CLASS(SensorLocation)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		SensorLocation();

	public:
		//! Copy constructor
		SensorLocation(const SensorLocation& other);

		//! Constructor with publicID
		SensorLocation(const std::string& publicID);

		//! Destructor
		~SensorLocation() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static SensorLocation* Create();
		static SensorLocation* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static SensorLocation* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		SensorLocation& operator=(const SensorLocation& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const SensorLocation& other) const;
		bool operator!=(const SensorLocation& other) const;

		//! Wrapper that calls operator==
		bool equal(const SensorLocation& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Station code (52.03)
		void setCode(const std::string& code);
		const std::string& code() const;

		//! Start of epoch in ISO datetime format
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of epoch
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Sensor latitude (52.10) with respect to the World Geodetic
		//! System
		//! 1984 (WGS84) reference system (National Imagery and Mapping
		//! Agency
		//! 2000) in degrees.
		void setLatitude(const OPT(double)& latitude);
		double latitude() const;

		//! Sensor longitude (52.11) with respect to the World Geodetic
		//! System
		//! 1984 (WGS84) reference system (National Imagery and Mapping
		//! Agency
		//! 2000) in degrees.
		void setLongitude(const OPT(double)& longitude);
		double longitude() const;

		//! Sensor elevation (52.12) with respect to the World Geodetic
		//! System
		//! 1984 (WGS84) reference system (National Imagery and Mapping
		//! Agency
		//! 2000) in meters.
		void setElevation(const OPT(double)& elevation);
		double elevation() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SensorLocationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const SensorLocation* lhs) const;

	
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
		bool add(Comment* obj);
		bool add(AuxStream* obj);
		bool add(Stream* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);
		bool remove(AuxStream* obj);
		bool remove(Stream* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);
		bool removeAuxStream(size_t i);
		bool removeAuxStream(const AuxStreamIndex& i);
		bool removeStream(size_t i);
		bool removeStream(const StreamIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t auxStreamCount() const;
		size_t streamCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;

		AuxStream* auxStream(size_t i) const;
		AuxStream* auxStream(const AuxStreamIndex& i) const;

		Stream* stream(size_t i) const;
		Stream* stream(const StreamIndex& i) const;

		//! Find an object by its unique attribute(s)
		Stream* findStream(const std::string& publicID) const;

		Station* station() const;

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
		SensorLocationIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(double) _latitude;
		OPT(double) _longitude;
		OPT(double) _elevation;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<AuxStreamPtr> _auxStreams;
		std::vector<StreamPtr> _streams;

	DECLARE_SC_CLASSFACTORY_FRIEND(SensorLocation);
};


}
}


#endif
