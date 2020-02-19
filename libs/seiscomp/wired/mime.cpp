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


#define SEISCOMP_COMPONENT Wire

#define CRLF "\r\n"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>

#include <string.h>

#include "mime.h"


using namespace std;
using namespace Seiscomp::Core;


namespace Seiscomp {
namespace Wire {
namespace MIME {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Multipart::ParseContentType(const string &contentType, string &subtype,
                                 string &boundary) {
	const char *data = contentType.data();
	int len = contentType.size();

	const char *tok = NULL;
	int len_tok = 0;

	subtype.clear();
	boundary.clear();

	// check multipart key word
	trimFront(data, len);
	if ( len < 9 || strncasecmp(data, "multipart", 9) )
		return false;
	data += 9; len -= 9;

	// read subtype (if any)
	tok = tokenize(data, ";", len, len_tok);
	trim(tok, len_tok);
	if ( len_tok > 0 ) {
		if ( *tok != '/' ) return false;
		subtype.assign(++tok, --len_tok);
	}

	// read parameters, stop on first boundary parameter
	while ( (tok = tokenize(data, ";", len, len_tok)) != NULL ) {
		trim(tok, len_tok);

		if ( len_tok >= 9 && !strncasecmp(tok, "boundary=", 9) ) {
			tok += 9; len_tok -= 9;

			// boundary must consist of at least one character
			if ( len_tok < 1 )
				return false;

			// remove quotes, if present
			if ( tok[0] == '"' ) {
				if ( len_tok < 3 || tok[len_tok-1] != '"')
					return false;
				++tok, len_tok -= 2;
			}

			boundary.assign(tok, len_tok);
			return true;
		}
	}

	return false;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Multipart::Multipart(const std::string &boundary, const std::string &s) {
	init(boundary, s.data(), s.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Multipart::Multipart(const std::string &boundary, const char *src, int l) {
	init(boundary, src, l);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Multipart::next() {
	type = disposition = transfer_enc = body = NULL;
	type_len = disposition_len = transfer_enc_len = body_len = 0;
	_customHeaders.clear();

	if ( _source == NULL ) return false;

	int pos = 0, tmpPos = 0;

	// first part: find boundary preamble, ignore data before
	if ( _part == 0 ) {
		// CRLF may be ommitted in first boundary if preamble is empty
		if ( _source_len >= (int)_boundary.size() - 2 &&
		     !strncmp(_source, &_boundary[2], _boundary.size() - 2) ) {
			advance(_source, _source_len, _boundary.size() - 2);
			SEISCOMP_DEBUG("initial boundary found at position 0");
		}
		else {
			pos = findString(_boundary.data(), _boundary.size());
			if ( pos < 0 ) {
				SEISCOMP_ERROR("no multipart boundary found");
				_source = NULL;
				return false;
			}
			advance(_source, _source_len, pos + _boundary.size());
			SEISCOMP_DEBUG("initial boundary found at position %i", pos + 2);
		}
	}

	// check if this is the last part
	if ( _source_len >= 2 && !strncmp(_source, "--", 2) ) {
		SEISCOMP_DEBUG("closing boundary found after part %i", _part);
		_source = NULL;
		return false;
	}

	++_part;

	// read CRLF from prevous boundary
	if ( _source_len < 2 || strncmp(_source, CRLF, 2) ) {
		SEISCOMP_ERROR("missing CRLF before part %i", _part);
		_source = NULL;
		return false;
	}
	advance(_source, _source_len, 2);

	SEISCOMP_DEBUG("reading part %i", _part);

	// read header until empty line is found
	int line = 1;
	while ( (pos = findString(CRLF, 2)) != 0 ) {
		if ( pos < 0 ) {
			SEISCOMP_ERROR("invalid header in part %i, line %i, could not "
			               "find CRLF", _part, line);
			_source = NULL;
			return false;
		}

		// Content-Type
		if ( pos >= 13 && !strncasecmp(_source, "Content-Type:", 13) ) {
			type = _source + 13;
			type_len = pos - 13;
			trim(type, type_len);
		}
		// Content-Disposition
		else if ( pos >= 20 && !strncasecmp(_source, "Content-Disposition:", 20) ) {
			disposition = _source + 20;
			disposition_len = pos - 20;
			trim(disposition, disposition_len);
		}
		// Content-Transfer-Encoding
		else if ( pos >= 26 && !strncasecmp(_source, "Content-Transfer-Encoding:", 26) ) {
			transfer_enc = _source + 26;
			transfer_enc_len = pos - 26;
			trim(transfer_enc, transfer_enc_len);
		}
		// custom header fields
		else if ( (tmpPos = findString(":", 1, pos-1)) > 0 ) {
			_customHeaders.resize(_customHeaders.size() + 1);
			CustomHeader &h = _customHeaders.back();
			h.name = _source;
			h.name_len = tmpPos;
			h.value = _source + tmpPos + 1;
			h.value_len = pos - tmpPos - 1;
			trim(h.name, h.name_len);
			trim(h.value, h.value_len);
		}

		advance(_source, _source_len, pos + 2);
	}
	advance(_source, _source_len, 2);

	// find boundary
	pos = findString(_boundary.data(), _boundary.size());
	if ( pos < 0 ) {
		SEISCOMP_ERROR("invalid body in part %i, could not find boundary", _part);
		_source = NULL;
		return false;
	}

	body = _source;
	body_len = pos;
	advance(_source, _source_len, pos + _boundary.size());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Multipart::typeEquals(const char *s) const {
	return !strncmp(s, type, type_len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Multipart::typeStartsWith(const char *s, int len) const {
	return len <= type_len && !strncmp(s, type, len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Multipart::init(const std::string &boundary, const char *src, int l) {
	_boundary = CRLF "--" + boundary;
	_source = src;
	_source_len = l;

	type = body = NULL;
	type_len = body_len = _part = 0;
	_customHeaders.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Multipart::findString(const char *needle, int len, int haystack_len) {
	if ( needle == NULL || len <= 0 )
		return -1;
	if ( haystack_len < 0 || haystack_len > _source_len )
		haystack_len = _source_len;

	const char *haystack = _source;
	for ( int iH = 0, iN = 0; iH < haystack_len; ++iH, ++haystack ) {
		if ( *haystack == *needle ) {
			if ( ++iN == len)
				return iH - iN + 1;
			++needle;
		}
		else if ( iN > 0 ) {
			needle -= iN;
			iN = 0;
			--haystack;
			--iH;
		}
	}
	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
