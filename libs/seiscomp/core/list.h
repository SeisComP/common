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


#ifndef SEISCOMP_CORE_LIST_H
#define SEISCOMP_CORE_LIST_H


#include <boost/intrusive_ptr.hpp>


namespace Seiscomp {
namespace Core {
namespace Generic {


template <typename T, int N=1>
struct IntrusiveListItem {};

// An item for the intrusive list constructed from plain pointers which
// are not managed by the list.
template <typename T, int N>
struct IntrusiveListItem<T*,N> {
	typedef T *Pointer;
	typedef T *Element;

	IntrusiveListItem<Element,N>() {
		for ( int i = 0; i < N; ++i )
			_ili_prev[i] = _ili_next[i] = nullptr;
	}

	// The second parameter is just a helper to derive the correct
	// function.
	void reset(int i, const Element &) { _ili_prev[i] = _ili_next[i] = nullptr; }
	void store(int i, Element &o) {}

	Element _ili_prev[N];
	Element _ili_next[N];
};


// An item for the instrusive list constructed from smart pointers. All
// items added to the list are managed as such as their reference counter
// is increased by one. If an item is erased from the list the reference
// counter is decreased by one.
template <typename T, int N>
struct IntrusiveListItem<boost::intrusive_ptr<T>,N> {
	typedef T *Pointer;
	typedef boost::intrusive_ptr<T> Element;

	IntrusiveListItem<Element,N>() {
		for ( int i = 0; i < N; ++i )
			_ili_prev[i] = _ili_next[i] = nullptr;
	}

	// The second parameter is just a helper to derive the correct
	// function.
	void reset(int i, const Element &) { _ili_self[i] = _ili_prev[i] = _ili_next[i] = nullptr; }
	void store(int i, Element &o) { _ili_self[i] = o; }

	Element _ili_self[N];
	Element _ili_prev[N];
	Element _ili_next[N];
};


template <typename T>
class IntrusiveListBase {};

template <typename T>
struct IntrusiveListBase<T*> { typedef T *PointerType; };

template <typename T>
struct IntrusiveListBase<boost::intrusive_ptr<T> > { typedef T *PointerType; };

template <typename T>
class IntrusiveList : IntrusiveListBase<T> {
	typedef typename IntrusiveListBase<T>::PointerType PointerType;

	public:
		struct iterator {
			iterator(int slot_idx = 0, T o = nullptr) : idx(slot_idx), item(o) {}

			int idx;
			T item;

			T operator*() const {
				return item;
			}

			bool operator==(const iterator &other) const {
				return item == other.item;
			}

			bool operator!=(const iterator &other) const {
				return item != other.item;
			}

			iterator& operator++() {
				item = item->_ili_next[idx];
				return *this;
			}
		};

		struct const_iterator {
			const_iterator(int slot_idx = 0, T o = nullptr) : idx(slot_idx), item(o) {}

			int idx;
			T item;

			const T operator*() const {
				return item;
			}

			bool operator==(const const_iterator &other) const {
				return item == other.item;
			}

			bool operator!=(const const_iterator &other) const {
				return item != other.item;
			}

			const_iterator& operator++() {
				item = item->_ili_next[idx];
				return *this;
			}
		};

		explicit IntrusiveList(int slotidx = 0) : _front(nullptr), _back(nullptr), _index(slotidx), _size(0) {}
		virtual ~IntrusiveList() { clear(); }

		void setSlot(int slotidx) { _index = slotidx; }

		T front() const { return _front; }
		T back() const { return _back; }

		iterator begin() { return iterator(_index, _front); }
		iterator end() { return iterator(_index); }

		const_iterator begin() const { return const_iterator(_index, _front); }
		const_iterator end() const { return const_iterator(_index); }

		iterator find(PointerType o) const {
			if ( o->_ili_prev[_index] != nullptr || o->_ili_next[_index] != nullptr ||
			     o == _front )
				return iterator(_index,o);

			return iterator();
		}

		void push_back(T o);

		void pop_front() { erase(_front); }

		void erase(T o);
		iterator erase(const iterator &it);

		void swap(T o1, T o2);

		void clear() {
			T n = _front;

			while ( n ) {
				T curr = n;
				n = n->_ili_next[_index];
				curr->reset(_index, curr);
			}

			_front = _back = nullptr;
			_size = 0;
		}

		size_t size() const {
			return _size;
		}

		bool empty() const {
			return _size == 0;
		}

	private:
		T      _front;
		T      _back;
		int    _index;
		size_t _size;
};


#include "list.ipp"


}
}
}


#endif
