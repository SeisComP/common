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



#ifndef SEISCOMP_GUI_CORE_GRADIENT_H
#define SEISCOMP_GUI_CORE_GRADIENT_H

#include <seiscomp/gui/qt.h>

#include <QColor>
#include <QMap>
#include <QPair>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API Gradient : public QMap<qreal, QPair<QColor, QString> > {
	public:
		typedef QPair<QColor, QString> ValueType;

	public:
		Gradient();

	public:
		//! Sets the color and text at a specified position
		void setColorAt(qreal position, const QColor &color,
		                const QString& text = "");

		//! Returns the color at position. If position falls between
		//! two color positions, the resulting color will be
		//! interpolated linearly between both colors. When the discrete flag
		//! is set, the interpolation will be disabled
		QColor colorAt(qreal position, bool discrete = false) const;
};


}
}


# endif
