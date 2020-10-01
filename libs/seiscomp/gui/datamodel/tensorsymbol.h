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


#ifndef SEISCOMP_GUI_FMSYMBOL_H
#define SEISCOMP_GUI_FMSYMBOL_H


#include <seiscomp/gui/core/tensorrenderer.h>
#include <seiscomp/gui/map/mapsymbol.h>
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API TensorSymbol : public Map::Symbol
{

	public:
		TensorSymbol(const Math::Tensor2Sd &t,
		             Map::Decorator* decorator = nullptr);
		~TensorSymbol();

		void setShadingEnabled(bool);
		void setDrawConnectorEnabled(bool);
		void setBorderColor(QColor);
		void setTColor(QColor);
		void setPColor(QColor);

		void setPosition(QPointF geoPosition);
		void setOffset(QPoint offset);


	public:
		virtual bool isInside(int x, int y) const;


	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter& painter);
		void resize(int w, int h);


	protected:
		TensorRenderer     _renderer;
		QImage             _buffer;
		Math::Tensor2Sd    _tensor;
		Math::Matrix3f     _rotation;

		QSize              _lastSize;
		QPoint             _offset;

		bool               _drawLocationConnector;

};



}
}

#endif
