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


#ifndef SEISCOMP_APPS_SCWSAS_SESSION_H__
#define SEISCOMP_APPS_SCWSAS_SESSION_H__


#include <seiscomp/wired/clientsession.h>
#include <seiscomp/wired/endpoint.h>
#include <seiscomp/wired/ipacl.h>

#include <seiscomp/io/recordinput.h>

#include <map>
#include <list>
#include <vector>
#include <set>


#define MAX_CHUNK_SIZE 10240


namespace Seiscomp {
namespace Applications {
namespace Wfas {


struct RequestItem {
	std::string net;
	std::string sta;
	std::string loc;
	std::string cha;
	Core::Time  startTime;
	Core::Time  endTime;
};

typedef std::list<RequestItem> RequestItems;


DEFINE_SMARTPOINTER(Request);
struct Request : Core::BaseObject {
	Request() : id(0) {}

	size_t       id;
	RequestItems items;
};

typedef std::map<size_t, RequestPtr> Requests;


class ArclinkListener : public Wired::AccessControlledEndpoint {
	public:
		ArclinkListener(const Wired::IPACL &allowedIPs,
		                const Wired::IPACL &deniedIPs,
		                Wired::Socket *socket = NULL);

	protected:
		Wired::Session *createSession(Wired::Socket *socket) override;

	private:
		Requests _requests;
};


class FDSNWSListener : public Wired::AccessControlledEndpoint {
	public:
		FDSNWSListener(const Wired::IPACL &allowedIPs,
		               const Wired::IPACL &deniedIPs,
		               Wired::Socket *socket = NULL);

	protected:
		Wired::Session *createSession(Wired::Socket *socket) override;
};


DEFINE_SMARTPOINTER(Chunk);
class Chunk : public Core::BaseObject {
	public:
		std::string header;
		std::string data;
};


class ClientSession : public Wired::ClientSession {
	public:
		ClientSession(Wired::Socket *s, size_t maxLen);

	protected:
		enum ClientState {
			Unspecific,
			StreamRequest,
			Downloading
		};

		ClientState          _state;
		RequestPtr           _currentRequest;
		RequestItem          _currentItem;
		IO::RecordStreamPtr  _recordStream;
		IO::RecordInputPtr   _recordInput;
};


// Util functions
bool parseTime(Core::Time &time, const char *data, size_t len);
bool validate(const std::string &code);


}
}
}


#endif
