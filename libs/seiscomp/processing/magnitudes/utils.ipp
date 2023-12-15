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


#include <seiscomp/core/strings.h>
#include <stdexcept>
#include <algorithm>


namespace Seiscomp {
namespace Processing {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool TableXY<T>::empty() const {
	return items.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool TableXY<T>::set(const std::string &definition) {
	items.clear();

	std::istringstream iss(definition);
	std::string item;

	if ( definition.find(':') != std::string::npos ) {
		// new format is a comma-separated list
		// x and y are separated by ':':
		// Example: 0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85
		while ( std::getline(iss, item,',') ) {
			double x;
			T y;
			auto pos = item.find(':');
			if ( pos != std::string::npos ) {
				if ( Core::fromString(x, item.substr(0, pos)) ) {
					if ( pos + 1 < item.length() ) {
						if ( Core::fromString(y, item.substr(pos + 1)) ) {
							items.push_back(Item(x, y));
							continue;
						}
					}
				}
			}

			return false;
		}
	}
	else {
		// legacy format is still supported:
		// 0 -1.3;60 -2.8;100 -3.0;400 -4.5;1000 -5.85
		while ( std::getline(iss, item,';') ) {
			std::istringstream iss_item(item);
			double x = std::numeric_limits<double>::lowest();
			std::string strY;
			iss_item >> x >> strY;
			if ( iss_item && (x > std::numeric_limits<double>::lowest()) ) {
				T y;
				if ( Core::fromString(y, strY) ) {
					items.push_back(Item(x, y));
					continue;
				}
			}

			return false;
		}
	}

	std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
		return a.first < b.first;
	});

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool TableXY<T>::set(const std::vector<std::string> &definition) {
	items.clear();

	for ( const auto &item: definition ) {
		double x;
		T y;
		auto pos = item.find(':');
		if ( pos != std::string::npos ) {
			if ( Core::fromString(x, item.substr(0, pos)) ) {
				if ( pos + 1 < item.length() ) {
					if ( Core::fromString(y, item.substr(pos + 1)) ) {
						items.push_back(Item(x, y));
						continue;
					}
				}
			}
		}

		return false;
	}

	std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
		return a.first < b.first;
	});

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
T TableXY<T>::at(double x) const {
	if ( items.size() >= 2 ) {
		for ( size_t i = 1; i < items.size(); ++i ) {
			if ( items[i - 1].first <= x && x <= items[i].first ) {
				double q = (x - items[i - 1].first) / (items[i].first - items[i - 1].first);
				return q * (items[i].second - items[i - 1].second) + items[i - 1].second;
			}
		}
	}
	else if ( items.size() == 1 ) {
		if ( items[0].first == x ) {
			return items[0].second;
		}
	}

	throw std::out_of_range("x out of range");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline std::ostream &operator<<(std::ostream &os, const TableXY<T> &table) {
	bool first = true;
	for ( auto &&item : table.items ) {
		if ( !first ) {
			os << ", ";
		}
		else {
			first = false;
		}
		os << item.first << ":" << item.second;
	}
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
