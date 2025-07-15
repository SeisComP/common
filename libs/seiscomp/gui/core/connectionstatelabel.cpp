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



#include <seiscomp/gui/core/connectionstatelabel.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/utils.h>

#include <QMouseEvent>
#include <QDateTime>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConnectionStateLabel::ConnectionStateLabel(QWidget *parent, Qt::WindowFlags f)
 : QLabel(parent, f) {
	_connected = icon("wifi").pixmap(fontMetrics().height());
	_disconnected = icon("wifi-off").pixmap(fontMetrics().height());
	setPixmap(_disconnected);
	setFrameStyle(QFrame::NoFrame);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionStateLabel::setPixmaps(const QPixmap &connected,
                                      const QPixmap &disconnected) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	bool isConnected = (pixmap()->toImage() == _connected.toImage());
#else
	bool isConnected = (pixmap().toImage() == _connected.toImage());
#endif

	_connected = connected;
	_disconnected = disconnected;

	setPixmap(isConnected ? _connected : _disconnected);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionStateLabel::start(const QString &source) {
	setPixmap(_connected);
	if ( source.isEmpty() ) {
		setToolTip(QString("Connected at: %1")
		           .arg(QDateTime::currentDateTime().toString()));
	}
	else {
		setToolTip(QString("Connected to %1 at: %2")
		           .arg(source, QDateTime::currentDateTime().toString()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionStateLabel::stop() {
	setPixmap(_disconnected);
	setToolTip("Disconnected at: " + QDateTime::currentDateTime().toString() );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionStateLabel::mousePressEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		emit customInfoWidgetRequested(event->pos());
		return;
	}

	QLabel::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
