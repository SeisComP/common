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



#ifndef SEISCOMP_GUI_CONNECTIONSTATELABEL_H
#define SEISCOMP_GUI_CONNECTIONSTATELABEL_H


#include <seiscomp/gui/qt.h>

#include <QLabel>
#include <QPixmap>


namespace Seiscomp::Gui {


class SC_GUI_API ConnectionStateLabel : public QLabel {
	Q_OBJECT

	public:
		ConnectionStateLabel(QWidget *parent = nullptr, Qt::WindowFlags f = {});

		void setPixmaps(const QPixmap &connected, const QPixmap &disconnected);

	public slots:
		void start(const QString &source);
		void stop();

	signals:
		void customInfoWidgetRequested(const QPoint &pos);

	protected:
		void mouseReleaseEvent(QMouseEvent *event) override;

	protected:
		QPixmap _connected;
		QPixmap _disconnected;
		bool    _isConnected{false};
};


}


#endif
