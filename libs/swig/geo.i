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

%module(package="seiscomp", docstring="Codes for working with geo features (e.g. polygons)") geo
%{
/* headers to be included in the wrapper code */
#include "seiscomp/geo/coordinate.h"
#include "seiscomp/geo/boundingbox.h"
#include "seiscomp/geo/feature.h"
#include "seiscomp/geo/featureset.h"

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
#include "seiscomp/math/restitution/td.h"
#include "seiscomp/math/restitution/transferfunction.h"

#include "seiscomp/core/typedarray.h"
#include "seiscomp/core/record.h"
#include "seiscomp/core/greensfunction.h"
#include "seiscomp/core/genericrecord.h"
#include "seiscomp/core/genericmessage.h"
#include "seiscomp/core/datamessage.h"
#include "seiscomp/core/interruptible.h"
%}


// Make Swig parse GCC attributes happily
#define __attribute__(x)

%include "stl.i"
%include "std_vector.i"
%import "math.i"
%feature ("autodoc", "1");
%include "seiscomp/core.h"

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

%include "seiscomp/geo/coordinate.h"
%include "seiscomp/geo/boundingbox.h"
%include "seiscomp/geo/feature.h"
%include "seiscomp/geo/featureset.h"

%template(Categories) std::vector<Seiscomp::Geo::Category*>;
%template(GeoFeatures) std::vector<Seiscomp::Geo::GeoFeature*>;
%template(Vertices) std::vector<Seiscomp::Geo::GeoCoordinate>;
%template(Indexes) std::vector<size_t>;
