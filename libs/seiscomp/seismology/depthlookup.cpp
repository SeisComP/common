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


#define SEISCOMP_COMPONENT DepthLookup

#include <seiscomp/seismology/depthlookup.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Seismology::DepthLookup, SC_SYSTEM_CORE_API);


namespace Seiscomp {
namespace Seismology {


// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

std::optional<double> parseAttr(const Geo::GeoFeature *f,
                                const std::string &key) {
	const auto &attrs = f->attributes();
	auto it = attrs.find(key);
	if ( it == attrs.end() || it->second.empty() ) {
		return std::nullopt;
	}
	double v;
	if ( !Core::fromString(v, it->second) ) {
		return std::nullopt;
	}
	return v;
}


// ---------------------------------------------------------------------------
// DepthLookupConstant
// ---------------------------------------------------------------------------

class DepthLookupConstant : public DepthLookup {
	public:
		bool init(const Config::Config &config) override {
			try {
				_value = config.getDouble("depths.constant.value");
			}
			catch ( ... ) {
				SEISCOMP_INFO("DepthLookup/Constant: depths.constant.value not set, "
				              "using default %.0f km", _value);
			}
			try {
				_maxDepth = config.getDouble("depths.constant.maxDepth");
			}
			catch ( ... ) {}
			return true;
		}

		double fetch(double, double) const override {
			return _value;
		}

		double fetchMaxDepth(double, double) const override {
			return _maxDepth;
		}

	private:
		double _value{10.0};
		double _maxDepth{1000.0};
};

REGISTER_DEPTH_LOOKUP(DepthLookupConstant, "Constant");


// ---------------------------------------------------------------------------
// DepthLookupPolygon
// ---------------------------------------------------------------------------

/**
 * Queries named polygon features from SeisComP's global GeoFeatureSet.
 *
 * Config keys:
 *   depths.polygon.regions   — ordered list of feature names
 *   depths.polygon.fallback  — default depth returned when no polygon matches (km)
 *   depths.polygon.maxDepth  — max depth returned when no polygon matches (km)
 */
class DepthLookupPolygon : public DepthLookup {
	public:
		bool init(const Config::Config &config) override {
			try {
				_fallback = config.getDouble("depths.polygon.fallback");
			}
			catch ( ... ) {
				SEISCOMP_INFO("DepthLookup/Polygon: depths.polygon.fallback not set, "
				              "using default %.0f km", _fallback);
			}
			try {
				_maxDepthFallback = config.getDouble("depths.polygon.maxDepth");
			}
			catch ( ... ) {}

			std::vector<std::string> names;
			try {
				names = config.getStrings("depths.polygon.regions");
			}
			catch ( ... ) {}

			if ( names.empty() ) {
				SEISCOMP_WARNING("DepthLookup/Polygon: no regions configured "
				                 "under depths.polygon.regions");
				return true;
			}

			const Geo::GeoFeatureSet &fs =
			    Geo::GeoFeatureSetSingleton::getInstance();

			for ( const auto *f : fs.features() ) {
				if ( !f->closedPolygon() ) {
					continue;
				}
				if ( std::find(names.begin(), names.end(), f->name()) == names.end() ) {
					continue;
				}

				auto dd = parseAttr(f, "defaultDepth");
				if ( !dd ) {
					SEISCOMP_WARNING("DepthLookup/Polygon: feature '%s' has "
					                 "no defaultDepth attribute — skipped",
					                 f->name().c_str());
					continue;
				}

				_entries.push_back({f, *dd, parseAttr(f, "maxDepth")});
				SEISCOMP_DEBUG("DepthLookup/Polygon: loaded region '%s' "
				               "defaultDepth=%.0f", f->name().c_str(), *dd);
			}

			SEISCOMP_INFO("DepthLookup/Polygon: %zu region(s) loaded",
			              _entries.size());
			return true;
		}

		double fetch(double lat, double lon) const override {
			for ( const auto &e : _entries ) {
				if ( e.feature->contains({lat, lon}) ) {
					return e.defaultDepth;
				}
			}
			return _fallback;
		}

		double fetchMaxDepth(double lat, double lon) const override {
			for ( const auto &e : _entries ) {
				if ( e.feature->contains({lat, lon}) ) {
					return e.maxDepth.value_or(_maxDepthFallback);
				}
			}
			return _maxDepthFallback;
		}

	private:
		struct Entry {
			const Geo::GeoFeature   *feature{nullptr};
			double                   defaultDepth{0.0};
			std::optional<double>    maxDepth;
		};

		double             _fallback{10.0};
		double             _maxDepthFallback{1000.0};
		std::vector<Entry> _entries;
};

REGISTER_DEPTH_LOOKUP(DepthLookupPolygon, "Polygon");

} // anonymous namespace


} // namespace Seismology
} // namespace Seiscomp
