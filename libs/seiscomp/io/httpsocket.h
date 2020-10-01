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


#ifndef SEISCOMP_IO_HTTPSOCKET_H
#define SEISCOMP_IO_HTTPSOCKET_H


#include <boost/iostreams/filter/zlib.hpp>


namespace Seiscomp {
namespace IO {


template <typename SocketType>
class SC_SYSTEM_CORE_API HttpSocket : public SocketType {
	public:
		HttpSocket();
		virtual ~HttpSocket();
		virtual void open(const std::string& serverHost,
			const std::string& user = "", const std::string& password = "");
		void httpGet(const std::string &path);
		void httpPost(const std::string &path, const std::string &msg);
		std::string httpReadRaw(int size);
		std::string httpReadSome(int size);
		std::string httpRead(int size);

	private:
		std::string _serverHost;
		std::string _user;
		std::string _password;
		std::string _error;
		bool _chunkMode;
		int _remainingBytes;
		boost::iostreams::zlib_decompressor *_decomp;

		void httpReadResponse();
		void sendAuthorization();
};


}
}


#endif
