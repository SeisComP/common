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


class QEventLoop;
class QPushButton;
class QResizeEvent;
class QShowEvent;


class HttpRequest : public QProgressDialog {
	Q_OBJECT

	public:
		HttpRequest(QWidget *parent = 0);
		~HttpRequest();

	public:
		int get(const QString &url);

		const QByteArray &response() { return _response; }

		/**
		 * @brief Replaces the cancel button by one which skips the current
		 *        request and adds a second one to its left which skips the
		 *        remaining requests of a series as well.
		 */
		void setSkipButtons(const QString &skip, const QString &skipAll);

		bool skippedAll() const { return _skippedAll; }

		/**
		 * @brief Keeps the window open between the requests of a series. It
		 *        closes when it is hidden or destroyed.
		 */
		void keepOpen();

		void done(int result) override;

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void showEvent(QShowEvent *event) override;

	private slots:
		void cancelRequest();
		void skipAll();
		void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
		void httpRequestFinished(QNetworkReply *reply);

	private:
		void layoutSkipAll();

	private:
		QNetworkAccessManager  _manager;
		QNetworkReply         *_reply;
		QByteArray             _response;
		int                    _responseCode;
		QPushButton           *_cancel{nullptr};
		QPushButton           *_skipAll{nullptr};
		bool                   _skippedAll{false};
		bool                   _keepOpen{false};
		QEventLoop            *_loop{nullptr};
};


#endif
