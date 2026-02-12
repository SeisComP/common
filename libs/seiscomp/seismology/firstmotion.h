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


#ifndef SEISCOMP_SEISMOLOGY_FIRSTMOTION_H
#define SEISCOMP_SEISMOLOGY_FIRSTMOTION_H


#include <seiscomp/math/conversions.h>
#include <seiscomp/core/baseobject.h>
#include <vector>


namespace Seiscomp::Seismology {


struct PolarityObservation {
	double azimuth;   // degrees, station azimuth from source
	double takeoff;   // degrees, takeoff angle at source
	int    polarity;  // +1 = up/positive, -1 = down/negative
	int    index;     // original arrival index (-1 if not set)

	PolarityObservation()
	: azimuth(0), takeoff(0), polarity(0), index(-1) {}

	PolarityObservation(double azi, double toff, int pol, int idx = -1)
	: azimuth(azi), takeoff(toff), polarity(pol), index(idx) {}
};


/**
 * @brief HASH quality grade.
 *
 * Quality grades following Hardebeck & Shearer (2002) and the HASH
 * reference implementation (hashpy). Grades are determined from four
 * criteria: weighted polarity misfit (mfrac), station distribution
 * ratio (STDR), RMS angular difference of acceptable mechanisms from
 * the preferred solution (var_avg), and fraction of acceptable
 * mechanisms within 30 degrees of the preferred solution (prob).
 *
 * A: prob > 0.8, var_avg < 25, mfrac <= 0.15, STDR >= 0.5
 * B: prob > 0.6, var_avg <= 35, mfrac <= 0.20, STDR >= 0.4
 * C: prob > 0.5, var_avg <= 45, mfrac <= 0.30, STDR >= 0.3
 * D: Anything worse than C.
 */
enum FMQualityGrade {
	FM_QUALITY_A,
	FM_QUALITY_B,
	FM_QUALITY_C,
	FM_QUALITY_D
};

const char *qualityGradeString(FMQualityGrade grade);


struct FMSolution {
	Math::NODAL_PLANE np1;
	Math::NODAL_PLANE np2;
	double            misfit;        // weighted polarity misfit fraction [0,1]
	double            stdr;          // station distribution ratio [0,1]
	double            azimuthalGap;  // largest azimuthal gap in degrees
	int               stationCount;  // number of polarities used
	int               misfitCount;   // number of misfitting stations
	double            rmsDiff;       // RMS angular diff of acceptable mechanisms (degrees)
	double            prob;          // fraction of acceptable mechanisms within 30 degrees
	double            quality;       // composite quality metric [0,1], higher is better
	FMQualityGrade    grade;         // HASH quality letter grade

	//! Indices of misfitting observations (from PolarityObservation::index)
	std::vector<int>  misfittingStations;
};


/**
 * @brief Compute the largest azimuthal gap in degrees between observations.
 *
 * A gap > 180 degrees typically indicates a poorly constrained solution.
 * For automatic processing, reject mechanisms with gap > 90-120 degrees.
 */
double computeAzimuthalGap(
	const std::vector<PolarityObservation> &observations
);


/**
 * @brief Determine HASH quality grade from solution metrics.
 *
 * Uses the four criteria from Hardebeck & Shearer (2002) as implemented
 * in HASH/hashpy: probability (fraction within 30 degrees), RMS angular
 * difference, weighted misfit fraction, and STDR.
 */
FMQualityGrade computeQualityGrade(
	double prob, double rmsDiff, double misfit, double stdr
);


/**
 * @brief Predict first motion polarity for a given mechanism and ray.
 * @param np Nodal plane (strike/dip/rake in degrees)
 * @param azimuth Ray azimuth in degrees
 * @param takeoff Ray takeoff angle in degrees
 * @return +1 for compressional, -1 for dilatational, 0 for nodal
 *
 * Public so it can be used to identify misfitting stations.
 */
int predictPolarity(
	const Math::NODAL_PLANE &np,
	double azimuth, double takeoff
);


/**
 * @brief HASH-style first motion polarity inversion.
 *
 * Performs a grid search over strike/dip/rake space to find the set
 * of acceptable focal mechanisms that fit observed P-wave first motion
 * polarities, following the HASH method (Hardebeck & Shearer, 2002,
 * Bull. Seismol. Soc. Am. 92, 2264-2276).
 *
 * Perturbation of takeoff angles and azimuths follows the approach of
 * SKHASH (Skoumal, Hardebeck & Shearer, 2024, Seismol. Res. Lett. 95,
 * 2519-2526) for the case where pre-computed takeoff angles are used
 * as input rather than velocity models and hypocentral locations.
 *
 * The preferred solution is computed by averaging the set of acceptable
 * mechanisms with iterative outlier removal, following the MECH_PROB
 * algorithm from HASH. Quality grades are assigned using the four
 * HASH criteria: probability (fraction within 30 degrees), RMS angular
 * difference, weighted misfit fraction, and STDR.
 */
class SC_SYSTEM_CORE_API FirstMotionInversion {
	public:
		FirstMotionInversion();

		//! Set grid search spacing in degrees (default: 5)
		void setGridSpacing(double degrees);
		double gridSpacing() const { return _gridSpacing; }

		//! Set number of perturbation trials (default: 50)
		void setNumTrials(int n);
		int numTrials() const { return _numTrials; }

		//! Set takeoff angle uncertainty in degrees (default: 10)
		void setTakeoffUncertainty(double degrees);
		double takeoffUncertainty() const { return _takeoffUncertainty; }

		//! Set azimuth uncertainty in degrees (default: 5)
		void setAzimuthUncertainty(double degrees);
		double azimuthUncertainty() const { return _azimuthUncertainty; }

		//! Set fraction of picks presumed bad (default: 0.1)
		void setBadFraction(double frac);
		double badFraction() const { return _badFrac; }

		/**
		 * @brief Run the inversion.
		 * @param observations Vector of polarity observations (azimuth, takeoff, polarity)
		 * @return The preferred (averaged) solution. Empty vector if fewer
		 *         than MIN_OBSERVATIONS valid observations or no acceptable
		 *         mechanisms found.
		 *
		 * Returns a single FMSolution representing the preferred mechanism
		 * computed by averaging the set of acceptable mechanisms with
		 * iterative outlier removal (HASH MECH_PROB algorithm).
		 */
		std::vector<FMSolution> compute(
			const std::vector<PolarityObservation> &observations
		) const;

		static constexpr size_t MIN_OBSERVATIONS = 6;

	private:
		double _gridSpacing;
		int    _numTrials;
		double _takeoffUncertainty;
		double _azimuthUncertainty;
		double _badFrac;
};


}


#endif
