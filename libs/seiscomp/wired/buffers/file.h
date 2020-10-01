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


#ifndef SEISCOMP_WIRED_BUFFERS_FILE_H
#define SEISCOMP_WIRED_BUFFERS_FILE_H


#include <seiscomp/wired/buffer.h>
#include <cstdio>


namespace Seiscomp {
namespace Wired {


DEFINE_SMARTPOINTER(FileBuffer);

/**
 * @brief The FileBuffer transfers a file in chunked mode over a HTTP
 *        connection. It only supports regular files, no symlinks, no
 *        directories, no pipes and so on.
 *        Technically, S_ISREG(stat(fn).st_mode) must evaluate to true.
 */
class SC_SYSTEM_CORE_API FileBuffer : public Buffer {
	public:
		FileBuffer(int max_size = 0x7fffffff);
		~FileBuffer() override;

	public:
		bool open(const std::string &fn, const char *mode = "rb");
		bool open(const char *fn, const char *mode = "rb");

		bool updateBuffer() override;
		size_t length() const override;

	public:
		enum Type {
			HTML,
			CSS,
			JS,
			JSON,
			PNG,
			JPG,
			SVG,
			XML,
			WOFF,
			TTF,
			TypeQuantity
		};

		static const char *mimeType(Type);
		static const char *mimeType(const char *filenameExtension);

		FILE  *fp;
		long fplen;
};


}
}


#endif
