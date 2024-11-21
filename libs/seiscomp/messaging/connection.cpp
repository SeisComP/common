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


#define SEISCOMP_COMPONENT Messaging

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/messaging/connection.h>
#include <seiscomp/messaging/status.h>
#include <seiscomp/utils/url.h>
#include <seiscomp/system/hostinfo.h>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include <iomanip>


using namespace std;
namespace bio = boost::iostreams;


namespace Seiscomp {
namespace Client {
namespace {


System::HostInfo HostInfo;
mutex Mutex;
Connection::Connections Pool;
Util::Timer Timer;


void Timeout() {
	static string ConstantInfo =
		string(Status::Tag(Status::Hostname).toString()) + "=" + HostInfo.name() + "&" +
		string(Status::Tag(Status::Programname).toString()) + "=" + HostInfo.programName() + "&" +
		string(Status::Tag(Status::PID).toString()) + "=" + Core::toString(HostInfo.pid()) + "&" +
		string(Status::Tag(Status::TotalMemory).toString()) + "=" + Core::toString(HostInfo.totalMemory()) + "&";

	lock_guard<mutex> l(Mutex);

	Core::Time timestamp = Core::Time::UTC();

	// Send state-of-health messages
	for ( auto &&connection : Pool ) {
		if ( !connection->protocol() ) continue;

		double usedCPU = floor(HostInfo.getCurrentCpuUsage() * 1E4) * 1E-4;
		if ( usedCPU < 0 ) usedCPU = 0;

		string content;
		{
			bio::stream_buffer<boost::iostreams::back_insert_device<string> > buf(content);
			ostream os(&buf);

			os << ConstantInfo
			   << Status::Tag(Status::Time).toString() << "=" << timestamp.iso() << "&"
			   << Status::Tag(Status::Clientname).toString() << "=" << connection->clientName() << "&"
			   << Status::Tag(Status::CPUUsage).toString() << "=" << fixed << setprecision(3) << usedCPU << "&"
			   << Status::Tag(Status::ClientMemoryUsage).toString() << "=" << HostInfo.getCurrentMemoryUsage() << "&"
			   << Status::Tag(Status::MessageQueueSize).toString() << "=" << connection->inboxSize() << "&"
			   << Status::Tag(Status::ObjectCount).toString() << "=" << Core::BaseObject::ObjectCount();

			const Protocol::State *state = connection->state();
			if ( state ) {
				os << "&"
				   << Status::Tag(Status::SentMessages).toString() << "=" << state->sentMessages << "&"
				   << Status::Tag(Status::SentBytes).toString() << "=" << state->bytesSent << "&"
				   << Status::Tag(Status::ReceivedMessages).toString() << "=" << state->receivedMessages << "&"
				   << Status::Tag(Status::ReceivedBytes).toString() << "=" << state->bytesReceived;
			}

			connection->getInfo(timestamp, os);
		}

		Result r;
		r = connection->protocol()->sendData(Protocol::STATUS_GROUP,
		                                     content.c_str(), content.size(),
		                                     Protocol::Status,
		                                     Protocol::ContentEncoding(Protocol::Identity),
		                                     Protocol::ContentType(Protocol::Text));
		if ( !r && r.code() != NotConnected )
			SEISCOMP_ERROR("Failed to send status message to %s: %d: %s",
			               Protocol::STATUS_GROUP.c_str(),
			               r.toInt(), r.toString());
	}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Connection::Connection(Protocol *proto)
: _protocol(proto)
, _defaultContentEncoding(Protocol::Deflate)
, _defaultContentType(Protocol::Binary)
, _timeoutMs(0) {
	registerConnection(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Connection::~Connection() {
	unregisterConnection(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::setContentEncoding(Protocol::ContentEncoding enc) {
	_defaultContentEncoding = enc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::setContentType(Protocol::ContentType type) {
	_defaultContentType = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::setSource(const char *URL) {
	Util::Url url(URL);
	if ( !url )
		return InvalidURL;

	string protoType = url.scheme();
	if ( protoType.empty() && !_protocol )
		protoType = "scmp";

	if ( !protoType.empty() ) {
		ProtocolPtr proto = ProtocolFactory::Create(protoType.c_str());
		if ( !proto ) {
			SEISCOMP_ERROR("Failed to create protocol '%s'", url.scheme().c_str());
			return InvalidProtocol;
		}

		_protocol = proto;
	}

	_address = url.withoutScheme();

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::setSource(const string &URL) {
	return setSource(URL.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::setMembershipInfo(bool enable) {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_protocol->setMembershipInfo(enable);
	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::connect(const string &clientName,
                           const string &primaryGroup,
                           unsigned int timeoutMs) {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_clientName = clientName;
	_primaryGroup = primaryGroup;
	_timeoutMs = timeoutMs;
	return reconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::disconnect() {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_lastError = _protocol->disconnect();
	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Connection::isConnected() const {
	if ( !_protocol ) return false;
	return _protocol->isConnected();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::reconnect() {
	if ( isConnected() ) {
		_lastError = disconnect();
		if ( _lastError != OK )
			return _lastError;
	}

	if ( !_protocol ) return InvalidProtocol;

	_lastError = _protocol->connect(
		_address.c_str(), _timeoutMs,
		_clientName.empty()
		?
		nullptr
		:
		_clientName.c_str()
	);

	if ( _lastError ) {
		if ( !_primaryGroup.empty()
		  && _protocol->groups().find(_primaryGroup) == _protocol->groups().end() ) {
			_protocol->disconnect();
			_lastError = GroupDoesNotExist;
		}
	}

	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::close() {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_lastError = _protocol->close();
	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::setTimeout(int milliseconds) {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_lastError = _protocol->setTimeout(milliseconds);
	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::subscribe(const char *group) {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_lastError = _protocol->subscribe(group);
	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::subscribe(const string &group) {
	return subscribe(group.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::unsubscribe(const char *group) {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_lastError = _protocol->unsubscribe(group);
	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::unsubscribe(const string &group) {
	return unsubscribe(group.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::fetchInbox() {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	return _lastError = _protocol->fetchInbox();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::syncOutbox() {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	return _lastError = _protocol->syncOutbox();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Message *Connection::recv(Packet **packet, Result *status) {
	if ( packet ) *packet = nullptr;

	if ( !_protocol ) {
		_lastError = InvalidProtocol;
		if ( status ) *status = _lastError;
		return nullptr;
	}

	while ( true ) {
		Packet *p = _protocol->recv(&_lastError);
		if ( !p ) {
			if ( status ) *status = _lastError;
			return nullptr;
		}

		if ( p->type != Packet::Data ) {
			if ( packet ) {
				_lastError = OK;
				*packet = p;
				if ( status ) *status = _lastError;
				return nullptr;
			}
			else
				delete p;

			// Skip that message and read the next one
			continue;
		}

		Core::Message *msg = nullptr;

		Protocol::ContentEncoding ce(Protocol::Identity);
		if ( p->headerContentEncoding.empty()
		  || ce.fromString(p->headerContentEncoding) ) {
			Protocol::ContentType ct;
			if ( !ct.fromString(p->headerContentType) )
				_lastError = ContentTypeUnknown;
			else
				msg = Protocol::decode(p->payload, ce, ct);
		}
		else
			_lastError = ContentEncodingUnknown;

		if ( packet ) {
			*packet = p;
			if ( !msg )
				_lastError = DecodingError;
			else
				_lastError = OK;
			if ( status ) *status = _lastError;
			return msg;
		}
		else {
			delete p;

			if ( msg ) {
				if ( status ) *status = _lastError;
				return msg;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Message *Connection::recv(PacketPtr &packet, Result *status) {
	Packet *tmp;
	Core::Message *msg = recv(&tmp, status);
	packet = tmp;
	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::sendMessage(const Core::Message *msg) {
	return sendMessage(_primaryGroup, msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::sendMessage(const string &targetGroup, const Core::Message *msg) {
	if ( !_protocol ) return _lastError = InvalidProtocol;
	_lastError = _protocol->sendMessage(targetGroup, msg,
	                                    Protocol::Regular,
	                                    _defaultContentEncoding,
	                                    _defaultContentType);
	return _lastError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string Connection::lastErrorMessage() const {
	static string NoProtocol = "Protocol not set";
	if ( !_protocol ) return NoProtocol;
	return _protocol->lastErrorMessage();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Connection::clientName() const {
	static string EmptyClientName;
	if ( !_protocol ) return EmptyClientName;
	return _protocol->clientName();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Version Connection::schemaVersion() const {
	if ( !_protocol ) return Core::Version();
	return _protocol->schemaVersion();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Protocol::KeyValueStore *Connection::extendedParameters() const {
	if ( !_protocol ) return nullptr;
	return &_protocol->extendedParameters();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Connection::inboxSize() const {
	if ( !_protocol ) return 0;
	return _protocol->inboxSize();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Protocol::State *Connection::state() const {
	if ( !_protocol ) return nullptr;
	return &_protocol->state();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol *Connection::protocol() const {
	return _protocol.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::setInfoCallback(InfoCallback icb) {
	_infoCallback = icb;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::getInfo(const Core::Time &timestamp, ostream &os) {
	if ( !_infoCallback ) return;
	_infoCallback(timestamp, os);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::registerConnection(Connection *con) {
	lock_guard<mutex> l(Mutex);
	con->_poolIterator = Pool.insert(Pool.end(), con);

	if ( !Timer.isActive() ) {
		// Send SOH every 12 seconds
		Timer.setTimeout(12);
		Timer.setCallback(Timeout);
		HostInfo.getCurrentCpuUsage(true);
		Timer.start();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Connection::unregisterConnection(Connection *con) {
	lock_guard<mutex> l(Mutex);
	if ( con->_poolIterator != Pool.end() ) {
		Pool.erase(con->_poolIterator);
		con->_poolIterator = Connections::iterator();

		if ( Pool.empty() && Timer.isActive() ) {
			Timer.stop();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Connection::setCertificate(const string &cert) {
	if ( !_protocol ) return _lastError = InvalidProtocol;

	_protocol->setCertificate(cert);
	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
