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


#include <seiscomp/processing/magnitudes/Mwp.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/config/config.h>
#include <seiscomp/logging/log.h>
#include <math.h>

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm*s";

// PREM (Dziewonski & Anderson 1981) depth layers.
// Each row: { max_depth_km, alpha_m_s, rho_kg_m3 }
// Used to correct M0 for source depth instead of the hardcoded mantle values.
struct PremLayer { double depthKm; double alpha; double rho; };
static const PremLayer PREM[] = {
	{  15,  6400, 2800},   // continental crust
	{  35,  6800, 2900},   // lower crust
	{  80,  8050, 3380},   // lithospheric mantle
	{ 220,  8100, 3380},   // upper mantle (LVZ)
	{ 400,  8905, 3540},   // upper transition zone
	{ 600,  9990, 3820},   // lower transition zone
	{ 660, 10266, 3993},   // 660-km discontinuity
	{ 771, 10752, 4381},   // lower mantle top
	{1000, 11065, 4526},   // lower mantle
	{2000, 12254, 5074},   // deep lower mantle
	{2891, 13716, 5566},   // CMB
};
static const int NPREM = sizeof(PREM) / sizeof(PREM[0]);

void premAtDepth(double depthKm, double &alpha, double &rho) {
	for ( int i = 0; i < NPREM; ++i ) {
		if ( depthKm <= PREM[i].depthKm ) {
			alpha = PREM[i].alpha;
			rho   = PREM[i].rho;
			return;
		}
	}
	alpha = PREM[NPREM-1].alpha;
	rho   = PREM[NPREM-1].rho;
}

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_Mwp, "Mwp");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_Mwp::MagnitudeProcessor_Mwp()
 : MagnitudeProcessor("Mwp") {
	_minimumDistanceDeg = 5.0;
	_maximumDistanceDeg = 105.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_Mwp::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) ) {
		return false;
	}

	// Read from localConfiguration (global.cfg plain key or scconfig module.trunk. prefix).
	// settings.getDouble() uses a namespaced lookup that doesn't match bare global.cfg keys.
	const Seiscomp::Config::Config *cfg = settings.localConfiguration;
	auto readDouble = [&](const std::string &key, double &val) {
		if ( !cfg ) return;
		if ( cfg->getDouble(val, key) ) return;
		cfg->getDouble(val, "module.trunk." + key);
	};

	readDouble("magnitudes.Mwp.estimateMw.a",        _mwA);
	readDouble("magnitudes.Mwp.estimateMw.b",        _mwB);
	readDouble("magnitudes.Mwp.estimateMw.stdError", _mwStdError);

	SEISCOMP_DEBUG("Mwp estimateMw: a=%.4f  b=%.4f  stdError=%.4f",
	               _mwA, _mwB, _mwStdError);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double depth,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *, const Locale *,
	double &value) {

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	double alpha, rho;
	premAtDepth(depth, alpha, rho);

	if ( Magnitudes::compute_Mwp(amplitude*1.E-9, delta, value,
	                              0.0, 1.0, alpha, rho) ) {
		return OK;
	}
	else {
		return DistanceOutOfRange;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::estimateMw(
	const Config::Config *,
	double magnitude,
	double &estimation,
	double &stdError)
{
	estimation = _mwA * magnitude + _mwB;
	stdError = _mwStdError;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
