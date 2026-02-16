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


// ===========================================================================
// predictPolarity tests — external reference values
//
// These tests verify polarity predictions against hand-computed values
// using the textbook P-wave radiation pattern formula:
//
//   np2nd(strike, dip, rake) -> n, d   (Aki & Richards z-UP convention)
//   r = (sin(ih)*cos(phi), sin(ih)*sin(phi), -cos(ih))
//   amplitude = (n . r)(d . r)
//   polarity = sign(amplitude)
//
// Each test case documents the n, d, r vectors and the hand-computed
// amplitude so the expected polarity is independently verifiable.
// ===========================================================================

BOOST_AUTO_TEST_SUITE(PredictPolarity)

BOOST_AUTO_TEST_CASE(vertical_strike_slip_ne_quadrant) {
	// Strike=0, Dip=90, Rake=0 (vertical left-lateral on N-S fault)
	// np2nd: n=(0, 1, 0), d=(1, 0, 0)
	//
	// Ray at azi=45, takeoff=60 (downgoing):
	//   r = (sin(60)*cos(45), sin(60)*sin(45), -cos(60))
	//     = (0.612, 0.612, -0.5)
	//   n.r = 0*0.612 + 1*0.612 + 0*(-0.5) = 0.612
	//   d.r = 1*0.612 + 0*0.612 + 0*(-0.5) = 0.612
	//   amplitude = 0.612 * 0.612 = 0.375 > 0 -> compressional
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;
	BOOST_CHECK_EQUAL(predictPolarity(np, 45, 60), 1);
}

BOOST_AUTO_TEST_CASE(vertical_strike_slip_se_quadrant) {
	// Same mechanism: Strike=0, Dip=90, Rake=0
	// np2nd: n=(0, 1, 0), d=(1, 0, 0)
	//
	// Ray at azi=135, takeoff=60:
	//   r = (sin(60)*cos(135), sin(60)*sin(135), -cos(60))
	//     = (-0.612, 0.612, -0.5)
	//   n.r = 0.612
	//   d.r = -0.612
	//   amplitude = 0.612 * (-0.612) = -0.375 < 0 -> dilatational
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;
	BOOST_CHECK_EQUAL(predictPolarity(np, 135, 60), -1);
}

BOOST_AUTO_TEST_CASE(vertical_strike_slip_sw_quadrant) {
	// Same mechanism: Strike=0, Dip=90, Rake=0
	// np2nd: n=(0, 1, 0), d=(1, 0, 0)
	//
	// Ray at azi=225, takeoff=60:
	//   r = (-0.612, -0.612, -0.5)
	//   n.r = -0.612
	//   d.r = -0.612
	//   amplitude = (-0.612) * (-0.612) = 0.375 > 0 -> compressional
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;
	BOOST_CHECK_EQUAL(predictPolarity(np, 225, 60), 1);
}

BOOST_AUTO_TEST_CASE(vertical_strike_slip_nw_quadrant) {
	// Same mechanism: Strike=0, Dip=90, Rake=0
	// np2nd: n=(0, 1, 0), d=(1, 0, 0)
	//
	// Ray at azi=315, takeoff=60:
	//   r = (0.612, -0.612, -0.5)
	//   n.r = -0.612
	//   d.r = 0.612
	//   amplitude = -0.375 < 0 -> dilatational
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;
	BOOST_CHECK_EQUAL(predictPolarity(np, 315, 60), -1);
}

BOOST_AUTO_TEST_CASE(vertical_strike_slip_on_nodal_plane) {
	// Same mechanism: Strike=0, Dip=90, Rake=0
	// Ray at azi=0, takeoff=60 (on the N-S fault plane -> nodal):
	//   r = (0.866, 0, -0.5)
	//   n.r = 0  -> nodal
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;
	BOOST_CHECK_EQUAL(predictPolarity(np, 0, 60), 0);
}

BOOST_AUTO_TEST_CASE(thrust_fault_downgoing) {
	// Strike=0, Dip=45, Rake=90 (pure thrust)
	// np2nd:
	//   n.x = -sin(45)*sin(0) = 0
	//   n.y = sin(45)*cos(0) = 0.707
	//   n.z = -cos(45) = -0.707
	//   d.x = cos(90)*cos(0) + cos(45)*sin(90)*sin(0) = 0
	//   d.y = cos(90)*sin(0) - cos(45)*sin(90)*cos(0) = -0.707
	//   d.z = -sin(45)*sin(90) = -0.707
	//
	// Ray at azi=0, takeoff=20 (steep downgoing to north):
	//   r = (sin(20)*cos(0), sin(20)*sin(0), -cos(20))
	//     = (0.342, 0, -0.940)
	//   n.r = 0 + 0 + (-0.707)*(-0.940) = 0.664
	//   d.r = 0 + 0 + (-0.707)*(-0.940) = 0.664
	//   amplitude = 0.664 * 0.664 = 0.441 > 0 -> compressional
	NODAL_PLANE np;
	np.str = 0; np.dip = 45; np.rake = 90;
	BOOST_CHECK_EQUAL(predictPolarity(np, 0, 20), 1);
}

BOOST_AUTO_TEST_CASE(normal_fault_steep_downgoing_east) {
	// Strike=0, Dip=45, Rake=-90 (pure normal fault)
	// np2nd:
	//   n.x = -sin(45)*sin(0) = 0
	//   n.y = sin(45)*cos(0) = 0.707
	//   n.z = -cos(45) = -0.707
	//   d.x = cos(-90)*cos(0) + cos(45)*sin(-90)*sin(0) = 0
	//   d.y = cos(-90)*sin(0) - cos(45)*sin(-90)*cos(0) = 0.707
	//   d.z = -sin(45)*sin(-90) = 0.707
	//
	// Ray at azi=90, takeoff=20 (steep downgoing to east):
	//   r = (sin(20)*cos(90), sin(20)*sin(90), -cos(20))
	//     = (0, 0.342, -0.940)
	//   n.r = 0.707*0.342 + (-0.707)*(-0.940) = 0.242 + 0.664 = 0.906
	//   d.r = 0.707*0.342 + 0.707*(-0.940) = 0.242 - 0.664 = -0.422
	//   amplitude = 0.906 * (-0.422) = -0.382 < 0 -> dilatational
	NODAL_PLANE np;
	np.str = 0; np.dip = 45; np.rake = -90;
	BOOST_CHECK_EQUAL(predictPolarity(np, 90, 20), -1);
}

BOOST_AUTO_TEST_CASE(normal_fault_horizontal_east) {
	// Same mechanism: Strike=0, Dip=45, Rake=-90
	// n=(0, 0.707, -0.707), d=(0, 0.707, 0.707)
	//
	// Ray at azi=90, takeoff=90 (horizontal to east):
	//   r = (0, 1, 0)
	//   n.r = 0.707
	//   d.r = 0.707
	//   amplitude = 0.500 > 0 -> compressional
	NODAL_PLANE np;
	np.str = 0; np.dip = 45; np.rake = -90;
	BOOST_CHECK_EQUAL(predictPolarity(np, 90, 90), 1);
}

BOOST_AUTO_TEST_CASE(oblique_fault_z_sensitive) {
	// Strike=45, Dip=60, Rake=30
	// This mechanism produces different polarities depending on whether
	// rz = -cos(ih) (correct, z-UP) or rz = +cos(ih) (wrong, z-DOWN).
	//
	// np2nd:
	//   n.x = -sin(60)*sin(45) = -0.612
	//   n.y = sin(60)*cos(45) = 0.612
	//   n.z = -cos(60) = -0.500
	//   d.x = cos(30)*cos(45) + cos(60)*sin(30)*sin(45) = 0.789
	//   d.y = cos(30)*sin(45) - cos(60)*sin(30)*cos(45) = 0.436
	//   d.z = -sin(60)*sin(30) = -0.433
	//
	// Ray at azi=0, takeoff=30:
	//   r = (sin(30)*cos(0), sin(30)*sin(0), -cos(30))
	//     = (0.500, 0, -0.866)
	//   n.r = (-0.612)*0.500 + 0.612*0 + (-0.500)*(-0.866) = -0.306 + 0.433 = 0.127
	//   d.r = 0.789*0.500 + 0.436*0 + (-0.433)*(-0.866) = 0.395 + 0.375 = 0.770
	//   amplitude = 0.127 * 0.770 = 0.098 > 0 -> compressional
	//
	// With WRONG rz = +cos(30) = 0.866:
	//   n.r = -0.306 + (-0.500)*0.866 = -0.306 - 0.433 = -0.739
	//   d.r = 0.395 + (-0.433)*0.866 = 0.395 - 0.375 = 0.020
	//   amplitude = -0.739 * 0.020 = -0.015 < 0 -> dilatational (WRONG)
	NODAL_PLANE np;
	np.str = 45; np.dip = 60; np.rake = 30;
	BOOST_CHECK_EQUAL(predictPolarity(np, 0, 30), 1);
}

BOOST_AUTO_TEST_CASE(oblique_fault_z_sensitive_2) {
	// Same mechanism: Strike=45, Dip=60, Rake=30
	//
	// Ray at azi=210, takeoff=40:
	//   r = (sin(40)*cos(210), sin(40)*sin(210), -cos(40))
	//     = (-0.557, -0.321, -0.766)
	//   n.r = (-0.612)*(-0.557) + 0.612*(-0.321) + (-0.500)*(-0.766)
	//       = 0.341 - 0.196 + 0.383 = 0.528
	//   d.r = 0.789*(-0.557) + 0.436*(-0.321) + (-0.433)*(-0.766)
	//       = -0.440 - 0.140 + 0.332 = -0.248
	//   amplitude = 0.528 * (-0.248) = -0.131 < 0 -> dilatational
	NODAL_PLANE np;
	np.str = 45; np.dip = 60; np.rake = 30;
	BOOST_CHECK_EQUAL(predictPolarity(np, 210, 40), -1);
}

BOOST_AUTO_TEST_CASE(upgoing_ray) {
	// Verify upgoing rays work correctly (takeoff > 90).
	// Strike=0, Dip=90, Rake=0 (vertical strike-slip)
	// n=(0, 1, 0), d=(1, 0, 0)
	//
	// Ray at azi=45, takeoff=150 (upgoing):
	//   r = (sin(150)*cos(45), sin(150)*sin(45), -cos(150))
	//     = (0.354, 0.354, 0.866)
	//   n.r = 0.354
	//   d.r = 0.354
	//   amplitude = 0.125 > 0 -> compressional
	// (Same as downgoing NE quadrant — the radiation pattern is symmetric)
	NODAL_PLANE np;
	np.str = 0; np.dip = 90; np.rake = 0;
	BOOST_CHECK_EQUAL(predictPolarity(np, 45, 150), 1);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// computeAzimuthalGap tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(AzimuthalGap)

BOOST_AUTO_TEST_CASE(uniform_coverage) {
	std::vector<PolarityObservation> obs;
	for ( int i = 0; i < 12; ++i ) {
		obs.emplace_back(i * 30.0, 60.0, 1, i);
	}
	double gap = computeAzimuthalGap(obs);
	BOOST_CHECK_CLOSE(gap, 30.0, 0.1);
}

BOOST_AUTO_TEST_CASE(half_coverage) {
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

BOOST_AUTO_TEST_CASE(empty_observations) {
	std::vector<PolarityObservation> obs;
	double gap = computeAzimuthalGap(obs);
	BOOST_CHECK_CLOSE(gap, 360.0, 0.1);
}

BOOST_AUTO_TEST_CASE(negative_azimuths_normalized) {
	// Azimuths with negative values should be normalized to [0, 360)
	std::vector<PolarityObservation> obs;
	obs.emplace_back(-10.0, 60.0, 1, 0);   // = 350
	obs.emplace_back(10.0, 60.0, 1, 1);
	double gap = computeAzimuthalGap(obs);
	BOOST_CHECK_CLOSE(gap, 340.0, 0.1);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// assessQuality tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(AssessQuality)

BOOST_AUTO_TEST_CASE(quality_A) {
	FMSolution sol;
	sol.misfit = 0.05;
	sol.azimuthalGap = 60.0;
	sol.stationCount = 12;
	BOOST_CHECK(assessQuality(sol) == FMQuality::A);
}

BOOST_AUTO_TEST_CASE(quality_B) {
	FMSolution sol;
	sol.misfit = 0.15;
	sol.azimuthalGap = 120.0;
	sol.stationCount = 9;
	BOOST_CHECK(assessQuality(sol) == FMQuality::B);
}

BOOST_AUTO_TEST_CASE(quality_C) {
	FMSolution sol;
	sol.misfit = 0.25;
	sol.azimuthalGap = 190.0;
	sol.stationCount = 7;
	BOOST_CHECK(assessQuality(sol) == FMQuality::C);
}

BOOST_AUTO_TEST_CASE(quality_D_high_misfit) {
	FMSolution sol;
	sol.misfit = 0.40;
	sol.azimuthalGap = 60.0;
	sol.stationCount = 12;
	BOOST_CHECK(assessQuality(sol) == FMQuality::D);
}

BOOST_AUTO_TEST_CASE(quality_D_huge_gap) {
	FMSolution sol;
	sol.misfit = 0.05;
	sol.azimuthalGap = 250.0;
	sol.stationCount = 12;
	BOOST_CHECK(assessQuality(sol) == FMQuality::D);
}

BOOST_AUTO_TEST_CASE(quality_D_too_few_stations) {
	FMSolution sol;
	sol.misfit = 0.0;
	sol.azimuthalGap = 60.0;
	sol.stationCount = 4;
	BOOST_CHECK(assessQuality(sol) == FMQuality::D);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// FMInversionConfig::validate tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(ConfigValidation)

BOOST_AUTO_TEST_CASE(valid_config_unchanged) {
	FMInversionConfig config;
	config.gridSpacing = 5.0;
	config.maxMisfitFraction = 0.2;
	BOOST_CHECK(config.validate() == true);
	BOOST_CHECK_CLOSE(config.gridSpacing, 5.0, 0.01);
	BOOST_CHECK_CLOSE(config.maxMisfitFraction, 0.2, 0.01);
}

BOOST_AUTO_TEST_CASE(grid_spacing_too_small) {
	FMInversionConfig config;
	config.gridSpacing = 0.1;
	BOOST_CHECK(config.validate() == false);
	BOOST_CHECK_CLOSE(config.gridSpacing, 1.0, 0.01);
}

BOOST_AUTO_TEST_CASE(grid_spacing_too_large) {
	FMInversionConfig config;
	config.gridSpacing = 100.0;
	BOOST_CHECK(config.validate() == false);
	BOOST_CHECK_CLOSE(config.gridSpacing, 30.0, 0.01);
}

BOOST_AUTO_TEST_CASE(misfit_fraction_negative) {
	FMInversionConfig config;
	config.maxMisfitFraction = -0.5;
	BOOST_CHECK(config.validate() == false);
	BOOST_CHECK_CLOSE(config.maxMisfitFraction, 0.0, 0.01);
}

BOOST_AUTO_TEST_CASE(misfit_fraction_too_large) {
	FMInversionConfig config;
	config.maxMisfitFraction = 0.9;
	BOOST_CHECK(config.validate() == false);
	BOOST_CHECK_CLOSE(config.maxMisfitFraction, 0.5, 0.01);
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// invertPolarities tests — synthetic data with known mechanisms
// ===========================================================================

BOOST_AUTO_TEST_SUITE(InvertPolarities)

BOOST_AUTO_TEST_CASE(too_few_observations) {
	std::vector<PolarityObservation> obs;
	for ( int i = 0; i < 3; ++i ) {
		obs.emplace_back(i * 120.0, 60.0, 1, i);
	}

	auto result = invertPolarities(obs);
	BOOST_CHECK(!result.valid);
	BOOST_CHECK(result.accepted.empty());
}

BOOST_AUTO_TEST_CASE(vertical_strike_slip_recovery) {
	// Generate polarities for Strike=0, Dip=90, Rake=0
	// using the known radiation pattern (NE/SW = compressional, SE/NW = dilatational)
	// These are hand-verified values, not generated by our own predictPolarity.
	std::vector<PolarityObservation> obs;
	// Compressional (NE and SW quadrants)
	obs.emplace_back(30.0, 50.0, 1, 0);   // NE
	obs.emplace_back(60.0, 50.0, 1, 1);   // NE
	obs.emplace_back(210.0, 50.0, 1, 2);  // SW
	obs.emplace_back(240.0, 50.0, 1, 3);  // SW
	// Dilatational (SE and NW quadrants)
	obs.emplace_back(120.0, 50.0, -1, 4); // SE
	obs.emplace_back(150.0, 50.0, -1, 5); // SE
	obs.emplace_back(300.0, 50.0, -1, 6); // NW
	obs.emplace_back(330.0, 50.0, -1, 7); // NW

	FMInversionConfig config;
	config.gridSpacing = 5;
	config.maxMisfitFraction = 0.2;

	auto result = invertPolarities(obs, config);
	BOOST_REQUIRE(result.valid);

	// The recovered mechanism should have 0 misfits (perfect data)
	BOOST_CHECK_EQUAL(result.best.misfitCount, 0);
	BOOST_CHECK_GT(result.best.stationCount, 0);

	// Quality should be assigned
	BOOST_CHECK(result.best.quality != FMQuality::D);

	// Accepted solutions should include the best
	BOOST_CHECK_GE(result.accepted.size(), 1u);

	// Nodal plane angles should be in valid degree ranges
	BOOST_CHECK_GE(result.best.np1.dip, 0.0);
	BOOST_CHECK_LE(result.best.np1.dip, 90.0);
	BOOST_CHECK_GE(result.best.np2.dip, 0.0);
	BOOST_CHECK_LE(result.best.np2.dip, 90.0);
}

BOOST_AUTO_TEST_CASE(nd2dc_returns_degrees_not_radians) {
	// Regression test: nd2dc already converts to degrees internally.
	// The old code applied rad2deg() again, producing values like 5156
	// instead of 90 for dip. Verify angles are in valid ranges.
	std::vector<PolarityObservation> obs;
	obs.emplace_back(30.0, 50.0, 1, 0);
	obs.emplace_back(60.0, 50.0, 1, 1);
	obs.emplace_back(210.0, 50.0, 1, 2);
	obs.emplace_back(240.0, 50.0, 1, 3);
	obs.emplace_back(120.0, 50.0, -1, 4);
	obs.emplace_back(150.0, 50.0, -1, 5);
	obs.emplace_back(300.0, 50.0, -1, 6);
	obs.emplace_back(330.0, 50.0, -1, 7);

	auto result = invertPolarities(obs);
	BOOST_REQUIRE(result.valid);

	// Strike must be [0, 360), dip [0, 90], rake [-180, 180]
	BOOST_CHECK_GE(result.best.np1.str, 0.0);
	BOOST_CHECK_LT(result.best.np1.str, 360.0);
	BOOST_CHECK_GE(result.best.np1.dip, 0.0);
	BOOST_CHECK_LE(result.best.np1.dip, 90.0);
	BOOST_CHECK_GE(result.best.np1.rake, -180.0);
	BOOST_CHECK_LE(result.best.np1.rake, 180.0);

	BOOST_CHECK_GE(result.best.np2.str, 0.0);
	BOOST_CHECK_LT(result.best.np2.str, 360.0);
	BOOST_CHECK_GE(result.best.np2.dip, 0.0);
	BOOST_CHECK_LE(result.best.np2.dip, 90.0);
	BOOST_CHECK_GE(result.best.np2.rake, -180.0);
	BOOST_CHECK_LE(result.best.np2.rake, 180.0);
}

BOOST_AUTO_TEST_CASE(accepted_solutions_sorted_by_misfit) {
	// With one wrong polarity, solutions with 0 and 1 misfit should appear
	std::vector<PolarityObservation> obs;
	obs.emplace_back(30.0, 50.0, 1, 0);
	obs.emplace_back(60.0, 50.0, 1, 1);
	obs.emplace_back(210.0, 50.0, 1, 2);
	obs.emplace_back(240.0, 50.0, 1, 3);
	obs.emplace_back(120.0, 50.0, -1, 4);
	obs.emplace_back(150.0, 50.0, -1, 5);
	obs.emplace_back(300.0, 50.0, -1, 6);
	// Deliberately wrong polarity for NW quadrant
	obs.emplace_back(330.0, 50.0, 1, 7);

	FMInversionConfig config;
	config.gridSpacing = 5;
	config.maxMisfitFraction = 0.2;  // allows 1 out of 8

	auto result = invertPolarities(obs, config);
	BOOST_REQUIRE(result.valid);

	// Accepted solutions should be sorted: best misfit first
	for ( size_t i = 1; i < result.accepted.size(); ++i ) {
		BOOST_CHECK_LE(result.accepted[i - 1].misfitCount,
		               result.accepted[i].misfitCount);
	}

	// Best should identify the misfitting station
	if ( result.best.misfitCount > 0 ) {
		BOOST_CHECK(!result.best.misfittingStations.empty());
	}
}

BOOST_AUTO_TEST_SUITE_END()


// ===========================================================================
// qualityLabel tests
// ===========================================================================

BOOST_AUTO_TEST_SUITE(QualityLabel)

BOOST_AUTO_TEST_CASE(all_labels_non_null) {
	BOOST_CHECK(qualityLabel(FMQuality::A) != nullptr);
	BOOST_CHECK(qualityLabel(FMQuality::B) != nullptr);
	BOOST_CHECK(qualityLabel(FMQuality::C) != nullptr);
	BOOST_CHECK(qualityLabel(FMQuality::D) != nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
