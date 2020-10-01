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


#ifndef SEISCOMP_GUI_DATAMODEL_ORIGINEVALUATOR_H
#define SEISCOMP_GUI_DATAMODEL_ORIGINEVALUATOR_H


#include <QThread>
#include <QMutex>
#include <QHash>
#include <QSet>
#include <QLinkedList>
#include <QProcess>
#include <QStringList>

#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/databasereader.h>
#endif


namespace Seiscomp {
namespace Gui {


class PublicObjectEvaluator : public QThread {
	Q_OBJECT

	private:
		PublicObjectEvaluator();


	public:
		static PublicObjectEvaluator &Instance();

		bool setDatabaseURI(const char *uri);

		//! Adds a job for an publicID of a specific PublicObject type and a
		//! script to be called.
		//! Multiple calls will increase the reference count of
		//! script while removeJob will decrease it. A reference
		//! count of 0 removes this script from the job list.
		bool append(void *owner, const QString &publicID,
		            const Core::RTTI& classType, const QString &script);
		bool append(void *owner, const QString &publicID,
		            const Core::RTTI& classType, const QStringList &scripts);

		bool prepend(void *owner, const QString &publicID,
		             const Core::RTTI& classType, const QString &script);
		bool prepend(void *owner, const QString &publicID,
		             const Core::RTTI& classType, const QStringList &scripts);

		bool moveToFront(const QString &publicID);

		//! Remove a job for an publicID and a script. This decreases
		//! the reference count of script if it exists already and removes
		//! it when the reference count drops to 0.
		bool erase(void *owner, const QString &publicID, const QString &script);

		//! Removes all scripts for an publicID and does not check for
		//! reference counts.
		bool erase(void *owner, const QString &publicID);

		int pendingJobs() const;

		//! Removes all pending jobs of owner. If owner is set to nullptr
		//! all jobs are removed
		void clear(void *owner);

		//! Does an evaluation. The evaluation is not queued but executed
		//! in place,
		bool eval(DataModel::PublicObject *po, const QStringList &scripts);

		QString errorMsg(int errorCode) const;


	protected:
		void run();


	private:
		bool connect();


	signals:
		void resultAvailable(const QString &publicID, const QString &className,
		                     const QString &script, const QString &result);

		void resultError(const QString &publicID, const QString &className,
		                 const QString &script, int errorCode);


	private:
		// A script which is an owner. If two or more objects
		// added the same script then owner is set to nullptr.
		typedef QHash<QString,void*> Scripts;
		struct Job {
			Job(const QString &id, const Core::RTTI &ct)
			 : publicID(id), classType(ct) {}

			QString     publicID;
			Core::RTTI  classType;
			Scripts     scripts;
		};

		typedef QLinkedList<Job> JobList;
		typedef QHash<QString, JobList::iterator> JobIDMap;

		static PublicObjectEvaluator    *_instance;
		mutable QMutex                   _mutexJobList;
		std::string                      _databaseURI;
		DataModel::DatabaseReader        _reader;

		JobIDMap                         _jobIDLookup;
		JobList                          _jobs;
};


}
}


#endif
