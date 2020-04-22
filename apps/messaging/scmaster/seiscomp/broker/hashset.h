/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#ifndef GEMPA_BROKER_HASHSET_H
#define GEMPA_BROKER_HASHSET_H


#include <stdint.h>
#include <utility>
#include <string>

#include <seiscomp/broker/api.h>
#include <seiscomp/broker/utils/khash.h>


namespace Seiscomp {


KHASH_SET_INIT_INT(int)
KHASH_SET_INIT_INT64(int64)
KHASH_SET_INIT_STR(str)
KHASH_MAP_INIT_STR(m_str, void*)


template <typename T>
class KHashSet {};

template <typename K, typename V>
class KHashMap {};


template <>
class KHashSet<uint32_t> {
	public:
		struct iterator {
			iterator() {}
			iterator(const iterator &other) : h(other.h), k(other.k) {}
			iterator(khash_t(int) *h_, unsigned k_) : h(h_), k(k_) {}

			bool operator==(const iterator &other) const {
				return k == other.k;
			}

			bool operator!=(const iterator &other) const {
				return k != other.k;
			}

			// Prefix
			iterator &operator++() {
				++k;
				while ( k != kh_end(h) && !kh_exist(h, k) )
					++k;

				return *this;
			}

			// Postfix
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}

			uint32_t operator*() const {
				return kh_key(h, k);
			}

			khash_t(int) *h;
			unsigned k;
		};


		KHashSet<uint32_t>() {
			_h = kh_init(int);
		}

		~KHashSet() {
			kh_destroy(int, _h);
		}


	public:
		iterator begin() const {
			unsigned k = kh_begin(_h);
			while ( k != kh_end(_h) && !kh_exist(_h, k) )
				++k;
			return iterator(_h, k);
		}

		iterator end() const {
			return iterator(_h, kh_end(_h));
		}

		size_t size() const {
			return kh_size(_h);
		}

		void clear() {
			kh_clear(int, _h);
		}

		int insert(uint32_t v) {
			int ret;
			kh_put(int, _h, v, &ret);
			return ret;
		}

		iterator find(uint32_t v) const {
			return iterator(_h, kh_get(int, _h, v));
		}

		void erase(iterator it) {
			kh_del(int, _h, it.k);
		}

		bool contains(uint32_t v) const {
			return find(v) != end();
		}


	private:
		khash_t(int) *_h;
};


template <>
class KHashSet<uint64_t> {
	public:
		struct iterator {
			iterator() {}
			iterator(const iterator &other) : h(other.h), k(other.k) {}
			iterator(khash_t(int64) *h_, unsigned k_) : h(h_), k(k_) {}

			bool operator==(const iterator &other) const {
				return k == other.k;
			}

			bool operator!=(const iterator &other) const {
				return k != other.k;
			}

			// Prefix
			iterator &operator++() {
				++k;
				while ( k != kh_end(h) && !kh_exist(h, k) )
					++k;

				return *this;
			}

			// Postfix
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}

			uint64_t operator*() const {
				return kh_key(h, k);
			}

			khash_t(int64) *h;
			unsigned k;
		};


		KHashSet<uint64_t>() {
			_h = kh_init(int64);
		}

		~KHashSet() {
			kh_destroy(int64, _h);
		}


	public:
		iterator begin() const {
			unsigned k = kh_begin(_h);
			while ( k != kh_end(_h) && !kh_exist(_h, k) )
				++k;
			return iterator(_h, k);
		}

		iterator end() const {
			return iterator(_h, kh_end(_h));
		}

		size_t size() const {
			return kh_size(_h);
		}

		void clear() {
			kh_clear(int64, _h);
		}

		int insert(uint64_t v) {
			int ret;
			kh_put(int64, _h, v, &ret);
			return ret;
		}

		iterator find(uint64_t v) const {
			return iterator(_h, kh_get(int64, _h, v));
		}

		void erase(iterator it) {
			kh_del(int64, _h, it.k);
		}

		bool contains(uint64_t v) const {
			return find(v) != end();
		}


	private:
		khash_t(int64) *_h;
};


template <>
class KHashSet<const char*> {
	public:
		struct iterator {
			iterator() {}
			iterator(const iterator &other) : h(other.h), k(other.k) {}
			iterator(khash_t(str) *h_, unsigned k_) : h(h_), k(k_) {}

			bool operator==(const iterator &other) const {
				return k == other.k;
			}

			bool operator!=(const iterator &other) const {
				return k != other.k;
			}

			// Prefix
			iterator &operator++() {
				++k;
				while ( k != kh_end(h) && !kh_exist(h, k) )
					++k;

				return *this;
			}

			// Postfix
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}

			const char *operator*() const {
				return kh_key(h, k);
			}

			khash_t(str) *h;
			unsigned k;
		};


		KHashSet<const char*>() {
			_h = kh_init(str);
		}

		~KHashSet() {
			kh_destroy(str, _h);
		}


	public:
		iterator begin() const {
			unsigned k = kh_begin(_h);
			while ( k != kh_end(_h) && !kh_exist(_h, k) )
				++k;
			return iterator(_h, k);
		}

		iterator end() const {
			return iterator(_h, kh_end(_h));
		}

		size_t size() const {
			return kh_size(_h);
		}

		void clear() {
			kh_clear(str, _h);
		}

		int insert(const char *v) {
			int ret;
			kh_put(str, _h, v, &ret);
			return ret;
		}

		iterator find(const char *str) const {
			return iterator(_h, kh_get(str, _h, str));
		}

		iterator find(const std::string &str) const {
			return iterator(_h, kh_get(str, _h, str.c_str()));
		}

		void erase(iterator it) {
			kh_del(str, _h, it.k);
		}

		bool contains(const char *str) const {
			return find(str) != end();
		}

		bool contains(const std::string &str) const {
			return find(str) != end();
		}


	private:
		khash_t(str) *_h;
};


template <typename V>
class KHashMap<const char*, V*> {
	public:
		struct iterator {
			iterator() {}
			iterator(const iterator &other) : h(other.h), k(other.k) {}
			iterator(khash_t(m_str) *h_, unsigned k_) : h(h_), k(k_) {}

			bool operator==(const iterator &other) const {
				return k == other.k;
			}

			bool operator!=(const iterator &other) const {
				return k != other.k;
			}

			// Prefix
			iterator &operator++() {
				++k;
				while ( k != kh_end(h) && !kh_exist(h, k) )
					++k;

				return *this;
			}

			// Postfix
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}

			const char *operator*() const {
				return kh_key(h, k);
			}

			const char *key() const {
				return kh_key(h, k);
			}

			V *value() const {
				return (V*)kh_value(h, k);
			}

			khash_t(m_str) *h;
			unsigned k;
		};


		KHashMap<const char*, V*>() {
			_h = kh_init(m_str);
		}

		~KHashMap() {
			kh_destroy(m_str, _h);
		}


	public:
		iterator begin() const {
			unsigned k = kh_begin(_h);
			while ( k != kh_end(_h) && !kh_exist(_h, k) )
				++k;
			return iterator(_h, k);
		}

		iterator end() const {
			return iterator(_h, kh_end(_h));
		}

		size_t size() const {
			return kh_size(_h);
		}

		void clear() {
			kh_clear(m_str, _h);
		}

		int insert(const char *v, V *value) {
			int ret;
			khiter_t k = kh_put(m_str, _h, v, &ret);
			if ( k >= 0 )
				kh_value(_h, k) = value;
			return ret;
		}

		iterator find(const char *str) const {
			return iterator(_h, kh_get(m_str, _h, str));
		}

		iterator find(const std::string &str) const {
			return iterator(_h, kh_get(m_str, _h, str.c_str()));
		}

		void erase(iterator it) {
			kh_del(m_str, _h, it.k);
		}

		bool contains(const char *str) const {
			return find(str) != end();
		}

		bool contains(const std::string &str) const {
			return find(str) != end();
		}


	private:
		khash_t(m_str) *_h;
};



template <typename T, int BYTES>
class KHashSetPtrBase {};


template <typename T>
class KHashSetPtrBase<T, 4> {
	public:
		struct iterator {
			iterator() {}
			iterator(const iterator &other) : h(other.h), k(other.k) {}
			iterator(khash_t(int) *h_, unsigned k_) : h(h_), k(k_) {}

			bool operator==(const iterator &other) const {
				return k == other.k;
			}

			bool operator!=(const iterator &other) const {
				return k != other.k;
			}

			// Prefix
			iterator &operator++() {
				++k;
				while ( k != kh_end(h) && !kh_exist(h, k) )
					++k;

				return *this;
			}

			// Postfix
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}

			T operator*() const {
				return (T)kh_key(h, k);
			}

			khash_t(int) *h;
			unsigned k;
		};


	public:
		KHashSetPtrBase<T, 4>() {
			_h = kh_init(int);
		}

		~KHashSetPtrBase<T, 4>() {
			kh_destroy(int, _h);
		}


	public:
		iterator begin() const {
			unsigned k = kh_begin(_h);
			while ( k != kh_end(_h) && !kh_exist(_h, k) )
				++k;
			return iterator(_h, k);
		}

		iterator end() const {
			return iterator(_h, kh_end(_h));
		}

		size_t size() const {
			return kh_size(_h);
		}

		void clear() {
			kh_clear(int, _h);
		}

		int insert(T v) {
			int ret;
			kh_put(int, _h, (uintptr_t)v, &ret);
			return ret;
		}

		iterator find(const void *v) const {
			return iterator(_h, kh_get(int, _h, (uintptr_t)v));
		}

		void erase(iterator it) {
			kh_del(int, _h, it.k);
		}

		bool contains(const void *v) const {
			return kh_get(int, _h, (uintptr_t)v) != kh_end(_h);
		}


	private:
		khash_t(int) *_h;
};


template <typename T>
class KHashSetPtrBase<T, 8> {
	public:
		struct iterator {
			iterator() {}
			iterator(const iterator &other) : h(other.h), k(other.k) {}
			iterator(khash_t(int64) *h_, unsigned k_) : h(h_), k(k_) {}

			bool operator==(const iterator &other) const {
				return k == other.k;
			}

			bool operator!=(const iterator &other) const {
				return k != other.k;
			}

			// Prefix
			iterator &operator++() {
				++k;
				while ( k != kh_end(h) && !kh_exist(h, k) )
					++k;

				return *this;
			}

			// Postfix
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}

			T operator*() const {
				return (T)kh_key(h, k);
			}

			khash_t(int64) *h;
			unsigned k;
		};


	public:
		KHashSetPtrBase<T, 8>() {
			_h = kh_init(int64);
		}

		~KHashSetPtrBase<T, 8>() {
			kh_destroy(int64, _h);
		}


	public:
		iterator begin() const {
			unsigned k = kh_begin(_h);
			while ( k != kh_end(_h) && !kh_exist(_h, k) )
				++k;
			return iterator(_h, k);
		}

		iterator end() const {
			return iterator(_h, kh_end(_h));
		}

		size_t size() const {
			return kh_size(_h);
		}

		void clear() {
			kh_clear(int64, _h);
		}

		int insert(T v) {
			int ret;
			kh_put(int64, _h, (uintptr_t)v, &ret);
			return ret;
		}

		iterator find(const void *v) const {
			return iterator(_h, kh_get(int64, _h, (uintptr_t)v));
		}

		void erase(iterator it) {
			kh_del(int64, _h, it.k);
		}

		bool contains(const void *v) const {
			return kh_get(int64, _h, (uintptr_t)v) != kh_end(_h);
		}


	private:
		khash_t(int64) *_h;
};


struct Arch {
	enum {
		PtrSize = sizeof(intptr_t)
	};
};


template <typename T>
class KHashSet<T*> : public KHashSetPtrBase<T*, Arch::PtrSize> {
	public:
		KHashSet<T*>() {}
};


}


#endif
