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


#ifndef SEISCOMP_GUI_DECORATOR_H
#define SEISCOMP_GUI_DECORATOR_H


#include <memory>

#include <QPainter>

#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {
namespace Map {

class Canvas;


/** \class Decorator
 *
 * Abstract class to decorate a Symbol derived class. When a derived decorator is
 * passed to a symbol the map and baseObject member are automatically set by the
 * respective symbol. The same is true for all other (nested) decorator which are
 * associated whith the passed decorator. Therefore the user is obliged to assure
 * that the other decorator operate on the same type of datamodel object (pick, origin...)
 */

class SC_GUI_API Decorator {

		// ------------------------------------------------------------------
		// X'struction
		// ------------------------------------------------------------------
	public:
		Decorator(Decorator* decorator = nullptr)
		 : _decorator(decorator),
		   _visible(true) {
		}

		virtual ~Decorator() {}


		// ------------------------------------------------------------------
		// Public Interface
		// ------------------------------------------------------------------
	public:
		void draw(const Canvas *canvas, QPainter &painter) {
			if ( _decorator.get() )
				_decorator->draw(canvas, painter);
			customDraw(canvas, painter);
		}

		void setVisible(bool visible) {
			if ( _decorator.get() )
				_decorator->setVisible(visible);
			_visible = visible;
		}

		bool isVisible() const {
			return _visible;
		}


		// ------------------------------------------------------------------
		// Protected Interface
		// ------------------------------------------------------------------
	protected:
		virtual void customDraw(const Canvas *canvas, QPainter &painter) = 0;


		// ------------------------------------------------------------------
		// Private Members
		// ------------------------------------------------------------------
	private:
		std::unique_ptr<Decorator> _decorator;
		bool                       _visible;

};


} // namespace Map
} // namespace Gui
} // namespace Seiscomp

#endif
