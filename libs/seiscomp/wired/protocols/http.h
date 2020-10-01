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


#ifndef SEISCOMP_WIRED_PROTOCOLS_HTTP_H
#define SEISCOMP_WIRED_PROTOCOLS_HTTP_H


#include <seiscomp/core/enumeration.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/wired/clientsession.h>

#include "websocket.h"


namespace Seiscomp {
namespace Wired {


MAKEENUM(
	HttpStatus,
	EVALUES(
		HTTP_101,
		HTTP_200,
		HTTP_201,
		HTTP_202,
		HTTP_203,
		HTTP_204,
		HTTP_205,
		HTTP_206,
		HTTP_207,
		HTTP_300,
		HTTP_301,
		HTTP_302,
		HTTP_303,
		HTTP_304,
		HTTP_305,
		HTTP_306,
		HTTP_307,
		HTTP_310,
		HTTP_400,
		HTTP_401,
		HTTP_402,
		HTTP_403,
		HTTP_404,
		HTTP_405,
		HTTP_406,
		HTTP_407,
		HTTP_408,
		HTTP_409,
		HTTP_410,
		HTTP_411,
		HTTP_412,
		HTTP_413,
		HTTP_414,
		HTTP_415,
		HTTP_416,
		HTTP_417,
		HTTP_418,
		HTTP_421,
		HTTP_422,
		HTTP_423,
		HTTP_424,
		HTTP_425,
		HTTP_426,
		HTTP_500,
		HTTP_501,
		HTTP_502,
		HTTP_503,
		HTTP_504,
		HTTP_505,
		HTTP_506,
		HTTP_507,
		HTTP_509,
		HTTP_510,
		HTTP_LAST
	),
	ENAMES(
		"101 Switching Protocols",
		"200 OK",
		"201 Created",
		"202 Accepted",
		"203 Non-Authoritative Information",
		"204 No Content",
		"205 Reset Content",
		"206 Partial Content",
		"207 Multi-Status",
		"300 Multiple Choice",
		"301 Moved Permanently",
		"302 Found",
		"303 See Other",
		"304 Not Modified",
		"305 Use Proxy",
		"306 (reserved)",
		"307 Temporary Redirect",
		"310 ERR_TOO_MANY_REDIRECTS",
		"400 Bad Request",
		"401 Unauthorized",
		"402 Payment Required",
		"403 Forbidden",
		"404 Not Found",
		"405 Method Not Allowed",
		"406 Not Acceptable",
		"407 Proxy Authentication Required",
		"408 Request Time-out",
		"409 Conflict",
		"410 Gone",
		"411 Length Required",
		"412 Precondition Failed",
		"413 Request Entity Too Large",
		"414 Request-URI Too Long",
		"415 Unsupported Media Type",
		"416 Requested range not satisfiable",
		"417 Expectation Failed",
		"418 I'm a Teapot",
		"421 There are too many connections from your internet address",
		"422 Unprocessable Entity",
		"423 Locked",
		"424 Failed Dependency",
		"425 Unordered Collection",
		"426 Upgrade Required",
		"500 Internal Server Error",
		"501 Not Implemented",
		"502 Bad Gateway",
		"503 Service Unavailable",
		"504 Gateway Time-out",
		"505 HTTP Version not supported",
		"506 Variant Also Negotiates",
		"507 Insufficient Storage",
		"509 Bandwidth Limit Exceeded",
		"510 Not Extended",
		""
	)
);


struct HttpRequest {
	typedef Seiscomp::Core::Time Time;

	MAKEENUM(
		Type,
		EVALUES(GET, POST, OPTIONS, HEAD, PUT, DELETE, TRACE),
		ENAMES("GET", "POST", "OPTIONS", "HEAD", "PUT", "DELETE", "TRACE")
	);

	enum State {
		WAITING,  // Waiting state
		ENABLED,  // Request has enabled but not yet started
		READING,  // Still reading the request
		RUNNING,  // Running independently
		QRUNNING, // Running by using a request queue
		FINISHED  // Finished
	};

	Type        type;
	State       state;
	std::string version;
	std::string userAgent;
	std::string host;
	std::string path;
	std::string contentType;
	std::string options;
	std::string cookie;
	std::string referer;
	std::string origin;
	std::string upgradeTo;
	std::string data;
	std::string secWebsocketProtocol;
	std::string secWebsocketKey;
	int         secWebsocketVersion;
	Time        ifModifiedSince;
	bool        keepAlive;
	bool        addKeepAliveHeader;
	bool        upgrade;
	bool        isXMLHTTP;
	HttpStatus  status;
	Device::count_t tx;
};


struct URLOptionName {
	URLOptionName(const char *s) : name(s) {}
	const char *name;
};


struct URLOptionValue {
	URLOptionValue(const char *s) : value(s) {}
	const char *value;
};


struct URLOptions {
	URLOptions(const std::string &s) : _source(s.data()), _source_len(s.size()) {}

	URLOptions(const char *src, size_t l) : _source(src), _source_len(l) {}

	bool next();

	bool nameEquals(const char *s) const;
	bool valueEquals(const char *s) const;

	bool operator==(const URLOptionName &wrapper) const {
		return nameEquals(wrapper.name);
	}

	bool operator==(const URLOptionValue &wrapper) const {
		return valueEquals(wrapper.value);
	}

	const char  *_source;
	size_t       _source_len;

	const char  *name_start;
	size_t       name_len;
	const char  *val_start;
	size_t       val_len;
};


struct URLInsituOptions {
	URLInsituOptions(std::string &s) : _source(&s[0]), _source_len(s.size()) {}

	URLInsituOptions(char *src, size_t l) : _source(src), _source_len(l) {}

	bool next();

	bool nameEquals(const char *s) const;
	bool valueEquals(const char *s) const;

	bool operator==(const URLOptionName &wrapper) const {
		return nameEquals(wrapper.name);
	}

	bool operator==(const URLOptionValue &wrapper) const {
		return valueEquals(wrapper.value);
	}

	const char  *_source;
	size_t       _source_len;

	char        *name;
	size_t       name_len;
	char        *val;
	size_t       val_len;
};


class URLPath {
	public:
		URLPath(const std::string &s);
		URLPath(const char *src, size_t l);


	public:
		bool empty() const { return part_len == 0; }
		bool next();
		bool partEquals(const char *s) const;
		const char *remainder() const;
		size_t remainderLength() const;

		// Checks a sub path for equality
		bool operator==(const char *s) const { return partEquals(s); }
		bool operator!=(const char *s) const { return !partEquals(s); }


	public:
		const char  *part_start;
		size_t       part_len;

	private:
		const char  *_source;
		size_t       _source_len;
	};


class URLInsituPath {
	public:
		URLInsituPath(std::string &s);
		URLInsituPath(char *src, size_t l);


	public:
		bool empty() const { return part_len == 0; }
		bool next();
		bool partEquals(const char *s) const;
		char *savePart();
		char *remainder() const;
		size_t remainderLength() const;

		// Checks a sub path for equality
		bool operator==(const char *s) const { return partEquals(s); }
		bool operator!=(const char *s) const { return !partEquals(s); }


	public:
		char        *part_start;
		size_t       part_len;

	private:
		char        *_source;
		size_t       _source_len;
};



DEFINE_SMARTPOINTER(HttpSession);


class SC_SYSTEM_CORE_API HttpSession : public ClientSession {
	public:
		static std::string EmptyString;


	public:
		HttpSession(Device *sock, const char *protocol,
		            const char *server = nullptr);
		~HttpSession() override;


	public:
		void sendResponse(HttpStatus status);
		void sendResponse(const std::string &,
		                  HttpStatus status,
		                  const char *contentType,
		                  const char *cookie = nullptr);
		void sendResponse(const char *, size_t len,
		                  HttpStatus status,
		                  const char *contentType,
		                  const char *cookie = nullptr);

		/*
		 * additionalHeader must contain a trailing newline (\r\n) for
		 * each line.
		 */
		void sendResponse(Buffer*,
		                  HttpStatus status,
		                  const char *contentType,
		                  const char *cookie = nullptr,
		                  const char *additionalHeader = nullptr);
		void sendStatus(HttpStatus status, const std::string &content = EmptyString,
		                const char *contentType = "text/plain");

		void close() override;

		void redirect(const char *path);

		//! Resets a connection to its default state. Return false if
		//! an error should be raised and to terminate the session.
		//! reset is used if a waiting connection is reused and receives
		//! a new request.
		virtual bool reset();

		virtual void handleHeader(const char *name, size_t nlen,
		                          const char *value, size_t vlen);
		virtual bool handleRequest(HttpRequest &req);

		virtual bool handleGETRequest(HttpRequest &req);
		virtual bool handlePOSTRequest(HttpRequest &req);
		virtual bool handleOPTIONSRequest(HttpRequest &req);
		virtual bool handleHEADRequest(HttpRequest &req);
		virtual bool handlePUTRequest(HttpRequest &req);
		virtual bool handleDELETERequest(HttpRequest &req);
		virtual bool handleTRACERequest(HttpRequest &req);

		// Websocket functions
		void upgradeToWebsocket(HttpRequest &req, const char *protocol,
		                        uint64_t maxPayloadSize = 0);
		virtual void handleWebsocketFrame(Websocket::Frame &frame);

		//! Send a response as Websocket frame. If status is not NoStatus
		//! the connection is closed and the close parameter is ignored.
		void sendWebsocketResponse(const char *data, int len,
		                           Websocket::Frame::Type type,
		                           Websocket::Status statusCode =
		                               Websocket::NoStatus,
		                           bool close = false);

		static std::string urlencode(const std::string &s);
		static std::string urlencode(const char *s, int len);

		static std::string urldecode(const std::string &s);
		static std::string urldecode(const char *s, int len);

		static int urldecode(char *s, int len);


	protected:
		//! Handles a socket read for Websockets
		void handleReceive(const char *data, size_t len) override;
		void handleInbox(const char *data, size_t len) override;
		void handleInboxError(Error error) override;
		void handlePostData(const char *data, size_t len) override;
		void outboxFlushed() override;

		virtual bool validatePostDataSize(size_t postDataSize);
		virtual void requestFinished();


	protected:
		//! const char reference that needs to be managed by the application
		const char         *_protocol;
		const char         *_server;
		bool                _requestStarted;
		size_t              _dataSize;
		bool                _dataStarted;
		bool                _acceptGzip;
		bool                _upgradedToWebsocket;
		Websocket::FramePtr _websocketFrame;

		HttpRequest         _request;
};



inline URLPath::URLPath(const std::string &s)
: _source(&s[0])
, _source_len(s.size())
{
	while ( _source_len && (*_source == '/') ) {
		++_source;
		--_source_len;
	}
}

inline URLPath::URLPath(const char *src, size_t l)
: _source(src)
, _source_len(l)
{
	while ( _source_len && (*_source == '/') ) {
		++_source;
		--_source_len;
	}
}

inline bool URLPath::next() {
	size_t len;
	const char *data = Core::tokenize(_source, "/", _source_len, len);

	if ( data != nullptr ) {
		Core::trim(data, len);

		part_start = data;
		part_len = len;

		return true;
	}

	part_start = nullptr;
	part_len = 0;

	return false;
}

inline bool URLPath::partEquals(const char *s) const {
	if ( strlen(s) != part_len ) return false;
	return !strncmp(s, part_start, part_len);
}

inline const char *URLPath::remainder() const {
	return _source;
}

inline size_t URLPath::remainderLength() const {
	return _source_len;
}

inline URLInsituPath::URLInsituPath(std::string &s)
: part_start(&s[0])
, part_len(0)
, _source(part_start)
, _source_len(s.size())
{
	while ( _source_len && (*_source == '/') ) {
		++_source;
		--_source_len;
	}
}

inline URLInsituPath::URLInsituPath(char *src, size_t l)
: part_start(src)
, part_len(0)
, _source(part_start)
, _source_len(l)
{
	while ( _source_len && (*_source == '/') ) {
		++_source;
		--_source_len;
	}
}

inline bool URLInsituPath::next() {
	size_t len;
	char *data = Core::tokenize2(_source, "/", _source_len, len);

	if ( data != nullptr ) {
		Core::trim(data,len);

		part_start = data;
		part_len = len;

		return true;
	}

	part_start = nullptr;
	part_len = 0;

	return false;
}

inline bool URLInsituPath::partEquals(const char *s) const {
	if ( strlen(s) != part_len ) return false;
	return !strncmp(s, part_start, part_len);
}

inline char *URLInsituPath::savePart() {
	part_start[part_len] = '\0';
	return part_start;
}

inline char *URLInsituPath::remainder() const {
	return _source;
}

inline size_t URLInsituPath::remainderLength() const {
	return _source_len;
}


}
}


#endif
