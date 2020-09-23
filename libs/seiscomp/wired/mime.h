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


#ifndef SEISCOMP_WIRED_MIME_H
#define SEISCOMP_WIRED_MIME_H


#include <list>
#include <seiscomp/core/enumeration.h>


namespace Seiscomp {
namespace Wire {
namespace MIME {


// Section 7.2
class Multipart {
	public:
		struct CustomHeader {
			CustomHeader()
			: name(nullptr), name_len(0), value(nullptr), value_len(0) {}

			const char  *name;
			size_t       name_len;
			const char  *value;
			size_t       value_len;
		};

		typedef std::list<CustomHeader> CustomHeaders;

		static bool ParseContentType(const std::string &contentType,
		                             std::string &subtype,
		                             std::string &boundary);

		Multipart(const std::string &boundary, const std::string &s);
		Multipart(const std::string &boundary, const char *src, size_t l);

		bool next();

		bool typeEquals(const char *s) const;
		bool typeStartsWith(const char *s, size_t len) const;

		bool operator==(const std::string &type) const {
			return typeEquals(type.c_str());
		}

		const CustomHeaders &customHeaders() { return _customHeaders; }


	protected:
		void init(const std::string &boundary, const char *src, size_t l);
		size_t findString(const char *needle, size_t len, size_t haystack_len = std::string::npos);


	public:
		const char    *type;
		size_t         type_len;
		const char    *disposition;
		size_t         disposition_len;
		const char    *transfer_enc;
		size_t         transfer_enc_len;
		const char    *body;
		size_t         body_len;


	protected:
		std::string    _boundary;
		const char    *_source;
		size_t         _source_len;
		size_t         _part;

		CustomHeaders  _customHeaders;
};


}
}
}


#endif
