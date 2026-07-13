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


#ifndef SEISCOMP_IO_HTTPCLIENT_H
#define SEISCOMP_IO_HTTPCLIENT_H


#include <ios>
#include <string>

#include <seiscomp/core.h>
#include <seiscomp/io/socket.h>
#include <seiscomp/utils/url.h>


namespace Seiscomp::IO {


/**
 * @brief Generic HTTP/HTTPS client supporting GET and POST requests with connection
 *        reuse (HTTP keep-alive).
 *
 * The HTTPClient class encapsulates connection handling for HTTP and HTTPS including
 * proxy support (via the standard http_proxy/HTTPS_PROXY/no_proxy environment
 * variables), HTTP Basic and Digest authentication and redirect following. Response
 * bodies can be read with readBinary() which transparently handles both Content-Length
 * and chunked transfer encoding.
 *
 * Each call to get() or post() takes the target URL as a parameter. When the new URL
 * points at the same host/port/scheme as the previous one and the server signalled
 * support for keep-alive (HTTP/1.1 without "Connection: close", or HTTP/1.0 with
 * "Connection: keep-alive"), the existing TCP/TLS connection is reused. Otherwise a
 * fresh connection is opened.
 *
 * Typical usage:
 *
 * \code{.cpp}
 * IO::HTTPClient http;
 * http.setTimeout(10);
 *
 * http.get("https://example.org/resource");
 * std::string body = http.readBinary(http.remainingBytes());
 *
 * // The next call reuses the connection if the server kept it alive
 * http.post("https://example.org/upload", "key=value",
 *           "application/x-www-form-urlencoded");
 * std::string reply = http.readBinary(http.remainingBytes());
 * \endcode
 *
 * This class is not a complete HTTP implementation: it is sufficient for the
 * request/response patterns used by SeisComP's web service clients but does not attempt
 * to cover the whole RFC 7230 family.
 */
class SC_SYSTEM_CORE_API HTTPClient {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Default constructor.
		 *
		 * The client starts without a socket; one will be created implicitly by the
		 * first get() or post() call. The caller may optionally tune the socket timeout
		 * up front via setTimeout() — that setting is propagated to every newly created
		 * socket.
		 */
		HTTPClient() = default;

		virtual ~HTTPClient() = default;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Sets the read/write timeout. The timeout is applied to the current
		 *        socket (if any) and remembered for future sockets created by get()/post().
		 * @param seconds Timeout in seconds.
		 */
		void setTimeout(int seconds);

		/**
		 * @brief Interrupts a blocking socket operation. Safe to call from a different
		 *        thread, just as IO::Socket::interrupt() is.
		 */
		void close();

		/**
		 * @brief Closes the socket and resets the response body parsing state (chunked
		 *        mode flag, remaining body bytes and any accumulated error text). The
		 *        URL and credentials are preserved so a fresh request can be issued.
		 */
		void reset();

		/**
		 * @brief Closes the active TCP/TLS connection, if any.
		 *
		 * Calling disconnect() forces the next get() / post() to open a fresh
		 * connection even when the server signalled keep-alive. Use it to release
		 * resources held by the client or to start over after a protocol error.
		 */
		void disconnect();

		/**
		 * @brief Issues an HTTP GET request for \p url and reads the response status
		 *        line and headers.
		 *
		 * The URL replaces any previously configured one. If the client already holds a
		 * keep-alive connection to the same host/port/scheme that connection is reused;
		 * otherwise a new connection is opened.
		 *
		 * On success the body is left in the socket ready to be read via readBinary().
		 * The chunked() and remainingBytes() accessors describe how it should be
		 * consumed.
		 *
		 * Authentication challenges (401 Digest) are handled internally by re-issuing
		 * the request with the appropriate Authorization header. Redirects (3xx) are
		 * followed up to a fixed depth.
		 *
		 * @param url           The request URL. May or may not include a scheme;
		 *                      "https" is assumed if none is given.
		 * @param authHeader    Internal use; pre-computed Authorization header used
		 *                      when retrying after a 401 response.
		 * @param redirectCount Internal counter used while following redirects. Callers
		 *                      should leave this at the default.
		 *
		 * Throws Core::GeneralException on protocol errors.
		 */
		void get(const std::string &url, std::string authHeader = {},
		         size_t redirectCount = 0);

		/**
		 * @brief Issues an HTTP POST request for \p url with \p postData as the request
		 *        body and reads the response status line and headers.
		 *
		 * Behavior, response body handling, authentication, redirect handling and
		 * connection reuse are the same as for get().
		 *
		 * @param url           The request URL, see get().
		 * @param postData      The request body.
		 * @param contentType   Value of the Content-Type request header.  Defaults to
		 *                      "text/plain"; callers that send XML, JSON or any other
		 *                      format should pass the appropriate media type so that
		 *                      the server can dispatch on it.
		 * @param authHeader    Internal use, see get().
		 * @param redirectCount Internal counter, see get().
		 *
		 * Throws Core::GeneralException on protocol errors.
		 */
		void post(const std::string &url,
		          const std::string &postData,
		          const std::string &contentType = "text/plain",
		          std::string authHeader = {},
		          size_t redirectCount = 0);

		/**
		 * @brief Reads up to \p size bytes from the response body.
		 *
		 * Transparently handles both fixed Content-Length and chunked transfer
		 * encoding. Returns the data read; the actual number of bytes returned may be
		 * less than \p size if the body ends before \p size bytes were available.
		 *
		 * When the response is fully consumed the socket is closed unless the server
		 * signalled keep-alive, in which case it is kept open for reuse.
		 *
		 * @param size  Maximum number of body bytes to return. Values <= 0 yield an
		 *              empty string.
		 * @return The bytes read.
		 *
		 * Throws Core::GeneralException on protocol errors or when an error body had
		 * been accumulated via error() and the chunked stream terminates.
		 */
		std::string readBinary(std::streamsize size);

		/**
		 * @brief Reads up to \p count bytes from the response body into the
		 *        caller-provided buffer \p b.
		 *
		 * This is the buffer-oriented counterpart of readBinary(): it shares the
		 * same body-parsing machinery (transparent Content-Length and chunked
		 * transfer decoding, keep-alive handling) but writes directly into \p b
		 * instead of allocating a std::string. It is convenient for streaming a
		 * response into a fixed-size buffer or an std::streambuf.
		 *
		 * Fewer than \p count bytes are returned when the body ends first. Once the
		 * body is fully consumed the socket is closed unless the server signalled
		 * keep-alive.
		 *
		 * @param b     Destination buffer; must hold at least \p count bytes.
		 * @param count Maximum number of body bytes to read. Values <= 0 read
		 *              nothing and return 0.
		 * @return The number of bytes written to \p b.
		 *
		 * Throws Core::GeneralException on protocol errors or when an error body had
		 * been accumulated via error() and the chunked stream terminates.
		 */
		std::streamsize read(void *b, std::streamsize count);


	// ------------------------------------------------------------------
	//  Accessors
	// ------------------------------------------------------------------
	public:
		//! Returns the scheme of the URL of the last get() / post() call ("http" or
		//! "https"). Empty until a request has been issued.
		std::string protocol() const;

		//! Returns the underlying socket. Null until the first get() or post() call.
		IO::Socket *socket() const { return _socket.get(); }

		//! Returns the default TCP port for the scheme of the URL of the last get()
		//! / post() call: 80 for "http", 443 for //! "https".
		//! Returns 0 for an unknown / empty scheme.
		int defaultPort() const;

		//! Returns the URL of the last get() / post() call.
		const Util::Url &url() const { return _url; }

		//! Returns true if the current response uses chunked transfer encoding.
		bool chunked() const { return _chunkMode; }

		//! Returns the number of body bytes still to be read. For chunked responses
		//! this is the number of bytes left in the current chunk. A value of -1 means
		//! "not yet known".
		std::streamsize remainingBytes() const { return _remainingBytes; }

		//! True if the server indicated that the TCP/TLS connection is being kept open
		//! and may be reused for the next request.
		bool keepAlive() const { return _keepAlive; }

		//! Accumulated error text. Callers may append to this to influence readBinary()
		//! behavior when a chunked stream terminates.
		const std::string &error() const { return _error; }
		std::string &error() { return _error; }


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		/**
		 * @brief Parses and stores the request URL and prepares the socket for the
		 *        URL's scheme.
		 *
		 * If \p url has no scheme it is assumed to be "https". The scheme determines
		 * the socket type that is allocated internally: IO::Socket for "http" and
		 * IO::SSLSocket for "https". A previously held socket is replaced and any open
		 * connection is dropped.
		 *
		 * This hook is virtual so that subclasses can apply service-specific URL
		 * normalization (e.g., mapping fdsnws:// to http://) before delegating to the
		 * base class implementation. It is called internally by get() and post() when a
		 * new connection must be opened.
		 *
		 * @param url The URL to be used for subsequent requests.
		 * @return True if the URL is syntactically valid, false otherwise.
		 */
		virtual bool setURL(const std::string &url);

		/**
		 * @brief Opens the TCP/TLS connection to the configured URL, honoring proxy
		 *        settings from the environment.
		 *
		 * If an HTTPS proxy is configured this also performs the CONNECT handshake
		 * (including optional proxy authentication) and upgrades the socket to TLS. The
		 * socket may be replaced during this call if the proxy uses a different scheme
		 * than the target URL.
		 *
		 * Called internally by get() / post() when no keep-alive connection can be
		 * reused. Made protected so service-specific subclasses can drive the connect
		 * step explicitly when needed.
		 *
		 * Throws Core::GeneralException on protocol errors.
		 */
		void openConnection();

		/**
		 * @brief Returns the proxy server URL applicable to the current request, or
		 *        null if no proxy applies.
		 *
		 * Inspects the environment variables NO_PROXY/no_proxy first, then
		 * http_proxy/HTTP_PROXY (for "http") or HTTPS_PROXY/ https_proxy (for "https").
		 * The returned pointer is owned by the C runtime (getenv) and must not be
		 * freed.
		 */
		const char *getProxy() const;

		/**
		 * @brief Common implementation of get() and post().
		 * @param method        The HTTP method ("GET" or "POST").
		 * @param body          The request body. Sent only for POST.
		 * @param contentType   Value of the Content-Type header. Ignored for methods
		 *                      that send no body.
		 * @param authHeader    Pre-computed Authorization header for retry.
		 * @param redirectCount Redirect recursion counter.
		 */
		void request(const char *method, const std::string &body,
		             const std::string &contentType, std::string authHeader,
		             size_t redirectCount);

		/**
		 * @brief Returns the request target (path plus query string) for the current
		 *        URL, e.g., "/api/1/try-to-associate?allocate".
		 *
		 * The HTTP request line must carry the query string, otherwise server-side
		 * query options (such as ?allocate) are silently lost. Util::Url::path()
		 * returns only the path, so the query is rebuilt here from the parsed query
		 * items.
		 */
		std::string requestTarget() const;

		/**
		 * @brief Prepares the connection for a request to \p url.
		 *
		 * If the existing socket is open, the server signalled keep-alive on the
		 * previous response, and the new URL targets the same host/port/scheme, the
		 * socket is left untouched and only the URL is updated. Otherwise any existing
		 * connection is dropped, setURL() is called to (re)allocate the right socket
		 * type, and openConnection() is invoked.
		 */
		void prepareConnection(const std::string &url);

		/**
		 * @brief Shared body-reading state machine backing readBinary() and read().
		 *
		 * Reads up to \p count bytes of the response body into \p out, decoding
		 * Content-Length or chunked transfer encoding transparently. When the body
		 * is fully consumed the socket is closed unless \p close is false or the
		 * server signalled keep-alive.
		 *
		 * @param out   Destination buffer; must hold at least \p count bytes.
		 * @param count Maximum number of bytes to read.
		 * @param close Whether to close the socket once the body is fully consumed
		 *              (subject to keep-alive).
		 * @return The number of bytes written to \p b.
		 */
		std::streamsize readBody(char *out, std::streamsize count,
		                         bool close = true);


	// ------------------------------------------------------------------
	//  Protected members
	// ------------------------------------------------------------------
	protected:
		IO::SocketPtr     _socket;
		Util::Url         _url;
		int               _timeout{0};

		//! Identity of the currently open keep-alive connection.
		//! Empty when no connection is open or keep-alive is off.
		std::string       _connectionHost;
		size_t            _connectionPort{0};
		std::string       _connectionScheme;

		//! Set to true while the most recent response declared the connection reusable
		//! (HTTP/1.1 implicit keep-alive or explicit "Connection: keep-alive"). Reset
		//! to false on any error or after the connection is dropped.
		bool              _keepAlive{false};

		//! Response body parsing state, populated by get()/post() and consumed by
		//! readBinary().
		bool              _chunkMode{false};
		std::streamsize   _remainingBytes{0};
		std::string       _error;
};


}


#endif
