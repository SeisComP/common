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


#include <seiscomp/core/optional.h>
#include <seiscomp/utils/url.h>



namespace std {


ostream &operator<<(ostream &os, const nullopt_t &) {
	os << "--";
	return os;
}

template <typename T>
ostream &operator<<(ostream &os, const optional<T> &value) {
	if ( value ) {
		os << "--";
	}
	else {
		os << *value;
	}
	return os;
}


}


#include <seiscomp/unittest/unittests.h>


using namespace Seiscomp::Util;


BOOST_AUTO_TEST_SUITE(seiscomp_utils_url)


BOOST_AUTO_TEST_CASE(urls) {
	Url url;

	BOOST_REQUIRE(url.setUrl("slink://localhost:18000"));
	BOOST_CHECK_EQUAL(url.toString(), "slink://localhost:18000");
	BOOST_CHECK_EQUAL(static_cast<bool>(url), true);
	BOOST_CHECK_EQUAL(url.isValid(), true);
	BOOST_CHECK_EQUAL(url.scheme(), "slink");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18000);
	BOOST_CHECK(url.path().empty());

	BOOST_REQUIRE(url.setUrl("test.mseed"));
	BOOST_CHECK_EQUAL(url.toString(), "test.mseed");
	BOOST_CHECK_EQUAL(url.isValid(), true);
	BOOST_CHECK_EQUAL(url.scheme(), "");
	BOOST_CHECK_EQUAL(url.host(), "test.mseed");
	BOOST_CHECK_EQUAL(url.port(), Seiscomp::Core::None);
	BOOST_CHECK_EQUAL(url.path(), "");

	BOOST_REQUIRE(url.setUrl("http://username:pass:word@example.org/"));
	BOOST_CHECK_EQUAL(url.toString(), "http://username:pass:word@example.org/");
	BOOST_CHECK_EQUAL(url.isValid(), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "username");
	BOOST_CHECK_EQUAL(url.password(), "pass:word");
	BOOST_CHECK_EQUAL(url.host(), "example.org");
	BOOST_CHECK_EQUAL(*url.port(), 80);
	BOOST_CHECK_EQUAL(url.path(), "/");

	BOOST_REQUIRE(url.setUrl("http://example.org/@foo"));
	BOOST_CHECK_EQUAL(url.toString(), "http://example.org/@foo");
	BOOST_CHECK_EQUAL(url.isValid(), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "example.org");
	BOOST_CHECK_EQUAL(*url.port(), 80);
	BOOST_CHECK_EQUAL(url.path(), "/@foo");

	BOOST_REQUIRE(url.setUrl("fdsnws://localhost:8080/fdsnws/1/station?max-dist=12&cha=BHZ#help"));
	BOOST_CHECK_EQUAL(url.toString(), "fdsnws://localhost:8080/fdsnws/1/station?max-dist=12&cha=BHZ#help");
	BOOST_CHECK_EQUAL(url.isValid(), true);
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
	BOOST_CHECK_EQUAL(url.toString(), "http://localhost:18100/testing");
	BOOST_CHECK_EQUAL(url.isValid(), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");

	BOOST_REQUIRE(url.setUrl("http://localhost:18100/testing?ack-window=30"));
	BOOST_CHECK_EQUAL(url.toString(), "http://localhost:18100/testing?ack-window=30");
	BOOST_CHECK_EQUAL(url.isValid(), true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(*url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");
	BOOST_CHECK(url.hasQueryItem("ack-window"));
	BOOST_CHECK_EQUAL(url.queryItemValue("ack-window"), "30");

	BOOST_REQUIRE(url.setUrl("http://localhost:18100/testing?ack-window=30&ssl=true"));
	BOOST_CHECK_EQUAL(url.toString(), "http://localhost:18100/testing?ack-window=30&ssl=true");
	BOOST_CHECK_EQUAL(url.isValid(), true);
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
	BOOST_CHECK_EQUAL(url.toString(), "http://[::1]:18100/testing?ack-window=30&ssl=true");
	BOOST_CHECK_EQUAL(url.isValid(), true);
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

	BOOST_REQUIRE(url.setUrl("host:3306/database"));
	BOOST_CHECK_EQUAL(url.host(), "host");
	BOOST_CHECK_EQUAL(url.port(), 3306);
	BOOST_CHECK_EQUAL(url.path(), "/database");

	BOOST_REQUIRE(url.setUrl("https://example.com/search?Hello%20World%21%26lang%3Den"));
	BOOST_REQUIRE(url.setUrl("ftp://ftp.is.co.za/rfc/rfc1808.txt"));
	BOOST_REQUIRE(url.setUrl("http://www.ietf.org/rfc/rfc2396.txt"));
	BOOST_REQUIRE(url.setUrl("ldap://[2001:db8::7]/c=GB?objectClass?one"));
	BOOST_REQUIRE(url.setUrl("telnet://192.0.2.16:80/"));

	BOOST_REQUIRE(url.setUrl("postgresql://sysop:sysop@127.0.0.1:5432/test?debug"));
	BOOST_CHECK_EQUAL(url.scheme(), "postgresql");
	BOOST_CHECK_EQUAL(url.username(), "sysop");
	BOOST_CHECK_EQUAL(url.password(), "sysop");
	BOOST_CHECK_EQUAL(url.host(), "127.0.0.1");
	BOOST_CHECK_EQUAL(url.path(), "/test");
	BOOST_CHECK_EQUAL(url.hasQueryItem("debug"), true);
	BOOST_CHECK_EQUAL(url.queryItems().size(), 1);

	BOOST_REQUIRE(url.setUrl("https://sys%2Fop:sys%2Fop@exam%3Aple.com/sear%23ch?q=Hello%20World%21&lang%3Den"));
	BOOST_CHECK_EQUAL(url.scheme(), "https");
	BOOST_CHECK_EQUAL(url.username(), "sys/op");
	BOOST_CHECK_EQUAL(url.password(), "sys/op");
	BOOST_CHECK_EQUAL(url.host(), "exam:ple.com");
	BOOST_CHECK_EQUAL(url.path(), "/sear#ch");
}

BOOST_AUTO_TEST_CASE(Queries) {
	Url url;
	BOOST_REQUIRE(url.setUrl("postgresql://sysop:sysop@127.0.0.1:5432/test?debug"));
	BOOST_CHECK_EQUAL(url.hasQueryItem("debug"), true);
	BOOST_CHECK_EQUAL(url.queryItems().size(), 1);

	BOOST_REQUIRE(url.setUrl("postgresql://sysop:sysop@127.0.0.1:5432/test?debug="));
	BOOST_CHECK_EQUAL(url.hasQueryItem("debug"), true);
	BOOST_CHECK_EQUAL(url.queryItems().size(), 1);

	BOOST_CHECK(!url.setUrl("postgresql://sysop:sysop@127.0.0.1:5432/test?=true"));

	BOOST_REQUIRE(url.setUrl("postgresql://sysop:sysop@127.0.0.1:5432/test?debug&&verbose=false"));
	BOOST_CHECK_EQUAL(url.hasQueryItem("debug"), true);
	BOOST_CHECK_EQUAL(url.queryItemValue("verbose"), "false");
	BOOST_CHECK_EQUAL(url.queryItems().size(), 2);
}

BOOST_AUTO_TEST_CASE(Extract) {
	BOOST_CHECK_EQUAL(Url("http://localhost").withoutScheme(), "localhost");
	BOOST_CHECK_EQUAL(Url("loc:/localhost").withoutScheme(), "loc:/localhost");
	BOOST_CHECK_EQUAL(Url("urn:localhost").withoutScheme(), "urn:localhost");
	BOOST_CHECK_EQUAL(Url("urn:localhost:proc").withoutScheme(), "urn:localhost:proc");
}

BOOST_AUTO_TEST_CASE(encode) {
	BOOST_CHECK_EQUAL(
		Url("http", {}, {}, "localhost", {}, "index.html", {}, {}).encode().toString(),
		"http://localhost/index.html"
	);
	BOOST_CHECK_NE(
		Url("http", {}, {}, "localhost", {}, "index.html", "anker", {
			{ "page", "4" },
			{ "limit", "20" },
		}).encode().toString(),
		"http://localhost/index.html#anker?page=4&limit=20"
	);
	BOOST_CHECK_EQUAL(
		Url("http", {}, {}, "localhost", {}, "index.html", "anker", {
			{ "page", "4" },
			{ "limit", "20" },
		}).encode(),
		"http://localhost/index.html#anker?limit=20&page=4"
	);
	BOOST_CHECK_EQUAL(
		Url("my scheme", {}, {}, "my computer", {}, "&path%", "frag ment", {
			{ "&page", "4%" },
			{ "&limit", "20%" },
		}).encode(),
		"my%20scheme://my%20computer/%26path%25#frag%20ment?%26limit=20%25&%26page=4%25"
	);
}

BOOST_AUTO_TEST_CASE(Encode) {
	BOOST_CHECK_EQUAL(Url::Encode("Hello World!"), "Hello%20World!");
	BOOST_CHECK_EQUAL(Url::Encode("https://example.com/search?q=Hello World!&lang=en"),
	                  "https://example.com/search?q=Hello%20World!&lang=en");
	BOOST_CHECK_EQUAL(Url::Encode("Hello World & Others!"), "Hello%20World%20&%20Others!");
}

BOOST_AUTO_TEST_CASE(EncodeComponent) {
	BOOST_CHECK_EQUAL(Url::EncodeComponent("Hello World!"), "Hello%20World!");
	BOOST_CHECK_EQUAL(Url::EncodeComponent("https://example.com/search?q=Hello World!&lang=en"),
	                  "https%3A%2F%2Fexample.com%2Fsearch%3Fq%3DHello%20World!%26lang%3Den");
	BOOST_CHECK_EQUAL(Url::EncodeComponent("Hello World & Others!"), "Hello%20World%20%26%20Others!");
}

BOOST_AUTO_TEST_CASE(Decode) {
	BOOST_CHECK_EQUAL(Url::Decode("Hello%20World!"), "Hello World!");
	BOOST_CHECK_EQUAL(Url::Decode("https://exam%3Aple.com/sear%23ch"),
	                  "https://exam:ple.com/sear#ch");
	BOOST_CHECK_EQUAL(Url::Decode("https://example.com/search?q=Hello%20World%21&lang%3Den"),
	                  "https://example.com/search?q=Hello World!&lang=en");
	BOOST_CHECK_EQUAL(Url::Decode("https%3A%2F%2Fexample.com%2Fsearch%3Fq%3DHello%20World!%26lang%3Den"),
	                  "https://example.com/search?q=Hello World!&lang=en");
}

BOOST_AUTO_TEST_SUITE_END()
