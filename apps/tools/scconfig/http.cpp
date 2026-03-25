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


#include <QMessageBox>
#include <QUrl>
#include <QtNetwork/QSslError>

#include <iostream>

#include "http.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HttpRequest::HttpRequest(QWidget *parent)
: QProgressDialog(parent), _reply(0) {
	setModal(true);
	setRange(0, 0);
	setAutoClose(true);

	connect(this, &HttpRequest::canceled, this, &HttpRequest::cancelRequest);
	connect(&_manager, &QNetworkAccessManager::sslErrors, this, &HttpRequest::sslErrors);
	connect(&_manager, &QNetworkAccessManager::finished, this, &HttpRequest::httpRequestFinished);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HttpRequest::~HttpRequest() {
	if ( _reply ) {
		_reply->abort();
		_reply->deleteLater();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int HttpRequest::get(const QString &url_) {
	cancelRequest();

	QUrl url(url_);

	QNetworkRequest request;
	request.setUrl(url);
	#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)) && (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	#elif QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
	#endif

	_responseCode = -1;
	_response = QByteArray();
	_reply = _manager.get(request);

	show();

	int result = exec();
	if ( result != QProgressDialog::Accepted )
		return -1;

	return _responseCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpRequest::cancelRequest() {
	if ( _reply ) {
		_reply->abort();
		delete _reply;
		_reply = 0;
	}

	_response = QByteArray();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpRequest::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors) {
	reply->ignoreSslErrors();
	std::cerr << "SSL Errors:" << std::endl;
	foreach ( QSslError err, errors ) {
		std::cerr << " * " << qPrintable(err.errorString()) << std::endl;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpRequest::httpRequestFinished(QNetworkReply *reply) {
	if ( reply == _reply ) {
		reset();

		_response = reply->readAll();
		QVariant rc = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		if ( rc.isValid() ) {
			_responseCode = rc.toInt();
		}

		if ( _reply->error() == QNetworkReply::NoError ) {
			accept();
		}
		else {
			if ( _responseCode < 0 ) {
				// Only show network errors
				QMessageBox::critical(
					this, tr("Connection error %1").arg(_reply->error()),
					_reply->errorString()
				);
			}

			reject();
		}

		_reply->deleteLater();
		_reply = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
