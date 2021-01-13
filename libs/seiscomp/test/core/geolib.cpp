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


#define SEISCOMP_TEST_MODULE SeisComP
#include <seiscomp/unittest/unittests.h>

#include <seiscomp/geo/coordinate.h>
#include <seiscomp/geo/boundingbox.h>
#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/index/quadtree.h>


namespace bu = boost::unit_test;
using namespace std;
using namespace Seiscomp::Geo;


BOOST_AUTO_TEST_SUITE(seiscomp_core_geolib)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(coords) {
	BOOST_CHECK_EQUAL(GeoCoordinate::width(0,10), 10);
	BOOST_CHECK_EQUAL(GeoCoordinate::width(-10,10), 20);
	BOOST_CHECK_EQUAL(GeoCoordinate::width(10,-10), 340);
	BOOST_CHECK_EQUAL(GeoCoordinate::width(-400,400), 360);

	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(0), 0);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(10), 10);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(-10), -10);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(100), 80);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(180), 0);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(270), -90);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(300), -60);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(-100), -80);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(-180), 0);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(-270), 90);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLat(-300), 60);

	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(0), 0);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(-90), -90);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(90), 90);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(200), -160);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(-200), 160);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(400), 40);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(-400), -40);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(3630), 30);
	BOOST_CHECK_EQUAL(GeoCoordinate::normalizeLon(-3630), -30);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(boundingbox) {
	GeoBoundingBox bbox(-20, -30, 20, 30);
	BOOST_CHECK_EQUAL(bbox.height(), 40);
	BOOST_CHECK_EQUAL(bbox.width(), 60);

	bbox.merge(GeoBoundingBox(-20, -30, 20, 30));
	BOOST_CHECK_EQUAL(bbox.south, -20);
	BOOST_CHECK_EQUAL(bbox.north, 20);
	BOOST_CHECK_EQUAL(bbox.west, -30);
	BOOST_CHECK_EQUAL(bbox.east, 30);

	bbox = GeoBoundingBox(0, 0, 25, 50);
	bbox.merge(GeoBoundingBox(-25, -50, 0, 0));
	BOOST_CHECK_EQUAL(bbox.south, -25);
	BOOST_CHECK_EQUAL(bbox.north, 25);
	BOOST_CHECK_EQUAL(bbox.west, -50);
	BOOST_CHECK_EQUAL(bbox.east, 50);

	bbox = GeoBoundingBox(0, 155, 25, -155);
	bbox.merge(GeoBoundingBox(0, 100, 10, 125));
	BOOST_CHECK_EQUAL(bbox.south, 0);
	BOOST_CHECK_EQUAL(bbox.north, 25);
	BOOST_CHECK_EQUAL(bbox.west, 100);
	BOOST_CHECK_EQUAL(bbox.east, -155);
	BOOST_CHECK_EQUAL(bbox.width(), 105);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);

	bbox.merge(GeoBoundingBox(0, -155, 10, 100));
	BOOST_CHECK_EQUAL(bbox.south, 0);
	BOOST_CHECK_EQUAL(bbox.north, 25);
	BOOST_CHECK_EQUAL(bbox.west, -180);
	BOOST_CHECK_EQUAL(bbox.east, 180);
	BOOST_CHECK_EQUAL(bbox.width(), 360);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), true);

	for ( size_t i = 0; i < 1000; ++i ) {
		GeoCoordinate::ValueType lat = -180.0+i*360.0/(1000-1);
		BOOST_CHECK_EQUAL(bbox.contains(GeoCoordinate(5, lat)), true);
	}

	bbox = GeoBoundingBox(-20, 30, 20, -30);
	BOOST_CHECK_EQUAL(bbox.width(), 300);
	bbox.merge(GeoBoundingBox(0, -180, 1, 180));
	BOOST_CHECK_EQUAL(bbox.width(), 360);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), true);
	BOOST_CHECK_EQUAL(bbox.west, -180);
	BOOST_CHECK_EQUAL(bbox.east, 180);

	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, -30, 20, 30) & GeoBoundingBox(-20, -30, 20, 30), true);
	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, -30, 20, 30) & GeoBoundingBox(-20, 30, 20, 40), false);
	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, -30, 20, 30) & GeoBoundingBox(-20, 20, 20, 40), true);

	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, -30, 20, 30) & GeoBoundingBox(20, -30, 40, 30), false);
	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, -30, 20, 30) & GeoBoundingBox(-40, -30, -20, 30), false);
	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, -30, 20, 30) & GeoBoundingBox(10, -30, 30, 30), true);

	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, 170, 20, -170) & GeoBoundingBox(-10, 180, 10, -160), true);
	BOOST_CHECK_EQUAL(GeoBoundingBox(-20, 170, 20, -170) & GeoBoundingBox(-30, 160, 30, -160), true);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 20, 20, -20));
	BOOST_CHECK_EQUAL(bbox.width(), 350);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 30, 20, -20));
	BOOST_CHECK_EQUAL(bbox.width(), 340);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);
	BOOST_CHECK_EQUAL(bbox.west, 30);
	BOOST_CHECK_EQUAL(bbox.east, 10);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 20, 20, -30));
	BOOST_CHECK_EQUAL(bbox.width(), 340);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);
	BOOST_CHECK_EQUAL(bbox.west, -10);
	BOOST_CHECK_EQUAL(bbox.east, -30);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 5, 20, -11));
	BOOST_CHECK_EQUAL(bbox.width(), 359);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);
	BOOST_CHECK_EQUAL(bbox.west, -10);
	BOOST_CHECK_EQUAL(bbox.east, -11);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 9, 20, -15));
	BOOST_CHECK_EQUAL(bbox.width(), 355);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);
	BOOST_CHECK_EQUAL(bbox.west, -10);
	BOOST_CHECK_EQUAL(bbox.east, -15);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 11, 20, -5));
	BOOST_CHECK_EQUAL(bbox.width(), 359);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);
	BOOST_CHECK_EQUAL(bbox.west, 11);
	BOOST_CHECK_EQUAL(bbox.east, 10);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	bbox.merge(GeoBoundingBox(-20, 15, 20, -11));
	BOOST_CHECK_EQUAL(bbox.width(), 355);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);
	BOOST_CHECK_EQUAL(bbox.west, 15);
	BOOST_CHECK_EQUAL(bbox.east, 10);

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	BOOST_CHECK(bbox.contains(GeoBoundingBox(-20, -10, 20, 10)));

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	BOOST_CHECK(!bbox.contains(GeoBoundingBox(-19, -10, 21, 10)));

	bbox = GeoBoundingBox(-20, -10, 20, 10);
	BOOST_CHECK(!bbox.contains(GeoBoundingBox(-20, 9, 20, -9)));

	BOOST_CHECK(bbox.center() == GeoCoordinate(0,0));

	BOOST_CHECK(GeoBoundingBox(-90, -180, 90, 180).center() == GeoCoordinate(0,0));
	BOOST_CHECK(GeoBoundingBox(-90, -180, 0, 0).center() == GeoCoordinate(-45,-90));
	BOOST_CHECK(GeoBoundingBox(0, -180, 90, 0).center() == GeoCoordinate(45,-90));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(polybox) {
	GeoBoundingBox bbox;
	GeoCoordinate coords[4];

	coords[0] = GeoCoordinate(0,0);
	coords[1] = GeoCoordinate(10,0);
	coords[2] = GeoCoordinate(10,10);
	coords[3] = GeoCoordinate(0,10);

	bbox.fromPolygon(4, coords);
	BOOST_CHECK_EQUAL(bbox.south, 0);
	BOOST_CHECK_EQUAL(bbox.north, 10);
	BOOST_CHECK_EQUAL(bbox.west, 0);
	BOOST_CHECK_EQUAL(bbox.east, 10);
	BOOST_CHECK_EQUAL(bbox.width(), 10);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);

	coords[3] = GeoCoordinate(0,180);
	bbox.fromPolygon(4, coords);
	BOOST_CHECK_EQUAL(bbox.width(), 180);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);

	coords[3] = GeoCoordinate(0,-171);
	bbox.fromPolygon(4, coords);
	BOOST_CHECK_EQUAL(bbox.width(), 360);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), true);

	bbox.merge(GeoBoundingBox(0,-179,1,20));
	BOOST_CHECK_EQUAL(bbox.width(), 360);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), true);

	coords[3] = GeoCoordinate(0,179);
	bbox.fromPolygon(4, coords);
	bbox.merge(GeoBoundingBox(0,179,1,20));
	BOOST_CHECK_EQUAL(bbox.width(), 360);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(feature) {
	GeoFeature feature;
	feature.setClosedPolygon(true);

	feature.addVertex(GeoCoordinate(0,0));
	feature.addVertex(GeoCoordinate(0,10));
	feature.addVertex(GeoCoordinate(10,10));
	feature.addVertex(GeoCoordinate(10,0));

	feature.addVertex(GeoCoordinate(1,1), true);
	feature.addVertex(GeoCoordinate(2,1));
	feature.addVertex(GeoCoordinate(2,2));
	feature.addVertex(GeoCoordinate(1,2));

	BOOST_CHECK_EQUAL(feature.area(), 99);

	feature.addVertex(GeoCoordinate(0,20), true);
	feature.addVertex(GeoCoordinate(0,30));
	feature.addVertex(GeoCoordinate(10,30));
	feature.addVertex(GeoCoordinate(10,20));

	BOOST_CHECK_EQUAL(feature.area(), 199);

	feature.updateBoundingBox();

	const GeoBoundingBox &bbox = feature.bbox();
	BOOST_CHECK_EQUAL(bbox.south, 0);
	BOOST_CHECK_EQUAL(bbox.north, 10);
	BOOST_CHECK_EQUAL(bbox.west, 0);
	BOOST_CHECK_EQUAL(bbox.east, 30);
	BOOST_CHECK_EQUAL(bbox.width(), 30);
	BOOST_CHECK_EQUAL(bbox.coversFullLongitude(), false);

	BOOST_CHECK_EQUAL(bbox.contains(GeoCoordinate(5,5)), true);
	BOOST_CHECK_EQUAL(feature.contains(GeoCoordinate(1.5,1.5)), false);
	BOOST_CHECK_EQUAL(feature.contains(GeoCoordinate(0.5,0.5)), true);
	BOOST_CHECK_EQUAL(feature.contains(GeoCoordinate(5,5)), true);
	BOOST_CHECK_EQUAL(feature.contains(GeoCoordinate(5,25)), true);
	BOOST_CHECK_EQUAL(feature.contains(GeoCoordinate(5,15)), false);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
struct QtVisitor {
	QtVisitor() : depth(0) {}

	bool accept(const GeoBoundingBox &bb) const {
		BOOST_CHECK(bb.west == -180);
		BOOST_CHECK(bb.east == 180);
		BOOST_CHECK(bb.north == 90);
		BOOST_CHECK(bb.south == -90);
		++depth;
		return true;
	}

	bool visit(const GeoFeature *f) const {
		BOOST_CHECK(depth == 1);
		BOOST_CHECK(f->name() == "f1" || f->name() == "f2");
		return true;
	}

	mutable int depth;
};


BOOST_AUTO_TEST_CASE(quadtree) {
	GeoFeature f1(string("f1"), nullptr, 1);
	GeoFeature f2(string("f2"), nullptr, 1);

	f1.addVertex(GeoCoordinate(0,0));
	f1.addVertex(GeoCoordinate(0,10));
	f1.addVertex(GeoCoordinate(10,10));
	f1.addVertex(GeoCoordinate(10,0));
	f1.setClosedPolygon(true);

	f2.addVertex(GeoCoordinate(1,1));
	f2.addVertex(GeoCoordinate(2,1));
	f2.addVertex(GeoCoordinate(2,2));
	f2.addVertex(GeoCoordinate(1,2));
	f2.setClosedPolygon(true);

	// A quadtree covers the full region
	QuadTree qt;
	qt.addItem(&f1);
	qt.addItem(&f2);

	qt.accept(QtVisitor());
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
