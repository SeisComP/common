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

%module(package="seiscomp", docstring="Codes for various seismological computations") seismology
%{
/* headers to be included in the wrapper code */
#include "seiscomp/core/typedarray.h"
#include "seiscomp/core/genericrecord.h"
#include "seiscomp/core/exceptions.h"
#include "seiscomp/core/datamessage.h"
#include "seiscomp/core/greensfunction.h"
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
#include "seiscomp/io/recordstream.h"
#include "seiscomp/io/recordinput.h"
#include "seiscomp/io/recordfilter.h"
#include "seiscomp/io/recordfilter/demux.h"
#include "seiscomp/io/recordfilter/iirfilter.h"
#include "seiscomp/io/recordfilter/resample.h"
#include "seiscomp/io/importer.h"
#include "seiscomp/io/exporter.h"
#include "seiscomp/io/gfarchive.h"
#include "seiscomp/io/archive/binarchive.h"
#include "seiscomp/io/archive/xmlarchive.h"
#include "seiscomp/io/records/mseedrecord.h"
#include "seiscomp/io/recordstream/file.h"
#include "seiscomp/io/recordstream/slconnection.h"
#include "seiscomp/io/recordstream/arclink.h"
#include "seiscomp/io/recordstream/combined.h"

#include "seiscomp/datamodel/publicobjectcache.h"
#include "seiscomp/datamodel/messages.h"
#include "seiscomp/datamodel/databasequery.h"
#include "seiscomp/datamodel/eventparameters_package.h"
#include "seiscomp/datamodel/inventory_package.h"
#include "seiscomp/datamodel/config_package.h"
#include "seiscomp/datamodel/routing_package.h"

#include "seiscomp/seismology/regions.h"
#include "seiscomp/seismology/ttt.h"
#include "seiscomp/seismology/locator/locsat.h"
%}

%include "std_string.i"
%include "std_list.i"
%include "std_vector.i"

%import "io.i"
%import "datamodel.i"
%import "../../../system/libs/swig/config.i"

%template(TravelTimeList_internal) std::list<Seiscomp::TravelTime>;

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

%newobject Seiscomp::TravelTimeTableInterface::Create;
%newobject Seiscomp::TravelTimeTableInterface::compute;
%newobject Seiscomp::TravelTimeTable::compute;

%include "seiscomp/core.h"
%include "seiscomp/seismology/regions.h"
%include "seiscomp/seismology/locatorinterface.h"
%include "seiscomp/seismology/ttt.h"
%include "seiscomp/seismology/locator/locsat.h"
