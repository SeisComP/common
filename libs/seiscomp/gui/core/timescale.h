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



#ifndef SEISCOMP_GUI_TIMESCALE_H
#define SEISCOMP_GUI_TIMESCALE_H

#include "ruler.h"
#include <seiscomp/core/datetime.h>

#define  REPAINT_WITHOUT_ERASE   FALSE
#define  REPAINT_AFTER_ERASE     TRUE

namespace Seiscomp {
namespace Gui {

class SC_GUI_API TimeScale : public Ruler {
	Q_OBJECT

	public:
		TimeScale(QWidget *parent = 0, Qt::WindowFlags f = 0, Position pos = Bottom);
		~TimeScale(){}

		void setTimeRange(double tmin, double tmax) {
			setRange(tmin, tmax);
		}

		double tmin() const { return _min; }
		double tmax() const { return _max; }

		void setAlignment(const Core::Time& t);
		const Core::Time &alignment() const { return _alignment; }


	public slots:
		void setAbsoluteTimeEnabled(bool absoluteTime, bool absoluteDate = true);

	protected:
		bool getTickText(double pos, double lastPos,
		                 int line, QString &str) const;
		void updateIntervals();

	protected:
		Core::Time  _alignment;
		bool        _showAbsoluteValues;
		bool        _showAbsoluteDate;
		const char *_primaryTimeFormat;
		const char *_secondaryTimeFormat;
		const char *_relativeTimeFormat;
};


}
}

# endif
