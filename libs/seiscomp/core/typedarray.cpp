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


#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <seiscomp/core/typedarray.h>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


template<typename T>
Array::DataType dispatchType();

template<>
Array::DataType dispatchType<char>() {
	return Array::CHAR;
}

template<>
Array::DataType dispatchType<std::int32_t>() {
	return Array::INT;
}

template<>
Array::DataType dispatchType<float>() {
	return Array::FLOAT;
}

template<>
Array::DataType dispatchType<double>() {
	return Array::DOUBLE;
}

template<>
Array::DataType dispatchType<Core::Time>() {
	return Array::DATETIME;
}

template<>
Array::DataType dispatchType<std::string>() {
	return Array::STRING;
}

template<>
Array::DataType dispatchType< std::complex<float> >() {
	return Array::COMPLEX_FLOAT;
}

template<>
Array::DataType dispatchType< std::complex<double> >() {
	return Array::COMPLEX_DOUBLE;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
Array::DataType TypedArray<T>::ArrayType = dispatchType<T>();
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>::TypedArray(): Array(dispatchType<T>()) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>::TypedArray(int size): Array(dispatchType<T>()), _data(size) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>::TypedArray(int size, const T* data): Array(dispatchType<T>()) {
	setData(size, data);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>::TypedArray(const TypedArray &array): Array(dispatchType<T>()) {
	_data.assign(array.begin(),array.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>::~TypedArray() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>& TypedArray<T>::operator=(const TypedArray &array) {
	if (this != &array) {
		_data.assign(array.begin(),array.end());
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
Array* TypedArray<T>::copy(DataType dt) const {
	return ArrayFactory::Create(dt, this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::setData(int size, const T* data) {
	_data.assign(data,data+size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
const void* TypedArray<T>::data() const {
	return _data.data();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T* TypedArray<T>::typedData() {
	return _data.data();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
const T* TypedArray<T>::typedData() const {
	return _data.data();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
int TypedArray<T>::elementSize() const {
	return sizeof(T);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::resize(int size) {
	_data.resize(size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::clear() {
	_data.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::fill(const T &v) {
	std::fill(_data.begin(), _data.end(), v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::append(const Array *array) {
	if (array->dataType() == dispatchType<T>())
		_data.insert(_data.end(),static_cast<const TypedArray<T> *>(array)->begin(),static_cast<const TypedArray<T> *>(array)->end());
	else
		std::cerr << "Can not concatenate arrays of different types.\n";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::append(int size, const T* data) {
	_data.insert(_data.end(), data, data + size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::append(int n, T value) {
	_data.insert(_data.end(), n, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::prepend(const Array *array) {
	if (array->dataType() == dispatchType<T>())
		_data.insert(_data.begin(),static_cast<const TypedArray<T> *>(array)->begin(),static_cast<const TypedArray<T> *>(array)->end());
	else
		std::cerr << "Can not concatenate arrays of different types.\n";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::prepend(int size, const T* data) {
	_data.insert(_data.begin(), data, data + size);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::prepend(int n, T value) {
	_data.insert(_data.begin(), n, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>::NumericArray(): TypedArray<T>() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>::NumericArray(int size): TypedArray<T>(size) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>::NumericArray(int size, const T* data): TypedArray<T>(size,data) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>::NumericArray(const NumericArray &array): TypedArray<T>(array) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>::~NumericArray() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>& NumericArray<T>::operator=(const NumericArray &array) {
	TypedArray<T>::operator=(array);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>template<typename T>
template<typename T>
T NumericArray<T>::max() const {
	return *std::max_element(TypedArray<T>::_data.begin(),TypedArray<T>::_data.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T NumericArray<T>::firstMax() const {
	typename TypedArray<T>::DataArray::size_type i = 0;
	T max = TypedArray<T>::_data[i];

	while (++i < TypedArray<T>::_data.size() && TypedArray<T>::_data[i] > max)
		max = TypedArray<T>::_data[i];

	return max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T NumericArray<T>::absMax() const {
	return *std::max_element(
		TypedArray<T>::_data.begin(),
		TypedArray<T>::_data.end(),
		[](const T &x, const T &y) {
			return std::abs(x) < std::abs(y);
		}
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T NumericArray<T>::min() const {
	return *std::min_element(TypedArray<T>::_data.begin(),TypedArray<T>::_data.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T NumericArray<T>::median() const {
	std::vector<T> vec(TypedArray<T>::_data.size());
	std::partial_sort_copy(TypedArray<T>::_data.begin(),TypedArray<T>::_data.end(),vec.begin(),vec.end());

	return vec[vec.size()/2];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T NumericArray<T>::mean() const {
	T sum = std::accumulate(TypedArray<T>::_data.begin(),TypedArray<T>::_data.end(),static_cast<T>(0));

	return static_cast<T>(sum/TypedArray<T>::_data.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
T NumericArray<T>::rms(T offset) const {
	size_t n = TypedArray<T>::_data.size();
	const T *f = TypedArray<T>::typedData();
	T fi, r = 0;

	if ( offset ) {
		for ( size_t i = 0; i < n; ++i, ++f ) {
			fi = ((*f)-offset);
			r += fi*fi;
		}
	}
	else {
		for ( size_t i = 0; i < n; ++i, ++f )
			r += (*f)*(*f);
	}

	return static_cast<T>(n ? sqrt(r/n) : 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T> &NumericArray<T>::operator+=(T v) {
	size_t n = TypedArray<T>::_data.size();
	T *f = TypedArray<T>::typedData();
	while ( n ) {
		*f += v;
		--n; ++f;
	}
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T> &NumericArray<T>::operator-=(T v) {
	size_t n = TypedArray<T>::_data.size();
	T *f = TypedArray<T>::typedData();
	while ( n ) {
		*f -= v;
		--n; ++f;
	}
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T> &NumericArray<T>::operator*=(T v) {
	size_t n = TypedArray<T>::_data.size();
	T *f = TypedArray<T>::typedData();
	while ( n ) {
		*f *= v;
		--n; ++f;
	}
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T> &NumericArray<T>::operator/=(T v) {
	size_t n = TypedArray<T>::_data.size();
	T *f = TypedArray<T>::typedData();
	while ( n ) {
		*f /= v;
		--n; ++f;
	}
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
TypedArray<T>* TypedArray<T>::slice(int m, int n) const {
	if ( m < 0 || n < 0 || m >= n ) return nullptr;
	if ( m >= (int)_data.size() ) return nullptr;
	if ( n > (int)_data.size() ) n = _data.size();
	return new TypedArray<T>(n-m, &(_data[m]));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
NumericArray<T>* NumericArray<T>::slice(int m, int n) const {
	if ( m < 0 || n < 0 || m >= n ) return nullptr;
	if ( m >= (int)TypedArray<T>::_data.size() ) return nullptr;
	if ( n > (int)TypedArray<T>::_data.size() ) n = TypedArray<T>::_data.size();
	return new NumericArray<T>(n-m, &(TypedArray<T>::_data[m]));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void TypedArray<T>::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("data", _data, Archive::XML_CDATA);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_TEMPLATE_RTTI_METHODS(TypedArray, "TypedArray")
IMPLEMENT_TEMPLATE_RTTI_METHODS(NumericArray, "NumericArray")

IMPLEMENT_TEMPLATE_RTTI(CharArray, "CharArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, CharArray);
template class SC_SYSTEM_CORE_API TypedArray<char>;
template class SC_SYSTEM_CORE_API NumericArray<char>;

IMPLEMENT_TEMPLATE_RTTI(IntArray, "IntArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, Int32Array);
template class SC_SYSTEM_CORE_API TypedArray<std::int32_t>;
template class SC_SYSTEM_CORE_API NumericArray<std::int32_t>;

IMPLEMENT_TEMPLATE_RTTI(FloatArray, "FloatArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, FloatArray);
template class SC_SYSTEM_CORE_API TypedArray<float>;
template class SC_SYSTEM_CORE_API NumericArray<float>;

IMPLEMENT_TEMPLATE_RTTI(DoubleArray, "DoubleArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, DoubleArray);
template class SC_SYSTEM_CORE_API TypedArray<double>;
template class SC_SYSTEM_CORE_API NumericArray<double>;

IMPLEMENT_TEMPLATE_RTTI(DateTimeArray, "DateTimeArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, DateTimeArray);
template class SC_SYSTEM_CORE_API TypedArray<Core::Time>;

IMPLEMENT_TEMPLATE_RTTI(StringArray, "StringArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, StringArray);
template class SC_SYSTEM_CORE_API TypedArray<std::string>;

IMPLEMENT_TEMPLATE_RTTI(ComplexFloatArray, "ComplexFloatArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, ComplexFloatArray);
template class SC_SYSTEM_CORE_API TypedArray< std::complex<float> >;

IMPLEMENT_TEMPLATE_RTTI(ComplexDoubleArray, "ComplexDoubleArray", Array)
REGISTER_CLASS(Seiscomp::Core::BaseObject, ComplexDoubleArray);
template class SC_SYSTEM_CORE_API TypedArray< std::complex<double> >;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
