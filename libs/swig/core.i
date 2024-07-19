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

%module(package="seiscomp") core

%exceptionclass GeneralException;

%{
#include <exception>
#include "seiscomp/core/baseobject.h"
#include "seiscomp/core/exceptions.h"
#include "seiscomp/core/array.h"
#include "seiscomp/core/bitset.h"
#include "seiscomp/core/datetime.h"
#include "seiscomp/core/enumeration.h"
#include "seiscomp/core/typedarray.h"
#include "seiscomp/core/record.h"
#include "seiscomp/core/greensfunction.h"
#include "seiscomp/core/genericrecord.h"
#include "seiscomp/core/interruptible.h"
#include "seiscomp/core/datamessage.h"
#include "seiscomp/core/version.h"
#ifdef HAVE_NUMPY
#include <numpy/ndarrayobject.h>
#endif
%}

%init
%{
#ifdef HAVE_NUMPY
import_array();
#endif
%}

%pythonbegin %{
import datetime
%}

%newobject *::Cast;
%newobject *::copy;
%newobject *::clone;
%newobject Seiscomp::Core::DataMessage::get;
%ignore Seiscomp::Core::InterruptibleObject;
%ignore Seiscomp::Core::Smartpointer<Seiscomp::Core::BaseObject>::Impl;
%ignore Seiscomp::Core::GreensFunction::setData(int,Seiscomp::Array*);
%ignore Seiscomp::Core::GreensFunction::data(int) const;

%include exception.i
%include std_except.i
%include "base.i"

enum(Seiscomp::Core::GreensFunctionComponent);

/* There is a bytes() method in Array we don't want to wrap.
 * Using the same name bytes() to do something different is not
 * quite optimal and thus may change.
%ignore Seiscomp::Array::bytes() const;
%ignore Seiscomp::Array::str() const;
 */

%include "seiscomp/core/array.h"
%include "seiscomp/core/bitset.h"
%include "seiscomp/core/typedarray.h"
%include "seiscomp/core/record.h"
%include "seiscomp/core/greensfunction.h"
%include "seiscomp/core/genericrecord.h"
%include "seiscomp/core/message.h"
%import "seiscomp/core/genericmessage.h"

%template(DataMessageBase) Seiscomp::Core::GenericMessage<Seiscomp::Core::BaseObject>;

%include "seiscomp/core/datamessage.h"

%extend Seiscomp::Array {
#if defined(SWIGPYTHON)
	PyObject* numpy() {
%#ifdef HAVE_NUMPY
		npy_intp n = (npy_intp) self->size();
		int type = NPY_CHAR;
		switch ( self->dataType() ) {
			case Seiscomp::Array::CHAR:
				type = NPY_INT8;
				break;
			case Seiscomp::Array::INT:
				type = NPY_INT32;
				break;
			case Seiscomp::Array::FLOAT:
				type = NPY_FLOAT32;
				break;
			case Seiscomp::Array::DOUBLE:
				type = NPY_FLOAT64;
				break;
			case Seiscomp::Array::COMPLEX_FLOAT:
				type = NPY_CFLOAT;
				break;
			case Seiscomp::Array::COMPLEX_DOUBLE:
				type = NPY_CDOUBLE;
				break;
			default:
				SWIG_exception(SWIG_TypeError, "unsupported array type");
				goto fail;
		}
		return PyArray_SimpleNewFromData(1, &n, type, (char*)self->data());
%#else
		SWIG_exception(SWIG_SystemError, "missing support for NumPy");
%#endif
		fail:
			return NULL;
	}

	PyObject* setNumpy(PyObject *obj) {
%#ifdef HAVE_NUMPY
		PyArrayObject *arr;
		switch ( self->dataType() ) {
			case Seiscomp::Array::CHAR:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, NPY_INT8, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D char array");
				static_cast<Seiscomp::CharArray*>(self)->setData(arr->dimensions[0],(char *)(arr->data));
				break;
			case Seiscomp::Array::INT:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, NPY_INT32, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D int array");
				static_cast<Seiscomp::IntArray*>(self)->setData(arr->dimensions[0],(int *)(arr->data));
				break;
			case Seiscomp::Array::FLOAT:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, NPY_FLOAT32, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D float array");
				static_cast<Seiscomp::FloatArray*>(self)->setData(arr->dimensions[0],(float *)(arr->data));
				break;
			case Seiscomp::Array::DOUBLE:
				arr = (PyArrayObject*) PyArray_ContiguousFromObject(obj, NPY_FLOAT64, 1, 1);
				if ( arr == NULL )
					return PyErr_Format(PyExc_TypeError,
						"Unable to convert object to 1-D double array");
				static_cast<Seiscomp::DoubleArray*>(self)->setData(arr->dimensions[0],(double *)(arr->data));
				break;
			default:
				SWIG_exception(SWIG_TypeError, "unsupported array type");
				goto fail;
		}

		Py_XDECREF(arr);
		Py_RETURN_NONE;
%#else
		SWIG_exception(SWIG_SystemError, "missing support for NumPy");
%#endif
		fail:
			Py_RETURN_NONE;
	}

	PyObject* bytes() {
		PyObject *b;
		switch ( self->dataType() ) {
			case Seiscomp::Array::CHAR:
%#if PY_MAJOR_VERSION == 2
				b = PyString_FromStringAndSize((char*)self->data(), self->size());
				//Py_INCREF(b);
				return b;
%#elif PY_MAJOR_VERSION >= 3
				b = PyBytes_FromStringAndSize((char*)self->data(), self->size());
				//Py_INCREF(b);
				return b;
%#else
				SWIG_exception(SWIG_TypeError, "unsupported Python version");
%#endif
			default:
				SWIG_exception(SWIG_TypeError, "unsupported array type");
				goto fail;
		}
		fail:
			Py_RETURN_NONE;
	}
	%pythoncode %{
		def str(self):
			""" For backwards compatibility """
			return self.bytes()

		def __str__(self):
			""" For backwards compatibility """
			return self.bytes()

		def __bytes__(self):
			return self.bytes()

		def numeric(self):
			import sys
			sys.stderr.write("Use of Array.numeric() is deprecated - use numpy() instead\n")
			return self.numpy()
	%}
#endif
};

%template(CharArrayT) Seiscomp::TypedArray<char>;
%template(IntArrayT) Seiscomp::TypedArray<int>;
%template(FloatArrayT) Seiscomp::TypedArray<float>;
%template(DoubleArrayT) Seiscomp::TypedArray<double>;
%template(ComplexFloatArray) Seiscomp::TypedArray< std::complex<float> >;
%template(ComplexDoubleArray) Seiscomp::TypedArray< std::complex<double> >;
%template(DateTimeArray) Seiscomp::TypedArray<Seiscomp::Core::Time>;
%template(StringArray) Seiscomp::TypedArray<std::string>;

%template(CharArray) Seiscomp::NumericArray<char>;
%template(IntArray) Seiscomp::NumericArray<int>;
%template(FloatArray) Seiscomp::NumericArray<float>;
%template(DoubleArray) Seiscomp::NumericArray<double>;


%extend Seiscomp::Core::Time {
        %pythoncode %{
                def __str__(self):
                        return self.toString("%Y-%m-%d %H:%M:%S.%f000000")[:23]

                def datetime(self):
                        return datetime.datetime(*self.get()[1:])
        %}
};

%extend Seiscomp::Core::TimeSpan {
        %pythoncode %{
                def __float__(self):
                        return self.length()
        %}
};


%extend Seiscomp::Core::Message {
	%pythoncode %{
		def __iter__(self):
			return self.iter()
	%}
};

%extend Seiscomp::Core::MessageIterator {
	void step() {
		++(*self);
	}

	%pythoncode %{
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
