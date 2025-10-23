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


#ifndef SCCONFIG_HTTP_H
#define SCCONFIG_HTTP_H


#include <QProgressDialog>
#include <QBuffer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


class HttpRequest : public QProgressDialog {
	Q_OBJECT

	public:
		HttpRequest(QWidget *parent = 0);
		~HttpRequest();

	public:
		int get(const QString &url);

		const QByteArray &response() { return _response; }

	private slots:
		void cancelRequest();
		void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
		void httpRequestFinished(QNetworkReply *reply);

	private:
		QNetworkAccessManager  _manager;
		QNetworkReply         *_reply;
		QByteArray             _response;
		int                    _responseCode;
};


#endif
