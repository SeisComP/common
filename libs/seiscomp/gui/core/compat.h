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



#ifndef SEISCOMP_GUI_COMPAT_H
#define SEISCOMP_GUI_COMPAT_H


#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
	#include <QIODevice>
	#include <QProcess>
#endif

// 5.0:
#define QT_WE_DELTA(wheelEvent) (\
	wheelEvent->angleDelta().x() ? \
	wheelEvent->angleDelta().x() : wheelEvent->angleDelta().y()\
)

// 5.11: QFontMetrics width replaced by horizontalAdvance
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
	#define QT_FM_WIDTH(fm, str) fm.horizontalAdvance(str)
#else
	#define QT_FM_WIDTH(fm, str) fm.width(str)
#endif

// 5.14:
//  QWheelEvent::pos() -> position()
//  QWheelEvent::globalPos() -> globalPosition()
//  QString::KeepEmptyParts|SkipEmptyParts -> Qt::QString::KeepEmptyParts|SkipEmptyParts
//  right -> Qt::right
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
	#define QT_WE_POSF(wheelEvent) wheelEvent->position()
	#define QT_WE_POS(wheelEvent) QT_WE_POSF(wheelEvent).toPoint()
	#define QT_WE_GLOBALPOSF(wheelEvent) wheelEvent->globalPosition()
	#define QT_WE_GLOBALPOS(wheelEvent) QT_WE_GLOBALPOSF(wheelEvent).toPoint()

	#define QT_KEEP_EMPTY_PARTS Qt::KeepEmptyParts
	#define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts

	#define QT_RIGHT Qt::right
	#define QT_FIXED Qt::fixed
	#define QT_ENDL Qt::endl
#else
	#define QT_WE_POS(wheelEvent) wheelEvent->pos()
	#define QT_WE_POSF(wheelEvent) QPointF(QT_WE_POS(wheelEvent))
	#define QT_WE_GLOBALPOS(wheelEvent) wheelEvent->globalPos()
	#define QT_WE_GLOBALPOSF(wheelEvent) QPointF(QT_WE_GLOBALPOS(wheelEvent))

	#define QT_KEEP_EMPTY_PARTS QString::KeepEmptyParts
	#define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts

	#define QT_RIGHT right
	#define QT_FIXED fixed
	#define QT_ENDL endl
#endif

// 5.15:
//  QProcess::start(QString) -> start(QString, QStringList)
//  QProcess::startDetached(QString) -> startDetached(QString, QStringList)
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
	inline void QProcessStart(QProcess *process, const QString &command,
	                          QIODevice::OpenMode mode = QIODevice::ReadWrite) {
		auto args = QProcess::splitCommand(command);
		QString cmd;
		if ( !args.isEmpty() ) {
			cmd = args.takeFirst();
		}
		process->start(cmd, args, mode);
	}
	inline bool QProcessStartDetached(const QString &command,
	                                  const QString &workingDirectory = QString(),
	                                  qint64 *pid = nullptr) {
		auto args = QProcess::splitCommand(command);
		QString cmd;
		if ( !args.isEmpty() ) {
			cmd = args.takeFirst();
		}
		return QProcess::startDetached(cmd, args, workingDirectory, pid);
	}
	#define QT_PROCESS_START(proc, cmd) QProcessStart(proc, cmd)
	#define QT_PROCESS_START2(proc, cmd, mode) QProcessStart(proc, cmd, mode)
	#define QT_PROCESS_STARTDETACHED(cmd) QProcessStartDetached(cmd)
	#define QT_PROCESS_STARTDETACHED2(cmd, wd) QProcessStartDetached(cmd, wd)
	#define QT_PROCESS_STARTDETACHED3(cmd, wd, pid) QProcessStartDetached(cmd, wd, pid)
#else
	// start(command) is deprecated since 5.15 but uses internally the same
	// code as splitCommand() to separate the command from the argument list
	#define QT_PROCESS_START(proc, cmd) (proc)->start(cmd)
	#define QT_PROCESS_START_MODE(proc, cmd, mode) (proc)->start(cmd, mode)
	#define QT_PROCESS_STARTDETACHED(cmd) QProcess::startDetached(cmd)
	#define QT_PROCESS_STARTDETACHED2(cmd, wd) QProcess::startDetached(cmd, wd)
	#define QT_PROCESS_STARTDETACHED3(cmd, wd, pid) QProcess::startDetached(cmd, wd, pid)
#endif

#endif
