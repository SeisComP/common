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
#include <seiscomp/seismology/firstmotion.h>
#include <seiscomp/math/math.h>

#include <cmath>
#include <vector>


using namespace Seiscomp;
using namespace Seiscomp::Seismology;
using namespace Seiscomp::Math;


// ---------------------------------------------------------------------------
// Helper: generate synthetic polarities from a known mechanism.
// Uses multiple takeoff angles for better constraint.
// ---------------------------------------------------------------------------
std::vector<PolarityObservation> generatePolarities(
	double strike, double dip, double rake,
	int numAzimuths, double startAzimuth, double azimuthStep
) {
	NODAL_PLANE np;
	np.str = strike;
	np.dip = dip;
	np.rake = rake;

	// Use several takeoff angles to better constrain the solution
	double takeoffs[] = { 30.0, 50.0, 70.0 };

	std::vector<PolarityObservation> obs;
	int idx = 0;
	for ( double toff : takeoffs ) {
		for ( int i = 0; i < numAzimuths; ++i ) {
			double azi = startAzimuth + i * azimuthStep;
			if ( azi >= 360.0 ) azi -= 360.0;

			int pol = predictPolarity(np, azi, toff);
			if ( pol != 0 ) {
				obs.emplace_back(azi, toff, pol, idx);
			}
			++idx;
		}
	}
	return obs;
}


// ---------------------------------------------------------------------------
// Helper: minimum rotation angle between two nodal planes (via np2nd)
// ---------------------------------------------------------------------------
double npRotation(double str1, double dip1, double rake1,
                  double str2, double dip2, double rake2) {
	NODAL_PLANE np1, np2;
	np1.str = str1; np1.dip = dip1; np1.rake = rake1;
	np2.str = str2; np2.dip = dip2; np2.rake = rake2;

	Vector3d n1, d1, n2, d2;
	np2nd(np1, n1, d1);
	np2nd(np2, n2, d2);

	return mechanismRotation(n1, d1, n2, d2);
}


// ===========================================================================
// mechanismRotation tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(MechanismRotation)

BOOST_AUTO_TEST_CASE(identity) {
	// Same mechanism should give 0 rotation
	double rot = npRotation(0, 90, 0, 0, 90, 0);
	BOOST_CHECK_SMALL(rot, 0.1);
}

BOOST_AUTO_TEST_CASE(identity_thrust) {
	double rot = npRotation(45, 30, 90, 45, 30, 90);
	BOOST_CHECK_SMALL(rot, 0.1);
}

BOOST_AUTO_TEST_CASE(symmetry) {
	// rot(A,B) == rot(B,A)
	double rot1 = npRotation(0, 90, 0, 45, 60, -90);
	double rot2 = npRotation(45, 60, -90, 0, 90, 0);
	BOOST_CHECK_CLOSE(rot1, rot2, 0.1);
}

BOOST_AUTO_TEST_CASE(small_perturbation) {
	// A 5-degree change in strike should produce a small rotation
	double rot = npRotation(0, 90, 0, 5, 90, 0);
	BOOST_CHECK_LT(rot, 10.0);
	BOOST_CHECK_GT(rot, 0.1);
}

BOOST_AUTO_TEST_CASE(equivalent_planes) {
	// The auxiliary plane of a mechanism should give 0 rotation.
	// NP1: strike=0, dip=90, rake=0 (vertical left-lateral)
	// NP2 (auxiliary): strike=90, dip=90, rake=180
	double rot = npRotation(0, 90, 0, 90, 90, 180);
	BOOST_CHECK_SMALL(rot, 1.0);
}

BOOST_AUTO_TEST_CASE(orthogonal_mechanisms) {
	// Strike-slip (0/90/0) vs normal fault (0/45/-90) should be
	// a large rotation
	double rot = npRotation(0, 90, 0, 0, 45, -90);
	BOOST_CHECK_GT(rot, 30.0);
}

BOOST_AUTO_TEST_CASE(range_check) {
	// Result should always be in [0, 180]
	double rot = npRotation(10, 80, 170, 200, 30, -45);
	BOOST_CHECK_GE(rot, 0.0);
	BOOST_CHECK_LE(rot, 180.0);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// predictPolarity tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(PredictPolarity)

BOOST_AUTO_TEST_CASE(strike_slip_quadrants) {
	// Vertical left-lateral strike-slip: strike=0, dip=90, rake=0
	// Compressional quadrants: NE and SW
	// Dilatational quadrants: NW and SE
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;

	// NE quadrant (azimuth 45, takeoff 60) -> compressional
	BOOST_CHECK_EQUAL(predictPolarity(np, 45, 60), 1);
	// SW quadrant (azimuth 225, takeoff 60) -> compressional
	BOOST_CHECK_EQUAL(predictPolarity(np, 225, 60), 1);
	// NW quadrant (azimuth 315, takeoff 60) -> dilatational
	BOOST_CHECK_EQUAL(predictPolarity(np, 315, 60), -1);
	// SE quadrant (azimuth 135, takeoff 60) -> dilatational
	BOOST_CHECK_EQUAL(predictPolarity(np, 135, 60), -1);
}

BOOST_AUTO_TEST_CASE(thrust_fault) {
	// Pure thrust: strike=0, dip=45, rake=90
	// Should produce compression at most azimuths for steep takeoff
	NODAL_PLANE np;
	np.str = 0; np.dip = 45; np.rake = 90;

	int pol = predictPolarity(np, 0, 30);
	BOOST_CHECK(pol == 1 || pol == -1);  // just verify it's not nodal
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// computeAzimuthalGap tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(AzimuthalGap)

BOOST_AUTO_TEST_CASE(uniform_coverage) {
	// 12 stations every 30 degrees -> gap = 30
	std::vector<PolarityObservation> obs;
	for ( int i = 0; i < 12; ++i ) {
		obs.emplace_back(i * 30.0, 60.0, 1, i);
	}
	double gap = computeAzimuthalGap(obs);
	BOOST_CHECK_CLOSE(gap, 30.0, 0.1);
}

BOOST_AUTO_TEST_CASE(half_coverage) {
	// Stations from 0 to 180 every 30 degrees -> wrap gap = 180
	std::vector<PolarityObservation> obs;
	for ( int i = 0; i <= 6; ++i ) {
		obs.emplace_back(i * 30.0, 60.0, 1, i);
	}
	double gap = computeAzimuthalGap(obs);
	BOOST_CHECK_CLOSE(gap, 180.0, 0.1);
}

BOOST_AUTO_TEST_CASE(single_station) {
	std::vector<PolarityObservation> obs;
	obs.emplace_back(90.0, 60.0, 1, 0);
	double gap = computeAzimuthalGap(obs);
	BOOST_CHECK_CLOSE(gap, 360.0, 0.1);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// computeQualityGrade tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(QualityGrade)

BOOST_AUTO_TEST_CASE(grade_A) {
	FMQualityGrade g = computeQualityGrade(0.9, 20, 0.10, 0.6);
	BOOST_CHECK_EQUAL(g, FM_QUALITY_A);
}

BOOST_AUTO_TEST_CASE(grade_B) {
	FMQualityGrade g = computeQualityGrade(0.7, 30, 0.18, 0.45);
	BOOST_CHECK_EQUAL(g, FM_QUALITY_B);
}

BOOST_AUTO_TEST_CASE(grade_C) {
	FMQualityGrade g = computeQualityGrade(0.55, 40, 0.25, 0.35);
	BOOST_CHECK_EQUAL(g, FM_QUALITY_C);
}

BOOST_AUTO_TEST_CASE(grade_D) {
	FMQualityGrade g = computeQualityGrade(0.3, 60, 0.50, 0.1);
	BOOST_CHECK_EQUAL(g, FM_QUALITY_D);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// Full inversion tests with synthetic data
// ===========================================================================

BOOST_AUTO_TEST_SUITE(FullInversion)

BOOST_AUTO_TEST_CASE(recover_normal_fault) {
	// Normal fault: strike=45, dip=45, rake=-90
	// Well-constrained because the dipping fault plane produces
	// different polarity patterns at different takeoff angles.
	// 12 stations every 30 degrees, at multiple takeoff angles.
	auto obs = generatePolarities(45, 45, -90, 12, 0, 30);
	BOOST_REQUIRE_GE(obs.size(), FirstMotionInversion::MIN_OBSERVATIONS);

	FirstMotionInversion inv;
	inv.setGridSpacing(10);
	inv.setNumTrials(5);
	auto results = inv.compute(obs);

	BOOST_REQUIRE_EQUAL(results.size(), 1u);

	double rot = npRotation(
		results[0].np1.str, results[0].np1.dip, results[0].np1.rake,
		45, 45, -90
	);
	// With 10-degree grid and 5 trials, 45 degrees is reasonable
	BOOST_CHECK_LT(rot, 45.0);
}

BOOST_AUTO_TEST_CASE(recover_thrust_fault) {
	// Thrust fault: strike=0, dip=30, rake=90
	// 12 stations every 30 degrees, at multiple takeoff angles.
	auto obs = generatePolarities(0, 30, 90, 12, 0, 30);
	BOOST_REQUIRE_GE(obs.size(), FirstMotionInversion::MIN_OBSERVATIONS);

	FirstMotionInversion inv;
	inv.setGridSpacing(10);
	inv.setNumTrials(5);
	auto results = inv.compute(obs);

	BOOST_REQUIRE_EQUAL(results.size(), 1u);

	double rot = npRotation(
		results[0].np1.str, results[0].np1.dip, results[0].np1.rake,
		0, 30, 90
	);
	BOOST_CHECK_LT(rot, 35.0);
}

BOOST_AUTO_TEST_CASE(too_few_observations) {
	std::vector<PolarityObservation> obs;
	for ( int i = 0; i < 3; ++i ) {
		obs.emplace_back(i * 120.0, 60.0, 1, i);
	}

	FirstMotionInversion inv;
	auto results = inv.compute(obs);
	BOOST_CHECK(results.empty());
}

BOOST_AUTO_TEST_CASE(minimum_observations) {
	// Exactly 6 observations should produce a result
	std::vector<PolarityObservation> obs;
	obs.emplace_back(15.0, 60.0, 1, 0);
	obs.emplace_back(75.0, 60.0, 1, 1);
	obs.emplace_back(135.0, 60.0, -1, 2);
	obs.emplace_back(195.0, 60.0, -1, 3);
	obs.emplace_back(255.0, 60.0, 1, 4);
	obs.emplace_back(315.0, 60.0, -1, 5);

	FirstMotionInversion inv;
	inv.setGridSpacing(5);
	inv.setNumTrials(10);
	auto results = inv.compute(obs);

	// With exactly MIN_OBSERVATIONS it should still produce a result
	BOOST_CHECK_EQUAL(results.size(), 1u);
}

BOOST_AUTO_TEST_CASE(configuration_setters) {
	FirstMotionInversion inv;

	inv.setGridSpacing(10.0);
	BOOST_CHECK_CLOSE(inv.gridSpacing(), 10.0, 0.01);

	inv.setNumTrials(100);
	BOOST_CHECK_EQUAL(inv.numTrials(), 100);

	inv.setTakeoffUncertainty(15.0);
	BOOST_CHECK_CLOSE(inv.takeoffUncertainty(), 15.0, 0.01);

	inv.setAzimuthUncertainty(8.0);
	BOOST_CHECK_CLOSE(inv.azimuthUncertainty(), 8.0, 0.01);

	inv.setBadFraction(0.2);
	BOOST_CHECK_CLOSE(inv.badFraction(), 0.2, 0.01);

	// Invalid values should be rejected
	inv.setGridSpacing(-1.0);
	BOOST_CHECK_CLOSE(inv.gridSpacing(), 10.0, 0.01);

	inv.setGridSpacing(50.0);
	BOOST_CHECK_CLOSE(inv.gridSpacing(), 10.0, 0.01);

	inv.setNumTrials(-5);
	BOOST_CHECK_EQUAL(inv.numTrials(), 100);

	inv.setBadFraction(0.0);
	BOOST_CHECK_CLOSE(inv.badFraction(), 0.2, 0.01);
}

BOOST_AUTO_TEST_SUITE_END()
