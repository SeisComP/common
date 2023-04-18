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



#define SEISCOMP_COMPONENT Gui::MagnitudeMap
#include "magnitudemap.h"
#include <seiscomp/client/inventory.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/stationmagnitudecontribution.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/math.h>
#include <seiscomp/math/geo.h>

#include <seiscomp/gui/datamodel/advancedoriginsymbol.h>
#include <seiscomp/gui/datamodel/stationsymbol.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/map/layers/annotationlayer.h>

#include <algorithm>


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
	StationLayer(MagnitudeMap *map)
	: Map::Layer(map) {}

	~StationLayer() override {
		clear();
	}

	void setVisible(bool v) {
		Map::Layer::setVisible(v);
		if ( !v ) {
			for ( auto entry : stations ) {
				if ( entry->annotation )
					entry->annotation->visible = false;
			}
		}
		else {
			for ( auto entry : stations ) {
				if ( entry->annotation )
					entry->annotation->visible = entry->isVisible();
			}
		}
	}

	void calculateMapPosition(const Map::Canvas *canvas) override {
		for ( auto entry : stations ) {
			if ( !entry->validLocation ) {
				entry->setVisible(false);
			}
			else {
				entry->setVisible(true);
				entry->calculateMapPosition(canvas);
			}

			if ( entry->annotation ) {
				entry->annotation->visible = entry->isVisible();
			}
		}
	}

	bool isInside(const QMouseEvent *event, const QPointF &) override {
		int tmpHoverId = -1;

		for ( int i = 0; i < stations.count(); ++i ) {
			if ( !stations[i]->isVisible() ) {
				continue;
			}

			if ( stations[i]->isInside(event->x(), event->y()) ) {
				tmpHoverId = i;
				break;
			}
		}

		if ( tmpHoverId != hoverId ) {
			hoverId = tmpHoverId;
			if ( hoverId != -1 ) {
				if ( !stations[hoverId]->net.empty()
				  && !stations[hoverId]->code.empty() ) {
					setToolTip((stations[hoverId]->net + "." + stations[hoverId]->code).c_str());
				}
				else {
					setToolTip(QString());
				}
			}
			else {
				setToolTip(QString());
			}
		}

		return hoverId != -1;
	}

	void draw(const Map::Canvas *canvas, QPainter &p) override {
		int size = SCScheme.map.stationSize;

		QPoint annotationOffset(0, -size - p.fontMetrics().height() / 4);

		if ( canvas->symbolCollection()->count() > 0 ) {
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
			for ( auto s : stations ) {
				if ( !s->validLocation || !s->isMagnitude ) {
					continue;
				}

				if ( !s->isActive ) {
					continue;
				}

				canvas->drawLine(p, originSymbol->location(), s->location());
			}

			if ( cutOff )
				p.setClipping(false);
		}

		p.setPen(SCScheme.colors.map.outlines);
		for ( int i = stations.count()-1; i >= 0; --i ) {
			if ( stations[i]->isClipped() || !stations[i]->isVisible() ) {
				continue;
			}

			if ( !interactive && !stations[i]->isActive ) {
				continue;
			}

			stations[i]->setColor(
				stations[i]->isMagnitude
				?
				stations[i]->isActive ? stations[i]->color : SCScheme.colors.magnitudes.disabled
				:
				stations[i]->isActive ? SCScheme.colors.stations.idle : Qt::gray
			);

			stations[i]->draw(canvas, p);

			if ( stations[i]->annotation ) {
				stations[i]->annotation->updateLabelRect(p, stations[i]->pos() + annotationOffset);
			}
		}
	}

	void sort() {
		std::sort(stations.begin(), stations.end(), [](Symbol *s1, Symbol *s2) {
			return s1->latitude() < s2->latitude();
		});
	}

	void clear() {
		for ( auto symbol : stations ) {
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
		, net(nc), code(sc)
		, annotation(annotation) {
			setOutlineColor(SCScheme.colors.map.outlines);
			setFrameSize(0);
			setVisible(false);
		}

		QColor               color;
		bool                 validLocation{false};
		bool                 isActive{false};
		bool                 isMagnitude{false};
		std::string          net;
		std::string          code;
		double               residual;
		int                  magnitudeId{-1};
		Map::AnnotationItem *annotation{nullptr};
	};

	QVector<Symbol*> stations;
	bool             interactive{true};
	int              hoverId{-1};
};


#define SYMBOLLAYER static_cast<StationLayer*>(_symbolLayer)


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeMap::MagnitudeMap(const MapsDesc &maps,
                           QWidget *parent, Qt::WindowFlags f)
: MapWidget(maps, parent, f) {
	_symbolLayer = new StationLayer(this);
	canvas().addLayer(_symbolLayer);
	_annotationLayer = new Map::AnnotationLayer(this, new Map::Annotations(this));
	_annotationLayer->setVisible(false);
	canvas().addLayer(_annotationLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeMap::MagnitudeMap(Map::ImageTree* mapTree,
                           QWidget *parent, Qt::WindowFlags f)
 : MapWidget(mapTree, parent, f) {
	_symbolLayer = new StationLayer(this);
	canvas().addLayer(_symbolLayer);
	_annotationLayer = new Map::AnnotationLayer(this, new Map::Annotations(this));
	_annotationLayer->setVisible(false);
	canvas().addLayer(_annotationLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeMap::~MagnitudeMap() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationsMaxDist(double maxDist) {
	_stationsMaxDist = maxDist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::mouseDoubleClickEvent(QMouseEvent* event) {
	if ( (event->button() == Qt::LeftButton) && SYMBOLLAYER->isVisible() && SYMBOLLAYER->interactive ) {
		if ( event->modifiers() == Qt::NoModifier ) {
			if ( SYMBOLLAYER->hoverId != -1 ) {
				auto symbol = SYMBOLLAYER->stations[SYMBOLLAYER->hoverId];
				if ( symbol->isMagnitude ) {
					setMagnitudeState(symbol->magnitudeId, !symbol->isActive);
					emit magnitudeChanged(symbol->magnitudeId, symbol->isActive);
				}

				return;
			}
		}
	}

	MapWidget::mouseDoubleClickEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::mousePressEvent(QMouseEvent* event) {
	if ( !isMeasuring() && !isDragging() ) {
		if ( (event->button() == Qt::LeftButton)
		   && SYMBOLLAYER->isVisible()
		   && SYMBOLLAYER->interactive ) {
			if ( event->modifiers() == Qt::NoModifier ) {
				if ( SYMBOLLAYER->hoverId != -1 ) {
					auto symbol = SYMBOLLAYER->stations[SYMBOLLAYER->hoverId];
					if ( symbol->isMagnitude )
						emit clickedMagnitude(symbol->magnitudeId);
					else
						emit clickedStation(symbol->net, symbol->code);

					return;
				}
			}
		}
	}

	MapWidget::mousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationsInteractive(bool e) {
	SYMBOLLAYER->interactive = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setMagnitude(DataModel::Magnitude* nm) {
	_magnitude = nm;
	_magnitudes.clear();

	for ( int i = 0; i < SYMBOLLAYER->stations.size(); ++i ) {
		SYMBOLLAYER->stations[i]->isActive = true;
		SYMBOLLAYER->stations[i]->isMagnitude = false;
		SYMBOLLAYER->stations[i]->magnitudeId = -1;
	}

	if ( _magnitude ) {
		for ( size_t i = 0; i < _magnitude->stationMagnitudeContributionCount(); ++i ) {
			StationMagnitude* staMag = StationMagnitude::Find(_magnitude->stationMagnitudeContribution(i)->stationMagnitudeID());
			if ( !staMag ) {
				SEISCOMP_DEBUG("StationMagnitude '%s' not found", _magnitude->stationMagnitudeContribution(i)->stationMagnitudeID().c_str());
				continue;
			}

			double residual = staMag->magnitude().value() - _magnitude->magnitude().value();
			addStationMagnitude(staMag, i);
			setMagnitudeResidual(i, residual);
		}

		for ( auto symbol : SYMBOLLAYER->stations ) {
			if ( symbol->annotation ) {
				symbol->annotation->highlighted = symbol->isActive && symbol->isMagnitude;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setOrigin(DataModel::Origin* o) {
	_origin = o;

	SYMBOLLAYER->clear();
	canvas().symbolCollection()->clear();
	_annotationLayer->annotations()->clear();
	_stationCodes.clear();

	setMagnitude(nullptr);

	if ( !_origin ) return;

	AdvancedOriginSymbol *symbol = new AdvancedOriginSymbol(o);
	try { symbol->setDepth(o->depth()); } catch ( Core::ValueException& ) {}
	canvas().symbolCollection()->add(symbol);

	for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
		bool foundStation = false;
		Arrival* arrival = _origin->arrival(i);
		Pick* p = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( p ) {
			try {
				StationLocation loc = Client::Inventory::Instance()->stationLocation(
					p->waveformID().networkCode(),
					p->waveformID().stationCode(),
					p->time()
				);

				std::string stationCode = p->waveformID().networkCode() + "." + p->waveformID().stationCode();
				if ( _stationCodes.find(stationCode) != _stationCodes.end() ) {
					foundStation = true;
					continue;
				}

				SYMBOLLAYER->stations.push_back(
					new StationLayer::Symbol(
						QPointF(loc.longitude,loc.latitude),
						p->waveformID().networkCode(),
						p->waveformID().stationCode(),
						true,
						_annotationLayer->annotations()->add(stationCode.c_str())
					)
				);
				foundStation = true;
			}
			catch ( Core::ValueException& e ) {
				SEISCOMP_DEBUG("While fetching the station location an error occured: %s -> computing position", e.what());
				foundStation = false;
			}
		}
		else {
			SEISCOMP_DEBUG("pick for arrival not found -> setting arrival color [undefined]");
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
					new StationLayer::Symbol(QPointF(lon,lat), "", "", true)
				);
			}
			catch ( ... ) {
				SYMBOLLAYER->stations.push_back(
					new StationLayer::Symbol(QPointF(0,0), "", "", false)
				);
			}
		}
	}

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

				// Limit to 20 degrees
				if ( dist > _stationsMaxDist ) continue;

				std::string stationCode = net->code() + "." + sta->code();

				// Station already registered as arrival
				if ( _stationCodes.find(stationCode) != _stationCodes.end() )
					continue;

				SYMBOLLAYER->stations.push_back(
					new StationLayer::Symbol(
						QPointF(sta->longitude(),sta->latitude()),
						net->code(), sta->code().c_str(),
						true,
						_annotationLayer->annotations()->add(stationCode.c_str())
					)
				);

				SYMBOLLAYER->stations.back()->isActive = true;
			}
		}
	}

	SYMBOLLAYER->sort();

	for ( int i = 0; i < SYMBOLLAYER->stations.size(); ++i ) {
		auto sym = SYMBOLLAYER->stations[i];
		_stationCodes[sym->net + "." + sym->code] = i;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::addStationMagnitude(StationMagnitude* staMag, int index) {
	if ( index < _magnitudes.size() ) return;

	try {
		std::string stationCode = staMag->waveformID().networkCode() + "." + staMag->waveformID().stationCode();
		int stationId = findStation(stationCode);
		if ( stationId == -1 ) {
			stationId = addStation(staMag->waveformID().networkCode(), staMag->waveformID().stationCode());
		}

		if ( stationId != -1 ) {
			addMagnitude(stationId, index);
		}
	}
	catch ( ... ) {
		SEISCOMP_DEBUG("WaveformID in magnitude '%s' not set", staMag->publicID().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setDrawStations(bool f) {
	SYMBOLLAYER->setVisible(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setDrawStationAnnotations(bool f) {
	_annotationLayer->setVisible(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MagnitudeMap::findStation(const std::string& stationCode) const {
	std::map<std::string, int>::const_iterator it;
	it = _stationCodes.find(stationCode);
	if ( it != _stationCodes.end() )
		return it->second;
	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MagnitudeMap::addStation(const std::string &netCode, const std::string &staCode) {
	if ( !_origin ) return -1;
	// Add station
	DataModel::Inventory *inv = Client::Inventory::Instance()->inventory();
	if ( !inv ) return -1;

	for ( size_t n = 0; n < inv->networkCount(); ++n ) {
		DataModel::Network *net = inv->network(n);
		if ( net->code() != netCode ) continue;
		if ( net->start() > _origin->time().value() )
			continue;
		try { if ( net->end() < _origin->time().value() ) continue; }
		catch ( ... ) {}

		for ( size_t s = 0; s < net->stationCount(); ++s ) {
			DataModel::Station *sta = net->station(s);
			if ( sta->code() != staCode ) continue;
			if ( sta->start() > _origin->time().value() )
				continue;
			try { if ( sta->end() < _origin->time().value() ) continue; }
			catch ( ... ) {}

			std::string stationCode = net->code() + "." + sta->code();

			SYMBOLLAYER->stations.push_back(
				new StationLayer::Symbol(
					QPointF(sta->longitude(),sta->latitude()),
					net->code(), sta->code().c_str(), true
				)
			);

			int stationId = SYMBOLLAYER->stations.size()-1;
			SYMBOLLAYER->stations.back()->isActive = true;
			_stationCodes[stationCode] = stationId;
			return stationId;
		}
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::addMagnitude(int stationId, int magId) {
	SYMBOLLAYER->stations[stationId]->magnitudeId = magId;
	SYMBOLLAYER->stations[stationId]->isMagnitude = true;

	if ( magId >= _magnitudes.size() )
		_magnitudes.resize(magId+1);

	_magnitudes[magId] = stationId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setMagnitudeState(int id, bool state) {
	if ( id < 0 || id >= _magnitudes.size() ) return;
	int stationId = _magnitudes[id];
	setStationState(stationId, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setMagnitudeResidual(int id, double residual) {
	if ( id < 0 || id >= _magnitudes.size() ) return;
	int stationId = _magnitudes[id];
	setStationResidual(stationId, residual);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationState(int i, bool state) {
	if ( SYMBOLLAYER->stations[i]->isActive == state ) return;
	SYMBOLLAYER->stations[i]->isActive = state;
	if ( SYMBOLLAYER->stations[i]->isVisible() ) {
		if ( SYMBOLLAYER->stations[i]->annotation ) {
			SYMBOLLAYER->stations[i]->annotation->highlighted = state;
		}
		//update(p.x()-5, p.y()-5, 10, 10);
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationState(const std::string &code, bool state) {
	std::map<std::string, int>::iterator it = _stationCodes.find(code);
	if ( it == _stationCodes.end() ) return;
	setStationState((*it).second, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeMap::setStationResidual(int i, double residual) {
	SYMBOLLAYER->stations[i]->residual = residual;
	SYMBOLLAYER->stations[i]->color = SCScheme.colors.magnitudes.residuals.colorAt(residual);

	if ( SYMBOLLAYER->isVisible() )
		update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
