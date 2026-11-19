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


#include <seiscomp/core/datamessage.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/messaging/connection.h>
#include <seiscomp/unittest/unittests.h>

#include <cstdio>
#include <cstdlib>
#include <thread>
#include <future>


using namespace Seiscomp;


struct scmaster {
	scmaster() {
		std::cerr << "starting scmaster\n";
		setenv("LD_LIBRARY_PATH", LIB_DIR, 1);
		URL = "localhost:18189/test";
		_pid = fork();
		BOOST_REQUIRE(_pid >= 0);
		if ( _pid == 0 ) {
			// Child process
			const char *args[] = {
				OUTPUT_DIR "/scmaster",
				"--config-file",
				SOURCE_DIR "/scmaster.cfg",
				"--console",
				"1",
				nullptr
			};

			execvp(args[0], (char *const *)args);
			_exit(202); // ERROR_CHILD_EXEC
		}

		sleep(2);
	}

	~scmaster() {
		std::cerr << "teardown scmaster\n";
		int status = 0;
		kill(_pid, SIGTERM);
		BOOST_CHECK_EQUAL(waitpid(_pid, &status, WUNTRACED | WCONTINUED), _pid);
	}

	std::string URL;
	size_t messageCount{10000};

	private:
		pid_t _pid;
};


BOOST_AUTO_TEST_SUITE(seiscomp_messaging_messages)


BOOST_FIXTURE_TEST_CASE(SendAndReceive, scmaster) {
	Client::ConnectionPtr c1 = new Client::Connection;
	Client::ConnectionPtr c2 = new Client::Connection;
	Client::ConnectionPtr c3 = new Client::Connection;
	BOOST_REQUIRE_EQUAL(c1->setSource(URL), Client::OK);
	BOOST_REQUIRE_EQUAL(c2->setSource(URL), Client::OK);
	BOOST_REQUIRE_EQUAL(c3->setSource(URL), Client::OK);
	BOOST_REQUIRE_EQUAL(c1->connect("flood", "IMPORT_GROUP"), Client::OK);
	BOOST_REQUIRE_EQUAL(c2->connect("read_and_flood", "PICK"), Client::OK);
	BOOST_REQUIRE_EQUAL(c2->subscribe("IMPORT_GROUP"), Client::OK);
	BOOST_REQUIRE_EQUAL(c3->connect("read", Client::Protocol::LISTENER_GROUP), Client::OK);
	BOOST_REQUIRE_EQUAL(c3->subscribe("PICK"), Client::OK);

	auto futSender = std::async([this, c1]() {
		for ( size_t i = 0; i < messageCount; ++i ) {
			Core::DataMessagePtr msg = new Core::DataMessage;
			msg->attach(new DataModel::Pick(Core::toString(i)));
			c1->send(msg.get());
			// usleep(rand() % 2000 + 1000);
		}

		c1->syncOutbox();
	});

	auto futSenderAndReceiver = std::async([this, c2]() {
		auto futReceiver = std::async([this, c2]() {
			size_t count = 0;
			while ( count < messageCount ) {
				auto *msg = c2->recv();
				if ( msg ) {
					auto *dmsg = Core::DataMessage::Cast(msg);
					if ( dmsg ) {
						BOOST_CHECK_EQUAL(dmsg->size(), 1);
						BOOST_CHECK_EQUAL(DataModel::Pick::Cast(*dmsg->begin())->publicID(), Core::toString(count));
						++count;
					}

					delete msg;
				}
			}
		});

		for ( size_t i = 0; i < messageCount; ++i ) {
			Core::DataMessagePtr msg = new Core::DataMessage;
			msg->attach(new DataModel::Pick(Core::toString(1000000 + i)));
			c2->send(msg.get());
			usleep(rand() % 2000 + 1000);
		}

		BOOST_REQUIRE(futReceiver.wait_for(std::chrono::seconds(5)) != std::future_status::timeout);

		c2->syncOutbox();
	});

	auto futReceiver = std::async([this, c3]() {
		size_t count = 0;
		while ( count < messageCount ) {
			auto *msg = c3->recv();
			if ( msg ) {
				auto *dmsg = Core::DataMessage::Cast(msg);
				if ( dmsg ) {
					BOOST_CHECK_EQUAL(dmsg->size(), 1);
					BOOST_CHECK_EQUAL(DataModel::Pick::Cast(*dmsg->begin())->publicID(), Core::toString(1000000 + count));
					++count;
				}

				delete msg;
			}
		}
	});

	BOOST_REQUIRE(futSender.wait_for(std::chrono::seconds(60)) != std::future_status::timeout);
	BOOST_REQUIRE(futSenderAndReceiver.wait_for(std::chrono::seconds(60)) != std::future_status::timeout);
	BOOST_REQUIRE(futReceiver.wait_for(std::chrono::seconds(10)) != std::future_status::timeout);

	BOOST_CHECK_EQUAL(c1->disconnect(), Client::OK);
	BOOST_CHECK_EQUAL(c2->disconnect(), Client::OK);
	BOOST_CHECK_EQUAL(c3->disconnect(), Client::OK);
}


BOOST_AUTO_TEST_SUITE_END()
