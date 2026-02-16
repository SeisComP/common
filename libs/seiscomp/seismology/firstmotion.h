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
	double takeoff;   // degrees, takeoff angle at source (0=down, 180=up)
	int    polarity;  // +1 = compressional (up), -1 = dilatational (down)
	int    index;     // original arrival index (-1 if not set)

	PolarityObservation()
	: azimuth(0), takeoff(0), polarity(0), index(-1) {}

	PolarityObservation(double azi, double toff, int pol, int idx = -1)
	: azimuth(azi), takeoff(toff), polarity(pol), index(idx) {}
};


/**
 * @brief Quality grade for a focal mechanism solution.
 */
enum class FMQuality {
	A,  //!< Well constrained: low misfit, small gap, good coverage
	B,  //!< Acceptable: moderate misfit or gap
	C,  //!< Poorly constrained: high misfit or large gap
	D   //!< Unreliable: very high misfit, huge gap, or too few stations
};


struct FMSolution {
	Math::NODAL_PLANE np1{0, 0, 0};
	Math::NODAL_PLANE np2{0, 0, 0};
	double            misfit{0};        // polarity misfit fraction [0,1]
	double            azimuthalGap{360}; // largest azimuthal gap in degrees
	int               stationCount{0};  // number of polarities used
	int               misfitCount{0};   // number of misfitting stations
	FMQuality         quality{FMQuality::D};

	//! Indices of misfitting observations (from PolarityObservation::index)
	std::vector<int>  misfittingStations;
};


/**
 * @brief Result of the polarity inversion including all accepted solutions.
 *
 * The best solution is in `best`. The `accepted` vector contains ALL
 * mechanisms that satisfied the misfit threshold, sorted by misfit count.
 * Use `accepted` to render a "cloud" of nodal lines showing the range
 * of possible orientations (solution ambiguity).
 */
struct FMInversionResult {
	FMSolution              best;
	std::vector<FMSolution> accepted;
	bool                    valid{false};
};


/**
 * @brief Predict P-wave first motion polarity for a given mechanism and ray.
 *
 * Uses the P-wave radiation pattern: amplitude = (n . r)(d . r)
 * where n = fault normal, d = slip direction (from np2nd in Aki & Richards
 * z-UP convention), and r = ray direction.
 *
 * Takeoff angle convention: 0 = downgoing, 90 = horizontal, 180 = upgoing,
 * matching SeisComP's TravelTimeTableInterface output (libtau, LOCSAT).
 *
 * @param np Nodal plane (strike/dip/rake in degrees)
 * @param azimuth Station azimuth from source in degrees
 * @param takeoff Takeoff angle at source in degrees (0=down, 180=up)
 * @return +1 for compressional, -1 for dilatational, 0 for nodal
 */
SC_SYSTEM_CORE_API int predictPolarity(
	const Math::NODAL_PLANE &np,
	double azimuth, double takeoff
);


/**
 * @brief Compute the largest azimuthal gap between observations.
 * @return Gap in degrees. Returns 360 if fewer than 2 observations.
 */
SC_SYSTEM_CORE_API double computeAzimuthalGap(
	const std::vector<PolarityObservation> &observations
);


/**
 * @brief Assess quality grade for a focal mechanism solution.
 *
 * Grades:
 * - A: misfit <= 10%, azimuthal gap <= 90, stations >= 10
 * - B: misfit <= 20%, azimuthal gap <= 150, stations >= 8
 * - C: misfit <= 30%, azimuthal gap <= 200, stations >= 6
 * - D: anything worse
 */
SC_SYSTEM_CORE_API FMQuality assessQuality(const FMSolution &sol);


/**
 * @brief Return a human-readable label for a quality grade.
 */
SC_SYSTEM_CORE_API const char *qualityLabel(FMQuality q);


/**
 * @brief Configuration for the polarity inversion.
 */
struct FMInversionConfig {
	double gridSpacing{5.0};        // degrees for strike/dip/rake grid
	double maxMisfitFraction{0.2};  // max fraction of wrong polarities to accept

	static constexpr size_t MIN_OBSERVATIONS = 6;

	/**
	 * @brief Validate and clamp configuration to sane ranges.
	 * @return true if config was already valid, false if clamping was needed.
	 */
	SC_SYSTEM_CORE_API bool validate();
};


/**
 * @brief Grid-search first motion polarity inversion.
 *
 * Searches over strike/dip/rake space to find the focal mechanism(s)
 * that best fit the observed P-wave first motion polarities.
 *
 * For each grid node, predicts polarities using the radiation pattern
 * and counts misfits. Mechanisms with misfit fraction <= maxMisfitFraction
 * are accepted.
 *
 * Returns an FMInversionResult containing the best solution and all
 * accepted solutions (for rendering the "cloud" of possible nodal lines).
 *
 * @param observations Vector of polarity observations
 * @param config Inversion parameters
 * @return FMInversionResult with valid=true on success
 */
SC_SYSTEM_CORE_API FMInversionResult invertPolarities(
	const std::vector<PolarityObservation> &observations,
	const FMInversionConfig &config = FMInversionConfig()
);


}


#endif
