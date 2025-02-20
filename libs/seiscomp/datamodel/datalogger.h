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


#ifndef SEISCOMP_DATAMODEL_DATALOGGER_H
#define SEISCOMP_DATAMODEL_DATALOGGER_H


#include <string>
#include <seiscomp/datamodel/blob.h>
#include <vector>
#include <seiscomp/datamodel/dataloggercalibration.h>
#include <seiscomp/datamodel/decimation.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Datalogger);
DEFINE_SMARTPOINTER(DataloggerCalibration);
DEFINE_SMARTPOINTER(Decimation);

class Inventory;


class SC_SYSTEM_CORE_API DataloggerIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataloggerIndex();
		DataloggerIndex(const std::string& name);

		//! Copy constructor
		DataloggerIndex(const DataloggerIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DataloggerIndex&) const;
		bool operator!=(const DataloggerIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a datalogger (digitizer and recorder)
 */
class SC_SYSTEM_CORE_API Datalogger : public PublicObject {
	DECLARE_SC_CLASS(Datalogger)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Datalogger();

	public:
		//! Copy constructor
		Datalogger(const Datalogger &other);

		//! Constructor with publicID
		Datalogger(const std::string& publicID);

		//! Destructor
		~Datalogger() override;


	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Datalogger *Create();
		static Datalogger *Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Datalogger *Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Datalogger &operator=(const Datalogger &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Datalogger &other) const;
		bool operator!=(const Datalogger &other) const;

		//! Wrapper that calls operator==
		bool equal(const Datalogger &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique datalogger name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Datalogger description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! Digitizer model
		void setDigitizerModel(const std::string& digitizerModel);
		const std::string& digitizerModel() const;

		//! Digitizer manufacturer
		void setDigitizerManufacturer(const std::string& digitizerManufacturer);
		const std::string& digitizerManufacturer() const;

		//! Recorder model
		void setRecorderModel(const std::string& recorderModel);
		const std::string& recorderModel() const;

		//! Recorder manufacturer
		void setRecorderManufacturer(const std::string& recorderManufacturer);
		const std::string& recorderManufacturer() const;

		//! Clock model (mostly unused)
		void setClockModel(const std::string& clockModel);
		const std::string& clockModel() const;

		//! Clock manufacturer (mostly unused)
		void setClockManufacturer(const std::string& clockManufacturer);
		const std::string& clockManufacturer() const;

		//! Clock type (mostly unused)
		void setClockType(const std::string& clockType);
		const std::string& clockType() const;

		//! Sensitivity of digitizer, counts/V (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Max clock drift, seconds/second (not identical to 52.19,
		//! which is seconds/sample)
		void setMaxClockDrift(const OPT(double)& maxClockDrift);
		double maxClockDrift() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DataloggerIndex &index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Datalogger *lhs) const;


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
		bool add(DataloggerCalibration *obj);
		bool add(Decimation *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(DataloggerCalibration *obj);
		bool remove(Decimation *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeDataloggerCalibration(size_t i);
		bool removeDataloggerCalibration(const DataloggerCalibrationIndex &i);
		bool removeDecimation(size_t i);
		bool removeDecimation(const DecimationIndex &i);

		//! Retrieve the number of objects of a particular class
		size_t dataloggerCalibrationCount() const;
		size_t decimationCount() const;

		//! Index access
		//! @return The object at index i
		DataloggerCalibration *dataloggerCalibration(size_t i) const;
		DataloggerCalibration *dataloggerCalibration(const DataloggerCalibrationIndex &i) const;

		Decimation *decimation(size_t i) const;
		Decimation *decimation(const DecimationIndex &i) const;

		//! Find an object by its unique attribute(s)

		Inventory *inventory() const;

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
		DataloggerIndex _index;

		// Attributes
		std::string _description;
		std::string _digitizerModel;
		std::string _digitizerManufacturer;
		std::string _recorderModel;
		std::string _recorderManufacturer;
		std::string _clockModel;
		std::string _clockManufacturer;
		std::string _clockType;
		OPT(double) _gain;
		OPT(double) _maxClockDrift;
		OPT(Blob) _remark;

		// Aggregations
		std::vector<DataloggerCalibrationPtr> _dataloggerCalibrations;
		std::vector<DecimationPtr> _decimations;

	DECLARE_SC_CLASSFACTORY_FRIEND(Datalogger);
};


}
}


#endif
