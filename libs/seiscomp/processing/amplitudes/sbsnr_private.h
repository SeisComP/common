/***************************************************************************
 * Copyright (C) Preparatory Commission for the Comprehensive              *
 * Nuclear-Test-Ban Treaty Organization (CTBTO).                           *
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



#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_SBSNR_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_SBSNR_H


#include <seiscomp/processing/amplitudeprocessor.h>


namespace {


class AmplitudeSBSNR : public Seiscomp::Processing::AmplitudeProcessor {
	public:
		AmplitudeSBSNR();

	protected:
		bool computeAmplitude(const Seiscomp::DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr);

	protected:
		const bool _demean;
		const double _filbuf;
		const double _taper;
		const int _ford;
		const double _flo;
		const double _fhi;
		const bool _zp;
		const bool _coherent;
		const double _stavLength;
		const double _stavFraction;
		double (*_stavFunction)(double x);
		const double _maxStavWindowLength;
		const double _ltavLength;
		const double _ltavFraction;
		double (*_ltavFunction)(double x);
		const double _ltavStabilityLength;

	private:
		double _preTriggerDataBufferLength;
		double _postTriggerDataBufferLength;
};


}


#endif
