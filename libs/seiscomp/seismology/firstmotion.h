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


namespace Seiscomp {
namespace Seismology {


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
 * @brief HASH-style quality grade (Hardebeck & Shearer 2002).
 *
 * A: Well-constrained. Misfit <= 0.15, STDR >= 0.5, gap <= 90, stations >= 8
 * B: Fairly well-constrained. Misfit <= 0.2, STDR >= 0.4, gap <= 120, stations >= 8
 * C: Poorly constrained. Misfit <= 0.3, STDR >= 0.3, gap <= 150, stations >= 6
 * D: Unreliable. Anything worse than C.
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
	double            misfit;        // fraction of misfitting polarities [0,1]
	double            stdr;          // station distribution ratio [0,1]
	double            azimuthalGap;  // largest azimuthal gap in degrees
	int               stationCount;  // number of polarities used
	int               misfitCount;   // number of misfitting stations
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
 */
FMQualityGrade computeQualityGrade(
	double misfit, double stdr, double azimuthalGap, int stationCount
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
 * Performs a grid search over strike/dip/rake space to find focal
 * mechanisms that best fit observed P-wave first motion polarities.
 * Uses perturbation trials to assess solution stability (HASH method,
 * Hardebeck & Shearer 2002).
 */
class SC_SYSTEM_CORE_API FirstMotionInversion {
	public:
		FirstMotionInversion();

		//! Set grid search spacing in degrees (default: 5)
		void setGridSpacing(double degrees);
		double gridSpacing() const { return _gridSpacing; }

		//! Set number of perturbation trials for stability (default: 100)
		void setNumTrials(int n);
		int numTrials() const { return _numTrials; }

		//! Set angular uncertainty for perturbation in degrees (default: 10)
		void setAngleUncertainty(double degrees);
		double angleUncertainty() const { return _angleUncertainty; }

		/**
		 * @brief Run the inversion.
		 * @param observations Vector of polarity observations (azimuth, takeoff, polarity)
		 * @return Sorted vector of solutions (best first). Empty if fewer than
		 *         MIN_OBSERVATIONS valid observations.
		 */
		std::vector<FMSolution> compute(
			const std::vector<PolarityObservation> &observations
		) const;

		static const int MIN_OBSERVATIONS = 6;

	private:
		static double computeMisfit(
			const Math::NODAL_PLANE &np,
			const std::vector<PolarityObservation> &observations
		);

		static double computeSTDR(
			const Math::NODAL_PLANE &np,
			const std::vector<PolarityObservation> &observations
		);

		static std::vector<int> findMisfittingStations(
			const Math::NODAL_PLANE &np,
			const std::vector<PolarityObservation> &observations
		);

		double _gridSpacing;
		int    _numTrials;
		double _angleUncertainty;
};


}
}


#endif
