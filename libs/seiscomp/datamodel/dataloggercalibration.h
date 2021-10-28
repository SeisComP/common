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


#ifndef SEISCOMP_DATAMODEL_DATALOGGERCALIBRATION_H
#define SEISCOMP_DATAMODEL_DATALOGGERCALIBRATION_H


#include <string>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/blob.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataloggerCalibration);

class Datalogger;


class SC_SYSTEM_CORE_API DataloggerCalibrationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataloggerCalibrationIndex();
		DataloggerCalibrationIndex(const std::string& serialNumber,
		                           int channel,
		                           Seiscomp::Core::Time start);

		//! Copy constructor
		DataloggerCalibrationIndex(const DataloggerCalibrationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DataloggerCalibrationIndex&) const;
		bool operator!=(const DataloggerCalibrationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string serialNumber;
		int channel;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a datalogger calibration
 */
class SC_SYSTEM_CORE_API DataloggerCalibration : public Object {
	DECLARE_SC_CLASS(DataloggerCalibration)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataloggerCalibration();

		//! Copy constructor
		DataloggerCalibration(const DataloggerCalibration& other);

		//! Destructor
		~DataloggerCalibration() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DataloggerCalibration& operator=(const DataloggerCalibration& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const DataloggerCalibration& other) const;
		bool operator!=(const DataloggerCalibration& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataloggerCalibration& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Referred from network/station/Stream/@dataloggerSerialNumber
		void setSerialNumber(const std::string& serialNumber);
		const std::string& serialNumber() const;

		//! Referred from network/station/Stream/@dataloggerChannel
		void setChannel(int channel);
		int channel() const;

		//! Start of validity
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of validity
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Overrides nominal gain of calibrated datalogger
		//! (48.05/58.04)
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
		const DataloggerCalibrationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const DataloggerCalibration* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Datalogger* datalogger() const;

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
		DataloggerCalibrationIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(Blob) _remark;
};


}
}


#endif
