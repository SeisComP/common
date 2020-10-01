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


#ifndef SEISCOMP_GUI_MAP_LAYERS_SYMBOLLAYER_H
#define SEISCOMP_GUI_MAP_LAYERS_SYMBOLLAYER_H


#include <seiscomp/gui/map/layer.h>
#include <seiscomp/gui/map/mapsymbol.h>
#include <QList>


namespace Seiscomp {
namespace Gui {
namespace Map {


class SC_GUI_API SymbolLayer : public Layer {
	// ----------------------------------------------------------------------
	//  Types
	// ----------------------------------------------------------------------
	public:
		typedef QList<Symbol*> Symbols;
		typedef Symbols::iterator iterator;
		typedef Symbols::const_iterator const_iterator;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SymbolLayer(QObject* parent = nullptr);
		//! D'tor
		~SymbolLayer();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Adds a symbol to the layer.
		 * @param symbol The symbol instance. Ownership is  transferred to
		 *               the layer
		 * @return Success flag
		 */
		bool add(Symbol *symbol);
		Symbols::const_iterator remove(Symbol *symbol);

		void clear();

		Symbols::size_type count() const;

		bool bringToTop(Symbol *drawable);
		bool sendToBack(Symbol *drawable);

		void setTop(Symbol *topDrawable);
		Symbol *top() const;

		Symbols::iterator begin();
		Symbols::iterator end();

		Symbols::const_iterator begin() const;
		Symbols::const_iterator end() const;

		void sortByLatitude();


	// ----------------------------------------------------------------------
	//  Layer interface
	// ----------------------------------------------------------------------
	public:
		virtual void setVisible(bool);
		virtual void calculateMapPosition(const Canvas *canvas);
		virtual bool isInside(const QMouseEvent *event, const QPointF &geoPos);
		virtual void draw(const Canvas *canvas, QPainter &p);


	private:
		Symbols  _symbols;
		Symbol  *_topSymbol;
};


} // namespace Map
} // namespace Gui
} // namespace Seiscomp


#endif
