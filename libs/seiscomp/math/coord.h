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


#ifndef SEISCOMP_MATH_GEO_COORD_H
#define SEISCOMP_MATH_GEO_COORD_H

#include <seiscomp/core/baseobject.h>
#include <string>


namespace Seiscomp {
namespace Math {
namespace Geo {


template<typename T>
struct Coord : public Core::BaseObject {
	DECLARE_SERIALIZATION;

	typedef T ValueType;

	Coord();
	Coord(T lat_, T lon_);

	void set(T lat_, T lon_);

	T latitude() const { return lat; }
	T longitude() const { return lon; }

	bool operator==(const Coord<T> &other) const;
	bool operator!=(const Coord<T> &other) const;

	T lat;
	T lon;
};

typedef Coord<float> CoordF;
typedef Coord<double> CoordD;


template<typename T>
class NamedCoord : public Coord<T> {
	public:
		NamedCoord();
		NamedCoord(const std::string& name, T lat_, T lon_);

		~NamedCoord();

		using Coord<T>::set;
		void set(const std::string& name, T lat_, T lon_);

	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void serialize(Core::BaseObject::Archive& ar);

	private:
		std::string _name;
};


typedef NamedCoord<float> NamedCoordF;
typedef NamedCoord<double> NamedCoordD;


template<typename T>
class City : public NamedCoord<T> {
	public:
		City();
		City(const std::string& name,
		     T lat_, T lon_, size_t population);

		City(const std::string& name, const std::string& countryID,
		     T lat_, T lon_, size_t population);

		~City();

	public:
		//! Population in thousands
		void setPopulation(double population);
		double population() const;

		void setCountryID(const std::string &);
		const std::string &countryID() const;

		void setCategory(std::string &);
		const std::string &category() const;

		void serialize(Core::BaseObject::Archive& ar);

	private:
		std::string _countryID;
		double _population;
		std::string _category;
};


typedef City<float> CityF;
typedef City<double> CityD;


} // of ns  Geo
} // of ns  Math
} // of ns Seiscomp


#endif
