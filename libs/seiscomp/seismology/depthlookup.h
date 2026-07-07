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


#ifndef SEISCOMP_SEISMOLOGY_DEPTHLOOKUP_H
#define SEISCOMP_SEISMOLOGY_DEPTHLOOKUP_H


#include <string>
#include <vector>

#include <seiscomp/config/config.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace Seismology {


DEFINE_SMARTPOINTER(DepthLookup);

/**
 * @brief Abstract interface for region- and slab-based depth lookup,
 *        used by scautoloc and other SeisComP processing modules.
 *
 * Concrete implementations are registered via the REGISTER_DEPTH_LOOKUP macro
 * and instantiated at runtime through DepthLookupFactory::Create().
 *
 * Two implementations ship with the library:
 *  - "Constant"  Always returns a fixed depth read from depths.constant.value.
 *                Maximum depth is configurable via depths.constant.maxDepth.
 *  - "Polygon"   Queries named polygon features from SeisComP's global
 *                GeoFeatureSet; each polygon must carry a defaultDepth
 *                attribute (km, required) and may carry maxDepth (km).
 *                Falls back to depths.polygon.fallback / depths.polygon.maxDepth
 *                when no polygon matches.
 *
 * All depth knowledge — including the fallback for locations outside any
 * configured region — is fully encapsulated in the backend. The client
 * never needs to supply or handle a fallback value.
 */
class SC_SYSTEM_CORE_API DepthLookup : public Core::BaseObject {
	public:
		virtual ~DepthLookup() = default;

		/**
		 * @brief Initialise the implementation.
		 *
		 * Called once after construction. Each implementation reads its
		 * own settings from @p config.
		 *
		 * @return True on success.
		 */
		virtual bool init(const Config::Config &config) = 0;

		/**
		 * @brief Return the default depth (km) at (@p lat, @p lon).
		 *
		 * Always returns a finite value. When no region or slab zone
		 * contains the given location the backend returns its own
		 * configured fallback depth.
		 */
		virtual double fetch(double lat, double lon) const = 0;

		/**
		 * @brief Return the maximum acceptable depth (km) at (@p lat, @p lon).
		 *
		 * Always returns a finite value. When no region or slab zone
		 * contains the given location the backend returns its own
		 * configured fallback.
		 */
		virtual double fetchMaxDepth(double lat, double lon) const = 0;

		/**
		 * @brief Return candidate seed depths (km) at (@p lat, @p lon).
		 *
		 * For regions with dual seismicity (shallow crustal layer above a
		 * subducting slab), a backend may return more than one depth so the
		 * caller can evaluate each seed independently. The default
		 * implementation returns {fetch(lat, lon)}, i.e. a single depth,
		 * which is the correct behaviour for all current backends.
		 */
		virtual std::vector<double> fetchCandidates(double lat, double lon) const {
			return {fetch(lat, lon)};
		}
};


DEFINE_INTERFACE_FACTORY(DepthLookup);


} // namespace Seismology
} // namespace Seiscomp


#define REGISTER_DEPTH_LOOKUP(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Seismology::DepthLookup, Class> \
__##Class##InterfaceFactory__(Service)


#endif  // SEISCOMP_SEISMOLOGY_DEPTHLOOKUP_H
