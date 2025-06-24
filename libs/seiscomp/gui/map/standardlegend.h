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


#ifndef SEISCOMP_GUI_MAP_STANDARDLEGEND_H
#define SEISCOMP_GUI_MAP_STANDARDLEGEND_H


#include <seiscomp/gui/map/legend.h>


#include <QPen>
#include <QBrush>
#include <QRect>


namespace Seiscomp::Gui::Map {


class StandardLegend;


class SC_GUI_API StandardLegendItem {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		StandardLegendItem(StandardLegend *legend = nullptr);
		StandardLegendItem(QPen p, QString l);
		StandardLegendItem(QPen p, QString l, int s);
		StandardLegendItem(QPen p, const QBrush &b, QString l);
		StandardLegendItem(QPen p, const QBrush &b, QString l, int s);
		StandardLegendItem(QPixmap sym, QString l);
		StandardLegendItem(QPixmap sym, QString l, int s);

		virtual ~StandardLegendItem() = default;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(QPainter *painter, const QRect &symbolRect,
		                  const QRect &textRect);


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		void drawSymbol(QPainter *painter, const QRect &rec);
		void drawText(QPainter *painter, const QRect &rec);


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		QPen    pen;
		QBrush  brush;
		QPixmap symbol;
		QString label;
		int     size{-1};
};


class SC_GUI_API StandardLegend : public Legend {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		StandardLegend(QObject *parent);
		//! D'tor
		~StandardLegend() override;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Add an item to the legend. The legend will take the ownership of
		//! the item.
		void addItem(StandardLegendItem *item);

		//! Removes all items in the legend.
		void clear();

		//! Returns the number of items in the legend.
		int count();

		//! Returns the item at index
		StandardLegendItem *itemAt(int index) const;

		//! Removes and returns the item from the given index in the leged;
		//! otherwise returns 0. Items taken from a legend will not be managed
		//! by the legend and will need to be deleted manually.
		StandardLegendItem *takeItem(int index);

		void setMaximumColumns(int columns);
		void setOrientation(Qt::Orientation orientation);

		//! This method needs to be called if the configuration of the
		//! legend has changed to update its layout. Otherwise the update
		//! will be delayed to the second render pass and might cause
		//! one frame of distored rendering.
		void update();


	// ----------------------------------------------------------------------
	//  Legend interface
	// ----------------------------------------------------------------------
	public:
		void contextResizeEvent(const QSize &size) override;
		void draw(const QRect &r, QPainter &p) override;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void updateLayout(const QSize &size);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		Qt::Orientation            _orientation;
		QList<StandardLegendItem*> _items;
		int                        _columns;
		int                        _columnWidth;
		QSize                      _symbolSize;
		int                        _maxColumns;
		bool                       _layoutDirty;

};


inline int StandardLegend::count() {
	return _items.count();
}

inline StandardLegendItem *StandardLegend::itemAt(int index) const {
	return _items[index];
}


} // namespace Seiscomp::Gui::Map


#endif
