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

%{
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/record.h>
#include <seiscomp/core/greensfunction.h>
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/datamessage.h>
#include <seiscomp/geo/coordinate.h>
#include <seiscomp/geo/boundingbox.h>
#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/featureset.h>
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
#include <seiscomp/io/recordstream.h>
#include <seiscomp/io/recordinput.h>
#include <seiscomp/io/recordfilter.h>
#include <seiscomp/io/recordfilter/crop.h>
#include <seiscomp/io/recordfilter/demux.h>
#include <seiscomp/io/recordfilter/iirfilter.h>
#include <seiscomp/io/recordfilter/pipe.h>
#include <seiscomp/io/recordfilter/mseedencoder.h>
#include <seiscomp/io/recordfilter/resample.h>
#include <seiscomp/io/importer.h>
#include <seiscomp/io/exporter.h>
#include <seiscomp/io/gfarchive.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobjectcache.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/datamodel/diff.h>
#include <seiscomp/io/recordstream/file.h>
#include <seiscomp/io/recordstream/slconnection.h>
#include <seiscomp/io/recordstream/arclink.h>
#include <seiscomp/io/recordstream/combined.h>
%}

%feature("director") Seiscomp::DataModel::CachePopCallback;

%include stl.i
%include std_complex.i
%include std_vector.i
%import "io.i"
%import "geo.i"

%newobject *::Create;
%newobject *::find;
%newobject *::Find;

%newobject Seiscomp::DataModel::copy;
%newobject Seiscomp::DataModel::DatabaseIterator::get;
%newobject Seiscomp::DataModel::DatabaseArchive::getObject;
%newobject Seiscomp::DataModel::NotifierMessage::get;
%newobject Seiscomp::DataModel::DiffMerge::diff2Message;
%newobject Seiscomp::DataModel::Diff::diff2Message;
%newobject Seiscomp::DataModel::Notifier::GetMessage;
%newobject Seiscomp::DataModel::Notifier::Create;
%ignore Seiscomp::DataModel::Diff::diff;
%ignore Seiscomp::DataModel::Diff2::diff;
%ignore Seiscomp::DataModel::DatabaseIterator::next;

optional(Seiscomp::DataModel::Operation);
enum(Seiscomp::DataModel::Operation);

namespace std {
   %template(vectord) vector<double>;
   %template(vectorc) vector< std::complex<double> >;
};

%ignore Seiscomp::DataModel::PublicObjectCache::const_iterator;
%ignore Seiscomp::DataModel::PublicObjectCache::begin;
%ignore Seiscomp::DataModel::PublicObjectCache::end;

%template(NotifierMessageBase) Seiscomp::Core::GenericMessage<Seiscomp::DataModel::Notifier>;

%exception {
  try {
    $action
  }
  catch ( const Swig::DirectorException &e ) {
    SWIG_fail;
  }
  catch ( const Seiscomp::Core::ValueException &e ) {
    SWIG_exception(SWIG_ValueError, e.what());
  }
  catch ( const std::exception &e ) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
  catch ( ... ) {
    SWIG_exception(SWIG_UnknownError, "C++ anonymous exception");
  }
}


%include "seiscomp/client.h"
%include "seiscomp/datamodel/object.h"
%include "seiscomp/datamodel/publicobject.h"
%include "seiscomp/datamodel/databasearchive.h"
%include "seiscomp/datamodel/publicobjectcache.h"
%include "seiscomp/datamodel/notifier.h"
%include "seiscomp/datamodel/utils.h"
%include "seiscomp/datamodel/diff.h"


%extend Seiscomp::DataModel::DatabaseIterator {
	void step() {
		++(*self);
	}

	%pythoncode %{
		def __iter__(self):
		    return self

		def __next__(self):
		    o = self.get()
		    if not o:
		        raise StopIteration

		    self.step()
		    return o

		# for Python 2 compatibility
		def next(self):
		    return self.__next__()
	%}
};


%inline %{
	class PublicObjectCacheIterator {
		public:
			PublicObjectCacheIterator(Seiscomp::DataModel::PublicObjectCache *cache)
			: _cache(cache) {
				_it = _cache->begin();
			}

			Seiscomp::DataModel::PublicObject *next() {
				if ( _it == _cache->end() ) {
					return nullptr;
				}

				auto obj = *_it;
				++_it;
				return obj;
			}

		private:
			Seiscomp::DataModel::PublicObjectCache *_cache;
			Seiscomp::DataModel::PublicObjectCache::const_iterator _it;
	};
%}

%extend PublicObjectCacheIterator {
	%pythoncode %{
		def __next__(self):
		    o = self.next()
		    if o is None:
		        raise StopIteration
		    return o
	%}
};

%extend Seiscomp::DataModel::PublicObjectCache {
	%pythoncode %{
		def get(self, klass, publicID):
		    o = self.find(klass.TypeInfo(), publicID)
		    return klass.Cast(o)

		def __iter__(self):
		    return PublicObjectCacheIterator(self)
	%}
};
