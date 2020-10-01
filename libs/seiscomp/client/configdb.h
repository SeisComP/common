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


#ifndef SEISCOMP_CLIENT_CONFIG_H
#define SEISCOMP_CLIENT_CONFIG_H


#include <seiscomp/datamodel/config.h>
#include <seiscomp/datamodel/configmodule.h>
#include <seiscomp/datamodel/configstation.h>
#include <seiscomp/datamodel/parameterset.h>
#include <seiscomp/datamodel/databasereader.h>
#include <seiscomp/client.h>

#include <map>
#include <set>


namespace Seiscomp {
namespace Client {


class SC_SYSTEM_CLIENT_API ConfigDB {
	private:
		ConfigDB();

	public:
		static ConfigDB* Instance();

		void load(Seiscomp::DataModel::DatabaseReader* reader,
			const OPT(std::string)& moduleName = Seiscomp::Core::None,
			const OPT(std::string)& networkCode = Seiscomp::Core::None,
			const OPT(std::string)& stationCode = Seiscomp::Core::None,
			const OPT(std::string)& setupName = Seiscomp::Core::None,
			const std::set<std::string>& parameterNames = std::set<std::string>());

		void load(const char *xml);

		Seiscomp::DataModel::Config* config();

	private:
		std::map<int, Seiscomp::DataModel::ConfigModulePtr> _configModules;
		std::map<int, Seiscomp::DataModel::ConfigStationPtr> _configStations;
		std::map<int, Seiscomp::DataModel::ParameterSetPtr> _parameterSets;
		Seiscomp::DataModel::ConfigPtr _config;
		static ConfigDB *_instance;

	Seiscomp::DataModel::DatabaseIterator getConfigObjects(Seiscomp::DataModel::DatabaseReader* reader,
		const Seiscomp::Core::RTTI& classType,
		const OPT(std::string)& moduleName,
		const OPT(std::string)& networkCode,
		const OPT(std::string)& stationCode,
		const OPT(std::string)& setupName,
		const std::set<std::string>& parameterNames);
};


}
}


#endif

