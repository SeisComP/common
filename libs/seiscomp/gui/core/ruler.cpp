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

#include "ruler.h"

#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QStyleOptionFocusRect>

#include <algorithm>
#include <iostream>
#include <float.h>
#include <math.h>
#include <limits>

#include <seiscomp/gui/core/compat.h>
#include <seiscomp/math/math.h>

#define CHCK255(x) ((x)>255?255:((x)<0?0:(x)))


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Ruler::Ruler(QWidget *parent, Qt::WindowFlags f, Position pos)
 : QFrame(parent, f) {
	setFrameStyle(QFrame::Panel | QFrame::Plain);
	setLineWidth(0);
	setPosition(pos);
	setLimits(-std::numeric_limits<double>::max(),
	          std::numeric_limits<double>::max(),
	          0, 0);
	setLineCount(1);
	setSelectionHandleCount(2);
	setSelectionEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setLineCount(int lc, int spacing) {
	_lc = lc < 1 ? 1 : lc;
	int fontHeight = fontMetrics().height();

	_tickLong  = fontHeight/2+1;
	_tickShort = fontHeight/4+1;
	_lineSpacing = spacing;

	int h = _lc*(fontHeight + _lineSpacing) + _tickLong + 1;
	isHorizontal() ? setFixedHeight(h) : setFixedWidth(h);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setSelectionHandleCount(int cnt) {
	_selectionHandles.resize(cnt);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Ruler::selectionHandleCount() const {
	return _selectionHandles.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Ruler::selectionHandlePos(int i) const {
	return _selectionHandles[i].pos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setWheelEnabled(bool scale, bool translate) {
	_wheelScale = scale;
	_wheelTranslate = translate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setPosition(Position pos, bool allowLabelTextRotation) {
	int h = isHorizontal() ? height() : width();
	_position = pos;
	_enableLabelRotation = allowLabelTextRotation;
	if ( isHorizontal() ) {
		setMinimumWidth(0);
		setMaximumWidth(QWIDGETSIZE_MAX);
		setFixedHeight(h);
	}
	else {
		setFixedWidth(h);
		setMinimumHeight(0);
		setMaximumHeight(QWIDGETSIZE_MAX);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setReverseDirection(bool reverse) {
	_leftToRight = !reverse;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setScale(double scl) {
	_scl = scl;

	// If the scale does not fit the configured min/max resolution
	// the scale will be changed to a valid value
	if ( _limitMinRange > 0 && _scl > rulerWidth() / _limitMinRange ) _scl = rulerWidth() / _limitMinRange;
	if ( _limitMaxRange > 0 && _scl < rulerWidth() / _limitMaxRange ) {
		double diff = _max - _min;
		_min += diff / 2.0 - _limitMaxRange / 2.0;
		_max = _min + _limitMaxRange;
		_scl = rulerWidth() / _limitMaxRange;
	}

	emit scaleChanged(_scl);
	updateIntervals();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setRange(double min, double max) {
	_min = min;
	_max = max;

	if ( _autoScale ) {
		if ( _max-_min <= 0 ) {
			_min = (_min+_max)*0.5-1;
			_max = _min+2;
		}

		setScale(rulerWidth()/(_max-_min));
	}
	else {
		updateIntervals();
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::showRange(double min, double max) {
	_min = min;
	_max = max;

	if ( _max-_min > 0 )
		setScale(rulerWidth()/(_max-_min));
	else
		update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::translate(double tx) {
	_min += tx;
	_max += tx;
	updateIntervals();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Ruler::setSelected(double smin, double smax) {
	if ( _selectionHandles.count() != 2 ) return false;

	_selectionHandles[0].pos = smin;
	_selectionHandles[0].enabled = true;
	_selectionHandles[1].pos = smax;
	_selectionHandles[1].enabled = true;
	std::sort(_selectionHandles.begin(), _selectionHandles.end());
	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Ruler::setSelectionHandle(int handle, double pos) {
	if ( handle < 0 || handle >= _selectionHandles.count() )
		return false;

	_selectionHandles[handle].pos = pos;
	//std::sort(selectionHandles.begin(), selectionHandles.end());

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Ruler::setSelectionHandleEnabled(int handle, bool enable) {
	if ( handle < 0 || handle >= _selectionHandles.count() )
		return false;

	if ( _selectionHandles[handle].enabled == enable )
		return false;

	_selectionHandles[handle].enabled = enable;
	//std::sort(selectionHandles.begin(), selectionHandles.end());

	if ( handle == _currentSelectionHandle )
		_currentSelectionHandle = -1;

	update();
	return true;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setSelectionEnabled(bool enable) {
	_enableSelection = enable;
	setMouseTracking(enable);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setRangeSelectionEnabled(bool enable,
                                     bool emitRangeChangeWhileDragging) {
	_enableRangeSelection = enable;
	_emitRangeChangeWhileDragging = emitRangeChangeWhileDragging;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setAutoScaleEnabled(bool e) {
	_autoScale = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setAnnot(double da) {
	if ( _da == da) return;

	_da = _drx[0] = da;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setTicks(double dt) {
	if ( _dt == dt) return;

	_dt = _drx[1] = dt;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Ruler::minimumSelection() const {
	return _enableSelection ? _selectionHandles.front().pos : 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Ruler::maximumSelection() const {
	return _enableSelection ? _selectionHandles.back().pos : 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSize Ruler::sizeHint() const {
	if ( _position == Bottom || _position == Top )
		return QFrame::sizeHint();

	QSize size = QFrame::sizeHint();
	return QSize(size.height(), size.width());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::changed(int pos) {
	_pos = pos;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::drawSelection(QPainter &p) {
	static QPointF marker[3] = { QPointF(0, 0), QPointF(0, 0), QPointF(0, 0) };
	if ( !_enableSelection ) {
		return;
	}

	p.save();
	p.setRenderHints(QPainter::Antialiasing, true);
	int selHeight = _tickLong * 1.5;
	int selHalfWidth = selHeight * 0.5;
	int maxPaintWidth = rulerWidth() + selHalfWidth;

	p.setBrush(palette().color(QPalette::WindowText));
	for ( int i = 0; i < _selectionHandles.count(); ++i ) {
		if ( ( _hover || _dragMode > 0 ) && _enableSelection &&
		     i == _currentSelectionHandle ) continue;
		int iPos = int((_selectionHandles[i].pos-_min)*_scl);
		if ( iPos < -selHalfWidth || iPos > maxPaintWidth ) continue;
		marker[0] = r2wPos(iPos-selHalfWidth, selHeight);
		marker[1] = r2wPos(iPos+selHalfWidth, selHeight);
		marker[2] = r2wPos(iPos,0);
		p.drawPolygon(marker, 3);
	}
	if ( ( _hover || _dragMode > 0 ) && _enableSelection &&
	     _currentSelectionHandle >= 0 &&
	     _currentSelectionHandle < _selectionHandles.count() ) {
		p.setBrush(palette().color(QPalette::BrightText));
		int iPos = int((_selectionHandles[_currentSelectionHandle].pos-_min)*_scl);
		if ( iPos >= -selHalfWidth && iPos <= maxPaintWidth ) {
			marker[0] = r2wPos(iPos-selHalfWidth, selHeight);
			marker[1] = r2wPos(iPos+selHalfWidth, selHeight);
			marker[2] = r2wPos(iPos,0);
			p.drawPolygon(marker, 3);
		}
	}
	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::drawRangeSelection(QPainter &p) {
	if ( _rangemin != _rangemax ) {
		QRectF rect;
		int offset = _tickLong + 1;
		int height = fontMetrics().height()+1;

		QLinearGradient filler(r2wPos(0, offset), r2wPos(0, offset+height));
		if ( _rangemin < _rangemax ) {
			filler.setColorAt(0, QColor(0, 128, 0, 128));
			filler.setColorAt(0.5, QColor(0, 255, 0, 128));
			filler.setColorAt(1, QColor(0, 128, 0, 128));
			rect = r2wRect(_rangemin, offset, _rangemax-_rangemin, height);
		}
		else {
			filler.setColorAt(0, QColor(128, 0, 0, 128));
			filler.setColorAt(0.5, QColor(255, 0, 0, 128));
			filler.setColorAt(1, QColor(128, 0, 0, 128));
			rect = r2wRect(_rangemax, offset, _rangemin-_rangemax, height);
		}

		if ( !_rangeValid ) {
			filler.setColorAt(0, QColor(0, 0, 0, 128));
			filler.setColorAt(0.5, QColor(255, 255, 255, 128));
			filler.setColorAt(1, QColor(0, 0, 0, 128));
		}
		p.fillRect(rect, filler);
		p.drawLine(r2wPos(_rangemin, 0), r2wPos(_rangemin, offset+height-1));
		p.drawLine(r2wPos(_rangemax, 0), r2wPos(_rangemax, offset+height-1));

		p.setBrush(Qt::NoBrush);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF Ruler::r2wPos(int rx, int ry) const {
	return QPointF(
		isHorizontal() ?
			_leftToRight ?
				rx
				:
				width() - 1 - rx
			:
			isRight() ?
				ry
				:
				width() - ry - 1,
		isVertical() ?
			_leftToRight ?
				height() - 1 - rx
				:
				rx
			:
			isBottom() ?
				ry
				:
				height() - ry - 1
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF Ruler::w2rPos(int x, int y) const {
	return QPointF(
		isHorizontal() ?
			_leftToRight ?
				x
				:
				width() - 1 - x
			:
			_leftToRight ?
				height() - y - 1
				:
				y,
		isBottom() ?
			y
			:
			isTop() ?
				height() - y - 1
				:
				isRight() ?
					x
					:
					width() - x - 1
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRectF Ruler::r2wRect(int rx, int ry, int w, int h) const {
	return _leftToRight ?
		QRectF(
			isHorizontal() ?
				rx
				:
				isRight() ?
					ry
					:
					width() - ry - h - 1,
			isVertical() ?
				height() - rx - w - 1
				:
				isBottom() ?
					ry
					:
					height() - ry - h - 1,
			isHorizontal() ? w : h,
			isHorizontal() ? h : w
		)
		:
		QRectF(
			isHorizontal() ?
				width() - 1 - rx - w
				:
				isRight() ?
					ry
					:
					width() - ry - h - 1,
			isVertical() ?
				rx
				:
				isBottom() ?
					ry
					:
					height() - ry - h - 1,
			isHorizontal() ? w : h,
			isHorizontal() ? h : w
		);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Ruler::rulerDrawText(QPainter &p, int rx, int ry, const QString &text,
                          bool allowClip, bool allowRotate) const {
	// Text width and height
	auto fm = p.fontMetrics();
	int tw = fm.boundingRect(text).width() * 5 / 4;
	int th = fm.height();

	// Top/left position of text box in widget coordinates
	QPointF pos;
	if ( isHorizontal() || allowRotate ) {
		pos = r2wPos(rx-tw/2, isTop() || isLeft() ? ry+th : ry);
	}
	else {
		pos = r2wPos(rx+th/2, isRight() ? ry : ry+tw);
	}

	bool rotate = isVertical() && allowRotate;
	// Is text clipped?
	QRectF rect(0, 0, width(), height());
	QPointF pos2(pos.x() + (rotate?th:tw), pos.y() + (rotate?-tw:th));
// 	std::cout << "rotate/tw,th/rect/pos/pos2: " << rotate << " / "
// 		<< tw << "," << th << " / "
// 		<< rect.x() << "," << rect.y() << "," << rect.width() << "," << rect.height() << " / "
// 		<< pos.x() << "," << pos.y() << " / "
// 		<< pos2.x() << "," << pos2.y() << std::endl;
	if ( !allowClip && (!rect.contains(pos) || !rect.contains(pos2)) ) {
		return false;
	}

	int flags = Qt::AlignCenter | Qt::AlignBottom;
	if ( rotate ) {
		p.save();
		if ( _leftToRight ) {
			p.translate(pos);
			p.rotate(-90);
		}
		else {
			p.translate(pos + QPoint(th, 0));
			p.rotate(90);
		}
		p.drawText(-tw, 0, 3 * tw, th, flags, text);
		p.restore();
	}
	else {
		p.drawText(pos.x() - tw, pos.y(), 3 * tw, th, flags, text);
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Ruler::rulerDrawTextAtLine(QPainter &p, int rx, int line, const QString &text,
                                bool allowClip, bool allowRotate) const {
	if ( line >= _lc ) return false;

	int tickOffset = _lineSpacing + _tickLong;
	int lineHeight = fontMetrics().height() + _lineSpacing;
	if ( isHorizontal() || allowRotate )
		return rulerDrawText(p, rx, tickOffset + line*lineHeight, text, allowClip, allowRotate);
	else {
		int rxOffset = (_lc-1)/2 - line;
		return rulerDrawText(p, rx + rxOffset, tickOffset, text, allowClip, allowRotate);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::paintEvent(QPaintEvent *e) {
	QFrame::paintEvent(e);

	if ( Seiscomp::Math::isNaN(_min) ) return;
	if ( _min == std::numeric_limits<double>::infinity() ) return;
	if ( _min == -std::numeric_limits<double>::infinity() ) return;

	QPainter painter(this);
	if ( painter.device()->devicePixelRatioF() > 1.0 ) {
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.translate(0.5, 0.5);
	}

	// Draw ruler base line
	int rw = rulerWidth();
	painter.drawLine(r2wPos(0, 0), r2wPos(rw, 0));
	drawRangeSelection(painter);

	if ( _scl > 0 ) {
		for ( int k = 0; k < 2; ++k ) {
			if ( _drx[k] <= 0 ) {
				continue; // no ticks/annotations
			}

			double pos = _min + _ofs;
			double cpos = floor((pos + _drx[k] * 1E-2) / _drx[k]) * _drx[k];
			if ( fabs(cpos) < _drx[k] * 1E-2 ) {
				cpos = 0.0;
			}

			double offset = cpos;
			pos -= offset;
			cpos = 0;

			int tick = k == 0 ? _tickLong : _tickShort;

			// Draw ticks and counts
			int rx = static_cast<int>((cpos - pos) * _scl);
			QString str;
			QVector<double> lastPos(_lc, 0);

			while ( rx < rw ) {
				painter.drawLine(r2wPos(rx, 0), r2wPos(rx, tick));
				if ( k == 0 ) {
					for ( int l = 0; l < _lc; ++l ) {
						if ( getTickText(cpos + offset, lastPos[l], l, str) &&
						     rulerDrawTextAtLine(painter, rx, l, str, false, _enableLabelRotation) ) {
							lastPos[l] = cpos + offset;
						}
					}
				}

				cpos += _drx[k];
				if ( fabs(cpos) < _drx[k] * 1E-2 ) {
					cpos = 0.0;
				}

				rx = static_cast<int>((cpos - pos) * _scl);
			}
		}
	}

	drawSelection(painter);

	painter.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Ruler::getTickText(double pos, double lastPos, int line, QString &str) const {
	if ( line != 0 )
		return false;

	str.setNum(pos);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::enterEvent(QEvent *e) {
	_hover = true;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::leaveEvent(QEvent *e) {
	_hover = false;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::mousePressEvent(QMouseEvent *e) {
	// Already in a drag mode?
	if ( _dragMode != 0 ) return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QPointF rp = w2rPos(e->x(), e->y());
#else
	QPointF rp = e->position();
	rp = w2rPos(rp.x(), rp.y());
#endif
	int rx = rp.x();
	_iDragStart = rx;
	_dragStart = (_pos+rx)/_scl + _min;

	if ( e->button() == Qt::LeftButton ) {
		if ( _enableSelection && _currentSelectionHandle >= 0 )
			_dragMode = _currentSelectionHandle + 1;
		else {
			_dragMode = -1;
			emit dragStarted();
			QFrame::mousePressEvent(e);
			return;
		}

		update();
	}
	else if ( e->button() == Qt::RightButton ) {
		if ( _enableRangeSelection ) {
			_dragMode = -2;
			_rangemin = rx;
			_rangemax = rx;
			_rangeValid = false;
		}
	}

	QFrame::mousePressEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::mouseReleaseEvent(QMouseEvent *e) {
	if ( e->button() == Qt::LeftButton ) {
		if ( _dragMode == -1 )
			emit dragFinished();
		else if ( _dragMode > 0 )
			emit selectionHandleMoveFinished();
		_dragMode = 0;
	}
	else if ( e->button() == Qt::RightButton ) {
		// Reset range selection
		if ( _rangeValid ) {
			double smin = (_pos+_rangemin) / _scl + _min;
			double smax = (_pos+_rangemax) / _scl + _min;

			if ( smin < smax )
				changeRange(smin, smax);
			else {
				std::swap(smin, smax);
				double max = rulerWidth() / _scl + _min;
				double s = (max-_min) / (smax-smin);
				double tmin = s * (_min-smin) + _min;
				double tmax = s * (max-smax) + max;
				changeRange(tmin, tmax);
			}
		}
		_rangemin = _rangemax = 0;
		update();
		_dragMode = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::checkLimit(double &tmin, double &tmax) {
	tmin += _ofs;
	tmax += _ofs;

	double trange = tmax-tmin;
	double tcen = tmin + trange*0.5;

	// Clip to allowed ranges
	if ( _limitMinRange > 0 && trange < _limitMinRange ) {
		trange = _limitMinRange;
		tmin = tcen - trange*0.5;
		tmax = tcen + trange*0.5;
	}
	else if ( _limitMaxRange > 0 && trange > _limitMaxRange ) {
		trange = _limitMaxRange;
		tmin = tcen - trange*0.5;
		tmax = tcen + trange*0.5;
	}

	if ( (tmin < _limitLeft) || (tmax < _limitLeft) ) {
		tmin = _limitLeft;
		tmax = tmin + trange;
	}
	else if ( (tmin > _limitRight) || (tmax > _limitRight) ) {
		tmax = _limitRight;
		tmin = tmax - trange;
	}

	tmin -= _ofs;
	tmax -= _ofs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::changeRange(double tmin, double tmax) {
	checkLimit(tmin, tmax);
	emit rangeChangeRequested(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::mouseMoveEvent(QMouseEvent *e) {
	if ( _dragMode == 0 ) {
		if ( _enableSelection ) {
			// check if mouse is over selection handle and if so, determine best
			// matching handle and highlight it
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			QPointF rp = w2rPos(e->x(), e->y());
#else
			QPointF rp = e->position();
			rp = w2rPos(rp.x(), rp.y());
#endif
			int rx = rp.x();
			int ry = rp.y();
			int selHeight = _tickLong * 1.5;
			int selHalfWidth = selHeight * 0.5;
			int index = -1;
			if ( rx >= 0 && rx <= rulerWidth() && ry >= 0 && ry <= selHeight ) {
				int minDist = rulerWidth();
				for ( int i = 0; i < _selectionHandles.count(); ++i ) {
					if ( !_selectionHandles[i].enabled ) continue;
					int iPos = int((_selectionHandles[i].pos - _min) * _scl);
					int dist = abs(iPos - rx);
					if ( dist <= selHalfWidth && dist < minDist ) {
						minDist = dist;
						index = i;
					}
				}
			}
			if ( index != _currentSelectionHandle ) {
				_currentSelectionHandle = index;
				update();
			}
		}
		return;
	}

	// position on the ruler
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QPointF rPoint = w2rPos(e->x(), e->y());
#else
	QPointF rPoint = e->position();
	rPoint = w2rPos(rPoint.x(), rPoint.y());
#endif
	int rx = rPoint.x();
	double p = (_pos+rx) / _scl + _min;
	double dragOffset = _iDragStart - rx;

	if ( p < _limitLeft ) p = _limitLeft;
	else if ( p > _limitRight ) p = _limitRight;

	if ( _dragMode == -1 ) {
		double fDragOffset = double(dragOffset) / _scl;
		_dragStart = p;
		_iDragStart = rx;
		double tmin = _min+fDragOffset;
		double tmax = _max+fDragOffset;
		checkLimit(tmin, tmax);
		emit dragged(tmin-_min);
		if ( _enableRangeSelection && _emitRangeChangeWhileDragging )
			emit rangeChangeRequested(tmin, tmax);
	}
	else if ( _dragMode == -2 ) {
		_rangemax = rx < 0 ? 0 : rx >= rulerWidth() ? rulerWidth()-1 : rx;
		_rangeValid = _rangemax != _rangemin &&
		              rPoint.y() >= 0 && rPoint.y() < rulerHeight();
		update();
	}
	else if ( _dragMode > 0 ) {
		if ( _enableSelection ) {
			int idx = _dragMode - 1;

			if ( idx != _currentSelectionHandle ) {
				// Need to find another item as current
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				QPointF rp = w2rPos(e->x(), e->y());
#else
				QPointF rp = e->position();
				rp = w2rPos(rp.x(), rp.y());
#endif
				int rx = rp.x();
				int ry = rp.y();
				int selHeight = _tickLong * 1.5;
				int selHalfWidth = selHeight * 0.5;
				_currentSelectionHandle = -1;
				if ( rx >= 0 && rx <= rulerWidth() && ry >= 0 && ry <= selHeight ) {
					int minDist = rulerWidth();
					for ( int i = 0; i < _selectionHandles.count(); ++i ) {
						if ( !_selectionHandles[i].enabled ) continue;
						int iPos = int((_selectionHandles[i].pos - _min) * _scl);
						int dist = abs(iPos - rx);
						if ( dist <= selHalfWidth && dist < minDist ) {
							minDist = dist;
							_currentSelectionHandle = i;
						}
					}
				}

				if ( _currentSelectionHandle >= 0 ) {
					_dragMode = _currentSelectionHandle + 1;
					idx = _currentSelectionHandle;
				}
				else {
					_dragMode = 0;
					return;
				}
			}

			// Clip to previous enabled handle
			int pidx = idx-1;
			while ( pidx >= 0 ) {
				if ( _selectionHandles[pidx].enabled ) {
					if ( p < _selectionHandles[pidx].pos ) {
						p = _selectionHandles[pidx].pos;
						break;
					}
				}

				--pidx;
			}

			// Clip to next enabled handle
			int nidx = idx+1;
			while ( nidx < _selectionHandles.count() ) {
				if ( _selectionHandles[nidx].enabled ) {
					if ( p > _selectionHandles[nidx].pos ) {
						p = _selectionHandles[nidx].pos;
						break;
					}
				}

				++nidx;
			}

			if ( _selectionHandles[idx].pos != p ) {
				_selectionHandles[idx].pos = p;

				if ( _selectionHandles.count() == 2 )
					emit changedSelection(_selectionHandles[0].pos, _selectionHandles[1].pos);

				emit selectionHandleMoved(idx, _selectionHandles[idx].pos, e->modifiers());
				update();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::wheelEvent(QWheelEvent *event) {
	if ( !event || (!_wheelScale && !_wheelTranslate) )
		return;

	// shift modifier works as an amplifier
	int delta = event->modifiers() & Qt::ShiftModifier ?
	            QT_WE_DELTA(event) * 2 : QT_WE_DELTA(event) / 4;

	// scale (Ctrl key)
	if ( event->modifiers() & Qt::ControlModifier ) {
		QPointF pos = QT_WE_POS(event);
		QPointF rp = w2rPos(pos.x(), pos.y());
		double center = (double)(rp.x() + _pos) / rulerWidth();
		double ofs = delta / _scl;
		changeRange(_min + ofs * center, _max - ofs * (1 - center));
	}
	// translate
	else {
		double ofs = delta / _scl;
		changeRange(_min + ofs, _max + ofs);
	}

	event->accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::resizeEvent(QResizeEvent *e) {
	if ( _autoScale ) {
		if ( _max-_min > 0 ) {
			if ( rulerWidth() > 0 )
				setScale(rulerWidth() / (_max-_min));
		}
		else
			updateIntervals();
	}
	else
		updateIntervals();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::updateIntervals() {
	bool changed = false;
	double max = _min + rulerWidth() / _scl;
	_max = max;

	double tmpdx[] = { _da, _dt };

	for ( int k = 0; k < 2; ++k ) {
		if ( tmpdx[k] < -0.1 ) {
			changed = true;

			// compute adequate tick/annotation interval
			// the 1st factor is for fine-tuning
			double textDim = isHorizontal() ?
			                 QT_FM_WIDTH(fontMetrics(), " XX:XX:XX ") :
			                 fontMetrics().height()*2;
			double q = log10(2*(max-_min)*textDim/rulerWidth());
			double rx = q - floor(q);
			int d = rx < 0.3 ? 1 : rx > 0.7 ? 5 : 2;
			_drx[0] = d * pow (10., int(q-rx));

			switch (d) {
				case 1: _drx[1] = 0.20 * _drx[0];  break;
				case 2: _drx[1] = 0.25 * _drx[0];  break;
				case 5: _drx[1] = 0.20 * _drx[0];  break;
				default: break;
			}
		}
	}

	if ( changed )
		emit changedInterval(_drx[0], _drx[1], _ofs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Ruler::setLimits(double leftValue, double rightValue, double minRange, double maxRange) {
	_limitLeft = leftValue;
	_limitRight = rightValue;
	_limitMinRange = minRange;
	_limitMaxRange = maxRange;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Gui
} // ns Seiscomp
