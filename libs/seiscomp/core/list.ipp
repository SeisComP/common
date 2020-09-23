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

template <typename T>
void IntrusiveList<T>::push_back(T o) {
	o->_ili_prev[_index] = _back;
	o->_ili_next[_index] = nullptr;

	// Update links
	if ( o->_ili_prev[_index] )
		o->_ili_prev[_index]->_ili_next[_index] = o;
	else
		_front = o;
	_back = o;

	// Register self in smart pointer to keep a reference
	o->store(_index, o);

	++_size;

	//std::cerr << "add(" << o << ") to " << this << ": " << _front << "/" << _back << "/" << _size << std::endl;
}


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

	//std::cerr << "erase(" << o << ") from " << this << ": " << _front << "/" << _back << "/" << _size << std::endl;
}


template <typename T>
typename IntrusiveList<T>::iterator
IntrusiveList<T>::erase(const iterator &it) {
	if ( it.item == nullptr ) return iterator();

	T n = it.item->_ili_next[_index];
	erase(it.item);
	return iterator(_index,n);
}


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
