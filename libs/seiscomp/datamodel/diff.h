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


#ifndef SEISCOMP_DATAMODEL_DIFF_H__
#define SEISCOMP_DATAMODEL_DIFF_H__


#include <seiscomp/core.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/object.h>

#include <vector>
#include <map>


namespace Seiscomp {
namespace DataModel {


class Diff2 {
	public:
		DEFINE_SMARTPOINTER(LogNode);
		class LogNode: public Seiscomp::Core::BaseObject {
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

		typedef std::vector<Seiscomp::DataModel::NotifierPtr> Notifiers;
		typedef std::map<std::string, const Core::MetaProperty*> PropertyIndex;


	public:
		Diff2();
		virtual ~Diff2();

		void diff(Seiscomp::DataModel::Object *o1, Seiscomp::DataModel::Object *o2,
		          const std::string &o1ParentID, Notifiers &notifiers,
		          LogNode *logNode = nullptr);

		NotifierMessage *diff2Message(Seiscomp::DataModel::Object *o1, Seiscomp::DataModel::Object *o2,
		                              const std::string &o1ParentID, LogNode *logNode = nullptr);

	protected:
		std::string o2t(const Core::BaseObject *o) const;
		void createLogNodes(LogNode *rootLogNode, const std::string &rootID,
		                    Notifiers::const_iterator begin,
		                    Notifiers::const_iterator end);

		virtual bool blocked(const Core::BaseObject *o, LogNode *node, bool local);
};


class Diff3 : public Diff2 {
	public:
		void diff(Seiscomp::DataModel::Object *o1, Seiscomp::DataModel::Object *o2,
		          const std::string &o1ParentID, Notifiers &notifiers,
		          LogNode *logNode = nullptr);


	protected:
		virtual bool confirmUpdate(const Core::BaseObject *localO,
		                           const Core::BaseObject *remoteO,
		                           LogNode *node) = 0;
};


} // ns DataModel
} // ns Seiscomp

#endif // SEISCOMP_DATAMODEL_DIFF_H__
