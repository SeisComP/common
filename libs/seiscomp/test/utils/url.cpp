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


#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/utils/url.h>


using namespace Seiscomp::Util;


BOOST_AUTO_TEST_SUITE(seiscomp_utils_url)


BOOST_AUTO_TEST_CASE(urls) {
	Url url;

	BOOST_REQUIRE(url.setUrl("slink://localhost:18000"));
	BOOST_CHECK_EQUAL((const char*)url, "slink://localhost:18000");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "slink");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(url.port(), 18000);
	BOOST_CHECK_EQUAL(url.path(), "/");

	BOOST_REQUIRE(url.setUrl("test.mseed"));
	BOOST_CHECK_EQUAL((const char*)url, "test.mseed");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "");
	BOOST_CHECK_EQUAL(url.host(), "test.mseed");
	BOOST_CHECK_EQUAL(url.port(), -1);
	BOOST_CHECK_EQUAL(url.path(), "/");

	BOOST_REQUIRE(url.setUrl("http://username:pass:word@example.org/"));
	BOOST_CHECK_EQUAL((const char*)url, "http://username:pass:word@example.org/");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "username");
	BOOST_CHECK_EQUAL(url.password(), "pass:word");
	BOOST_CHECK_EQUAL(url.host(), "example.org");
	BOOST_CHECK_EQUAL(url.port(), -1);
	BOOST_CHECK_EQUAL(url.path(), "/");

	BOOST_REQUIRE(url.setUrl("http://example.org/@foo"));
	BOOST_CHECK_EQUAL((const char*)url, "http://example.org/@foo");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "http");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "example.org");
	BOOST_CHECK_EQUAL(url.port(), -1);
	BOOST_CHECK_EQUAL(url.path(), "/@foo");

	BOOST_REQUIRE(url.setUrl("fdsnws://localhost:8080/fdsnws/1/station?max-dist=12&cha=BHZ#help"));
	BOOST_CHECK_EQUAL((const char*)url, "fdsnws://localhost:8080/fdsnws/1/station?max-dist=12&cha=BHZ#help");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "fdsnws");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(url.port(), 8080);
	BOOST_CHECK_EQUAL(url.path(), "/fdsnws/1/station");
	BOOST_CHECK_EQUAL(url.query(), "max-dist=12&cha=BHZ");
	BOOST_CHECK_EQUAL(url.queryItemValue("max-dist"), "12");
	BOOST_CHECK_EQUAL(url.queryItemValue("cha"), "BHZ");
	BOOST_CHECK_EQUAL(url.fragment(), "help");

	BOOST_REQUIRE(url.setUrl("localhost:18100/testing"));
	BOOST_CHECK_EQUAL((const char*)url, "localhost:18100/testing");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");

	BOOST_REQUIRE(url.setUrl("localhost:18100/testing?ack-window=30"));
	BOOST_CHECK_EQUAL((const char*)url, "localhost:18100/testing?ack-window=30");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");
	BOOST_CHECK(url.hasQuery());
	BOOST_CHECK(url.hasQueryItem("ack-window"));
	BOOST_CHECK_EQUAL(url.queryItemValue("ack-window"), "30");

	BOOST_REQUIRE(url.setUrl("localhost:18100/testing?ack-window=30&ssl=true"));
	BOOST_CHECK_EQUAL((const char*)url, "localhost:18100/testing?ack-window=30&ssl=true");
	BOOST_CHECK_EQUAL((bool)url, true);
	BOOST_CHECK_EQUAL(url.scheme(), "");
	BOOST_CHECK_EQUAL(url.username(), "");
	BOOST_CHECK_EQUAL(url.password(), "");
	BOOST_CHECK_EQUAL(url.host(), "localhost");
	BOOST_CHECK_EQUAL(url.port(), 18100);
	BOOST_CHECK_EQUAL(url.path(), "/testing");
	BOOST_CHECK(url.hasQuery());
	BOOST_CHECK(url.hasQueryItem("ack-window"));
	BOOST_CHECK(url.hasQueryItem("ssl"));
	BOOST_CHECK_EQUAL(url.queryItemValue("ack-window"), "30");
	BOOST_CHECK_EQUAL(url.queryItemValue("ssl"), "true");
}


BOOST_AUTO_TEST_CASE(decode) {
	Url url;

	BOOST_CHECK_EQUAL(Url::Decoded("Hello%20World!"), "Hello World!");
	BOOST_CHECK_EQUAL(Url::Decoded("https://exam%3Aple.com/sear%23ch"),
	                  "https://exam:ple.com/sear#ch");
	BOOST_CHECK_EQUAL(Url::Decoded("https://example.com/search?q=Hello%20World%21&lang%3Den"),
	                  "https://example.com/search?q=Hello World!&lang=en");

	BOOST_REQUIRE(url.setUrl("https://sys%2Fop:sys%2Fop@exam%3Aple.com/sear%23ch?q=Hello%20World%21&lang%3Den"));
	BOOST_CHECK_EQUAL(url.scheme(), "https");
	BOOST_CHECK_EQUAL(url.username(), "sys/op");
	BOOST_CHECK_EQUAL(url.password(), "sys/op");
	BOOST_CHECK_EQUAL(url.host(), "exam:ple.com");
	BOOST_CHECK_EQUAL(url.path(), "/sear#ch");
}


BOOST_AUTO_TEST_SUITE_END()
