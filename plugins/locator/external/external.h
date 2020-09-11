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


#ifndef SEISCOMP_SEISMOLOGY_LOCATOR_EXTERNAL_H
#define SEISCOMP_SEISMOLOGY_LOCATOR_EXTERNAL_H


#include <seiscomp/core/exceptions.h>
#include <seiscomp/seismology/locatorinterface.h>

#include <vector>


using namespace Seiscomp;
using namespace Seiscomp::Seismology;


namespace {


class ExternalLocator : public LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		ExternalLocator();


	// ----------------------------------------------------------------------
	//  Locator interface
	// ----------------------------------------------------------------------
	public:
		virtual bool init(const Config::Config &config) override;

		virtual IDList profiles() const override;
		virtual void setProfile(const std::string &name) override;

		virtual int capabilities() const override;

		virtual DataModel::Origin *locate(PickList &pickList) override;
		virtual DataModel::Origin *locate(PickList &pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  const Seiscomp::Core::Time& initTime) override;

		virtual DataModel::Origin *relocate(const DataModel::Origin *origin) override;


	private:
		// Configuration
		IDList                 _profiles;
		ParameterMap           _scripts;
		ParameterMap::iterator _currentScript;
};


}


#endif
