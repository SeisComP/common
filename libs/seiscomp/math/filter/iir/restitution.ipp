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


template<class TYPE>
RestitutionFilter<TYPE>::RestitutionFilter(
	double _T0, double _h, double _gain)
	: T0(_T0), h(_h), gain(_gain)
{
	_fsamp = 0;
}


template<class TYPE>
RestitutionFilter<TYPE>::~RestitutionFilter()
{
	this->clear();
}


template<class TYPE>
void RestitutionFilter<TYPE>::init()
{
std::cerr << "init"<<std::endl;
	std::string error;
	bool OK = coefficients_from_T0_h(_fsamp, gain, T0, h, c0, c1, c2);
	if ( ! OK)
		error = "could not compute coefficients";

	// sanity check
	if (_fsamp <= 0)
		error = "sampling frequency not specified";
	if (bp_order < 0)
		error = "bandpass not specified";
	if (c0 == 0 && c1 == 0 && c2 == 0)
		error = "restitution coefficients not specified";
	if ( ! error.empty())
		throw std::runtime_error(error);

	// Start from scratch by removing all biquads
	this->clear();

	// Add the biquad that produces the restituted acceleration
	double b0 = c2, b1 = c1, b2 = c0;
	double a0 =  1, a1 = -1, a2 =  0;
	Biquad<TYPE> biquad(b0, b1, b2, a0, a1, a2);
	this->push_back(biquad);

	// Add the bandpass but don't exceed the Nyquist frequency!
	// If either the band pass fmax was unset (in other words: set to 0)
	// or exceeds the Nyquist frequency, then we create a high pass
	// rather than a band pass.
	double fnyqist = 0.5*_fsamp;

	if (bp_fmax == 0 || bp_fmax > 0.8*fnyqist) {
		ButterworthHighpass<TYPE> flt(bp_order, bp_fmin);
		flt.setSamplingFrequency(_fsamp);
		for (auto &biquad : flt)
			this->push_back(biquad);
	}
	else {
		ButterworthBandpass<TYPE> flt(bp_order, bp_fmin, bp_fmax);
		flt.setSamplingFrequency(_fsamp);
		for (auto &biquad : flt)
			this->push_back(biquad);
	}

	// Add the double integration.
	double q = 0.5/(_fsamp*_fsamp);
	Biquad<TYPE> flt(q, q, 0, 1, -2, 1);
	flt.setSamplingFrequency(_fsamp);
	this->push_back(flt);
}


template<class TYPE>
void RestitutionFilter<TYPE>::setParameters(double _T0, double _h, double _gain)
{
	T0 = _T0;
	h = _h;
	gain = _gain;
}


template<class TYPE>
int RestitutionFilter<TYPE>::setParameters(int n, const double *params)
{
	if (n != 3)
		return 3; // TODO: clarify

	T0 = params[0];
	h = params[1];
	gain = params[2];

        return n;
}


template<class TYPE>
void RestitutionFilter<TYPE>::setBandpass(int order, double fmin, double fmax)
{
	bp_order = order;
	bp_fmin  = fmin;
	bp_fmax  = fmax;
}


template<class TYPE>
void RestitutionFilter<TYPE>::setSamplingFrequency(double fsamp)
{
std::cerr << "RestitutionFilter<TYPE>::setSamplingFrequency()"<<std::endl;
	_fsamp = fsamp;
	init();
}


template<class TYPE>
void RestitutionFilter<TYPE>::setCoefficients(double _c0, double _c1, double _c2)
{
	c0 = _c0;
	c1 = _c1;
	c2 = _c2;
}


template<class TYPE>
std::string RestitutionFilter<TYPE>::info() const
{
	std::stringstream tmp;
	tmp << "IIR::RestitutionFilter" << std::endl;
	if (_fsamp <= 0.)
		tmp << "  Not yet initialized" << std::endl;
	else {
		tmp << "  fsamp   = " <<  _fsamp << std::endl;
		tmp << "  gain    = " <<    gain << std::endl;
		tmp << "  c0*gain = " << c0*gain << std::endl;
		tmp << "  c1*gain = " << c1*gain << std::endl;
		tmp << "  c2*gain = " << c2*gain << std::endl;
	}
	return tmp.str();
}
