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


#define SEISCOMP_COMPONENT MAGNITUDES

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/processing/magnitudes/utils.h>
#include <sstream>


namespace Seiscomp {
namespace Processing {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LogA0::set(const std::string &definition) {
	nodes.clear();

	std::istringstream iss(definition);
	std::string item;


	if ( definition.find(':') != std::string::npos ) {
		// new format is a comma-separated list
		// distance and values are separated by ':':
		// Example: 0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85
		while ( getline(iss, item,',') ) {
			double dist, val;
			auto pos = item.find(':');
			if ( pos != std::string::npos ) {
				if ( !Core::fromString(dist, item.substr(0, pos)) ) {
					SEISCOMP_ERROR("Unsupported logA0 value in %s",
					               item.c_str());
					return false;
				}
				if ( pos+1 < item.length() ) {
					if ( !Core::fromString(val, item.substr(pos+1)) ) {
						SEISCOMP_ERROR("Unsupported logA0 value in %s",
						               item.c_str());
						return false;
					}
				}
				else {
					SEISCOMP_ERROR("Missing correction value of logA0 in %s",
					               item.c_str());
					return false;
				}
			}
			else {
				SEISCOMP_ERROR("Unsupported logA0, expecting: dist:logA0, got %s",
				               item.c_str());
				return false;
			}
			nodes.push_back(Node(dist, val));
		}
	}
	else {
		// legacy format is still supported:
		// 0 -1.3;60 -2.8;100 -3.0;400 -4.5;1000 -5.85
		while ( getline(iss, item,';') ) {
			std::istringstream iss_item(item);
			double dist = std::numeric_limits<double>::lowest();
			double val = std::numeric_limits<double>::max();
			iss_item >> dist >> val;
			if ( dist > std::numeric_limits<double>::lowest()
			    && val < std::numeric_limits<double>::max() ) {
				nodes.push_back(Node(dist, val));
			}
			else {
				SEISCOMP_ERROR("Unsupported logA0 format in %s",
				               definition.c_str());
				return false;
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double LogA0::at(double dist_km) const {
	if ( nodes.size() >= 2 ) {
		for ( size_t i = 1; i < nodes.size(); ++i ) {
			if ( nodes[i - 1].first <= dist_km && dist_km <= nodes[i].first ) {
				double q = (dist_km - nodes[i - 1].first) / (nodes[i].first - nodes[i - 1].first);
				return q * (nodes[i].second - nodes[i - 1].second) + nodes[i - 1].second;
			}
		}
	}

	throw Core::ValueException("distance out of range");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
