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


#ifndef SEISCOMP_PROCESSING_STREAM_H
#define SEISCOMP_PROCESSING_STREAM_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/processing/sensor.h>


namespace Seiscomp {

namespace DataModel {

class Stream;

}

namespace Processing  {


DEFINE_SMARTPOINTER(Stream);

class SC_SYSTEM_CLIENT_API Stream : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Stream();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Initialized the stream meta data from the inventory
		//! if available
		void init(const std::string &networkCode,
		          const std::string &stationCode,
		          const std::string &locationCode,
		          const std::string &channelCode,
		          const Core::Time &time);

		void init(const DataModel::Stream *stream);

		void setCode(const std::string &code);
		const std::string &code() const;

		Sensor *sensor() const;
		void setSensor(Sensor *sensor);

		//! Applies the gain the data.
		void applyGain(int n, double *inout);
		void applyGain(DoubleArray &inout);

		//! Removes the gain the data.
		void removeGain(int n, double *inout);
		void removeGain(DoubleArray &inout);


	public:
		Core::TimeWindow epoch;

		double      gain;
		OPT(double) gainFrequency;
		std::string gainUnit;
		double      azimuth;
		double      dip;


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		// Sensor information
		SensorPtr   _sensor;

		// Stream code
		std::string _code;
};


}
}

#endif
