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



#define SEISCOMP_COMPONENT Gui::OriginLocatorMap
#include "originlocatormap.h"
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/math/math.h>

#include <seiscomp/gui/datamodel/advancedoriginsymbol.h>
#include <seiscomp/gui/datamodel/stationsymbol.h>
#include <seiscomp/gui/datamodel/ttdecorator.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/map/layers/annotationlayer.h>

#include <QMenu>

#include <algorithm>
#include <utility>


#ifdef WIN32
#undef min
#undef max
#endif


using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;


namespace Seiscomp {
namespace Gui {


namespace {


struct StationLayer : Map::Layer {
	struct Symbol;

	StationLayer(OriginLocatorMap *map)
	: Map::Layer(map) {
		// No unassociated stations unless a maximum distance is set
		unassociated.maxDistance = -1;
	}

	~StationLayer() override {
		clear();
	}

	void setVisible(bool v) {
		Map::Layer::setVisible(v);
		if ( !v ) {
			for ( auto entry : std::as_const(stations) ) {
				if ( entry->annotation ) {
					entry->annotation->visible = false;
				}
			}
		}
		else {
			updateAnnotations();
		}
	}

	void calculateMapPosition(const Map::Canvas *canvas) override {
		for ( auto entry : std::as_const(stations) ) {
			if ( !entry->validLocation ) {
				entry->setVisible(false);
			}
			else {
				entry->setVisible(true);
				entry->calculateMapPosition(canvas);
			}
		}

		updateAnnotations();
	}

	// Computes the distance of the farthest used station
	void updateUsedDistance() {
		usedDistance = -1;
		for ( auto s : std::as_const(stations) ) {
			if ( s->isArrival && s->isActive && s->validLocation ) {
				usedDistance = std::max(usedDistance, s->distance);
			}
		}
	}

	bool isShown(const Symbol *s) const {
		return s->isArrival
		    || unassociated.isVisible(s->distance, usedDistance,
		                              s->isEnabled, s->isConfigured);
	}

	void updateAnnotations() {
		for ( auto entry : std::as_const(stations) ) {
			if ( entry->annotation ) {
				entry->annotation->visible = entry->isVisible() && isShown(entry);
			}
		}
	}

	// Returns the number of shown and enabled unassociated stations within
	// the used distance or -1 if no station is used
	int inRangeCount() const {
		if ( usedDistance < 0 ) {
			return -1;
		}

		int count = 0;
		for ( auto s : std::as_const(stations) ) {
			if ( !s->isArrival && s->isActive && s->isEnabled && isShown(s)
			  && unassociated.isInRange(s->distance, usedDistance) ) {
				++count;
			}
		}

		return count;
	}

	bool isInside(const QMouseEvent *event, const QPointF &) override {
		int tmpHoverId = -1;
		auto p = QT_EVENT_POS(event);

		for ( int i = 0; i < stations.count(); ++i ) {
			if ( !stations[i]->isVisible() || !isShown(stations[i]) ) {
				continue;
			}

			if ( stations[i]->isInside(p.x(), p.y()) ) {
				tmpHoverId = i;
				break;
			}
		}

		if ( tmpHoverId != hoverId ) {
			hoverId = tmpHoverId;
			if ( hoverId != -1 ) {
				auto s = stations[hoverId];
				int arrivalId = s->arrivalId;
				static_cast<OriginLocatorMap*>(parent())->hoverArrival(arrivalId);
				setToolTip(static_cast<OriginLocatorMap*>(parent())->toolTip());
				if ( !s->isArrival && !s->net.empty() && !s->code.empty() ) {
					setToolTip(unassociated.toolTip(s->net, s->code, s->distance,
					                                usedDistance, s->isEnabled,
					                                s->isConfigured));
				}
				else if ( toolTip().isEmpty() ) {
					if ( !stations[hoverId]->net.empty()
					  && !stations[hoverId]->code.empty() ) {
						setToolTip((stations[hoverId]->net + "." + stations[hoverId]->code).c_str());
					}
				}
			}
			else {
				setToolTip(QString());
				static_cast<OriginLocatorMap*>(parent())->hoverArrival(-1);
			}
		}

		return hoverId != -1;
	}

	void draw(const Map::Canvas *canvas, QPainter &p) override {
		int size = SCScheme.map.stationSize;

		QPoint annotationOffset(0, -size - p.fontMetrics().height() / 2);

		// The used stations might have changed interactively
		updateUsedDistance();
		updateAnnotations();

		if ( drawStationsLines && canvas->symbolCollection()->count() > 0 ) {
			const Map::Symbol *originSymbol = (*canvas->symbolCollection()->begin());
			int cutOff = originSymbol->size().width();

			if ( cutOff ) {
				p.setClipping(true);
				p.setClipRegion(
					QRegion(
						p.window()
					)
					-
					QRegion(
						QRect(
							originSymbol->pos().x() - cutOff / 2,
							originSymbol->pos().y() - cutOff / 2,
							cutOff, cutOff
						),
						QRegion::Ellipse
					)
				);
			}

			p.setPen(SCScheme.colors.map.lines);
			for ( auto s : std::as_const(stations) ) {
				if ( !s->validLocation || !s->isArrival ) {
					continue;
				}

				if ( !s->isActive ) {
					continue;
				}

				canvas->drawLine(p, originSymbol->location(), s->location());
			}

			if ( drawDirectivity ) {
				p.setPen(SCScheme.colors.map.directivity);
				for ( auto s : std::as_const(stations) ) {
					if ( s->backAzimuth ) {
						QPointF p0;
						Math::Geo::delandaz2coord(s->distance * 0.5, *s->backAzimuth,
						                          s->location().y(), s->location().x(),
						                          &p0.ry(), &p0.rx());
						canvas->drawLine(p, s->location(), p0);
					}
				}
			}

			if ( cutOff )
				p.setClipping(false);
		}

		if ( canvas->symbolCollection()->count() > 0 ) {
			unassociated.drawCircles(
				canvas, p, (*canvas->symbolCollection()->begin())->location(),
				usedDistance
			);
		}

		for ( int i = stations.count()-1; i >= 0; --i ) {
			if ( stations[i]->isClipped() || !stations[i]->isVisible() ) {
				continue;
			}

			if ( !interactive && !stations[i]->isActive ) {
				continue;
			}

			if ( !isShown(stations[i]) ) {
				continue;
			}

			if ( stations[i]->isArrival ) {
				stations[i]->setColor(
					stations[i]->isActive ? stations[i]->color : SCScheme.colors.arrivals.disabled
				);
			}
			else if ( !stations[i]->isActive ) {
				stations[i]->setColor(Qt::gray);
			}
			else if ( !stations[i]->isEnabled ) {
				stations[i]->setColor(SCScheme.colors.stations.disabled);
			}
			else if ( unassociated.highlightInRange
			       && unassociated.isInRange(stations[i]->distance, usedDistance) ) {
				stations[i]->setColor(SCScheme.colors.stations.unassociatedInRange);
			}
			else {
				stations[i]->setColor(SCScheme.colors.stations.idle);
			}

			stations[i]->draw(canvas, p);

			if ( stations[i]->annotation ) {
				stations[i]->annotation->updateLabelRect(p, stations[i]->pos() + annotationOffset);
			}
		}
	}

	QVector<int> sort() {
		QVector<int> permutation(stations.size());
		for ( int i = 0; i < stations.size(); ++i ) {
			permutation[i] = i;
		}

		std::sort(permutation.begin(), permutation.end(), [this](int i1, int i2) {
			return stations[i1]->latitude() < stations[i2]->latitude();
		});

		std::sort(stations.begin(), stations.end(), [](Symbol *s1, Symbol *s2) {
			return s1->latitude() < s2->latitude();
		});

		QVector<int> reversePermutation(permutation.size());
		for ( int i = 0; i < permutation.size(); ++i ) {
			reversePermutation[permutation[i]] = i;
		}

		return reversePermutation;
	}

	void clear() {
		for ( auto symbol : std::as_const(stations) ) {
			delete symbol;
		}
		stations.clear();
	}

	struct Symbol : StationSymbol {
		Symbol() = delete;

		Symbol(QPointF loc, const std::string &nc,
		       const std::string &sc, bool valid,
		       Map::AnnotationItem *annotation = nullptr)
		: StationSymbol(loc.y(), loc.x())
		, validLocation(valid)
		, isActive(false), isArrival(false)
		, net(nc), code(sc), arrivalId(-1)
		, annotation(annotation) {
			setOutlineColor(SCScheme.colors.map.outlines);
			setFrameSize(0);
			setVisible(false);
		}

		bool                 validLocation{false};
		bool                 isActive{false};
		bool                 isArrival{false};
		// The state in the configuration module, only used for
		// unassociated stations
		bool                 isEnabled{true};
		bool                 isConfigured{true};
		std::string          net;
		std::string          code;
		QColor               color;
		OPT(double)          backAzimuth;
		double               distance{0.0};
		int                  arrivalId{-1};
		Map::AnnotationItem *annotation{nullptr};
	};

	QVector<Symbol*>     stations;
	UnassociatedStations unassociated;
	double               usedDistance{-1};
	bool                 interactive{true};
	bool                 drawStationsLines{true};
	bool                 drawDirectivity{true};
	int                  hoverId{-1};
};


#define SYMBOLLAYER static_cast<StationLayer*>(_symbolLayer)


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorMap::OriginLocatorMap(const MapsDesc &maps,
                                   QWidget *parent, Qt::WindowFlags f)
: MapWidget(maps, parent, f)
, _origin(nullptr) {
	_symbolLayer = new StationLayer(this);
	canvas().addLayer(_symbolLayer);
	_annotationLayer = new Map::AnnotationLayer(this, new Map::Annotations(this));
	_annotationLayer->setVisible(false);
	canvas().addLayer(_annotationLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorMap::OriginLocatorMap(Map::ImageTree* mapTree,
                                   QWidget *parent, Qt::WindowFlags f)
: MapWidget(mapTree, parent, f)
, _origin(nullptr) {
	_symbolLayer = new StationLayer(this);
	canvas().addLayer(_symbolLayer);
	_annotationLayer = new Map::AnnotationLayer(this, new Map::Annotations(this));
	_annotationLayer->setVisible(false);
	canvas().addLayer(_annotationLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::contextMenuEvent(QContextMenuEvent *e) {
	QMenu menu(this);
	QAction *actionArtificialOrigin = nullptr;
	QAction *actionDrawDirectivity = nullptr;

	if ( _enabledCreateOrigin ) {
		actionArtificialOrigin = menu.addAction("Create artificial origin");
		menu.addSeparator();
	}

	updateContextMenu(&menu);

	actionDrawDirectivity = menu.addAction(tr("%1 directivity").arg(SYMBOLLAYER->drawDirectivity ? "Hide" : "Show"));

	QMenu *unassociatedMenu = SYMBOLLAYER->unassociated.addMenu(&menu, [this]() {
		SYMBOLLAYER->updateAnnotations();
		SYMBOLLAYER->update(Gui::Map::Layer::Redraw);
	}, SYMBOLLAYER->inRangeCount());

	QAction *action = menu.exec(e->globalPos());
	if ( action == nullptr ) return;

	if ( action->parent() == unassociatedMenu ) {
		// Already handled by the menu
		return;
	}
	else if ( action == actionDrawDirectivity ) {
		SYMBOLLAYER->drawDirectivity = !SYMBOLLAYER->drawDirectivity;
		SYMBOLLAYER->update(Gui::Map::Layer::Redraw);
	}
	else if ( action == actionArtificialOrigin ) {
		QPointF epicenter;
		if ( canvas().projection()->unproject(epicenter, e->pos()) )
			emit artificialOriginRequested(epicenter, e->globalPos());
	}
	else {
		executeContextMenuAction(action);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::mouseDoubleClickEvent(QMouseEvent *event) {
	if ( (event->button() == Qt::LeftButton) && SYMBOLLAYER->isVisible() && SYMBOLLAYER->interactive ) {
		if ( event->modifiers() == Qt::NoModifier ) {
			if ( SYMBOLLAYER->hoverId != -1 ) {
				auto symbol = SYMBOLLAYER->stations[SYMBOLLAYER->hoverId];
				if ( symbol->isArrival ) {
					setArrivalState(symbol->arrivalId, !symbol->isActive);
					emit arrivalChanged(symbol->arrivalId, symbol->isActive);
				}

				return;
			}
		}
	}

	MapWidget::mouseDoubleClickEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::mousePressEvent(QMouseEvent *event) {
	if ( !isMeasuring() && !isDragging() ) {
		if ( (event->button() == Qt::LeftButton)
		   && SYMBOLLAYER->isVisible()
		   && SYMBOLLAYER->interactive ) {
			if ( event->modifiers() == Qt::NoModifier ) {
				if ( SYMBOLLAYER->hoverId != -1 ) {
					auto symbol = SYMBOLLAYER->stations[SYMBOLLAYER->hoverId];
					if ( symbol->isArrival )
						emit clickedArrival(symbol->arrivalId);
					else
						emit clickedStation(symbol->net, symbol->code);

					return;
				}
			}
		}
		else if ( event->button() == Qt::MiddleButton && _enabledCreateOrigin ) {
			QPointF epicenter;
			if ( canvas().projection()->unproject(epicenter, event->pos()) )
				emit artificialOriginRequested(epicenter, QT_EVENT_GLOBALPOS(event));
		}
	}

	MapWidget::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setCache(DataModel::PublicObjectCache *cache) {
	_cache = cache;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationsMaxDist(double d) {
	SYMBOLLAYER->unassociated.maxDistance = d;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setUnassociatedStations(const UnassociatedStations &config) {
	SYMBOLLAYER->unassociated = config;
	SYMBOLLAYER->updateAnnotations();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const UnassociatedStations &OriginLocatorMap::unassociatedStations() const {
	return SYMBOLLAYER->unassociated;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationsInteractive(bool e) {
	SYMBOLLAYER->interactive = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setOrigin(DataModel::Origin* o) {
	_origin = o;
	_originSymbol = nullptr;
	SYMBOLLAYER->clear();
	canvas().symbolCollection()->clear();
	_annotationLayer->annotations()->clear();
	_arrivals.clear();
	_stationCodes.clear();

	if ( !_origin ) return;

	TTDecorator *ttd = new TTDecorator();
	ttd->setLatitude(o->latitude());
	ttd->setLongitude(o->longitude());
	ttd->setOriginTime(o->time());
	ttd->setVisible(_waveformPropagation);

	try {
		ttd->setDepth(o->depth());
	} catch ( Core::ValueException& ) {}

	if ( o->magnitudeCount() > 0 ) {
		ttd->setPreferredMagnitudeValue(o->magnitude(0)->magnitude());
	}

	_originSymbol = new AdvancedOriginSymbol(o, ttd);

	// TTDecorator::ShowWaveformPropagation(_waveformPropagation);
	canvas().symbolCollection()->add(_originSymbol);

	for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
		bool foundStation = false;
		QColor itemColor;
		Arrival* arrival = _origin->arrival(i);
		try {
			itemColor = SCScheme.colors.arrivals.residuals.colorAt(arrival->timeResidual());
		}
		catch ( ... ) {
			itemColor = SCScheme.colors.arrivals.undefined;
		}

		Pick *p = _cache ? _cache->get<Pick>(arrival->pickID()).get() : Pick::Find(arrival->pickID());
		if ( p ) {
			/*
			try {
				switch ( p->status() ) {
					case MANUAL_PICK:
						itemColor = SCScheme.colors.arrivals.manual;
						break;
					case AUTOMATIC_PICK:
						itemColor = SCScheme.colors.arrivals.automatic;
						break;
					default:
						itemColor = SCScheme.colors.arrivals.undefined;
						break;
				}
			}
			catch ( ... ) {
				itemColor = SCScheme.colors.arrivals.undefined;
			}
			*/

			try {
				StationLocation loc = Client::Inventory::Instance()->stationLocation(
					p->waveformID().networkCode(),
					p->waveformID().stationCode(),
					p->time()
				);

				std::string stationCode = p->waveformID().networkCode() + "." + p->waveformID().stationCode();
				auto symbol = new StationLayer::Symbol(
					QPointF(loc.longitude,loc.latitude),
					p->waveformID().networkCode(),
					p->waveformID().stationCode(),
					true,
					_annotationLayer->annotations()->add(stationCode.c_str())
				);

				SYMBOLLAYER->stations.push_back(symbol);

				try {
					symbol->distance = arrival->distance();
					symbol->backAzimuth = p->backazimuth().value();
				}
				catch ( ... ) {}

				_stationCodes[stationCode] = SYMBOLLAYER->stations.size()-1;
				addArrival();
				foundStation = true;

			}
			catch ( Core::ValueException& e ) {
				SEISCOMP_DEBUG("While fetching the station location an error occured: %s -> computing position", e.what());
				foundStation = false;
			}
		}
		else {
			SEISCOMP_DEBUG("pick for arrival not found -> setting arrival color [undefined]");
			//itemColor = SCScheme.colors.arrivals.undefined;
			foundStation = false;
		}

		if ( !foundStation ) {
			try {
				double lat, lon;
				Math::Geo::delandaz2coord(arrival->distance(), arrival->azimuth(),
				                    _origin->latitude(), _origin->longitude(),
				                    &lat, &lon);

				lon = fmod(lon+180.0,360.0);
				if ( lon < 0 ) lon += 360.0;
				lon -= 180.0;

				lat = fmod(lat+90.0,180.0);
				if ( lat < 0 ) lat += 180.0;
				lat -= 90.0;

				SYMBOLLAYER->stations.push_back(
					new StationLayer::Symbol(
						QPointF(lon,lat), "", "", true
					)
				);
				SYMBOLLAYER->stations.back()->distance = arrival->distance();
				addArrival();
			}
			catch ( ... ) {
				SYMBOLLAYER->stations.push_back(
					new StationLayer::Symbol(
						QPointF(0,0), "", "", false
					)
				);
				addArrival();
			}
		}

		try {
			SYMBOLLAYER->stations.back()->isActive = arrival->weight() > 0.0;
		}
		catch ( ... ) {
			SYMBOLLAYER->stations.back()->isActive = true;
		}

		SYMBOLLAYER->stations.back()->color = itemColor;
	}

	UnassociatedStations::StationStates states;
	states.read(SYMBOLLAYER->unassociated.requiredBinding);

	DataModel::Inventory *inv = Client::Inventory::Instance()->inventory();
	if ( inv ) {
		for ( size_t n = 0; n < inv->networkCount(); ++n ) {
			DataModel::Network *net = inv->network(n);
			if ( net->start() > _origin->time().value() )
				continue;
			try { if ( net->end() < _origin->time().value() ) continue; }
			catch ( ... ) {}

			for ( size_t s = 0; s < net->stationCount(); ++s ) {
				DataModel::Station *sta = net->station(s);
				if ( sta->start() > _origin->time().value() )
					continue;
				try { if ( sta->end() < _origin->time().value() ) continue; }
				catch ( ... ) {}

				double dist, azi1, azi2;
				try {
					Math::Geo::delazi(
						sta->latitude(), sta->longitude(),
						_origin->latitude(), _origin->longitude(),
						&dist, &azi1, &azi2);
				}
				catch ( ... ) {
					continue;
				}

				if ( dist > SYMBOLLAYER->unassociated.maxDistance ) continue;

				std::string stationCode = net->code() + "." + sta->code();

				// Station already registered as arrival
				if ( _stationCodes.find(stationCode) != _stationCodes.end() )
					continue;

				auto symbol = new StationLayer::Symbol(
					QPointF(sta->longitude(),sta->latitude()),
					net->code(), sta->code(),
					true,
					_annotationLayer->annotations()->add(stationCode.c_str())
				);

				symbol->isActive = true;
				symbol->distance = dist;
				states.get(net->code(), sta->code(), symbol->isEnabled, symbol->isConfigured);

				SYMBOLLAYER->stations.push_back(symbol);
				_stationCodes[stationCode] = SYMBOLLAYER->stations.size()-1;
			}
		}
	}

	for ( auto symbol : std::as_const(SYMBOLLAYER->stations) ) {
		if ( symbol->annotation ) {
			symbol->annotation->highlighted = symbol->isActive && symbol->isArrival;
		}
	}

	SYMBOLLAYER->setDirty();
	_arrivals = SYMBOLLAYER->sort();
	for ( auto it = _stationCodes.begin(); it != _stationCodes.end(); ++it ) {
		it->second = _arrivals[it->second];
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::addArrival() {
	SYMBOLLAYER->stations.back()->arrivalId = _arrivals.size();
	SYMBOLLAYER->stations.back()->isArrival = true;
	_arrivals.push_back(SYMBOLLAYER->stations.size()-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setDrawStations(bool draw) {
	SYMBOLLAYER->setVisible(draw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setDrawStationLines(bool draw) {
	if ( SYMBOLLAYER->drawStationsLines == draw ) return;

	SYMBOLLAYER->drawStationsLines = draw;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setDrawStationAnnotations(bool draw) {
	_annotationLayer->setVisible(draw);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorMap::drawStations() const {
	return SYMBOLLAYER->isVisible();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setWaveformPropagation(bool p) {
	if ( _waveformPropagation == p ) return;
	_waveformPropagation = p;
	if ( _originSymbol && _originSymbol->decorator() ) {
		// TTDecorator::ShowWaveformPropagation(_waveformPropagation);
		_originSymbol->decorator()->setVisible(_waveformPropagation);
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorMap::waveformPropagation() const {
	return _waveformPropagation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setArrivalState(int id, bool state) {
	if ( id < 0 || id >= _arrivals.size() ) return;
	int stationId = _arrivals[id];
	setStationState(stationId, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationState(int i, bool state) {
	if ( SYMBOLLAYER->stations[i]->isActive == state ) return;
	SYMBOLLAYER->stations[i]->isActive = state;
	if ( SYMBOLLAYER->isVisible() ) {
		if ( SYMBOLLAYER->stations[i]->isVisible() ) {
			if ( SYMBOLLAYER->stations[i]->annotation ) {
				SYMBOLLAYER->stations[i]->annotation->highlighted = state;
			}
			//update(p.x()-5, p.y()-5, 10, 10);
			update();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setOriginCreationEnabled(bool enable) {
	_enabledCreateOrigin = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::addLayer(Map::Layer *layer) {
	canvas().insertLayerBefore(_annotationLayer, layer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorMap::setStationState(const std::string& code, bool state) {
	std::map<std::string, int>::iterator it = _stationCodes.find(code);
	if ( it == _stationCodes.end() ) return;
	setStationState((*it).second, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
