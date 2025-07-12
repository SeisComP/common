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

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/geo/formats/geojson.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/utils/files.h>

// For GeoJSON support
#include <3rd-party/rapidjson/rapidjson.h>
#include <3rd-party/rapidjson/document.h>
#include <3rd-party/rapidjson/istreamwrapper.h>
#include <3rd-party/rapidjson/writer.h>
#include <3rd-party/rapidjson/stringbuffer.h>
#include <3rd-party/rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>
#include <utility>


using namespace std;
using namespace rapidjson;


#define JINIT Value::ConstMemberIterator jitr
#define JFAIL(node, member, type) ((jitr = node.FindMember(member)) == node.MemberEnd() || !jitr->value.Is##type())
#define JHAS(node, member) ((jitr = node.FindMember(member)) != node.MemberEnd())
#define JTYPE(type) (jitr->value.Is##type())
#define JOK(node, member, type) ((jitr = node.FindMember(member)) != node.MemberEnd() && jitr->value.Is##type())
#define JNAME jitr->name
#define JVAL jitr->value


// Reference: https://datatracker.ietf.org/doc/html/rfc7946


namespace Seiscomp::Geo {


namespace {


GeoCoordinate readCoordinate(const Value &node) {
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

	return {node[1].GetDouble(), node[0].GetDouble()};
}


void readPoint(GeoFeature *&f, const Value &node) {
	f->addVertex(readCoordinate(node), true);
}


void readMultiPoint(GeoFeature *&f, const Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException("MultiPoint definition not of type array");
	}

	const auto &points = node;
	for ( SizeType i = 0; i < points.Size(); ++i ) {
		readPoint(f, points[i]);
	}
}


void readRing(GeoFeature *f, const Value &node) {
	// a Ring is required to be of type array and must include at least 4
	// coordinates where the last coordinates is equal to the first
	if ( !node.IsArray() ) {
		throw Core::StreamException("Ring is not of type array");
	}

	const auto &coordinates = node;
	SizeType numCoordinates = coordinates.Size();
	if ( numCoordinates < 4 ) {
		throw Core::StreamException("Ring has less than 4 coordinates");
	}

	bool newSubFeature = true;
	size_t iFirstVertex = f->vertices().size();

	for ( SizeType i = 0; i < numCoordinates; ++i ) {
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


void readLineString(GeoFeature *&f, const Value &node) {
	if ( !node.IsArray() ) {
		throw Core::StreamException("LineString definition not of type array");
	}

	const auto &coordinates = node;
	SizeType numCoordinates = coordinates.Size();
	if ( numCoordinates < 2 ) {
		throw Core::StreamException("LineString has less than 2 coordinates");
	}

	bool newSubFeature = true;

	for ( SizeType i = 0; i < numCoordinates; ++i ) {
		f->addVertex(readCoordinate(coordinates[i]), newSubFeature);

		newSubFeature = false;
	}
}


void readMultiLineString(GeoFeature *&f, const Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException(
		            "MultiLineString definition not of type array");
	}

	const auto &lineStrings = node;
	for ( SizeType i = 0; i < lineStrings.Size(); ++i ) {
		readLineString(f, lineStrings[i]);
	}
}


void readPolygon(GeoFeature *&f, const Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException("Polygon definition not of type array");
	}

	const auto &rings = node;
	for ( SizeType i = 0; i < rings.Size(); ++i ) {
		readRing(f, rings[i]);
	}

	f->setClosedPolygon(true);
}


void readMultiPolygon(GeoFeature *&f, const Value &node) {
	// geometry is required to be of type array but may be empty
	if ( !node.IsArray() ) {
		throw Core::StreamException(
		            "MultiPolygon definition not of type array");
	}

	const auto &polygons = node;
	for ( SizeType i = 0; i < polygons.Size(); ++i ) {
		readPolygon(f, polygons[i]);
	}
}


// forward declaration
void readGeometry(GeoFeature *f, const Value &node, const char *type);


void readGeometryCollection(GeoFeature *&f, const Value &node) {
	if ( !node.IsArray() ) {
		throw Core::StreamException(
		            "GeometryCollection definition not of type array");
	}

	// geometries object is mandatory but may be empty
	const auto &geometries = node;
	for ( SizeType i = 0; i < geometries.Size(); ++i ) {
		readGeometry(f, geometries[i], nullptr);
	}

	// Features with mixed open and closed subfeatures are not supported.
	// The GeoJSON standard also states that if a GeometryCollection contains
	// only geometry objects of the same type the explicit
	// Mulit{Point,LineString,Polygon} types should be used.
	f->setClosedPolygon(false);
}


struct SymbolTable {
	public:
		using Symbol = void (*)(GeoFeature *&, const Value &);

		SymbolTable() {
			_symbols["Point"] = make_pair(readPoint, "coordinates");
			_symbols["MultiPoint"] = make_pair(readMultiPoint, "coordinates");
			_symbols["LineString"] = make_pair(readLineString, "coordinates");
			_symbols["MultiLineString"] = make_pair(readMultiLineString, "coordinates");
			_symbols["Polygon"] = make_pair(readPolygon, "coordinates");
			_symbols["MultiPolygon"] = make_pair(readMultiPolygon, "coordinates");
			_symbols["GeometryCollection"] = make_pair(readGeometryCollection, "geometries");
		}

		[[nodiscard]] bool hasSymbol(const string &name) const {
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

			//SEISCOMP_DEBUG("Found geometry of type: %s", name);
			return it->second.first;
		}

	private:
		map<string, pair<Symbol, const char*> > _symbols;
};

SymbolTable linker;


void readGeometry(GeoFeature *f, const Value &node,
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


void readFeature(GeoFeatureSet &featureSet, const Value &node,
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
                           const Value &node,
                           const Category *category) {
	JINIT;

	if ( JFAIL(node, "features", Array) ) {
		throw Core::StreamException(
		            "FeatureCollection has no array::features member");
	}

	const auto &features = JVAL;
	for ( SizeType i = 0; i < features.Size(); ++i ) {
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


inline Value createCoordinate(Document::AllocatorType& allocator,
                              const GeoCoordinate &coord) {
	Value coordinate(kArrayType);
	coordinate.PushBack(Value(coord.longitude()).Move(), allocator);
	coordinate.PushBack(Value(coord.latitude()).Move(), allocator);
	return coordinate;
}


inline void addCoordinate(Value &coordinates,
                          Document::AllocatorType& allocator,
                          const GeoCoordinate &coord) {
	coordinates.PushBack(createCoordinate(allocator, coord), allocator);
}


void addProperties(Value &feature, Document::AllocatorType& allocator,
                   const GeoFeature *gf) {

	Value properties(kObjectType);

	// name
	if ( !gf->name().empty() ) {
		properties.AddMember("name",
		                     StringRef(gf->name().data(), gf->name().size()),
		                     allocator);
	}

	// rank
	properties.AddMember("rank", gf->rank(), allocator);

	// attributes
	for ( const auto &[key, value] : gf->attributes() ) {
		properties.AddMember(StringRef(key.data(), key.size()),
		                     StringRef(value.data(), value.size()),
		                     allocator);
	}

	feature.AddMember("properties", properties, allocator);
}


bool setPoint(Value &geometry, Document::AllocatorType& allocator,
              const GeoCoordinate &coord) {

	geometry.AddMember("type", "Point", allocator);
	geometry.AddMember("coordinates", createCoordinate(allocator, coord),
	                   allocator);

	return true;
}


bool setMultiPoint(Value &geometry, Document::AllocatorType& allocator,
                   const GeoFeature *gf) {

	Value coordinates(kArrayType);

	for ( const auto &coord : gf->vertices() ) {
		addCoordinate(coordinates, allocator, coord);
	}

	geometry.AddMember("type", "MultiPoint", allocator);
	geometry.AddMember("coordinates", coordinates, allocator);

	return true;
}


bool setLineString(Value &geometry, Document::AllocatorType& allocator,
                   GeoFeature::GeoCoordinates::const_iterator &first,
                   GeoFeature::GeoCoordinates::const_iterator last) {
	if ( first == last ) {
		return false;
	}

	Value coordinates(kArrayType);
	for ( ; first != last; ++first ) {
		addCoordinate(coordinates, allocator, *first);
	}

	geometry.AddMember("type", "LineString", allocator);
	geometry.AddMember("coordinates", coordinates, allocator);

	return true;
}

bool setLineString(Value &geometry, Document::AllocatorType& allocator,
                   const GeoFeature *gf) {
	auto first = gf->vertices().begin();
	return setLineString(geometry, allocator, first, gf->vertices().end());
}


bool setMultiLineString(Value &geometry, Document::AllocatorType& allocator,
                        const GeoFeature *gf) {

	Value coordinates(kArrayType);
	Value lineString(kArrayType);

	auto sub_it = gf->subFeatures().begin();
	size_t idx = 0;
	// Test for sub-features with only one vertex is performed outside this
	// method.
	for ( const auto &coord : gf->vertices() ) {
		// Create a new line string for each sub feature
		if ( sub_it != gf->subFeatures().end() && *sub_it == idx ) {
			coordinates.PushBack(lineString, allocator);

			// lineString is reset during PushBack and needs to be reinitialized
			// as array type
			lineString.SetArray();

			++sub_it;
		}

		addCoordinate(lineString, allocator, coord);
		++idx;
	}

	coordinates.PushBack(lineString, allocator);

	if ( coordinates.Empty() ) {
		SEISCOMP_WARNING("Skipping MultiLineString '%s', no valid line string",
		                 gf->name());
		return false;
	}

	geometry.AddMember("type", "MultiLineString", allocator);
	geometry.AddMember("coordinates", coordinates, allocator);

	return true;
}



// Supports only Points and LineStrings due to GeoFeature limitation of not
// being able to distinguish lines and polygons in sub-features.
bool setGeometryCollection(Value &geometry, Document::AllocatorType& allocator,
                           const GeoFeature *gf) {

	Value geometries(kArrayType);

	const auto &vertices = gf->vertices();

	auto first = vertices.begin();
	auto sub_it = gf->subFeatures().begin();

	for ( ; first != vertices.end(); ++sub_it ) {
		// End iterator for current sub-feature
		auto last = sub_it == gf->subFeatures().end() ?
		            vertices.end() : next(vertices.begin(), *sub_it);
		Value geo(kObjectType);

		// Point
		if ( next(first) == last ) {
			setPoint(geo, allocator, *first++);
		}
		// LineString
		else {
			setLineString(geo, allocator, first, last);
		}

		geometries.PushBack(geo, allocator);
	}

	geometry.AddMember("type", "GeometryCollection", allocator);
	geometry.AddMember("geometries", geometries, allocator);

	return true;
}


bool setPolygon(Value &geometry,
                Document::AllocatorType& allocator,
                const GeoFeature *gf) {

	Value coordinates(kArrayType);
	Value ring(kArrayType);

	for ( const auto &coord : gf->vertices() ) {
		addCoordinate(ring, allocator, coord);
	}

	if ( gf->vertices().front() != gf->vertices().back() ) {
		addCoordinate(ring, allocator, gf->vertices().front());
	}

	if ( ring.Size() < 4 ) {
		SEISCOMP_WARNING("Skipping Polygon '%s', to few vertices", gf->name());
		return false;
	}

	coordinates.PushBack(ring, allocator);

	geometry.AddMember("type", "Polygon", allocator);
	geometry.AddMember("coordinates", coordinates, allocator);

	return true;
}


bool setMultiPolygon(Value &geometry,
                     Document::AllocatorType& allocator,
                     const GeoFeature *gf) {

	Value coordinates(kArrayType);
	Value rings(kArrayType);
	Value ring(kArrayType);

	auto sub_it = gf->subFeatures().begin();
	const GeoCoordinate *firstCoord = &gf->vertices().front();
	size_t idx = 0;
	for ( const auto &coord : gf->vertices() ) {
		// Create a new ring for each sub feature
		if ( sub_it != gf->subFeatures().end() && *sub_it == idx ) {
			if ( coord != *firstCoord ) {
				addCoordinate(ring, allocator, *firstCoord);
			}
			firstCoord = &coord;

			if ( ring.Size() >= 4 ) {
				rings.PushBack(ring, allocator);
			}
			else {
				SEISCOMP_WARNING("Skipping sub-polygon of '%s', to few vertices",
				                 gf->name());
				ring.Clear();
			}

			ring.SetArray();
			++sub_it;
		}

		addCoordinate(ring, allocator, coord);
		++idx;
	}

	if ( *firstCoord != gf->vertices().back() ) {
		addCoordinate(ring, allocator, *firstCoord);
	}

	if ( ring.Size() >= 4 ) {
		rings.PushBack(ring, allocator);
	}
	else {
		SEISCOMP_WARNING("Skipping sub-polygon of '%s', to few vertices",
		                 gf->name());
		ring.Clear();
	}

	if ( rings.Empty() ) {
		SEISCOMP_WARNING("Skipping MultiPolygon '%s', no valid polygons",
		                 gf->name());
		return false;
	}

	coordinates.PushBack(rings, allocator);

	geometry.AddMember("type", "MultiPolygon", allocator);
	geometry.AddMember("coordinates", coordinates, allocator);

	return true;
}


bool setFeature(Value &feature, Document::AllocatorType& allocator,
                const GeoFeature *gf) {

	feature.AddMember("type", "Feature", allocator);
	addProperties(feature, allocator, gf);

	Value geometry(kObjectType);

	const auto &vertices = gf->vertices();
	if ( vertices.empty() ) {
		SEISCOMP_WARNING("Skipping feature '%s', no verticies defined",
		                 gf->name());
		return false;
	}

	// Polygons - The SeisComP GeoFeature class does not distinguish between
	// islands (of a main land) and holes inside a polygon.
	bool result = false;
	if ( gf->closedPolygon() ) {
		// Polygon
		if ( gf->subFeatures().empty() ) {
			result = setPolygon(geometry, allocator, gf);
		}
		// MultiPolygon
		else {
			result = setMultiPolygon(geometry, allocator, gf);
		}
	}
	else {
		// Point
		if ( vertices.size() == 1 ) {
			result = setPoint(geometry, allocator, vertices.front());
		}
		// MultiPoint
		else if ( vertices.size() <= gf->subFeatures().size() + 1 ) {
			result = setMultiPoint(geometry, allocator, gf);
		}
		// Line
		else if ( gf->subFeatures().empty() ) {
			result = setLineString(geometry, allocator, gf);
		}
		// MultiLine or FeatureCollection with mixed Points and LineStrings
		else {
			bool hasPoints = false;
			size_t lastIdx = 0;
			for ( const auto &idx : gf->subFeatures() ) {
				if ( idx - lastIdx < 2 ) {
					hasPoints = true;
					break;
				}

				lastIdx = idx;
			}

			if ( hasPoints ) {
				result = setGeometryCollection(geometry, allocator, gf);
			}
			else {
				result = setMultiLineString(geometry, allocator, gf);
			}
		}
	}

	if ( result ) {
		feature.AddMember("geometry", geometry, allocator);
		return true;
	}

	return false;
}

void writeGeoJSON(std::ostream &out, Document &document, int indent = -1) {
	StringBuffer buffer;
	if ( indent >= 0 ) {
		PrettyWriter<StringBuffer> writer(buffer);
		writer.SetIndent(' ', indent);
		document.Accept(writer);
	}
	else {
		Writer<StringBuffer> writer(buffer);
		document.Accept(writer);
	}

	out << buffer.GetString();
}

} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t readGeoJSON(GeoFeatureSet &featureSet, const std::string &path,
                   const Category *category) {
	SEISCOMP_DEBUG("Reading features from GeoJSON file: %s", path);

	Document doc;
	ifstream ifs(path.c_str());
	if ( !ifs.is_open() ) {
		return 0;
	}

	IStreamWrapper isw(ifs);
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
	const Value &type = JVAL;

	if ( linker.hasSymbol(type.GetString()) ) {
		auto *f = new GeoFeature(category);
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool writeGeoJSON(std::ostream &os, const GeoFeature &feature, int indent) {
	Document document;

	// Setup document
	document.SetObject();
	Document::AllocatorType& allocator = document.GetAllocator();

	if ( !setFeature(document, allocator, &feature) ) {
		return false;
	}

	// Serialize document
	writeGeoJSON(os, document, indent);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool writeGeoJSON(const std::string &path, const GeoFeature &feature,
                  int indent) {
	SEISCOMP_DEBUG("Writing GeoFeature to GeoJSON file: %s", path);

	// Write buffer to file
	std::ofstream ofs(path);
	if ( !ofs.is_open() ) {
		SEISCOMP_ERROR("Could not open file for writing: %s", path);
		return false;
	}

	bool res = writeGeoJSON(ofs, feature, indent);
	ofs.close();

	if ( res ) {
		SEISCOMP_DEBUG("GeoJSON file created");
	}

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t writeGeoJSON(std::ostream &os, const GeoFeatureSet::Features &gfs,
                    int indent) {
	if ( gfs.empty() ) {
		return 0;
	}

	if ( gfs.size() == 1 ) {
		return writeGeoJSON(os, *gfs.front(), indent) ? 1 : 0;
	}

	Document document;
	Value features(kArrayType);

	// Setup document
	document.SetObject();
	Document::AllocatorType& allocator = document.GetAllocator();

	// Collect features
	size_t featureCount = 0;
	for ( const auto &gf : gfs ) {
		Value feature(kObjectType);
		if ( setFeature(feature, allocator, gf) ) {
			features.PushBack(feature, allocator);
			featureCount += 1;
		}
	}

	if ( !featureCount ) {
		return 0;
	}

	// FeatureColletion
	document.AddMember("type", "FeatureCollection", allocator);
	document.AddMember("features", features, allocator);

	// Serialize document
	writeGeoJSON(os, document, indent);

	return featureCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t writeGeoJSON(const std::string &path, const GeoFeatureSet::Features &gfs,
                    int indent) {
	if ( gfs.empty() ) {
		return 0;
	}

	std::ofstream ofs(path);
	if ( !ofs.is_open() ) {
		SEISCOMP_ERROR("Could not open file for writing: %s", path);
		return 0;
	}

	size_t featureCount = writeGeoJSON(ofs, gfs, indent);
	ofs.close();
	SEISCOMP_DEBUG("Wrote %zu features to GeoJSON file: %s", featureCount, path);

	return featureCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Seiscomp::Geo
