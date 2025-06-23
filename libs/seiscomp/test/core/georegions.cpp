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


#define SEISCOMP_COMPONENT TestGeoRegions
#define SEISCOMP_TEST_MODULE SeisComP


#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include <seiscomp/unittest/unittests.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/geo/coordinate.h>
#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/geo/formats/geojson.h>
#include <seiscomp/seismology/regions.h>
#include <seiscomp/seismology/regions/polygon.h>
#include <seiscomp/utils/files.h>

#include <boost/system/error_code.hpp>

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Geo;


struct RegionTest {
	double lat;
	double lon;
	string name;
};

#define ASSERT_MSG(cond, msg) do \
{ if (!(cond)) { \
	ostringstream oss; \
	oss << __FILE__ << "(" << __LINE__ << "): "<< msg << "\n"; cerr << oss.str(); \
	abort(); } \
} while(0)



struct F {
	F()
	: dataDir(string(SOURCE_DIR) + "/data/"),
	  resultDir(string(BINARY_DIR) + "/result/") {}

	string dataDir;
	string resultDir;
};

BOOST_FIXTURE_TEST_SUITE(seiscomp_core_georegions, F)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly1) {
	Logging::enableConsoleLogging(Logging::getAll());
	GeoFeature testPoly1("test1", nullptr, 0);
	testPoly1.addVertex(-20,-20);
	testPoly1.addVertex(20,-20);
	testPoly1.addVertex(20,20);
	testPoly1.addVertex(-20,20);
	testPoly1.setClosedPolygon(true);
	testPoly1.updateBoundingBox();

	BOOST_CHECK(testPoly1.contains(Vertex(0,0)));
	BOOST_CHECK(testPoly1.contains(Vertex(-19,-19)));
	BOOST_CHECK(testPoly1.contains(Vertex(19,19)));
	BOOST_CHECK(!testPoly1.contains(Vertex(21,19)));
	BOOST_CHECK(!testPoly1.contains(Vertex(19,21)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly2) {
	GeoFeature testPoly2("test2", nullptr, 0);

	testPoly2.addVertex(-20,160);
	testPoly2.addVertex(20,160);
	testPoly2.addVertex(20,-160);
	testPoly2.addVertex(-20,-160);
	testPoly2.setClosedPolygon(true);

	BOOST_CHECK(testPoly2.contains(Vertex(0,180)));
	BOOST_CHECK(testPoly2.contains(Vertex(-19,-161)));
	BOOST_CHECK(testPoly2.contains(Vertex(19,161)));
	BOOST_CHECK(!testPoly2.contains(Vertex(19,159)));
	BOOST_CHECK(!testPoly2.contains(Vertex(21,-159)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly3) {
	// Test of "contains" in sub polygons
	GeoFeature testPoly3("test3", nullptr, 0);
	testPoly3.addVertex(2,2);
	testPoly3.addVertex(1,2);
	testPoly3.addVertex(1,1);
	testPoly3.addVertex(2,1);
	testPoly3.addVertex(4,4, true);
	testPoly3.addVertex(3,4);
	testPoly3.addVertex(3,3);
	testPoly3.addVertex(4,3);
	testPoly3.addVertex(7,7, true);
	testPoly3.addVertex(5,6);
	testPoly3.addVertex(6,4);
	testPoly3.addVertex(7,5);
	testPoly3.setClosedPolygon(true);

	BOOST_CHECK(testPoly3.contains(Vertex(1.5,1.5)));
	BOOST_CHECK(testPoly3.contains(Vertex(3.5,3.5)));
	BOOST_CHECK(testPoly3.contains(Vertex(6,6)));
	BOOST_CHECK(!testPoly3.contains(Vertex(-10,11)));
	BOOST_CHECK(!testPoly3.contains(Vertex(21,-159)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly4) {
	GeoFeature testPoly4("test4", nullptr, 0);
	testPoly4.addVertex(46.4235710121, 7.35353852642);
	testPoly4.addVertex(46.3596457104, 7.42766849702);
	testPoly4.addVertex(46.318197187, 7.40082802204);
	testPoly4.addVertex(46.2801115086, 7.24510849846);
	testPoly4.addVertex(46.4161431163, 7.20000173128);
	testPoly4.addVertex(46.4220940655, 7.20817822901);
	testPoly4.setClosedPolygon(true);

	BOOST_CHECK(testPoly4.contains(Vertex(46.33,7.327)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly5) {
	GeoFeature testPoly5("test5", nullptr, 0);
	testPoly5.addVertex(46, 7);
	testPoly5.addVertex(46, 8);
	testPoly5.addVertex(45, 8);
	testPoly5.addVertex(45, 8.2);
	testPoly5.addVertex(45, 8.4);
	testPoly5.addVertex(45, 8.6);
	testPoly5.addVertex(45, 9);
	testPoly5.addVertex(44, 9);
	testPoly5.addVertex(44, 7);
	testPoly5.setClosedPolygon(true);

	BOOST_CHECK(testPoly5.contains(Vertex(45,7.5)));
	BOOST_CHECK(!testPoly5.contains(Vertex(45,8)));
	BOOST_CHECK(testPoly5.contains(Vertex(45,7.5)));
	BOOST_CHECK(testPoly5.contains(Vertex(44.5,8)));
	BOOST_CHECK(!testPoly5.contains(Vertex(45.5,8.5)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly6) {
	GeoFeature testPoly6("test6", nullptr, 0);
	testPoly6.addVertex(0, 0);
	testPoly6.addVertex(-2, 2);
	testPoly6.addVertex(-4, 2);
	testPoly6.addVertex(-5, 1);
	testPoly6.addVertex(-3, -1);
	testPoly6.addVertex(-1, -1);
	testPoly6.setClosedPolygon(true);

	BOOST_CHECK(!testPoly6.contains(Vertex(-10,-1)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly7) {
	GeoFeature testPoly7("test7", nullptr, 0);
	testPoly7.addVertex(-90, -180);
	testPoly7.addVertex(-90, -60);
	testPoly7.addVertex(-90, 60);
	testPoly7.addVertex(-90, 180);
	testPoly7.addVertex(90, 180);
	testPoly7.addVertex(90, 60);
	testPoly7.addVertex(90, -60);
	testPoly7.addVertex(90, -180);
	testPoly7.setClosedPolygon(true);

	BOOST_CHECK(testPoly7.contains(Vertex(0,0)));
	BOOST_CHECK(testPoly7.contains(Vertex(45,90)));
	BOOST_CHECK(testPoly7.contains(Vertex(-45,90)));
	BOOST_CHECK(testPoly7.contains(Vertex(10,10)));
	BOOST_CHECK(testPoly7.contains(Vertex(10,-10)));
	BOOST_CHECK(testPoly7.contains(Vertex(-45,180)));
	BOOST_CHECK(testPoly7.contains(Vertex(-45,-180)));
	BOOST_CHECK(testPoly7.contains(Vertex(-90,180)));
	BOOST_CHECK(testPoly7.contains(Vertex(-90,-180)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(fepRegions) {
	PolyRegions regions;
	regions.read(dataDir + "fep");

	ifstream ifs;
	ifs.open(dataDir + "region-test.csv");

	BOOST_REQUIRE(ifs.is_open());

	RegionTest tr;
	while ( ifs ) {
		ifs >> tr.lat >> tr.lon;
		getline(ifs, tr.name);
		Core::trim(tr.name);
		if ( tr.name.empty() ) {
			continue;
		}

		string name = regions.findRegionName(tr.lat, tr.lon);
		if ( name.empty() ) {
			name = Regions::getRegionName(tr.lat, tr.lon);
		}
		BOOST_CHECK_EQUAL(name, tr.name);
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(bnaRegions) {
	GeoFeatureSet features;
	features.readFile(dataDir + "bna/brandenburg.bna", nullptr);
	BOOST_REQUIRE(!features.features().empty());

	GeoFeature *f = features.features()[0];
	BOOST_CHECK_EQUAL(f->name(), "Brandenburg");

	BOOST_CHECK(f->bbox().contains(GeoCoordinate(52, 14)));
	BOOST_CHECK(f->contains(GeoCoordinate(52, 14)));

	features.clear();

	BOOST_REQUIRE(features.readFile(dataDir + "bna/bna-with-comments.bna", nullptr));

	features.clear();
	features.readFile(dataDir + "bna/header-with-attributes.bna", nullptr);

	vector<GeoFeature> expected;
	GeoFeature::Attributes atts;

	atts["eventType"] = "quarry blast";
	atts["maxDepth"] = "10";
	expected.emplace_back("name1", nullptr, 4, atts);
	atts.clear();

	atts["test1"] = "1,2,3";
	atts["test2"] = "1:2:3";
	atts["test3"] = "1,2,3";
	atts["test4"] = "1:2:3";
	expected.emplace_back("name2", nullptr, 5, atts);
	atts.clear();

	atts["test1"] = "1,2,3";
	atts["te:st"] = "bar";
	expected.emplace_back("name3", nullptr, 1, atts);
	atts.clear();

	BOOST_REQUIRE(features.features().size() == expected.size());
	auto exp_it = expected.begin();
	auto got_it = features.features().begin();
	for ( ; exp_it != expected.end(); ++exp_it, ++got_it ) {
		BOOST_CHECK_EQUAL(exp_it->name(), (*got_it)->name());
		BOOST_CHECK_EQUAL(exp_it->rank(), (*got_it)->rank());
		bool success = true;
		auto exp_att_it = exp_it->attributes().begin();
		for ( ; exp_att_it != exp_it->attributes().end() && success; ++exp_att_it ) {
			auto got_att_it = (*got_it)->attributes().find(exp_att_it->first);
			success = got_att_it != (*got_it)->attributes().end() &&
			          exp_att_it->second == got_att_it->second;
			BOOST_CHECK(success);
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(geojsonRegions) {
	string srcDir = dataDir + "geojson/";
	string srcFile;
	string dstDir = resultDir + "geojson/";

	ASSERT_MSG(Util::pathExists(srcDir),
			   "Test data dir not found: " << srcDir);
	ASSERT_MSG(Util::pathExists(dstDir) || Util::createPath(dstDir),
	           "Could not create output directory: " << dstDir);
	string testFile;

	GeoFeatureSet gfs;

	// *********************
	// Point outside Feature
	srcFile = srcDir + "point.geojson";
	BOOST_TEST_INFO("Invalid Point: " << srcFile);
	BOOST_REQUIRE(gfs.readFile(srcFile, nullptr));
	BOOST_REQUIRE_EQUAL(gfs.features().size(), 1);
	auto *f = gfs.features()[0];
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK(f->name().empty());
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 1);
	BOOST_CHECK(f->subFeatures().empty());
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(52, 13));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(52, 13, 52, 13));

	// ***********************
	// Feature with MultiPoint
	gfs.clear();
	srcFile = srcDir + "feature.geojson";
	BOOST_TEST_INFO("Invalid Feature: " << srcFile);
	BOOST_REQUIRE(gfs.readFile(srcFile, nullptr));
	BOOST_REQUIRE_EQUAL(gfs.features().size(), 1);
	f = gfs.features()[0];
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 1);
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "MultiPoint");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 2);
	BOOST_CHECK(f->subFeatures() == vector<size_t>({1}));
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(52, 14));
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(52, 15));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(52, 14, 52, 15));

	// ************************
	// Feature without Geometry
	gfs.clear();
	srcFile = srcDir + "feature-no-geometry.geojson";
	BOOST_TEST_INFO("Invalid feature: " << srcFile);
	BOOST_REQUIRE(gfs.readFile(srcFile, nullptr));
	BOOST_REQUIRE_EQUAL(gfs.features().size(), 1);
	f = gfs.features()[0];
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 0);
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "FeatureWithoutGeometry");
	BOOST_CHECK_EQUAL(f->rank(), 2);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK(f->vertices().empty());
	BOOST_CHECK(f->subFeatures().empty());
	BOOST_CHECK(f->bbox().isEmpty());

	// *****************
	// FeatureCollection
	gfs.clear();
	srcFile = srcDir + "featurecollection.geojson";
	BOOST_TEST_INFO("Invalid FeatureCollection: " << srcFile);
	BOOST_REQUIRE(gfs.readFile(srcFile, nullptr));
	BOOST_CHECK_EQUAL(gfs.features().size(), 7);

	// Point
	BOOST_TEST_INFO("Invalid Point in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 1);
	f = gfs.features()[0];
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "Point");
	BOOST_CHECK_EQUAL(f->rank(), 16);
	BOOST_CHECK_EQUAL(f->attributes().size(), 1);
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 0);
	auto it = f->attributes().find("foo");
	BOOST_CHECK(it != f->attributes().end());
	BOOST_CHECK_EQUAL(it->second, "bar");
	BOOST_CHECK_EQUAL(f->vertices().size(), 1);
	BOOST_CHECK(f->subFeatures().empty());
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(52, 13));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(52, 13, 52, 13));

	// MultiPoint
	BOOST_TEST_INFO("Invalid MultiPoint in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 2);
	f = gfs.features()[1];
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "MultiPoint");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 2);
	BOOST_CHECK(f->subFeatures() == vector<size_t>({1}));
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(52, 14));
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(52, 15));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(52, 14, 52, 15));

	// LineString
	BOOST_TEST_INFO("Invalid LineString in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 3);
	f = gfs.features()[2];
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "LineString");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 2);
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 0);
	BOOST_CHECK(f->subFeatures().empty());
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(-17, 20));
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(-17, 23));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(-17, 20, -17, 23));

	// MultiLineString
	BOOST_TEST_INFO("Invalid MultiLineString in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 4);
	f = gfs.features()[3];
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "MultiLineString");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 4);
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 1);
	BOOST_CHECK(f->subFeatures() == vector<size_t>({2}));
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(-15, 20));
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(-15, 23));
	BOOST_CHECK_EQUAL(f->vertices()[2], GeoCoordinate(-15, 26));
	BOOST_CHECK_EQUAL(f->vertices()[3], GeoCoordinate(-15, 29));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(-15, 20, -15, 29));

	// Polygon
	BOOST_TEST_INFO("Invalid Polygon in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 5);
	f = gfs.features()[4];
	BOOST_CHECK(f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "Polygon");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 4);
	BOOST_CHECK(f->subFeatures().empty());
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(-5, 0));
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(-5, 3));
	BOOST_CHECK_EQUAL(f->vertices()[2], GeoCoordinate(-2, 3));
	BOOST_CHECK_EQUAL(f->vertices()[3], GeoCoordinate(-2, 0));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(-5, 0, -2, 3));

	// MultiPolygon
	BOOST_TEST_INFO("Invalid MultiPolygon in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 6);
	f = gfs.features()[5];
	BOOST_CHECK(f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "MultiPolygon");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 6);
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 1);
	BOOST_CHECK(f->subFeatures() == vector<size_t>({3}));
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(-25, 10));
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(-25, 13));
	BOOST_CHECK_EQUAL(f->vertices()[2], GeoCoordinate(-22, 13));
	BOOST_CHECK_EQUAL(f->vertices()[3], GeoCoordinate(-25, 20));
	BOOST_CHECK_EQUAL(f->vertices()[4], GeoCoordinate(-25, 23));
	BOOST_CHECK_EQUAL(f->vertices()[5], GeoCoordinate(-22, 23));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(-25, 10, -22, 23));

	// GeometryCollection consisting of Point, LineString, MultiPoint and LineString
	BOOST_TEST_INFO("Invalid GeometryCollection in FeatureCollection: " << srcFile);
	BOOST_REQUIRE_GE(gfs.features().size(), 7);
	f = gfs.features()[6];
	BOOST_CHECK(!f->closedPolygon());
	BOOST_CHECK_EQUAL(f->name(), "GeometryCollection");
	BOOST_CHECK_EQUAL(f->rank(), 1);
	BOOST_CHECK(f->attributes().empty());
	BOOST_CHECK_EQUAL(f->vertices().size(), 7);
	BOOST_CHECK_EQUAL(f->subFeatures().size(), 4);
	BOOST_CHECK(f->subFeatures() == vector<size_t>({1, 3, 4, 5}));
	BOOST_CHECK_EQUAL(f->vertices()[0], GeoCoordinate(52, 50)); // Point
	BOOST_CHECK_EQUAL(f->vertices()[1], GeoCoordinate(52, 51)); // LineString
	BOOST_CHECK_EQUAL(f->vertices()[2], GeoCoordinate(52, 52));
	BOOST_CHECK_EQUAL(f->vertices()[3], GeoCoordinate(52, 53)); // MultiPoint
	BOOST_CHECK_EQUAL(f->vertices()[4], GeoCoordinate(52, 54));
	BOOST_CHECK_EQUAL(f->vertices()[5], GeoCoordinate(52, 55)); // LineString
	BOOST_CHECK_EQUAL(f->vertices()[6], GeoCoordinate(52, 56));
	BOOST_CHECK(f->bbox() == GeoBoundingBox(52, 50, 52, 56));

	// **********
	// Write file
	string dstFile = dstDir + "featurecollection.geojson";
	BOOST_TEST_INFO("Wrote invalid number of features to: " << dstFile);
	BOOST_CHECK_EQUAL(writeGeoJSON(dstFile, gfs.features(), 2),
	                  gfs.features().size());

	// Read output file
	GeoFeatureSet gfs2;
	BOOST_TEST_INFO("Invalid number of features in: " << dstFile);
	BOOST_CHECK_EQUAL(readGeoJSON(gfs2, dstFile), gfs.features().size());

	ASSERT_MSG(gfs.features().size() == gfs2.features().size(),
	           "GeoFeatureSets mismatch");

	auto exp = gfs.features().begin();
	for ( const auto *got : gfs2.features() ) {
		BOOST_TEST_INFO("Missmatch in feature: " << got->name());
		BOOST_CHECK_EQUAL(*got, **exp++);
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


BOOST_AUTO_TEST_SUITE_END()
