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

%module(package="seiscomp", directors="1") client
%{
#include "seiscomp/core/typedarray.h"
#include "seiscomp/core/genericrecord.h"
#include "seiscomp/core/exceptions.h"
#include "seiscomp/core/datamessage.h"
#include "seiscomp/core/greensfunction.h"
#include "seiscomp/geo/feature.h"
#include "seiscomp/geo/featureset.h"
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
#include "seiscomp/messaging/packet.h"
#include "seiscomp/messaging/protocol.h"
#include "seiscomp/messaging/connection.h"
#include "seiscomp/messaging/messages/service.h"
#include "seiscomp/messaging/messages/database.h"
#include "seiscomp/client/application.h"
#include "seiscomp/client/streamapplication.h"
#include "seiscomp/client/inventory.h"
#include "seiscomp/client/configdb.h"
#include "seiscomp/datamodel/publicobjectcache.h"
#include "seiscomp/datamodel/messages.h"
#include "seiscomp/io/recordstream/file.h"
#include "seiscomp/io/recordstream/slconnection.h"
#include "seiscomp/io/recordstream/arclink.h"
#include "seiscomp/io/recordstream/combined.h"

#include "seiscomp/system/commandline.h"
#include "seiscomp/system/environment.h"
#include "seiscomp/system/pluginregistry.h"
#include "seiscomp/system/schema.h"
#include "seiscomp/system/model.h"

#include "seiscomp/math/coord.h"
#include "seiscomp/math/geo.h"
#include "seiscomp/math/filter.h"
#include "seiscomp/math/filter/average.h"
#include "seiscomp/math/filter/stalta.h"
#include "seiscomp/math/filter/chainfilter.h"
#include "seiscomp/math/filter/biquad.h"
#include "seiscomp/math/filter/butterworth.h"
#include "seiscomp/math/filter/taper.h"
#include "seiscomp/math/filter/rmhp.h"
#include "seiscomp/math/restitution/transferfunction.h"

%}

%feature("director") Seiscomp::Client::Application;
%feature("director") Seiscomp::Client::StreamApplication;

%feature("director:except") {
  if ($error != NULL) {
    throw Swig::DirectorMethodException();
  }
}
%exception {
  try { $action }
  catch (Swig::DirectorException &e) { SWIG_fail; }
}

%feature("pythonprepend") Seiscomp::Client::Application::Application(int argc, char** argv) %{
    argv = [ bytes(a.encode()) for a in argv ]
%}

%feature("pythonprepend") Seiscomp::Client::StreamApplication::StreamApplication(int argc, char** argv) %{
    argv = [ bytes(a.encode()) for a in argv ]
%}

%feature("pythonprepend") Seiscomp::Client::CommandLine::parse(int argc, char** argv) %{
    argv = [ bytes(a.encode()) for a in argv ]
%}

%include "stl.i"
%include "std_vector.i"
%include "std_string.i"
%include "std_set.i"
%import "system.i"
%import "datamodel.i"
%import "math.i"
%import "utils.i"

// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
	/* Check if is a list */
	if (PyList_Check($input)) {
		int size = PyList_Size($input);
		int i;
		$1 = (char **) malloc((size+1)*sizeof(char *));
		for (i = 0; i < size; i++) {
			PyObject *o = PyList_GetItem($input,i);
			if (PyString_Check(o))
				$1[i] = PyString_AsString(PyList_GetItem($input,i));
			else {
				PyErr_SetString(PyExc_TypeError,"list must contain strings");
				free($1);
				return NULL;
			}
		}
		$1[i] = 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError,"not a list");
		return NULL;
	}
}

// This cleans up the char ** array we did malloc before the function call
%typemap(freearg) char ** {
	free($1);
}

%ignore Seiscomp::Client::Application::handleMonitorLog;
%feature("nodirector") Seiscomp::Client::Application::exit;
%ignore Seiscomp::Client::StreamApplication::storeRecord;
%ignore Seiscomp::Client::StreamApplication::acquisitionFinished;

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

// Allow GIL release for the following function(s)
%exception Seiscomp::Client::Connection::fetchInbox {
  Py_BEGIN_ALLOW_THREADS
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
  Py_END_ALLOW_THREADS
}

%exception Seiscomp::Client::Connection::recv {
  Py_BEGIN_ALLOW_THREADS
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
  Py_END_ALLOW_THREADS
}



enum(Seiscomp::Client::Result);
%newobject Seiscomp::Client::Protocol::decode;
%ignore Seiscomp::Client::Protocol::recv(Packet &);
%ignore Seiscomp::Client::Connection::recv(PacketPtr &, Result *);
%ignore Seiscomp::Client::Connection::recv(PacketPtr &);
%newobject Seiscomp::Client::Protocol::recv;
%newobject Seiscomp::Client::Connection::recv;
%newobject Seiscomp::Client::Protocol::Create;


%typemap(in, numinputs=0) Seiscomp::Client::Packet ** (Seiscomp::Client::Packet *tempPacket) {
  $1 = &tempPacket;
}

%typemap(in, numinputs=0) Seiscomp::Client::Result * (Seiscomp::Client::Result tempResult) {
  $1 = &tempResult;
}

%typemap(argout) Seiscomp::Client::Packet ** {
  PyObject* temp = NULL;
  if (!PyList_Check($result)) {
    temp = $result;
    $result = PyList_New(1);
    PyList_SetItem($result, 0, temp);
  }

  // Create shadow object (do not use SWIG_POINTER_NEW)
  temp = SWIG_NewPointerObj(SWIG_as_voidptr(*$1),
           $descriptor(Seiscomp::Client::Packet*),
           SWIG_POINTER_OWN | 0);

  PyList_Append($result, temp);
  Py_DECREF(temp);
}

%typemap(argout) Seiscomp::Client::Result * {
  PyObject* temp = NULL;
  if (!PyList_Check($result)) {
    temp = $result;
    $result = PyList_New(1);
    PyList_SetItem($result, 0, temp);
  }

  temp = PyInt_FromLong(static_cast<long>($1->toInt()));

  PyList_Append($result, temp);
  Py_DECREF(temp);
}


%include "seiscomp/client.h"
%include "seiscomp/messaging/packet.h"
%include "seiscomp/messaging/protocol.h"
%include "seiscomp/messaging/connection.h"
%include "seiscomp/messaging/messages/service.h"
%include "seiscomp/messaging/messages/database.h"
%include "seiscomp/client/monitor.h"
%include "seiscomp/client/application.h"
%include "seiscomp/client/streamapplication.h"
%include "seiscomp/client/inventory.h"
%include "seiscomp/client/configdb.h"
