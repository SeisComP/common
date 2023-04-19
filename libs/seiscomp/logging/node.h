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


#ifndef SC_LOGGING_NODE_H
#define SC_LOGGING_NODE_H

#include <list>
#include <set>
#include <time.h>
#include <string>
#include <mutex>

#include <seiscomp/core.h>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Node;


/*! @struct Data <seiscomp/logging/node.h>
 *  @brief Data published through Node

	Data is the data which is published from SEISCOMP_DEBUG(), SEISCOMP_WARNING(),
	SEISCOMP_ERROR() and SEISCOMP_LOG() macros.  It contains a link to the
	publisher, the (approximate) time of the publication, and the formatted message.

	Note that none of the data in the publication is considered to be static.
	Once control has returned to the publisher, the data may be destroyed. If
	it is necessary to hang on to any of the data, a deep copy must be made.
*/

struct SC_SYSTEM_CORE_API Data {
	struct PublishLoc *publisher;
	//! time of publication
	time_t time;
	//! formatted msg - gets destroyed when publish() call returns.
	const char *msg;

	// track which nodes have seen this message, to avoid getting
	// duplicates.  It would be nice if we could enforce this via the node
	// structure instead, but that is much harder.
	std::set<Node*> seen;
};


/*! @class Seiscomp::Logging::Node <seiscomp/logging/node.h>
  @brief Core of publication system, forms activation network.

  Node formes the core of the publication system.  It has two primary
  purposes :
  - link publications with subscribers
  - transfer meta-data in the form or node activations

  Both publishers (eg Publisher) and subscribers (eg StdioNode) are
  derived from Node, although Node can be used entirely unmodified.

  An Node contains a list of publishers which it is linked with.  It
  also contains a list of subscribers which it is linked with.
  Publications always flow from publishers to subscribers, and activation
  information flows the opposite direction from subscribers to publishers.

  An Node by default acts as both a subscriber and publisher -- when
  it has been subscribed to another node and receives a publication it
  simply repeats the information to all of its subscribers.

  More specifically, it only publishes to subscribers who have also voiced
  an interest in receiving the publication.  If a node has no subscribers
  which are also interested (or no subscribers at all), then it can be said
  to be dormant and it tells the publishers that it is subscribed to that
  it is no longer interested.  This propogates all the way up to
  Publishers which will disable the logging statement completely if
  there are no interested parties.

  @author Valient Gough
*/
class SC_SYSTEM_CORE_API Node {
	public:
		//! @brief Instantiate an empty Node.  No subscribers or publishers..
		Node();

		//! @brief Disconnects from any remaining subscribers and publishers
		virtual ~Node();

		//! @brief Force disconnection from any subscribers or publishers
		virtual void clear();

		/*! @brief Publish data.

		  This iterates over the list of subscribers which have stated interest and
		  sends them the data.
		*/
		virtual void publish(const Data &data);

		/*! @brief Have this node subscribe to a new publisher.

		  We become a subscriber of the publisher.  The publisher's addSubscriber()
		  function is called to complete the link.

		  If our node is active then we also tell the publisher that we want
		  publications.
		*/
		virtual void addPublisher(Node *);

		/*! @brief Drop our subscription to a publisher

		  A callback parameter is provided to help avoid loops in the code which may
		  affect the thread locking code.

		  @param callback If True, then we call publisher->dropSubscriber() to make
		  sure the publisher also drops us as a subscriber.
		*/
		virtual void dropPublisher(Node *, bool callbacks=true );

		/*! @brief Returns @e true if this node is active
		  @return @e true if we have one or more interested subscribers, otherwise false
		*/
		bool enabled() const;

		/*! @brief Add a subscriber.

		  Normally a subscriber calls this for itself when it's addPublisher() method is
		  called.
		*/
		virtual void addSubscriber(Node *);

		/*! @brief Remove a subscriber.

		  Normally a subscriber calls this for itself when it's dropPublisher() method
		  is called.

		  Note that the subscriber list is kept separate from the interest list.  If
		  the subscriber is active, then you must undo that by calling
		  isInterested(subscriber, false) in addition to dropSubscriber
		*/
		virtual void dropSubscriber(Node *);

		/*! @brief Change the state of one of our subscribers.

		  This allows a subscriber to say that it wants to be notified of publications
		  or not.  The @a node should already be registered as a subscriber.

		  If we previously had no interested parties and now we do, then we need to
		  notify the publishers in our publishers list that we are now interested.

		  If we previously had interested parties and we remove the last one, then we
		  can notify the publishers that we are no longer interested..
		*/

		virtual void isInterested(Node *node, bool isInterested);

	protected:
		/*! @brief For derived classes to get notified of activation status change.

		  This is called by isInterested() when our state changes.  If @e true is
		  passed, then this node has become active.  If @e false is passed, then this
		  node has just become dormant.
		*/
		virtual void setEnabled(bool newState);

		//! list of nodes we are subscribed to
		std::list<Node *> publishers;

		//! list of nodes we publish to
		std::list<Node *> subscribers;

		//! list of subscribers that are interested in receiving publications..
		std::list<Node *> interestList;

		std::mutex mutex;
};


}
}

#endif
