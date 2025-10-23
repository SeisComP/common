/***************************************************************************
 * Copyright (C) 2017 by gempa GmbH                                        *
 *                                                                         *
 * All Rights Reserved.                                                    *
 *                                                                         *
 * NOTICE: All information contained herein is, and remains                *
 * the property of gempa GmbH and its suppliers, if any. The intellectual  *
 * and technical concepts contained herein are proprietary to gempa GmbH   *
 * and its suppliers.                                                      *
 * Dissemination of this information or reproduction of this material      *
 * is strictly forbidden unless prior written permission is obtained       *
 * from gempa GmbH.                                                        *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
 ***************************************************************************/


#define SEISCOMP_COMPONENT scconfig

#include <seiscomp/logging/log.h>
#include <seiscomp/datamodel/inventory.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/importer.h>
#include <seiscomp/system/environment.h>

#include <QMessageBox>
#include <QTextEdit>

#include "importfdsnws.h"
#include "../http.h"


using namespace std;
using namespace Seiscomp;


namespace {


class MemStreamBuf : public basic_streambuf<char> {
	public:
		MemStreamBuf(char* p, size_t n) {
			setg(p, p, p + n);
		}
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void setup(QDateTimeEdit *edit) {
	edit->setMinimumDateTime(QDateTime::fromSecsSinceEpoch(0));
	edit->setDateTime(edit->minimumDateTime());
}

ImportFDSNWSDialog::ImportFDSNWSDialog(QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	auto *sideBar = new QWidget(this);
	_uiSideBar.setupUi(sideBar);

	_ui.sideBar->setWidget(sideBar);

	_ui.tableStations->horizontalHeader()->setStretchLastSection(true);
	_ui.okButton->setEnabled(false);
	_uiSideBar.comboGeoType->setCurrentIndex(0);
	_uiSideBar.frameGeoBoundingBox->setVisible(false);
	_uiSideBar.frameGeoCircle->setVisible(false);

	setup(_uiSideBar.dtStart);
	setup(_uiSideBar.dtEnd);
	setup(_uiSideBar.dtStartBefore);
	setup(_uiSideBar.dtStartAfter);
	setup(_uiSideBar.dtEndBefore);
	setup(_uiSideBar.dtEndAfter);

	auto now = QDateTime::currentDateTimeUtc();
	now.setTime(QTime());

	_uiSideBar.dtStartBefore->setDateTime(now);
	_uiSideBar.dtEndAfter->setDateTime(now);

	connect(_ui.btnFetch, &QPushButton::clicked, this, &ImportFDSNWSDialog::fetch);
	connect(_uiSideBar.comboGeoType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
		switch ( index ) {
			case 0:
				_uiSideBar.frameGeoBoundingBox->setVisible(false);
				_uiSideBar.frameGeoCircle->setVisible(false);
				break;
			case 1:
				_uiSideBar.frameGeoBoundingBox->setVisible(true);
				_uiSideBar.frameGeoCircle->setVisible(false);
				break;
			case 2:
				_uiSideBar.frameGeoBoundingBox->setVisible(false);
				_uiSideBar.frameGeoCircle->setVisible(true);
				break;
		}
	});
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::accept() {
	if ( _query.empty() ) {
		return;
	}

	HttpRequest req;
	req.setLabelText(tr("Requesting channels and responses"));

	string query = _query + "format=sc3ml&level=";
	if ( _uiSideBar.checkResponses->isChecked() ) {
		query += "response";
	}
	else {
		query += "channel";
	}

	SEISCOMP_DEBUG("FDSNWS: %s", query);

	Seiscomp::DataModel::InventoryPtr inventory;
	int code = req.get(query.c_str());
	if ( code == 200 ) {
		Seiscomp::IO::XMLArchive ar;
		MemStreamBuf buf(const_cast<char*>(req.response().constData()), req.response().size());
		ar.open(&buf);
		ar >> inventory;
		ar.close();

		if ( !inventory ) {
			QMessageBox::critical(nullptr, tr("Error"), tr("Import failed"));
			return;
		}
	}
	else if ( code == 204 ) {
		QMessageBox::critical(nullptr, tr("No content"), tr("The request did just return an empty document"));
		return;
	}
	else {
		Seiscomp::IO::ImporterPtr imp = Seiscomp::IO::Importer::Create("fdsnxml");
		if ( !imp ) {
			QMessageBox::critical(nullptr,
			                      tr("FDSN StationXML not supported"),
			                      tr("Make sure that the plugin 'fdsnxml' is loaded. This "
			                         "is configured in the default configuration file "
			                         "but if the plugin list is overwritten (plugins = a,b,c) "
			                         "rather than inherited (plugins = ${plugins},a,b,c) "
			                         "then this plugin might have been removed."));
			return;
		}

		query = _query + "format=xml&level=response";
		SEISCOMP_DEBUG("FDSNWS: %s", query);
		code = req.get(query.c_str());
		if ( code == 200 ) {
			MemStreamBuf buf(const_cast<char*>(req.response().constData()), req.response().size());
			Seiscomp::Core::BaseObjectPtr obj = imp->read(&buf);
			if ( !obj ) {
				QMessageBox::critical(nullptr, tr("Error"), tr("Conversion from StationXML failed"));
				return;
			}

			inventory = Seiscomp::DataModel::Inventory::Cast(obj);
			if ( !inventory ) {
				QMessageBox::critical(nullptr, tr("Error"), tr("Conversion from StationXML failed"));
				return;
			}
		}
		else {
			QMessageBox::critical(nullptr, tr("No content"), tr("The request did not return valid content"));
			return;
		}
	}

	auto *env = Seiscomp::Environment::Instance();
	auto output = env->installDir() + "/etc/inventory/";
	if ( _uiSideBar.editFilename->text().isEmpty() ) {
		output += "inv-";
		if ( _uiSideBar.editNetworks->text().isEmpty() ) {
			output += "all";
		}
		else {
			output += _uiSideBar.editNetworks->text().replace(" ", "").replace(",", "_").toStdString();
		}
		output += ".xml";
	}
	else {
		output += _uiSideBar.editFilename->text().toStdString();
	}

	Seiscomp::IO::XMLArchive ar;
	ar.create(output.data());
	ar.setFormattedOutput(true);
	ar << inventory;
	ar.close();

	QDialog::accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::clearTable() {
	while ( _ui.tableStations->rowCount() ) {
		_ui.tableStations->removeRow(0);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::fetch() {
	ostringstream oss;
	oss << _uiSideBar.editServerURL->text().toStdString() << "/station/1/query?";

	if ( _uiSideBar.dtStart->dateTime() > _uiSideBar.dtStart->minimumDateTime() ) {
		oss << "starttime=" << qPrintable(_uiSideBar.dtStart->dateTime().toString("yyyy-MM-ddTHH:mm:00")) << "&";
	}
	if ( _uiSideBar.dtEnd->dateTime() > _uiSideBar.dtEnd->minimumDateTime() ) {
		oss << "endtime=" << qPrintable(_uiSideBar.dtEnd->dateTime().toString("yyyy-MM-ddTHH:mm:00")) << "&";
	}
	if ( _uiSideBar.dtStartBefore->dateTime() > _uiSideBar.dtStartBefore->minimumDateTime() ) {
		oss << "startbefore=" << qPrintable(_uiSideBar.dtStartBefore->dateTime().toString("yyyy-MM-ddTHH:mm:00")) << "&";
	}
	if ( _uiSideBar.dtStartAfter->dateTime() > _uiSideBar.dtStartAfter->minimumDateTime() ) {
		oss << "startafter=" << qPrintable(_uiSideBar.dtStartAfter->dateTime().toString("yyyy-MM-ddTHH:mm:00")) << "&";
	}
	if ( _uiSideBar.dtEndBefore->dateTime() > _uiSideBar.dtEndBefore->minimumDateTime() ) {
		oss << "endbefore=" << qPrintable(_uiSideBar.dtEndBefore->dateTime().toString("yyyy-MM-ddTHH:mm:00")) << "&";
	}
	if ( _uiSideBar.dtEndAfter->dateTime() > _uiSideBar.dtEndAfter->minimumDateTime() ) {
		oss << "endafter=" << qPrintable(_uiSideBar.dtEndAfter->dateTime().toString("yyyy-MM-ddTHH:mm:00")) << "&";
	}

	if ( !_uiSideBar.editNetworks->text().isEmpty() ) {
		oss << "network=" << qPrintable(_uiSideBar.editNetworks->text()) << "&";
	}

	if ( !_uiSideBar.editStations->text().isEmpty() ) {
		oss << "station=" << qPrintable(_uiSideBar.editLocations->text()) << "&";
	}

	if ( !_uiSideBar.editLocations->text().isEmpty() ) {
		oss << "location=" << qPrintable(_uiSideBar.editLocations->text()) << "&";
	}

	if ( !_uiSideBar.editChannels->text().isEmpty() ) {
		oss << "channel=" << qPrintable(_uiSideBar.editChannels->text()) << "&";
	}

	if ( _uiSideBar.comboGeoType->currentIndex() == 1 ) {
		// Bounding Box
		oss << "minlatitude=" << _uiSideBar.spinMinLatitude->value() << "&";
		oss << "maxlatitude=" << _uiSideBar.spinMaxLatitude->value() << "&";
		oss << "minlongitude=" << _uiSideBar.spinMinLongitude->value() << "&";
		oss << "maxlongitude=" << _uiSideBar.spinMaxLongitude->value() << "&";
	}
	else if ( _uiSideBar.comboGeoType->currentIndex() == 2 ) {
		// Circle
		oss << "latitude=" << _uiSideBar.spinLatitude->value() << "&";
		oss << "longitude=" << _uiSideBar.spinLongitude->value() << "&";
		oss << "minradius=" << _uiSideBar.spinMinRadius->value() << "&";
		oss << "maxradius=" << _uiSideBar.spinMaxRadius->value() << "&";
	}

	_query = oss.str();
	string query = _query + "format=text&level=channel";
	SEISCOMP_DEBUG("FDSNWS: %s", query);

	HttpRequest req;
	req.setLabelText(tr("Requesting channels"));

	int code = req.get(query.c_str());
	if ( code == 200 ) {
		clearTable();
		_ui.okButton->setEnabled(true);

		QStringList rows = QString(req.response()).split("\n");
		int idx = 0;
		int iRow = 0;

		int colNet = 0;
		int colSta = 1;
		int colLoc = 2;
		int colCha = 3;
		int colLat = 4;
		int colLon = 5;
		int colElev = 6;
		int colDepth = 7;
		int colAzimuth = 8;
		int colDip = 9;
		int colSensor = 10;
		int colSPS = 14;
		int colStart = 15;
		int colEnd = 16;

		foreach ( QString row, rows ) {
			row = row.trimmed();
			if ( row.isEmpty() ) continue;

			SEISCOMP_DEBUG("FDSNWS: %s", row.toStdString());

			if ( row[0] == '#' ) {
				// This is the very first valid line -> header
				if ( !idx ) {
					QStringList headers = row.mid(1).trimmed().split("|");
					int col = 0;

					foreach ( const QString &header, headers ) {
						if ( header == "Network" ) {
							colNet = col;
						}
						else if ( header == "Station" ) {
							colSta = col;
						}
						else if ( header == "Location" ) {
							colLoc = col;
						}
						else if ( header == "Channel" ) {
							colCha = col;
						}
						else if ( header == "Latitude" ) {
							colLat = col;
						}
						else if ( header == "Longitude" ) {
							colLon = col;
						}
						else if ( header == "Elevation" ) {
							colElev = col;
						}
						else if ( header == "Depth" ) {
							colDepth = col;
						}
						else if ( header == "Azimuth" ) {
							colAzimuth = col;
						}
						else if ( header == "Dip" ) {
							colDip = col;
						}
						else if ( header == "SensorDescription" ) {
							colSensor = col;
						}
						else if ( header == "SampleRate" ) {
							colSPS = col;
						}
						else if ( header == "StartTime" ) {
							colStart = col;
						}
						else if ( header == "EndTime" ) {
							colEnd = col;
						}

						++col;
					}

					SEISCOMP_DEBUG("FDSNWS: Header: %s", row.toStdString());
				}
			}
			else {
				QStringList columns = row.split("|");
				int columnCount = columns.count();

				if ( colNet >= columnCount
				  || colSta >= columnCount
				  || colLoc >= columnCount
				  || colCha >= columnCount
				  || colLat >= columnCount
				  || colLon >= columnCount
				  || colElev >= columnCount
				  || colDepth >= columnCount
				  || colAzimuth >= columnCount
				  || colDip >= columnCount
				  || colSPS >= columnCount
				  || colStart >= columnCount
				  || colEnd >= columnCount ) {
					SEISCOMP_WARNING("FDSNWS: Some columns indexes are out of range, got %d columns", columnCount);
					continue;
				}

				_ui.tableStations->setRowCount(iRow+1);
				_ui.tableStations->setItem(iRow, 0, new QTableWidgetItem(columns[colNet]));
				_ui.tableStations->setItem(iRow, 1, new QTableWidgetItem(columns[colSta]));
				_ui.tableStations->setItem(iRow, 2, new QTableWidgetItem(columns[colLoc]));
				_ui.tableStations->setItem(iRow, 3, new QTableWidgetItem(columns[colCha]));
				_ui.tableStations->setItem(iRow, 4, new QTableWidgetItem(columns[colSPS]));
				_ui.tableStations->setItem(iRow, 5, new QTableWidgetItem(columns[colLat]));
				_ui.tableStations->setItem(iRow, 6, new QTableWidgetItem(columns[colLon]));
				_ui.tableStations->setItem(iRow, 7, new QTableWidgetItem(columns[colElev]));
				_ui.tableStations->setItem(iRow, 8, new QTableWidgetItem(columns[colSensor]));

				++iRow;
			}

			++idx;
		}

		// Reactivate sorting
		_ui.tableStations->setSortingEnabled(true);
		_ui.tableStations->sortItems(_ui.tableStations->horizontalHeader()->sortIndicatorSection(),
		                             _ui.tableStations->horizontalHeader()->sortIndicatorOrder());

		_ui.tableStations->resizeColumnsToContents();
		_ui.tableStations->horizontalHeader()->setStretchLastSection(true);
	}
	else if ( code == 204 ) {
		clearTable();
		_ui.okButton->setEnabled(true);
	}
	else {
		// Reset query
		_query.clear();
		_ui.okButton->setEnabled(false);

		QDialog errorDlg;

		errorDlg.setWindowTitle(tr("FDSNWS Error"));

		QVBoxLayout *vl = new QVBoxLayout;

		QLabel *headline = new QLabel;
		headline->setText(tr("Status: %1").arg(code));

		QPalette pal = headline->palette();
		pal.setColor(QPalette::WindowText, Qt::darkRed);
		pal.setColor(QPalette::Window, QColor(255,224,224));
		headline->setPalette(pal);
		headline->setAutoFillBackground(true);
		headline->setMargin(9);

		vl->addWidget(headline);

		QTextEdit *edit = new QTextEdit;
		edit->setText(req.response());
		edit->setReadOnly(true);
		edit->setWordWrapMode(QTextOption::WordWrap);
		vl->addWidget(edit);

		QPushButton *closeButton = new QPushButton;
		closeButton->setText(tr("Close"));
		connect(closeButton, SIGNAL(clicked(bool)), &errorDlg, SLOT(reject()));

		QHBoxLayout *hl = new QHBoxLayout;
		hl->addStretch();
		hl->addWidget(closeButton);

		vl->addLayout(hl);

		errorDlg.setLayout(vl);
		errorDlg.exec();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
