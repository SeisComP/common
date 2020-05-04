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


#ifndef SEISCOMP_PROCESSING_RESPONSE_H
#define SEISCOMP_PROCESSING_RESPONSE_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/math/restitution/transferfunction.h>
#include <seiscomp/math/filter/seismometers.h>
#include <seiscomp/client.h>

#include <vector>


namespace Seiscomp {
namespace Processing  {


DEFINE_SMARTPOINTER(Response);

class SC_SYSTEM_CLIENT_API Response : public Core::BaseObject {
	DECLARE_CASTS(Response)


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		// C'tor
		Response();
		// D'tor
		virtual ~Response();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Deconvolves data in the frequency domain.
		//! This method incorporates the gain. If no transfer function
		//! can be retrieved false is returned.
		bool deconvolveFFT(int n, float *inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		bool deconvolveFFT(int n, double *inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		bool deconvolveFFT(FloatArray &inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		bool deconvolveFFT(DoubleArray &inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		//! Returns a transfer function that can be used to deconvolve the
		//! data. The transfer function does not incorporate the gain.
		//! @param numberOfIntegrations How often to integrate. In case of
		//!                             'poles and zeros' this will push n
		//!                             additional zeros to 'zeros'.
		virtual Math::Restitution::FFT::TransferFunction *
			getTransferFunction(int numberOfIntegrations = 0);
};


DEFINE_SMARTPOINTER(ResponsePAZ);

class SC_SYSTEM_CLIENT_API ResponsePAZ : public Response {
	DECLARE_CASTS(ResponsePAZ)


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		typedef std::vector<Math::Complex> ComplexArray;
		typedef ComplexArray Poles;
		typedef ComplexArray Zeros;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		// C'tor
		ResponsePAZ();
		// D'tor
		virtual ~ResponsePAZ();


	// ----------------------------------------------------------------------
	//  Public attributes
	// ----------------------------------------------------------------------
	public:
		void setNormalizationFactor(const OPT(double)& normalizationFactor);
		double normalizationFactor() const;

		void setNormalizationFrequency(const OPT(double)& normalizationFrequency);
		double normalizationFrequency() const;

		void setPoles(const Poles& poles);
		const Poles& poles() const;

		void setZeros(const Zeros& zeros);
		const Zeros& zeros() const;

		void convertFromHz();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		Math::Restitution::FFT::TransferFunction *
			getTransferFunction(int numberOfIntegrations = 0) override;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		OPT(double) _normalizationFactor;
		OPT(double) _normalizationFrequency;

		Poles       _poles;
		Zeros       _zeros;
};


DEFINE_SMARTPOINTER(ResponseFAP);

class SC_SYSTEM_CLIENT_API ResponseFAP : public Response {
	DECLARE_CASTS(ResponseFAP)


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		// C'tor
		ResponseFAP();
		// D'tor
		virtual ~ResponseFAP();


	// ----------------------------------------------------------------------
	//  Public attributes
	// ----------------------------------------------------------------------
	public:
		void setFAPs(const Math::SeismometerResponse::FAPs& faps);
		const Math::SeismometerResponse::FAPs& faps() const;

		void convertFromHz();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		Math::Restitution::FFT::TransferFunction *
			getTransferFunction(int numberOfIntegrations = 0) override;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		Math::SeismometerResponse::FAPs _faps;
};


}
}

#endif
