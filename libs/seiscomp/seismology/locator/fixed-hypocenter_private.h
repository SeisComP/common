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

#include <cstdint>
#include <vector>


namespace {


using namespace Seiscomp;
using namespace Seiscomp::Seismology;


class SC_SYSTEM_CORE_API FixedHypocenter : public LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		FixedHypocenter() = default;


	// ----------------------------------------------------------------------
	//  Locator interface
	// ----------------------------------------------------------------------
	public:
		bool init(const Config::Config &config) override;

		//! Returns supported parameters to be changed.
		IDList parameters() const override;

		//! Returns the value of a parameter.
		std::string parameter(const std::string &name) const override;

		//! Sets the value of a parameter.
		bool setParameter(const std::string &name,
		                  const std::string &value) override;

		IDList profiles() const override;
		void setProfile(const std::string &name) override;

		int capabilities() const override;

		DataModel::Origin *locate(PickList& pickList) override;
		DataModel::Origin *locate(PickList& pickList,
		                          double initLat, double initLon, double initDepth,
		                          const Core::Time& initTime) override;

		virtual DataModel::Origin *relocate(const DataModel::Origin* origin) override;


	private:
		enum Flag {
			UsePickUncertainties   = 0x01,
			UseOriginUncertainties = 0x02
		};

		// Configuration
		IDList          _profiles;
		int             _degreesOfFreedom{8};
		double          _confidenceLevel{0.9};
		double          _defaultTimeError{1.0};
		union {
			uint8_t     _flags{UseOriginUncertainties};
			bool        _legacyAndUnusedFlag;
		};
		bool            _verbose{false};
		std::string     _lastError;
		OPT(double)     _initLat;
		OPT(double)     _initLon;
		OPT(double)     _initDepth;
		OPT(Core::Time) _initTime;

		// Runtime
		TravelTimeTableInterfacePtr _ttt;
};


}


#endif
