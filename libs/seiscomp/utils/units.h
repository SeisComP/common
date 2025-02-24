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
#include <seiscomp/core/strings.h>

#include <stdexcept>
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

		/**
		 * @brief Convertes a string to a numeric value in the requested unit.
		 *
		 * An example string is "1.23 km". If the requested unit is "m" then
		 * 1230 will be returned. If the requested unit is "m" and the input
		 * string is "1.23" then "1.23" will be returned.
		 *
		 * If the input cannot be converted, an exception is thrown.
		 *
		 * @param str The input string to be converted
		 * @param unit The requested unit of the value
		 * @return The value which will populated with the parsed value.
		 */
		template <typename T>
		static T parse(std::string_view str, const std::string &unit) {
			if ( str.empty() ) {
				throw std::invalid_argument("input string empty");
			}

			auto outputConv = get(unit);
			if ( !outputConv ) {
				throw std::invalid_argument("invalid target unit");
			}

			size_t p = str.size();
			while ( p > 0 ) {
				--p;

				if ( (str[p] >= '0') && (str[p] <= '9') ) {
					// Found digit
					break;
				}
			}

			++p;

			T value;

			if ( !Core::fromString(value, str.substr(0, p)) ) {
				throw std::invalid_argument("invalid value string");
			}

			if ( p < str.size() ) {
				auto inputUnit = Core::trimFront(str.substr(p));
				auto inputConv = get(std::string(inputUnit));
				if ( !inputConv) {
					throw std::invalid_argument(Core::stringify("invalid input unit: '%s'", inputUnit));
				}

				if ( inputConv->toUnit != outputConv->toUnit ) {
					// Incompatible units
					throw std::invalid_argument("units are not compatible");
				}

				value = inputConv->convert(outputConv->revert(value));
			}

			return value;
		}


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		using ConversionMap = std::map<std::string, UnitConversion>;
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
