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


#ifndef SEISCOMP_DATAMODEL_DIFF_H
#define SEISCOMP_DATAMODEL_DIFF_H


#include <seiscomp/core.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/object.h>

#include <vector>
#include <map>


namespace Seiscomp {
namespace DataModel {


/**
 * @brief An object tree difference calculator.
 *
 * This class implements a difference calculator between two object trees.
 * The goal is to have have same tree on both sides after the operation.
 * The result is a list of notifiers. Both object trees won't be changed.
 */
class Diff2 {
	public:
		DEFINE_SMARTPOINTER(LogNode);
		class LogNode: public Core::BaseObject {
			public:
				enum LogLevel {
					OPERATIONS  = 0, // e.g. ADD, UPDATE, REMOVE, MERGE
					DIFFERENCES = 1, // objects and its children which differ
					ALL = 2          // all objects and its children
				};

				LogNode(const std::string &title = "",
				        LogLevel level = OPERATIONS,
				        LogNode *parent = nullptr)
				 : _title(title), _level(level), _parent(parent) {}

				inline const std::string& title() const { return _title; }
				inline void setTitle(const std::string &title) { _title = title; }

				inline LogLevel level() const { return _level; }
				inline void setLevel(LogLevel level) { _level = level; }

				inline LogNode* parent() const { return _parent; }
				inline void setParent(LogNode* parent) { _parent = parent; }

				inline LogNode* addChild(const std::string &title,
				                         const std::string &msg = "") {
					LogNode *child = new LogNode(title, _level, this);
					child->setMessage(msg);
					_children.push_back(child);
					return child;
				}
				inline void addChild(LogNode *logNode, const std::string &msg = "") {
					if ( !msg.empty() )
						logNode->setMessage(msg);
					logNode->setParent(this);
					_children.push_back(logNode);
				}
				inline size_t childCount() { return _children.size(); }

				inline void setMessage(const std::string &msg) { _message = msg; }

				void write(std::ostream &os, int padding = 0, int indent = 1,
				           bool ignoreFirstPad = false) const;

			private:
				std::string              _title;
				LogLevel                 _level;
				LogNode                 *_parent;

				std::vector<LogNodePtr>  _children;
				std::string              _message;
		};

		typedef std::vector<NotifierPtr> Notifiers;
		typedef std::map<std::string, const Core::MetaProperty*> PropertyIndex;


	public:
		Diff2();
		virtual ~Diff2();

		/**
		 * @brief Calculates a list of notifiers so that o1 will reflect the
		 *        state of o2.
		 * @param o1 The target tree.
		 * @param o2 The reference tree.
		 * @param o1ParentID The optional parent ID of o1.
		 * @param notifiers The list of notifiers to be populated.
		 * @param logNode The log node root which will be populated with
		 *                log messages. This is optional.
		 */
		void diff(Object *o1, Object *o2,
		          const std::string &o1ParentID, Notifiers &notifiers,
		          LogNode *logNode = nullptr);

		NotifierMessage *diff2Message(Object *o1, Object *o2,
		                              const std::string &o1ParentID, LogNode *logNode = nullptr);

	protected:
		std::string o2t(const Core::BaseObject *o) const;
		void createLogNodes(LogNode *rootLogNode, const std::string &rootID,
		                    Notifiers::const_iterator begin,
		                    Notifiers::const_iterator end);

		/**
		 * @brief Allows for derived classes to block an object.
		 * @param o The object to be checked.
		 * @param node The log node which can be used to set reason for the blocking.
		 * @param local Whether it is the local object (true) or the remote (false).
		 * @return true if the object is blocked, false if should be processed.
		 */
		virtual bool blocked(const Core::BaseObject *o, LogNode *node, bool local);
};


/**
 * @brief An object tree difference calculator version 3.
 *
 * In addition to Diff2 this class allows for derived classes to skip possible
 * updates. For that the virtual method confirmUpdate has been introduced which
 * allows to block an update based on some criteria, e.g. comparison result of
 * creation or modification time.
 */
class Diff3 : public Diff2 {
	public:
		void diff(Object *o1, Object *o2,
		          const std::string &o1ParentID, Notifiers &notifiers,
		          LogNode *logNode = nullptr);


	protected:
		/**
		 * @brief Decide whether an object may be updated or not.
		 *
		 * If the algorithm decides that the local object must be updated
		 * to reflect the remote object it will call this method to ask for
		 * permission to do so.
		 *
		 * Even if the object is not allowed to be updated, the algorithm will
		 * still decent to child objects.
		 *
		 * @param localO The local object which would receive the update.
		 * @param remoteO The remote object, the reference.
		 * @param node The log node.
		 * @return true if the object may be updated, false otherwise.
		 */
		virtual bool confirmUpdate(const Core::BaseObject *localO,
		                           const Core::BaseObject *remoteO,
		                           LogNode *node) = 0;
};


/**
 * @brief An object tree difference calculator version 4.
 *
 * In addition to version 3 this class allows for derived classes to decide
 * whether decending to child nodes is allowed or not on a per node basis.
 *
 * Sometimes it might be desirable to skip an entire subtree if the parent
 * object must not be updated, e.g. if the modification time of the local object
 * is later than the modification time of the remote object and child objects
 * do not provide any information to compare modification times.
 *
 * @version 15.1
 */
class Diff4 : public Diff3 {
	public:
		void diff(Object *o1, Object *o2,
		          const std::string &o1ParentID, Notifiers &notifiers,
		          LogNode *logNode = nullptr);

	protected:
		/**
		 * @brief Confirm removal of an object.
		 *
		 * This method will be called if there is not remote counterpart.
		 *
		 * @param localO The local object
		 * @param node The log node
		 * @return true if the object is allowed to be removed, false otherwise
		 */
		virtual bool confirmRemove(const Core::BaseObject *localO,
		                           LogNode *node) = 0;

		/**
		 * @brief Confirm descending to child objects.
		 *
		 * This method will only be called on array properties.
		 *
		 * @param localO The local parent node.
		 * @param remoteO The remote parent node.
		 * @param updateConfirmed The state of the preceding confirmUpdate call.
		 * @param property The property which reflects the subtree.
		 * @param node The log node.
		 * @return true if descending is allowed, false otherwise.
		 */
		virtual bool confirmDescent(const Core::BaseObject *localO,
		                            const Core::BaseObject *remoteO,
		                            bool updateConfirmed,
		                            const Core::MetaProperty *property,
		                            LogNode *node) = 0;
};


} // ns DataModel
} // ns Seiscomp

#endif // SEISCOMP_DATAMODEL_DIFF_H__
