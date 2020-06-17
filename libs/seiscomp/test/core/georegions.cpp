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
#define SEISCOMP_TEST_MODULE TestGeoRegions


#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/seismology/regions.h>
#include <seiscomp/seismology/regions/polygon.h>

#include <boost/system/error_code.hpp>

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Geo;


struct RegionTest {
	double lat;
	double lon;
	string name;
};

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(poly1) {
	Logging::enableConsoleLogging(Logging::getAll());
	GeoFeature testPoly1("test1", NULL, 0);
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
	GeoFeature testPoly2("test2", NULL, 0);

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
	GeoFeature testPoly3("test3", NULL, 0);
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
	GeoFeature testPoly4("test4", NULL, 0);
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
	GeoFeature testPoly5("test5", NULL, 0);
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
	GeoFeature testPoly6("test6", NULL, 0);
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
	GeoFeature testPoly7("test7", NULL, 0);
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
	regions.read("./data/fep");

	ifstream ifs;
	ifs.open("./data/region-test.csv");

	BOOST_REQUIRE(ifs.is_open());

	RegionTest tr;
	while ( ifs ) {
		ifs >> tr.lat >> tr.lon;
		getline(ifs, tr.name);
		Core::trim(tr.name);
		if ( tr.name.empty() ) continue;
		string name = regions.findRegionName(tr.lat, tr.lon);
		if ( name.empty() )
			name = Regions::getRegionName(tr.lat, tr.lon);
		BOOST_CHECK_EQUAL(name, tr.name);
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(bnaRegions) {
	GeoFeatureSet features;
	features.readBNAFile("./data/bna/brandenburg.bna", NULL);
	BOOST_REQUIRE(!features.features().empty());

	GeoFeature *f = features.features()[0];
	BOOST_CHECK_EQUAL(f->name(), "Brandenburg");

	BOOST_CHECK(f->bbox().contains(GeoCoordinate(52, 14)));
	BOOST_CHECK(f->contains(GeoCoordinate(52, 14)));

	features.clear();

	BOOST_REQUIRE(features.readBNAFile("./data/bna/bna-with-comments.bna", NULL));

	features.clear();
	features.readBNAFile("./data/bna/header-with-attributes.bna", NULL);

	vector<GeoFeature> expected;
	GeoFeature::Attributes atts;

	atts["eventType"] = "quarry blast";
	atts["maxDepth"] = "10";
	expected.push_back(GeoFeature("name1", NULL, 4, atts));
	atts.clear();

	atts["test1"] = "1,2,3";
	atts["test2"] = "1:2:3";
	atts["test3"] = "1,2,3";
	atts["test4"] = "1:2:3";
	expected.push_back(GeoFeature("name2", NULL, 5, atts));
	atts.clear();

	atts["test1"] = "1,2,3";
	atts["te:st"] = "bar";
	expected.push_back(GeoFeature("name3", NULL, 1, atts));
	atts.clear();

	BOOST_REQUIRE(features.features().size() == expected.size());
	vector<GeoFeature>::const_iterator exp_it = expected.begin();
	vector<GeoFeature*>::const_iterator got_it = features.features().begin();
	for ( ; exp_it != expected.end(); ++exp_it, ++got_it ) {
		BOOST_CHECK_EQUAL(exp_it->name(), (*got_it)->name());
		BOOST_CHECK_EQUAL(exp_it->rank(), (*got_it)->rank());
		bool success = true;
		GeoFeature::Attributes::const_iterator exp_att_it = exp_it->attributes().begin();
		GeoFeature::Attributes::const_iterator got_att_it;
		for ( ; exp_att_it != exp_it->attributes().end() && success; ++exp_att_it ) {
			got_att_it = (*got_it)->attributes().find(exp_att_it->first);
			success = got_att_it != (*got_it)->attributes().end() &&
			          exp_att_it->second == got_att_it->second;
			BOOST_CHECK(success);
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
