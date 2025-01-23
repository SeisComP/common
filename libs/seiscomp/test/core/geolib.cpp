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

#include <seiscomp/math/geo.h>
#include <seiscomp/math/matrix3.h>
#include <seiscomp/geo/coordinate.h>
#include <seiscomp/geo/boundingbox.h>
#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/index/quadtree.h>


namespace bu = boost::unit_test;
using namespace std;
using namespace Seiscomp::Math;
using namespace Seiscomp::Geo;
using namespace Seiscomp::Math::Geo;


BOOST_AUTO_TEST_SUITE(seiscomp_core_geolib)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(distanceAndAzimuth) {
	double dist, az, baz;
	delazi(0.0, 0.0, 0.0, 0.0, &dist, &az, &baz);
	BOOST_CHECK_EQUAL(dist, 0.0);
}
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
BOOST_AUTO_TEST_CASE(latlon2xyz) {
	Vector3d v0, v1;

	ltp2vec(52, 12, 0, v0);
	ltp2vec(52, 12, -1000, v1);

	BOOST_CHECK_CLOSE((v0 - v1).length(), 1000, 1e-6);

	ltp2vec(52, 12, 1000, v1);

	BOOST_CHECK_CLOSE((v0 - v1).length(), 1000, 1e-6);

	ltp2vec(52, 12, -2000, v0);

	BOOST_CHECK_CLOSE((v0 - v1).length(), 3000, 1e-6);

	ltp2vec(52, 12, 0, v0);

	double lat, lon;
	delandaz2coord(1, 90, 52, 12, &lat, &lon);

	ltp2vec(lat, lon, -deg2km(1) * 1000, v1);

	auto down = Vector3d(0, 0, 0) - v0;
	down.normalize();
	BOOST_CHECK_CLOSE((v1 - v0).normalize().dot(down), 0.714, 0.1);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(mmult, *bu::tolerance(0.01)) {
	Matrix3d rh;
	rh.identity();
	rh[0][0] = -1.0;

	auto rotateZ = Matrix3d::RotationZ(deg2rad(90));
	auto trans = rotateZ * rh;

	BOOST_TEST(trans[0][0] == +0.0);
	BOOST_TEST(trans[0][1] == -1.0);
	BOOST_TEST(trans[0][2] == +0.0);
	BOOST_TEST(trans[1][0] == -1.0);
	BOOST_TEST(trans[1][1] == +0.0);
	BOOST_TEST(trans[1][2] == +0.0);
	BOOST_TEST(trans[2][0] == +0.0);
	BOOST_TEST(trans[2][1] == +0.0);
	BOOST_TEST(trans[2][2] == +1.0);

	rotateZ = Matrix3d::RotationZ(deg2rad(13));
	rh.identity();
	rh[0][0] = -1.0;

	Vector3d v1, v2;

	v1 = rotateZ * rh * Vector3d(1, 0, 0);

	v2 = rh * Vector3d(1, 0, 0);
	v2 = rotateZ * v2;

	BOOST_TEST(v1.x == v2.x);
	BOOST_TEST(v1.y == v2.y);
	BOOST_TEST(v1.z == v2.z);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(inclination, *bu::tolerance(0.005)) {
	Vector3d tgt, src;
	// Test from ZRT to LQT
	// ZRT is left-handed
	// LQT is right-handed

	// Convert into right-handed system
	Matrix3d rh;
	rh.identity();
	rh[1][1] = -1.0;

	//
	// Target is on top of Source (ray directly from beneath)
	// Inclination should be zero, meaning Z is L
	ltp2vec(52, 12, -1000, src);
	ltp2vec(52, 12, 0, tgt);

	auto thInclination = acos((src - tgt).normalize().dot(-tgt.normalized()));
	BOOST_TEST(thInclination == +0.0);

	// Validation vector: x is T, y is R, z is Z
	auto v = rh * Matrix3d::RotationX(+thInclination) * Vector3d(0, 1, 1);

	// Output vector in TQL
	BOOST_CHECK_CLOSE(v.x, +0.0, 1);
	BOOST_CHECK_CLOSE(v.y, -1.0, 1);
	BOOST_CHECK_CLOSE(v.z, +1.0, 1);

	//
	// Target is parallel to surface (ray directly from the side (west))
	// Inclination should be 90 degree / 1.5708 rad, meaning Z is Q, R is L
	ltp2vec(52, 12, 0, src);
	ltp2vec(52, 13, 0, tgt);

	thInclination = acos((src - tgt).normalize().dot(-tgt.normalized()));
	BOOST_CHECK_CLOSE(thInclination, +1.5708, 1);

	// Validation vector: x is T, y is R, z is Z
	v = rh * Matrix3d::RotationX(thInclination) * Vector3d(0, -1, 1);

	// Output vector in TQL
	BOOST_CHECK_CLOSE(v.x, +0.0, 1);
	BOOST_CHECK_CLOSE(v.y, +1.0, 1);
	BOOST_CHECK_CLOSE(v.z, -1.0, 1);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
