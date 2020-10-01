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


#ifndef SEISCOMP_UTILS_UNITS_H
#define SEISCOMP_UTILS_UNITS_H


#include <seiscomp/core.h>
#include <string>
#include <map>


namespace Seiscomp {
namespace Util {


struct SC_SYSTEM_CORE_API UnitConversion {
	UnitConversion() {}
	UnitConversion(const std::string &fromUnit_, const std::string &toUnit_,
	               const std::string &toQMLUnit_, const std::string &toSEEDUnit_,
	               double s)
	: fromUnit(fromUnit_)
	, toUnit(toUnit_)
	, toQMLUnit(toQMLUnit_)
	, toSEEDUnit(toSEEDUnit_)
	, scale(s) {}

	std::string fromUnit;
	std::string toUnit;
	std::string toQMLUnit;
	std::string toSEEDUnit;
	double      scale;

	template <typename T>
	T operator()(T value) const;

	//! Convert from input unit to SI unit
	template <typename T>
	T convert(T value) const;

	//! Convert from SI unit back to input unit
	template <typename T>
	T revert(T value) const;
};


/**
 * @brief The UnitConverter class provides a simple interface to retrieve
 *        information to convert a value from one unit to another.
 *
 * The class supports variants of displacement and motion units such as
 * velocity and acceleration.
 *
 * @code
 * double value = 123;
 * double convertedValue;
 * // Convert from m/s
 * const UnitConversion *uc = UnitConverter::get('cm/s');
 * if ( uc != nullptr )
 *     convertedValue = uc->convert(value);
 * @endcode
 */
class SC_SYSTEM_CORE_API UnitConverter {
	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns a conversion object for a particular input unit.
		 * @param fromUnit The unit to convert from.
		 * @return A pointer to the conversion object. If no conversion is
		 *         available then nullptr is returned.
		 */
		static const UnitConversion *get(const std::string &fromUnit);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef std::map<std::string, UnitConversion> ConversionMap;
		static ConversionMap _conversionMap;
};


template <typename T>
inline T UnitConversion::convert(T value) const {
	return value*scale;
}


template <typename T>
inline T UnitConversion::revert(T value) const {
	return value/scale;
}


template <typename T>
inline T UnitConversion::operator()(T value) const {
	return convert(value);
}


}
}


#endif
