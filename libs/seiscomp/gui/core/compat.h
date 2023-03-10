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

// 5.0:
//  QWheelEvent::delta()/orientation -> angleDelta()
//  QWhellEvent::globalPos() -> globalPosition()
#if QT_VERSION >= 0x050000
    #define QT_WE_ANGLEDELTA(wheelEvent) wheelEvent->angleDelta()
    #define QT_WE_DELTA(wheelEvent) (\
        QT_WE_ANGLEDELTA(wheelEvent).x() ? \
        QT_WE_ANGLEDELTA(wheelEvent).x() : QT_WE_ANGLEDELTA(wheelEvent).y()\
    )

    #define QT_WE_GLOBALPOSF(wheelEvent) wheelEvent->globalPosition()
    #define QT_WE_GLOBALPOS(wheelEvent) QT_WE_GLOBALPOSF(wheelEvent).toPoint()
#else
    #define QT_WE_ANGLEDELTA(wheelEvent) (\
        wheelEvent->orientation() == Qt::Vertical ? \
        QPoint(0, wheelEvent->delta()) : QPoint(wheelEvent->delta(), 0)\
    )
    #define QT_WE_DELTA(wheelEvent) wheelEvent->delta()

    #define QT_WE_GLOBALPOS(wheelEvent) wheelEvent->globalPos()
    #define QT_WE_GLOBALPOSF(wheelEvent) QPointF(QT_WE_GLOBALPOS(wheelEvent))
#endif

// 5.11: QFontMetrics width replaced by horizontalAdvance
#if QT_VERSION >= 0x050b00
    #define QT_FM_WIDTH(fm, str) fm.horizontalAdvance(str)
#else
    #define QT_FM_WIDTH(fm, str) fm.width(str)
#endif

// 5.14:
//  QWheelEvent::pos() -> position()
//  QString::KeepEmptyParts|SkipEmptyParts -> Qt::QString::KeepEmptyParts|SkipEmptyParts
#if QT_VERSION >= 0x050e00
    #define QT_WE_POSF(wheelEvent) wheelEvent->position()
    #define QT_WE_POS(wheelEvent) QT_WE_POSF(wheelEvent).toPoint()

	#define QT_KEEP_EMPTY_PARTS Qt::KeepEmptyParts
	#define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
    #define QT_WE_POS(wheelEvent) wheelEvent->pos()
    #define QT_WE_POSF(wheelEvent) QPointF(QT_WE_POS(wheelEvent))

    #define QT_KEEP_EMPTY_PARTS QString::KeepEmptyParts
    #define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif


#endif
