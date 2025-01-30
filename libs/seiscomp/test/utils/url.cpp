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


#define SEISCOMP_TEST_MODULE SeisComP

#include <boost/optional/optional_io.hpp>
#include <seiscomp/unittest/unittests.h>

#include <seiscomp/utils/url.h>


using namespace Seiscomp::Util;


BOOST_AUTO_TEST_SUITE(seiscomp_utils_url)


BOOST_AUTO_TEST_CASE(urls) {
	Url url;

	BOOST_REQUIRE(url.setUrl("slink://localhost:18000"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "slink://localhost:18000");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "slink");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18000);
	BOOST_CHECK(url.path().empty());

	BOOST_REQUIRE(url.setUrl("test.mseed"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "test.mseed");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "");
	BOOST_CHECK_EQUAL(url.host(), "test.mseed");
	BOOST_CHECK_EQUAL(url.port(), Seiscomp::Core::None);
	BOOST_CHECK_EQUAL(url.path(), "");

	BOOST_REQUIRE(url.setUrl("http://username:pass:word@example.org/"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "http://username:pass:word@example.org/");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "username");
	BOOST_CHECK_EQUAL(url.password(), "pass:word");
	BOOST_CHECK_EQUAL(url.host(), "example.org");
	BOOST_CHECK_EQUAL(*url.port(), 80);
	BOOST_CHECK_EQUAL(url.path(), "/");

	BOOST_REQUIRE(url.setUrl("http://example.org/@foo"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "http://example.org/@foo");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "example.org");
	BOOST_CHECK_EQUAL(*url.port(), 80);
	BOOST_CHECK_EQUAL(url.path(), "/@foo");

	BOOST_REQUIRE(url.setUrl("fdsnws://localhost:8080/fdsnws/1/station?max-dist=12&cha=BHZ#help"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "fdsnws://localhost:8080/fdsnws/1/station?max-dist=12&cha=BHZ#help");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "fdsnws");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 8080);
	BOOST_CHECK_EQUAL(url.path(), "/fdsnws/1/station");
	BOOST_CHECK_EQUAL(url.query(), "max-dist=12&cha=BHZ");
	BOOST_CHECK_EQUAL(url.queryItemValue("max-dist"), "12");
	BOOST_CHECK_EQUAL(url.queryItemValue("cha"), "BHZ");
	BOOST_CHECK_EQUAL(url.fragment(), "help");

	BOOST_REQUIRE(url.setUrl("http://localhost:18100/testing"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "http://localhost:18100/testing");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");

	BOOST_REQUIRE(url.setUrl("http://localhost:18100/testing?ack-window=30"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "http://localhost:18100/testing?ack-window=30");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");
	BOOST_CHECK(url.hasQueryItem("ack-window"));
	BOOST_CHECK_EQUAL(url.queryItemValue("ack-window"), "30");

	BOOST_REQUIRE(url.setUrl("http://localhost:18100/testing?ack-window=30&ssl=true"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "http://localhost:18100/testing?ack-window=30&ssl=true");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");
	BOOST_CHECK(url.hasQueryItem("ack-window"));
	BOOST_CHECK(url.hasQueryItem("ssl"));
	BOOST_CHECK_EQUAL(url.queryItemValue("ack-window"), "30");
	BOOST_CHECK_EQUAL(url.queryItemValue("ssl"), "true");

	BOOST_REQUIRE(url.setUrl("http://[::1]:18100/testing?ack-window=30&ssl=true"));
	BOOST_CHECK_EQUAL(static_cast<const char*>(url), "http://[::1]:18100/testing?ack-window=30&ssl=true");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "::1");
	BOOST_CHECK_EQUAL(*url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");
	BOOST_CHECK(url.hasQueryItem("ack-window"));
	BOOST_CHECK(url.hasQueryItem("ssl"));
	BOOST_CHECK_EQUAL(url.queryItemValue("ack-window"), "30");
	BOOST_CHECK_EQUAL(url.queryItemValue("ssl"), "true");

	BOOST_CHECK_EQUAL(Url("hostname").host(), "hostname");
	BOOST_CHECK_EQUAL(Url("hostname:18180").host(), "hostname");
	BOOST_CHECK_EQUAL(*Url("hostname:18180").port(), 18180);

	// IPv6
	BOOST_REQUIRE(url.setUrl("http://[::1]"));
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.host(), "::1");
	BOOST_CHECK_EQUAL(*url.port(), 80);

	BOOST_REQUIRE(url.setUrl("http://[::1]:1234"));
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.host(), "::1");
	BOOST_CHECK_EQUAL(*url.port(), 1234);

	BOOST_REQUIRE(url.setUrl("custom://[::1]"));
	BOOST_CHECK_EQUAL(url.scheme(), "custom");
	BOOST_CHECK_EQUAL(url.host(), "::1");
	BOOST_CHECK_EQUAL(url.port(), Seiscomp::Core::None);

	BOOST_REQUIRE(url.setUrl("custom://[::1]:1234"));
	BOOST_CHECK_EQUAL(url.scheme(), "custom");
	BOOST_CHECK_EQUAL(url.host(), "::1");
	BOOST_CHECK_EQUAL(*url.port(), 1234);

	BOOST_REQUIRE(url.setUrl("sqlite3:///path/to/file"));
	BOOST_CHECK_EQUAL(url.scheme(), "sqlite3");
	BOOST_CHECK_EQUAL(url.host(), "");
	BOOST_CHECK_EQUAL(url.path(), "/path/to/file");

	BOOST_REQUIRE(url.setUrl("sqlite3://./file"));
	BOOST_CHECK_EQUAL(url.host(), ".");
	BOOST_CHECK_EQUAL(url.path(), "/file");

	/*
	BOOST_REQUIRE(url.setUrl("urn:example:animal:ferret:nose"));
	BOOST_CHECK_EQUAL(url.scheme(), "urn");
	BOOST_CHECK_EQUAL(url.path(), "example:animal:ferret:nose");
	*/

	BOOST_REQUIRE(url.setUrl("https://example.com/search?Hello%20World%21%26lang%3Den"));
	BOOST_REQUIRE(url.setUrl("ftp://ftp.is.co.za/rfc/rfc1808.txt"));
	BOOST_REQUIRE(url.setUrl("http://www.ietf.org/rfc/rfc2396.txt"));
	BOOST_REQUIRE(url.setUrl("ldap://[2001:db8::7]/c=GB?objectClass?one"));
	//BOOST_REQUIRE(url.setUrl("mailto:John.Doe@example.com"));
	//BOOST_REQUIRE(url.setUrl("news:comp.infosystems.www.servers.unix"));
	//BOOST_REQUIRE(url.setUrl("tel:+1-816-555-1212"));
	BOOST_REQUIRE(url.setUrl("telnet://192.0.2.16:80/"));
}

BOOST_AUTO_TEST_CASE(Extract) {
	BOOST_CHECK_EQUAL(Url("http://localhost").withoutScheme(), "localhost");
	BOOST_CHECK_EQUAL(Url("loc:/localhost").withoutScheme(), "/localhost");
	BOOST_CHECK_EQUAL(Url("urn:localhost").withoutScheme(), "localhost");
	BOOST_CHECK_EQUAL(Url("urn:localhost:proc").withoutScheme(), "localhost:proc");
}

BOOST_AUTO_TEST_CASE(Encode) {
	BOOST_CHECK_EQUAL(Url::Encoded("Hello World!"), "Hello%20World!");
	BOOST_CHECK_EQUAL(Url::Encoded("https://example.com/search?q=Hello World!&lang=en"),
	                  "https://example.com/search?q=Hello%20World!&lang=en");
}

BOOST_AUTO_TEST_CASE(Decode) {
	BOOST_CHECK_EQUAL(Url::Decoded("Hello%20World!"), "Hello World!");
	BOOST_CHECK_EQUAL(Url::Decoded("https://exam%3Aple.com/sear%23ch"),
	                  "https://exam:ple.com/sear#ch");
	BOOST_CHECK_EQUAL(Url::Decoded("https://example.com/search?q=Hello%20World%21&lang%3Den"),
	                  "https://example.com/search?q=Hello World!&lang=en");
}


BOOST_AUTO_TEST_SUITE_END()
