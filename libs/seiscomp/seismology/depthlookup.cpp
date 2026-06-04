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
#include <stdexcept>
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

} // anonymous namespace


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
			return true;
		}

		double fetch(double, double) const override {
			return _value;
		}

		double fetchMaxDepth(double, double) const override {
			return _value;
		}

	private:
		double _value{10.0};
};

REGISTER_DEPTH_LOOKUP(DepthLookupConstant, "Constant");


// ---------------------------------------------------------------------------
// DepthLookupPolygon
// ---------------------------------------------------------------------------

/**
 * Queries named polygon features from SeisComP's global GeoFeatureSet.
 *
 * Config keys:
 *   depths.polygon.regions  — ordered list of feature names
 *
 * Throws std::out_of_range when no configured polygon contains the point.
 * Use fetchDepth() / fetchMaxDepth() utility functions for fallback behaviour.
 */
class DepthLookupPolygon : public DepthLookup {
	public:
		bool init(const Config::Config &config) override {
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
			throw std::out_of_range("No polygon contains the given location");
		}

		double fetchMaxDepth(double lat, double lon) const override {
			for ( const auto &e : _entries ) {
				if ( e.feature->contains({lat, lon}) ) {
					if ( e.maxDepth ) {
						return *e.maxDepth;
					}
					throw std::out_of_range("Polygon has no maxDepth attribute");
				}
			}
			throw std::out_of_range("No polygon contains the given location");
		}

	private:
		struct Entry {
			const Geo::GeoFeature   *feature{nullptr};
			double                   defaultDepth{0.0};
			std::optional<double>    maxDepth;
		};

		std::vector<Entry> _entries;
};

REGISTER_DEPTH_LOOKUP(DepthLookupPolygon, "Polygon");


// ---------------------------------------------------------------------------
// Utility functions
// ---------------------------------------------------------------------------

double fetchDepth(const DepthLookup *lookup,
                  double lat, double lon,
                  double fallback) noexcept {
	try {
		return lookup->fetch(lat, lon);
	}
	catch ( ... ) {
		return fallback;
	}
}

double fetchMaxDepth(const DepthLookup *lookup,
                     double lat, double lon,
                     double fallback) noexcept {
	try {
		return lookup->fetchMaxDepth(lat, lon);
	}
	catch ( ... ) {
		return fallback;
	}
}


} // namespace Seismology
} // namespace Seiscomp
