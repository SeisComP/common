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


#ifndef SEISCOMP_PROCESSING_PROCESSOR_H
#define SEISCOMP_PROCESSING_PROCESSOR_H


#include <seiscomp/config/exceptions.h>
#include <seiscomp/utils/keyvalues.h>
#include <seiscomp/client.h>


namespace Seiscomp {

namespace Config {
	class Config;
}

namespace Processing {


struct SC_SYSTEM_CLIENT_API Settings {
	Settings(const std::string &module,
	         const std::string &network,
	         const std::string &station,
	         const std::string &location,
	         const std::string &stream,
	         const Config::Config *config,
	         const Util::KeyValues *keys);

	//! Returns a parameter value for a station. The first
	//! lookup is in the global application configuration
	//! with name "key.module.network.station.parameter.
	//! If it is not found it tries to lookup the value in
	//! keyParameters. If no value is found an exception
	//! is thrown otherwise the value is returned.
	std::string getString(const std::string &parameter) const;

	int getInt(const std::string &parameter) const;

	double getDouble(const std::string &parameter) const;

	bool getBool(const std::string &parameter) const;

	//! Set the parameter value for a station. The first
	//! lookup is in the global application configuration
	//! with name "key.module.network.station.parameter.
	//! If it is not found it tries to lookup the value in
	//! keyParameters. If no value is found, false is returned,
	//! true otherwise
	bool getValue(std::string &value, const std::string &parameter) const;
	bool getValue(int &value, const std::string &parameter) const;
	bool getValue(double &value, const std::string &parameter) const;
	bool getValue(bool &value, const std::string &parameter) const;

	const std::string     &module;
	const std::string     &networkCode;
	const std::string     &stationCode;
	const std::string     &locationCode;
	const std::string     &channelCode;
	const Config::Config  *localConfiguration;
	const Util::KeyValues *keyParameters;
};


DEFINE_SMARTPOINTER(Processor);


class SC_SYSTEM_CLIENT_API Processor : public Core::BaseObject {
	DECLARE_SC_CLASS(Processor)

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Processor();

		//! D'tor
		virtual ~Processor();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! This method can be called to initialize the processor.
		//! 'parameters' contains simple name-value pairs (strings).
		//! The default implementation does nothing.
		virtual bool setup(const Settings &settings);
};


}
}


#endif
