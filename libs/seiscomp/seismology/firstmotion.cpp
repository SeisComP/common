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


namespace Seiscomp::Seismology {
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
	double prob, double rmsDiff, double misfit, double stdr
) {
	// HASH quality grades (Hardebeck & Shearer 2002, as implemented in
	// hashpy). Four criteria: probability (fraction of acceptable
	// mechanisms within 30 degrees), RMS angular difference, weighted
	// polarity misfit, and station distribution ratio.
	if ( prob > 0.8 && rmsDiff < 25 && misfit <= 0.15 && stdr >= 0.5 ) {
		return FM_QUALITY_A;
	}
	if ( prob > 0.6 && rmsDiff <= 35 && misfit <= 0.20 && stdr >= 0.4 ) {
		return FM_QUALITY_B;
	}
	if ( prob > 0.5 && rmsDiff <= 45 && misfit <= 0.30 && stdr >= 0.3 ) {
		return FM_QUALITY_C;
	}
	return FM_QUALITY_D;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int predictPolarity(
	const Math::NODAL_PLANE &np,
	double azimuth, double takeoff
) {
	Math::Vector3d n;
	Math::Vector3d d;
	Math::np2nd(np, n, d);

	// Ray direction in NED Cartesian (x=North, y=East, z=Down).
	// Takeoff angle measured from downward vertical (SeisComP/HASH
	// convention: 0=downgoing, 180=upgoing).
	// HASH TO_CAR uses z-UP with z=-cos(ih); converted to NED z-DOWN
	// by negating: rz=cos(ih).
	double ih = deg2rad(takeoff);
	double phi = deg2rad(azimuth);

	double rx = sin(ih) * cos(phi);
	double ry = sin(ih) * sin(phi);
	double rz = cos(ih);

	// P-wave radiation pattern: amplitude proportional to (n . r)(d . r)
	double nr = n.x * rx + n.y * ry + n.z * rz;
	double dr = d.x * rx + d.y * ry + d.z * rz;
	double amplitude = nr * dr;

	if ( amplitude > 0 ) {
		return 1;   // compressional (up)
	}
	else if ( amplitude < 0 ) {
		return -1;  // dilatational (down)
	}
	else {
		return 0;   // nodal
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FirstMotionInversion::FirstMotionInversion()
: _gridSpacing(5.0)
, _numTrials(50)
, _takeoffUncertainty(10.0)
, _azimuthUncertainty(5.0)
, _badFrac(0.1) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setGridSpacing(double degrees) {
	if ( degrees > 0 && degrees <= 30 ) {
		_gridSpacing = degrees;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setNumTrials(int n) {
	if ( n > 0 && n <= 10000 ) {
		_numTrials = n;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setTakeoffUncertainty(double degrees) {
	if ( degrees >= 0 && degrees <= 45 ) {
		_takeoffUncertainty = degrees;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setAzimuthUncertainty(double degrees) {
	if ( degrees >= 0 && degrees <= 45 ) {
		_azimuthUncertainty = degrees;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FirstMotionInversion::setBadFraction(double frac) {
	if ( frac > 0 && frac <= 0.5 ) {
		_badFrac = frac;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int countMisfits(
	const Math::NODAL_PLANE &np,
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double computeWeightedMisfit(
	const Math::NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	// Weighted misfit following HASH GET_MISF (pol_subs.f).
	// Weight = sqrt(|radiation pattern amplitude|) for each station.
	// This down-weights near-nodal observations where polarity is
	// ambiguous. The weighting is from FPFIT (Reasenberg & Oppenheimer
	// 1985) and is used for quality reporting only (it does not affect
	// the grid search acceptance criterion).
	Math::Vector3d n;
	Math::Vector3d d;
	Math::np2nd(np, n, d);

	double weightedMisfit = 0;
	double weightedTotal = 0;

	for ( const auto &obs : observations ) {
		double ih = deg2rad(obs.takeoff);
		double phi = deg2rad(obs.azimuth);

		double rx = sin(ih) * cos(phi);
		double ry = sin(ih) * sin(phi);
		double rz = cos(ih);

		double nr = n.x * rx + n.y * ry + n.z * rz;
		double dr = d.x * rx + d.y * ry + d.z * rz;
		double amp = fabs(nr * dr);
		if ( amp < 1e-10 ) {
			amp = 1e-10;
		}

		double wt = sqrt(amp);

		int predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		if ( predicted != 0 && predicted != obs.polarity ) {
			weightedMisfit += wt;
		}

		weightedTotal += wt;
	}

	if ( weightedTotal < 1e-10 ) {
		return 1.0;
	}
	return weightedMisfit / weightedTotal;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double computeSTDR(
	const Math::NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	// Station Distribution Ratio following HASH GET_MISF (pol_subs.f).
	// STDR = (sum of radiation pattern weights) / (number of observations)
	// This measures how well stations sample both compressional and
	// dilatational quadrants relative to the nodal planes. A value
	// near 1 means observations are far from nodal planes (well
	// constrained); near 0 means observations cluster near the nodal
	// planes. Introduced by Reasenberg & Oppenheimer (1985).
	Math::Vector3d n;
	Math::Vector3d d;
	Math::np2nd(np, n, d);

	double sumWeighted = 0;
	int count = 0;

	for ( const auto &obs : observations ) {
		double ih = deg2rad(obs.takeoff);
		double phi = deg2rad(obs.azimuth);

		double rx = sin(ih) * cos(phi);
		double ry = sin(ih) * sin(phi);
		double rz = cos(ih);

		double nr = n.x * rx + n.y * ry + n.z * rz;
		double dr = d.x * rx + d.y * ry + d.z * rz;
		double amp = fabs(nr * dr);
		if ( amp < 1e-10 ) {
			amp = 1e-10;
		}

		sumWeighted += sqrt(amp);
		++count;
	}

	if ( count == 0 ) {
		return 0;
	}
	return sumWeighted / count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<int> findMisfittingStations(
	const Math::NODAL_PLANE &np,
	const std::vector<PolarityObservation> &observations
) {
	std::vector<int> misfitting;
	for ( const auto &obs : observations ) {
		int predicted = predictPolarity(np, obs.azimuth, obs.takeoff);
		if ( predicted != 0 && predicted != obs.polarity ) {
			misfitting.push_back(obs.index);
		}
	}
	return misfitting;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}  // anonymous namespace




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double mechanismRotation(
	const Math::Vector3d &n1, const Math::Vector3d &d1,
	const Math::Vector3d &n2, const Math::Vector3d &d2
) {
	// Minimum rotation angle between two double-couple mechanisms.
	// Full implementation of HASH MECH_ROT (uncert_subs.f, Hardebeck &
	// Shearer 2002). Tests four combinations that account for the
	// ambiguity between fault plane and auxiliary plane:
	//   (1) n2, d2          (as-is)
	//   (2) -n2, -d2        (negate)
	//   (3) d2, n2          (swap)
	//   (4) -d2, -n2        (swap and negate)
	//
	// For each combination, compute the B-axes (null axes) as cross
	// products, then find the single rotation axis and angle that maps
	// one mechanism to the other.

	const double degrad = 180.0 / M_PI;

	// B-axis of mechanism 1
	Math::Vector3d B1;
	B1.x = n1.y * d1.z - n1.z * d1.y;
	B1.y = n1.z * d1.x - n1.x * d1.z;
	B1.z = n1.x * d1.y - n1.y * d1.x;

	struct Combo {
		Math::Vector3d na;
		Math::Vector3d da;
	};

	Combo combos[4] = {
		{ n2, d2 },
		{ {-n2.x, -n2.y, -n2.z}, {-d2.x, -d2.y, -d2.z} },
		{ d2, n2 },
		{ {-d2.x, -d2.y, -d2.z}, {-n2.x, -n2.y, -n2.z} }
	};

	double rotemp[4];

	for ( int c = 0; c < 4; ++c ) {
		const auto &nc = combos[c].na;
		const auto &dc = combos[c].da;

		// B-axis of this combination
		Math::Vector3d B2;
		B2.x = nc.y * dc.z - nc.z * dc.y;
		B2.y = nc.z * dc.x - nc.x * dc.z;
		B2.z = nc.x * dc.y - nc.y * dc.x;

		// Angles between corresponding axes
		double dot_n = n1.x * nc.x + n1.y * nc.y + n1.z * nc.z;
		double dot_d = d1.x * dc.x + d1.y * dc.y + d1.z * dc.z;
		double dot_b = B1.x * B2.x + B1.y * B2.y + B1.z * B2.z;

		// Clamp for numerical safety
		dot_n = std::max(-1.0, std::min(1.0, dot_n));
		dot_d = std::max(-1.0, std::min(1.0, dot_d));
		dot_b = std::max(-1.0, std::min(1.0, dot_b));

		double phi[3] = { acos(dot_n), acos(dot_d), acos(dot_b) };

		// If mechanisms are nearly identical
		if ( phi[0] < 1e-4 && phi[1] < 1e-4 && phi[2] < 1e-4 ) {
			rotemp[c] = 0.0;
		}
		// If one vector is the same, it is the rotation axis
		else if ( phi[0] < 1e-4 ) {
			rotemp[c] = degrad * phi[1];
		}
		else if ( phi[1] < 1e-4 ) {
			rotemp[c] = degrad * phi[2];
		}
		else if ( phi[2] < 1e-4 ) {
			rotemp[c] = degrad * phi[0];
		}
		else {
			// General case: find rotation axis from difference vectors.
			// The rotation axis is orthogonal to all three difference
			// vectors. We use the cross product of the two largest.

			// Vectors: v1 = n1, v2 = d1, v3 = B1
			double v1[3] = { n1.x, n1.y, n1.z };
			double v2[3] = { d1.x, d1.y, d1.z };
			double v3[3] = { B1.x, B1.y, B1.z };
			double w1[3] = { nc.x, nc.y, nc.z };
			double w2[3] = { dc.x, dc.y, dc.z };
			double w3[3] = { B2.x, B2.y, B2.z };

			// Difference vectors
			double dn[3][3];
			double scale[3];
			for ( int j = 0; j < 3; ++j ) {
				double *src1, *src2;
				if ( j == 0 ) { src1 = v1; src2 = w1; }
				else if ( j == 1 ) { src1 = v2; src2 = w2; }
				else { src1 = v3; src2 = w3; }

				double s = 0;
				for ( int i = 0; i < 3; ++i ) {
					dn[i][j] = src1[i] - src2[i];
					s += dn[i][j] * dn[i][j];
				}
				scale[j] = sqrt(s);
				if ( scale[j] > 1e-10 ) {
					for ( int i = 0; i < 3; ++i ) {
						dn[i][j] /= scale[j];
					}
				}
			}

			// Dot products between normalized difference vectors
			double qdot[3];
			qdot[2] = dn[0][0] * dn[0][1] + dn[1][0] * dn[1][1] + dn[2][0] * dn[2][1];
			qdot[1] = dn[0][0] * dn[0][2] + dn[1][0] * dn[1][2] + dn[2][0] * dn[2][2];
			qdot[0] = dn[0][1] * dn[0][2] + dn[1][1] * dn[1][2] + dn[2][1] * dn[2][2];

			// Find which difference vector to exclude: either nearly
			// parallel to another (qdot > 0.9999) or the smallest
			int iout = -1;
			for ( int i = 0; i < 3; ++i ) {
				if ( qdot[i] > 0.9999 ) { iout = i; break; }
			}
			if ( iout < 0 ) {
				double qmins = 1e10;
				for ( int i = 0; i < 3; ++i ) {
					if ( scale[i] < qmins ) {
						qmins = scale[i];
						iout = i;
					}
				}
			}

			// Select two difference vectors (excluding iout)
			double dv1[3], dv2[3];
			int k = 0;
			for ( int j = 0; j < 3; ++j ) {
				if ( j != iout ) {
					double *dst = (k == 0) ? dv1 : dv2;
					for ( int i = 0; i < 3; ++i ) { dst[i] = dn[i][j]; }
					++k;
				}
			}

			// Rotation axis = cross product of two difference vectors
			double R[3];
			R[0] = dv1[1] * dv2[2] - dv1[2] * dv2[1];
			R[1] = dv1[2] * dv2[0] - dv1[0] * dv2[2];
			R[2] = dv1[0] * dv2[1] - dv1[1] * dv2[0];
			double scaleR = sqrt(R[0] * R[0] + R[1] * R[1] + R[2] * R[2]);
			if ( scaleR > 1e-10 ) {
				R[0] /= scaleR; R[1] /= scaleR; R[2] /= scaleR;
			}

			// Find angle of each axis from the rotation axis
			double theta[3];
			theta[0] = acos(std::max(-1.0, std::min(1.0,
				v1[0] * R[0] + v1[1] * R[1] + v1[2] * R[2])));
			theta[1] = acos(std::max(-1.0, std::min(1.0,
				v2[0] * R[0] + v2[1] * R[1] + v2[2] * R[2])));
			theta[2] = acos(std::max(-1.0, std::min(1.0,
				v3[0] * R[0] + v3[1] * R[1] + v3[2] * R[2])));

			// Use the axis most perpendicular to the rotation axis
			// (closest to pi/2)
			double qmindif = 1000.0;
			int iuse = 0;
			for ( int i = 0; i < 3; ++i ) {
				double diff = fabs(theta[i] - M_PI / 2.0);
				if ( diff < qmindif ) {
					qmindif = diff;
					iuse = i;
				}
			}

			// Compute rotation angle using the spherical geometry
			// formula from HASH
			double sinT = sin(theta[iuse]);
			double cosT = cos(theta[iuse]);
			double val = (cos(phi[iuse]) - cosT * cosT) / (sinT * sinT);
			val = std::max(-1.0, std::min(1.0, val));
			rotemp[c] = degrad * acos(val);
		}
	}

	// Return minimum rotation angle across all 4 combinations
	double minAngle = 180.0;
	for ( int c = 0; c < 4; ++c ) {
		if ( fabs(rotemp[c]) < minAngle ) {
			minAngle = fabs(rotemp[c]);
		}
	}

	return minAngle;
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

	// Check wraparound gap
	double wrapGap = 360.0 - azimuths.back() + azimuths.front();
	if ( wrapGap > maxGap ) {
		maxGap = wrapGap;
	}

	return maxGap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<FMSolution> FirstMotionInversion::compute(
	const std::vector<PolarityObservation> &observations
) const {
	if ( observations.size() < MIN_OBSERVATIONS ) {
		SEISCOMP_DEBUG("FM inversion: only %zu observations, need %zu minimum",
		               observations.size(), MIN_OBSERVATIONS);
		return {};
	}

	int npol = static_cast<int>(observations.size());
	SEISCOMP_DEBUG("FM inversion: starting with %d observations, "
	               "grid=%.1f, trials=%d, takeoff_unc=%.1f, azi_unc=%.1f, "
	               "badfrac=%.2f",
	               npol, _gridSpacing, _numTrials,
	               _takeoffUncertainty, _azimuthUncertainty, _badFrac);

	for ( size_t i = 0; i < observations.size(); ++i ) {
		SEISCOMP_DEBUG("  obs[%zu]: azi=%.1f takeoff=%.1f pol=%s idx=%d",
		               i, observations[i].azimuth, observations[i].takeoff,
		               observations[i].polarity > 0 ? "UP" : "DOWN",
		               observations[i].index);
	}

	// Maximum allowed misfits following HASH (fmech_subs.f).
	// In HASH, impulsive and emergent picks are tracked separately.
	// Since we do not distinguish pick quality, we use a simplified
	// threshold: bestMisfit + nextra, where nextra allows additional
	// misfits above the minimum found in the grid search.
	// nextra = max(npol * badfrac * 0.5, 2)  (from hashpy)
	int nextra = std::max(static_cast<int>(npol * _badFrac * 0.5), 2);

	SEISCOMP_DEBUG("FM inversion: nextra=%d (badfrac=%.2f)", nextra, _badFrac);

	// Collect acceptable mechanisms across all trials.
	// Each entry stores the fault normal and slip vector.
	struct AcceptedMech {
		Math::Vector3d normal;
		Math::Vector3d slip;
	};

	std::vector<AcceptedMech> accepted;

	// RNG for perturbation trials (SKHASH-style: perturb takeoff and
	// azimuth directly when pre-computed angles are the input)
	std::mt19937 rng(42);  // fixed seed for reproducibility
	std::normal_distribution<double> takeoffPert(0.0, _takeoffUncertainty);
	std::normal_distribution<double> azimuthPert(0.0, _azimuthUncertainty);

	// Trial 0 uses the original observations, subsequent trials perturb
	int totalTrials = 1 + _numTrials;

	for ( int trial = 0; trial < totalTrials; ++trial ) {
		// Prepare observations for this trial
		std::vector<PolarityObservation> trialObs = observations;
		if ( trial > 0 ) {
			for ( auto &obs : trialObs ) {
				obs.takeoff += takeoffPert(rng);
				if ( obs.takeoff < 0 ) {
					obs.takeoff = -obs.takeoff;
				}
				if ( obs.takeoff > 180 ) {
					obs.takeoff = 360 - obs.takeoff;
				}
				obs.azimuth += azimuthPert(rng);
			}
		}

		// Grid search: find minimum misfit for this trial
		int bestMisfitCount = npol;

		for ( double strike = 0; strike < 360; strike += _gridSpacing ) {
			for ( double dip = _gridSpacing; dip <= 90; dip += _gridSpacing ) {
				for ( double rake = -180; rake < 180; rake += _gridSpacing ) {
					Math::NODAL_PLANE np;
					np.str = strike;
					np.dip = dip;
					np.rake = rake;

					int mc = countMisfits(np, trialObs);
					if ( mc < bestMisfitCount ) {
						bestMisfitCount = mc;
					}
				}
			}
		}

		// Acceptable misfit threshold for this trial:
		// Allow up to bestMisfit + nextra misfitting stations
		int threshold = bestMisfitCount + nextra;

		// Second pass: collect all mechanisms within threshold
		for ( double strike = 0; strike < 360; strike += _gridSpacing ) {
			for ( double dip = _gridSpacing; dip <= 90; dip += _gridSpacing ) {
				for ( double rake = -180; rake < 180; rake += _gridSpacing ) {
					Math::NODAL_PLANE np;
					np.str = strike;
					np.dip = dip;
					np.rake = rake;

					int mc = countMisfits(np, trialObs);
					if ( mc <= threshold ) {
						Math::Vector3d n;
						Math::Vector3d d;
						Math::np2nd(np, n, d);

						accepted.emplace_back(AcceptedMech{n, d});
					}
				}
			}
		}

		if ( trial == 0 ) {
			SEISCOMP_DEBUG("FM inversion: trial 0 (unperturbed): best misfit=%d, "
			               "threshold=%d, total accepted so far=%zu",
			               bestMisfitCount, threshold, accepted.size());
		}
	}

	if ( accepted.empty() ) {
		SEISCOMP_DEBUG("FM inversion: no acceptable mechanisms found");
		return {};
	}

	SEISCOMP_DEBUG("FM inversion: %zu total acceptable mechanisms from %d trials",
	               accepted.size(), totalTrials);

	// Average the acceptable mechanisms with iterative outlier removal
	// following HASH MECH_PROB (uncert_subs.f).
	//
	// Algorithm:
	// 1. Compute average normal and slip vectors
	// 2. Compute rotation angle of each mechanism from the average
	// 3. Remove the mechanism with the largest rotation
	// 4. Repeat until all remaining mechanisms are within 30 degrees
	// 5. The fraction of original mechanisms within 30 degrees = prob

	// Cutoff angle for outlier removal and probability computation.
	// Both HASH (hashpy) and SKHASH use 45 degrees as default.
	const double cutoffAngle = 45.0;
	auto nTotal = accepted.size();

	// Iterative outlier removal following HASH MECH_PROB (uncert_subs.f).
	// Batch approach: compute average, discard mechanisms beyond cutoff,
	// recompute average from the kept set, repeat until stable.
	std::vector<AcceptedMech> working = accepted;
	Math::Vector3d avgN = {0, 0, 0};
	Math::Vector3d avgD = {0, 0, 0};

	const int maxIter = 20;  // convergence typically in 2-3 iterations
	for ( int iter = 0; iter < maxIter; ++iter ) {
		if ( working.empty() ) {
			break;
		}

		// Compute average normal and slip vectors.
		// Handle sign ambiguity: for each mechanism, choose the sign
		// of n and d that is consistent with a fixed reference,
		// following HASH MECH_AVG (uncert_subs.f).
		// HASH uses the first mechanism as the reference (not the
		// running sum, which would bias toward np2nd's hemisphere).
		Math::Vector3d refN = working[0].normal;
		Math::Vector3d refD = working[0].slip;

		avgN = {0, 0, 0};
		avgD = {0, 0, 0};

		for ( size_t i = 0; i < working.size(); ++i ) {
			Math::Vector3d n = working[i].normal;
			Math::Vector3d d = working[i].slip;

			if ( i > 0 ) {
				// Test 4 sign combinations and pick the closest
				double bestRot = mechanismRotation(refN, refD, n, d);
				Math::Vector3d bestN = n;
				Math::Vector3d bestD = d;

				Math::Vector3d tn = {-n.x, -n.y, -n.z};
				Math::Vector3d td = {-d.x, -d.y, -d.z};
				double rot = mechanismRotation(refN, refD, tn, td);
				if ( rot < bestRot ) {
					bestRot = rot; bestN = tn; bestD = td;
				}

				rot = mechanismRotation(refN, refD, d, n);
				if ( rot < bestRot ) {
					bestRot = rot; bestN = d; bestD = n;
				}

				tn = {-d.x, -d.y, -d.z};
				td = {-n.x, -n.y, -n.z};
				rot = mechanismRotation(refN, refD, tn, td);
				if ( rot < bestRot ) {
					bestN = tn; bestD = td;
				}

				n = bestN;
				d = bestD;
			}

			avgN.x += n.x; avgN.y += n.y; avgN.z += n.z;
			avgD.x += d.x; avgD.y += d.y; avgD.z += d.z;
		}

		// Normalize
		double lenN = sqrt(avgN.x * avgN.x + avgN.y * avgN.y + avgN.z * avgN.z);
		double lenD = sqrt(avgD.x * avgD.x + avgD.y * avgD.y + avgD.z * avgD.z);
		if ( lenN > 1e-10 ) { avgN.x /= lenN; avgN.y /= lenN; avgN.z /= lenN; }
		if ( lenD > 1e-10 ) { avgD.x /= lenD; avgD.y /= lenD; avgD.z /= lenD; }

		// Ensure orthogonality: d = d - (d.n)n, then renormalize
		double dn = avgD.x * avgN.x + avgD.y * avgN.y + avgD.z * avgN.z;
		avgD.x -= dn * avgN.x;
		avgD.y -= dn * avgN.y;
		avgD.z -= dn * avgN.z;
		lenD = sqrt(avgD.x * avgD.x + avgD.y * avgD.y + avgD.z * avgD.z);
		if ( lenD > 1e-10 ) { avgD.x /= lenD; avgD.y /= lenD; avgD.z /= lenD; }

		// Keep only mechanisms within cutoff angle of the average
		auto prevSize = working.size();
		std::vector<AcceptedMech> kept;
		kept.reserve(working.size());
		for ( const auto &mech : working ) {
			double rot = mechanismRotation(avgN, avgD, mech.normal, mech.slip);
			if ( rot <= cutoffAngle ) {
				kept.push_back(mech);
			}
		}

		working = std::move(kept);

		SEISCOMP_DEBUG("FM inversion: MECH_PROB iter %d: %zu -> %zu mechanisms",
		               iter, prevSize, working.size());

		// Converged if nothing was removed
		if ( working.size() == prevSize ) {
			break;
		}
	}

	if ( working.empty() ) {
		SEISCOMP_DEBUG("FM inversion: all mechanisms removed as outliers");
		return {};
	}

	// Compute RMS angular difference and probability over ALL accepted
	// mechanisms (not just the working set after outlier removal).
	double sumSqRot = 0;
	size_t countWithin = 0;

	for ( const auto &mech : accepted ) {
		double rot = mechanismRotation(avgN, avgD, mech.normal, mech.slip);
		sumSqRot += rot * rot;
		if ( rot <= cutoffAngle ) {
			++countWithin;
		}
	}

	double rmsDiff = sqrt(sumSqRot / nTotal);
	double prob = static_cast<double>(countWithin) / nTotal;

	SEISCOMP_DEBUG("FM inversion: preferred solution from %zu/%zu mechanisms, "
	               "rmsDiff=%.1f, prob=%.3f",
	               working.size(), nTotal, rmsDiff, prob);

	// Convert averaged (n, d) back to nodal planes.
	// nd2dc returns degrees (it calls np2deg internally).
	FMSolution sol;
	Math::nd2dc(avgN, avgD, &sol.np1, &sol.np2);

	// Normalize strike to [0, 360)
	if ( sol.np1.str < 0 ) {
		sol.np1.str += 360;
	}
	if ( sol.np2.str < 0 ) {
		sol.np2.str += 360;
	}

	// Compute quality metrics against original (unperturbed) observations
	sol.misfit = computeWeightedMisfit(sol.np1, observations);
	sol.stdr = computeSTDR(sol.np1, observations);
	sol.azimuthalGap = computeAzimuthalGap(observations);
	sol.stationCount = npol;
	sol.misfittingStations = findMisfittingStations(sol.np1, observations);
	sol.misfitCount = sol.misfittingStations.size();
	sol.rmsDiff = rmsDiff;
	sol.prob = prob;

	// Quality grade (HASH convention)
	sol.grade = computeQualityGrade(prob, rmsDiff, sol.misfit, sol.stdr);

	// Composite quality score for internal ranking
	sol.quality = (1.0 - sol.misfit) * 0.3 + sol.stdr * 0.2
	            + prob * 0.3 + std::max(0.0, 1.0 - rmsDiff / 45.0) * 0.2;

	SEISCOMP_INFO("FM inversion: NP1=%.1f/%.1f/%.1f NP2=%.1f/%.1f/%.1f "
	              "misfit=%.3f(%d/%d) stdr=%.3f rmsDiff=%.1f prob=%.3f "
	              "gap=%.1f grade=%s quality=%.3f",
	              sol.np1.str, sol.np1.dip, sol.np1.rake,
	              sol.np2.str, sol.np2.dip, sol.np2.rake,
	              sol.misfit, sol.misfitCount, sol.stationCount,
	              sol.stdr, sol.rmsDiff, sol.prob,
	              sol.azimuthalGap, qualityGradeString(sol.grade),
	              sol.quality);

	return {sol};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
