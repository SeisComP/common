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
#include <utility>


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
	if ( !node.IsArray() ) {
		throw Core::StreamException("Coordinate is not an array");
	}

	if ( node.Size() < 2 ) {
		throw Core::StreamException("Coordinate has less than two elements");
	}

	if ( !node[0].IsNumber() ) {
		throw Core::StreamException("Coordinate[0] is no number");
	}

	if ( !node[1].IsNumber() ) {
		throw Core::StreamException("Coordinate[1] is no number");
	}

	return GeoCoordinate(node[1].GetDouble(), node[0].GetDouble());
}


void readPoint(GeoFeature *&f, const rapidjson::Value &node) {
	f->addVertex(readCoordinate(node), true);
}


void readMultiPoint(GeoFeature *&f, const rapidjson::Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException("MultiPoint definition not of type array");
	}

	const auto &points = node;
	for ( rapidjson::SizeType i = 0; i < points.Size(); ++i ) {
		readPoint(f, points[i]);
	}
}


void readRing(GeoFeature *f, const rapidjson::Value &node) {
	// a Ring is required to be of type array and must include at least 4
	// coordinates where the last coordinates is equal to the first
	if ( !node.IsArray() ) {
		throw Core::StreamException("Ring is not of type array");
	}

	const auto &coordinates = node;
	rapidjson::SizeType numCoordinates = coordinates.Size();
	if ( numCoordinates < 4 ) {
		throw Core::StreamException("Ring has less than 4 coordinates");
	}

	bool newSubFeature = true;
	size_t iFirstVertex = f ? f->vertices().size() : 0;

	for ( rapidjson::SizeType i = 0; i < numCoordinates; ++i ) {
		auto gc = readCoordinate(coordinates[i]);

		if ( i == (numCoordinates - 1) ) {
			// Check if first and last coord is equal
			if ( gc != f->vertices()[iFirstVertex] ) {
				throw Core::StreamException(
				            "Ring first coordinate does not match the last");
			}

			// Done with reading ...
			break;
		}

		f->addVertex(gc, newSubFeature);

		newSubFeature = false;
	}
}


void readLineString(GeoFeature *&f, const rapidjson::Value &node) {
	if ( !node.IsArray() ) {
		throw Core::StreamException("LineString definition not of type array");
	}

	const auto &coordinates = node;
	rapidjson::SizeType numCoordinates = coordinates.Size();
	if ( numCoordinates < 2 ) {
		throw Core::StreamException("LineString has less than 2 coordinates");
	}

	bool newSubFeature = true;

	for ( rapidjson::SizeType i = 0; i < numCoordinates; ++i ) {
		f->addVertex(readCoordinate(coordinates[i]), newSubFeature);

		newSubFeature = false;
	}
}


void readMultiLineString(GeoFeature *&f, const rapidjson::Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException(
		            "MultiLineString definition not of type array");
	}

	const auto &lineStrings = node;
	for ( rapidjson::SizeType i = 0; i < lineStrings.Size(); ++i ) {
		readLineString(f, lineStrings[i]);
	}
}


void readPolygon(GeoFeature *&f, const rapidjson::Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException("Polygon definition not of type array");
	}

	const auto &rings = node;
	for ( rapidjson::SizeType i = 0; i < rings.Size(); ++i ) {
		readRing(f, rings[i]);
	}

	f->setClosedPolygon(true);
}


void readMultiPolygon(GeoFeature *&f, const rapidjson::Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException(
		            "MultiPolygon definition not of type array");
	}

	const auto &polygons = node;
	for ( rapidjson::SizeType i = 0; i < polygons.Size(); ++i ) {
		readPolygon(f, polygons[i]);
	}
}


// forward declaration
void readGeometry(GeoFeature *f, const rapidjson::Value &node, const char *type);


void readGeometryCollection(GeoFeature *&f, const rapidjson::Value &node) {
	if ( !node.IsArray() ) {
		throw Core::StreamException(
		            "GeometryCollection definition not of type array");
	}

	// geometries object is mandatory but may be empty
	const auto &geometries = node;
	for ( rapidjson::SizeType i = 0; i < geometries.Size(); ++i ) {
		readGeometry(f, geometries[i], nullptr);
	}
}


struct SymbolTable {
	public:
		typedef void (*Symbol)(GeoFeature *&, const rapidjson::Value &);

		SymbolTable() {
			_symbols["Point"] = make_pair(readPoint, "coordinates");
			_symbols["MultiPoint"] = make_pair(readMultiPoint, "coordinates");
			_symbols["LineString"] = make_pair(readLineString, "coordinates");
			_symbols["MultiLineString"] = make_pair(readMultiLineString, "coordinates");
			_symbols["Polygon"] = make_pair(readPolygon, "coordinates");
			_symbols["MultiPolygon"] = make_pair(readMultiPolygon, "coordinates");
			_symbols["GeometryCollection"] = make_pair(readGeometryCollection, "geometries");
		}

		bool hasSymbol(const string &name) const {
			return _symbols.find(name) != _symbols.end();
		}

		Symbol getSymbol(const string &name, const char *& dataNode) const {
			auto it = _symbols.find(name);
			if ( it == _symbols.end() ) {
				stringstream ss;
				ss << "Unsupported geometry type: " << name;
				throw Core::StreamException(ss.str());
			}

			dataNode = it->second.second;

			//SEISCOMP_DEBUG("Found geometry of type: %s", name.c_str());
			return it->second.first;
		}

	private:
		map<string, pair<Symbol, const char*> > _symbols;
};

SymbolTable linker;


void readGeometry(GeoFeature *f, const rapidjson::Value &node,
                  const char *type) {
	JINIT;

	if ( !type ) {
		if ( JFAIL(node, "type", String) ) {
			throw Core::StreamException("Geometry has no string::type member");
		}

		type = JVAL.GetString();
	}

	const char *dataNode = nullptr;
	auto symbol = linker.getSymbol(type, dataNode);
	if ( symbol && dataNode ) {
		if ( !JHAS(node, dataNode) ) {
			stringstream ss;
			ss << "Geometry of type " << type
			   << " has no " << dataNode << " member";
			throw Core::StreamException(ss.str());

		}
		symbol(f, JVAL);
	}
}


void readFeature(GeoFeatureSet &featureSet, const rapidjson::Value &node,
                 const Category *category) {
	JINIT;

	// geometry member is mandatory but may be null
	if ( !JHAS(node, "geometry") ) {
		throw Core::StreamException("Feature has no geometry member");
	}

	GeoFeature *f = nullptr;

	const auto &geometry = JVAL;
	if ( !geometry.IsNull() ) {
		if ( !geometry.IsObject() ) {
			throw Core::StreamException(
			            "Feature member geometry is not of type object");
		}

		try {
			f = new GeoFeature(category);
			readGeometry(f, geometry, nullptr);
		}
		catch ( Core::StreamException &e ) {
			if ( f ) {
				delete f;
				f = nullptr;
			}
			throw;
		}

		f->updateBoundingBox();
		if ( f->area() < 0 ) {
			f->invertOrder();
		}
	}

	// properties member is mandatory but may be null
	if ( !JHAS(node, "properties") ) {
		if ( f ) {
			delete f;
			f = nullptr;
		}
		throw Core::StreamException("Feature has no properties member");
	}

	const auto &properties = JVAL;
	if ( properties.IsNull() ) {
		if ( f ) {
			featureSet.addFeature(f);
		}
		return;
	}

	if ( !properties.IsObject() ) {
		throw Core::StreamException(
		            "Feature member properties is not of type object");
	}

	if ( !f ) {
		f = new GeoFeature(category);
	}

	for ( jitr = properties.MemberBegin(); jitr != properties.MemberEnd(); ++jitr ) {
		string value;
		if ( JVAL.IsString() ) {
			value = JVAL.GetString();
		}
		else if ( JVAL.IsInt() ) {
			if ( !strcmp(JNAME.GetString(), "rank") ) {
				f->setRank(static_cast<unsigned int>(
				               JVAL.GetInt() < 0 ? 0 : JVAL.GetInt()));
				continue;
			}

			value = Core::toString(JVAL.GetInt());
		}
		else if ( JVAL.IsUint() ) {
			if ( !strcmp(JNAME.GetString(), "rank") ) {
				f->setRank(JVAL.GetUint());
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

		if ( !strcmp(JNAME.GetString(), "name") ) {
			f->setName(value);
		}
		else {
			f->setAttribute(JNAME.GetString(), value);
		}
	}

	featureSet.addFeature(f);
}


void readFeatureCollection(GeoFeatureSet &featureSet,
                           const rapidjson::Value &node,
                           const Category *category) {
	JINIT;

	if ( JFAIL(node, "features", Array) ) {
		throw Core::StreamException(
		            "FeatureCollection has no array::features member");
	}

	const auto &features = JVAL;
	for ( rapidjson::SizeType i = 0; i < features.Size(); ++i ) {
		const auto &feature = features[i];

		if ( JFAIL(feature, "type", String) ) {
			stringstream ss;
			ss << "Feature #" << i
			   << " of FeatureCollection has no string::type member";
			throw Core::StreamException(ss.str());
		}

		if ( strcmp(JVAL.GetString(), "Feature") ) {
			stringstream ss;
			ss << "Feature #" << i
			   << " of FeatureCollection is not of type Feature";
			throw Core::StreamException(ss.str());
		}

		readFeature(featureSet, feature, category);
	}
}


}


size_t readGeoJSON(GeoFeatureSet &featureSet, const std::string &path,
                   const Category *category) {
	SEISCOMP_DEBUG("Reading segments from file: %s", path.c_str());

	rapidjson::Document doc;
	ifstream ifs(path.c_str());
	if ( !ifs.is_open() ) {
		return 0;
	}

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
		GeoFeature *f = new GeoFeature(category);
		try {
			readGeometry(f, root, type.GetString());
		}
		catch ( Core::StreamException& ) {
			delete f;
			f = nullptr;
			throw;
		}

		f->updateBoundingBox();
		if ( f->area() < 0 ) {
			f->invertOrder();
		}

		featureSet.addFeature(f);
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
