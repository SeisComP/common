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


#ifndef SEISCOMP_GUI_EVENTLAYER_H
#define SEISCOMP_GUI_EVENTLAYER_H


#include <seiscomp/gui/map/layer.h>
#include <seiscomp/gui/map/legend.h>
#include <seiscomp/gui/datamodel/originsymbol.h>
#include <QMap>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API EventLayer : public Map::Layer {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		EventLayer(QObject* parent = nullptr);

		//! D'tor
		~EventLayer();


	// ----------------------------------------------------------------------
	//  Layer interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(const Map::Canvas *, QPainter &);
		virtual void calculateMapPosition(const Map::Canvas *canvas);
		virtual bool isInside(const QMouseEvent *event, const QPointF &geoPos);

		virtual void handleEnterEvent();
		virtual void handleLeaveEvent();
		virtual bool filterMouseMoveEvent(QMouseEvent *event, const QPointF &geoPos);
		virtual bool filterMouseDoubleClickEvent(QMouseEvent *event, const QPointF &geoPos);


	// ----------------------------------------------------------------------
	//  Slots
	// ----------------------------------------------------------------------
	public slots:
		virtual void clear();
		virtual void addEvent(Seiscomp::DataModel::Event*,bool);
		virtual void updateEvent(Seiscomp::DataModel::Event*);
		virtual void removeEvent(Seiscomp::DataModel::Event*);


	// ----------------------------------------------------------------------
	//  Signals
	// ----------------------------------------------------------------------
	signals:
		void eventHovered(const std::string &eventID);
		void eventSelected(const std::string &eventID);


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		typedef QMap<std::string, OriginSymbol*> SymbolMap;

		SymbolMap           _eventSymbols;
		mutable std::string _hoverId;
		mutable bool        _hoverChanged;
};


class EventLegend : public Map::Legend {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		EventLegend(QObject* parent = nullptr);

		//! D'tor
		~EventLegend();


	// ----------------------------------------------------------------------
	//  Legend interface
	// ----------------------------------------------------------------------
	public:
		virtual void contextResizeEvent(const QSize &size);
		virtual void draw(const QRect &rect, QPainter &p);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef QPair<QString, int> StringWithWidth;
		typedef QPair<QColor, StringWithWidth> DepthItem;
		typedef QPair<int, StringWithWidth> MagItem;

		QVector<DepthItem> _depthItems;
		QVector<MagItem> _magItems;

		int _depthWidth;
		int _magWidth;
		int _magHeight;
};


}
}


#endif
