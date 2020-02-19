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



#include <seiscomp/gui/datamodel/tensorsymbol.h>
#include <seiscomp/gui/map/canvas.h>
#include <seiscomp/gui/map/projection.h>
#include <seiscomp/math/conversions.h>
#include <QPainter>


namespace Seiscomp {
namespace Gui {


TensorSymbol::TensorSymbol(const Math::Tensor2Sd &t,
                           Map::Decorator* decorator)
: Symbol(decorator) {
	_tensor = t;
	Math::tensor2matrix(_tensor, _rotation);
	_renderer.setShadingEnabled(false);
	_lastSize = QSize(-1, -1);
	_drawLocationConnector = true;
	
}


TensorSymbol::~TensorSymbol() {}


void TensorSymbol::setShadingEnabled(bool e) {
	_renderer.setShadingEnabled(e);
}


void TensorSymbol::setDrawConnectorEnabled(bool e) {
	_drawLocationConnector = e;
}


void TensorSymbol::setBorderColor(QColor c) {
	_renderer.setBorderColor(c);
}


void TensorSymbol::setTColor(QColor c) {
	_renderer.setTColor(c);
}


void TensorSymbol::setPColor(QColor c) {
	_renderer.setPColor(c);
}


void TensorSymbol::setPosition(QPointF geoPosition) {
	setLocation(geoPosition);
}


void TensorSymbol::setOffset(QPoint offset) {
	_offset = offset;
}


void TensorSymbol::resize(int w, int h) {
	setSize(QSize(w,h));
	_buffer = QImage(size(), QImage::Format_ARGB32);
	_renderer.render(_buffer, _tensor, _rotation);
}


bool TensorSymbol::isInside(int x, int y) const {
	return false;
}


void TensorSymbol::customDraw(const Map::Canvas *, QPainter &p) {
	if ( size() != _lastSize ) {
		_lastSize = size();
		resize(_lastSize.width(), _lastSize.height());
	}

	if ( _drawLocationConnector ) {
		p.setPen(QPen(Qt::black, 1, Qt::DashLine));
		p.drawLine(pos(), pos() + _offset);
	}

	p.drawImage(pos() + _offset - QPoint(_size.width()/2, _size.height()/2), _buffer);
}


}
}
