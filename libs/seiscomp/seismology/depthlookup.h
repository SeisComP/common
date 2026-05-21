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

#include <seiscomp/config/config.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace Seismology {


DEFINE_SMARTPOINTER(DepthLookup);

/**
 * @brief Abstract interface for region- and slab-based default/maximum depth
 *        lookup, used by scautoloc and other SeisComP processing modules.
 *
 * Concrete implementations are registered via the REGISTER_DEPTH_LOOKUP macro
 * and instantiated at runtime through DepthLookupFactory::Create().
 *
 * Two implementations ship with the library:
 *  - "Constant"  Returns a fixed depth read from @c depths.constant.value.
 *  - "Polygon"   Queries named polygon features from SeisComP's global
 *                GeoFeatureSet; each polygon must carry a @c defaultDepth
 *                attribute (km, required) and may carry @c maxDepth (km).
 *                Fallback is read from @c depths.polygon.fallback.
 *
 * A separate @c dlslab2 plugin (seiscomp/main) provides depth lookup from
 * USGS Slab2.0 depth-footprint contours.
 *
 * Each implementation owns all depth knowledge including its fallback value;
 * callers pass no fallback.
 */
class SC_SYSTEM_CORE_API DepthLookup : public Core::BaseObject {
	public:
		virtual ~DepthLookup() = default;

		/**
		 * @brief Initialise the implementation.
		 *
		 * Called once after construction.  Each implementation reads its
		 * own settings from @p config.
		 *
		 * @return True on success.
		 */
		virtual bool init(const Config::Config &config) = 0;

		/**
		 * @brief Return the default depth (km) at (@p lat, @p lon).
		 *
		 * Always returns a finite value; the implementation supplies its
		 * own configured fallback when no region/slab matches.
		 */
		virtual double fetch(double lat, double lon) const = 0;

		/**
		 * @brief Return the maximum acceptable depth (km) at (@p lat, @p lon).
		 *
		 * Always returns a finite value; the implementation supplies its
		 * own configured fallback when no region/slab matches.
		 */
		virtual double fetchMaxDepth(double lat, double lon) const = 0;
};


DEFINE_INTERFACE_FACTORY(DepthLookup);


} // namespace Seismology
} // namespace Seiscomp


#define REGISTER_DEPTH_LOOKUP(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Seismology::DepthLookup, Class> \
__##Class##InterfaceFactory__(Service)


#endif  // SEISCOMP_SEISMOLOGY_DEPTHLOOKUP_H
