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


#include <QPainter>
#include <QToolTip>

#define SEISCOMP_COMPONENT Gui::RecordWidget
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/math.h>
#include <seiscomp/math/filter/butterworth.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/utils.h>

using namespace std;
using namespace Seiscomp;

#include "recordwidget.h"

namespace  sc = Seiscomp::Core;

#define CHCK255(x) ((x)>255?255:((x)<0?0:(x)))

#define CHCK_RANGE                                   \
	double diff = _tmax - _tmin;                     \
	if ( _tmin < sc::Time::MinTime ) {           \
		_tmin = sc::Time::MinTime;               \
		_tmax = _tmin + diff;                        \
	}                                                \
	                                                 \
	if ( _tmax > sc::Time::MaxTime ) {           \
		_tmax = sc::Time::MaxTime;               \
		_tmin = _tmax - diff;                        \
		if ( _tmin < sc::Time::MinTime ) {       \
			_tmin = sc::Time::MinTime;           \
			diff = _tmax - _tmin;                    \
			_pixelPerSecond = canvasWidth() / diff;  \
		}                                            \
	}                                                \


namespace {


bool minmax(const ::RecordSequence *seq, const Core::TimeWindow &tw,
            double &ofs, double &min, double &max, bool globalOffset = false) {
	ofs = 0;
	double tmpOfs = 0;
	int sampleCount = 0;
	int offsetSampleCount = 0;
	bool isFirst = true;
	auto it = tw ? seq->lowerBound(tw.startTime()) : seq->begin();
	min = max = 0;

	if ( globalOffset ) {
		// Compute pre time window offset if required
		for ( auto oit = seq->begin(); oit != it; ++oit ) {
			RecordCPtr rec = (*it);
			int ns = rec->sampleCount();
			if ( !ns || !rec->data() ) {
				continue;
			}

			auto dataType = rec->data()->dataType();
			if ( dataType == Array::FLOAT ) {
				auto arr = static_cast<const FloatArray*>(rec->data());
				for ( int i = 0; i < ns; ++i ) {
					tmpOfs += (*arr)[i];
				}
			}
			else if ( dataType == Array::DOUBLE ) {
				auto arr = static_cast<const DoubleArray*>(rec->data());
				for ( int i = 0; i < ns; ++i ) {
					tmpOfs += (*arr)[i];
				}
			}
			else if ( dataType == Array::INT ) {
				auto arr = static_cast<const IntArray*>(rec->data());
				for ( int i = 0; i < ns; ++i ) {
					tmpOfs += (*arr)[i];
				}
			}
			else {
				continue;
			}

			offsetSampleCount += ns;
		}
	}

	for ( ; it != seq->end(); ++it ) {
		RecordCPtr rec = (*it);
		int imin = 0, imax = 0;
		int ns = rec->sampleCount();
		if ( ns == 0 || rec->data() == nullptr ) {
			continue;
		}

		auto dataType = rec->data()->dataType();

		if ( tw ) { // limit search for min/max to specified time window
			try {
				if ( rec->startTime() < tw.endTime() ) {
					double fs = rec->samplingFrequency();
					double dt = static_cast<double>(tw.startTime() - rec->startTime());
					if ( dt > 0 ) {
						imin = int(dt*fs);
					}

					dt = static_cast<double>(rec->endTime() - tw.endTime());
					imax = ns;
					if ( dt > 0 ) {
						imax -= int(dt*fs);
					}
				}
				else {
					break;
				}
			}
			catch ( ... ) {
				break;
			}
		}
		else { // no time window specified -> search over whole record
			imax = ns;
		}

		sampleCount += imax - imin;

		double xmin, xmax;

		if ( dataType == Array::FLOAT ) {
			float tmpMin, tmpMax;
			::minmax(ns, static_cast<const FloatArray*>(rec->data())->typedData(), imin, imax, &tmpMin, &tmpMax);
			xmin = tmpMin;
			xmax = tmpMax;
		}
		else if ( dataType == Array::INT ) {
			int tmpMin, tmpMax;
			::minmax(ns, static_cast<const IntArray*>(rec->data())->typedData(), imin, imax, &tmpMin, &tmpMax);
			xmin = tmpMin;
			xmax = tmpMax;
		}
		else if ( dataType == Array::DOUBLE ) {
			::minmax(ns, static_cast<const DoubleArray*>(rec->data())->typedData(), imin, imax, &xmin, &xmax);
		}
		else {
			continue;
		}

		if ( dataType == Array::FLOAT ) {
			auto arr = static_cast<const FloatArray*>(rec->data());
			for ( int i = imin; i < imax; ++i ) {
				tmpOfs += (*arr)[i];
			}
		}
		else if ( dataType == Array::DOUBLE ) {
			auto arr = static_cast<const DoubleArray*>(rec->data());
			for ( int i = imin; i < imax; ++i ) {
				tmpOfs += (*arr)[i];
			}
		}
		else if ( dataType == Array::INT ) {
			auto arr = static_cast<const IntArray*>(rec->data());
			for ( int i = 0; i < ns; ++i ) {
				tmpOfs += (*arr)[i];
			}
		}

		offsetSampleCount = sampleCount;

		if( isFirst ) {
			min = xmin;
			max = xmax;
			isFirst = false;
		}
		else {
			if ( xmin < min ) {
				min = xmin;
			}

			if ( xmax > max ) {
				max = xmax;
			}
		}
	}

	if ( globalOffset ) {
		// Compute pre time window offset if required
		for ( auto oit = it; oit != seq->end(); ++oit ) {
			RecordCPtr rec = (*it);
			int ns = rec->sampleCount();
			if ( !ns || !rec->data() ) {
				continue;
			}

			auto dataType = rec->data()->dataType();
			if ( dataType == Array::FLOAT ) {
				auto arr = static_cast<const FloatArray*>(rec->data());
				for ( int i = 0; i < ns; ++i ) {
					tmpOfs += (*arr)[i];
				}
			}
			else if ( dataType == Array::DOUBLE ) {
				auto arr = static_cast<const DoubleArray*>(rec->data());
				for ( int i = 0; i < ns; ++i ) {
					tmpOfs += (*arr)[i];
				}
			}
			else if ( dataType == Array::INT ) {
				auto arr = static_cast<const IntArray*>(rec->data());
				for ( int i = 0; i < ns; ++i ) {
					tmpOfs += (*arr)[i];
				}
			}
			else {
				continue;
			}

			offsetSampleCount += ns;
		}
	}

	tmpOfs /= (offsetSampleCount ? offsetSampleCount : 1);
	ofs = tmpOfs;

	return sampleCount > 0;
}


QString valueToString(bool showEngineeringValues, double value, int precision = -1) {
	if ( precision < 0 ) {
		return showEngineeringValues ?
			Gui::numberToEngineering(value)
		:
			QString("%1 ").arg(value)
		;
	}
	else {
		return showEngineeringValues ?
			Gui::numberToEngineering(value, precision)
		:
			QString("%1 ").arg(value, 0, 'f', precision)
		;
	}
}


void updateVerticalAxis(double spacing[2], double rangeLower, double rangeUpper,
                        int dim, int textDim) {
	// compute adequate tick/annotation interval
	// the 1st factor is for fine-tuning
	double q = log10(2 * std::abs(rangeUpper-rangeLower) * textDim / dim);
	double rx = q - floor(q);
	int d = rx < 0.3 ? 1 : rx > 0.7 ? 5 : 2;
	spacing[0] = d * pow (10., int(q-rx));

	switch ( d ) {
		case 1:
			spacing[1] = 0.20 * spacing[0];
			break;
		case 2:
			spacing[1] = 0.25 * spacing[0];
			break;
		case 5:
			spacing[1] = 0.20 * spacing[0];
			break;
		default:
			spacing[1] = -1;
			break;
	}
}


void drawVerticalAxis(QPainter &p, double rangeLower, double rangeUpper,
                      double spacing[2], const QRect &rect, int tickLength,
                      const QString &label, bool leftAligned,
                      const QPen &fg, const QPen &grid,
                      int gridLeft, int gridRight, bool showEngineeringValues) {
	double direction = 1;

	if ( rangeLower > rangeUpper ) {
		direction = -1;
	}

	double axisRange = std::abs(rangeUpper - rangeLower);
	int h = rect.height();

	double ppa = (h - 1) / axisRange * direction;
	int baseLine, tickDir, labelFlags, labelPos, labelOffset;

	labelOffset = tickLength*3/2;

	if ( leftAligned ) {
		baseLine = rect.right();
		tickDir = -1;
		labelFlags = Qt::AlignVCenter | Qt::AlignRight;
		labelPos = rect.right() - labelOffset;
	}
	else {
		baseLine = rect.left();
		tickDir = 1;
		labelFlags = Qt::AlignVCenter | Qt::AlignLeft;
		labelPos = rect.left() + labelOffset;
	}

	// Draw baseline
	p.setPen(fg);
	p.drawLine(baseLine, rect.top(), baseLine, rect.bottom());

	if ( !label.isEmpty() ) {
		QRect labelRect = p.fontMetrics().boundingRect(label);
		if ( labelRect.width() < h ) {
			p.save();
			if ( leftAligned ) {
				labelRect.moveLeft(-labelRect.width()/2);
				labelRect.moveTop(0);
				p.translate(rect.left(), rect.center().y());
				p.rotate(-90);
				p.drawText(labelRect, Qt::AlignHCenter | Qt::AlignBottom, label);
			}
			else {
				labelRect.moveLeft(-labelRect.width()/2);
				labelRect.moveTop(0);
				p.translate(rect.right(), rect.center().y());
				p.rotate(90);
				p.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, label);
			}
			p.restore();
		}
	}

	for ( int k = 0; k < 2; ++k ) {
		if ( spacing[k] <= 0 ) continue; // no ticks/annotations

		double cpos = rangeLower - fmod(rangeLower, spacing[k]);
		if ( fabs(cpos) < spacing[k] * 1E-2 )
			cpos = 0.0;

		int tick = (k == 0 ? tickLength : tickLength / 2) * tickDir;

		// Draw ticks and counts
		int ry = rect.bottom() - static_cast<int>((cpos - rangeLower) * ppa);
		QString str;

		p.setPen(fg);

		int lastLabelTop = rect.bottom();

		while ( ry >= rect.top() ) {
			p.drawLine(baseLine, ry, baseLine + tick, ry);

			if ( k == 0 ) {
				str = valueToString(showEngineeringValues, cpos);
				QRect labelRect = p.fontMetrics().boundingRect(str);
				// Safety margin to not cut text
				labelRect.adjust(0, 0, labelRect.width(), 0);
				if ( leftAligned ) {
					labelRect.moveRight(labelPos);
				}
				else {
					labelRect.moveLeft(labelPos);
				}
				labelRect.moveTop(ry - labelRect.height() / 2);

				if ( labelRect.top() < rect.bottom() &&
				     labelRect.bottom() > rect.top() ) {
					if ( labelRect.bottom() > rect.bottom() ) {
						labelRect.moveBottom(rect.bottom());
					}
					else if ( labelRect.top() < rect.top() ) {
						labelRect.moveTop(rect.top());
					}

					if ( labelRect.bottom() <= lastLabelTop ) {
						p.drawText(labelRect, labelFlags, str);
						lastLabelTop = labelRect.top();
					}
				}
			}

			cpos += spacing[k] * direction;
			if ( std::abs(cpos) < spacing[k] * 1E-2 ) {
				cpos = 0.0;
			}

			ry = rect.bottom() - static_cast<int>((cpos - rangeLower) * ppa);
		}

		if ( k == 0 ) {
			p.setPen(grid);

			cpos = rangeLower - fmod(rangeLower, spacing[k]);
			ry = rect.bottom() - static_cast<int>((cpos - rangeLower) * ppa);
			while ( ry >= rect.top() ) {
				p.drawLine(gridLeft, ry, gridRight, ry);
				cpos += spacing[k] * direction;
				ry = rect.bottom() - static_cast<int>((cpos - rangeLower) * ppa);
			}
		}
	}
}


void flip(Gui::RecordPolyline &pl, int height) {
	for ( int i = 0; i < pl.count(); ++i ) {
		QPolygon &poly = pl[i];
		for ( int j = 0; j < poly.count(); ++j ) {
			poly[j].setY(height - poly[j].y());
		}
	}
}


void flip(Gui::RecordPolylineF &pl, int height) {
	for ( int i = 0; i < pl.count(); ++i ) {
		QPolygonF &poly = pl[i];
		for ( int j = 0; j < poly.count(); ++j ) {
			poly[j].setY(height - poly[j].y());
		}
	}
}


template <typename T>
bool isNegative(const T *v) {
	if ( !v ) {
		return false;
	}
	return *v < 0;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Gui {


RecordMarker::RecordMarker(RecordWidget *parent,
                           const Seiscomp::Core::Time& pos,
                           Qt::Alignment alignment)
: _parent(nullptr), _time(pos), _correctedTime(pos), _visible(true),
  _moveable(false), _moveCopy(false), _enabled(true), _id(-1), _alignment(alignment) {
	if ( parent )
		parent->addMarker(this);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker::RecordMarker(RecordWidget *parent,
                           const Seiscomp::Core::Time& pos,
                           const QString& text,
                           Qt::Alignment alignment)
: _parent(nullptr), _time(pos), _correctedTime(pos), _text(text), _visible(true),
  _moveable(false), _moveCopy(false), _enabled(true), _id(-1), _alignment(alignment) {
	if ( parent )
		parent->addMarker(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker::RecordMarker(RecordWidget *parent, const RecordMarker& m)
 : _parent(nullptr),
   _color(m._color),
   _modifierColor(m._modifierColor),
   _time(m._time),
   _correctedTime(m._correctedTime),
   _text(m._text),
   _description(m._description),
   _visible(m._visible),
   _moveable(m._moveable),
   _moveCopy(m._moveCopy),
   _enabled(m._enabled),
   _id(m._id),
   _alignment(m._alignment),
   _data(m._data) {
	if ( parent )
		parent->addMarker(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker::~RecordMarker() {
	if ( _parent )
		_parent->takeMarker(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setParent(RecordWidget* p) {
	_parent = p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget* RecordMarker::parent() const {
	return _parent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setCorrectedTime(const Seiscomp::Core::Time& t) {
	if ( !_moveable ) return;
	_correctedTime = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setColor(QColor c) {
	_color = c;
	_modifierColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordMarker::color() const {
	return _color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setAlignment(Qt::Alignment al) {
	_alignment = al;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::Alignment RecordMarker::alignment() const {
	return _alignment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setModifiedColor(QColor c) {
	_modifierColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordMarker::modifiedColor() const {
	return _modifierColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::Time& RecordMarker::time() const {
	return _time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::Time& RecordMarker::correctedTime() const {
	return _correctedTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setText(const QString &t) {
	_text = t;
	_aliases.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& RecordMarker::text() const {
	return _text;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::addAlias(const QString &alias) {
	_aliases.append(alias);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setDescription(const QString &desc) {
	_description = desc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::matches(const QString &text) const {
	if ( _text == text ) return true;
	return _aliases.contains(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& RecordMarker::description() const {
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& RecordMarker::renderText() const {
	return _description.isEmpty()?_text:_description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setVisible(bool visible) {
	if ( _visible == visible ) {
		return;
	}

	_visible = visible;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isVisible() const {
	return _visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isHidden() const {
	return !_visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setMovable(bool enable) {
	_moveable = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isMovable() const {
	return _moveable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setEnabled(bool enable) {
	_enabled = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isEnabled() const {
	return _enabled && (_parent?_parent->isEnabled():true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setMoveCopy(bool enable) {
	_moveCopy = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isMoveCopyEnabled() const {
	return _moveCopy;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setData(const QVariant& data) {
	_data = data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant RecordMarker::data() const {
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setId(int id) {
	_id = id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordMarker::id() const {
	return _id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isModified() const {
	return _time != _correctedTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::reset() {
	_correctedTime = _time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::apply() {
	_time = _correctedTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::update() {
	if ( _parent )
		_parent->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordMarker::copy() {
	return new RecordMarker(nullptr, *this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::drawBackground(QPainter &painter, RecordWidget *context,
                                  int x, int y1, int y2,
                                  QColor color, qreal lineWidth) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::draw(QPainter &painter, RecordWidget *,
                        int x, int y1, int y2,
                        QColor color, qreal lineWidth) {
	painter.setPen(QPen(color, lineWidth));
	painter.drawLine(x, y1, x, y2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordMarker::toolTip() const {
	return QString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidgetDecorator::RecordWidgetDecorator(QObject *parent)
: QObject(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static int StreamCount = 0;
static int RecordWidgetCount = 0;

RecordWidget::Stream::Stream(bool owner) {
	records[0] = records[1] = nullptr;
	filter = nullptr;

	pen = QPen(SCScheme.colors.records.foreground, SCScheme.records.lineWidth);
	antialiasing = SCScheme.records.antiAliasing;
	stepFunction = false;
	hasCustomBackgroundColor = false;
	scale = 1.0;
	axisSpacing[0] = axisSpacing[1] = 0;
	axisDirty = true;
	ownRawRecords = owner;
	ownFilteredRecords = true;
	visible = true;
	filtering = false;
	optimize = SCScheme.records.optimize;

	++StreamCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::Stream::~Stream() {
	free();
	--StreamCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::Stream::setDirty() {
	traces[Stream::Raw].reset();
	traces[Stream::Filtered].reset();

	traces[Stream::Raw].dirty = true;
	traces[Stream::Filtered].dirty = true;

	axisDirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::Stream::free() {
	if ( records[0] && ownRawRecords ) {
		delete records[0];
	}
	if ( records[1] && ownFilteredRecords ) {
		delete records[1];
	}
	if ( filter ) {
		delete filter;
	}

	filter = nullptr;

	for ( int i = 0; i < 2; ++i ) {
		records[i] = nullptr;
		traces[i].poly = nullptr;
		traces[i].dirty = true;
		traces[i].dirtyData = true;
		traces[i].status = QString();
		traces[i].timingQuality = -1;
		traces[i].timingQualityCount = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::RecordWidget(QWidget *parent)
: QWidget(parent) {
	init();
	setScale(1, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::RecordWidget(const DataModel::WaveformStreamID& streamID, QWidget *parent)
 : QWidget(parent), _streamID(streamID) {
	init();
	setScale(1, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::init() {
	++RecordWidgetCount;

	_showEngineeringValues = SCScheme.records.showEngineeringValues;

	removeCustomBackgroundColor();
	setBackgroundRole(QPalette::Base);

	_valuePrecision = SCScheme.precision.traceValues;
	_decorator = nullptr;
	_shadowWidget = nullptr;
	_shadowWidgetFlags = Raw | Style;
	_markerSourceWidget = nullptr;
	_drawMode = Single;
	_recordBorderDrawMode = SCScheme.records.recordBorders.drawMode;
	_axisPosition = Left;
	_axisSpacing = 4;
	_rowSpacing = 0;
	_axisWidth = fontMetrics().ascent() + _axisSpacing + fontMetrics().boundingRect(QString::number(-1.234567e99)).width() + fontMetrics().height()/2;
	memset(_margins, 0, sizeof(_margins));

	_currentSlot = _requestedSlot = 0;
	_maxFilterSlot = -1;

	_amplScale = 0;
	_tmin = 0;
	_tmax = 0;
	_smin = _smax = 0;
	_gridHSpacing[0] = _gridHSpacing[1] = 0;
	_gridVSpacing[0] = _gridVSpacing[1] = 0;
	_gridHOffset = _gridVOffset = 0;
	_gridVRange[0] = _gridVRange[1] = 0;
	_gridVScale = 0;

	_tracePaintOffset = 0;
	_scrollBar = nullptr;

	// pick/arrival times
	_alignment = Core::Time(0.);

	_activeMarker = nullptr;
	_hoveredMarker = nullptr;

	_enabled = isEnabled();
	setDrawAxis(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::~RecordWidget() {
	if ( _scrollBar ) delete _scrollBar;

	// Delete all stream pointers
	clearRecords();

	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		if ( *it == nullptr ) continue;
		delete *it;
	}

	// Clear marker
	while ( !_marker.isEmpty() )
		delete _marker[0];

	--RecordWidgetCount;

	if ( RecordWidgetCount == 0 )
		SEISCOMP_DEBUG("All RecordWidgets deleted, remaining streams = %d", StreamCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::clearRecords() {
	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		if ( *it == nullptr ) continue;
		(*it)->free();
	}

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->clearRecords();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setEnabled(bool enabled) {
	QWidget::setEnabled(enabled);
	_enabled = enabled;
	/*
	update();
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::Stream *RecordWidget::getStream(int idx) {
	if ( idx < 0 ) return nullptr;
	if ( idx < _streams.size() ) return _streams[idx];

	setSlotCount(idx+1);

	if ( _drawMode == InRows ) emit layoutRequest();

	return _streams[idx];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RecordWidget::Stream *RecordWidget::getStream(int idx) const {
	if ( idx < 0 ) return nullptr;
	if ( idx < _streams.size() ) return _streams[idx];
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCustomBackgroundColor(QColor c) {
	_hasCustomBackground = true;
	_customBackgroundColor = c;

	for ( int i = 0; i < _streams.size(); ++i )
		setRecordBackgroundColor(i, c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::removeCustomBackgroundColor() {
	_hasCustomBackground = false;

	for ( int i = 0; i < _streams.size(); ++i )
		removeRecordBackgroundColor(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecords(int slot, RecordSequence *s, bool owner) {
	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->setRecords(slot, s, false);
	}

	Stream *stream = getStream(slot);
	if ( !stream ) {
		return false;
	}

	// If the same sequence is set again, make sure that it will
	// not be destroyed by free()
	if ( stream->records[Stream::Raw] == s ) {
		stream->ownRawRecords = false;
	}

	// Reset filter to forget all old buffered samples
	Filter *newFilter;
	if ( stream->filter && !(_shadowWidgetFlags & Filtered) ) {
		newFilter = stream->filter->clone();
	}
	else {
		newFilter = nullptr;
	}

	// Delete old record sequence
	stream->free();

	stream->records[Stream::Raw] = s;
	stream->traces[Stream::Raw].dirtyData = true;
	stream->ownRawRecords = owner;
	stream->filter = newFilter;

	if ( stream->records[Stream::Raw] ) {
		float quality = -1;
		int count = 0;
		bool success = s->timingQuality(count, quality);
		stream->traces[Stream::Raw].timingQualityCount = success ? count   :  0;
		stream->traces[Stream::Raw].timingQuality      = success ? quality : -1;

		if ( stream->filtering ) {
			createFilter(slot);
		}

		_drawRecords = true;
	}

	changedRecords(slot, s);

	stream->axisDirty = true;
	stream->traces[Stream::Raw].dirtyData = true;
	stream->traces[Stream::Filtered].dirtyData = true;

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setFilteredRecords(int slot, RecordSequence *s, bool owner) {
	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Filtered) ) {
		_shadowWidget->setFilteredRecords(slot, s, owner);
		_shadowWidget->setDirty();
	}

	Stream *stream = getStream(slot);
	if ( !stream ) {
		return false;
	}

	if ( stream->ownFilteredRecords && stream->records[Stream::Filtered] ) {
		delete stream->records[Stream::Filtered];
	}

	stream->records[Stream::Filtered] = s;
	stream->traces[Stream::Filtered].dirtyData = true;
	stream->ownFilteredRecords = owner;

	if ( s ) {
		_drawRecords = true;
	}

	changedRecords(slot, s);
	stream->setDirty();

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::changedRecords(int slot, RecordSequence*) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordFilter(int slot, const Filter *filter) {
	Stream *stream = getStream(slot);
	if ( !stream ) {
		return false;
	}

	if ( _shadowWidgetFlags & Filtered ) {
		return true;
	}

	if ( stream->filter ) {
		delete stream->filter;
	}

	if ( filter ) {
		stream->filter = filter->clone();
	}
	else {
		// Create a default filter
		stream->filter = new Math::Filtering::SelfFilter<double>();
		//SEISCOMP_DEBUG("Create default filter");
	}

	if ( stream->records[Stream::Filtered] && stream->ownFilteredRecords ) {
		delete stream->records[Stream::Filtered];
	}

	stream->records[Stream::Filtered] = nullptr;
	stream->traces[Stream::Filtered].dirtyData = true;
	stream->traces[Stream::Filtered].status = QString();

	if ( stream->records[Stream::Raw] && !stream->records[Stream::Raw]->empty() ) {
		const Record *rec = stream->records[Stream::Raw]->front().get();
		double fs = rec->samplingFrequency();
		try {
			stream->filter->setSamplingFrequency(fs);
			stream->filter->setStartTime(rec->startTime());
			stream->filter->setStreamID(rec->networkCode(), rec->stationCode(),
			                            rec->locationCode(), rec->channelCode());
			filterRecords(stream);
		}
		catch ( std::exception &e ) {
			stream->traces[Stream::Filtered].status = e.what();
			SEISCOMP_ERROR("%s: filter: %s", rec->streamID().c_str(), e.what());
		}
	}

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		if ( !(_shadowWidget->_shadowWidgetFlags & Filtered) ) {
			_shadowWidget->setRecordFilter(slot, filter);
		}
		else {
			_shadowWidget->setFilteredRecords(slot,
			                                  stream->records[Stream::Filtered], false);
		}
		_shadowWidget->setDirty();
	}

	setDirty();
	update();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordScale(int slot, double scale) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->scale = scale;

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->setRecordScale(slot, scale);
	}

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isRecordVisible(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	return stream->visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordVisible(int slot, bool visible) {
	Stream *stream = getStream(slot);
	if ( !stream ) {
		return false;
	}

	stream->visible = visible;

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->setRecordVisible(slot, visible);
	}

	if ( _drawMode != Single ) {
		setDirty();
	}

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordID(int slot, const QString &id) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->id = id;

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->setRecordID(slot, id);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordLabel(int slot, const QString &label) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->axisLabel = label;

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->setRecordLabel(slot, label);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordColor(int slot, QColor c) {
	return setRecordPen(slot, c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordPen(int slot, const QPen &pen) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->pen = pen;
	stream->setDirty();
	update();

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->setRecordPen(slot, pen);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordAntialiasing(int slot, bool antialiasing) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->antialiasing = antialiasing;
	update();

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->setRecordAntialiasing(slot, antialiasing);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordOptimization(int slot, bool enable) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	if ( stream->optimize == enable ) return true;

	stream->optimize = enable;
	stream->setDirty();
	update();

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->setRecordOptimization(slot, enable);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordStepFunction(int slot, bool enable) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	if ( stream->stepFunction == enable ) return true;

	stream->stepFunction = enable;
	stream->setDirty();
	update();

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->setRecordStepFunction(slot, enable);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordBackgroundColor(int slot, QColor c) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->customBackgroundColor = c;
	stream->hasCustomBackgroundColor = true;
	update();

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->setRecordBackgroundColor(slot, c);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::removeRecordBackgroundColor(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->hasCustomBackgroundColor = false;
	update();

	if ( _shadowWidget && ((_shadowWidget->_shadowWidgetFlags & (Raw | Style)) == (Raw | Style)) ) {
		_shadowWidget->removeRecordBackgroundColor(slot);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordUserData(int slot, QVariant data) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->userData = data;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordStatus(int slot, bool filtered, QString status) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	stream->traces[filtered ? 1 : 0].status = status;
	update();

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->setRecordStatus(slot, filtered, status);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordWidget::recordID(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return QString();
	return stream->id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordWidget::recordLabel(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return QString();
	return stream->axisLabel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::recordStepFunction(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;
	return stream->stepFunction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordWidget::recordColor(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return QColor();
	return stream->pen.color();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPen RecordWidget::recordPen(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return QPen();
	return stream->pen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::Trace *RecordWidget::traceInfo(int slot, bool filtered) {
	if ( (slot < 0) || (slot >= _streams.size()) ) return nullptr;
	if ( _streams[slot] == nullptr ) return nullptr;
	return &_streams[slot]->traces[filtered?Stream::Filtered:Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RecordWidget::Trace *RecordWidget::traceInfo(int slot, bool filtered) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return nullptr;
	return &stream->traces[filtered?Stream::Filtered:Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double *RecordWidget::recordScale(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == nullptr ) return nullptr;
	return &stream->scale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant RecordWidget::recordUserData(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return QVariant();
	return stream->userData;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::createRecords(int slot, bool owner) {
	if ( _streams.empty() ) return nullptr;

	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		Stream *s = *it;
		if ( s->records[Stream::Raw] != nullptr ) {
			Stream *ns = getStream(slot);
			if ( ns == nullptr ) return nullptr;

			if ( ns->records[Stream::Raw] ) return nullptr;

			ns->free();

			RecordSequence *seq = s->records[Stream::Raw]->clone();
			ns->records[Stream::Raw] = seq;
			ns->traces[Stream::Raw].dirtyData = true;
			ns->ownRawRecords = owner;

			setRecordFilter(slot, s->filter);

			if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
				_shadowWidget->setRecords(slot, seq, false);
			}

			if ( s->filtering )
				createFilter(slot);

			return seq;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RecordWidget::Filter *RecordWidget::recordFilter(int slot) const {
	if ( slot < 0 || slot >= _streams.size() ) return nullptr;
	return _streams[slot]->filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::setCurrentRecords(int slot) {
	if ( _currentSlot == slot ) return _currentSlot;

	_currentSlot = slot;

	update();

	if ( _shadowWidget && (_shadowWidget->_shadowWidgetFlags & Raw) ) {
		_shadowWidget->setCurrentRecords(slot);
	}

	if ( _drawMode == Single ) {
		emit traceUpdated(this);
	}

	return _currentSlot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::currentRecords() const {
	return _currentSlot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawMode(DrawMode mode) {
	if ( mode == _drawMode ) return;

	// Request a new layout when the mode changes in terms of needed
	// widget height
	if ( (_drawMode == InRows && mode != InRows) ||
	     (_drawMode != InRows && mode == InRows) ) {
		_drawMode = mode;
		emit layoutRequest();
	}
	else
		_drawMode = mode;

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::DrawMode RecordWidget::drawMode() const {
	return _drawMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawOffset(bool f) {
	_drawOffset = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawSPS(bool f) {
	_drawSPS = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawRecordID(bool f) {
	_drawRecordID = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawAxis(bool f) {
	_drawAxis = f;
	if ( !_drawAxis ) {
		_margins[0] = _margins[2] = 0;
	}
	else {
		switch ( _axisPosition ) {
			case Left:
				_margins[0] = _axisWidth;
				_margins[2] = 0;
				break;
			case Right:
				_margins[2] = _axisWidth;
				_margins[0] = 0;
				break;
			default:
				_margins[0] = 0;
				_margins[2] = 0;
				break;
		}
	}

	_canvasRect = QRect(_margins[0], _margins[1], width()-_margins[0]-_margins[2], height()-_margins[1]-_margins[3]);
	setDirty();
	update();

	emit axisSettingsChanged(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAxisWidth(int width) {
	bool needUpdate = false;

	width = qMax(width, 0);
	_axisWidth = width;

	int marginLeft, marginRight;
	switch ( _axisPosition ) {
		default:
		case Left:
			marginLeft = width;
			marginRight = 0;
			break;
		case Right:
			marginRight = width;
			marginLeft = 0;
			break;
	}

	if ( _margins[0] != marginLeft ) {
		_margins[0] = marginLeft;
		needUpdate = true;
	}

	if ( _margins[2] != marginRight ) {
		_margins[2] = marginRight;
		needUpdate = true;
	}

	if ( needUpdate ) {
		setDrawAxis(_drawAxis);
		emit axisSettingsChanged(this);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAxisPosition(AxisPosition position) {
	_axisPosition = position;
	setAxisWidth(_axisWidth);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setRowSpacing(int spacing) {
	if ( spacing < 0 ) spacing = 0;
	if ( _rowSpacing == spacing ) return;
	_rowSpacing = spacing;
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAxisSpacing(int spacing) {
	if ( spacing < 0 ) spacing = 0;
	if ( _axisSpacing == spacing ) return;
	_axisSpacing = spacing;
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setValuePrecision(int p) {
	_valuePrecision = p;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setClippingEnabled(bool f) {
	_clipRows = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isClippingEnabled() const {
	return _clipRows;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setShadowWidget(RecordWidget *shadow, bool copyMarker, int flags) {
	if ( _shadowWidget ) {
		_shadowWidget->_shadowWidgetFlags = 0;
	}

	_shadowWidget = shadow;

	if ( !_shadowWidget ) {
		return;
	}

	//_shadowWidget->clearRecords();
	if ( copyMarker ) {
		_shadowWidget->clearMarker();
	}

	_shadowWidget->_shadowWidgetFlags = flags;

	if ( flags & Raw ) {
		_shadowWidget->setSlotCount(slotCount());

		for ( int i = 0; i < slotCount(); ++i ) {
			_shadowWidget->setRecords(i, _streams[i]->records[Stream::Raw], false);
			if ( flags & Filtered ) {
				_shadowWidget->setFilteredRecords(i, _streams[i]->records[Stream::Filtered], false);
			}
		}

		for ( int i = 0; i < slotCount(); ++i ) {
			_shadowWidget->setRecordScale(i, _streams[i]->scale);
			_shadowWidget->setRecordID(i, _streams[i]->id);
			_shadowWidget->setRecordLabel(i, _streams[i]->axisLabel);
			_shadowWidget->setRecordStatus(i, false, _streams[i]->traces[0].status);
			_shadowWidget->setRecordStatus(i, true, _streams[i]->traces[1].status);

			if ( flags & Style ) {
				_shadowWidget->setRecordVisible(i, _streams[i]->visible);
				_shadowWidget->setRecordPen(i, _streams[i]->pen);
				_shadowWidget->setRecordAntialiasing(i, _streams[i]->antialiasing);
				if ( _streams[i]->hasCustomBackgroundColor ) {
					_shadowWidget->setRecordBackgroundColor(i, _streams[i]->customBackgroundColor);
				}
				else {
					_shadowWidget->removeRecordBackgroundColor(i);
				}
			}
		}
	}

	if ( copyMarker ) {
		for ( auto *m : qAsConst(_marker) ) {
			_shadowWidget->addMarker(m->copy());
		}
	}

	_shadowWidget->_hoveredMarker = nullptr;
	_shadowWidget->setCurrentRecords(_currentSlot);
	_shadowWidget->_streamID = _streamID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setMarkerSourceWidget(RecordWidget *source) {
	_markerSourceWidget = source;

	_hoveredMarker = markerAt(mapFromGlobal(QCursor::pos()),false,4);
	if ( _hoveredMarker ) {
		setToolTip(_hoveredMarker?_hoveredMarker->toolTip():QString());
		QToolTip::showText(QCursor::pos(), toolTip());
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDecorator(RecordWidgetDecorator *decorator) {
	_decorator = decorator;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::records() const {
	return _streams.empty()?nullptr:_streams[0]->records[Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::records(int slot) const {
	if ( slot < 0 || slot >= _streams.size() ) return nullptr;
	return _streams[slot]->records[Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::filteredRecords(int slot) const {
	if ( slot < 0 || slot >= _streams.size() ) return nullptr;
	return _streams[slot]->records[Stream::Filtered];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence* RecordWidget::takeRecords(int slot) {
	if ( slot < 0 || slot >= _streams.size() ) {
		return nullptr;
	}

	RecordSequence *seq = _streams[slot]->records[Stream::Raw];
	_streams[slot]->records[Stream::Raw] = nullptr;
	_streams[slot]->traces[Stream::Raw].dirtyData = true;
	delete _streams[slot];
	_streams.remove(slot);

	if ( _drawMode == InRows ) {
		emit layoutRequest();
	}

	if ( _shadowWidget ) {
		_shadowWidget->takeRecords(slot);
	}

	return seq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setTimeScale (double t) {
	_pixelPerSecond = t;
	_tmax = _tmin + (_pixelPerSecond > 0 && canvasWidth()?canvasWidth()/_pixelPerSecond:0);

	CHCK_RANGE
	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else
		setDirty();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAmplScale(double a) {
	_amplScale = a;

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::scroll(int v) {
	_tracePaintOffset = -v;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setScale(double t, double a) {
	_pixelPerSecond = t;
	_tmax = _tmin + (_pixelPerSecond > 0 && canvasWidth()?canvasWidth()/_pixelPerSecond:0);

	CHCK_RANGE
	setAmplScale(a);

	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else
		setDirty();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setTimeRange (double t1, double t2) {
	_tmin = t1;
	_tmax = _tmin + (_pixelPerSecond > 0 && canvasWidth()?canvasWidth()/_pixelPerSecond:0);

	CHCK_RANGE
	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAmplRange (double a1, double a2) {
	_useFixedAmplitudeRange = true;
	_useMinAmplitudeRange = false;
	_amplitudeRange[0] = float(a1);
	_amplitudeRange[1] = float(a2);
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setMinimumAmplRange( double a1, double a2) {
	_useFixedAmplitudeRange = false;
	_useMinAmplitudeRange = true;
	_amplitudeRange[0] = float(a1);
	_amplitudeRange[1] = float(a2);
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAmplAutoScaleEnabled(bool enabled) {
	if ( _useFixedAmplitudeRange == !enabled ) return;
	_useFixedAmplitudeRange = !enabled;
	setDirty();
	update();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showTimeRange(double t1, double t2) {
	if ( t1 >= t2 ) t2 = t1 + 1;
	setTimeRange(t1, t2);
	setScale(canvasWidth()/(t2-t1));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::translate(double dt) {
	setTimeRange(_tmin + dt, _tmax + dt);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordWidget::leftTime() const {
	return _alignment + Core::TimeSpan(_tmin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordWidget::rightTime() const {
	return _alignment + Core::TimeSpan(_tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::TimeWindow RecordWidget::visibleTimeWindow() const {
	return Core::TimeWindow(leftTime(), rightTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::TimeWindow RecordWidget::selectedTimeWindow() const {
	return Core::TimeWindow(_alignment + Core::TimeSpan(_smin),
	                        _alignment + Core::TimeSpan(_smax));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::TimeWindow & RecordWidget::normalizationWindow() const {
	return _normalizationWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::prepareRecords(Stream *s) {
	for ( int i = 0; i < 2; ++i ) {
		auto &trace = s->traces[i];
		if ( s->records[i] && ((s->filtering == static_cast<bool>(i)) || _showAllRecords) ) {
			if ( (!_useGlobalOffset && _normalizationWindow && s->traces[i].dirty)
			  || s->traces[i].dirtyData ) {
				trace.visible = minmax(s->records[i], _normalizationWindow,
				                       trace.dOffset, trace.dyMin, trace.dyMax,
				                       _useGlobalOffset);
				trace.absMax = std::max(std::abs(trace.dOffset - trace.dyMin),
				                        std::abs(trace.dOffset - trace.dyMax));
			}
			else {
				trace.visible = true;
			}
			trace.dirtyData = false;

			// Copy data range initially
			trace.yOffset = trace.dOffset;
			trace.yMin = trace.dyMin - trace.dOffset;
			trace.yMax = trace.dyMax - trace.dOffset;
		}
		else {
			trace.visible = false;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::alignTrace(Trace &trace) {
	if ( _useFixedAmplitudeRange ) {
		trace.yOffset = (_amplitudeRange[0] + _amplitudeRange[1]) * 0.5;
		trace.yMin = _amplitudeRange[0] - trace.yOffset;
		trace.yMax = _amplitudeRange[1] - trace.yOffset;
	}
	else if ( _useMinAmplitudeRange ) {
		trace.yMin = std::min(trace.yMin, _amplitudeRange[0]) - trace.yOffset;
		trace.yMax = std::max(trace.yMax, _amplitudeRange[1]) - trace.yOffset;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Draw the seismogram(s) internally into a RecordPolyline.
// to be called either by resize events or if new records are assigned
void RecordWidget::render(Stream *s) {
	//Core::TimeWindow tw(leftTime(), rightTime());
	double magnify = 1; // or 0.2 etc.

	if ( _amplScale > 0 ) {
		magnify = 1.0 / _amplScale;
	}

	int hMargin = s->pen.width() - 1;

	if ( hMargin < 0 ) {
		hMargin = 0;
	}

	Trace *trace = &s->traces[Stream::Raw];

	if ( s->records[Stream::Raw] && (!s->filtering || _showAllRecords) ) {
		alignTrace(*trace);

		trace->fyMin = magnify * trace->yMin;
		trace->fyMax = magnify * trace->yMax;

		createPolyline(s, trace->poly, s->records[Stream::Raw], _pixelPerSecond,
		               trace->fyMin, trace->fyMax, trace->yOffset, s->height - hMargin);

		trace->fyMin += trace->yOffset;
		trace->fyMax += trace->yOffset;

		trace->yMin += trace->yOffset;
		trace->yMax += trace->yOffset;

		trace->pyMin = int(trace->poly->baseline() * (1 - _amplScale));
		trace->pyMax = trace->pyMin + int(s->height * _amplScale);

		trace->dirty = false;
	}
	else {
		trace->fyMin = -1;
		trace->fyMax = 1;
		trace->poly = nullptr;
	}

	trace = &s->traces[Stream::Filtered];
	if ( s->records[Stream::Filtered] && (s->filtering || _showAllRecords) ) {
		alignTrace(*trace);

		trace->fyMin = magnify * trace->yMin;
		trace->fyMax = magnify * trace->yMax;

		createPolyline(s, trace->poly, s->records[Stream::Filtered], _pixelPerSecond,
		               trace->fyMin, trace->fyMax, trace->yOffset, s->height - hMargin);

		trace->fyMin += trace->yOffset;
		trace->fyMax += trace->yOffset;

		trace->yMin += trace->yOffset;
		trace->yMax += trace->yOffset;

		trace->pyMin = int(trace->poly->baseline() * (1 - _amplScale));
		trace->pyMax = trace->pyMin + int(s->height * _amplScale);

		trace->dirty = false;
	}
	else {
		trace->fyMin = -1;
		trace->fyMax = 1;
		trace->poly = nullptr;
	}

	if ( _amplScale > 1 ) {
		if ( !_scrollBar ) {
			_scrollBar = new QScrollBar(Qt::Vertical, this);
			connect(_scrollBar, SIGNAL(valueChanged(int)),
			        this, SLOT(scroll(int)));
			_scrollBar->setCursor(Qt::ArrowCursor);
		}
		_scrollBar->setGeometry(
			QRect(_canvasRect.right()-_scrollBar->sizeHint().width(), _canvasRect.top(),
			      _scrollBar->sizeHint().width(), _canvasRect.height())
		);
		_scrollBar->show();
	}
	else if ( _scrollBar ) {
		_scrollBar->hide();
		_tracePaintOffset = 0;
	}

	if ( _scrollBar && _scrollBar->isVisible() ) {
		int frontIndex = s->filtering?Stream::Filtered:Stream::Raw;

		_scrollBar->setRange(s->traces[frontIndex].pyMin, s->traces[frontIndex].pyMax-s->height);
		_scrollBar->setSingleStep(1);
		_scrollBar->setPageStep(s->height);
		_scrollBar->setValue(0);

		_tracePaintOffset = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::createPolyline(Stream *s, AbstractRecordPolylinePtr &polyline,
                                  RecordSequence const *seq, double pixelPerSecond,
                                  double amplMin, double amplMax, double amplOffset,
                                  int height) {
	if ( s->stepFunction ) {
		RecordPolylinePtr pl = new RecordPolyline;
		pl->createSteps(seq, leftTime(), rightTime(), pixelPerSecond,
		                amplMin, amplMax, amplOffset, height);

		if ( _showScaledValues && (s->scale < 0) ) {
			flip(*pl, height);
		}

		polyline = pl;
	}
	else {
		if ( s->antialiasing ) {
			RecordPolylineFPtr pl = new RecordPolylineF;
			pl->create(seq, leftTime(), rightTime(), pixelPerSecond,
			           amplMin, amplMax, amplOffset,
			           height, nullptr, nullptr, s->optimize);

			if ( _showScaledValues && (s->scale < 0) ) {
				flip(*pl, height);
			}

			polyline = pl;
		}
		else {
			RecordPolylinePtr pl = new RecordPolyline;
			pl->create(seq, leftTime(), rightTime(), pixelPerSecond,
			           amplMin, amplMax, amplOffset,
			           height, nullptr, nullptr, s->optimize);

			if ( _showScaledValues && (s->scale < 0) ) {
				flip(*pl, height);
			}

			polyline = pl;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double *RecordWidget::value(int slot, const Seiscomp::Core::Time& t) const {
	static double value;

	if ( slot < 0 || slot >= _streams.size() ) return nullptr;

	Stream *s = _streams[slot];

	RecordSequence *rs = s->filtering?s->records[Stream::Filtered]:s->records[Stream::Raw];
	if ( !rs )
		return nullptr;

	for ( RecordSequence::const_iterator it = rs->begin();
	      it != rs->end(); ++it ) {
		const Seiscomp::Record *rec = (*it).get();
		if ( t >= rec->startTime() && t <= rec->endTime() ) {
			if ( !rec->data() ) {
				return nullptr;
			}

			int pos = int(double(t - rec->startTime()) * rec->samplingFrequency());
			FloatArrayPtr tmp;
			const FloatArray *ar = FloatArray::ConstCast(rec->data());
			if ( !ar ) {
				tmp = (FloatArray*)rec->data()->copy(Array::FLOAT);
				ar = tmp.get();
			}

			if ( !ar ) return nullptr;

			if ( ar->size() <= (int)pos )
				return nullptr;

			value = (*ar)[pos];
			if ( _showScaledValues )
				value *= s->scale < 0 ? -s->scale : s->scale;

			return &value;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawAxis(QPainter &painter, const QPen &fg) {
	int fontHeight = fontMetrics().height();
	int tickLength = fontHeight/2+1;

	QRect rect;

	switch ( _drawMode ) {
		case Single:
		case SameOffset:
		case Stacked:
		{
			Stream *stream = nullptr;
			double axisLower{-1}, axisUpper{1};

			if ( _drawMode == Single ) {
				if ( _currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible ) {
					stream = _streams[_currentSlot];
				}
			}
			else {
				for ( auto *s : qAsConst(_streams) ) {
					int frontIndex = s->filtering ? Stream::Filtered : Stream::Raw;
					if ( s->visible && s->traces[frontIndex].visible ) {
						stream = s;
						break;
					}
				}
			}

			if ( stream ) {
				int frontIndex = stream->filtering ? Stream::Filtered : Stream::Raw;
				axisLower = stream->traces[frontIndex].fyMin;
				axisUpper = stream->traces[frontIndex].fyMax;

				if ( _drawMode == SameOffset ) {
					axisLower -= stream->traces[frontIndex].yOffset;
					axisUpper -= stream->traces[frontIndex].yOffset;
				}

				if ( _tracePaintOffset ) {
					if ( _tracePaintOffset > 0 ) {
						double pos = double(-_tracePaintOffset) / stream->traces[frontIndex].pyMin;
						double diff = stream->traces[frontIndex].yMax - stream->traces[frontIndex].fyMax;
						axisLower += diff*pos;
						axisUpper += diff*pos;
					}
					else {
						double pos = double(-_tracePaintOffset) / (stream->traces[frontIndex].pyMax - stream->height);
						double diff = stream->traces[frontIndex].yMin - stream->traces[frontIndex].fyMin;
						axisLower += diff*pos;
						axisUpper += diff*pos;
					}
				}

				if ( _showScaledValues ) {
					axisLower *= stream->scale;
					axisUpper *= stream->scale;

					if ( axisLower > axisUpper ) {
						std::swap(axisLower, axisUpper);
					}
				}

				if ( _axisPosition == Right ) {
					rect = QRect(width()-_margins[2]+_axisSpacing, stream->posY, _margins[2]-_axisSpacing, stream->height);
				}
				else {
					rect = QRect(0, stream->posY, _margins[0]-_axisSpacing, stream->height);
				}

				// double axisRange = axisUpper - axisLower;
				if ( stream->height > 1 ) {
					if ( stream->axisDirty ) {
						updateVerticalAxis(stream->axisSpacing, axisLower, axisUpper,
						                   stream->height - 1, fontHeight * 2);
						stream->axisDirty = false;
					}

					if ( _axisPosition == Right ) {
						rect = QRect(width()-_margins[2]+_axisSpacing, stream->posY, _margins[2]-_axisSpacing, stream->height);
					}
					else {
						rect = QRect(0, stream->posY, _margins[0]-_axisSpacing, stream->height);
					}

					drawVerticalAxis(painter, axisLower, axisUpper, stream->axisSpacing,
					                 rect, tickLength, stream->axisLabel,
					                 _axisPosition == Left, fg, SCScheme.colors.records.gridPen,
					                 _canvasRect.left(), _canvasRect.right(), _showEngineeringValues);
				}
			}

			break;
		}

		case InRows:
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = *it;
				if ( stream == nullptr ) continue;
				if ( stream->height == 0 ) continue;

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				double axisLower = stream->traces[frontIndex].fyMin,
				       axisUpper = stream->traces[frontIndex].fyMax;

				if ( _tracePaintOffset ) {
					if ( _tracePaintOffset > 0 ) {
						double pos = double(-_tracePaintOffset) / stream->traces[frontIndex].pyMin;
						double diff = stream->traces[frontIndex].yMax - stream->traces[frontIndex].fyMax;
						axisLower += diff*pos;
						axisUpper += diff*pos;
					}
					else {
						double pos = double(-_tracePaintOffset) / (stream->traces[frontIndex].pyMax - stream->height);
						double diff = stream->traces[frontIndex].yMin - stream->traces[frontIndex].fyMin;
						axisLower += diff*pos;
						axisUpper += diff*pos;
					}
				}

				if ( _showScaledValues ) {
					axisLower *= stream->scale;
					axisUpper *= stream->scale;

					if ( axisLower > axisUpper ) {
						std::swap(axisLower, axisUpper);
					}
				}

				double axisRange = axisUpper - axisLower;
				if ( stream->height > 1 && axisRange > 0 ) {
					if ( stream->axisDirty ) {
						updateVerticalAxis(stream->axisSpacing, axisLower, axisUpper,
						                   stream->height-1, fontMetrics().height()*2);
						stream->axisDirty = false;
					}

					if ( _axisPosition == Right )
						rect = QRect(width()-_margins[2]+_axisSpacing, stream->posY, _margins[2]-_axisSpacing, stream->height);
					else
						rect = QRect(0, stream->posY, _margins[0]-_axisSpacing, stream->height);

					drawVerticalAxis(painter, axisLower, axisUpper, stream->axisSpacing,
					                 rect, tickLength, stream->axisLabel,
					                 _axisPosition == Left, fg, SCScheme.colors.records.gridPen,
					                 _canvasRect.left(), _canvasRect.right(), _showEngineeringValues);
				}
			}
			break;

		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showAllRecords(bool enable) {
	if ( _showAllRecords == enable ) return;
	_showAllRecords = enable;

	if ( _showAllRecords )
		createFilter();

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showScaledValues(bool enable) {
	if ( _showScaledValues == enable ) {
		return;
	}

	_showScaledValues = enable;

	for ( auto *s : _streams ) {
		if ( !s ) {
			continue;
		}

		if ( s->scale < 0 ) {
			s->setDirty();
		}
		else {
			s->axisDirty = true;
		}
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showEngineeringValues(bool enable) {
	if ( _showEngineeringValues == enable ) {
		return;
	}

	_showEngineeringValues = enable;

	/*
	for ( auto *s : _streams ) {
		if ( !s ) {
			continue;
		}

		s->axisDirty = true;
	}
	*/

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDirty() {
	_drawRecords = true;
	for ( Stream *s : qAsConst(_streams) ) {
		if ( s ) {
			s->setDirty();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isActive() const {
	return _active;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isFilteringEnabled() const {
	return _filtering;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isGlobalOffsetEnabled() const {
	return _useGlobalOffset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::event(QEvent *event) {
	/*
	if ( event->type() == QEvent::ToolTip ) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);

		RecordMarker *activeMarker = _hoveredMarker;

		if ( activeMarker )
			QToolTip::showText(helpEvent->globalPos(), activeMarker->toolTip());
		else
			QToolTip::showText(helpEvent->globalPos(), QString());
	}
	*/

	return QWidget::event(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::paintEvent(QPaintEvent *event) {
	QPainter painter(this);

	if ( _pixelPerSecond <= 0 ) return;
	//bool emptyTrace = _poly[frontIndex].isEmpty();

	int w = width(), h = height();

	if ( _scrollBar && _scrollBar->isVisible() )
		w -= _scrollBar->width();

	if ( h == 0 || w == 0 )
		return; // actually this must never happen

	QRect rect = event->rect();
	QColor fg;
	QColor bg = palette().color(QPalette::Base);
	QColor alignColor;
	int    x;

	/*
	if ( emptyTrace )
		bg = blend(bg, Qt::red, 90);
	*/

	if ( !_enabled ) {
		fg = QColor(160,160,160);
		alignColor = fg;
	}
	else {
		fg = palette().color(foregroundRole());
		alignColor = SCScheme.colors.records.alignment;
	}

	painter.setClipRect(rect);
	painter.translate(_canvasRect.left(), _canvasRect.top());

	int sel_xmin = int((_smin-_tmin)*_pixelPerSecond),
	    sel_xmax = int((_smax-_tmin)*_pixelPerSecond);

	if ( sel_xmin < 0 ) sel_xmin = 0;
	if ( sel_xmin > _canvasRect.width() ) sel_xmin = _canvasRect.width();
	if ( sel_xmax < 0 ) sel_xmax = 0;
	if ( sel_xmax > _canvasRect.width() ) sel_xmax = _canvasRect.width();

	int sel_w = sel_xmax - sel_xmin;

	bool emitUpdated = false;

	int slot;

	switch ( _drawMode ) {
		default:
		case Single:
		{
			Stream *stream = nullptr;

			if ( (_currentSlot >= _streams.size() || _currentSlot < 0) || !_streams[_currentSlot]->visible ) {
				stream = nullptr;
			}
			else {
				stream = _streams[_currentSlot];
			}

			for ( int i = 0; i < _streams.size(); ++i ) {
				Stream *str = _streams[i];
				if ( !str ) {
					continue;
				}

				if ( str == stream ) {
					str->posY = 0;
					str->height = h;
				}
				else {
					str->posY = 0;
					str->height = 0;
				}
			}

			if ( stream ) {
				if ( stream->hasCustomBackgroundColor ) {
					painter.fillRect(0, stream->posY, _canvasRect.width(), stream->height, blend(bg, stream->customBackgroundColor));
				}

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
				     (stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					prepareRecords(stream);
					render(stream);
					emitUpdated = true;
				}
			}

			break;
		}

		case InRows:
		{
			int visibleSlots = 0;
			int streamHeight = h;
			int streamYOffset = 0;

			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				if ( (*it)->visible ) {
					++visibleSlots;
				}
			}

			if ( visibleSlots > 0 ) {
				streamHeight = (streamHeight - (visibleSlots - 1) * _rowSpacing) / visibleSlots;
			}

			slot = 0;
			for ( auto it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
				Stream *stream = *it;
				if ( !stream->visible ) {
					stream->posY = 0;
					stream->height = 0;
					continue;
				}

				stream->posY = streamYOffset;
				stream->height = streamHeight;

				if ( stream->hasCustomBackgroundColor )
					painter.fillRect(0, stream->posY, _canvasRect.width(), stream->height, blend(bg, stream->customBackgroundColor));

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					(stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					prepareRecords(stream);
					render(stream);
					emitUpdated = true;
				}

				streamYOffset += streamHeight + _rowSpacing;
			}

			break;
		}

		case Stacked:
		{
			bool isDirty = false;
			bool isFirst[2] = {true, true};
			double minAmpl[2] = {0, 0}, maxAmpl[2] = {0, 0};
			QColor customBackgroundColor;

			// Two passes: First pass fetches the amplitude range and so on and scales all records appropriate
			for ( auto *stream : qAsConst(_streams) ) {
				if ( !stream->visible ) {
					stream->posY = 0;
					stream->height = 0;
					continue;
				}

				stream->posY = 0;
				stream->height = h;

				if ( stream->hasCustomBackgroundColor && !customBackgroundColor.isValid() ) {
					customBackgroundColor = stream->customBackgroundColor;
				}

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					 (stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					isDirty = true;
					prepareRecords(stream);
				}

				for ( int i = 0; i < 2; ++i ) {
					if ( stream->traces[i].visible ) {
						if ( isFirst[i] ) {
							minAmpl[i] = stream->traces[i].dyMin;
							maxAmpl[i] = stream->traces[i].dyMax;
							isFirst[i] = false;
						}
						else {
							minAmpl[i] = std::min(minAmpl[i], stream->traces[i].dyMin);
							maxAmpl[i] = std::max(maxAmpl[i], stream->traces[i].dyMax);
						}
					}
				}
			}

			if ( customBackgroundColor.isValid() ) {
				painter.fillRect(_canvasRect.translated(-_canvasRect.topLeft()), blend(bg, customBackgroundColor));
			}

			if ( !isDirty ) {
				break;
			}

			// Second pass draws all records
			slot = 0;
			for ( auto *stream : qAsConst(_streams) ) {
				if ( !stream->visible ) {
					continue;
				}

				stream->traces[0].yMin = minAmpl[0] - stream->traces[0].yOffset;
				stream->traces[0].yMax = maxAmpl[0] - stream->traces[0].yOffset;

				stream->traces[1].yMin = minAmpl[1] - stream->traces[1].yOffset;
				stream->traces[1].yMax = maxAmpl[1] - stream->traces[1].yOffset;

				render(stream);

				emitUpdated = true;
				++slot;
			}
			break;
		}

		case SameOffset:
		{
			bool isDirty = false;
			bool isFirst[2] = { true, true };
			double minAmpl[2] = { 0, 0 }, maxAmpl[2] = { 0, 0 };
			QColor customBackgroundColor;

			// Two passes: First pass fetches the amplitude range and so on and scales all records appropriate
			for ( auto *stream : qAsConst(_streams) ) {
				if ( !stream->visible ) {
					stream->posY = 0;
					stream->height = 0;
					continue;
				}

				stream->posY = 0;
				stream->height = h;

				if ( stream->hasCustomBackgroundColor &&
					 !customBackgroundColor.isValid() )
					customBackgroundColor = stream->customBackgroundColor;

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					(stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					isDirty = true;
					prepareRecords(stream);
				}

				// i == 0: foreground
				// i == 1: background
				for ( int i = 0; i < 2; ++i ) {
					int j = i ^ (stream->filtering ? 1 : 0);
					if ( !stream->traces[j].visible ) {
						continue;
					}

					if ( isFirst[i] ) {
						minAmpl[i] = stream->traces[j].yMin;
						maxAmpl[i] = stream->traces[j].yMax;
						isFirst[i] = false;
					}
					else {
						minAmpl[i] = std::min(minAmpl[i], stream->traces[j].yMin);
						maxAmpl[i] = std::max(maxAmpl[i], stream->traces[j].yMax);
					}
				}
			}

			if ( customBackgroundColor.isValid() ) {
				painter.fillRect(_canvasRect.translated(-_canvasRect.topLeft()), blend(bg, customBackgroundColor));
			}

			if ( !isDirty ) {
				break;
			}

			// Second pass draws all records
			slot = 0;
			for ( auto it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
				Stream *stream = *it;
				if ( !stream->visible ) {
					continue;
				}

				for ( int i = 0; i < 2; ++i ) {
					int j = i ^ (stream->filtering ? 1 : 0);
					if ( !stream->traces[i].visible ) {
						continue;
					}

					stream->traces[i].yMin = minAmpl[j];
					stream->traces[i].yMax = maxAmpl[j];
				}

				render(stream);
				emitUpdated = true;
			}
			break;
		}
	}

	_drawRecords = false;

	QColor sel = blend(bg, SCScheme.colors.recordView.selectedTraceZoom);
	painter.fillRect(sel_xmin, 0, sel_w, h, sel);

	drawCustomBackground(painter);

	// Draw gaps
	switch ( _drawMode ) {
		default:
		case Stacked:
		case Single:
		{
			Stream *stream = (_currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible) ?
			                 _streams[_currentSlot] : nullptr;

			if ( stream ) {
				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( stream->traces[frontIndex].validTrace() ) {
					double offset[2] = {0,0};
					int x_tmin[2];

					if ( stream->records[Stream::Raw] )
						offset[0] = -_tmin;
					if ( stream->records[Stream::Filtered] )
						offset[1] = -_tmin;

					x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
					x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

					painter.setPen(fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset));
					stream->traces[frontIndex].poly->drawGaps(painter, 0, stream->height,
					                                          SCScheme.colors.records.gaps,
					                                          SCScheme.colors.records.overlaps);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset));
				}
			}
			break;
		}

		case SameOffset:
			break;

		case InRows:
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = (*it)->visible?*it:nullptr;
				if ( stream == nullptr ) continue;

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( stream->traces[frontIndex].validTrace() ) {
					double offset[2] = {0,0};
					int x_tmin[2];

					if ( stream->records[Stream::Raw] )
						offset[0] = -_tmin;
					if ( stream->records[Stream::Filtered] )
						offset[1] = -_tmin;

					x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
					x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

					painter.setPen(fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset + stream->posY));
					stream->traces[frontIndex].poly->drawGaps(painter, 0, stream->height,
					                                          SCScheme.colors.records.gaps,
					                                          SCScheme.colors.records.overlaps);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset - stream->posY));
				}
			}
			break;
	}

	QPen gridPen[2] = {SCScheme.colors.records.gridPen, SCScheme.colors.records.subGridPen};

	if ( _gridHSpacing[0] > 0 && !(_pixelPerSecond <= 0) && !Math::isNaN(_pixelPerSecond) && !Math::isNaN(_tmin) ) {
		//for ( int k = 1; k >= 0; --k ) {
		for ( int k = 0; k < 2; ++k ) {
			if ( !gridPen[k].color().alpha() ) continue;
			painter.setPen(gridPen[k]);

			double left = _tmin + _gridHOffset;
			double correctedLeft = floor((left+_gridHSpacing[k]*1E-2) / _gridHSpacing[k])*_gridHSpacing[k];
			if ( fabs(correctedLeft) < _gridHSpacing[k]*1E-2 )
				correctedLeft = 0.0;

			double offset = correctedLeft;
			left -= offset;
			correctedLeft = 0;

			int x = (int)((correctedLeft-left)*_pixelPerSecond);
			int w = canvasWidth();
			while ( x < w ) {
				if ( x >= 0 ) painter.drawLine(x,0,x,h);
				correctedLeft += _gridHSpacing[k];
				x = (int)((correctedLeft-left)*_pixelPerSecond);
			}
		}
	}

	if ( (_gridVSpacing[0] > 0) && (h > 0) && (_gridVScale > 0) ) {
		//for ( int k = 1; k >= 0; --k ) {
		for ( int k = 0; k < 2; ++k ) {
			if ( !gridPen[k].color().alpha() ) continue;
			painter.setPen(gridPen[k]);

			double bottom = _gridVRange[0] + _gridVOffset;
			double correctedBottom = floor((bottom+_gridVSpacing[k]*1E-2) / _gridVSpacing[k])*_gridVSpacing[k];
			if ( fabs(correctedBottom) < _gridVSpacing[k]*1E-2 )
				correctedBottom = 0.0;

			double offset = correctedBottom;
			bottom -= offset;
			correctedBottom = 0;

			int y = h-1-(int)((correctedBottom-bottom)*_gridVScale);
			while ( y >= 0 ) {
				painter.drawLine(0,y,w,y);
				correctedBottom += _gridVSpacing[k];
				y = h-1-(int)((correctedBottom-bottom)*_gridVScale);
			}
		}
	}

	if ( _drawAxis ) {
		painter.translate(-_canvasRect.left(), -_canvasRect.top());
		drawAxis(painter, fg);
		painter.translate(_canvasRect.left(), _canvasRect.top());
	}

	painter.setClipRect(0, 0, _canvasRect.width(), _canvasRect.height());

	// Draw marker background
	int markerCanvasOffset = 0;
	int markerCanvasHeight = h;

	QVector<RecordMarker*> *markerList = &_marker;

	if ( _markerSourceWidget ) {
		markerList = &_markerSourceWidget->_marker;
	}

	QColor offsetColor = blend(bg, SCScheme.colors.records.offset.color(), 75);

	foreach ( RecordMarker* m, *markerList ) {
		if ( m->isHidden() ) continue;

		int startY = markerCanvasOffset, endY = startY + markerCanvasHeight;

		switch ( m->_alignment ) {
			case Qt::AlignTop:
				endY = markerCanvasOffset + markerCanvasHeight*2/4-1;
				break;
			case Qt::AlignBottom:
				startY = markerCanvasOffset + markerCanvasHeight*2/4+1;
				break;
		}

		bool enabled = _enabled && m->isEnabled();

		x = mapTime(m->correctedTime());
		x -= _canvasRect.left();

		//painter.drawRect(textRect.translated(x,textY-3));
		if ( m->isMovable() && m->isModified() ) {
			m->drawBackground(painter, this, x, markerCanvasOffset,
			                  markerCanvasOffset + markerCanvasHeight,
			                  enabled?m->modifiedColor():fg,
			                  SCScheme.marker.lineWidth);
		}
		else {
			m->drawBackground(painter, this, x, startY, endY,
			                  enabled?m->color():fg,
			                  SCScheme.marker.lineWidth);
		}
	}

	customPaintTracesBegin(painter);

	bool isAntialiasing = painter.renderHints() & QPainter::Antialiasing;

	// Draw traces
	switch ( _drawMode ) {
		default:
		case Single:
		{
			Stream *stream = (_currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible) ?
			                 _streams[_currentSlot] : nullptr;
			if ( stream ) {
				double offset[2] = {0,0};
				int x_tmin[2];

				if ( stream->records[Stream::Raw] )
					offset[0] = -_tmin;
				if ( stream->records[Stream::Filtered] )
					offset[1] = -_tmin;

				x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
				x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( stream->traces[1-frontIndex].validTrace() && _showAllRecords ) {
					painter.setPen(SCScheme.colors.records.offset);
					painter.translate(QPoint(x_tmin[1-frontIndex], _tracePaintOffset));
					//_trace[1-frontIndex].poly.translate(x_tmin[1-frontIndex], _tracePaintOffset);
					stream->traces[1-frontIndex].poly->draw(painter);
					painter.translate(QPoint(-x_tmin[1-frontIndex], -_tracePaintOffset));
				}

				auto &trace = stream->traces[frontIndex];
				if ( trace.validTrace() ) {
					if ( _drawOffset ) {
						if ( _drawAxis ) {
							QPen penOffset(SCScheme.colors.records.offset);
							penOffset.setColor(offsetColor);
							painter.setPen(penOffset);
						}
						else
							painter.setPen(offsetColor);

						painter.drawLine(
							0,
							_tracePaintOffset + trace.poly->baseline(),
							_canvasRect.width(),
							_tracePaintOffset + trace.poly->baseline()
						);
					}

					if ( stream->antialiasing != isAntialiasing )
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing = stream->antialiasing);

					int hMargin = stream->pen.width()-1;
					if ( hMargin < 0 ) hMargin = 0;

					drawTrace(painter, &trace,
					          stream->records[frontIndex],
					          QPen(_enabled ? stream->pen : fg),
					          QPoint(x_tmin[frontIndex], _tracePaintOffset + hMargin));
				}
			}
			break;
		}

		case InRows:
			for ( int i = 0; i < _streams.size(); ++i ) {
				Stream *stream = _streams[i]->visible?_streams[i]:nullptr;
				if ( stream == nullptr ) continue;

				double offset[2] = {0,0};
				int x_tmin[2];

				if ( stream->records[Stream::Raw] )
					offset[0] = -_tmin;
				if ( stream->records[Stream::Filtered] )
					offset[1] = -_tmin;

				x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
				x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

				if ( stream->height > 0 ) {
					// Enable clipping to rows if enabled
					if ( _clipRows )
						painter.setClipRect(QRect(0, stream->posY, w, stream->height));
					else
						painter.setClipRect(QRect(0, 0, w, h));

					if ( i == _currentSlot && _streams.size() > 1 ) {
						painter.setRenderHint(QPainter::Antialiasing, false);
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(palette().color(_enabled?QPalette::Active:QPalette::Disabled, QPalette::Text), 1, Qt::DashLine));
						painter.drawRect(QRect(0, stream->posY, _canvasRect.width()-1, stream->height-1));
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing);
					}
				}

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( stream->traces[1-frontIndex].validTrace() && _showAllRecords ) {
					painter.setPen(SCScheme.colors.records.offset);
					painter.translate(QPoint(x_tmin[1-frontIndex], _tracePaintOffset + stream->posY));
					//_trace[1-frontIndex].poly.translate(x_tmin[1-frontIndex], _tracePaintOffset);
					stream->traces[1-frontIndex].poly->draw(painter);
					painter.translate(QPoint(-x_tmin[1-frontIndex], -_tracePaintOffset - stream->posY));
				}

				auto &trace = stream->traces[frontIndex];
				if ( trace.validTrace() ) {
					if ( _drawOffset ) {
						if ( _drawAxis )
							painter.setPen(QPen(offsetColor, 1, Qt::DashLine));
						else
							painter.setPen(offsetColor);
						painter.drawLine(
							0, _tracePaintOffset + trace.poly->baseline() + stream->posY,
							_canvasRect.width(),
							_tracePaintOffset + trace.poly->baseline() + stream->posY
						);
					}

					if ( stream->antialiasing != isAntialiasing )
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing = stream->antialiasing);

					int hMargin = stream->pen.width()-1;
					if ( hMargin < 0 ) hMargin = 0;

					drawTrace(painter, &trace,
					          stream->records[frontIndex],
					          QPen(_enabled ? stream->pen : fg),
					          QPoint(x_tmin[frontIndex], _tracePaintOffset + stream->posY + hMargin));
				}
			}
			break;

		case Stacked:
		case SameOffset:
			// Draw offset
			if ( _drawOffset ) {
				for ( int i = 0; i < _streams.size(); ++i ) {
					Stream *stream = _streams[i]->visible?_streams[i] : nullptr;
					if ( !stream ) {
						continue;
					}

					int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
					if ( stream->traces[frontIndex].validTrace() ) {
						if ( _drawAxis ) {
							painter.setPen(QPen(offsetColor, 1, Qt::DashLine));
						}
						else {
							painter.setPen(offsetColor);
						}
						painter.drawLine(0,_tracePaintOffset+stream->traces[frontIndex].poly->baseline(), _canvasRect.width(),_tracePaintOffset+stream->traces[frontIndex].poly->baseline());
					}
				}
			}

			// Draw backtraces
			if ( _showAllRecords ) {
				for ( int i = 0; i < _streams.size(); ++i ) {
					Stream *stream = _streams[i]->visible?_streams[i] : nullptr;
					if ( !stream ) {
						continue;
					}

					double offset[2] = {0,0};
					int x_tmin[2];

					if ( stream->records[Stream::Raw] ) {
						offset[0] = -_tmin;
					}
					if ( stream->records[Stream::Filtered] ) {
						offset[1] = -_tmin;
					}

					x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
					x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

					int frontIndex = stream->filtering ? Stream::Filtered : Stream::Raw;
					if ( stream->traces[1-frontIndex].validTrace() ) {
						painter.setPen(SCScheme.colors.records.offset);
						painter.translate(QPoint(x_tmin[1-frontIndex], _tracePaintOffset));
						stream->traces[1-frontIndex].poly->draw(painter);
						painter.translate(QPoint(-x_tmin[1-frontIndex], -_tracePaintOffset));
					}
				}
			}

			// Draw records
			for ( int i = 0; i < _streams.size(); ++i ) {
				Stream *stream = _streams[i]->visible?_streams[i]:nullptr;
				if ( stream == nullptr ) continue;

				double offset[2] = {0,0};
				int x_tmin[2];

				if ( stream->records[Stream::Raw] ) {
					offset[0] = -_tmin;
				}
				if ( stream->records[Stream::Filtered] ) {
					offset[1] = -_tmin;
				}

				x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
				x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

				int frontIndex = stream->filtering ? Stream::Filtered : Stream::Raw;
				auto &trace = stream->traces[frontIndex];

				if ( trace.validTrace() ) {
					if ( stream->antialiasing != isAntialiasing ) {
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing = stream->antialiasing);
					}

					int hMargin = stream->pen.width() - 1;
					if ( hMargin < 0 ) {
						hMargin = 0;
					}

					drawTrace(painter, &trace,
					          stream->records[frontIndex],
					          QPen(_enabled ? stream->pen : fg),
					          QPoint(x_tmin[frontIndex], _tracePaintOffset + hMargin));
				}
			}
			break;
	}

	if ( isAntialiasing ) {
		painter.setRenderHint(QPainter::Antialiasing, false);
	}

	painter.setClipRect(0, 0, _canvasRect.width(), _canvasRect.height());

	customPaintTracesEnd(painter);

	painter.setPen(alignColor);
	x = (int)(-_tmin*_pixelPerSecond);
	painter.drawLine(x, 0, x, h);

	// make the font a bit smaller (for the phase annotations)
	QFont font;
	drawMarkers(painter, font, fg);

	// Draw labels
	switch ( _drawMode ) {
		default:
		case Single:
		{
			Stream *stream = (_currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible) ?
			                 _streams[_currentSlot] : nullptr;

			if ( stream ) {
				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				auto &trace = stream->traces[frontIndex];

				if ( trace.validTrace() ) {
					painter.setPen(fg);
					font.setBold(false);
					painter.setFont(font);

					if ( _drawOffset ) {
						QString str;
						if ( _showScaledValues )
							str = tr("amax: %1%2")
							      .arg(valueToString(_showEngineeringValues, trace.absMax * (stream->scale > 0 ? stream->scale : -stream->scale)),
							           stream->axisLabel);
						else
							str = tr("amax: %1%2")
							      .arg(valueToString(_showEngineeringValues, trace.absMax, _valuePrecision),
							           stream->axisLabel);

						int rh = 2 * painter.fontMetrics().ascent() + 4;
						int y = stream->height - rh;
						if  ( y < 0 ) y = 0;

						painter.drawText(4,y, w-4,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop, str);

						if ( stream->height >= y+rh ) {
							if ( _showScaledValues )
								str = tr("mean: %1%2")
								      .arg(valueToString(_showEngineeringValues, trace.dOffset * (stream->scale > 0 ? stream->scale : -stream->scale)),
								           stream->axisLabel);
							else
								str = tr("mean: %1%2")
								      .arg(valueToString(_showEngineeringValues, trace.dOffset, _valuePrecision),
								           stream->axisLabel);

							painter.drawText(4,y, w-4,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignBottom, str);
						}
					}

					if ( _drawSPS && stream->records[frontIndex] ) {
						painter.drawText(
							0, 0, _canvasRect.width() - 4, stream->height,
							Qt::TextSingleLine | Qt::AlignRight | Qt::AlignBottom,
							QString("%1 sps")
							.arg(stream->records[frontIndex]->front()->samplingFrequency())
						);
					}

					/*
					// Draw timing quality
					if ( stream->traces[Stream::Raw].timingQuality > 0 ) {
						str.setNum((int)stream->traces[Stream::Raw].timingQuality);
						painter.drawText(0,0, w,streamHeight, Qt::TextSingleLine | Qt::AlignRight | Qt::AlignTop, str);
					}
					*/
				}

				if ( !trace.status.isEmpty() ) {
					painter.setPen(fg);
					painter.drawText(4, stream->posY,
					                 _canvasRect.width() - 4,
					                 stream->height - 4,
					                 Qt::AlignLeft | Qt::AlignBottom,
					                 trace.status);
				}

				if ( !stream->id.isEmpty() && _drawRecordID ) {
					painter.setPen(fg);
					font.setBold(true);
					painter.setFont(font);
					QRect br = painter.fontMetrics().boundingRect(stream->id);
					br.adjust(0,0,4,4);
					//br.moveCenter(QPoint(br.center().x(), streamHeight/2+streamYOffset));
					br.moveTopLeft(QPoint(0,0));
					painter.fillRect(br, bg);
					painter.drawRect(br);
					painter.drawText(br, Qt::AlignCenter, stream->id);
				}
			}
			break;
		}

		case InRows:
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = (*it)->visible?*it:nullptr;
				if ( stream == nullptr ) continue;

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				auto &trace = stream->traces[frontIndex];

				if ( trace.validTrace() ) {
					painter.setPen(fg);
					font.setBold(false);
					painter.setFont(font);

					if ( _drawOffset ) {
						QString str;

						if ( _showScaledValues )
							str = tr("amax: %1%2")
							      .arg(valueToString(_showEngineeringValues, trace.absMax * (stream->scale > 0 ? stream->scale : -stream->scale)),
							           stream->axisLabel);
						else
							str = tr("amax: %1%2")
							      .arg(valueToString(_showEngineeringValues, trace.absMax, _valuePrecision),
							           stream->axisLabel);

						int rh = 2*painter.fontMetrics().ascent()+4;
						int y = stream->posY + stream->height - rh;
						if ( y < stream->posY ) y = stream->posY;

						painter.drawText(4,y, w-4,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop, str);

						if ( stream->posY+stream->height >= y+rh ) {
							if ( _showScaledValues )
								str = tr("mean: %1%2")
								      .arg(valueToString(_showEngineeringValues, trace.dOffset * (stream->scale > 0 ? stream->scale : -stream->scale)),
								           stream->axisLabel);
							else
								str = tr("mean: %1%2")
								      .arg(valueToString(_showEngineeringValues, trace.dOffset, _valuePrecision),
								           stream->axisLabel);
							painter.drawText(4,y, w-4,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignBottom, str);
						}
					}

					if ( _drawSPS && stream->records[frontIndex] ) {
						painter.drawText(
							0, stream->posY, _canvasRect.width()-4, stream->height,
							Qt::TextSingleLine | Qt::AlignRight | Qt::AlignBottom,
							QString("%1 sps")
							.arg(stream->records[frontIndex]->front()->samplingFrequency())
						);
					}
				}

				if ( !trace.status.isEmpty() ) {
					if ( !trace.validTrace() ) {
						painter.setPen(fg);
						font.setBold(false);
						painter.setFont(font);
					}

					painter.drawText(4, stream->posY,
					                 _canvasRect.width() - 4,
					                 stream->height - 4,
					                 Qt::AlignLeft | Qt::AlignBottom,
					                 trace.status);
				}

				if ( !stream->id.isEmpty() && _drawRecordID ) {
					painter.setPen(fg);
					font.setBold(true);
					painter.setFont(font);
					QRect br = painter.fontMetrics().boundingRect(stream->id);
					br.adjust(0,0,4,4);
					//br.moveCenter(QPoint(br.center().x(), streamHeight/2+streamYOffset));
					br.moveTopLeft(QPoint(0,stream->posY));
					painter.fillRect(br, bg);
					painter.drawRect(br);
					painter.drawText(br, Qt::AlignCenter, stream->id);
				}
			}
			break;

		case Stacked:
		case SameOffset:
			{
				double offset = 0;
				int cnt = 0;

				Stream *stream = nullptr;
				for ( auto it = _streams.begin(); it != _streams.end(); ++it ) {
					Stream *tmpStream = (*it)->visible ? *it : nullptr;
					if ( !tmpStream ) {
						continue;
					}

					int frontIndex = tmpStream->filtering ? Stream::Filtered : Stream::Raw;
					if ( !tmpStream->traces[frontIndex].validTrace() ) {
						continue;
					}
					++cnt;
					offset += tmpStream->traces[frontIndex].dOffset;
					stream = tmpStream;
				}

				if ( !cnt ) {
					break;
				}

				offset /= cnt;

				//Stream *stream = _streams[0];
				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				double absMax = std::max(std::abs(stream->traces[frontIndex].dOffset - stream->traces[frontIndex].dyMin),
				                         std::abs(stream->traces[frontIndex].dOffset - stream->traces[frontIndex].dyMax));

				painter.setPen(fg);
				font.setBold(false);
				painter.setFont(font);

				if ( _drawOffset ) {
					QString str;

					if ( _showScaledValues ) {
						str = tr("amax: %1%2")
						      .arg(valueToString(_showEngineeringValues, absMax * (stream->scale > 0 ? stream->scale : -stream->scale)),
						           stream->axisLabel);
					}
					else {
						str = tr("amax: %1%2")
						      .arg(valueToString(_showEngineeringValues, absMax, _valuePrecision),
						           stream->axisLabel);
					}

					int rh = 2*painter.fontMetrics().ascent()+4;
					int y = stream->height - rh;
					if ( y < 0 ) y = 0;

					painter.drawText(4,y, w-4,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop, str);

					if ( stream->height >= y+rh ) {
						if ( _showScaledValues )
							str = tr("mean: %1%2")
							      .arg(valueToString(_showEngineeringValues, offset * (stream->scale > 0 ? stream->scale : -stream->scale)),
							           stream->axisLabel);
						else
							str = tr("mean: %1%2")
							      .arg(valueToString(_showEngineeringValues, offset, _valuePrecision),
							           stream->axisLabel);
						painter.drawText(4,y, w-4,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignBottom, str);
					}
				}

				if ( _drawSPS && stream->records[frontIndex] ) {
					painter.drawText(
						0, 0, _canvasRect.width()-4, stream->height,
						Qt::TextSingleLine | Qt::AlignRight | Qt::AlignBottom,
						QString("%1 sps")
						.arg(stream->records[frontIndex]->front()->samplingFrequency())
					);
				}
			}
			break;
	}

	painter.translate(-_canvasRect.left(), -_canvasRect.top());
	painter.setClipping(false);

	if ( _decorator ) {
		if ( font.bold() ) {
			font.setBold(false);
			painter.setFont(font);
		}

		_decorator->drawDecoration(&painter, this);
	}

	if ( emitUpdated ) emit traceUpdated(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawActiveCursor(QPainter &painter, int x, int y) {
	int h = height();

	painter.setPen(palette().color(QPalette::Text));
	painter.drawLine(x, 0, x, h);
	//painter.drawLine(0, _currentPos.y(), w, _currentPos.y());
	painter.drawText(0,0,x-2,h, Qt::TextSingleLine | Qt::AlignRight | Qt::AlignTop, _cursorText);

	QFont f = painter.font();
	f.setBold(false);
	painter.setFont(f);

	QString time = timeToString(_cursorPos, "%T.%f000000").mid(0,11);

	painter.drawText(rect(), Qt::TextSingleLine | Qt::AlignRight | Qt::AlignTop, time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mousePressEvent(QMouseEvent *event) {
	if ( event->button() == Qt::MiddleButton ) {
		emit clickedOnTime(unmapTime(event->x()));
		return;
	}

	if ( !_enabled ) {
		event->ignore();
		return;
	}

	if ( event->button() == Qt::LeftButton /*|| event->button() == Qt::RightButton*/ )
		setCurrentMarker(markerAt(event->pos(),false,4));

	if ( !_active ) {
		event->ignore();
		return;
	}

	if ( event->button() == Qt::LeftButton ) {
		setCursorPos(event->pos());
		_startDragPos = _cursorPos;
		event->ignore();
		return;
	}
	else if ( event->button() == Qt::RightButton ) {
		RecordMarker* m = nearestMarker(unmapTime(event->x()));
		if ( m )
			setCursorPos(m->correctedTime());

		return;
	}

	event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mouseReleaseEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		if ( _startDragPos ) {
			if ( *_startDragPos < _cursorPos ) {
				emit selectedTimeRange(*_startDragPos, _cursorPos);
			}
			else {
				emit selectedTimeRange(_cursorPos, *_startDragPos);
			}
			_startDragPos = Core::None;
		}
	}

	event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mouseDoubleClickEvent(QMouseEvent *event) {
	if ( !_enabled || !_active ) {
		event->ignore();
		return;
	}

	if ( event->button() == Qt::LeftButton ) {
		emit selectedTime(unmapTime(event->x()));
		return;
	}
	else if ( event->button() == Qt::RightButton ) {
		RecordMarker* m = nearestMarker(unmapTime(event->x()));
		if ( m )
			emit selectedTime(m->correctedTime());
		return;
	}

	event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mouseMoveEvent(QMouseEvent *event) {
	if ( hasMouseTracking() ) {
		// Update marker hover state
		RecordMarker *hovered = markerAt(event->pos(),false,4);
		if ( _hoveredMarker != hovered ) {
			_hoveredMarker = hovered;
			setToolTip(_hoveredMarker?_hoveredMarker->toolTip():QString());
			QToolTip::showText(event->globalPos(), toolTip());
			update();
		}

		/*
		setCurrentMarker(markerAt(event->pos(),false,4));

		RecordMarker *activeMarker = _activeMarker;
		if ( _markerSourceWidget )
			activeMarker = _markerSourceWidget->_activeMarker;

		setToolTip(activeMarker?activeMarker->toolTip():QString());
		*/

		if ( _enabled && _active && event->buttons() == Qt::NoButton ) {
			_currentCursorYPos = event->pos().y();
			setCursorPos(event->pos());
		}
	}


	if ( event->buttons() == Qt::LeftButton ) {
		if ( _enabled && _active ) {
			_currentCursorYPos = event->pos().y();
			setCursorPos(event->pos());

			if ( _startDragPos ) {
				if ( *_startDragPos < _cursorPos ) {
					emit selectedTimeRangeChanged(*_startDragPos, _cursorPos);
				}
				else {
					emit selectedTimeRangeChanged(_cursorPos, *_startDragPos);
				}
			}
		}
		else {
			event->ignore();
		}
	}

	/*
	if ( _enabled ) {
		if ( _activeMarker ) {
			Core::Time t;
			t = _alignment +
				Core::TimeSpan((float)event->x()/_pixelPerSecond+_tmin);
			_activeMarker->setCorrectedTime(t);
			update();
		}
		else {
			RecordMarker* marker = markerAt(event->x(), event->y(), true);
			if ( marker )
				setCursor(Qt::SizeHorCursor);
			else
				setCursor(Qt::ArrowCursor);
		}

		if ( _active )
			update();
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enterEvent(QEvent *) {
	_hoveredMarker = nullptr;
	emit mouseOver(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::leaveEvent(QEvent *) {
	_hoveredMarker = nullptr;
	emit mouseOver(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawCustomBackground(QPainter &) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::customPaintTracesBegin(QPainter &painter) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::customPaintTracesEnd(QPainter &painter) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::resizeEvent (QResizeEvent *event) {
	QWidget::resizeEvent(event);

	_canvasRect = QRect(_margins[0], _margins[1], width()-_margins[0]-_margins[2], height()-_margins[1]-_margins[3]);
	if ( _pixelPerSecond == 0 )
		_pixelPerSecond = 1;

	if ( _scrollBar ) {
		_scrollBar->setGeometry(QRect(width()-_scrollBar->width(), 0, _scrollBar->width(), height()));
		//_scrollBar->resize(_scrollBar->width(), height());
		//_scrollBar->move(width()-_scrollBar->width(), 0);
	}

	_tmax = _tmin + (_pixelPerSecond > 0 && canvasWidth()?canvasWidth()/_pixelPerSecond:0);

	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else if( size() != event->oldSize() )
		// if the widget height has changed
		setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setSelected (double t1, double t2) {
	_smin = t1;
	_smax = t2;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setSelected(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2) {
	_smin = (double)(t1 - _alignment);
	_smax = (double)(t2 - _alignment);
	if ( _smin > _smax ) std::swap(_smin, _smax);

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAlignment(Core::Time t) {
	if ( _alignment == t ) return;

	_alignment = t;
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::alignOnMarker(const QString& text) {
	for ( int i = 0; i < _marker.count(); ++i ) {
		if ( _marker[i]->text() != text ) continue;
		setAlignment(_marker[i]->correctedTime());
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enableFiltering(bool enable) {
	if ( enable == _filtering ) return;

	_filtering = enable;

	if ( _filtering )
		createFilter();

	// Erase already prepared and unused data
	if ( !_showAllRecords ) {
		for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
			Stream* stream = *it;
			if ( stream->filtering != enable )
				stream->filtering = enable;
			stream->traces[_filtering?Stream::Raw:Stream::Filtered].poly = nullptr;
		}
	}

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enableGlobalOffset(bool enable) {
	_useGlobalOffset = enable;

	setDirty();
	update();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enableRecordFiltering(int slot, bool enable) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return;
	if ( stream->filtering == enable ) return;

	stream->filtering = enable;
	stream->traces[enable?Stream::Raw:Stream::Filtered].poly = nullptr;

	setDirty();
	update();

	if ( _shadowWidget ) _shadowWidget->enableRecordFiltering(slot, enable);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isRecordFilteringEnabled(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == nullptr ) return false;

	return stream->filtering;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setGridSpacing(double large, double small, double ofs) {
	_gridHSpacing[0] = large;
	_gridHSpacing[1] = small;
	_gridHOffset = ofs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setGridVSpacing(double large, double small, double ofs) {
	_gridVSpacing[0] = large;
	_gridVSpacing[1] = small;
	_gridVOffset = ofs;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setGridVRange(double min, double max) {
	_gridVRange[0] = min; _gridVRange[1] = max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setGridVScale(double scale) {
	_gridVScale = scale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setActive(bool a) {
	_active = a;

	/*
	RecordMarker* m = enabledMarker(_cursorText);
	if ( m )
		setCursorPos(m->correctedTime());
	else
		setCursorPos(alignment());
	*/

	setCursor(_active && !_cursorText.isEmpty()?Qt::CrossCursor:Qt::ArrowCursor);

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAutoMaxScale(bool e) {
	if ( _autoMaxScale == e ) return;

	_autoMaxScale = e;
	if ( _autoMaxScale ) {
		setNormalizationWindow(visibleTimeWindow());
	}
	else {
		for ( Stream *s : qAsConst(_streams) ) {
			if ( s ) {
				s->traces[Stream::Raw].dirtyData = true;
				s->traces[Stream::Filtered].dirtyData = true;
			}
		}
		setNormalizationWindow(Core::TimeWindow());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setNormalizationWindow(const Seiscomp::Core::TimeWindow &tw) {
	_normalizationWindow = tw;
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::createFilter() {
	if ( _shadowWidgetFlags & Filtered ) return true;

	int slot = 0;
	for ( StreamMap::iterator it = _streams.begin();
	      it != _streams.end(); ++it, ++slot )
		createFilter(slot);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::createFilter(int slot) {
	Stream* s = getStream(slot);

	if ( !s ) {
		return false;
	}

	if ( _shadowWidgetFlags & Filtered ) {
		return true;
	}

	if ( !s->filter ) {
		setRecordFilter(slot, nullptr);
		return true;
	}

	s->traces[Stream::Filtered].status = QString();

	if (s->records[Stream::Raw] && !s->records[Stream::Raw]->empty()) {
		const Record *rec = s->records[Stream::Raw]->front().get();
		double fs = rec->samplingFrequency();
		if ( s->records[Stream::Filtered] && !s->records[Stream::Filtered]->empty() ) {
			return false;
		}
		else {
			try {
				s->filter->setSamplingFrequency(fs);
				s->filter->setStartTime(rec->startTime());
				s->filter->setStreamID(rec->networkCode(), rec->stationCode(),
				                       rec->locationCode(), rec->channelCode());
				filterRecords(s);
			}
			catch ( std::exception &e ) {
				s->traces[Stream::Filtered].status = e.what();
				SEISCOMP_ERROR("%s: filter: %s", rec->streamID().c_str(), e.what());
			}
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setFilterSlotMax(int max) {
	_maxFilterSlot = max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setFilter(Filter *filter) {
	int slot = 0;
	for ( auto it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
		if ( slot < _maxFilterSlot || _maxFilterSlot < 0 )
			setRecordFilter(slot, filter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::filterRecords(Stream *s) {
	s->records[Stream::Filtered] = s->records[Stream::Raw]->clone();
	s->traces[Stream::Filtered].dirtyData = true;

	RecordPtr lastRec;
	for ( RecordSequence::const_iterator it = s->records[Stream::Raw]->begin();
	      it != s->records[Stream::Raw]->end(); ++it) {
		try {
			RecordPtr rec = filteredRecord(s->filter, (*it).get(), lastRec.get(),
			                               s->records[Stream::Filtered]->tolerance());
			if ( rec ) {
				s->records[Stream::Filtered]->feed(rec.get());
				s->traces[Stream::Filtered].dirty = true;
				lastRec = rec;
			}
		}
		catch ( std::exception &e ) {
			s->traces[Stream::Filtered].status = e.what();
			SEISCOMP_ERROR("%s: filter: %s", (*it).get()->streamID().c_str(), e.what());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *RecordWidget::filteredRecord(Filter *&filter,
                                     const Record* rec, const Record* lastRec,
                                     double tolerance) const {
	if ( rec->data() == nullptr ) return nullptr;

	DoubleArrayPtr arr = static_cast<DoubleArray*>(rec->data()->copy(Array::DOUBLE));

	GenericRecord* crec = new GenericRecord(*rec);

	if ( lastRec ) {
		tolerance = tolerance / rec->samplingFrequency();
		double diff;

		try {
			diff = abs(double(rec->startTime() - lastRec->endTime()));
		}
		catch ( ... ) {
			diff = tolerance * 2;
		}

		if ( diff > tolerance ) {
			Filter *tmp = filter;
			filter = filter->clone();
			delete tmp;

			try {
				filter->setSamplingFrequency(rec->samplingFrequency());
				filter->setStartTime(rec->startTime());
				filter->setStreamID(rec->networkCode(), rec->stationCode(),
				                    rec->locationCode(), rec->channelCode());
			}
			catch ( std::exception &e ) {
				delete crec;
				throw e;
			}
		}
	}

	filter->apply(arr->size(), arr->typedData());

	try {
		crec->endTime();
	}
	catch (...) {
		SEISCOMP_ERROR("Filtered record has invalid endtime -> skipping");
		delete crec;
		return nullptr;
	}

	crec->setData(arr.get());

	return crec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float RecordWidget::timingQuality(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return -1;
	return _streams[slot]->traces[Stream::Raw].timingQuality;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordWidget::centerTime() {
	return _alignment + Core::TimeSpan((_tmin+_tmax)*0.5);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::streamYPos(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return 0;
	return _streams[slot]->posY;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::streamHeight(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return height();
	return _streams[slot]->height;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPair<double,double> RecordWidget::amplitudeDataRange(int slot) const {
	if ( (slot >= _streams.size()) || (slot < 0) ) {
		return QPair<double,double>(0,0);
	}

	if ( _showScaledValues ) {
		return QPair<double,double>(
			_streams[slot]->traces[_streams[slot]->filtering?Stream::Filtered:Stream::Raw].dyMin * _streams[slot]->scale,
			_streams[slot]->traces[_streams[slot]->filtering?Stream::Filtered:Stream::Raw].dyMax * _streams[slot]->scale
		);
	}
	else {
		return QPair<double,double>(
			_streams[slot]->traces[_streams[slot]->filtering?Stream::Filtered:Stream::Raw].dyMin,
			_streams[slot]->traces[_streams[slot]->filtering?Stream::Filtered:Stream::Raw].dyMax
		);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPair<double, double> RecordWidget::amplitudeRange(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return QPair<double,double>(-1,1);
	return QPair<double,double>(
		_streams[slot]->traces[_streams[slot]->filtering?Stream::Filtered:Stream::Raw].fyMin,
		_streams[slot]->traces[_streams[slot]->filtering?Stream::Filtered:Stream::Raw].fyMax
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::ensureVisibility(const Core::Time &time,
                                    int pixelMargin) {
	Core::Time left = time - Core::TimeSpan(pixelMargin / _pixelPerSecond);
	Core::Time right = time + Core::TimeSpan(pixelMargin / _pixelPerSecond);

	double offset = 0;

	if ( right > rightTime() ) {
		offset = static_cast<double>(right - rightTime());
	}
	else if ( left < leftTime() ) {
		offset = static_cast<double>(left - leftTime());
	}

	if ( offset != 0 ) {
		setTimeRange(tmin() + offset, tmax() + offset);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::fed(int slot, const Seiscomp::Record *rec) {
	bool newlyCreated = false;

	if ( (slot < 0) || (slot >= _streams.size()) ) {
		return;
	}

	Stream *s = _streams[slot];

	s->axisDirty = true;
	s->traces[Stream::Raw].dirty = true;
	s->traces[Stream::Raw].dirtyData = true;

	if ( rec->timingQuality() >= 0 ) {
		if ( s->traces[Stream::Raw].timingQualityCount == 0 ) {
			s->traces[Stream::Raw].timingQuality = rec->timingQuality();
		}
		else
			s->traces[Stream::Raw].timingQuality =
				(s->traces[Stream::Raw].timingQuality * s->traces[Stream::Raw].timingQualityCount +
				 rec->timingQuality()) / (s->traces[Stream::Raw].timingQualityCount+1);

		++s->traces[Stream::Raw].timingQualityCount;
	}

	_drawRecords = true;

	if ( !(_shadowWidgetFlags & Filtered) && (s->filtering || s->filter) ) {
		newlyCreated = createFilter(slot);

		if ( _shadowWidget ) {
			if ( (_shadowWidget->_shadowWidgetFlags & Filtered) &&
			      !_shadowWidget->_streams[slot]->records[Stream::Filtered]) {
				_shadowWidget->setFilteredRecords(slot, s->records[Stream::Filtered], false);
			}
			_shadowWidget->fed(slot, rec);
		}
	}
	else {
		if ( _shadowWidget  ) {
			if ( (_shadowWidget->_shadowWidgetFlags & Filtered) &&
			      !_shadowWidget->_streams[slot]->records[Stream::Filtered]) {
				_shadowWidget->setFilteredRecords(slot, s->records[Stream::Filtered], false);
			}
			_shadowWidget->fed(slot, rec);
		}

		s->traces[Stream::Filtered].dirty = true;
		s->traces[Stream::Filtered].dirtyData = true;
		return;
	}

	if ( !s->records[Stream::Filtered] || !s->filter ) {
		return;
	}

	if ( !newlyCreated ) {
		try {
			RecordPtr frec = filteredRecord(s->filter, rec,
			                                s->records[Stream::Filtered]->empty() ?
			                                nullptr : s->records[Stream::Filtered]->back().get(),
			                                s->records[Stream::Filtered]->tolerance());
			if ( frec ) {
				s->records[Stream::Filtered]->feed(frec.get());
				s->traces[Stream::Filtered].dirty = true;
				s->traces[Stream::Filtered].dirtyData = true;
			}
		}
		catch ( std::exception &e ) {
			s->traces[Stream::Filtered].status = e.what();
			SEISCOMP_ERROR("%s: filter: %s", rec->streamID().c_str(), e.what());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::addMarker(RecordMarker* marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->addMarker(marker);

	if ( marker == nullptr ) return false;

	if ( marker->parent() != this ) {
		if ( marker->parent() ) {
			marker->parent()->takeMarker(marker);
		}
	}

	marker->setParent(this);
	_marker.push_back(marker);
	//setMouseTracking(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::insertMarker(int pos, RecordMarker* marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->insertMarker(pos, marker);

	if ( marker == nullptr ) return false;

	if ( marker->parent() != this ) {
		if ( marker->parent() )
			marker->parent()->takeMarker(marker);
	}

	marker->setParent(this);

	_marker.insert(pos, marker);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::indexOfMarker(RecordMarker* marker) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->indexOfMarker(marker);

	return _marker.indexOf(marker);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::takeMarker(int pos) {
	if ( _markerSourceWidget ) return _markerSourceWidget->takeMarker(pos);

	if ( pos >= 0 ) {
		RecordMarker *m = _marker[pos];
		if ( m == _activeMarker ) {
			setCurrentMarker(nullptr);
		}
		if ( m == _hoveredMarker )
			_hoveredMarker = nullptr;
		m->setParent(nullptr);
		_marker.remove(pos);
		return m;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::takeMarker(RecordMarker *marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->takeMarker(marker);
	return takeMarker(_marker.indexOf(marker));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::removeMarker(int pos) {
	if ( _markerSourceWidget ) return _markerSourceWidget->removeMarker(pos);
	RecordMarker *m = takeMarker(pos);
	if ( m ) {
		delete m;
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::removeMarker(RecordMarker* marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->removeMarker(marker);
	return removeMarker(_marker.indexOf(marker));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::markerCount() const {
	if ( _markerSourceWidget ) return _markerSourceWidget->markerCount();
	return _marker.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::marker(int i) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->marker(i);
	return _marker[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::marker(const QString& txt, bool movableOnly) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->marker(txt, movableOnly);
	for ( int i = 0; i < _marker.count(); ++i ) {
		if ( !_marker[i]->matches(txt) ) continue;

		if ( !movableOnly || (movableOnly && _marker[i]->isMovable()) )
			return _marker[i];
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::enabledMarker(const QString& txt) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->enabledMarker(txt);
	for ( int i = 0; i < _marker.count(); ++i )
		if ( _marker[i]->matches(txt) && _marker[i]->isEnabled() )
			return _marker[i];

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::clearMarker() {
	_hoveredMarker = nullptr;

	if ( _markerSourceWidget ) {
		_markerSourceWidget->clearMarker();
		return;
	}

	if ( !_marker.isEmpty() ) {
		update();
	}

	while ( !_marker.isEmpty() ) {
		delete _marker[0];
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCurrentMarker(RecordMarker *m) {
	if ( _markerSourceWidget ) {
		if ( m != _markerSourceWidget->_activeMarker )
			update();

		_markerSourceWidget->setCurrentMarker(m);

		return;
	}

	if ( _activeMarker == m ) return;

	_activeMarker = m;
	emit currentMarkerChanged(_activeMarker);

	if ( _shadowWidget && _shadowWidget->_markerSourceWidget == this ) {
		_shadowWidget->update();
		emit _shadowWidget->currentMarkerChanged(_activeMarker);
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::currentMarker() const {
	if ( _markerSourceWidget ) return _markerSourceWidget->currentMarker();
	return _activeMarker;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::hoveredMarker() const {
	return _hoveredMarker;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::markerAt(const QPoint& p, bool movableOnly, int maxDist) const {
	return markerAt(p.x(), p.y(), movableOnly, maxDist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::markerAt(int x, int y, bool movableOnly, int maxDist) const {
	int minDist = maxDist;
	int h = height();

	RecordMarker *m = nullptr;
	for ( int i = markerCount()-1; i >= 0; --i ) {
		RecordMarker *cm = marker(i);
		if ( !cm->isVisible() || (movableOnly && !cm->isMovable()) ) {
			continue;
		}

		int startY = 0, endY = h;

		switch ( cm->_alignment ) {
			case Qt::AlignTop:
				endY = startY + h*2/4-1;
				break;
			case Qt::AlignBottom:
				startY = startY + h*2/4+1;
				break;
		}

		if ( y < startY || y > endY ) {
			continue;
		}

		int x0 = mapTime(cm->correctedTime());
		int dist = abs(x-x0);
		if ( dist < minDist ) {
			minDist = dist;
			m = cm;
		}
	}

	return m;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::lastMarker(const Seiscomp::Core::Time& t) {
	if ( _markerSourceWidget ) return _markerSourceWidget->lastMarker(t);

	int minI = -1;
	double minT = -1;
	for ( int i = 0; i < markerCount(); ++i ) {
		if ( !marker(i)->isVisible() ) {
			continue;
		}
		double delta = (double)(t - marker(i)->correctedTime());
		if ( delta > 0 && (delta < minT || minT < 0) ) {
			minT = delta;
			minI = i;
		}
	}

	if ( minI != -1 )
		return marker(minI);

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::nextMarker(const Seiscomp::Core::Time& t) {
	if ( _markerSourceWidget ) return _markerSourceWidget->nextMarker(t);

	int minI = -1;
	double minT = -1;
	for ( int i = 0; i < markerCount(); ++i ) {
		if ( !marker(i)->isVisible() ) {
			continue;
		}
		double delta = (double)(marker(i)->correctedTime() - t);
		if ( delta > 0 && (delta < minT || minT < 0) ) {
			minT = delta;
			minI = i;
		}
	}

	if ( minI != -1 )
		return marker(minI);

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::nearestMarker(const Seiscomp::Core::Time &t,
                                          int maxDist) {
	int minI = -1;
	double minT = -1;
	for ( int i = 0; i < markerCount(); ++i ) {
		if ( !marker(i)->isVisible() ) {
			continue;
		}
		double delta = fabs((marker(i)->correctedTime() - t).length());
		if ( delta < minT || minT < 0 ) {
			minT = delta;
			minI = i;
		}
	}

	if ( maxDist >= 0 && minT*_pixelPerSecond > maxDist )
		return nullptr;

	if ( minI != -1 )
		return marker(minI);

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::hasMovableMarkers() const {
	if ( _markerSourceWidget ) return _markerSourceWidget->hasMovableMarkers();

	foreach(RecordMarker* m, _marker) {
		if ( m->isMovable() && m->isEnabled() ) {
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCursorText(const QString& text) {
	_cursorText = text;

	setCursor(_active && !_cursorText.isEmpty()?Qt::CrossCursor:Qt::ArrowCursor);

	if ( _active )
		update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordWidget::cursorText() const {
	return _cursorText;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCursorPos(const QPoint& p) {
	double dt = (p.x()-_canvasRect.left())/_pixelPerSecond+_tmin;
	_cursorPos = _alignment + Core::TimeSpan(dt);

	if ( _enabled && _active ) {
		int slot;
		if ( _drawMode == InRows ) {
			int visibleStreams = 0;
			for ( int i = 0; i < _streams.size(); ++i ) {
				if ( _streams[i] && _streams[i]->visible )
					visibleStreams++;
			}
			slot = p.y() * visibleStreams / height();
		}
		else
			slot = _currentSlot;

		update();
		emit cursorMoved(mapToGlobal(p));
		emit cursorUpdated(this, slot);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCursorPos(const Seiscomp::Core::Time& t) {
	_cursorPos = t;
	if ( _enabled && _active ) {
		update();
		emit cursorMoved(mapToGlobal(QPoint(mapTime(_cursorPos), _canvasRect.top()+canvasHeight()/2)));
		emit cursorUpdated(this, _currentSlot);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::Time& RecordWidget::cursorPos() const {
	return _cursorPos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double *RecordWidget::value(const Seiscomp::Core::Time& t) const {
	return _drawMode == Single ? value(_currentSlot, t) : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::mapTime(const Seiscomp::Core::Time &t) const {
	return _canvasRect.left()+(int)(((double)(t-_alignment)-_tmin)*_pixelPerSecond);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::mapCanvasTime(const Core::Time &t) const {
	return (int)(((double)(t-_alignment)-_tmin)*_pixelPerSecond);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time RecordWidget::unmapTime(int x) const {
	return _alignment + Core::TimeSpan((x-_canvasRect.left())/_pixelPerSecond+_tmin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::WaveformStreamID& RecordWidget::streamID() const {
	return _streamID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setSlotCount(int c) {
	if ( _shadowWidget ) {
		_shadowWidget->setSlotCount(c);
	}

	bool isDirty = false;

	int oldSize = _streams.size();
	if ( c < oldSize ) {
		// Delete ununsed slots
		for ( int i = c; i < oldSize; ++i ) {
			delete _streams[i];
			isDirty = true;
		}
	}

	_streams.resize(c);

	for ( int i = oldSize; i < _streams.size(); ++i ) {
		_streams[i] = new Stream(true);
		_streams[i]->hasCustomBackgroundColor = _hasCustomBackground;
		_streams[i]->customBackgroundColor = _customBackgroundColor;
		_streams[i]->filtering = _filtering;
		isDirty = true;
	}

	if ( isDirty && ((_drawMode == Stacked) || (_drawMode == SameOffset)) ) {
		setDirty();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::slotCount() const {
	return _streams.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::updateRecords() {
	if ( _drawRecords ) {
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setData(const QVariant& data) {
	_data = data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant RecordWidget::data() const {
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawTrace(QPainter &painter,
                             const Trace *trace,
                             const RecordSequence *seq, const QPen &pen,
                             const QPoint &paintOffset) const {
	painter.setPen(pen);
	painter.translate(paintOffset);
	trace->poly->draw(painter);
	drawRecordBorders(painter, seq);
	painter.translate(paintOffset * -1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawRecordBorders(QPainter &painter, const RecordSequence *seq) const {
	if ( !_showRecordBorders ) return;

	painter.setRenderHint(QPainter::Antialiasing, false);

	int height = fontMetrics().height() / 3;
	Core::Time start = leftTime(),
	           end = rightTime();

	for ( RecordSequence::const_iterator it = seq->begin();
	      it != seq->end(); ++it ) {
		const Record *rec = it->get();

		// Skip records that are out of time window [start:end]
		try {
			if ( rec->endTime() <= start ) continue;
		}
		catch ( ... ) { continue; }

		if ( rec->startTime() >= end ) break;

		int nsamp = rec->sampleCount();
		if ( nsamp == 0 ) continue;

		int xMin = int(-(_tmin + static_cast<double>((_alignment - rec->startTime()))) *_pixelPerSecond);
		int xMax = int(-(_tmin + static_cast<double>((_alignment - rec->endTime()))) *_pixelPerSecond);

		QPen pen;
		QBrush brush;

		if ( rec->authentication() == Record::SIGNATURE_VALIDATED ) {
			pen = SCScheme.colors.records.borders.signatureValid.pen;
			brush = SCScheme.colors.records.borders.signatureValid.brush;
		}
		else if ( rec->authentication() == Record::SIGNATURE_VALIDATION_FAILED ) {
			pen = SCScheme.colors.records.borders.signatureInvalid.pen;
			brush = SCScheme.colors.records.borders.signatureInvalid.brush;
		}
		else {
			pen = SCScheme.colors.records.borders.standard.pen;
			brush = SCScheme.colors.records.borders.standard.brush;
		}

		QRect rect;
		if ( _recordBorderDrawMode == TopLine ) {
			rect = QRect(xMin, 0 , xMax - xMin, height);
		}
		else if ( _recordBorderDrawMode == BottomLine ) {
			rect = QRect(xMin, this->height() - height - 1, xMax - xMin, height);
		}
		else {
			rect = QRect(xMin, 0 , xMax - xMin, this->height() - 1);
		}

		painter.setPen(pen);
		painter.setBrush(brush);
		painter.drawRect(rect);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showRecordBorders(bool enable) {
	if ( _showRecordBorders == enable ) return;

	_showRecordBorders = enable;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setRecordBorderDrawMode(RecordBorderDrawMode mode) {
	if ( _recordBorderDrawMode == mode ) return;

	_recordBorderDrawMode = mode;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::RecordBorderDrawMode RecordWidget::recordBorderDrawMode() const {
	return _recordBorderDrawMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawMarkers(QPainter &painter, QFont &font, const QColor &fg) {
	font = painter.font();
	QFontInfo fi(font);
	int fontSize = std::min(height() / 2, fi.pixelSize());

	font.setPixelSize(fontSize);

	//font.setBold(true);
	painter.setFont(font);

	auto markerList = &_marker;
	auto activeMarker = _activeMarker;

	if ( _markerSourceWidget ) {
		markerList = &_markerSourceWidget->_marker;
		activeMarker = _markerSourceWidget->_activeMarker;
	}

	int markerCanvasOffset = 0;
	int markerCanvasHeight = height();

	int x;

	foreach ( RecordMarker* m, *markerList ) {
		if ( m->isHidden() ) continue;

		int startY = markerCanvasOffset, endY = startY + markerCanvasHeight;
		int textY = markerCanvasOffset + fontSize + 1;

		switch ( m->_alignment ) {
			case Qt::AlignTop:
				endY = markerCanvasOffset + markerCanvasHeight*2/4-1;
				break;
			case Qt::AlignBottom:
				startY = markerCanvasOffset + markerCanvasHeight*2/4+1;
				textY = markerCanvasOffset + markerCanvasHeight-2;
				break;
		}

		bool enabled = _enabled && m->isEnabled();

		if ( m->isMoveCopyEnabled() ) {
			x = mapTime(m->time());
			x -= _canvasRect.left();

			QColor c(enabled?m->color():fg);
			c.setAlpha(64);

			painter.setPen(QPen(c, SCScheme.marker.lineWidth));
			painter.drawLine(x, startY, x, endY);
			//painter.drawRect(textRect.translated(x,textY-3));
			painter.drawText(x+2, textY, m->renderText());
		}

		x = mapTime(m->correctedTime());
		x -= _canvasRect.left();

		/*
		QColor c(m->color());
		c.setRed(c.red()/2);
		c.setGreen(c.green()/2);
		c.setBlue(c.blue()/2);

		painter.setPen(c);
		painter.drawLine(x+1, 0, x+1, h);
		painter.drawText(x+2+1, fontSize+1, m->renderText());
		*/

		//painter.drawRect(textRect.translated(x,textY-3));
		if ( m->isMovable() && m->isModified() ) {
			painter.setPen(enabled?m->modifiedColor():fg);
			painter.drawText(x+2, textY, m->renderText());
			m->draw(painter, this, x, markerCanvasOffset, markerCanvasOffset + markerCanvasHeight,
			        enabled?m->modifiedColor():fg, SCScheme.marker.lineWidth);
		}
		else {
			painter.setPen(enabled?m->color():fg);
			painter.drawText(x+2, textY, m->renderText());
			m->draw(painter, this, x, startY, endY,
			        enabled?m->color():fg, SCScheme.marker.lineWidth);
		}

		if ( m == _hoveredMarker ) {
			QPen pen = painter.pen();
			painter.setPen(QPen(palette().color(QPalette::Highlight), 1, Qt::DotLine));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x-4, startY, 8, endY-startY+1);
			painter.setPen(pen);
		}

		if ( m == activeMarker ) {
			QPen pen = painter.pen();
			painter.setPen(QPen(palette().color(QPalette::Text), 1, Qt::DashLine));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x-4, startY, 8, endY-startY+1);
			painter.setPen(pen);
		}
	}

	if ( _active && _enabled && !_cursorText.isEmpty() ) {
		painter.translate(-_canvasRect.left(), -_canvasRect.top());
		drawActiveCursor(painter, mapTime(_cursorPos), _currentCursorYPos);
		painter.translate(_canvasRect.left(), _canvasRect.top());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
