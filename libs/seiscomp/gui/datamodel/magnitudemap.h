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


#ifndef SEISCOMP_GUI_MAGNITUDEMAP_H
#define SEISCOMP_GUI_MAGNITUDEMAP_H


#include <QtGui>
#include <seiscomp/gui/map/mapwidget.h>
#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {

namespace DataModel {
DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(Magnitude);
class StationMagnitude;
DEFINE_SMARTPOINTER(Station);
}

namespace Gui {

namespace Map {

class AnnotationLayer;

}

class SC_GUI_API MagnitudeMap : public MapWidget {
	Q_OBJECT

	public:
		MagnitudeMap(const MapsDesc &maps,
		             QWidget *parent = 0, Qt::WindowFlags f = 0);

		MagnitudeMap(Map::ImageTree* mapTree,
		             QWidget *parent = 0, Qt::WindowFlags f = 0);

		~MagnitudeMap();

		//! Sets the maximum distance for stations to be displayed
		//! if they are not part of the origin
		void setStationsMaxDist(double);

		//! Make the station state changeable interactively
		//! The default is TRUE
		void setStationsInteractive(bool);
		void setOrigin(DataModel::Origin* o);
		void setMagnitude(DataModel::Magnitude* nm);

		void setMagnitudeState(int id, bool state);
		void setStationState(const std::string& code, bool state);

		void addStationMagnitude(DataModel::StationMagnitude* staMag, int index);


	public slots:
		void setDrawStations(bool);
		void setDrawStationAnnotations(bool);


	signals:
		void clickedMagnitude(int id);
		void clickedStation(const std::string &net, const std::string &code);
		void magnitudeChanged(int id, bool state);
		void stationChanged(const std::string &code, bool state);


	protected:
		void mousePressEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);


	private:
		void addMagnitude(int stationId, int magId);
		void setStationState(int i, bool state);
		void setStationResidual(int i, double residual);
		void setMagnitudeResidual(int id, double residual);
		int findStation(const std::string& stationCode) const;
		int addStation(const std::string &net, const std::string &sta);
		void sortSymbols();


	private:
		DataModel::OriginPtr       _origin;
		Map::Layer                *_symbolLayer;
		Map::AnnotationLayer      *_annotationLayer;
		DataModel::MagnitudePtr    _magnitude;
		QVector<int>               _magnitudes;
		std::map<std::string, int> _stationCodes;
		int                        _hoverId{-1};
		double                     _stationsMaxDist{-1};
};


}
}


#endif
