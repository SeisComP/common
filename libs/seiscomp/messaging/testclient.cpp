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


#include "protocols/websocket.h"

#include <seiscomp/utils/timer.h>
#include <boost/thread.hpp>

#include <cerrno>
#include <functional>
#include <iostream>
#include <deque>


using namespace std;
using namespace Seiscomp;



bool connectionPublishTest(int nMessages) {
	Messaging::Client::WebsocketConnection conn;
	conn.subscribe("PICK");
	//conn.subscribe("AMPLITUDE");

	conn.setMembershipInfo(true);
	conn.setAckWindow(20, true);
	//conn.setSSL(true);

	if ( !conn.connect("127.0.0.1", 8000, "testing") ) {
		cerr << "Failed to connect: " << conn.errorMessage() << endl;
		return false;
	}

	string content = "{"
	                   "\"Pick\":{"
	                     "\"time\":{"
	                       "\"value\":\"2018-01-01T12:00:00.123Z\""
	                     "},"
	                     "\"waveformID\":{"
	                       "\"networkCoce\":\"GE\","
	                       "\"stationCode\":\"MORC\","
	                       "\"channelCode\":\"BHZ\""
	                     "}"
	                   "}"
	                 "}";
	//content.append(1000, ' ');

	Messaging::Client::Message msg;
	int n = nMessages;
	bool keepReading = true;

	while ( n && keepReading ) {
		Messaging::Client::Connection::Result r;
		r = conn.send("PICK", content.data(), content.size(), "text/plain");
		switch ( r ) {
			case Messaging::Client::Connection::OK:
				--n;
				break;
			case Messaging::Client::Connection::Error:
				keepReading = false;
				break;
			default:
				break;
		}

		conn.recv(msg);
		conn.clearInbox();
	}

	if ( conn.erroneous() )
		std::cerr << "ERROR: " << conn.errorMessage() << std::endl;

	while ( conn.state().receivedMessages < conn.state().localSequenceNumber ) {
		if ( !conn.recv(msg) )
			break;
	}

	// Wait for all messages to be acknowledged
	conn.syncOutbox();

	std::cerr << conn.state().receivedMessages << " / " << conn.state().localSequenceNumber << std::endl;

	conn.disconnect();

	return true;
}


void readMessages(Messaging::Client::WebsocketConnection *conn) {
	std::cerr << "Starting reader" << std::endl;

	OPT(uint64_t) lastSeqNo;

	while ( true ) {
		Messaging::Client::MessagePtr msg = conn->recv();
		if ( msg == nullptr )
			break;

		if ( msg->type == Messaging::Client::Message::Regular ) {
			if ( lastSeqNo ) {
				if ( msg->seqNo - *lastSeqNo != 1 )
					std::cout << "********** Something wrong in ochestration: "
					          << msg->seqNo << " != " << *lastSeqNo << std::endl;
			}

			lastSeqNo = msg->seqNo;
		}
	}

	std::cerr << "Finished reader" << std::endl;
}


bool selfTest(int nMessages) {
	Messaging::Client::WebsocketConnection conn;
	conn.subscribe("PICK");
	//conn.subscribe("AMPLITUDE");

	conn.setMembershipInfo(true);
	conn.setAckWindow(20, true);
	//conn.setSSL(true);

	if ( !conn.connect("127.0.0.1", 8000, "testing") ) {
		cerr << "Failed to connect: " << conn.errorMessage() << endl;
		return false;
	}

	Seiscomp::Util::StopWatch timer;
	timer.restart();

	string content = "{"
	                   "\"Pick\":{"
	                     "\"time\":{"
	                       "\"value\":\"2018-01-01T12:00:00.123Z\""
	                     "},"
	                     "\"waveformID\":{"
	                       "\"networkCoce\":\"GE\","
	                       "\"stationCode\":\"MORC\","
	                       "\"channelCode\":\"BHZ\""
	                     "}"
	                   "}"
	                 "}";
	//content.append(1000, ' ');

	Messaging::Client::Message msg;
	int n = nMessages;
	bool keepReading = true;

	boost::thread readingThread(bind(readMessages, &conn));
	readingThread.yield();

	while ( n && keepReading ) {
		Messaging::Client::Connection::Result r;
		//std::cerr << "Sending" << std::endl;
		r = conn.send("PICK", content.data(), content.size(), "text/plain");
		//std::cerr << "Sent" << std::endl;
		switch ( r ) {
			case Messaging::Client::Connection::OK:
				--n;
				break;
			case Messaging::Client::Connection::Error:
				keepReading = false;
				break;
			default:
				std::cerr << "Error: " << r << std::endl;
				break;
		}
	}

	if ( conn.erroneous() )
		cerr << "ERROR: " << conn.errorMessage() << endl;

	// Wait for all messages to be acknowledged
	conn.syncOutbox();
	conn.disconnect();
	cerr << "Remaining outbox messages: " << conn.outboxSize() << endl;

	readingThread.join();

	double elapsed = timer.elapsed();

	cerr << "Sent " << nMessages << " messages, took " << elapsed << "s" << endl;
	cerr << "Average time per message: " << (elapsed*1000000/(nMessages)) << "us" << endl;
	cerr << "Average number of message per second: " << (nMessages/elapsed) << endl;

	cerr << conn.state().receivedMessages << " / " << conn.state().localSequenceNumber << endl;

	conn.close();

	return true;
}


void threadedConnectionTest() {
	Seiscomp::Util::StopWatch timer;
	timer.restart();

#define N 100000
#define N_THREADS 5

	boost::thread_group threads;
	for ( int i = 0; i < N_THREADS; ++i ) {
		threads.create_thread(bind(connectionPublishTest, N));
	}

	threads.join_all();
	//connectionPublishTest(N);

	double elapsed = timer.elapsed();

	cerr << "Sent " << (N*N_THREADS) << " messages, took " << elapsed << "s" << endl;
	cerr << "Average time per message: " << (elapsed*1000000/(N*N_THREADS)) << "us" << endl;
	cerr << "Average number of message per second: " << ((N*N_THREADS)/elapsed) << endl;
	/*
	cerr << "Bytes sent: " << conn.state().bytesSent << endl;
	cerr << "Send transfer rate: " << int(conn.state().bytesSent/elapsed) << " b/s" << endl;

	std::cerr << "PEAKB : " << conn.state().maxBufferedBytes << std::endl;
	std::cerr << "PEAKM : " << conn.state().maxBufferedMessages << std::endl;
	std::cerr << "OUTBOX: " << conn.outboxSize() << std::endl;
	std::cerr << "INBOX : " << conn.inboxSize() << std::endl;
	std::cerr << "OUT: " << conn.state().localSequenceNumber << std::endl;
	std::cerr << "IN : " << conn.state().receivedMessages << std::endl;
	if ( conn.state().sequenceNumber )
		std::cerr << "SEQ: " << *conn.state().sequenceNumber << std::endl;

	conn.disconnect();
	*/
}


int main(int argc, char **argv) {
	//threadedConnectionTest();
	selfTest(100000);
	return 0;
}
