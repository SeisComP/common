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



#ifndef SEISCOMP_GUI_EVENTSUMMARY_H
#define SEISCOMP_GUI_EVENTSUMMARY_H

#include <string>
#include <set>

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/focalmechanism.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/databasequery.h>
#endif
#include <seiscomp/gui/core/gradient.h>
#include <seiscomp/gui/map/mapwidget.h>
#include <seiscomp/gui/qt.h>


namespace Ui {
	class EventSummary;
}


namespace Seiscomp {
namespace Gui {

class OriginSymbol;

class SC_GUI_API EventSummaryMagnitudeRow : public QHBoxLayout {
	Q_OBJECT

	public:
		EventSummaryMagnitudeRow(const std::string &type,
		                         QWidget *parent = 0);

		void reset();
		void select(bool);

		void set(const std::string &id, const double *value, int stationCount);

	signals:
		void clicked(const std::string &magID);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		void setMagnitude(const double *value, int stationCount);

	public:
		std::string magnitudeID;
		QLabel *label;
		QLabel *value;
};

class SC_GUI_API EventSummary : public QWidget {
	Q_OBJECT

	public:
		EventSummary(const MapsDesc &maps,
		             DataModel::DatabaseQuery* reader,
		             QWidget *parent = 0);
		EventSummary(Map::ImageTree* mapTree,
		             DataModel::DatabaseQuery* reader,
		             QWidget * parent = 0);
		~EventSummary();

	public:
		DataModel::Event *currentEvent() const;
		DataModel::Origin *currentOrigin() const;

		//! Negative value switches to default behaviour
		void setDefaultEventRadius(double radius);
		void setSecondDisplayUpToMaxMinutes(int);

		void addVisibleMagnitudeType(const std::string &mag);
		QList<QFrame*> separators() const;

	signals:
		void selected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*);
		void magnitudeSelected(const std::string &magnitudeID);

	public slots:
		void addObject(const QString& parentID, Seiscomp::DataModel::Object* obj);
		void updateObject(const QString& parentID, Seiscomp::DataModel::Object* obj);
		void removeObject(const QString& parentID, Seiscomp::DataModel::Object* obj);

		void updateOrigin(Seiscomp::DataModel::Origin* origin);

		void setEvent(Seiscomp::DataModel::Event *event,
		              Seiscomp::DataModel::Origin* org = nullptr,
		              bool fixed = false);

		void showOrigin(Seiscomp::DataModel::Origin*);

	public:
		QPushButton *exportButton() const;
		MapWidget *mapWidget() const;


	private slots:
		void updateTimeAgo();
		void magnitudeClicked(const std::string &magnitudeID);

	private:
		void init();
		void setTextContrast(bool);

		void mapClicked();

		void setFocalMechanism(DataModel::FocalMechanism*);

		void setOrigin(DataModel::Origin *origin);
		void setOrigin(const std::string &originID);

		void setMagnitude(const DataModel::Magnitude *mag);
		void selectMagnitude(const std::string &type);

		void resetContent();
		void resetMagnitudes();

		void updateContent();
		void updateMagnitude();
		void updateOrigin();
		void updateAlert();

	private:
		struct AlertSettings {
			AlertSettings() : textSize(-1) {}

			bool empty() { return commentId.empty(); }

			std::string                 commentId;
			std::vector<std::string >   commentBlacklist;

			int                         textSize;
			Gui::Gradient               gradient;
		};

	private:
		::Ui::EventSummary *_ui;
		Map::ImageTreePtr _maptree;
		MapWidget *_map;

		QTimer _timeAgo;

		DataModel::DatabaseQuery*        _reader;

		DataModel::EventPtr              _currentEvent;
		DataModel::OriginPtr             _currentOrigin;
		DataModel::MagnitudeCPtr         _currentMag;
		DataModel::FocalMechanismPtr     _currentFocalMechanism;

		OriginSymbol *_symbol;

		QBoxLayout *_magnitudeRows;

		typedef std::set<std::string> MagnitudeTypes;
		typedef std::map<std::string, EventSummaryMagnitudeRow*> MagnitudeList;

		MagnitudeTypes                   _visibleMagnitudes;
		MagnitudeList                    _magnitudes;
		AlertSettings                    _alertSettings;

		bool                             _alertActive;
		bool                             _fixedView;
		bool                             _showComment;

		double                           _defaultEventRadius;
		int                              _maxMinutesSecondDisplay;

	friend class EventSummaryMap;
};


}
}

#endif
