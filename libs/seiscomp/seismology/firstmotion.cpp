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


#define SEISCOMP_COMPONENT FirstMotion

#include <seiscomp/seismology/firstmotion.h>
#include <seiscomp/math/math.h>
#include <seiscomp/logging/log.h>

#include <algorithm>
#include <cmath>
#include <random>


namespace Seiscomp {
namespace Seismology {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const int FirstMotionInversion::MIN_OBSERVATIONS;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *qualityGradeString(FMQualityGrade grade) {
	switch ( grade ) {
		case FM_QUALITY_A: return "A";
		case FM_QUALITY_B: return "B";
		case FM_QUALITY_C: return "C";
		case FM_QUALITY_D: return "D";
		default: return "?";
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FMQualityGrade computeQualityGrade(
	double misfit, double stdr, double azimuthalGap, int stationCount
) {
	// HASH-style quality grades (Hardebeck & Shearer 2002, Table 1)
	// A: misfit <= 0.15, STDR >= 0.5, gap <= 90, stations >= 8
	// B: misfit <= 0.20, STDR >= 0.4, gap <= 120, stations >= 8
	// C: misfit <= 0.30, STDR >= 0.3, gap <= 150, stations >= 6
	// D: anything worse
	if ( misfit <= 0.15 && stdr >= 0.5 && azimuthalGap <= 90 && stationCount >= 8 )
		return FM_QUALITY_A;
	if ( misfit <= 0.20 && stdr >= 0.4 && azimuthalGap <= 120 && stationCount >= 8 )
		return FM_QUALITY_B;
	if ( misfit <= 0.30 && stdr >= 0.3 && azimuthalGap <= 150 && stationCount >= 6 )
		return FM_QUALITY_C;
	return FM_QUALITY_D;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int predictPolarity(
	const Math::NODAL_PLANE &np,
	double azimuth, double takeoff
) {
	// Convert nodal plane to fault normal and slip vector
	Math::Vector3d n, d;
	Math::NODAL_PLANE npRad;
	npRad.str = deg2rad(np.str);
	npRad.dip = deg2rad(np.dip);
	npRad.rake = deg2rad(np.rake);
	Math::np2nd(npRad, n, d);

	// Compute the ray direction vector in Cartesian coordinates
	// Convention: x=North, y=East, z=Down
	// For a ray leaving the source at azimuth phi and takeoff angle ih:
	//   ray = (sin(ih)*cos(phi), sin(ih)*sin(phi), -cos(ih))
	// Note: takeoff angle is measured from downward vertical
	double ih = deg2rad(takeoff);
	double phi = deg2rad(azimuth);

	double rx = sin(ih) * cos(phi);
	double ry = sin(ih) * sin(phi);
	double rz = -cos(ih);

	// P-wave radiation pattern: A_P proportional to (n . r)(d . r)
	// Sign gives first motion polarity.
	double nr = n.x * rx + n.y * ry + n.z * rz;
	double dr = d.x * rx + d.y * ry + d.z * rz;
	double amplitude = nr * dr;

	if ( amplitude > 0 )
		return 1;   // compressional (up)
	else if ( amplitude < 0 )
		return -1;  // dilatational (down)
	else
		return 0;   // nodal
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FirstMotionInversion::FirstMotionInversion()
: _gridSpacing(5.0)
, _numTrials(100)
, _angleUncertainty(10.0) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setGridSpacing(double degrees) {
	if ( degrees > 0 && degrees <= 30 )
		_gridSpacing = degrees;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setNumTrials(int n) {
	if ( n > 0 && n <= 10000 )
		_numTrials = n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setAngleUncertainty(double degrees) {
	if ( degrees >= 0 && degrees <= 45 )
		_angleUncertainty = degrees;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double FirstMotionInversion::computeMisfit(
	const Math::NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	if ( observations.empty() ) return 1.0;

	int misfits = 0;
	for ( const auto &obs : observations ) {
		int predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		if ( predicted != 0 && predicted != obs.polarity )
			++misfits;
	}

	return static_cast<double>(misfits) / static_cast<double>(observations.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double FirstMotionInversion::computeSTDR(
	const Math::NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	// Station Distribution Ratio (STDR) measures how well stations
	// sample both compressional and dilatational quadrants.
	// Reasenberg & Oppenheimer (1985): STDR near 1 = good sampling,
	// near 0 = all stations in one quadrant.

	Math::Vector3d n, d;
	Math::NODAL_PLANE npRad;
	npRad.str = deg2rad(np.str);
	npRad.dip = deg2rad(np.dip);
	npRad.rake = deg2rad(np.rake);
	Math::np2nd(npRad, n, d);

	double sumAbs = 0;
	double sumSigned = 0;

	for ( const auto &obs : observations ) {
		double ih = deg2rad(obs.takeoff);
		double phi = deg2rad(obs.azimuth);

		double rx = sin(ih) * cos(phi);
		double ry = sin(ih) * sin(phi);
		double rz = -cos(ih);

		double nr = n.x * rx + n.y * ry + n.z * rz;
		double dr = d.x * rx + d.y * ry + d.z * rz;
		double amp = nr * dr;

		double absAmp = fabs(amp);
		if ( absAmp < 1e-10 ) absAmp = 1e-10;

		sumAbs += absAmp;
		sumSigned += amp > 0 ? absAmp : -absAmp;
	}

	if ( sumAbs < 1e-10 ) return 0;

	return 1.0 - fabs(sumSigned) / sumAbs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<int> FirstMotionInversion::findMisfittingStations(
	const Math::NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	std::vector<int> misfitting;
	for ( const auto &obs : observations ) {
		int predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		if ( predicted != 0 && predicted != obs.polarity )
			misfitting.push_back(obs.index);
	}
	return misfitting;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double computeAzimuthalGap(
	const std::vector<PolarityObservation> &observations
) {
	if ( observations.size() < 2 ) return 360.0;

	std::vector<double> azimuths;
	azimuths.reserve(observations.size());
	for ( const auto &obs : observations ) {
		double az = fmod(obs.azimuth, 360.0);
		if ( az < 0 ) az += 360.0;
		azimuths.push_back(az);
	}

	std::sort(azimuths.begin(), azimuths.end());

	double maxGap = 0;
	for ( size_t i = 1; i < azimuths.size(); ++i ) {
		double gap = azimuths[i] - azimuths[i - 1];
		if ( gap > maxGap ) maxGap = gap;
	}

	// Check wraparound gap
	double wrapGap = 360.0 - azimuths.back() + azimuths.front();
	if ( wrapGap > maxGap ) maxGap = wrapGap;

	return maxGap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<FMSolution> FirstMotionInversion::compute(
	const std::vector<PolarityObservation> &observations
) const {
	if ( static_cast<int>(observations.size()) < MIN_OBSERVATIONS ) {
		SEISCOMP_DEBUG("FM inversion: only %zu observations, need %d minimum",
		               observations.size(), MIN_OBSERVATIONS);
		return {};
	}

	SEISCOMP_DEBUG("FM inversion: starting grid search with %zu observations, "
	               "grid spacing=%.1f, trials=%d, uncertainty=%.1f",
	               observations.size(), _gridSpacing, _numTrials, _angleUncertainty);

	for ( size_t i = 0; i < observations.size(); ++i ) {
		SEISCOMP_DEBUG("  obs[%zu]: azi=%.1f takeoff=%.1f pol=%s idx=%d",
		               i, observations[i].azimuth, observations[i].takeoff,
		               observations[i].polarity > 0 ? "UP" : "DOWN",
		               observations[i].index);
	}

	// Phase 1: Grid search to find candidate solutions
	struct Candidate {
		Math::NODAL_PLANE np;
		double misfit;
	};

	std::vector<Candidate> candidates;
	double bestMisfit = 1.0;

	for ( double strike = 0; strike < 360; strike += _gridSpacing ) {
		for ( double dip = _gridSpacing; dip <= 90; dip += _gridSpacing ) {
			for ( double rake = -180; rake < 180; rake += _gridSpacing ) {
				Math::NODAL_PLANE np;
				np.str = strike;
				np.dip = dip;
				np.rake = rake;

				double misfit = computeMisfit(np, observations);

				if ( misfit <= bestMisfit + 0.01 ) {
					if ( misfit < bestMisfit - 0.01 ) {
						candidates.erase(
							std::remove_if(candidates.begin(), candidates.end(),
								[&](const Candidate &c) {
									return c.misfit > misfit + 0.01;
								}),
							candidates.end()
						);
						bestMisfit = misfit;
					}
					candidates.push_back({np, misfit});
				}
			}
		}
	}

	if ( candidates.empty() ) {
		SEISCOMP_DEBUG("FM inversion: no candidates found in grid search");
		return {};
	}

	SEISCOMP_DEBUG("FM inversion: grid search found %zu candidates, best misfit=%.3f",
	               candidates.size(), bestMisfit);

	// Phase 2: HASH-style perturbation for stability assessment
	std::mt19937 rng(42);  // fixed seed for reproducibility
	std::normal_distribution<double> perturbation(0.0, _angleUncertainty / 2.0);

	std::vector<FMSolution> solutions;
	double azGap = computeAzimuthalGap(observations);

	for ( auto &cand : candidates ) {
		int stableCount = 0;

		for ( int trial = 0; trial < _numTrials; ++trial ) {
			std::vector<PolarityObservation> perturbed = observations;
			for ( auto &obs : perturbed ) {
				obs.azimuth += perturbation(rng);
				obs.takeoff += perturbation(rng);
				if ( obs.takeoff < 0 ) obs.takeoff = -obs.takeoff;
				if ( obs.takeoff > 180 ) obs.takeoff = 360 - obs.takeoff;
			}

			double pertMisfit = computeMisfit(cand.np, perturbed);
			if ( pertMisfit <= cand.misfit + 0.05 )
				++stableCount;
		}

		double stability = static_cast<double>(stableCount) / _numTrials;
		double stdr = computeSTDR(cand.np, observations);

		// Compute both nodal planes via normal/slip decomposition
		Math::Vector3d n, d;
		Math::NODAL_PLANE npRad;
		npRad.str = deg2rad(cand.np.str);
		npRad.dip = deg2rad(cand.np.dip);
		npRad.rake = deg2rad(cand.np.rake);
		Math::np2nd(npRad, n, d);

		FMSolution sol;
		Math::nd2dc(n, d, &sol.np1, &sol.np2);
		sol.misfit = cand.misfit;
		sol.stdr = stdr;
		sol.azimuthalGap = azGap;
		sol.stationCount = static_cast<int>(observations.size());

		// Find misfitting stations
		sol.misfittingStations = findMisfittingStations(sol.np1, observations);
		sol.misfitCount = static_cast<int>(sol.misfittingStations.size());

		// Quality grade (HASH convention)
		sol.grade = computeQualityGrade(sol.misfit, stdr, azGap, sol.stationCount);

		// Quality score: weighted combination with azimuthal gap penalty
		double gapPenalty = 1.0;
		if ( azGap > 180.0 )
			gapPenalty = 0.2;
		else if ( azGap > 120.0 )
			gapPenalty = 0.5;
		else if ( azGap > 90.0 )
			gapPenalty = 0.8;

		sol.quality = ((1.0 - sol.misfit) * 0.5 + stdr * 0.2 + stability * 0.3) * gapPenalty;

		SEISCOMP_DEBUG("FM inversion: candidate NP1=%.1f/%.1f/%.1f NP2=%.1f/%.1f/%.1f "
		               "misfit=%.3f(%d/%d) stdr=%.3f stability=%.3f grade=%s quality=%.3f",
		               sol.np1.str, sol.np1.dip, sol.np1.rake,
		               sol.np2.str, sol.np2.dip, sol.np2.rake,
		               sol.misfit, sol.misfitCount, sol.stationCount,
		               sol.stdr, stability, qualityGradeString(sol.grade),
		               sol.quality);

		solutions.push_back(sol);
	}

	// Sort by quality descending
	std::sort(solutions.begin(), solutions.end(),
		[](const FMSolution &a, const FMSolution &b) {
			return a.quality > b.quality;
		}
	);

	// Remove duplicates (solutions with very similar nodal planes)
	std::vector<FMSolution> unique;
	for ( const auto &sol : solutions ) {
		bool isDuplicate = false;
		for ( const auto &u : unique ) {
			double dStrike = fabs(sol.np1.str - u.np1.str);
			if ( dStrike > 180 ) dStrike = 360 - dStrike;
			double dDip = fabs(sol.np1.dip - u.np1.dip);
			double dRake = fabs(sol.np1.rake - u.np1.rake);
			if ( dRake > 180 ) dRake = 360 - dRake;

			if ( dStrike < 2 * _gridSpacing &&
			     dDip < 2 * _gridSpacing &&
			     dRake < 2 * _gridSpacing ) {
				isDuplicate = true;
				break;
			}
		}
		if ( !isDuplicate )
			unique.push_back(sol);
	}

	if ( unique.size() > 10 )
		unique.resize(10);

	SEISCOMP_DEBUG("FM inversion: returning %zu unique solutions (from %zu after dedup)",
	               unique.size(), solutions.size());
	for ( size_t i = 0; i < unique.size(); ++i ) {
		SEISCOMP_DEBUG("  solution[%zu]: NP1=%.1f/%.1f/%.1f NP2=%.1f/%.1f/%.1f "
		               "misfit=%.3f(%d) stdr=%.3f gap=%.1f grade=%s quality=%.3f",
		               i, unique[i].np1.str, unique[i].np1.dip, unique[i].np1.rake,
		               unique[i].np2.str, unique[i].np2.dip, unique[i].np2.rake,
		               unique[i].misfit, unique[i].misfitCount,
		               unique[i].stdr, unique[i].azimuthalGap,
		               qualityGradeString(unique[i].grade), unique[i].quality);
	}

	return unique;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
