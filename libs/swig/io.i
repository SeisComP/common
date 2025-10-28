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

%module(package="seiscomp", directors="1") io
%{
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/greensfunction.h>
#include <seiscomp/core/datamessage.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/math/coord.h>
#include <seiscomp/math/math.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/rmhp.h>
#include <seiscomp/math/filter/taper.h>
#include <seiscomp/math/filter/average.h>
#include <seiscomp/math/filter/stalta.h>
#include <seiscomp/math/filter/chainfilter.h>
#include <seiscomp/math/filter/biquad.h>
#include <seiscomp/math/filter/butterworth.h>
#include <seiscomp/math/filter/taper.h>
#include <seiscomp/math/filter/seismometers.h>
#include <seiscomp/math/restitution/transferfunction.h>
#include <seiscomp/io/database.h>
#include <seiscomp/io/recordinput.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/io/recordfilter.h>
#include <seiscomp/io/recordfilter/pipe.h>
#include <seiscomp/io/recordfilter/crop.h>
#include <seiscomp/io/recordfilter/demux.h>
#include <seiscomp/io/recordfilter/iirfilter.h>
#include <seiscomp/io/recordfilter/mseedencoder.h>
#include <seiscomp/io/recordfilter/resample.h>
#include <seiscomp/io/recordstreamexceptions.h>
#include <seiscomp/io/importer.h>
#include <seiscomp/io/exporter.h>
#include <seiscomp/io/gfarchive.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/io/recordstream/file.h>
#include <seiscomp/io/recordstream/slconnection.h>
#include <seiscomp/io/recordstream/combined.h>
%}

%feature("director") Seiscomp::IO::ExportSink;

%newobject Seiscomp::IO::DatabaseInterface::Create;
%newobject Seiscomp::IO::DatabaseInterface::Open;
%newobject Seiscomp::IO::RecordStream::Create;
%newobject Seiscomp::IO::RecordStream::Open;
%newobject Seiscomp::IO::RecordStream::next;
%newobject Seiscomp::IO::RecordInput::next;
%newobject Seiscomp::IO::RecordIterator::current;
%newobject Seiscomp::IO::Importer::read;
%newobject Seiscomp::IO::GFArchive::get;
%newobject Seiscomp::IO::Exporter::Create;

%include std_ios.i
%include std_char_traits.i

%rename Seiscomp::RecordStream::File FileRecordStream;

%newobject Seiscomp::IO::RecordFilterInterface::feed;
%newobject Seiscomp::IO::RecordFilterInterface::flush;

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

%typemap(in) (const char *data, int size) {
    $1 = PyBytes_AsString($input);
    $2 = PyBytes_Size($input);
}

%typemap(directorin) (const char *data, int size) {
    $input = PyBytes_FromStringAndSize($1, $2);
}

%import "math.i"
%include "seiscomp/core.h"
%include "seiscomp/io/database.h"
%include "seiscomp/io/gfarchive.h"
%include "seiscomp/io/recordstream.h"
%include "seiscomp/io/recordinput.h"
%include "seiscomp/io/recordfilter.h"
%include "seiscomp/io/recordfilter/pipe.h"
%include "seiscomp/io/recordfilter/crop.h"
%include "seiscomp/io/recordfilter/demux.h"
%include "seiscomp/io/recordfilter/iirfilter.h"
%include "seiscomp/io/recordfilter/mseedencoder.h"
%include "seiscomp/io/recordfilter/resample.h"
%include "seiscomp/io/recordstreamexceptions.h"
%include "seiscomp/io/importer.h"
%include "seiscomp/io/exporter.h"
%include "seiscomp/io/archive/xmlarchive.h"
%include "seiscomp/io/archive/binarchive.h"
//#ifdef HAVE_MSEED
%include "seiscomp/io/records/mseedrecord.h"
%include "seiscomp/io/recordstream/file.h"
%include "seiscomp/io/recordstream/slconnection.h"
%include "seiscomp/io/recordstream/combined.h"
//#endif


%extend Seiscomp::IO::RecordInput {
	%pythoncode %{
		def __iter__(self):
		    while 1:
		        rec = self.next()
		        if not rec:
		            return
		        yield rec

		def __next__(self):
		    rec = self.next()
		    if not rec:
		        return
		    return rec

		## for Python 2 compatibility
		#def next(self):
		#    return self.__next__()
	%}
};


%template(RecordIIRFilterF) Seiscomp::IO::RecordIIRFilter<float>;
%template(RecordIIRFilterD) Seiscomp::IO::RecordIIRFilter<double>;

%template(RecordResamplerF) Seiscomp::IO::RecordResampler<float>;
%template(RecordResamplerD) Seiscomp::IO::RecordResampler<double>;
%template(RecordResamplerI) Seiscomp::IO::RecordResampler<int>;

%template(ExportObjectList) std::vector<Seiscomp::Core::BaseObject*>;
