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


#include <seiscomp/core/strings.h>
#include <seiscomp/wired/ipacl.h>
#include <cstdio>
#include <iostream>


using namespace std;


namespace Seiscomp {
namespace Wired {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string toString(const IPMask &mask) {
	char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
	string ret;
	if ( mask.addr.toString(buf) > 0 )
		ret = buf;
	ret += '/';

	if ( mask.mask.zero() ) {
		ret += '0';
		return ret;
	}

	// Count subsequent bits set
	int bits = 0;
	bool checkOne = true;

	for ( int ch = 0; ch < 4; ++ch ) {
		for ( int i = 0; i < 32; ++i ) {
			if ( checkOne ) {
				if ( (mask.mask.dwords[ch] >> (31-i)) & 0x01 )
					++bits;
				else
					checkOne = false;
			}
			else {
				if ( (mask.mask.dwords[ch] >> (31-i)) & 0x01 ) {
					bits = -1;
					break;
				}
			}
		}

		if ( bits < 0 )
			break;
	}

	if ( bits >= 0 ) {
		ret += Core::toString(bits);
	}
	else {
		if ( mask.mask.toString(buf) > 0 )
			ret += buf;
		else
			ret += '0';
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fromString(IPMask &mask, const std::string &ipstr) {
	Socket::IPAddress addr;

	size_t p = ipstr.find('/');
	if ( p == string::npos ) {
		if ( addr.fromString(ipstr.c_str()) ) {
			Socket::IPAddress all;
			all.dwords[0] = 0xffffffff;
			if ( !addr.isV4() ) {
				all.dwords[1] = 0xffffffff;
				all.dwords[2] = 0xffffffff;
				all.dwords[3] = 0xffffffff;
			}

			mask = IPMask(addr, all);
			return true;
		}
	}
	else {
		Socket::IPAddress m;
		int maskbits, maxbits;

		if ( addr.fromString(ipstr.substr(0, p).c_str()) ) {
			if ( addr.isV4() )
				maxbits = 32;
			else
				maxbits = 128;

			if ( m.fromString(ipstr.substr(p+1).c_str()) ) {
				mask = IPMask(addr, m);
				return true;
			}
			else if ( Core::fromString(maskbits, ipstr.substr(p+1)) ) {
				if ( maskbits < 0 || maskbits > maxbits )
					return false;

				int bit = 0;
				for ( int ch = 0; ch < 4; ++ch ) {
					for ( int i = 0; i < 32; ++i, ++bit ) {
						if ( bit < maskbits )
							m.dwords[ch] |= 1 << (31-i);
					}
				}

				mask = IPMask(addr, m);
				return true;
			}
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IPACL::add(const string &ipstr) {
	IPMask mask;
	if ( !fromString(mask, ipstr) )
		return false;

	_masks.push_back(mask);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IPACL::check(const Socket::IPAddress &addr) const {
	if ( _masks.empty() ) return true;

	const_iterator p;
	for ( p = _masks.begin(); p != _masks.end(); ++p ) {
		bool match = true;
		for ( int i = 0; i < Socket::IPAddress::DWORDS; ++i ) {
			if ( (addr.dwords[i] ^ p->addr.dwords[i]) & p->mask.dwords[i] ) {
				match = false;
				break;
			}
		}

		if ( match )
			return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IPACL::not_check(const Socket::IPAddress &addr) const {
	if ( _masks.empty() ) return true;

	const_iterator p;
	for ( p = _masks.begin(); p != _masks.end(); ++p ) {
		bool match = true;

		for ( int i = 0; i < Socket::IPAddress::DWORDS; ++i ) {
			if ( (addr.dwords[i] ^ p->addr.dwords[i]) & p->mask.dwords[i] ) {
				match = false;
				break;
			}
		}

		if ( match )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IPACL::clear() {
	_masks.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IPACL::empty() const {
	return _masks.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IPACL::operator==(const IPACL &other) const {
	if ( _masks.size() != other._masks.size() ) return false;

	const_iterator p, o;
	for ( p = _masks.begin(), o = other._masks.begin(); p != _masks.end(); ++p, ++o ) {
		if ( *p != *o )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IPACL &IPACL::operator+=(const IPACL &other) {
	const_iterator it;
	for ( it = other.begin(); it != other.end(); ++it ) {
		const_iterator it_this = begin();
		bool found = false;
		for ( ; it_this != end(); ++it_this ) {
			if ( *it_this == *it ) {
				found = true;
				break;
			}
		}

		if ( !found )
			_masks.push_back(*it);
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IPACL &IPACL::operator-=(const IPACL &other) {
	const_iterator it;
	for ( it = other.begin(); it != other.end(); ++it ) {
		IPMasks::iterator it_this = _masks.begin();
		for ( ; it_this != _masks.end(); ++it_this ) {
			if ( *it_this == *it ) {
				_masks.erase(it_this);
				break;
			}
		}
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string toString(const IPACL &acl) {
	std::string ret;
	bool first = true;
	for ( IPACL::const_iterator it = acl.begin(); it != acl.end(); ++it ) {
		if ( !first ) ret += " ";
		ret += toString(*it);
		first = false;
	}
	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fromString(IPACL &acl, const std::string &str) {
	vector<string> items;
	if ( !Core::fromString(items, str) )
		return false;

	acl = IPACL();

	for ( size_t i = 0; i < items.size(); ++i ) {
		if ( !acl.add(items[i]) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
