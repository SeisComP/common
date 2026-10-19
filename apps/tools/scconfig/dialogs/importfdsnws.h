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


#ifndef SCCONFIG_DIALOGS_IMPORTSTATIONSFDSNWS_H
#define SCCONFIG_DIALOGS_IMPORTSTATIONSFDSNWS_H


#include "ui_importfdsnws.h"
#include "ui_importfdsnws_sidebar.h"


class QNetworkReply;


class ImportFDSNWSDialog : public QDialog {
	Q_OBJECT

	public:
		explicit ImportFDSNWSDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());


	public:
		virtual void accept();


	private:
		// Name of the agency and the URL to query, in that order.
		using DataCenters = QList<QPair<QString, QString>>;

		void checkDates();
		void setSaveEnabled(bool enabled);
		void clearTable();
		void updateConstraints();
		void queryDataCenters();
		void updateDataCenters(QNetworkReply *reply);
		void setDataCenters(const DataCenters &dataCenters);
		int currentDataCenter() const;
		QString serverURL() const;
		void showServerURL();
		void updateServerTitle();
		QString defaultFilename() const;
		void updateFilename();
		QString queryURL() const;
		QString queryURL(const QString &base) const;
		void clearStationCounts();
		void highlightServer();
		std::string request(const char *parameters) const;
		QString queryFormat();
		std::string saveParameters() const;
		void updateURL();


	private slots:
		void fetch();
		void testServers();


	private:
		Ui::ImportFDSNWS        _ui;
		Ui::ImportFDSNWSSidebar _uiSideBar;
		QString                 _serverTitle;
		QString                 _defaultFilename;
		bool                    _fetched{false};
		QString                 _format{"xml"};
		int                     _presetCount{0};
		int                     _customIndex{-1};
};


#endif
