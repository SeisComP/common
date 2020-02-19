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

%module(package="seiscomp", directors="1") system

//
// includes
//
%{
#include "seiscomp/system/application.h"
#include "seiscomp/system/commandline.h"
#include "seiscomp/system/environment.h"
#include "seiscomp/system/hostinfo.h"
#include "seiscomp/system/model.h"
#include "seiscomp/system/pluginregistry.h"
#include "seiscomp/system/schema.h"
#include "seiscomp/core/array.h"
#include "seiscomp/core/enumeration.h"
#include "seiscomp/core/typedarray.h"
#include "seiscomp/core/record.h"
#include "seiscomp/core/greensfunction.h"
#include "seiscomp/core/genericrecord.h"
#include "seiscomp/core/interruptible.h"
#include "seiscomp/core/datamessage.h"
%}


%rename(SystemApplication) Seiscomp::System::Application;
%feature("director") Seiscomp::System::Application;

%feature("director:except") {
  if ($error != NULL) {
    throw Swig::DirectorMethodException();
  }
}
%exception {
  try { $action }
  catch (Swig::DirectorException &e) { SWIG_fail; }
}

// 
// typemaps
//
%include "stl.i"
%include "std_vector.i"
%include "std_string.i"
%include "std_map.i"
%include "std_set.i"
%import "core.i"
%import "logging.i"
%import "../../../system/libs/swig/config.i"

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

%rename(execute) Seiscomp::System::Application::exec;

%ignore Seiscomp::System::PluginRegistry::begin;
%ignore Seiscomp::System::PluginRegistry::end;
%ignore Seiscomp::System::Application::handleInterrupt;
%feature("nodirector") Seiscomp::System::Application::exit;

%exception {
  try {
    $action
  }
  catch ( const Swig::DirectorException &e ) {
    SWIG_fail;
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

optional(bool);

%include "seiscomp/core.h"
%include "seiscomp/system/commandline.h"
%include "seiscomp/system/environment.h"
%include "seiscomp/system/pluginregistry.h"
%include "seiscomp/system/application.h"
%include "seiscomp/system/hostinfo.h"
%include "seiscomp/system/schema.h"
%include "seiscomp/system/model.h"

%template(BindingMap) std::map<Seiscomp::System::StationID, Seiscomp::System::ModuleBindingPtr>;
%template(GroupVector) std::vector<Seiscomp::System::GroupPtr>;
%template(ParameterVector) std::vector<Seiscomp::System::ParameterPtr>;
%template(StructureVector) std::vector<Seiscomp::System::StructurePtr>;

%template(optionInt) Seiscomp::System::CommandLine::option<int>;
%template(optionBool) Seiscomp::System::CommandLine::option<bool>;
%template(optionDouble) Seiscomp::System::CommandLine::option<double>;
%template(optionString) Seiscomp::System::CommandLine::option<std::string>;

%extend Seiscomp::System::CommandLine {
	void addIntOption(const char* group, const char* option,
	                  const char* description) {
		self->addOption(group, option, description, (int*)NULL);
	}

	void addIntOption(const char* group, const char* option,
	                  const char* description, int defaultValue) {
		self->addOption(group, option, description, (int*)NULL, defaultValue);
	}

	void addDoubleOption(const char* group, const char* option,
	                     const char* description) {
		self->addOption(group, option, description, (double*)NULL);
	}

	void addDoubleOption(const char* group, const char* option,
	                     const char* description, double defaultValue) {
		self->addOption(group, option, description, (double*)NULL, defaultValue);
	}

	void addBoolOption(const char* group, const char* option,
	                   const char* description) {
		self->addOption(group, option, description, (bool*)NULL);
	}

	void addBoolOption(const char* group, const char* option,
	                   const char* description, bool defaultValue) {
		self->addOption(group, option, description, (bool*)NULL, defaultValue);
	}

	void addStringOption(const char* group, const char* option, const char* description) {
		self->addOption(group, option, description, (std::string*)NULL);
	}

	void addStringOption(const char* group, const char* option,
	                     const char* description, const std::string& defaultValue) {
		self->addOption(group, option, description, (std::string*)NULL, defaultValue);
	}
};
