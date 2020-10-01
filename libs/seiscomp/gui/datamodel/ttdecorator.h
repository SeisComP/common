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


#ifndef SEISCOMP_GUI_TTDECORATOR_H
#define SEISCOMP_GUI_TTDECORATOR_H


#include <list>

#include <seiscomp/math/geo.h>
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/datamodel/timequantity.h>
#include <seiscomp/gui/map/mapwidget.h>
#include <seiscomp/gui/qt.h>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API TTDecorator : public Map::Decorator {
	// ------------------------------------------------------------------
	// Nested Types
	// ------------------------------------------------------------------
	private:
		typedef std::vector<double> TravelTimes;
		enum Direction { NORTH_SOUTH, EAST_WEST };


	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		TTDecorator(Map::Decorator *decorator = nullptr);

		void setPreferredMagnitudeValue(double val);

		double longitude() const;
		void setLongitude(double longitude);

		double latitude() const;
		void setLatitude(double latitude);

		double depth() const;
		void setDepth(double depth);

		const DataModel::TimeQuantity& originTime() const;
		void setOriginTime(const DataModel::TimeQuantity& time);


	// ------------------------------------------------------------------
	// Protected Interface
	// ------------------------------------------------------------------
	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);


	// ------------------------------------------------------------------
	// Private Interface
	// ------------------------------------------------------------------
	private:
		void computeTTT(TravelTimes& travelTimes, const std::string& phase,
		                double phaseDistance);

		double computeTTTPolygon(const std::vector<double>& travelTimes,
		                         std::vector<QPointF>& polygon);

		void drawPolygon(const Map::Canvas *canvas, QPainter& painter,
		                 const std::vector<QPointF>& polygon);

		void annotatePropagation(const Map::Canvas *canvas, QPainter& painter,
		                         double distance, Direction direction);

	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		std::vector<QPointF> _polygonP;
		std::vector<QPointF> _polygonS;
		TravelTimeTable      _ttTable;
		TravelTimes          _travelTimesP;
		TravelTimes          _travelTimesS;
		int                  _deltaDepth;
		double               _maxPDistance;
		double               _maxSDistance;
		double               _pDistance;
		int                  _deltaDist;
		int                  _rotDelta;
		double               _preferredMagnitudeVal;
		double               _longitude;
		double               _latitude;
		double               _depth;
		DataModel::TimeQuantity _originTime;

};


} // namespace Gui
} // namespace Seiscomp

#endif
