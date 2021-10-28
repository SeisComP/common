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


#ifndef SEISCOMP_DATAMODEL_SENSORCALIBRATION_H
#define SEISCOMP_DATAMODEL_SENSORCALIBRATION_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/blob.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(SensorCalibration);

class Sensor;


class SC_SYSTEM_CORE_API SensorCalibrationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorCalibrationIndex();
		SensorCalibrationIndex(const std::string& serialNumber,
		                       int channel,
		                       Seiscomp::Core::Time start);

		//! Copy constructor
		SensorCalibrationIndex(const SensorCalibrationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SensorCalibrationIndex&) const;
		bool operator!=(const SensorCalibrationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string serialNumber;
		int channel;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a sensor calibration
 */
class SC_SYSTEM_CORE_API SensorCalibration : public Object {
	DECLARE_SC_CLASS(SensorCalibration)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorCalibration();

		//! Copy constructor
		SensorCalibration(const SensorCalibration& other);

		//! Destructor
		~SensorCalibration() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SensorCalibration& operator=(const SensorCalibration& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const SensorCalibration& other) const;
		bool operator!=(const SensorCalibration& other) const;

		//! Wrapper that calls operator==
		bool equal(const SensorCalibration& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Referred from network/station/Stream/@sensorSerialNumber
		void setSerialNumber(const std::string& serialNumber);
		const std::string& serialNumber() const;

		//! Referred from network/station/Stream/@sensorChannel
		void setChannel(int channel);
		int channel() const;

		//! Start of validity
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of validity
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Overrides nominal gain of calibrated sensor (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SensorCalibrationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const SensorCalibration* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Sensor* sensor() const;

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
		SensorCalibrationIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(Blob) _remark;
};


}
}


#endif
