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


#ifndef SEISCOMP_SEISMOLOGY_FIXED_HYPOCENTER_H
#define SEISCOMP_SEISMOLOGY_FIXED_HYPOCENTER_H


#include <seiscomp/core/exceptions.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/seismology/locatorinterface.h>
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/core.h>

#include <vector>


namespace Seiscomp{
namespace Seismology {


class SC_SYSTEM_CORE_API FixedHypocenter : public LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		FixedHypocenter();


	// ----------------------------------------------------------------------
	//  Locator interface
	// ----------------------------------------------------------------------
	public:
		virtual bool init(const Config::Config &config) override;

		//! Returns supported parameters to be changed.
		virtual IDList parameters() const override;

		//! Returns the value of a parameter.
		virtual std::string parameter(const std::string &name) const override;

		//! Sets the value of a parameter.
		virtual bool setParameter(const std::string &name,
		                          const std::string &value) override;

		virtual IDList profiles() const override;
		virtual void setProfile(const std::string &name) override;

		virtual int capabilities() const override;

		virtual DataModel::Origin *locate(PickList& pickList) override;
		virtual DataModel::Origin *locate(PickList& pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  const Seiscomp::Core::Time& initTime) override;

		virtual DataModel::Origin *relocate(const DataModel::Origin* origin) override;


	private:
		// Configuration
		IDList      _profiles;
		int         _degreesOfFreedom;
		double      _confidenceLevel;
		double      _defaultTimeError;
		bool        _usePickUncertainties;
		bool        _verbose;
		std::string _lastError;

		// Runtime
		TravelTimeTableInterfacePtr _ttt;
};


}
}


#endif
