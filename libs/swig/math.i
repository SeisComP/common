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

%module(package="seiscomp", docstring="Codes for various geographical computations and filters") math
%{
/* headers to be included in the wrapper code */
#include "seiscomp/math/geo.h"
#include "seiscomp/math/coord.h"
#include "seiscomp/math/math.h"
#include "seiscomp/math/filter.h"
#include "seiscomp/math/filter/rmhp.h"
#include "seiscomp/math/filter/taper.h"
#include "seiscomp/math/filter/average.h"
#include "seiscomp/math/filter/stalta.h"
#include "seiscomp/math/filter/chainfilter.h"
#include "seiscomp/math/filter/biquad.h"
#include "seiscomp/math/filter/butterworth.h"
#include "seiscomp/math/filter/taper.h"
#include "seiscomp/math/filter/seismometers.h"
#include "seiscomp/math/restitution/transferfunction.h"
#include "seiscomp/math/restitution/td.h"


#include "seiscomp/core/typedarray.h"
#include "seiscomp/core/record.h"
#include "seiscomp/core/greensfunction.h"
#include "seiscomp/core/genericrecord.h"
#include "seiscomp/core/genericmessage.h"
#include "seiscomp/core/datamessage.h"
#include "seiscomp/core/interruptible.h"
%}

%include "stl.i"
%include "std_vector.i"
%import "core.i"
%feature ("autodoc", "1");
%include "seiscomp/core.h"

namespace std {
   %template(vectorf) vector<float>;
   %template(vectord) vector<double>;
};

%exception {
  try {
    $action
  }
  catch ( const Seiscomp::Core::ValueException &e) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
  catch ( const std::exception &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
  catch ( ... ) {
    SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
  }
}

%apply double *OUTPUT { double *out_dist,  double *out_azi1, double *out_azi2 };
%apply double *OUTPUT { double *out_lat,   double *out_lon };
%apply double *OUTPUT { double *out_latx1, double *out_lonx1,
                        double *out_latx2, double *out_lonx2 };
%apply double *OUTPUT { double *dist, double *azi };

%ignore nearestHotspot(double, double, double, int, const NamedCoordD*, double *, double *);

%include "seiscomp/math/math.h"
%include "seiscomp/math/coord.h"

%template(CoordF) Seiscomp::Math::Geo::Coord<float>;
%template(CoordD) Seiscomp::Math::Geo::Coord<double>;

%template(NamedCoordF) Seiscomp::Math::Geo::NamedCoord<float>;
%template(NamedCoordD) Seiscomp::Math::Geo::NamedCoord<double>;

%template(HotspotListF) std::vector<Seiscomp::Math::Geo::NamedCoordF>;
%template(HotspotListD) std::vector<Seiscomp::Math::Geo::NamedCoordD>;

%template(CityF) Seiscomp::Math::Geo::City<float>;
%template(CityD) Seiscomp::Math::Geo::City<double>;

%template(CityListF) std::vector<Seiscomp::Math::Geo::CityF>;
%template(CityListD) std::vector<Seiscomp::Math::Geo::CityD>;

%newobject Seiscomp::Math::Filtering::InPlaceFilter<float>::Create;
%newobject Seiscomp::Math::Filtering::InPlaceFilter<double>::Create;
%ignore Seiscomp::Math::Filtering::InPlaceFilter::apply(int, float*);
%ignore Seiscomp::Math::Filtering::InPlaceFilter::apply(int, double*);
%include "seiscomp/math/filter.h"

%template(InPlaceFilterF) Seiscomp::Math::Filtering::InPlaceFilter<float>;
%template(InPlaceFilterD) Seiscomp::Math::Filtering::InPlaceFilter<double>;

%include "seiscomp/math/filter/average.h"

%template(AverageFilterF) Seiscomp::Math::Filtering::Average<float>;
%template(AverageFilterD) Seiscomp::Math::Filtering::Average<double>;

%include "seiscomp/math/filter/stalta.h"

%template(STALTAFilterF) Seiscomp::Math::Filtering::STALTA<float>;
%template(STALTAFilterD) Seiscomp::Math::Filtering::STALTA<double>;
%template(STALTA2FilterF) Seiscomp::Math::Filtering::STALTA2<float>;
%template(STALTA2FilterD) Seiscomp::Math::Filtering::STALTA2<double>;
%template(STALTAClassicFilterF) Seiscomp::Math::Filtering::STALTA_Classic<float>;
%template(STALTAClassicFilterD) Seiscomp::Math::Filtering::STALTA_Classic<double>;



%include "seiscomp/math/filter/rmhp.h"

%template(RunningMeanFilterF) Seiscomp::Math::Filtering::RunningMean<float>;
%template(RunningMeanFilterD) Seiscomp::Math::Filtering::RunningMean<double>;

%template(RunningMeanHighPassFilterF) Seiscomp::Math::Filtering::RunningMeanHighPass<float>;
%template(RunningMeanHighPassFilterD) Seiscomp::Math::Filtering::RunningMeanHighPass<double>;

%include "seiscomp/math/filter/taper.h"

%template(InitialTaperFilterF) Seiscomp::Math::Filtering::InitialTaper<float>;
%template(InitialTaperFilterD) Seiscomp::Math::Filtering::InitialTaper<double>;

%include "seiscomp/math/filter/biquad.h"

%template(BiquadCascadeF) Seiscomp::Math::Filtering::IIR::BiquadCascade<float>;
%template(BiquadCascadeD) Seiscomp::Math::Filtering::IIR::BiquadCascade<double>;

%include "seiscomp/math/filter/butterworth.h"

%template(ButterworthLowpassF) Seiscomp::Math::Filtering::IIR::ButterworthLowpass<float>;
%template(ButterworthLowpassD) Seiscomp::Math::Filtering::IIR::ButterworthLowpass<double>;

%template(ButterworthHighpassF) Seiscomp::Math::Filtering::IIR::ButterworthHighpass<float>;
%template(ButterworthHighpassD) Seiscomp::Math::Filtering::IIR::ButterworthHighpass<double>;

%template(ButterworthBandpassF) Seiscomp::Math::Filtering::IIR::ButterworthBandpass<float>;
%template(ButterworthBandpassD) Seiscomp::Math::Filtering::IIR::ButterworthBandpass<double>;

%template(ButterworthHighLowpassF) Seiscomp::Math::Filtering::IIR::ButterworthHighLowpass<float>;
%template(ButterworthHighLowpassD) Seiscomp::Math::Filtering::IIR::ButterworthHighLowpass<double>;

%template(ButterworthBandstopF) Seiscomp::Math::Filtering::IIR::ButterworthBandstop<float>;
%template(ButterworthBandstopD) Seiscomp::Math::Filtering::IIR::ButterworthBandstop<double>;


%newobject Seiscomp::Math::Filtering::ChainFilter<float>::take;
%newobject Seiscomp::Math::Filtering::ChainFilter<double>::take;

%include "seiscomp/math/filter/chainfilter.h"

%apply SWIGTYPE *DISOWN { Seiscomp::Math::Filtering::InPlaceFilter<float> *filter };
%apply SWIGTYPE *DISOWN { Seiscomp::Math::Filtering::InPlaceFilter<double> *filter };

%template(ChainFilterF) Seiscomp::Math::Filtering::ChainFilter<float>;
%template(ChainFilterD) Seiscomp::Math::Filtering::ChainFilter<double>;

%ignore Seiscomp::Math::Filtering::IIR::Filter<float>::apply(int, float*);
%ignore Seiscomp::Math::Filtering::IIR::Filter<double>::apply(int, double*);

%include "seiscomp/math/filter/seismometers.h"

%template(SeismometerFilterF) Seiscomp::Math::Filtering::IIR::Filter<float>;
%template(SeismometerFilterD) Seiscomp::Math::Filtering::IIR::Filter<double>;
%template(WWSSN_SPF) Seiscomp::Math::Filtering::IIR::WWSSN_SP_Filter<float>;
%template(WWSSN_SPD) Seiscomp::Math::Filtering::IIR::WWSSN_SP_Filter<double>;
%template(WWSSN_LPF) Seiscomp::Math::Filtering::IIR::WWSSN_LP_Filter<float>;
%template(WWSSN_LPD) Seiscomp::Math::Filtering::IIR::WWSSN_LP_Filter<double>;
%template(WoodAndersonFilterF) Seiscomp::Math::Filtering::IIR::WoodAndersonFilter<float>;
%template(WoodAndersonFilterD) Seiscomp::Math::Filtering::IIR::WoodAndersonFilter<double>;
%template(GenericSeismometerFilterF) Seiscomp::Math::Filtering::IIR::GenericSeismometer<float>;
%template(GenericSeismometerFilterD) Seiscomp::Math::Filtering::IIR::GenericSeismometer<double>;

%include "seiscomp/math/geo.h"

%template(deg2km) Seiscomp::Math::Geo::deg2km<double>;

%rename(TransferFunctionPAZ) Seiscomp::Math::Restitution::FFT::PolesAndZeros;

%include "seiscomp/math/restitution/types.h"
%include "seiscomp/math/restitution/transferfunction.h"

%template(vectorc) std::vector<Seiscomp::Math::Complex>;

%include "seiscomp/math/restitution/td.h"

//%template(TimeDomainF) Seiscomp::Math::Restitution::TimeDomain<float>;
//%template(TimeDomainD) Seiscomp::Math::Restitution::TimeDomain<double>;
%template(TimeDomain_from_T0_h_F) Seiscomp::Math::Restitution::TimeDomain_from_T0_h<float>;
%template(TimeDomain_from_T0_h_D) Seiscomp::Math::Restitution::TimeDomain_from_T0_h<double>;
%template(TimeDomain_from_T1_T2_F) Seiscomp::Math::Restitution::TimeDomain_from_T1_T2<float>;
%template(TimeDomain_from_T1_T2_D) Seiscomp::Math::Restitution::TimeDomain_from_T1_T2<double>;
%template(TimeDomainNullFilterF) Seiscomp::Math::Restitution::TimeDomainNullFilter<float>;
%template(TimeDomainNullFilterD) Seiscomp::Math::Restitution::TimeDomainNullFilter<double>;
%template(TimeDomainGenericF) Seiscomp::Math::Restitution::TimeDomainGeneric<float>;
%template(TimeDomainGenericD) Seiscomp::Math::Restitution::TimeDomainGeneric<double>;
