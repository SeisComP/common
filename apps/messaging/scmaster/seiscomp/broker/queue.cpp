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


#define SEISCOMP_COMPONENT MASTER

#include "queue.h"
#include "group.h"
#include "client.h"
#include "message.h"
#include "messagedispatcher.h"

#include <seiscomp/broker/utils/utils.h>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/version.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/messaging/status.h>
#include <seiscomp/system/hostinfo.h>
#include <seiscomp/utils/base64.h>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include <stdio.h>
#include <iomanip>


#define MASTER_NAME "MASTER"

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
namespace bio = boost::iostreams;


namespace Seiscomp {
namespace Messaging {
namespace Broker {

namespace {

System::HostInfo HostInfo;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Queue(const std::string &name, uint64_t maxPayloadSize)
: _name(name)
, _processedMessageDispatcher(nullptr)
, _sequenceNumber(0)
, _messageProcessor(nullptr)
, _allocatedClientHeap(0)
, _sohInterval(12)
, _inactivityLimit(36)
, _maxPayloadSize(maxPayloadSize)
{
	// Queue the last 10000 messages
	_messages.reserve(10000);

	_tasks.resize(10);
	_results.resize(10);

	// Register MASTER name to block it
	_clients.insert(senderName(), nullptr);

	_created = Core::Time::GMT();

	// Add the required status group
	addGroup(StatusGroup);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::~Queue() {
	shutdown();

	for ( MessageProcessorPtr &proc : _messageProcessors ) {
		proc->_queue = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Queue::add(MessageProcessor *proc) {
	if ( proc->_queue )
		return false;

	_processors.push_back(proc);

	if ( proc->isMessageProcessingEnabled() )
		_messageProcessors.push_back(proc);

	if ( proc->isConnectionProcessingEnabled() )
		_connectionProcessors.push_back(proc);

	proc->_queue = this;
	proc->attach(this);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Result Queue::addGroup(const std::string &name) {
	// Groups does already exist
	if ( _groups.find(name) != _groups.end() )
		return GroupNameNotUnique;

	_groups[name] = new Group(name.c_str());
	_groupNames.push_back(name);
	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Queue::senderName() const {
	return MASTER_NAME;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::setMessageDispatcher(MessageDispatcher *dispatcher) {
	_processedMessageDispatcher = dispatcher;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Result Queue::push(Client *sender, Message *msg, int packetSize) {
	flushProcessedMessages();

	/*
	SEISCOMP_DEBUG("+ message from %s to %s/%s",
	               sender->name().c_str(), name().c_str(),
	               msg->target.c_str());
	*/

	// Check if the target exists
	Groups::iterator git = _groups.find(msg->target);
	if ( git == _groups.end() ) {
		Clients::iterator cit = _clients.find(msg->target);
		if ( cit == _clients.end() )
			return GroupDoesNotExist;
	}
	else {
		++git->second->_txMessages.received;
		git->second->_txBytes.received += packetSize;
		git->second->_txPayload.received += msg->payload.size();
	}

	++_txMessages.received;
	_txBytes.received += packetSize;
	_txPayload.received += msg->payload.size();

	// Tag the sender
	msg->sender = sender->_name;
	// Reset inactivity counter
	sender->_inactivityCounter = 0;

	switch ( msg->type ) {
		case Message::Type::Status:
		{
			sender->_lastSOHReceived = Core::Time::GMT();

			map<string, string> infoMap;

			{
				vector<string> nameValuePairs;
				Core::split(nameValuePairs, msg->payload, "&");
				for ( auto &&item : nameValuePairs ) {
					size_t p = item.find('=');
					if ( p == string::npos )
						infoMap[item] = string();
					else
						infoMap[item.substr(0, p)] = item.substr(p+1);
				}
			}

			infoMap["uptime"] = Core::toString(floor(double(sender->_lastSOHReceived - sender->created())));
			infoMap["address"] = toString(sender->IPAddress());

			msg->payload = string();
			for ( auto &&it : infoMap ) {
				if ( !msg->payload.empty() )
					msg->payload += "&";
				msg->payload += it.first;
				if ( !it.second.empty() ) {
					msg->payload += '=';
					msg->payload += it.second;
				}
			}

			msg->selfDiscard = false;

			break;
		}

		default:
			break;
	}

	if ( (msg->type >= Message::Type::Transient)
	  || _messageProcessors.empty() ) {
		// 1. Transient messages are published without processing, they bypass
		//    the queue and all sequence numbers.
		// 2. If no processors are configured then processing must not be
		//    scheduled.
		publish(sender, msg);
	}
	else
		_tasks.push(ProcessingTask(sender, msg));

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Queue::publish(Client *sender, Message *msg) {
	msg->timestamp = Seiscomp::Core::Time::GMT();

	// Save message in smart pointer to prevent memory leaks as it might
	// bypass the ring buffer (transient, service).
	MessagePtr guard(msg);

	if ( msg->type == Message::Type::Regular ) {
		++_sequenceNumber;
		msg->sequenceNumber = _sequenceNumber;
		_messages.push_back(msg);
	}

	//NOTIFY(0, publish, sender, msg);

	if ( sender ) {
		// Increase the senders sequence number
		++sender->_sequenceNumber;

		if ( sender->_acknowledgeCounter > 0 ) {
			--sender->_acknowledgeCounter;
			if ( sender->_acknowledgeCounter == 0 ) {
				sender->_acknowledgeCounter = sender->_acknowledgeWindow;
				sender->ack();
				sender->_ackInitiated = Core::Time();
			}
			else if ( !sender->_ackInitiated )
				sender->_ackInitiated = msg->timestamp;
		}
	}

	auto git = _groups.find(msg->target);
	if ( git == _groups.end() ) {
		// Peer to peer
		auto cit = _clients.find(msg->target);
		if ( cit == _clients.end() )
			return false;

		cit.value()->publish(sender, msg);

		++_txMessages.sent;
		_txPayload.sent += msg->payload.size();
	}
	else {
		// Distribute to members
		auto group = git->second.get();
		msg->_internalGroupPtr = group;

		for ( auto client : group->_members ) {
			client->publish(sender, msg);
			// Each message sent to a member of a particular group is tagged
			// as sent.
			++git->second->_txMessages.sent;
			git->second->_txBytes.sent += msg->payload.size();

			++_txMessages.sent;
			_txPayload.sent += msg->payload.size();
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Result Queue::subscribe(Client *client, const std::string &groupName) {
	Groups::iterator it = _groups.find(groupName);
	if ( it == _groups.end() )
		// GROUP NOT FOUND
		return GroupDoesNotExist;

	Group *group = it->second.get();

	if ( !group->addMember(client) )
		return GroupAlreadySubscribed;

	Message msg;

	msg.sender = senderName();
	msg.target = group->name();
	msg.timestamp = Seiscomp::Core::Time::GMT();

	client->enter(group, client, &msg);

	for ( auto member : group->_members ) {
		if ( member ->wantsMembershipInformation() && client != member ) {
			member ->enter(group, member , &msg);
		}
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Result Queue::unsubscribe(Client *client, const std::string &groupName) {
	Groups::iterator it = _groups.find(groupName);
	if ( it == _groups.end() )
		// GROUP NOT FOUND
		return GroupDoesNotExist;

	Group *group = it->second.get();

	if ( !group->removeMember(client) )
		return GroupNotSubscribed;

	Message msg;

	msg.sender = senderName();
	msg.target = group->name();
	msg.timestamp = Seiscomp::Core::Time::GMT();

	client->leave(group, client, &msg);

	for ( auto member : group->_members ) {
		if ( member->wantsMembershipInformation() && client != member ) {
			member->leave(group, client, &msg);
		}
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Message *Queue::getMessage(SequenceNumber sequenceNumber,
                           const Client *client) const {
	SequenceNumber firstSeqNo, lastSeqNo, idx;

	if ( _messages.empty() )
		return nullptr;

	firstSeqNo = _messages.front()->sequenceNumber;
	lastSeqNo = _messages.back()->sequenceNumber;

	if ( firstSeqNo > lastSeqNo ) {
		if ( (sequenceNumber < firstSeqNo) &&
			 (sequenceNumber > lastSeqNo) )
			sequenceNumber = firstSeqNo;
	}
	else {
		if ( sequenceNumber < firstSeqNo )
			sequenceNumber = firstSeqNo;

		if ( sequenceNumber > lastSeqNo )
			return nullptr;
	}

	idx = sequenceNumber-firstSeqNo;
	while ( idx < _messages.size() ) {
		Message *msg = _messages[idx].get();
		// If the messages target group has client as member, return it
		if ( msg->_internalGroupPtr->hasMember(client) ) {
			// Update statistics
			++msg->_internalGroupPtr->_txMessages.sent;
			msg->_internalGroupPtr->_txBytes.sent += msg->payload.size();

			++_txMessages.sent;
			_txBytes.sent += msg->payload.size();
			return msg;
		}
		// If the message is a private message for client, return it
		if ( msg->target == client->name() ) {
			++_txMessages.sent;
			_txBytes.sent += msg->payload.size();
			return msg;
		}
		++idx;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Result Queue::connect(Client *client,
                             const KeyCStrValues inParams, int inParamCount,
                             KeyValues &outParams) {
	if ( client->_name.empty() ) {
		int numTests = 10;
		while ( numTests-- ) {
			uint32_t id;
			if ( Utils::Randomizer::Instance().fill(id) ) {
				toHex(client->_name, id);
			}
			else {
				SEISCOMP_ERROR("Failed to read from random device");
				return InternalError;
			}

			if ( !_clients.contains(client->_name)
			  && _groups.find(client->_name) == _groups.end() )
				break;

			client->_name.clear();
		}

		if ( client->_name.empty() ) {
			// No free random name found
			return ClientNameNotUnique;
		}
	}

	if ( _clients.contains(client->_name) ) {
		SEISCOMP_ERROR("Client name '%s' not unique", client->_name.c_str());
		return ClientNameNotUnique;
	}

	if ( _groups.find(client->_name) != _groups.end() ) {
		SEISCOMP_ERROR("Client name '%s' not unique: taken by a group",
		               client->_name.c_str());
		return ClientNameNotUnique;
	}

	if ( !_connectionProcessors.empty() ) {
		for ( auto proc : _connectionProcessors ) {
			if ( !proc->acceptConnection(client, inParams, inParamCount, outParams) ) {
				return ClientNotAccepted;
			}
		}
	}

	client->_created = Core::Time::GMT();
	_clients.insert(client->_name.c_str(), client);
	client->_queue = this;
	SEISCOMP_DEBUG("Connect client '%s' to '%s'" ,
	               client->_name.c_str(), _name.c_str());

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Queue::Result Queue::disconnect(Client *client) {
	Seiscomp::Core::Time now = Seiscomp::Core::Time::GMT();
	for ( auto group : _groups ) {
		if ( !group.second->removeMember(client) ) continue;

		// Notify all remaining clients about the membership change
		Message msg;

		for ( auto member : group.second->_members ) {
			if ( member->wantsMembershipInformation() ) {
				if ( msg.sender.empty() ) {
					msg.sender = senderName();
					msg.target = group.second->name();
					msg.timestamp = now;
				}

				member->leave(group.second.get(), client, &msg);
			}
		}
	}

	if ( !_connectionProcessors.empty() ) {
		// Notify all client processor about the disconnect
		MessageProcessors::iterator cit;
		for ( cit = _connectionProcessors.begin(); cit != _connectionProcessors.end(); ++cit )
			(*cit)->dropConnection(client);
	}

	_clients.erase(_clients.find(client->_name.c_str()));
	client->_queue = nullptr;

	SEISCOMP_DEBUG("Disconnect client '%s'" , client->_name.c_str());

	{
		Message msg;

		for ( auto cit = _clients.begin(); cit != _clients.end(); ++cit ) {
			if ( cit.value() && cit.value()->wantsMembershipInformation() ) {
				if ( msg.sender.empty() ) {
					msg.sender = senderName();
					msg.timestamp = now;
				}

				cit.value()->disconnected(client, &msg);
			}
		}
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::flushProcessedMessages() {
	if ( !_processedMessageDispatcher ) return;

	ProcessingTask result;

	try {
		while ( _results.pop(result) ) {
			Clients::iterator cit = _clients.find(result.second->sender);
			if ( cit == _clients.end() )
				// Client not registered anymore, clear it
				result.first = nullptr;
			else if ( cit.value() != result.first )
				// It is another instance due to reconnect, clear it
				result.first = nullptr;
			publish(result.first, result.second);
		}
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("[queue] Flush: %s", e.what());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::returnToSender(Message *msg, BaseObject *obj) {
	// Send back to sender
	msg->target = msg->sender;
	msg->sender = senderName();
	msg->object = obj;

	// Reset other attributes
	msg->encodingWebSocket = nullptr;
	msg->_internalGroupPtr = nullptr;

	// Disable self discarding as the sender has changed
	msg->selfDiscard = false;

	msg->encode();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::processingLoop() {
	ProcessingTask task;

	SEISCOMP_DEBUG("[queue] worker is running");

	try {

	while ( true ) {
		task = _tasks.pop();
		process(task);
		taskReady(task);
	}

	}
	catch ( std::exception & ) {
		// Queue closed
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::process(ProcessingTask &task) {
	MessageProcessors::iterator it;
	for ( auto &proc : _messageProcessors ) {
		if ( task.second->type == Message::Type::Regular )
			proc->process(task.second);
		task.second->processed = true;
		// TODO: Decide whether to skip messages where processing failed or not
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::taskReady(const ProcessingTask &task) {
	if ( _processedMessageDispatcher ) {
		_results.push(task);
		_processedMessageDispatcher->messageAvailable(this);
	}
	else {
		publish(task.first, task.second);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::activate() {
	if ( _messageProcessor ) {
		return;
	}

	if ( _messageProcessors.empty() ) {
		return;
	}

	// Start the processing thread
	_messageProcessor = new thread(bind(&Queue::processingLoop, this));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::shutdown() {
	SEISCOMP_DEBUG("[queue] Shutdown");

	// Close the queues and let the thread terminate
	_tasks.close();
	_results.close();

	if ( _messageProcessor ) {
		_messageProcessor->join();
		delete _messageProcessor;
		_messageProcessor = nullptr;
	}

	// Disconnect all clients
	{
		Clients::iterator it;
		for ( it = _clients.begin(); it != _clients.end(); ++it ) {
			if ( it.value() ) {
				it.value()->_queue = nullptr;
			}
		}
		_clients.clear();
	}

	// Remove all members from all groups
	for ( auto it = _groups.begin(); it != _groups.end(); ++it ) {
		it->second->clearMembers();
	}

	// Clear remaining tasks
	_tasks.reopen();
	while ( _tasks.canPop() ) {
		ProcessingTask task = _tasks.pop();
		delete task.second;
	}
	_tasks.close();

	// Clear pending results
	_results.reopen();
	while ( _results.canPop() ) {
		ProcessingTask result = _results.pop();
		delete result.second;
	}
	_results.close();

	// Clear message ring
	_messages.clear();

	// Reset sequence number counter
	_sequenceNumber = 0;

	// Shutdown processors
	for ( auto &proc : _messageProcessors )
		proc->close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::timeout() {
	static string ConstantInfo =
		string(Status::Tag(Status::Hostname).toString()) + "=" + HostInfo.name() + "&" +
		string(Status::Tag(Status::Programname).toString()) + "=" + HostInfo.programName() + "&" +
		string(Status::Tag(Status::PID).toString()) + "=" + Core::toString(HostInfo.pid()) + "&" +
		string(Status::Tag(Status::TotalMemory).toString()) + "=" + Core::toString(HostInfo.totalMemory()) + "&";

	Core::Time now = Core::Time::GMT();
	Clients::iterator cit;

	for ( cit = _clients.begin(); cit != _clients.end(); ) {
		Client *client = cit.value();
		++cit;

		if ( !client ) continue;

		if ( client->_ackInitiated ) {
			Core::TimeSpan dt = now - client->_ackInitiated;
			if ( dt.seconds() > 0 ) {
				client->_acknowledgeCounter = client->_acknowledgeWindow;
				client->ack();
				client->_ackInitiated = Core::Time();
			}
		}

		++client->_inactivityCounter;
		if ( client->_inactivityCounter > _inactivityLimit ) {
			// The implementation will remove itself from the queue
			SEISCOMP_INFO("Remove client %s due to inactivity",
			              client->_name.c_str());
			client->dispose();
		}
	}

	if ( !_lastSOHTimestamp.valid() ) {
		_lastSOHTimestamp = now;
	}
	else if ( now - _lastSOHTimestamp >= Core::TimeSpan(_sohInterval,0) ) {
		// Create scmaster SOH message
		_lastSOHTimestamp = now;

		Message sohMessage;
		sohMessage.type = Message::Type::Status;
		sohMessage.sender = MASTER_NAME;
		sohMessage.target = StatusGroup;
		sohMessage.timestamp = now;

		double usedCPU = floor(HostInfo.getCurrentCpuUsage() * 1E4) * 1E-4;
		if ( usedCPU < 0 ) usedCPU = 0;

		{
			bio::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(sohMessage.payload);
			std::ostream os(&buf);

			os << ConstantInfo
			   << Status::Tag(Status::Time).toString() << "=" << sohMessage.timestamp.iso() << "&"
			   << Status::Tag(Status::Clientname).toString() << "=" << MASTER_NAME << "&"
			   << Status::Tag(Status::CPUUsage).toString() << "=" << fixed << setprecision(3) << usedCPU << "&"
			   << Status::Tag(Status::ClientMemoryUsage).toString() << "=" << HostInfo.getCurrentMemoryUsage() << "&"
			   << Status::Tag(Status::ObjectCount).toString() << "=" << Core::BaseObject::ObjectCount() << "&"
			   << Status::Tag(Status::MessageQueueSize).toString() << "=" << _tasks.size() << "&"
			   << Status::Tag(Status::Uptime).toString() << "=" << Core::toString(floor(double(now - _created)*100 + 0.5)*0.01);

			for ( auto &&item : _processors )
				item->getInfo(now, os);
		}

		double lengthPayload = sohMessage.payload.size();

		auto git = _groups.find(sohMessage.target);
		if ( git != _groups.end() ) {
			// Distribute to members
			Group *group = git->second.get();
			sohMessage._internalGroupPtr = group;

			Group::Members::iterator mit;
			for ( mit = group->_members.begin(); mit != group->_members.end(); ++mit ) {
				double lengthMessage = (*mit)->publish(nullptr, &sohMessage);
				// Each message sent to a member of a particular group is tagged
				// as sent.
				++git->second->_txMessages.sent;
				git->second->_txPayload.sent += lengthPayload;
				git->second->_txBytes.sent += lengthMessage;

				++_txMessages.sent;
				_txPayload.sent += lengthPayload;
				_txBytes.sent += lengthMessage;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Queue::getStatisticsSnapshot(QueueStatistics &stats, bool reset) {
	Groups::iterator it;
	size_t idx;

	stats.name = _name;
	stats.messages = _txMessages;
	stats.bytes = _txBytes;
	stats.payload = _txPayload;
	stats.groups.resize(_groups.size());

	for ( idx = 0, it = _groups.begin(); it != _groups.end(); ++it, ++idx ) {
		stats.groups[idx].name = it->first;
		stats.groups[idx].messages = it->second->_txMessages;
		stats.groups[idx].bytes = it->second->_txBytes;
		stats.groups[idx].payload = it->second->_txPayload;
		if ( reset )
			it->second->_txMessages =
			it->second->_txBytes =
			it->second->_txPayload = Tx();
	}

	if ( reset )
		_txMessages = _txBytes = _txPayload = Tx();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Queue::allocateClientHeap(int bytes) {
	if ( _allocatedClientHeap + bytes > Client::MaxLocalHeapSize )
		return -NotEnoughClientHeap;

	int offset = _allocatedClientHeap;
	_allocatedClientHeap += bytes;
	return offset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
