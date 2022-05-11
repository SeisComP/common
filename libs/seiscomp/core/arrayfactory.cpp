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


#include <algorithm>
#include <iostream>

#include <seiscomp/core/arrayfactory.h>
#include <seiscomp/core/typedarray.h>


using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TGT, typename SRC>
struct convert {
	TGT operator()(SRC value) {
		return static_cast<TGT>(value);
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename SRC>
struct convert< std::complex<float>, SRC > {
	std::complex<float> operator()(SRC value) {
		return std::complex<float>(value, 0);
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TGT>
struct convert<TGT, std::complex<float> > {
	TGT operator()(std::complex<float> value) {
		return (TGT)abs(value);
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
struct convert< std::complex<float>, std::complex<float> > {
	std::complex<float> operator()(std::complex<float> value) {
		return value;
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename SRC>
struct convert< std::complex<double>, SRC > {
	std::complex<double> operator()(SRC value) {
		return std::complex<double>(value, 0);
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TGT>
struct convert<TGT, std::complex<double> > {
	TGT operator()(std::complex<double> value) {
		return (TGT)abs(value);
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
struct convert< std::complex<double>, std::complex<double> > {
	std::complex<double> operator()(std::complex<double> value) {
		return value;
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
struct convert< std::complex<float>, std::complex<double> > {
	std::complex<float> operator()(std::complex<double> value) {
		return std::complex<float>(value.real(), value.imag());
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<>
struct convert< std::complex<double>, std::complex<float> > {
	std::complex<double> operator()(std::complex<float> value) {
		return std::complex<double>(value.real(), value.imag());
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename CONTAINER, typename T>
void convertArray(CONTAINER &c, int size, const T *data) {
	std::transform(data,data+size,std::back_inserter(c),convert<typename CONTAINER::value_type,T>());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define CONVERT_ARRAY(TARGET_ARRAY_TYPE, SOURCE_TYPE) \
	ar = new TARGET_ARRAY_TYPE();\
	convertArray(static_cast<TARGET_ARRAY_TYPE*>(ar)->_data, size, static_cast<const SOURCE_TYPE*>(data))
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Array* ArrayFactory::Create(Array::DataType toCreate, Array::DataType caller, int size, const void *data) {
	Array *ar = nullptr;

	switch ( toCreate ) {
		case Array::CHAR:
			switch ( caller ) {
				case Array::CHAR:
					CONVERT_ARRAY(CharArray, char);
					break;
				case Array::INT:
					CONVERT_ARRAY(CharArray, int);
					break;
				case Array::FLOAT:
					CONVERT_ARRAY(CharArray, float);
					break;
				case Array::DOUBLE:
					CONVERT_ARRAY(CharArray, double);
					break;
				case Array::COMPLEX_FLOAT:
					CONVERT_ARRAY(CharArray, std::complex<float>);
					break;
				case Array::COMPLEX_DOUBLE:
					CONVERT_ARRAY(CharArray, std::complex<double>);
					break;
				default:
					break;
			}
			break;
		case Array::INT:
			switch ( caller ) {
				case Array::CHAR:
					CONVERT_ARRAY(IntArray, char);
					break;
				case Array::INT:
					CONVERT_ARRAY(IntArray, int);
					break;
				case Array::FLOAT:
					CONVERT_ARRAY(IntArray, float);
					break;
				case Array::DOUBLE:
					CONVERT_ARRAY(IntArray, double);
					break;
				case Array::COMPLEX_FLOAT:
					CONVERT_ARRAY(IntArray, std::complex<float>);
					break;
				case Array::COMPLEX_DOUBLE:
					CONVERT_ARRAY(IntArray, std::complex<double>);
					break;
				default:
					break;
			}
			break;
		case Array::FLOAT:
			switch ( caller ) {
				case Array::CHAR:
					CONVERT_ARRAY(FloatArray, char);
					break;
				case Array::INT:
					CONVERT_ARRAY(FloatArray, int);
					break;
				case Array::FLOAT:
					CONVERT_ARRAY(FloatArray, float);
					break;
				case Array::DOUBLE:
					CONVERT_ARRAY(FloatArray, double);
					break;
				case Array::COMPLEX_FLOAT:
					CONVERT_ARRAY(FloatArray, std::complex<float>);
					break;
				case Array::COMPLEX_DOUBLE:
					CONVERT_ARRAY(FloatArray, std::complex<double>);
					break;
				default:
					break;
			}
			break;
		case Array::DOUBLE:
			switch ( caller ) {
				case Array::CHAR:
					CONVERT_ARRAY(DoubleArray, char);
					break;
				case Array::INT:
					CONVERT_ARRAY(DoubleArray, int);
					break;
				case Array::FLOAT:
					CONVERT_ARRAY(DoubleArray, float);
					break;
				case Array::DOUBLE:
					CONVERT_ARRAY(DoubleArray, double);
					break;
				case Array::COMPLEX_FLOAT:
					CONVERT_ARRAY(DoubleArray, std::complex<float>);
					break;
				case Array::COMPLEX_DOUBLE:
					CONVERT_ARRAY(DoubleArray, std::complex<double>);
					break;
				default:
					break;
			}
			break;
		case Array::COMPLEX_FLOAT:
			switch ( caller ) {
				case Array::CHAR:
					CONVERT_ARRAY(ComplexFloatArray, char);
					break;
				case Array::INT:
					CONVERT_ARRAY(ComplexFloatArray, int);
					break;
				case Array::FLOAT:
					CONVERT_ARRAY(ComplexFloatArray, float);
					break;
				case Array::DOUBLE:
					CONVERT_ARRAY(ComplexFloatArray, double);
					break;
				case Array::COMPLEX_FLOAT:
					CONVERT_ARRAY(ComplexFloatArray, std::complex<float>);
					break;
				case Array::COMPLEX_DOUBLE:
					CONVERT_ARRAY(ComplexFloatArray, std::complex<double>);
					break;
				default:
					break;
			}
			break;
		case Array::COMPLEX_DOUBLE:
			switch ( caller ) {
				case Array::CHAR:
					CONVERT_ARRAY(ComplexDoubleArray, char);
					break;
				case Array::INT:
					CONVERT_ARRAY(ComplexDoubleArray, int);
					break;
				case Array::FLOAT:
					CONVERT_ARRAY(ComplexDoubleArray, float);
					break;
				case Array::DOUBLE:
					CONVERT_ARRAY(ComplexDoubleArray, double);
					break;
				case Array::COMPLEX_FLOAT:
					CONVERT_ARRAY(ComplexDoubleArray, std::complex<float>);
					break;
				case Array::COMPLEX_DOUBLE:
					CONVERT_ARRAY(ComplexDoubleArray, std::complex<double>);
					break;
				default:
					break;
			}
			break;
		default:
			ar = 0;
	}

	return ar;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Array* ArrayFactory::Create(Array::DataType toCreate, const Array *source) {
	return Create(
		toCreate,
		source->dataType(),
		source->size(),
		const_cast<void *>(source->data())
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
