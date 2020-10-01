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


#ifndef SEISCOMP_GUI_PLOT_LEGEND_H
#define SEISCOMP_GUI_PLOT_LEGEND_H


#include <seiscomp/gui/plot/abstractlegend.h>


namespace Seiscomp {
namespace Gui {


/**
 * @brief The Legend class renders a default legend for all visible graphs
 *        where the name is not empty.
 */
class SC_GUI_API Legend : public AbstractLegend {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit Legend(Qt::Alignment align = Qt::AlignTop | Qt::AlignRight,
		                QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Render interface
	// ----------------------------------------------------------------------
	public:
		virtual void draw(QPainter &p, const QRect &plotRect,
		                  const QList<Graph*> &graphs);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		Qt::Alignment _alignment;
};


}
}


#endif
