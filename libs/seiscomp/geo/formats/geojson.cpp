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


#define SEISCOMP_COMPONENT Geo

// For GeoJSON support
#include <3rd-party/rapidjson/rapidjson.h>
#include <3rd-party/rapidjson/document.h>
#include <3rd-party/rapidjson/istreamwrapper.h>

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/geo/formats/geojson.h>

#include <iostream>
#include <fstream>


using namespace std;


#define JINIT rapidjson::Value::ConstMemberIterator jitr
#define JFAIL(node, member, type) ((jitr = node.FindMember(member)) == node.MemberEnd() || !jitr->value.Is##type())
#define JHAS(node, member) ((jitr = node.FindMember(member)) != node.MemberEnd())
#define JTYPE(type) (jitr->value.Is##type())
#define JOK(node, member, type) ((jitr = node.FindMember(member)) != node.MemberEnd() && jitr->value.Is##type())
#define JNAME jitr->name
#define JVAL jitr->value


// Reference: https://datatracker.ietf.org/doc/html/rfc7946


namespace Seiscomp {
namespace Geo {


namespace {


GeoCoordinate readCoordinate(const rapidjson::Value &node) {
	if ( node.Size() < 2 ) {
		throw Core::StreamException("Coordinate has less than two elements");
	}

	if ( !node[0].IsNumber() ) {
		throw Core::StreamException("Coordinate[0] is no number");
	}

	if ( !node[1].IsNumber() ) {
		throw Core::StreamException("Coordinate[1] is no number");
	}

	return GeoCoordinate(node[1].GetFloat(), node[0].GetFloat());
}


void readPoint(GeoFeature *&f,
               const rapidjson::Value &node,
               const Category *category,
               unsigned int rank) {
	if ( node.Size() != 1 )
		throw Core::StreamException("Point must have exactly one coordinate");

	auto gc = readCoordinate(node[0]);
	if ( !f ) f = new GeoFeature(category, rank);
	f->addVertex(gc, true);
}


void readMultiPoint(GeoFeature *&f,
                    const rapidjson::Value &node,
                    const Category *category,
                    unsigned int rank) {
	const auto &points = node;
	if ( points.Size() < 1 ) {
		throw Core::StreamException("MultiPoint has less than 1 Point");
	}

	for ( rapidjson::SizeType p = 0; p < points.Size(); ++p ) {
		const auto &point = points[p];
		readPoint(f, point, category, rank);
	}
}


void readRing(GeoFeature *&f,
              const rapidjson::Value &node,
              const Category *category,
              unsigned int rank,
              bool closed) {
	if ( closed ) {
		if ( node.Size() < 2 )
			throw Core::StreamException("Closed LineString has less than 4 coordinates");
	}
	else {
		if ( node.Size() < 2 )
			throw Core::StreamException("LineString has less than 2 coordinates");
	}

	bool newSubFeature = true;

	rapidjson::SizeType numCoords = node.Size();
	size_t iFirstVertex = f ? f->vertices().size() : 0;

	for ( rapidjson::SizeType c = 0; c < numCoords; ++c ) {
		const auto &coord = node[c];
		auto gc = readCoordinate(coord);

		if ( closed && (c == (numCoords - 1)) ) {
			// Check if first and last coord is equal
			if ( gc != f->vertices()[iFirstVertex] ) {
				throw Core::StreamException("Closed LineString first coordinate does not match the last");
			}

			// Done with reading ...
			break;
		}

		if ( !f ) f = new GeoFeature(category, rank);
		f->addVertex(gc, newSubFeature);

		newSubFeature = false;
	}
}


void readLineString(GeoFeature *&f,
                    const rapidjson::Value &node,
                    const Category *category,
                    unsigned int rank) {
	readRing(f, node, category, rank, false);
}


void readMultiLineString(GeoFeature *&f,
                         const rapidjson::Value &node,
                         const Category *category,
                         unsigned int rank) {
	const auto &lineStrings = node;
	if ( lineStrings.Size() < 1 ) {
		throw Core::StreamException("MultiLineString has less than 1 LineString");
	}

	for ( rapidjson::SizeType p = 0; p < lineStrings.Size(); ++p ) {
		const auto &lineString = lineStrings[p];
		readLineString(f, lineString, category, rank);
	}
}


void readPolygon(GeoFeature *&f,
                 const rapidjson::Value &node,
                 const Category *category,
                 unsigned int rank) {
	if ( node.Size() < 1 ) {
		throw Core::StreamException("Polygon has less than 1 Ring");
	}

	for ( rapidjson::SizeType r = 0; r < node.Size(); ++r ) {
		const auto &ring = node[r];
		readRing(f, ring, category, rank, true);
	}

	f->setClosedPolygon(true);
}


void readMultiPolygon(GeoFeature *&f,
                      const rapidjson::Value &node,
                      const Category *category,
                      unsigned int rank) {
	const auto &polys = node;
	if ( polys.Size() < 1 ) {
		throw Core::StreamException("MultiPolygon has less than 1 Polygon");
	}

	for ( rapidjson::SizeType p = 0; p < polys.Size(); ++p ) {
		const auto &poly = polys[p];
		readPolygon(f, poly, category, rank);
	}
}


void readGeometryCollection(GeoFeatureSet &featureSet,
                            const rapidjson::Value &node,
                            const Category *category,
                            unsigned int rank = 1) {
	throw Core::StreamException("Geometry collections are not supported");
}


struct SymbolTable {
	public:
		typedef void (*Symbol)(GeoFeature *&, const rapidjson::Value &, const Category *, unsigned int);

		SymbolTable() {
			_symbols["Point"] = readPoint;
			_symbols["MultiPoint"] = readMultiPoint;
			_symbols["LineString"] = readLineString;
			_symbols["MultiLineString"] = readMultiLineString;
			_symbols["Polygon"] = readPolygon;
			_symbols["MultiPolygon"] = readMultiPolygon;
		}

		bool hasSymbol(const string &name) const {
			return _symbols.find(name) != _symbols.end();
		}

		Symbol getSymbol(const string &name) const {
			auto it = _symbols.find(name);
			if ( it == _symbols.end() )
				return nullptr;
			return it->second;
		}

	private:
		map<string, Symbol> _symbols;
};

SymbolTable linker;


GeoFeature *readGeometry(const rapidjson::Value &node,
                         const char *type,
                         const Category *category,
                         unsigned int rank = 1) {
	JINIT;

	if ( !type ) {
		if ( JFAIL(node, "type", String) ) {
			SEISCOMP_WARNING("Geometry has no type:string attribute");
			return nullptr;
		}

		type = JVAL.GetString();
	}

	if ( !JHAS(node, "coordinates") ) {
		SEISCOMP_WARNING("Geometry has no coordinates:list attribute");
		return nullptr;
	}

	GeoFeature *feature = nullptr;

	auto symbol = linker.getSymbol(type);
	if ( symbol ) {
		try {
			symbol(feature, JVAL, category, rank);
		}
		catch ( ... ) {
			if ( feature ) {
				delete feature;
				feature = nullptr;
			}
		}
	}

	return feature;
}


void readFeature(GeoFeatureSet &featureSet, const rapidjson::Value &node,
                 const Category *category) {
	JINIT;

	unsigned int rank = 1;

	if ( JFAIL(node, "geometry", Object) ) {
		SEISCOMP_WARNING("Feature is missing geometry:object property");
		return;
	}

	const auto &geometry = JVAL;

	auto f = readGeometry(geometry, nullptr, category, rank);
	if ( !f ) {
		return;
	}

	if ( JOK(node, "properties", Object) ) {
		const auto &objects = JVAL;
		for ( jitr = objects.MemberBegin(); jitr != objects.MemberEnd(); ++jitr ) {
			string value;
			if ( JVAL.IsString() ) {
				value = JVAL.GetString();
			}
			else if ( JVAL.IsInt() ) {
				if ( !strcmp(JNAME.GetString(), "rank") ) {
					rank = static_cast<unsigned int>(JVAL.GetInt() < 0 ? 0 : JVAL.GetInt());
					continue;
				}

				value = Core::toString(JVAL.GetInt());
			}
			else if ( JVAL.IsUint() ) {
				if ( !strcmp(JNAME.GetString(), "rank") ) {
					rank = JVAL.GetUint();
					continue;
				}

				value = Core::toString(JVAL.GetUint());
			}
			else if ( JVAL.IsInt64() ) {
				value = Core::toString(JVAL.GetInt64());
			}
			else if ( JVAL.IsUint64() ) {
				value = Core::toString(JVAL.GetUint64());
			}
			else if ( JVAL.IsDouble() ) {
				value = Core::toString(JVAL.GetDouble());
			}

			if ( !strcmp(JNAME.GetString(), "name") )
				f->setName(value);
			else
				f->setAttribute(JNAME.GetString(), value);
		}
	}

	f->updateBoundingBox();
	if ( f->area() < 0 )
		f->invertOrder();

	featureSet.addFeature(f);
}


void readFeatureCollection(GeoFeatureSet &featureSet, const rapidjson::Value &node,
                           const Category *category) {
	JINIT;
	if ( !JHAS(node, "features") )
		return;

	const auto &features = JVAL;
	for ( rapidjson::SizeType f = 0; f < features.Size(); ++f ) {
		readFeature(featureSet, features[f], category);
	}
}


}


size_t readGeoJSON(GeoFeatureSet &featureSet, const std::string &path,
                   const Category *category) {
	SEISCOMP_DEBUG("Reading segments from file: %s", path.c_str());

	rapidjson::Document doc;
	ifstream ifs(path.c_str());
	if ( !ifs.is_open() ) return false;
	rapidjson::IStreamWrapper isw(ifs);
	doc.ParseStream(isw);
	if ( doc.HasParseError() ) {
		throw Core::StreamException(path + ": file parse error (geojson)");
	}

	if ( !doc.IsObject() ) {
		throw Core::StreamException(path + ": root is not an object (geojson)");
	}

	const auto &root = doc;

	JINIT;
	if ( JFAIL(root, "type", String) ) {
		throw Core::StreamException(path + ": type is not a string");
	}

	size_t currentNumberOfFeatures = featureSet.features().size();
	const rapidjson::Value &type = JVAL;

	if ( linker.hasSymbol(type.GetString()) ) {
		GeoFeature *f = readGeometry(root, type.GetString(), category);
		if ( f ) {
			featureSet.addFeature(f);
		}
	}
	else if ( !strcmp(type.GetString(), "GeometryCollection") ) {
		readGeometryCollection(featureSet, root, category);
	}
	else if ( !strcmp(type.GetString(), "Feature") ) {
		readFeature(featureSet, root, category);
	}
	else if ( !strcmp(type.GetString(), "FeatureCollection") ) {
		readFeatureCollection(featureSet, root, category);
	}

	return featureSet.features().size() - currentNumberOfFeatures;
}


}
}
