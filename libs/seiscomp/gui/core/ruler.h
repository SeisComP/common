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



#ifndef SEISCOMP_GUI_RULER_H
#define SEISCOMP_GUI_RULER_H

#include <seiscomp/gui/qt.h>

#include <QFrame>

namespace Seiscomp {
namespace Gui {

class SC_GUI_API Ruler : public QFrame
{
	Q_OBJECT

	public:
		enum Position { Bottom, Top, Left, Right };

		Ruler(QWidget* = 0, Qt::WindowFlags f = Qt::WindowFlags(), Position pos = Bottom);
		~Ruler() {}

		void setPosition(Position, bool allowLabelTextRotation = false);
		void setReverseDirection(bool reverse);
		void setRange(double, double);
		void setLimits(double leftValue, double rightValue, double minRange, double maxRange);
		void setScale(double);
		bool setSelected(double, double);
		bool setSelectionHandle(int handle, double pos);
		bool setSelectionHandleEnabled(int handle, bool enable);
		void setAnnot(double = -1); // -1 is "auto", 0 is none
		void setTicks(double = -1); // -1 is "auto", 0 is none

		void setSelectionHandleCount(int);

		bool isSelectionEnabled() const { return _enableSelection; }
		int selectionHandleCount() const;
		double selectionHandlePos(int) const;

		void setWheelEnabled(bool scale, bool translate);

		double minimumRange() const { return _min; }
		double maximumRange() const { return _max; }

		double minimumSelection() const;
		double maximumSelection() const;

		double scale() const { return _scl; }
		double pixelPerUnit() const { return _scl; }

		double dA() const { return _drx[0]; }
		double dT() const { return _drx[1]; }
		double dOfs() const { return _ofs; }

		virtual QSize sizeHint() const;

		//! Functions for position independent drawing
		int rulerWidth() const { return isHorizontal() ? width() : height(); }
		int rulerHeight() const { return isHorizontal() ? height() : width(); }

		bool isBottom() const { return _position == Bottom; }
		bool isTop() const { return _position == Top; }
		bool isLeft() const { return _position == Left; }
		bool isRight() const { return _position == Right; }
		bool isHorizontal() const { return isBottom() || isTop(); }
		bool isVertical() const { return !isHorizontal(); }


	public slots:
		void showRange(double, double);
		void translate(double);
		void setAutoScaleEnabled(bool);
		void setSelectionEnabled(bool);

		void setRangeSelectionEnabled(
		       bool enabled,
		       bool emitRangeChangeWhileDragging = false
		);

		void changed(int pos = -1);


	signals:
		void scaleChanged(double);
		void changedSelection(double, double);
		void selectionHandleMoved(int handle, double pos, Qt::KeyboardModifiers);
		void selectionHandleMoveFinished();
		void changedInterval(double dA, double dT, double ofs);
		void dragged(double diffTime);
		void dragStarted();
		void dragFinished();

		void rangeChangeRequested(double tmin, double tmax);


	protected:
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);

		//! Should be reimplemented in derived classes to
		//! customize the displayed string. str holds the string
		//! used for displaying and value describes the position
		//! on the ruler.
		virtual bool getTickText(double pos, double lastPos,
		                         int line, QString &str) const;
		virtual void updateIntervals();

		void enterEvent(QEvent *e);
		void leaveEvent(QEvent *e);

		void setLineCount(int lines, int spacing = 4);
		virtual void drawSelection(QPainter &p);
		void drawRangeSelection(QPainter &p);

		//! Converts ruler position to point in widget coordinates, rx is the
		//! position on the ruler, ry the distance from the rulers baseline
		QPointF r2wPos(int rx, int ry) const;
		//! Converts widget coordinates to ruler position
		QPointF w2rPos(int x, int y) const;
		//! Converts ruler rectangle to rectangle in widget coordinates.
		//! rx is the position on the ruler, ry the distance from the rulers
		//! baseline
		QRectF r2wRect(int rx, int ry, int rw, int rh) const;
		//! Draws text at the specified ruler position (rx) with
		//! the specified distance (ry) from the rulers baseline.
		//! If allowRotate is set to 'true' the text is rotated
		//! by 90 degrees if the ruler is in a vertical mode.
		bool rulerDrawText(QPainter &p, int rx, int ry,
		                   const QString &text, bool allowClip = false,
		                   bool allowRotate = false) const;
		bool rulerDrawTextAtLine(QPainter &p, int rx, int line,
		                         const QString &text, bool allowClip = false,
		                         bool allowRotate = false) const;

		void checkLimit(double &tmin, double &tmax);
		void changeRange(double tmin, double tmax);

	protected:
		struct Handle {
			Handle() : enabled(true) {}
			double pos;
			bool   enabled;

			bool operator<(const Handle &other) const {
				return pos < other.pos;
			}
		};

		Position _position;

		double  _ofs{0};
		double  _scl{1.0},
		        _min{0}, _max{0},       // ruler range
		        _da{-1},              // annotation interval
		        _dt{-1},              // tick mark interval
		        _limitLeft, _limitRight,
		        _limitMinRange, _limitMaxRange;
		int     _pos{0}, _tickLong, _tickShort, _lc, _lineSpacing;
		QVector<Handle> _selectionHandles;
		int     _currentSelectionHandle{-1};

		double  _drx[2];   // current intervals

		int     _dragMode{0};
		double  _dragStart;
		int     _iDragStart;

		int     _rangemin{0}, _rangemax{0};
		bool    _rangeValid;

		bool    _enableSelection;
		bool    _enableRangeSelection{false};
		bool    _enableLabelRotation{false};
		bool    _leftToRight{true}; // Or bottomToTop
		bool    _emitRangeChangeWhileDragging{false};
		bool    _hover{false};
		bool    _wheelScale{true};
		bool    _wheelTranslate{true};
		bool    _autoScale{false};
};


} // ns Gui
} // ns Seiscomp

# endif // _RULER_H_
