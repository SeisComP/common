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


#ifndef SEISCOMP_GUI_DATAMODEL_UTIL_H
#define SEISCOMP_GUI_DATAMODEL_UTIL_H


#include <seiscomp/gui/qt.h>
#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/utils.h>
#endif
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/core/enumeration.h>
#include <QCheckBox>

#include <functional>
#include <map>
#include <string>


class QMenu;
class QPainter;
class QPointF;


namespace Seiscomp {
namespace Gui {
namespace Map {

class Canvas;

}
}
}


namespace Seiscomp {
namespace Gui {


class CheckBox : public QCheckBox {
	Q_OBJECT

	public:
		explicit CheckBox(QWidget *parent=0)
		: QCheckBox(parent) {}

		explicit CheckBox(const QString &text, QWidget *parent=0)
		: QCheckBox(text, parent) {}


	public slots:
		void check() {
			setChecked(true);
		}

		void unCheck() {
			setChecked(false);
		}
};


SC_GUI_API double elevation(const DataModel::SensorLocation *sloc);
SC_GUI_API double elevation(const DataModel::Station *sta);


SC_GUI_API double computeDistance(const DataModel::Origin *org,
                                  const DataModel::Station *sta,
                                  double defaultDepth,
                                  double *az = nullptr, double *baz = nullptr,
                                  double *epicentral = nullptr);
SC_GUI_API double computeDistance(const DataModel::Origin *org,
                                  const DataModel::SensorLocation *sloc,
                                  double defaultDepth,
                                  double *az = nullptr, double *baz = nullptr,
                                  double *epicentral = nullptr);


/**
 * @brief Controls which unassociated stations are shown in the origin and
 *        magnitude maps. Unassociated stations are inventory stations which
 *        do not contribute to the current solution, e.g. stations without
 *        arrival.
 *
 * The distance of the used station which is farthest from the epicenter is
 * called the used distance. It is negative if no station is used.
 */
struct SC_GUI_API UnassociatedStations {
	MAKEENUM(
		Mode,
		EVALUES(
			// Show all stations up to maxDistance
			All,
			// Show stations up to distanceFactor times the used distance
			WithinUsedDistance,
			// Show no unassociated stations
			Off
		),
		ENAMES(
			"all",
			"auto",
			"off"
		)
	);

	Mode   mode{All};
	//! The maximum distance in degrees of stations to be considered
	double maxDistance{360};
	//! The factor applied to the used distance in mode WithinUsedDistance
	double distanceFactor{1.2};
	//! Hide stations which are disabled in the configuration module
	bool   hideDisabled{false};
	//! Hide stations without bindings in the configuration module
	bool   hideUnconfigured{false};
	//! If not empty, a station is only considered configured if it has an
	//! enabled binding of this module, e.g. "scautopick"
	std::string requiredBinding;
	//! Highlight stations within the used distance
	bool   highlightInRange{true};
	//! Draw circles at the used distance and at the distance limit of
	//! mode WithinUsedDistance
	bool   drawDistanceCircles{true};

	//! Reads the olv.map.stations.* parameters from the application
	//! configuration.
	void readConfig();

	//! Returns whether an unassociated station with the given epicentral
	//! distance and state is shown.
	bool isVisible(double distance, double usedDistance,
	               bool enabled, bool configured) const;

	//! Returns whether a station with the given epicentral distance is
	//! within the used distance.
	bool isInRange(double distance, double usedDistance) const;

	//! Adds a submenu to change the options interactively. The callback is
	//! called after an option has been changed. If inRangeCount is not
	//! negative it is shown as number of stations within the used distance.
	QMenu *addMenu(QMenu *parent, const std::function<void()> &changed,
	               int inRangeCount = -1);

	//! Returns the tool tip of an unassociated station
	QString toolTip(const std::string &net, const std::string &sta,
	                double distance, double usedDistance,
	                bool enabled, bool configured) const;

	//! Draws the circles around the epicenter (lon, lat) if enabled
	void drawCircles(const Map::Canvas *canvas, QPainter &painter,
	                 const QPointF &epicenter, double usedDistance) const;

	//! The binding and enabled state of stations in the configuration
	//! module of the application.
	struct SC_GUI_API StationStates {
		//! Reads the states from the configuration module. If
		//! requiredBinding is not empty, only stations with an enabled
		//! binding of this module are configured.
		void read(const std::string &requiredBinding = std::string());

		//! Returns the state of a station. Without configuration module
		//! all stations are configured and enabled.
		void get(const std::string &net, const std::string &sta,
		         bool &enabled, bool &configured) const;

		struct State {
			bool enabled{true};
			bool configured{false};
		};

		bool                         available{false};
		std::map<std::string, State> stations;
	};
};


}
}


#endif
