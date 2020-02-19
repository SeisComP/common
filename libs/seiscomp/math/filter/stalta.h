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


#ifndef SEISCOMP_PICKER_STALTA_H__
#define SEISCOMP_PICKER_STALTA_H__

#include<vector>
#include<seiscomp/math/filter.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
class STALTA : public InPlaceFilter<TYPE> {

  public:
	STALTA(double lenSTA=2, double lenLTA=50, double fsamp=1.);

	void setSaveIntermediate(bool);

	// Apply the picker in place to the (previously filtered) data.
	void apply(int ndata, TYPE *data);

	void reset();

	bool changed() { return true; }

	void setSamplingFrequency(double fsamp);
	int setParameters(int n, const double *params);

	InPlaceFilter<TYPE>* clone() const;

	const std::vector<TYPE>& getSTA()    const { return _staVector; };
	const std::vector<TYPE>& getLTA()    const { return _ltaVector; };

  protected:
	bool _saveIntermediate;

  protected:
	// config
	int     _numSTA, _numLTA,
		_sampleCount, _initLength; // number of samples for init/reset
	double  _lenSTA, _lenLTA, _fsamp;

	// state
	double _sta, _lta; // must be double

  protected:
	std::vector<TYPE> _staVector, _ltaVector;
};


template<typename TYPE>
class STALTA2 : public InPlaceFilter<TYPE> {

  public:
	STALTA2(double lenSTA=2, double lenLTA=50, double eventON=3.,
	        double eventOFF=1., double fsamp=1.);

	void setSaveIntermediate(bool);

	// Apply the picker in place to the (previously filtered) data.
	void apply(int ndata, TYPE *data);

	void reset();

	bool changed() { return true; }

	void setSamplingFrequency(double fsamp);
	int setParameters(int n, const double *params);

	InPlaceFilter<TYPE>* clone() const;

	const std::vector<TYPE>& getSTA()    const { return _staVector; };
	const std::vector<TYPE>& getLTA()    const { return _ltaVector; };

  protected:
	bool _saveIntermediate;

  protected:
	// config
	int     _numSTA, _numLTA,
	        _sampleCount, _initLength; // number of samples for init/reset
	double  _lenSTA, _lenLTA, _fsamp;
	double  _eventOn, _eventOff;
	double  _bleed;

	// state
	double _sta, _lta; // must be double

  protected:
	std::vector<TYPE> _staVector, _ltaVector;
};


} // namespace Seiscomp::Math::Filter
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
