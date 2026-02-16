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


using namespace Seiscomp::Math;


namespace Seiscomp::Seismology {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int predictPolarity(
	const NODAL_PLANE &np,
	double azimuth, double takeoff
) {
	Vector3d n;
	Vector3d d;
	np2nd(np, n, d);

	// Ray direction in Aki & Richards convention (x=North, y=East, z=Up).
	// np2nd() returns n,d in this same coordinate system.
	//
	// Takeoff angle: 0 = downgoing (straight down), 90 = horizontal,
	// 180 = upgoing. This matches SeisComP's TravelTimeTableInterface
	// (libtau, LOCSAT, homogeneous).
	//
	// In z-UP coordinates a downgoing ray has rz < 0:
	//   rz = -cos(takeoff)  =>  takeoff=0 -> rz=-1 (down)
	//                           takeoff=90 -> rz=0  (horizontal)
	//                           takeoff=180 -> rz=+1 (up)
	double ih = deg2rad(takeoff);
	double phi = deg2rad(azimuth);

	double rx = sin(ih) * cos(phi);
	double ry = sin(ih) * sin(phi);
	double rz = -cos(ih);

	// P-wave radiation pattern: amplitude proportional to (n . r)(d . r)
	double nr = n.x * rx + n.y * ry + n.z * rz;
	double dr = d.x * rx + d.y * ry + d.z * rz;
	double amplitude = nr * dr;

	if ( fabs(amplitude) < 1e-10 ) {
		return 0;
	}

	return amplitude > 0 ? 1 : -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double computeAzimuthalGap(
	const std::vector<PolarityObservation> &observations
) {
	if ( observations.size() < 2 ) {
		return 360.0;
	}

	std::vector<double> azimuths;
	azimuths.reserve(observations.size());
	for ( const auto &obs : observations ) {
		double az = fmod(obs.azimuth, 360.0);
		if ( az < 0 ) {
			az += 360.0;
		}
		azimuths.push_back(az);
	}

	std::sort(azimuths.begin(), azimuths.end());

	double maxGap = 0;
	for ( size_t i = 1; i < azimuths.size(); ++i ) {
		double gap = azimuths[i] - azimuths[i - 1];
		if ( gap > maxGap ) {
			maxGap = gap;
		}
	}

	// Wrap-around gap
	double wrapGap = 360.0 - azimuths.back() + azimuths.front();
	if ( wrapGap > maxGap ) {
		maxGap = wrapGap;
	}

	return maxGap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FMQuality assessQuality(const FMSolution &sol) {
	if ( sol.misfit <= 0.10 && sol.azimuthalGap <= 90.0 && sol.stationCount >= 10 ) {
		return FMQuality::A;
	}
	if ( sol.misfit <= 0.20 && sol.azimuthalGap <= 150.0 && sol.stationCount >= 8 ) {
		return FMQuality::B;
	}
	if ( sol.misfit <= 0.30 && sol.azimuthalGap <= 200.0 && sol.stationCount >= 6 ) {
		return FMQuality::C;
	}
	return FMQuality::D;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *qualityLabel(FMQuality q) {
	switch ( q ) {
		case FMQuality::A: return "A (well constrained)";
		case FMQuality::B: return "B (acceptable)";
		case FMQuality::C: return "C (poorly constrained)";
		case FMQuality::D: return "D (unreliable)";
	}
	return "unknown";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FMInversionConfig::validate() {
	bool valid = true;

	if ( gridSpacing < 1.0 ) {
		SEISCOMP_WARNING("FM config: gridSpacing %.2f < 1.0, clamping to 1.0",
		                 gridSpacing);
		gridSpacing = 1.0;
		valid = false;
	}
	else if ( gridSpacing > 30.0 ) {
		SEISCOMP_WARNING("FM config: gridSpacing %.2f > 30.0, clamping to 30.0",
		                 gridSpacing);
		gridSpacing = 30.0;
		valid = false;
	}

	if ( maxMisfitFraction < 0.0 ) {
		SEISCOMP_WARNING("FM config: maxMisfitFraction %.2f < 0, clamping to 0",
		                 maxMisfitFraction);
		maxMisfitFraction = 0.0;
		valid = false;
	}
	else if ( maxMisfitFraction > 0.5 ) {
		SEISCOMP_WARNING("FM config: maxMisfitFraction %.2f > 0.5, clamping to 0.5",
		                 maxMisfitFraction);
		maxMisfitFraction = 0.5;
		valid = false;
	}

	return valid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


// Count misfitting polarities and collect their indices in a single pass.
struct MisfitResult {
	int              count{0};
	std::vector<int> indices;
};


MisfitResult computeMisfits(
	const NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	MisfitResult result;
	for ( const auto &obs : observations ) {
		int predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		if ( predicted != 0 && predicted != obs.polarity ) {
			++result.count;
			result.indices.push_back(obs.index);
		}
	}
	return result;
}


// Fast count-only version for the grid search inner loop.
int countMisfits(
	const NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	int misfits = 0;
	for ( const auto &obs : observations ) {
		int predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		if ( predicted != 0 && predicted != obs.polarity ) {
			++misfits;
		}
	}
	return misfits;
}


// Build a complete FMSolution from a candidate nodal plane.
FMSolution buildSolution(
	const NODAL_PLANE &candidateNP,
	const std::vector<PolarityObservation> &observations,
	double azGap
) {
	size_t nobs = observations.size();

	// nd2dc already returns degrees (calls np2deg internally)
	Vector3d n;
	Vector3d d;
	np2nd(candidateNP, n, d);

	NODAL_PLANE np1;
	NODAL_PLANE np2;
	nd2dc(n, d, &np1, &np2);

	MisfitResult misfits = computeMisfits(candidateNP, observations);

	FMSolution sol;
	sol.np1 = np1;
	sol.np2 = np2;
	sol.misfit = static_cast<double>(misfits.count) / nobs;
	sol.azimuthalGap = azGap;
	sol.stationCount = static_cast<int>(nobs);
	sol.misfitCount = misfits.count;
	sol.misfittingStations = std::move(misfits.indices);
	sol.quality = assessQuality(sol);

	return sol;
}


}  // anonymous namespace
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FMInversionResult invertPolarities(
	const std::vector<PolarityObservation> &observations,
	const FMInversionConfig &config
) {
	FMInversionResult result;
	size_t nobs = observations.size();

	if ( nobs < FMInversionConfig::MIN_OBSERVATIONS ) {
		SEISCOMP_WARNING("FM inversion: only %zu observations, need %zu minimum",
		                 nobs, FMInversionConfig::MIN_OBSERVATIONS);
		return result;
	}

	double azGap = computeAzimuthalGap(observations);

	SEISCOMP_DEBUG("FM inversion: %zu observations, grid=%.1f deg, "
	               "maxMisfitFrac=%.2f, azGap=%.1f",
	               nobs, config.gridSpacing, config.maxMisfitFraction, azGap);

	int maxMisfits = static_cast<int>(nobs * config.maxMisfitFraction);

	// Grid search over strike/dip/rake.
	// Collect all accepted candidates along with the best.
	struct Candidate {
		NODAL_PLANE np;
		int         misfits;
	};

	int bestMisfits = static_cast<int>(nobs) + 1;
	Candidate bestCandidate{{0, 0, 0}, bestMisfits};
	std::vector<Candidate> acceptedCandidates;

	double step = config.gridSpacing;

	for ( double strike = 0; strike < 360; strike += step ) {
		for ( double dip = step; dip <= 90; dip += step ) {
			for ( double rake = -180; rake < 180; rake += step ) {
				NODAL_PLANE np;
				np.str = strike;
				np.dip = dip;
				np.rake = rake;

				int misfits = countMisfits(np, observations);

				if ( misfits <= maxMisfits ) {
					acceptedCandidates.push_back({np, misfits});
				}

				if ( misfits < bestMisfits ) {
					bestMisfits = misfits;
					bestCandidate = {np, misfits};
				}
			}
		}
	}

	if ( bestMisfits > maxMisfits ) {
		SEISCOMP_WARNING("FM inversion: best misfit %d/%zu exceeds threshold %d",
		                 bestMisfits, nobs, maxMisfits);
		return result;
	}

	// Build the best solution
	result.best = buildSolution(bestCandidate.np, observations, azGap);
	result.valid = true;

	// Sort accepted candidates by misfit count, then build solutions
	std::sort(acceptedCandidates.begin(), acceptedCandidates.end(),
	          [](const Candidate &a, const Candidate &b) {
		          return a.misfits < b.misfits;
	          });

	result.accepted.reserve(acceptedCandidates.size());
	for ( const auto &cand : acceptedCandidates ) {
		result.accepted.push_back(
			buildSolution(cand.np, observations, azGap)
		);
	}

	SEISCOMP_INFO("FM inversion: NP1=%.1f/%.1f/%.1f NP2=%.1f/%.1f/%.1f "
	              "misfit=%d/%zu (%.2f) gap=%.1f quality=%s "
	              "accepted=%zu solutions",
	              result.best.np1.str, result.best.np1.dip,
	              result.best.np1.rake,
	              result.best.np2.str, result.best.np2.dip,
	              result.best.np2.rake,
	              result.best.misfitCount, nobs, result.best.misfit, azGap,
	              qualityLabel(result.best.quality),
	              result.accepted.size());

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
