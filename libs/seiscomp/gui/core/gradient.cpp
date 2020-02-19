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



#include <seiscomp/gui/core/gradient.h>

namespace Seiscomp {
namespace Gui {


namespace {

QColor blend(const QColor& c1, const QColor& c2, qreal ratio) {
	qreal invRatio = 1-ratio;
	return QColor((int)(c1.red()*invRatio + c2.red()*ratio),
	              (int)(c1.green()*invRatio + c2.green()*ratio),
	              (int)(c1.blue()*invRatio + c2.blue()*ratio),
	              (int)(c1.alpha()*invRatio + c2.alpha()*ratio));
}


}


Gradient::Gradient() {}


void Gradient::setColorAt(qreal position, const QColor &color, const QString& text) {
	insert(position, qMakePair(color, text));
}


QColor Gradient::colorAt(qreal position, bool discrete) const {
	const_iterator last = end();
	for ( const_iterator it = begin(); it != end(); ++it ) {
		if ( it.key() == position )
			return it.value().first;
		else if ( it.key() > position ) {
			if ( last != end() ) {
				if ( discrete ) {
					return last.value().first;
				}
				else {
					qreal v1 = last.key();
					qreal v2 = it.key();
					return blend(last.value().first, it.value().first, (position-v1)/(v2-v1));
				}
			}
			else
				return it.value().first;
		}

		last = it;
	}

	if ( last != end() )
		return last.value().first;

	return QColor();
}


}
}
