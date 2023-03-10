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



#include <QWheelEvent>

#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/datamodel/pickerzoomframe.h>

namespace Seiscomp {
namespace Gui {


ZoomRecordFrame::ZoomRecordFrame(QWidget* parent, Qt::WindowFlags f)
 : QFrame(parent, f) {}


void ZoomRecordFrame::wheelEvent(QWheelEvent* e) {
	if ( e->modifiers() != Qt::NoModifier ) {
		if ( e->modifiers() & Qt::ShiftModifier ) {
			if ( QT_WE_DELTA(e) < 0 )
				emit horizontalZoomOut();
			else
				emit horizontalZoomIn();
		}
	
		if ( e->modifiers() & Qt::ControlModifier ) {
			if ( QT_WE_DELTA(e) < 0 )
				emit verticalZoomOut();
			else
				emit verticalZoomIn();
		}
	}
	else {
		if ( QT_WE_DELTA(e) < 0 )
			emit lineDown();
		else
			emit lineUp();
	}
}


}
}
