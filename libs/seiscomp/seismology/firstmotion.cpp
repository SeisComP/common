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
namespace {


// Zoeppritz free-surface reflection coefficients for an incident P-wave.
// Aki & Richards (2002), Table 5.1.
struct FreeSurfaceCoeff {
	double PP;  // P-to-P reflection coefficient
	double PS;  // P-to-SV conversion coefficient
};


FreeSurfaceCoeff zoeppritzFreeSurface(
	double sinI, double cosI,
	double sinJ, double cosJ,
	double vpvs
) {
	double gamma = 1.0 / vpvs;  // Vs/Vp
	double gamma2 = gamma * gamma;
	double sin2I = sinI * sinI;

	// a = 1 - 2*gamma^2*sin^2(i)
	double a = 1.0 - 2.0 * gamma2 * sin2I;
	double a2 = a * a;

	// D = a^2 + 4*gamma^2*sin^2(i)*cos(i)*cos(j)
	double D = a2 + 4.0 * gamma2 * sin2I * cosI * cosJ;

	FreeSurfaceCoeff c;

	if ( fabs(D) < 1e-15 ) {
		c.PP = 0;
		c.PS = 0;
		return c;
	}

	// PP = -(a^2 - 4*gamma^2*sin^2(i)*cos(i)*cos(j)) / D
	c.PP = -(a2 - 4.0 * gamma2 * sin2I * cosI * cosJ) / D;

	// PS = 4*gamma*sin(i)*cos(i)*a / D
	c.PS = 4.0 * gamma * sinI * cosI * a / D;

	return c;
}


}  // anonymous namespace
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double computeRadiationAmplitude(
	const NODAL_PLANE &np,
	double azimuth, double takeoff,
	double rayParam, double surfaceVp, double surfaceVpVs
) {
	Vector3d n;
	Vector3d d;
	np2nd(np, n, d);

	double ih = deg2rad(takeoff);
	double phi = deg2rad(azimuth);

	double rx = sin(ih) * cos(phi);
	double ry = sin(ih) * sin(phi);
	double rz = -cos(ih);

	double nr = n.x * rx + n.y * ry + n.z * rz;
	double dr = d.x * rx + d.y * ry + d.z * rz;
	double baseAmplitude = nr * dr;

	// If no valid ray parameter, return uncorrected amplitude
	if ( rayParam < 0 ) {
		return baseAmplitude;
	}

	// Convert ray parameter from sec/deg to sec/km
	// dtdd is in sec/deg; p_km = dtdd / (R_earth * pi/180)
	static constexpr double R_EARTH_KM = 6371.0;
	static constexpr double DEG2KM = R_EARTH_KM * M_PI / 180.0;
	double p_skm = fabs(rayParam) / DEG2KM;

	// Compute P-wave incidence angle at the surface: sin(i) = p * Vp
	double sinI = p_skm * surfaceVp;
	if ( sinI >= 1.0 ) {
		// At or beyond critical angle for P: return base amplitude
		return baseAmplitude;
	}
	double cosI = sqrt(1.0 - sinI * sinI);

	// Snell's law: sin(j)/Vs = sin(i)/Vp => sin(j) = sin(i) / vpvs
	double sinJ = sinI / surfaceVpVs;
	if ( sinJ >= 1.0 ) {
		// Post-critical for S: no SV conversion, return base amplitude
		return baseAmplitude;
	}
	double cosJ = sqrt(1.0 - sinJ * sinJ);

	// Compute Zoeppritz free-surface coefficients
	auto coeff = zoeppritzFreeSurface(sinI, cosI, sinJ, cosJ, surfaceVpVs);

	// Nakamura (2002) Eq. 3: the free-surface correction factor for
	// P-wave vertical displacement at the surface.
	//
	// Rp = 2(cos i - PP*cos i + PS*sin j)(gamma . A)(gamma . B)
	//
	// The correction factor (relative to the base radiation pattern) is:
	//   cos(i)*(1 - PP) + PS*sin(j)
	//
	// Verification at vertical incidence (i=0, j=0, PP=-1, PS=0):
	//   1*(1-(-1)) + 0 = 2  (free surface doubles displacement) ✓
	// At grazing incidence (i→90°, cosI→0, PS→0):
	//   0 + 0 = 0  (no vertical displacement for horizontal ray) ✓
	//
	// The factor is always positive for realistic incidence angles,
	// meaning the free surface does NOT flip the polarity — it only
	// modifies the amplitude. The polarity is determined by the
	// radiation pattern at the source.
	double correctionFactor = cosI * (1.0 - coeff.PP) + coeff.PS * sinJ;

	// The correction factor should always be positive for normal incidence
	// angles. If it becomes zero or negative at extreme angles, that indicates
	// a polarity reversal due to the free surface effect.
	return baseAmplitude * correctionFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int predictPolarity(
	const NODAL_PLANE &np,
	double azimuth, double takeoff,
	double rayParam, double surfaceVp, double surfaceVpVs
) {
	double amplitude = computeRadiationAmplitude(
		np, azimuth, takeoff, rayParam, surfaceVp, surfaceVpVs
	);

	if ( fabs(amplitude) < 1e-10 ) {
		return 0;
	}

	return amplitude > 0 ? 1 : -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double computeObservationWeight(
	double locatorWeight, double timeResidual, double residualDecay
) {
	if ( residualDecay <= 0 ) {
		return locatorWeight;
	}

	double w = locatorWeight * exp(-fabs(timeResidual) / residualDecay);

	if ( w < 0 ) {
		return 0;
	}
	if ( w > 1.0 ) {
		return 1.0;
	}

	return w;
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

	if ( surfaceVp <= 0 ) {
		SEISCOMP_WARNING("FM config: surfaceVp %.2f <= 0, clamping to 5.8",
		                 surfaceVp);
		surfaceVp = 5.8;
		valid = false;
	}

	if ( surfaceVpVs <= 1.0 ) {
		SEISCOMP_WARNING("FM config: surfaceVpVs %.2f <= 1.0, clamping to 1.73",
		                 surfaceVpVs);
		surfaceVpVs = 1.73;
		valid = false;
	}

	if ( residualDecay <= 0 ) {
		SEISCOMP_WARNING("FM config: residualDecay %.2f <= 0, clamping to 1.0",
		                 residualDecay);
		residualDecay = 1.0;
		valid = false;
	}

	if ( reliabilityEpsilon < 0 ) {
		SEISCOMP_WARNING("FM config: reliabilityEpsilon %.2f < 0, clamping to 0",
		                 reliabilityEpsilon);
		reliabilityEpsilon = 0;
		valid = false;
	}

	return valid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


// Weighted misfit accumulation result
struct WeightedMisfitResult {
	double           weightedMisfit{0};  // sum of Wi for misfitting obs
	double           totalWeight{0};     // sum of all Wi
	int              count{0};           // integer count of misfitting obs
	std::vector<int> indices;            // indices of misfitting obs
};


WeightedMisfitResult computeWeightedMisfits(
	const NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations,
	const FMInversionConfig &config
) {
	WeightedMisfitResult result;

	for ( const auto &obs : observations ) {
		result.totalWeight += obs.weight;

		int predicted;
		if ( config.freeSurfaceCorrection && obs.rayParam >= 0 ) {
			predicted = predictPolarity(np, obs.azimuth, obs.takeoff,
			                            obs.rayParam, config.surfaceVp,
			                            config.surfaceVpVs);
		}
		else {
			predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		}

		if ( predicted != 0 && predicted != obs.polarity ) {
			result.weightedMisfit += obs.weight;
			++result.count;
			result.indices.push_back(obs.index);
		}
	}

	return result;
}


// Fast weighted misfit for the grid search inner loop (no index tracking)
double countWeightedMisfits(
	const NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations,
	const FMInversionConfig &config
) {
	double misfits = 0;

	for ( const auto &obs : observations ) {
		int predicted;
		if ( config.freeSurfaceCorrection && obs.rayParam >= 0 ) {
			predicted = predictPolarity(np, obs.azimuth, obs.takeoff,
			                            obs.rayParam, config.surfaceVp,
			                            config.surfaceVpVs);
		}
		else {
			predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		}

		if ( predicted != 0 && predicted != obs.polarity ) {
			misfits += obs.weight;
		}
	}

	return misfits;
}


// Build a complete FMSolution from a candidate nodal plane
FMSolution buildSolution(
	const NODAL_PLANE &candidateNP,
	const std::vector<PolarityObservation> &observations,
	double azGap,
	const FMInversionConfig &config
) {
	Vector3d n;
	Vector3d d;
	np2nd(candidateNP, n, d);

	NODAL_PLANE np1;
	NODAL_PLANE np2;
	nd2dc(n, d, &np1, &np2);

	auto misfits = computeWeightedMisfits(candidateNP, observations, config);

	FMSolution sol;
	sol.np1 = np1;
	sol.np2 = np2;
	sol.misfit = (misfits.totalWeight > 0)
		? misfits.weightedMisfit / misfits.totalWeight
		: 0.0;
	sol.azimuthalGap = azGap;
	sol.stationCount = static_cast<int>(observations.size());
	sol.misfitCount = misfits.count;
	sol.misfittingStations = std::move(misfits.indices);
	sol.quality = assessQuality(sol);

	return sol;
}


// Angular distance between two unit vectors on the focal sphere
double angularDistance(const Vector3d &a, const Vector3d &b) {
	double dot = a.x * b.x + a.y * b.y + a.z * b.z;
	if ( dot > 1.0 ) dot = 1.0;
	if ( dot < -1.0 ) dot = -1.0;
	return acos(dot);
}


// Compute P and T axis reliability (Nakamura 2002 Fig. 4)
void computeReliability(
	const FMSolution &best,
	const std::vector<FMSolution> &accepted,
	double epsilon,
	double &pReliability,
	double &tReliability
) {
	// Get P and T axes of the best solution
	Vector3d bestN, bestD;
	np2nd(best.np1, bestN, bestD);

	Vector3d bestT, bestP;
	nd2pa(bestN, bestD, bestT, bestP);

	double maxPAngle = 0;
	double maxTAngle = 0;

	double misfitThreshold = best.misfit + epsilon;

	for ( const auto &sol : accepted ) {
		if ( sol.misfit > misfitThreshold ) {
			continue;
		}

		Vector3d solN, solD;
		np2nd(sol.np1, solN, solD);

		Vector3d solT, solP;
		nd2pa(solN, solD, solT, solP);

		// Axes have 180-degree ambiguity on the focal sphere;
		// take minimum of angle to +axis and -axis
		Vector3d negSolP{-solP.x, -solP.y, -solP.z};
		Vector3d negSolT{-solT.x, -solT.y, -solT.z};

		double pAngle = std::min(
			angularDistance(bestP, solP),
			angularDistance(bestP, negSolP)
		);
		double tAngle = std::min(
			angularDistance(bestT, solT),
			angularDistance(bestT, negSolT)
		);

		if ( pAngle > maxPAngle ) maxPAngle = pAngle;
		if ( tAngle > maxTAngle ) maxTAngle = tAngle;
	}

	// Express in units of pi (Nakamura convention)
	pReliability = maxPAngle / M_PI;
	tReliability = maxTAngle / M_PI;
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

	// Compute total weight for threshold calculation.
	// If all weights are zero, fall back to uniform weights.
	std::vector<PolarityObservation> fallbackObs;
	const std::vector<PolarityObservation> *obsPtr = &observations;

	double totalWeight = 0;
	for ( const auto &obs : observations ) {
		totalWeight += obs.weight;
	}

	if ( totalWeight < 1e-10 ) {
		SEISCOMP_WARNING("FM inversion: total observation weight near zero, "
		                 "falling back to uniform weights");
		fallbackObs = observations;
		for ( auto &obs : fallbackObs ) {
			obs.weight = 1.0;
		}
		obsPtr = &fallbackObs;
		totalWeight = static_cast<double>(nobs);
	}

	const auto &obs = *obsPtr;

	SEISCOMP_DEBUG("FM inversion: %zu observations, totalWeight=%.2f, grid=%.1f deg, "
	               "maxMisfitFrac=%.2f, azGap=%.1f, freeSurface=%s",
	               nobs, totalWeight, config.gridSpacing,
	               config.maxMisfitFraction, azGap,
	               config.freeSurfaceCorrection ? "on" : "off");

	double maxWeightedMisfits = totalWeight * config.maxMisfitFraction;

	// Grid search over strike/dip/rake
	struct Candidate {
		NODAL_PLANE np;
		double      misfits;
	};

	double bestMisfits = totalWeight + 1;
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

				double misfits = countWeightedMisfits(np, obs, config);

				if ( misfits <= maxWeightedMisfits ) {
					acceptedCandidates.push_back({np, misfits});
				}

				if ( misfits < bestMisfits ) {
					bestMisfits = misfits;
					bestCandidate = {np, misfits};
				}
			}
		}
	}

	if ( bestMisfits > maxWeightedMisfits ) {
		SEISCOMP_WARNING("FM inversion: best weighted misfit %.2f/%.2f exceeds threshold %.2f",
		                 bestMisfits, totalWeight, maxWeightedMisfits);
		return result;
	}

	// Build the best solution
	result.best = buildSolution(bestCandidate.np, obs, azGap, config);
	result.valid = true;

	// Sort accepted candidates by misfit, then build solutions
	std::sort(acceptedCandidates.begin(), acceptedCandidates.end(),
	          [](const Candidate &a, const Candidate &b) {
		          return a.misfits < b.misfits;
	          });

	result.accepted.reserve(acceptedCandidates.size());
	for ( const auto &cand : acceptedCandidates ) {
		result.accepted.push_back(
			buildSolution(cand.np, obs, azGap, config)
		);
	}

	// Compute continuous reliability metric if requested
	if ( config.computeReliability ) {
		// Nakamura's epsilon is in misfit count units (Q), but
		// sol.misfit is a fraction (weighted misfit / totalWeight).
		// Normalize epsilon to fraction units for comparison.
		computeReliability(
			result.best, result.accepted,
			config.reliabilityEpsilon / totalWeight,
			result.pAxisReliability, result.tAxisReliability
		);
		result.best.pAxisReliability = result.pAxisReliability;
		result.best.tAxisReliability = result.tAxisReliability;
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

	if ( result.pAxisReliability >= 0 ) {
		SEISCOMP_INFO("FM reliability: P-axis=%.3f*pi T-axis=%.3f*pi",
		              result.pAxisReliability, result.tAxisReliability);
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
