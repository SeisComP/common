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




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
IntrusiveList<T>::IntrusiveList(IntrusiveList &&other) {
	_front = other._front;
	_back = other._back;
	_size = other._size;
	_index = other._index;

	other._front = other._back = nullptr;
	other._size = 0;
	other._index = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
IntrusiveList<T> &IntrusiveList<T>::operator=(IntrusiveList<T> &&other) {
	if ( this != &other ) {
		clear();

		_front = other._front;
		_back = other._back;
		_size = other._size;
		_index = other._index;

		other._front = other._back = nullptr;
		other._size = 0;
		other._index = 0;
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
typename IntrusiveList<T>::iterator IntrusiveList<T>::find(PointerType o) const {
	if ( o->_ili_prev[_index] || o->_ili_next[_index] || o == _front ) {
		return iterator(_index,o);
	}

	return iterator();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void IntrusiveList<T>::push_back(T o) {
	o->_ili_prev[_index] = _back;
	o->_ili_next[_index] = nullptr;

	// Update links
	if ( o->_ili_prev[_index] ) {
		o->_ili_prev[_index]->_ili_next[_index] = IntrusiveTraits<T>::toPointer(o);
	}
	else {
		_front = IntrusiveTraits<T>::toPointer(o);
	}
	_back = IntrusiveTraits<T>::toPointer(o);

	++_size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void IntrusiveList<T>::erase(T o) {
	if ( o->_ili_prev[_index] )
		o->_ili_prev[_index]->_ili_next[_index] = o->_ili_next[_index];
	else
		_front = o->_ili_next[_index];

	if ( o->_ili_next[_index] )
		o->_ili_next[_index]->_ili_prev[_index] = o->_ili_prev[_index];
	else
		_back = o->_ili_prev[_index];

	o->reset(_index, o);

	--_size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
typename IntrusiveList<T>::iterator
IntrusiveList<T>::erase(const iterator &it) {
	if ( !it.item ) {
		return iterator();
	}

	T n = it.item->_ili_next[_index];
	erase(it.item);
	return iterator(_index,n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void IntrusiveList<T>::swap(T o1, T o2) {
	T tmp;
	tmp = o1->_ili_prev[_index];
	o1->_ili_prev[_index] = o2->_ili_prev[_index];
	o2->_ili_prev[_index] = tmp;

	tmp = o1->_ili_next[_index];
	o1->_ili_next[_index] = o2->_ili_next[_index];
	o2->_ili_next[_index] = tmp;

	if ( o1->_ili_prev[_index] == o1 )
		o1->_ili_prev[_index] = o2;
	if ( o2->_ili_prev[_index] == o2 )
		o2->_ili_prev[_index] = o1;

	if ( o1->_ili_next[_index] == o1 )
		o1->_ili_next[_index] = o2;
	if ( o2->_ili_next[_index] == o2 )
		o2->_ili_next[_index] = o1;

	if ( _front == o2 )
		_front = o1;
	else if ( _front == o1 )
		_front = o2;

	if ( _back == o2 )
		_back = o1;
	else if ( _back == o1 )
		_back = o2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void IntrusiveList<T>::replace(PointerType existingItem, PointerType newItem) {
	newItem->_ili_next[_index] = existingItem->_ili_next[_index];
	newItem->_ili_prev[_index] = existingItem->_ili_prev[_index];

	existingItem->reset(_index, existingItem);

	if ( newItem->_ili_prev[_index] ) {
		newItem->_ili_prev[_index]->_ili_next[_index] = newItem;
	}

	if ( newItem->_ili_next[_index] ) {
		newItem->_ili_next[_index]->_ili_prev[_index] = newItem;
	}

	if ( _front == existingItem ) {
		_front = newItem;
	}

	if ( _back == existingItem ) {
		_back = newItem;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void IntrusiveList<T>::clear() {
	T n = _front;

	while ( n ) {
		T curr = n;
		n = n->_ili_next[_index];
		curr->reset(_index, curr);
	}

	_front = _back = nullptr;
	_size = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
