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


#ifndef SEISCOMP_SEISMOLOGY_LOCATOR_ROUTER_H
#define SEISCOMP_SEISMOLOGY_LOCATOR_ROUTER_H

#include <seiscomp/datamodel/origin.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/seismology/locatorinterface.h>

namespace {

namespace scdm = Seiscomp::DataModel;

class RouterLocator : public Seiscomp::Seismology::LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		RouterLocator() = default;


	// ----------------------------------------------------------------------
	//  Locator interface
	// ----------------------------------------------------------------------
	public:
		virtual bool init(const Seiscomp::Config::Config &config) override;

		virtual IDList profiles() const override;
		virtual void setProfile(const std::string &name) override;

		virtual int capabilities() const override;

		virtual scdm::Origin *locate(PickList &pickList) override;
		virtual scdm::Origin *locate(PickList &pickList,
		                             double initLat, double initLon,
		                             double initDepth,
		                             const Seiscomp::Core::Time& initTime) override;

		virtual scdm::Origin *relocate(const scdm::Origin *origin) override;

	protected:
		struct LocatorProfile {
			std::string locatorName;
			std::string profileName;
			OPT(double) minDepth;
			OPT(double) maxDepth;
			Seiscomp::Geo::GeoFeature *feature;
			Seiscomp::Seismology::LocatorInterface *locator{nullptr};
		};
		using LocatorProfiles = std::vector<LocatorProfile>;

		// Maps a <locator>_<profile> name to an initialized locator instance.
		// An individual locator instance is required for each profile since
		// the locator interface does not support resetting the profile nor does
		// it provide access to the default profile name.
		using LocatorPool = std::map<std::string,
		                             Seiscomp::Seismology::LocatorInterfacePtr>;

		const LocatorProfile* lookup(const scdm::Origin *origin) const;

		Seiscomp::Seismology::LocatorInterfacePtr   _initialLocator;
		Seiscomp::Geo::GeoFeatureSet                _geoFeatureSet;

		LocatorProfiles                             _profiles;
		LocatorPool                                 _locators;
		int                                         _joinedCapabilities{0};
};


}


#endif
