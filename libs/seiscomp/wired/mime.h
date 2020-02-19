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


#ifndef SEISCOMP_WIRED_MIME_H__
#define SEISCOMP_WIRED_MIME_H__


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
			: name(NULL), name_len(0), value(NULL), value_len(0) {}

			const char  *name;
			int          name_len;
			const char  *value;
			int          value_len;
		};

		typedef std::list<CustomHeader> CustomHeaders;

		static bool ParseContentType(const std::string &contentType,
		                             std::string &subtype,
		                             std::string &boundary);

		Multipart(const std::string &boundary, const std::string &s);
		Multipart(const std::string &boundary, const char *src, int l);

		bool next();

		bool typeEquals(const char *s) const;
		bool typeStartsWith(const char *s, int len) const;

		bool operator==(const std::string &type) const {
			return typeEquals(type.c_str());
		}

		const CustomHeaders &customHeaders() { return _customHeaders; }


	protected:
		void init(const std::string &boundary, const char *src, int l);
		int findString(const char *needle, int len, int haystack_len = -1);


	public:
		const char    *type;
		int            type_len;
		const char    *disposition;
		int            disposition_len;
		const char    *transfer_enc;
		int            transfer_enc_len;
		const char    *body;
		int            body_len;


	protected:
		std::string    _boundary;
		const char    *_source;
		int            _source_len;
		int            _part;

		CustomHeaders  _customHeaders;
};


}
}
}


#endif
