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



#ifndef SEISCOMP_GUI_EVENTSUMMARYVIEW_H
#define SEISCOMP_GUI_EVENTSUMMARYVIEW_H


#include <QtGui>

#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/focalmechanism.h>
#include <seiscomp/datamodel/types.h>

#include <seiscomp/gui/datamodel/originlocatormap.h>
#include <seiscomp/gui/map/mapwidget.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/magnitude.h>
#endif

#include <seiscomp/gui/core/application.h>

#include <QProcess>


namespace Ui {
	class EventSummaryView;
	class Hypocenter;
}



namespace Seiscomp {

namespace DataModel {
DEFINE_SMARTPOINTER(Pick);
class DatabaseReader;
}

namespace Gui {


class MagList;
class TensorSymbol;


//!
//! magnitude-type-quality display line
//!
class SC_GUI_API MagRow : public QWidget
{
	Q_OBJECT

	public:
		MagRow(DataModel::Magnitude* netMag, bool bold, QWidget *parent = 0);
		MagRow(const std::string& type, bool bold, QWidget *parent = 0);
		~MagRow();

		void setMagnitude(DataModel::Magnitude*);
		DataModel::Magnitude* magnitude() const { return _netMag; }

		void setReferenceMagnitude(DataModel::Magnitude*);
		DataModel::Magnitude* referenceMagnitude() const { return _netMagReference; }

		void setReferenceMagnitudeVisible(bool);
		void setReferenceMagnitudeColor(QColor);

		void setVisible(bool);

		void updateContent();

	private:
		void init();
		void setBold(bool);

	private:
		QHBoxLayout* _rowsLayout;
		QLabel* _type;
		QLabel* _magnitude;
		QLabel* _magnitudeReference;
		QLabel* _quality;
		QLabel* _qualityReference;
		QLabel* _stdev;
		QLabel* _stdevReference;
		DataModel::Magnitude* _netMag;
		DataModel::Magnitude* _netMagReference;
		bool _header;
		bool _referenceMagVisible;

	friend class MagList;
};


//!
//! widget for displaying the magnitude-type-quality lines
//!
class SC_GUI_API MagList : public QWidget
{
	Q_OBJECT
	
	public:
		MagList(QWidget *parent = 0);
		~MagList();

		void addMag(DataModel::Magnitude* netMag, bool bold, bool visible);
		void addMag(const std::string& type, bool bold, bool visible);
		void updateMag(DataModel::Magnitude* netMag);
		void updateReferenceMag(DataModel::Magnitude* netMag);

		void selectMagnitude(const char *id);

		void reset();
		void clear();

		void showAll();
		void hideTypes(const std::set<std::string>&);

		void setReferenceMagnitudesVisible(bool);
		void setReferenceMagnitudesColor(QColor);

		int rowCount() { return _magRows.size(); }
		MagRow *rowAt(int i) const { return _magRows[i]; }
		MagRow* row(const std::string& type) const;

// 		QSize sizeHint() const;


	private:
		QWidget* _widget;
		QGridLayout* _mainLayout;
		QVector<MagRow*> _magRows;
		MagRow* _header;
		QColor _referenceColor;
		bool _space;
		bool _referenceMagsVisible;
};


class SC_GUI_API EventSummaryView : public QWidget
{
	Q_OBJECT

	public:
		EventSummaryView(const MapsDesc &maps,
		                 Seiscomp::DataModel::DatabaseQuery* reader,
		                 QWidget *parent = 0);
		EventSummaryView(Map::ImageTree* mapTree,
		                 Seiscomp::DataModel::DatabaseQuery* reader,
		                 QWidget * parent = 0);
		~EventSummaryView();

		void setToolButtonText(const QString&);

		void setScript0(const std::string&, bool oldStyle, bool exportMap);
		void setScript1(const std::string&, bool oldStyle, bool exportMap);

		Seiscomp::DataModel::Event* currentEvent() const;
		Seiscomp::DataModel::Origin* currentOrigin() const;
		Seiscomp::DataModel::Magnitude* currentMagnitude() const;

		OriginLocatorMap* map() const;

		bool ignoreOtherEvents() const;


	public slots:
		void addObject(const QString &parentID, Seiscomp::DataModel::Object *obj);
		void updateObject(const QString &parentID, Seiscomp::DataModel::Object *obj);
		void removeObject(const QString &parentID, Seiscomp::DataModel::Object *obj);
		void showEvent(Seiscomp::DataModel::Event* event, Seiscomp::DataModel::Origin* org = nullptr);
		//! Shows an origin that maybe does not belong to an event yet
		void showOrigin(Seiscomp::DataModel::Origin* origin);
		void updateTimeAgoLabel();
		void updateEvent();
		void deferredMapUpdate();

		void setWaveformPropagation(bool);
		void drawStations(bool);
		void drawBeachballs(bool);
		void drawFullTensor(bool);
		void setAutoSelect(bool);

		void setInteractiveEnabled(bool);

	private slots:
		void runScript0();
		void runScript1();

		void switchToAutomaticPressed();
		void showVisibleMagnitudes(bool);
		void setLastAutomaticOriginColor(QColor c);
		void setLastAutomaticFMColor(QColor c);

		void setLastAutomaticOriginVisible(bool);

		void showOnlyMostRecentEvent(bool);
		void ignoreOtherEvents(bool);

	signals:
		void toolButtonPressed();
		void requestNonFakeEvent();
		void newNofifier(Seiscomp::DataModel::Notifier *n);
		void showInStatusbar(QString, int);

	private:
		void init();
		void runScript(const QString&, const QString& name, bool oldStyle,
		               bool exportMap);

		void processEventMsg(DataModel::Event* event, DataModel::Origin* org = nullptr);

		bool setOriginParameter(std::string OriginID);
		void setPrefMagnitudeParameter(std::string MaginitudeID);
		void setOrigin(DataModel::Origin* origin);
		void setAutomaticOrigin(DataModel::Origin* origin);
		void setAutomaticFM(DataModel::FocalMechanism* fm);
		void setMagnitudeParameter(DataModel::Origin* origin);
		void setAutomaticMagnitudeParameter(DataModel::Origin* origin);
		void setFMParametersVisible(bool);
		void updateEventComment();
		void updateEventName();

		void showFocalMechanism(DataModel::FocalMechanism *fm, int ofsX, int ofsY,
		                        QColor borderColor);
		void setFM(DataModel::FocalMechanism *fm);

		void clearMagnitudeParameter();
		void clearPrefMagnitudeParameter();
		void clearOriginParameter();
		void clearAutomaticOriginParameter();
		void clearAutomaticFMParameter();
		void clearMap();
		void updateMap(bool realignView);
		void updateMagnitude(DataModel::Magnitude *mag);
		bool updateLastAutomaticOrigin(DataModel::Origin *origin);
		bool updateLastAutomaticFM(DataModel::FocalMechanism *fm);

		// for calculating map boundaries from max sta dist
		DataModel::Pick* getPick(DataModel::Arrival* arrival);
		DataModel::Station* getStation(DataModel::Pick* pick);
		void calcMinMax(double& latMin, double& latMax, double& lonMin, double& lonMax );

		std::string description(DataModel::Origin*) const;

		bool checkAndDisplay(DataModel::Event *);
		void calcOriginDistances();


	private:
		Ui::EventSummaryView *_ui;
		Ui::Hypocenter       *_uiHypocenter;
		MagList              *_magList;

		Seiscomp::DataModel::EventPtr _currentEvent;
		Seiscomp::DataModel::EventPtr _lastEvent;
		Seiscomp::DataModel::OriginPtr _currentOrigin;
		Seiscomp::DataModel::OriginPtr _lastAutomaticOrigin;
		Seiscomp::DataModel::FocalMechanismPtr _currentFocalMechanism;
		Seiscomp::DataModel::FocalMechanismPtr _lastAutomaticFocalMechanism;
		Seiscomp::DataModel::MagnitudePtr _currentNetMag;

		Seiscomp::Gui::Map::ImageTreePtr _maptree;
		OriginLocatorMap *_map;

		Seiscomp::DataModel::DatabaseQuery* _reader;

		QColor _automaticOriginColor;
		QColor _automaticFMColor;
		QColor _automaticOriginEnabledColor;
		QColor _automaticOriginDisabledColor;

		bool _interactive;
		bool _autoSelect;
		bool _displayFocMechs;
		bool _recenterMap;
		bool _recenterMapConfig;
		bool _ignoreOtherEvents;
		bool _showLastAutomaticSolution;
		bool _showOnlyMostRecentEvent;
		bool _enableFullTensor;
		int  _maxMinutesSecondDisplay;

		QTimer* _mapTimer;

		double _maxHotspotDist;
		double _minHotspotPopulation;
		std::string _hotSpotDescriptionPattern;
		std::string _script0;
		std::string _script1;
		bool        _scriptStyle0;
		bool        _scriptStyle1;
		bool        _scriptExportMap0;
		bool        _scriptExportMap1;

		std::string _displayCommentID;
		std::string _displayCommentDefault;
		bool        _displayComment;

		std::string _displayEventCommentID;
		std::string _displayEventCommentDefault;
		bool        _displayEventComment;

		//typedef std::vector< std::pair<Seiscomp::DataModel::StationPtr, double> > StationDistances;
		//StationDistances _originStations;
		size_t _originStationsIndex;

		std::set<std::string> _visibleMagnitudes;
};


}
}


#endif // of EVENTSUMMARYVIEW_H
