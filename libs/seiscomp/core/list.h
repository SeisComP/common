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


#ifndef SEISCOMP_CORE_INTRUSIVE_LIST_H
#define SEISCOMP_CORE_INTRUSIVE_LIST_H


#include <boost/intrusive_ptr.hpp>


namespace Seiscomp {
namespace Core {
namespace Generic {


template <typename T>
struct IntrusiveTraits {};

//! An item for the intrusive list constructed from plain pointers which
//! are not managed by the list.
template <typename T>
struct IntrusiveTraits< T* > {
	typedef T *Pointer;
	typedef T *Element;

	static Pointer toPointer(const Element &o) {
		return o;
	}
};

//! An item for the instrusive list constructed from smart pointers. All
//! items added to the list are managed as such as their reference counter
//! is increased by one. If an item is erased from the list the reference
//! counter is decreased by one.
template <typename T>
struct IntrusiveTraits< boost::intrusive_ptr<T> > {
	typedef T *Pointer;
	typedef boost::intrusive_ptr<T> Element;

	static Pointer toPointer(const Element &o) {
		return o.get();
	}
};


template <typename T, int N=1>
struct IntrusiveListItem {
	typedef typename IntrusiveTraits<T>::Pointer Pointer;
	typedef typename IntrusiveTraits<T>::Element Element;

	IntrusiveListItem() {
		for ( int i = 0; i < N; ++i )
			_ili_next[i] = _ili_prev[i] = nullptr;
	}

	// The second parameter is just a helper to derive the correct
	// function.
	void reset(int i, const Element &) { _ili_next[i] = _ili_prev[i] = nullptr; }

	Pointer _ili_prev[N];
	Element _ili_next[N];
};


template <typename T>
class IntrusiveList {
	// ----------------------------------------------------------------------
	//  Public type definitions
	// ----------------------------------------------------------------------
	public:
		typedef typename IntrusiveTraits<T>::Pointer PointerType;
		typedef typename IntrusiveTraits<T>::Element ElementType;


	// ----------------------------------------------------------------------
	//  Iterators
	// ----------------------------------------------------------------------
	public:
		struct iterator {
			iterator(int slot_idx = 0, PointerType o = nullptr)
			: idx(slot_idx), item(o) {}

			int idx;
			PointerType item;

			PointerType operator*() const {
				return item;
			}

			bool operator==(const iterator &other) const {
				return item == other.item;
			}

			bool operator!=(const iterator &other) const {
				return item != other.item;
			}

			iterator &operator++() {
				item = IntrusiveTraits<T>::toPointer(item->_ili_next[idx]);
				return *this;
			}
		};

		struct const_iterator {
			const_iterator(int slot_idx = 0, PointerType o = nullptr)
			: idx(slot_idx), item(o) {}

			int idx;
			PointerType item;

			const PointerType operator*() const {
				return item;
			}

			bool operator==(const const_iterator &other) const {
				return item == other.item;
			}

			bool operator!=(const const_iterator &other) const {
				return item != other.item;
			}

			const_iterator &operator++() {
				item = IntrusiveTraits<T>::toPointer(item->_ili_next[idx]);
				return *this;
			}
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		explicit IntrusiveList(int slotidx = 0) : _index(slotidx) {}
		IntrusiveList(IntrusiveList &&);
		~IntrusiveList() { clear(); }


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		//! Move assignment operator
		IntrusiveList<T> &operator=(IntrusiveList<T> &&other);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		PointerType front() const { return IntrusiveTraits<T>::toPointer(_front); }
		PointerType back() const { return _back; }

		iterator begin() { return iterator(_index, IntrusiveTraits<T>::toPointer(_front)); }
		iterator end() { return iterator(_index); }

		const_iterator begin() const { return const_iterator(_index, IntrusiveTraits<T>::toPointer(_front)); }
		const_iterator end() const { return const_iterator(_index); }

		iterator find(PointerType o) const;

		void push_back(T o);
		void pop_front() { erase(_front); }

		void erase(T o);
		iterator erase(const iterator &it);

		void clear();

		void swap(T o1, T o2);
		void replace(PointerType existingItem, PointerType newItem);

		size_t size() const {
			return _size;
		}

		bool empty() const {
			return _size == 0;
		}


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		ElementType _front{nullptr};
		PointerType _back{nullptr};
		size_t      _size{0};
		int         _index;
};


#include "list.ipp"


}
}
}


#endif
