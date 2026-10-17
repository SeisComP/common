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


#define SEISCOMP_COMPONENT Gui::Utils

#include <seiscomp/datamodel/configmodule.h>
#include <seiscomp/datamodel/configstation.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/math/math.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/datamodel/utils.h>
#include <seiscomp/gui/map/canvas.h>
#include <seiscomp/gui/map/projection.h>

#include <QActionGroup>
#include <QMenu>
#include <QPainter>

#include <algorithm>


namespace Seiscomp {
namespace Gui {


double elevation(const DataModel::Station *sta) {
	try {
		return sta->elevation();
	}
	catch ( ... ) {
		return 0;
	}
}


double elevation(const DataModel::SensorLocation *sloc) {
	try {
		return sloc->elevation();
	}
	catch ( ... ) {
		if ( sloc->station() ) {
			return elevation(sloc->station());
		}
	}
	return 0;
}


double computeDistance(const DataModel::Origin *org,
                       const DataModel::Station *sta,
                       double defaultDepth,
                       double *az, double *baz,
                       double *epicentral) {
	double salt = elevation(sta);
	double edep = defaultDepth;
	try {
		edep = org->depth();
	}
	catch ( ... ) {}

	return computeDistance(org->latitude(), org->longitude(), edep,
	                       sta->latitude(), sta->longitude(), salt,
	                       az, baz, epicentral);
}


double computeDistance(const DataModel::Origin *org,
                       const DataModel::SensorLocation *sloc,
                       double defaultDepth, double *az, double *baz,
                       double *epicentral) {
	double salt = elevation(sloc);
	double edep = defaultDepth;
	try {
		edep = org->depth();
	}
	catch ( ... ) {}

	return computeDistance(org->latitude(), org->longitude(), edep,
	                       sloc->latitude(), sloc->longitude(), salt,
	                       az, baz, epicentral);
}


void UnassociatedStations::readConfig() {
	if ( !SCApp ) {
		return;
	}

	try {
		maxDistance = SCApp->configGetDouble("olv.map.stations.unassociatedMaxDist");
	}
	catch ( ... ) {}

	try {
		auto value = SCApp->configGetString("olv.map.stations.unassociated");
		if ( !mode.fromString(value) ) {
			SEISCOMP_WARNING("olv.map.stations.unassociated: invalid value '%s', "
			                 "using '%s'", value.c_str(), mode.toString());
		}
	}
	catch ( ... ) {}

	try {
		distanceFactor = SCApp->configGetDouble("olv.map.stations.unassociatedDistanceFactor");
		if ( distanceFactor <= 0 ) {
			SEISCOMP_WARNING("olv.map.stations.unassociatedDistanceFactor: "
			                 "must be positive, using 1");
			distanceFactor = 1;
		}
	}
	catch ( ... ) {}

	try {
		hideDisabled = SCApp->configGetBool("olv.map.stations.hideDisabled");
	}
	catch ( ... ) {}

	try {
		hideUnconfigured = SCApp->configGetBool("olv.map.stations.hideUnconfigured");
	}
	catch ( ... ) {}

	try {
		requiredBinding = SCApp->configGetString("olv.map.stations.requiredBinding");
	}
	catch ( ... ) {}

	try {
		highlightInRange = SCApp->configGetBool("olv.map.stations.highlightInRange");
	}
	catch ( ... ) {}

	try {
		drawDistanceCircles = SCApp->configGetBool("olv.map.stations.drawDistanceCircles");
	}
	catch ( ... ) {}
}


bool UnassociatedStations::isVisible(double distance, double usedDistance,
                                     bool enabled, bool configured) const {
	if ( mode == Off ) {
		return false;
	}

	if ( hideDisabled && !enabled ) {
		return false;
	}

	if ( hideUnconfigured && !configured ) {
		return false;
	}

	// Without used stations all stations are shown, e.g. to find stations
	// for an artificial origin
	if ( (mode == WithinUsedDistance) && (usedDistance >= 0)
	  && (distance > usedDistance * distanceFactor) ) {
		return false;
	}

	return true;
}


bool UnassociatedStations::isInRange(double distance, double usedDistance) const {
	return (usedDistance >= 0) && (distance <= usedDistance);
}


QMenu *UnassociatedStations::addMenu(QMenu *parent,
                                     const std::function<void()> &changed,
                                     int inRangeCount) {
	auto menu = parent->addMenu(QObject::tr("Unassociated stations"));
	auto group = new QActionGroup(menu);

	auto addMode = [&](const QString &text, EMode value) {
		auto action = menu->addAction(text);
		action->setCheckable(true);
		action->setChecked(mode == value);
		group->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, value, changed]() {
			mode = value;
			changed();
		});
	};

	auto addFlag = [&](const QString &text, bool &flag) {
		auto action = menu->addAction(text);
		action->setCheckable(true);
		action->setChecked(flag);
		QObject::connect(action, &QAction::triggered, [&flag, changed](bool checked) {
			flag = checked;
			changed();
		});
	};

	addMode(QObject::tr("Show all"), All);
	addMode(QObject::tr("Show within %1 x used distance").arg(distanceFactor), WithinUsedDistance);
	addMode(QObject::tr("Hide"), Off);
	menu->addSeparator();
	if ( inRangeCount >= 0 ) {
		addFlag(QObject::tr("Highlight within used distance (%1)").arg(inRangeCount), highlightInRange);
	}
	else {
		addFlag(QObject::tr("Highlight within used distance"), highlightInRange);
	}
	addFlag(QObject::tr("Draw distance circles"), drawDistanceCircles);
	addFlag(QObject::tr("Hide disabled stations"), hideDisabled);
	if ( requiredBinding.empty() ) {
		addFlag(QObject::tr("Hide stations without bindings"), hideUnconfigured);
	}
	else {
		addFlag(QObject::tr("Hide stations without %1 binding").arg(requiredBinding.c_str()), hideUnconfigured);
	}

	return menu;
}


QString UnassociatedStations::toolTip(const std::string &net,
                                      const std::string &sta,
                                      double distance, double usedDistance,
                                      bool enabled, bool configured) const {
	QString text = QString("%1.%2").arg(net.c_str(), sta.c_str());

	if ( SCScheme.unit.distanceInKM ) {
		text += QString("\n%1 km").arg(Math::Geo::deg2km(distance), 0, 'f',
		                               SCScheme.precision.distance);
	}
	else {
		text += QString("\n%1°").arg(distance, 0, 'f', 1);
	}

	if ( isInRange(distance, usedDistance) ) {
		text += QObject::tr("\nWithin used distance, not associated");
	}

	if ( !enabled ) {
		text += QObject::tr("\nDisabled");
	}

	if ( !configured ) {
		if ( requiredBinding.empty() ) {
			text += QObject::tr("\nNo bindings");
		}
		else {
			text += QObject::tr("\nNo %1 binding").arg(requiredBinding.c_str());
		}
	}

	return text;
}


void UnassociatedStations::drawCircles(const Map::Canvas *canvas,
                                       QPainter &painter,
                                       const QPointF &epicenter,
                                       double usedDistance) const {
	if ( !drawDistanceCircles || (usedDistance <= 0) ) {
		return;
	}

	auto drawCircle = [&](double distance) {
		QPointF last;
		for ( int az = 0; az <= 360; az += 2 ) {
			QPointF p;
			Math::Geo::delandaz2coord(distance, az, epicenter.y(), epicenter.x(),
			                          &p.ry(), &p.rx());
			if ( az > 0 ) {
				canvas->projection()->drawLine(painter, last, p);
			}
			last = p;
		}
	};

	painter.save();
	painter.setBrush(Qt::NoBrush);

	painter.setPen(SCScheme.colors.map.usedDistance);
	drawCircle(usedDistance);

	if ( (mode == WithinUsedDistance) && (distanceFactor != 1) ) {
		painter.setPen(SCScheme.colors.map.distanceLimit);
		drawCircle(std::min({usedDistance * distanceFactor, maxDistance, 180.0}));
	}

	painter.restore();
}


void UnassociatedStations::StationStates::read(const std::string &requiredBinding) {
	stations.clear();

	auto module = SCApp ? SCApp->configModule() : nullptr;
	available = module != nullptr;
	if ( !module ) {
		return;
	}

	for ( size_t i = 0; i < module->configStationCount(); ++i ) {
		auto cs = module->configStation(i);
		auto &state = stations[cs->networkCode() + "." + cs->stationCode()];
		state.enabled = cs->enabled();
		state.configured = requiredBinding.empty()
		                || DataModel::findSetup(cs, requiredBinding, false);
	}
}


void UnassociatedStations::StationStates::get(const std::string &net,
                                              const std::string &sta,
                                              bool &enabled,
                                              bool &configured) const {
	if ( !available ) {
		enabled = configured = true;
		return;
	}

	auto it = stations.find(net + "." + sta);
	if ( it == stations.end() ) {
		enabled = true;
		configured = false;
		return;
	}

	enabled = it->second.enabled;
	configured = it->second.configured;
}


}
}
