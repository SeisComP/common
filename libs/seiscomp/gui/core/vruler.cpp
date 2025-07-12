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


#include "vruler.h"

#include <seiscomp/gui/core/recordwidget.h>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VRuler::VRuler(QWidget *p, Qt::WindowFlags f)
: Seiscomp::Gui::Ruler(p, f, Seiscomp::Gui::Ruler::Left) {
	setAutoScaleEnabled(true);
	setRange(-1, 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void VRuler::setAnnotation(QString annotation) {
	_annotation = annotation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString &VRuler::annotation() const {
	return _annotation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool VRuler::getTickText(double pos, double lastPos, int line, QString &str) const {
	if ( line != 0 ) {
		return false;
	}

	double apos = fabs(pos);
	if ( apos > 1000 ) {
		str.setNum(pos, 'g', 2);
	}
	else if ( apos > 1 ) {
		str.setNum(pos, 'f', 1);
	}
	else {
		double epsilon = fabs(pos-lastPos) * 1E-2;
		if ( apos < epsilon ) {
			str = "0";
		}
		else {
			str.setNum(pos);
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void VRuler::updateScale(Seiscomp::Gui::RecordWidget *w) {
	if ( w->drawMode() == Seiscomp::Gui::RecordWidget::Stacked ) {
		int slotCount = w->slotCount();
		QPair<double,double> amps(0,0);
		bool first = true;

		for ( int i = 0; i < slotCount; ++i ) {
			if ( !w->isRecordVisible(i) ) {
				continue;
			}

			if ( w->isFilteringEnabled() ) {
				if ( !w->filteredRecords(i) ) {
					continue;
				}
			}
			else {
				if ( !w->records(i) ) {
					continue;
				}
			}

			auto range = w->amplitudeRange(i);

			if ( w->areScaledValuesShown() ) {
				auto scale = w->recordScale(i);
				if ( scale ) {
					range.first *= *scale;
					range.second *= *scale;
				}
			}

			if ( range.first < range.second ) {
				if ( first ) {
					amps = range;
				}
				else {
					amps.first = std::min(amps.first, range.first);
					amps.second = std::max(amps.second, range.second);
				}
			}
		}

		setRange(amps.first, amps.second);
		w->setGridVRange(amps.first, amps.second);
	}
	else {
		// Returns the visible amplitude range with optional scale
		auto range = w->amplitudeRange(w->currentRecords());

		if ( w->areScaledValuesShown() ) {
			auto scale = w->recordScale(w->currentRecords());
			if ( scale ) {
				range.first *= *scale;
				range.second *= *scale;
			}
		}

		setRange(range.first, range.second);
		w->setGridVRange(range.first, range.second);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void VRuler::paintEvent(QPaintEvent *e) {
	Ruler::paintEvent(e);

	if ( _annotation.isEmpty() ) {
		return;
	}

	QPainter p(this);
	QRect r = rect();

	QSize ts = p.fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, _annotation).size();
	if ( ts.width() >= r.height() ) {
		return;
	}

	int x = r.left();
	int y = (r.height() + r.width()) / 2 + r.top();

	p.save();
	p.translate(x, y);
	p.rotate(-90);
	p.drawText(0, 0, r.width(), ts.height(), Qt::TextDontClip | Qt::AlignCenter, _annotation);
	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
