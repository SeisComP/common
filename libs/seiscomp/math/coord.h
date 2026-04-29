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
#include <seiscomp/core/enumeration.h>
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

		void serialize(Core::BaseObject::Archive& ar) override;

	private:
		std::string _name;
};


typedef NamedCoord<float> NamedCoordF;
typedef NamedCoord<double> NamedCoordD;


/**
 * Location type of a city derived from GeoNames feature codes.
 * Serializes as a lowercase string attribute in XML.
 */
MAKEENUM(
	CityType,
	EVALUES(
		CITYTYPE_UNKNOWN, //!< Absent or unrecognised type attribute
		CITYTYPE_CITY,    //!< Capital or administrative centre (PPLC, PPLA, PPLA2)
		CITYTYPE_TOWN,    //!< Populated place or minor admin centre (PPL, PPLA3, PPLA4)
		CITYTYPE_VILLAGE, //!< Small settlement (PPLF, PPLL, PPLR, PPLS, etc.)
		CITYTYPE_SUBURB   //!< Section of a populated place (PPLX)
	),
	ENAMES(
		"",
		"city",
		"town",
		"village",
		"suburb"
	)
);


/**
 * @brief Administrative region (state, province, etc.) associated with a city.
 *
 * Serializes as an XML child element carrying an optional abbreviation
 * attribute and the full name as element text, e.g.:
 * @code
 *   <state abbr="NSW">New South Wales</state>
 * @endcode
 *
 * The @c abbr field holds the ISO 3166-2 subdivision suffix (alphabetic
 * portion only, e.g. "NSW" from "AU-NSW", "CA" from "US-CA"). It is left
 * empty for subdivisions that use numeric codes or where no ISO code exists.
 */
struct SC_SYSTEM_CORE_API AdminRegion : public Core::BaseObject {
	DECLARE_SERIALIZATION;

	std::string abbr; //!< ISO 3166-2 suffix, e.g. "NSW", "CA" (may be empty)
	std::string name; //!< Full region name, e.g. "New South Wales"

	bool empty() const { return name.empty(); }
};


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

		//! Full country name, e.g. "Australia". Serializes as <country> child element.
		void setCountry(const std::string &);
		const std::string &country() const;

		void setCategory(std::string &);
		const std::string &category() const;

		void setType(CityType);
		CityType type() const;

		/**
		 * @brief Administrative region (state/province).
		 *
		 * Serializes as a child element, e.g.:
		 * @code
		 *   <state abbr="NSW"><name>New South Wales</name></state>
		 * @endcode
		 * The @c abbr field holds the ISO 3166-2 subdivision suffix (alphabetic
		 * portion only). It is left empty where no alphabetic ISO code exists.
		 */
		void setAdminRegion(const AdminRegion &);
		const AdminRegion &adminRegion() const;

		void serialize(Core::BaseObject::Archive& ar) override;

	private:
		std::string _countryID;
		std::string _country;
		double _population;
		std::string _category;
		CityType _type;
		AdminRegion _adminRegion;
};


typedef City<float> CityF;
typedef City<double> CityD;


} // of ns  Geo
} // of ns  Math
} // of ns Seiscomp


#endif
