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


#ifndef SEISCOMP_PROCESSING_SENSOR_H
#define SEISCOMP_PROCESSING_SENSOR_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/processing/response.h>
#include <seiscomp/client.h>


namespace Seiscomp {
namespace Processing  {


DEFINE_SMARTPOINTER(Sensor);

class SC_SYSTEM_CLIENT_API Sensor : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Sensor();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setModel(const std::string& model);
		const std::string& model() const;

		void setManufacturer(const std::string& manufacturer);
		const std::string& manufacturer() const;

		void setType(const std::string& type);
		const std::string& type() const;

		void setUnit(const std::string& unit);
		const std::string& unit() const;

		void setLowFrequency(const OPT(double)& lowFrequency);
		double lowFrequency() const;

		void setHighFrequency(const OPT(double)& highFrequency);
		double highFrequency() const;

		Response *response() const;
		void setResponse(Response *response);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		ResponsePtr _response;

		std::string _model;
		std::string _manufacturer;
		std::string _type;
		std::string _unit;

		OPT(double) _lowFrequency;
		OPT(double) _highFrequency;
};


}
}

#endif
