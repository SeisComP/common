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


#define SEISCOMP_COMPONENT Mag

#include <seiscomp/logging/log.h>
#include <seiscomp/processing/magnitudeprocessor.h>
#include <seiscomp/utils/units.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <string.h>

IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Processing::MagnitudeProcessor, SC_SYSTEM_CLIENT_API);

namespace Seiscomp {
namespace Processing {

IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(MagnitudeProcessor, Core::BaseObject, "MagnitudeProcessor");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::MagnitudeProcessor() {
	setCorrectionCoefficients(1.0, 0.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::MagnitudeProcessor(const std::string& type)
 : _type(type) {
	setCorrectionCoefficients(1.0, 0.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::~MagnitudeProcessor() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &MagnitudeProcessor::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor::typeMw() const {
	return "Mw(" + _type + ")";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor::amplitudeType() const {
	return type();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::setup(const Settings &settings) {
	try {
		_linearCorrection = settings.getDouble("mag." + type() + ".multiplier");
		SEISCOMP_DEBUG("%s.%s: Setting mag.%s.multiplier to %.2f",
		               settings.networkCode.c_str(),
		               settings.stationCode.c_str(),
		               _type.c_str(), _linearCorrection);
	}
	catch ( ... ) {
		_linearCorrection = 1.0;
	}

	try {
		_constantCorrection = settings.getDouble("mag." + type() + ".offset");
		SEISCOMP_DEBUG("%s.%s: Setting mag.%s.offset to %.2f",
		               settings.networkCode.c_str(),
		               settings.stationCode.c_str(),
		               _type.c_str(), _constantCorrection);
	}
	catch ( ... ) {
		_constantCorrection = 0.0;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor::estimateMw(
	double magnitude,
	double &estimation,
	double &error)
{
	return MwEstimationNotSupported;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor::setCorrectionCoefficients(double a, double b) {
	_linearCorrection = a;
	_constantCorrection = b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MagnitudeProcessor::correctMagnitude(double val) const {
	return _linearCorrection * val + _constantCorrection;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::convertAmplitude(double &amplitude,
                                          const std::string &amplitudeUnit,
                                          const std::string &desiredAmplitudeUnit) const {
	if ( amplitudeUnit.empty() || (amplitudeUnit == desiredAmplitudeUnit) ) {
		// No changes required
		return true;
	}

	const Util::UnitConversion *uc = Util::UnitConverter::get(amplitudeUnit);
	if ( uc == NULL ) {
		// No conversion known, invalid amplitude unit
		return false;
	}

	// Convert to SI
	double amplitudeSI = uc->convert(amplitude);

	uc = Util::UnitConverter::get(desiredAmplitudeUnit);
	if ( uc == NULL ) {
		SEISCOMP_ERROR("This must not happen: no converter for amplitude target unit '%s'",
		               desiredAmplitudeUnit.c_str());
		// This must not happen. The desired amplitude unit should always
		// have a mapping.
		return false;
	}

	double desiredAmplitude = uc->revert(amplitudeSI);

	SEISCOMP_DEBUG("Converted amplitude from %f %s to %f %s",
	               amplitude, amplitudeUnit.c_str(),
	               desiredAmplitude, desiredAmplitudeUnit.c_str());

	amplitude = desiredAmplitude;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::treatAsValidMagnitude() const {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor::finalizeMagnitude(DataModel::StationMagnitude *magnitude) const {
	// Nothing
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
