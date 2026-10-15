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


#ifndef SEISCOMP_PROCESSING_SPECTRALDIAGNOSTICS_H
#define SEISCOMP_PROCESSING_SPECTRALDIAGNOSTICS_H


#include <seiscomp/core/timewindow.h>
#include <seiscomp/core/optional.h>
#include <seiscomp/client.h>

#include <string>
#include <utility>
#include <vector>


namespace Seiscomp {
namespace Processing {


/**
 * @brief A curve of values over frequency, e.g. a signal spectrum, a noise
 *        spectrum, a fitted source model or a path correction.
 */
struct SC_SYSTEM_CLIENT_API SpectralCurve {
	enum Role {
		Signal,     //!< Measured signal spectrum
		Noise,      //!< Measured noise spectrum (same processing as signal)
		Model,      //!< Fitted or theoretical model
		Correction, //!< Applied correction, e.g. attenuation or site term
		Response,   //!< Instrument response
		Other
	};

	Role                role{Other};
	//! Label shown in the legend, e.g. "Z corrected"
	std::string         label;
	//! Component code (e.g. 'Z', 'N', 'E') or 0 if not applicable
	char                component{0};
	//! Unit of the values, e.g. "nm*s"
	std::string         unit;
	//! Frequencies in Hz, ascending
	std::vector<double> freq;
	//! Linear values, one per frequency
	std::vector<double> value;
};


/**
 * @brief A named scalar result of a spectral measurement, e.g. the spectral
 *        level, a corner frequency or a fit misfit.
 */
struct SC_SYSTEM_CLIENT_API SpectralParameter {
	std::string id;
	double      value{0};
	OPT(double) lowerUncertainty;
	OPT(double) upperUncertainty;
	std::string unit;
	//! Whether a viewer should draw the value as a frequency marker
	bool        frequencyMarker{false};
};


/**
 * @brief A time window of the waveform from which a spectrum was computed.
 */
struct SC_SYSTEM_CLIENT_API SpectralWindow {
	//! SpectralCurve::Signal or SpectralCurve::Noise
	SpectralCurve::Role role{SpectralCurve::Signal};
	//! Component code or 0 if not applicable
	char                component{0};
	Core::TimeWindow    window;
};


/**
 * @brief Everything a viewer needs to display and validate a spectral
 *        amplitude measurement of one station.
 */
struct SC_SYSTEM_CLIENT_API SpectralDiagnostics {
	std::vector<SpectralCurve>            curves;
	//! Frequency bands in Hz which were used, e.g. the fit band
	std::vector<std::pair<double,double>> bands;
	std::vector<SpectralParameter>        parameters;
	//! The waveform time windows the spectra were computed from
	std::vector<SpectralWindow>           windows;
	//! Empty or "ok" if the measurement succeeded, the reason otherwise
	std::string                           status;

	void clear();
	bool empty() const;
};


/**
 * @brief Interface an amplitude processor can implement in addition to
 *        AmplitudeProcessor to expose the spectral diagnostics of its last
 *        measurement. Processors implementing it should report the
 *        AmplitudeProcessor::Spectrum capability.
 *
 * Diagnostics are kept in memory only. They are valid after the processor
 * has processed data (or has been reprocessed) and until it is reset.
 */
class SC_SYSTEM_CLIENT_API SpectralDiagnosticsProvider {
	public:
		virtual ~SpectralDiagnosticsProvider();

	public:
		//! Returns the diagnostics of the last measurement or nullptr.
		virtual const SpectralDiagnostics *spectralDiagnostics() const = 0;

		//! Whether the analysed frequency band can be set by the user.
		virtual bool canSetSpectralBand() const;

		/**
		 * @brief Sets the frequency band used for the measurement, e.g. the
		 *        fit band. It is applied with the next (re)processing.
		 * @param fmin Lower frequency in Hz. fmin <= 0 restores the
		 *             automatic band selection.
		 * @param fmax Upper frequency in Hz
		 * @return Whether the band was accepted
		 */
		virtual bool setSpectralBand(double fmin, double fmax);
};


}
}


#endif
