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

#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/map/layers/geofeaturelayer.h>
#include <seiscomp/gui/map/canvas.h>
#include <seiscomp/gui/map/standardlegend.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/seismology/regions.h>

#include <QMenu>

#include <functional>
#include <iostream>

using namespace std;

namespace Seiscomp {
namespace Gui {
namespace Map {

namespace {


#define CFG_LAYER_PREFIX "map.layers"

QPen readPen(const Config::Config &cfg, const string &query, const QPen &base) {
	QPen p(base);

	try {
		const string &q = query + ".color";
		p.setColor(readColor(q, cfg.getString(q), base.color()));
	}
	catch ( ... ) {}

	try {
		const string &q = query + ".style";
		p.setStyle(readPenStyle(q, cfg.getString(q), base.style()));
	}
	catch ( ... ) {}

	try {
		p.setWidth(cfg.getDouble(query + ".width"));
	}
	catch ( ... ) {}

	return p;
}

QBrush readBrush(const Config::Config &cfg, const string &query, const QBrush &base) {
	QBrush b(base);

	try {
		const string &q = query + ".color";
		b.setColor(readColor(q, cfg.getString(q), base.color()));
	}
	catch ( ... ) {}

	try {
		const string &q = query + ".style";
		b.setStyle(readBrushStyle(q, cfg.getString(q), base.style()));
	}
	catch ( ... ) {}

	return b;
}

QFont readFont(const Config::Config &cfg, const string& query, const QFont &base) {
	QFont f(base);

	try {
		f.setFamily(cfg.getString(query + ".family").c_str());
	}
	catch ( ... ) {}

	try {
		f.setPointSize(cfg.getInt(query + ".size"));
	}
	catch ( ... ) {}

	try {
		f.setBold(cfg.getBool(query + ".bold"));
	}
	catch ( ... ) {}

	try {
		f.setItalic(cfg.getBool(query + ".italic"));
	}
	catch ( ... ) {}

	try {
		f.setUnderline(cfg.getBool(query + ".underline"));
	}
	catch ( ... ) {}

	try {
		f.setOverline(cfg.getBool(query + ".overline"));
	}
	catch ( ... ) {}

	return f;
}

Qt::Orientation getOrientation(const std::string &name) {
	if ( name == "horizontal" ) {
		return Qt::Horizontal;
	}

	// "vertical"
	return Qt::Vertical;
}

Qt::Alignment getAlignment(const std::string &name,
                           Qt::Alignment fallback={Qt::AlignTop | Qt::AlignLeft}) {

	return name == "topleft" ? Qt::Alignment(Qt::AlignTop | Qt::AlignLeft) :
	       name == "topcenter" ? Qt::Alignment(Qt::AlignTop | Qt::AlignHCenter) :
	       name == "topright"  ? Qt::Alignment(Qt::AlignTop | Qt::AlignRight) :
	       name == "centerleft" ? Qt::Alignment(Qt::AlignVCenter | Qt::AlignLeft) :
	       name == "center" ? Qt::Alignment(Qt::AlignVCenter | Qt::AlignHCenter) :
	       name == "centerright" ? Qt::Alignment(Qt::AlignVCenter | Qt::AlignRight) :
	       name == "bottomleft" ? Qt::Alignment(Qt::AlignBottom | Qt::AlignLeft) :
	       name == "bottomcenter" ? Qt::Alignment(Qt::AlignBottom | Qt::AlignHCenter) :
	       name == "bottomright" ? Qt::Alignment(Qt::AlignBottom | Qt::AlignRight) :
	       fallback;
}


QPainter::CompositionMode getCompositionMode(const std::string &name) {
	static const map<string, QPainter::CompositionMode> modeMap = {
	    {"src-over", QPainter::CompositionMode_SourceOver},
	    {"dst-over", QPainter::CompositionMode_DestinationOver},
	    {"clear", QPainter::CompositionMode_Clear},
	    {"src", QPainter::CompositionMode_Source},
	    {"dst", QPainter::CompositionMode_Destination},
	    {"src-in", QPainter::CompositionMode_SourceIn},
	    {"dst-in", QPainter::CompositionMode_DestinationIn},
	    {"src-out", QPainter::CompositionMode_SourceOut},
	    {"dst-out", QPainter::CompositionMode_DestinationOut},
	    {"src-atop", QPainter::CompositionMode_SourceAtop},
	    {"dst-atop", QPainter::CompositionMode_DestinationAtop},
	    {"xor", QPainter::CompositionMode_Xor},
	    {"plus", QPainter::CompositionMode_Plus},
	    {"multiply", QPainter::CompositionMode_Multiply},
	    {"screen", QPainter::CompositionMode_Screen},
	    {"overlay", QPainter::CompositionMode_Overlay},
	    {"darken", QPainter::CompositionMode_Darken},
	    {"lighten", QPainter::CompositionMode_Lighten},
	    {"color-dodge", QPainter::CompositionMode_ColorDodge},
	    {"color-burn", QPainter::CompositionMode_ColorBurn},
	    {"hard-light", QPainter::CompositionMode_HardLight},
	    {"soft-light", QPainter::CompositionMode_SoftLight},
	    {"difference", QPainter::CompositionMode_Difference},
	    {"exclusion", QPainter::CompositionMode_Exclusion},
	    {"src-or-dst", QPainter::RasterOp_SourceOrDestination},
	    {"src-and-dst", QPainter::RasterOp_SourceAndDestination},
	    {"src-xor-dst", QPainter::RasterOp_SourceXorDestination},
	    {"not-src-and-not-dst", QPainter::RasterOp_NotSourceAndNotDestination},
	    {"not-src-or-not-dst", QPainter::RasterOp_NotSourceOrNotDestination},
	    {"not-src-xor-dst", QPainter::RasterOp_NotSourceXorDestination},
	    {"not-src", QPainter::RasterOp_NotSource},
	    {"not-src-and-dst", QPainter::RasterOp_NotSourceAndDestination},
	    {"src-and-not-dst", QPainter::RasterOp_SourceAndNotDestination}
	};

	const auto res = modeMap.find(name);
	if ( res == modeMap.end() ) {
		return QPainter::CompositionMode_SourceOver;
	}

	return res->second;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::LayerProperties::isChild(const LayerProperties* child) const {
	while ( child ) {
		if ( child == this ) {
			return true;
		}

		child = child->parent;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::LayerProperties::SymbolShape
GeoFeatureLayer::LayerProperties::getSymbolShape(const std::string &type) {
	if ( type == "none" ) {
		return None;
	}

	if ( type == "circle" ) {
		return Circle;
	}

	if ( type == "triangle" ) {
		return Triangle;
	}

	if ( type == "square" ) {
		return Square;
	}

	if ( type == "diamond" ) {
		return Diamond;
	}

	return Disabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::LayerProperties::read(const string &dataDir) {
	const static string cfgVisible = "visible";
	const static string cfgPen = "pen";
	const static string cfgBrush = "brush";
	const static string cfgFont = "font";
	const static string cfgDrawName = "drawName";
	const static string cfgDebug = "debug";
	const static string cfgRank = "rank";
	const static string cfgRoughness = "roughness";
	const static string cfgSymbolSize = "symbol.size";
	const static string cfgSymbolShape = "symbol.shape";
	const static string cfgSymbolIcon = "symbol.icon";
	const static string cfgSymbolIconX = "symbol.icon.hotspot.x";
	const static string cfgSymbolIconY = "symbol.icon.hotspot.y";
	const static string cfgSymbolNameAlign = "symbol.name.alignment";
	const static string cfgSymbolNameMargin = "symbol.name.margin";
	const static string cfgSymbolNamePen = "symbol.name.pen";
	const static string cfgTitle = "title";
	const static string cfgLabel = "label";
	const static string cfgIndex = "index";
	const static string cfgLegendArea = "legendArea";
	const static string cfgLegendOrientation = "orientation";
	const static string cfgCompositionMode = "composition";

	string symbolIconPath;

	// Read additional configuration file (e.g. map.cfg in spatial vector folder)
	if ( !dataDir.empty() ) {
		Config::Config cfg;
		if ( cfg.readConfig(dataDir + "/map.cfg", -1, true) ) {
			try { visible = cfg.getBool(cfgVisible); } catch( ... ) {}
			pen = readPen(cfg, cfgPen, pen);
			brush = readBrush(cfg, cfgBrush, brush);
			font = readFont(cfg, cfgFont, font);
			try { drawName = cfg.getBool(cfgDrawName); } catch( ... ) {}
			try { debug = cfg.getBool(cfgDebug); } catch( ... ) {}
			try { rank = cfg.getInt(cfgRank); } catch( ... ) {}
			try { roughness = cfg.getInt(cfgRoughness); } catch( ... ) {}
			try { symbolSize = cfg.getInt(cfgSymbolSize); } catch( ... ) {}
			try { symbolShape = getSymbolShape(cfg.getString(cfgSymbolShape)); } catch( ... ) {}
			try { symbolIconPath = cfg.getString(cfgSymbolIcon); } catch( ... ) {}
			try { symbolIconHotspot.setX(cfg.getInt(cfgSymbolIconX)); } catch( ... ) {}
			try { symbolIconHotspot.setY(cfg.getInt(cfgSymbolIconY)); } catch( ... ) {}
			try { symbolNameAlignment = getAlignment(cfg.getString(cfgSymbolNameAlign), symbolNameAlignment); } catch( ... ) {}
			try { symbolNameMargin = cfg.getInt(cfgSymbolNameMargin); } catch( ... ) {}
			try { title = cfg.getString(cfgTitle); } catch( ... ) {}
			try { label = cfg.getString(cfgLabel); } catch( ... ) {}
			try { index = cfg.getInt(cfgIndex); } catch( ... ) {}
			try { orientation = getOrientation(cfg.getString(cfgLegendOrientation)); } catch( ... ) {}
			try { legendArea = getAlignment(cfg.getString(cfgLegendArea)); } catch( ... ) {}
			try { compositionMode = getCompositionMode(cfg.getString(cfgCompositionMode)); } catch( ... ) {}
		}
	}

	// Query properties from config
	string query = CFG_LAYER_PREFIX ".";
	if ( !name.empty() ) {
		query += name + ".";
	}

	if ( SCApp ) {
		try { visible = SCApp->configGetBool(query + cfgVisible); } catch( ... ) {}
		pen = SCApp->configGetPen(query + cfgPen, pen);
		brush = SCApp->configGetBrush(query + cfgBrush, brush);
		font = SCApp->configGetFont(query + cfgFont, font);
		try { drawName = SCApp->configGetBool(query + cfgDrawName); } catch( ... ) {}
		try { debug = SCApp->configGetBool(query + cfgDebug); } catch( ... ) {}
		try { rank = SCApp->configGetInt(query + cfgRank); } catch( ... ) {}
		try { roughness = SCApp->configGetInt(query + cfgRoughness); } catch( ... ) {}
		try { symbolSize = SCApp->configGetInt(query + cfgSymbolSize); } catch( ... ) {}
		try { symbolShape = getSymbolShape(SCApp->configGetString(query + cfgSymbolShape)); } catch( ... ) {}
		try { symbolIconPath = SCApp->configGetString(cfgSymbolIcon); } catch( ... ) {}
		try { symbolIconHotspot.setX(SCApp->configGetInt(query + cfgSymbolIconX)); } catch( ... ) {}
		try { symbolIconHotspot.setY(SCApp->configGetInt(query + cfgSymbolIconY)); } catch( ... ) {}
		try { symbolNameAlignment = getAlignment(SCApp->configGetString(query + cfgSymbolNameAlign), symbolNameAlignment); } catch( ... ) {}
		try { symbolNameMargin = SCApp->configGetInt(query + cfgSymbolNameMargin); } catch( ... ) {}
		try { title = SCApp->configGetString(query + cfgTitle); } catch( ... ) {}
		try { label = SCApp->configGetString(query + cfgLabel); } catch( ... ) {}
		try { index = SCApp->configGetInt(query + cfgIndex); } catch( ... ) {}
		try { orientation = getOrientation(SCApp->configGetString(query + cfgLegendOrientation)); } catch( ... ) {}
		try { legendArea = getAlignment(SCApp->configGetString(query + cfgLegendArea)); } catch( ... ) {}
		try { compositionMode = getCompositionMode(SCApp->configGetString(cfgCompositionMode)); } catch( ... ) {}
	}

	filled = brush.style() != Qt::NoBrush;

	// read and scale symbol icon
	if ( !symbolIconPath.empty() ) {
		if ( symbolIconPath[0] == '/' ) {
			symbolIcon = QImage(symbolIconPath.c_str());
		}
		else {
			symbolIcon = QImage((dataDir + '/' + symbolIconPath).c_str());
		}

		// symbol could not be loaded: fall back to empty symbol shape
		if ( symbolIcon.isNull() ) {
			symbolShape = None;
		}
		// scale icon and hotspot
		else if ( symbolSize > 0 ) {
			QSize oldSize = symbolIcon.size();
			symbolIcon = symbolIcon.scaled(symbolSize, symbolSize,
			                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QPoint scaledHotspot(symbolIconHotspot.x() * symbolIcon.size().width() / oldSize.width(),
			                     symbolIconHotspot.y() * symbolIcon.size().height() / oldSize.height());
			symbolRect = symbolIcon.rect();
			symbolRect.moveTo(-scaledHotspot);
			return;
		}
		// use original icon and hotspot dimension
		else {
			symbolRect = symbolIcon.rect();
			symbolRect.moveTo(-symbolIconHotspot);
			return;
		}
	}

	if ( symbolShape == Disabled ) {
		symbolRect = {};
		return;
	}

	int size = max(0, symbolSize);
	if ( !size ) {
		symbolShape = None;
	}

	switch ( symbolShape ) {
		case  Triangle:
			// create equilateral triangle shape with edge length of symbolSize
			// and coordinate set to balance point of triangle
			symbolPolygon << QPoint(0, -0.577*size) // top, y: inner radius - height
			              << QPoint(size*0.5, int(0.289*size)) // right, y: inner radius
			              << QPoint(-size*0.5, int(0.289*size)); // left)
			symbolRect = symbolPolygon.boundingRect();
			break;
		case Diamond:
			// create square rotated by 45 degrees
			symbolPolygon << QPoint(0, -size*0.5)
			              << QPoint(size*0.5, 0)
			              << QPoint(0, size*0.5)
			              << QPoint(-size*0.5, 0);
			symbolRect = symbolPolygon.boundingRect();
			break;

		case Circle:
		case Square:
		case None:
			symbolRect = {0, 0, size, size};
			symbolRect.moveCenter({0, 0});
			break;

		default:
			symbolRect = {};
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode::CategoryNode(const Geo::Category *c)
: category(c) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode::~CategoryNode() {
	if ( properties ) {
		delete properties;
		properties = nullptr;
	}

	for ( const auto &child : childs ) {
		delete child;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode *
GeoFeatureLayer::CategoryNode::nodeForCategory(const Geo::Category *cat) {
	if ( category == cat ) {
		return this;
	}

	for ( const auto &child : childs ) {
		auto *node = child->nodeForCategory(cat);
		if ( node ) {
			return node;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode *
GeoFeatureLayer::CategoryNode::nodeForProperties(const LayerProperties *props) {
	if ( properties == props ) {
		return this;
	}

	for ( const auto &child : childs ) {
		auto *node = child->nodeForProperties(props);
		if ( node ) {
			return node;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::GeoFeatureLayer(QObject *parent)
: Layer(parent)
, _initialized(false)
, _root(nullptr) {
	setName("features");
	Geo::GeoFeatureSetSingleton::getInstance().registerObserver(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::~GeoFeatureLayer() {
	if ( _root ) {
		delete _root;
		_root = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geo::GeoFeature *GeoFeatureLayer::findFeature(const Geo::GeoCoordinate &coord) const {
	if ( !isVisible() ) {
		return nullptr;
	}

	if ( !canvas() ) {
		// No canvas, no rank clipping possible
		return nullptr;
	}

	if ( !_root ) {
		return nullptr;
	}

	return findFeature(_root, coord);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geo::GeoFeature *GeoFeatureLayer::findFeature(qreal lat, qreal lon) const {
	return findFeature(Geo::GeoCoordinate(lat, lon));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::renderFeatures(Canvas *canvas, QPainter &painter) {
	if ( !_initialized ) {
		_initialized = true;
		initLayerProperites();
	}

	if ( !_root ) {
		return;
	}

	// Debug pen and label point
	QPen debugPen;
	debugPen.setColor(Qt::black);
	debugPen.setWidth(1);
	debugPen.setStyle(Qt::SolidLine);

	painter.setRenderHint(QPainter::Antialiasing,
	                      !canvas->previewMode() && SCScheme.map.vectorLayerAntiAlias);

	drawFeatures(_root, canvas, painter, debugPen);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::setVisible(bool flag) {
	if ( flag == isVisible() ) {
		return;
	}

	Layer::setVisible(flag);
	emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::bufferUpdated(Canvas *canvas, QPainter &painter) {
	painter.save();
	renderFeatures(canvas, painter);
	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::drawFeatures(CategoryNode *node, Canvas *canvas,
                                   QPainter &painter, const QPen &debugPen) {
	auto *layProp = node->properties;
	if ( !layProp->visible ) {
		return;
	}

	auto *proj = canvas->projection();

	if ( proj->isClipped(node->bbox) ) {
		//std::cerr << "Clipped node '" << node->properties->name << "'" << std::endl;
		return;
	}

	painter.setFont(layProp->font);
	painter.setPen(layProp->pen);
	if ( layProp->filled ) {
		painter.setBrush(layProp->brush);
	}
	else {
		painter.setBrush(Qt::NoBrush);
	}

	node->quadtree.query(proj->boundingBox(),
	                     std::bind(&GeoFeatureLayer::drawFeature, this, canvas,
	                               &painter, &debugPen, layProp,
	                               std::placeholders::_1),
	                     true);
	/*
	for ( size_t i = 0; i < node->features.size(); ++i ) {
		Geo::GeoFeature *f = node->features[i];
		drawFeature(canvas, painter, debugPen, layProp, f);
	}
	*/

	for ( const auto &child : node->childs ) {
		drawFeatures(child, canvas, painter, debugPen);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::drawFeature(Canvas *canvas, QPainter *painter,
                                  const QPen *debugPen,
                                  const LayerProperties *props,
                                  const Geo::GeoFeature *f) {
	// Skip feature if it contains no vertices
	if ( f->vertices().empty() ) {
		return true;
	}

	// Skip feature if its rank exceeds the current zoom level. The rank is
	// obtained from the associated LayerPropeties or alternatively from the
	// GeoFeature itself.
	int rank = props->rank < 0?f->rank():props->rank;
	if ( rank > canvas->zoomLevel() ) {
		return true;
	}

	// Skip feature if its bounding box is clipped
	const Geo::GeoBoundingBox &bbox = f->bbox();
	Projection *proj = canvas->projection();
	if ( proj->isClipped(QPointF(bbox.east, bbox.south),
	                     QPointF(bbox.west, bbox.north)) ) {
		return true;
	}

	if ( props->compositionMode != painter->compositionMode() ) {
		painter->setCompositionMode(props->compositionMode);
	}

	QString name;
	QRect textRect;

	// Symbol
	if ( !props->symbolIcon.isNull() ||
	     props->symbolShape != LayerProperties::Disabled ) {
		QPoint p;

		if ( props->drawName ) {
			name = f->name().c_str();
			auto tmp = painter->fontMetrics().boundingRect(name);
			auto margin2 = props->symbolNameMargin * 2;
			textRect = { 0, 0,
			             props->symbolRect.width() + margin2 + 2*tmp.width(),
			             props->symbolRect.height() + margin2 + 2*tmp.height() };
			textRect.moveCenter(props->symbolRect.center());
		}

		// Icon
		if ( !props->symbolIcon.isNull() ) {
			if ( name.isEmpty() ) {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawImage(props->symbolRect.translated(p),
						                   props->symbolIcon);
					}
				}
			}
			else {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawImage(props->symbolRect.translated(p),
						                   props->symbolIcon);
						painter->drawText(textRect.translated(p),
						                  props->symbolNameAlignment, name);
					}
				}
			}
		}
		// Shape, preprocessed into Polygon
		else if ( !props->symbolPolygon.isEmpty() ) {
			if ( name.isEmpty() ) {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawPolygon(props->symbolPolygon.translated(p));
					}
				}
			}
			else {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawPolygon(props->symbolPolygon.translated(p));
						painter->drawText(textRect.translated(p),
						                  props->symbolNameAlignment, name);
					}
				}
			}
		}
		// Circle
		else if ( props->symbolShape == LayerProperties::Circle ) {
			if ( name.isEmpty() ) {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawEllipse(props->symbolRect.translated(p));
					}
				}
			}
			else {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawEllipse(props->symbolRect.translated(p));
						painter->drawText(textRect.translated(p),
						                  props->symbolNameAlignment, name);
					}
				}
			}
		}
		// Square
		else if ( props->symbolShape == LayerProperties::Square ) {
			if ( name.isEmpty() ) {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawRect(props->symbolRect.translated(p));
					}
				}
			}
			else {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawRect(props->symbolRect.translated(p));
						painter->drawText(textRect.translated(p),
										  props->symbolNameAlignment, name);
					}
				}
			}
		}
		// LayerProperties::None
		else {
			if ( !name.isEmpty() ) {
				for ( const auto &v : f->vertices() ) {
					if ( proj->project(p, QPointF(v.lon, v.lat)) ) {
						painter->drawText(textRect.translated(p),
						                  props->symbolNameAlignment, name);
					}
				}
			}
		}
	}
	// Points, polylines and polygons
	else {
		canvas->drawFeature(*painter, f, props->filled, props->roughness);

		// Draw the name if requested and if there is enough space
		if ( props->drawName ) {
			// project the center of the bounding box
			Geo::GeoCoordinate center = bbox.center();
			QPoint c;
			if ( proj->project(c, QPointF(center.lon, center.lat)) ) {
				QString name = f->name().c_str();
				QRect textRect = painter->fontMetrics().boundingRect(name);
				float textGeoWidth = textRect.width() / proj->pixelPerDegree();
				float maxBBoxEdge = max(bbox.width(), bbox.height());
				if ( textGeoWidth < maxBBoxEdge * 0.8 ) {
					textRect.moveTo(c.x() - textRect.width()/2,
					                c.y() - textRect.height()/2);
					painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, name);
				}
			}
		}
	}

	// Debug: Print the segment name and draw the bounding box
	if ( props->debug ) {
		QPoint debugPoint;
		painter->setPen(*debugPen);
		// project the center of the bounding box
		float bboxWidth = bbox.width();
		float bboxHeight = bbox.height();
		Geo::GeoCoordinate center = bbox.center();

		if ( proj->project(debugPoint, QPointF(center.lon, center.lat)) ) {
			QFont font;
			float maxBBoxEdge = bboxWidth > bboxHeight ? bboxWidth : bboxHeight;
			int pixelSize = (int)(proj->pixelPerDegree() * maxBBoxEdge / 10.0);
			font.setPixelSize(pixelSize < 1 ? 1 : pixelSize > 30 ? 30 : pixelSize);
			QFontMetrics metrics(font);
			QRect labelRect(metrics.boundingRect(f->name().c_str()));
			labelRect.moveTo(debugPoint.x() - labelRect.width()/2,
			                 debugPoint.y() - labelRect.height()/2);

			painter->setFont(font);
			painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignTop,
			                  f->name().c_str());
		}

		proj->moveTo(QPointF(bbox.west, bbox.south));
		proj->lineTo(*painter, QPointF(bbox.east, bbox.south));
		proj->lineTo(*painter, QPointF(bbox.east, bbox.north));
		proj->lineTo(*painter, QPointF(bbox.west, bbox.north));
		proj->lineTo(*painter, QPointF(bbox.west, bbox.south));

		painter->setPen(props->pen);
		painter->setFont(props->font);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMenu *GeoFeatureLayer::menu(QMenu *parentMenu) const {
	QMenu *menu = buildMenu(_root, parentMenu);

	auto *reloadAction = new QAction(tr("Reload features"), menu);
	connect(reloadAction, SIGNAL(triggered()), this, SLOT(reloadFeatures()));

	if ( menu->isEmpty() ) {
		menu->addAction(reloadAction);
	}
	else {
		menu->insertAction(menu->actions().first(), reloadAction);
	}

	return menu;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::geoFeatureSetUpdated() {
	_initialized = false;

	if ( _root != nullptr ) {
		delete _root;
		_root = nullptr;
	}

	emit updateRequested(RasterLayer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::showFeatures() {
	auto *action = static_cast<QAction*>(sender());
	void *nodePtr = action->data().value<void*>();
	auto *node = reinterpret_cast<CategoryNode*>(nodePtr);

	bool wantUpdate = false;
	for ( const auto &child : node->childs ) {
		if ( toggleVisibility(child, true) ) {
			wantUpdate = true;
		}
	}

	if ( wantUpdate ) {
		emit updateRequested(RasterLayer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::hideFeatures() {
	auto *action = static_cast<QAction*>(sender());
	void *nodePtr = action->data().value<void*>();
	auto *node = reinterpret_cast<CategoryNode*>(nodePtr);

	bool wantUpdate = false;
	for ( const auto &child : node->childs ) {
		if ( toggleVisibility(child, false) ) {
			wantUpdate = true;
		}
	}

	if ( wantUpdate ) {
		emit updateRequested(RasterLayer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::reloadFeatures() {
	Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();
	featureSet.load();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureLayer::CategoryNode *
GeoFeatureLayer::createOrGetNodeForCategory(const Geo::Category *cat) {
	if ( !_root ) {
		_root = new CategoryNode(nullptr);
		_root->properties = new LayerProperties("");
		_root->properties->read();
	}

	if ( !cat) {
		return _root;
	}

	auto *node = _root->nodeForCategory(cat);
	if ( node ) {
		return node;
	}

	// Create parent chain
	auto *parentNode = createOrGetNodeForCategory(cat->parent);
	node = new CategoryNode(cat);

	node->properties = new LayerProperties(cat->name.empty() ? "local" : cat->name.c_str(), parentNode->properties);
	node->properties->read(cat->dataDir);

	parentNode->childs.push_back(node);
	parentNode->childsByName.push_back(node);

	return node;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::buildLegends(CategoryNode *node) {
	if ( !node ) {
		return;
	}

	auto *prop = node->properties;
	if ( !prop->title.empty() ) {
		auto *legend = new StandardLegend(this);
		legend->setTitle(prop->title.c_str());
		legend->setArea(prop->legendArea);
		legend->setOrientation(prop->orientation);

		QVector<LayerProperties*> items;
		// Find all child labels
		collectLegendItems(node, items);

		sort(items.begin(), items.end(), compareByIndex);

		for ( const auto &item : items ) {
			if ( item->filled ) {
				legend->addItem(new StandardLegendItem(item->pen,
				                                       item->brush,
				                                       item->label.c_str()));
			}
			else {
				legend->addItem(new StandardLegendItem(item->pen,
				                                       item->label.c_str()));
			}
		}

		addLegend(legend);
	}

	for ( const auto &child : node->childs ) {
		buildLegends(child);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMenu *GeoFeatureLayer::buildMenu(CategoryNode *node, QMenu *parentMenu) const {
	auto *menu = new QMenu(parentMenu);
	if ( !node ) {
		return menu;
	}
	size_t visibleCount = 0;
	for ( const auto &child : node->childsByName ) {
		auto *childProps = child->properties;
		std::string title = childProps->name;
		if ( child->category && !child->category->localName.empty() ) {
			title = child->category->localName;
		}

		if ( !childProps->visible || child->childs.empty() ) {
			auto *action = menu->addAction(title.c_str());
			action->setCheckable(true);
			action->setChecked(childProps->visible);
			if ( childProps->visible ) {
				++visibleCount;
			}
			action->setData(QVariant::fromValue<void*>(childProps));
			connect(action, SIGNAL(toggled(bool)),
			        this, SLOT(toggleFeatureVisibility(bool)));
		}
		else {
			auto *subMenu = buildMenu(child, menu);
			subMenu->setTitle(title.c_str());
			menu->addMenu(subMenu);
		}
	}

	// Add "Select all" and "Select none" options if more than 1 property
	// is available
	auto *firstPropertyAction = menu->actions().first();

	if ( (node != _root) && !node->childs.empty() ) {
		// Toggle layer
		auto *toggleAction = new QAction(tr("Hide layer"), menu);
		toggleAction->setData(QVariant::fromValue<void*>(node->properties));
		connect(toggleAction, SIGNAL(triggered()), this, SLOT(disableFeatureVisibility()));
		menu->insertAction(firstPropertyAction, toggleAction);

		// Separator
		menu->insertSeparator(firstPropertyAction);
	}

	if ( node->childs.size() >= 2 ) {
		// Select all
		auto *allAction = new QAction(tr("Show all sublayers"), menu);
		allAction->setEnabled(visibleCount < node->childs.size());
		allAction->setData(QVariant::fromValue<void*>(node));
		connect(allAction, SIGNAL(triggered()), this, SLOT(showFeatures()));
		menu->insertAction(firstPropertyAction, allAction);

		// Select none
		auto *noneAction = new QAction(tr("Hide all sublayers"), menu);
		noneAction->setEnabled(visibleCount > 0);
		noneAction->setData(QVariant::fromValue<void*>(node));
		connect(noneAction, SIGNAL(triggered()), this, SLOT(hideFeatures()));
		menu->insertAction(firstPropertyAction, noneAction);

		// Separator
		menu->insertSeparator(firstPropertyAction);
	}

	return menu;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::collectLegendItems(GeoFeatureLayer::CategoryNode *node,
                                         QVector<LayerProperties*> &items) {
	if ( !node->properties->label.empty() ) {
		items.push_back(node->properties);
	}

	for ( const auto &child : node->childs ) {
		collectLegendItems(child, items);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::orderTree(CategoryNode *node) {
	//sort(node->features.begin(), node->features.end(), compareByRank);
	sort(node->childs.begin(), node->childs.end(), compareNodeByIndex);
	sort(node->childsByName.begin(), node->childsByName.end(), compareNodeByName);

	for ( const auto &child : node->childs ) {
		orderTree(child);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::updateBbox(CategoryNode *node) {
	for ( const auto &child : node->childs ) {
		updateBbox(child);
	}

	// Do calculation
	/*
	bool noFeatures = node->features.empty();
	for ( size_t i = 0; i < node->features.size(); ++i ) {
		const Geo::GeoBoundingBox &bbox = node->features[i]->bbox();
		if ( !i )
			node->bbox = bbox;
		else
			node->bbox += bbox;
	}
	*/
	bool noFeatures = false;
	node->bbox = node->quadtree.bbox();

	for ( const auto &child : node->childs ) {
		if ( noFeatures) {
			node->bbox = child->bbox;
			noFeatures = false;
		}
		else {
			node->bbox += child->bbox;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::initLayerProperites() {
	// remove legends
	while ( legendCount() ) {
		removeLegend(legend(0));
	}

	// Create a layer properties from BNA geo features
	const Geo::GeoFeatureSet &featureSet = Geo::GeoFeatureSetSingleton::getInstance();

	for ( const auto &feature : featureSet.features() ) {
		auto *node = createOrGetNodeForCategory(feature->category());
		node->quadtree.addItem(feature);
		//node->features.push_back(*itf);
	}

	const auto &fepRegions = Regions::polyRegions();
	if ( fepRegions.regionCount() > 0 ) {
		// Add fep properties
		auto *fepNode = new CategoryNode(nullptr);
		createOrGetNodeForCategory(nullptr)->childs.push_back(fepNode);
		fepNode->properties = new LayerProperties("fep", _root->properties);
		fepNode->properties->read(fepRegions.dataDir());

		for ( size_t i = 0; i < fepRegions.regionCount(); ++i ) {
			//fepNode->features.push_back(fepRegions.region(i));
			fepNode->quadtree.addItem(fepRegions.region(i));
		}
	}

	if ( _root ) {
		// Build legends
		buildLegends(_root);
		orderTree(_root);
		updateBbox(_root);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Geo::GeoFeature *GeoFeatureLayer::findFeature(CategoryNode *node,
                                                    const Geo::GeoCoordinate &coord) const {
	if ( !node->properties->visible ) {
		return nullptr;
	}

	if ( !node->bbox.contains(coord) ) {
		return nullptr;
	}

	for ( auto rit = node->childs.rbegin(); rit != node->childs.rend(); ++rit ) {
		const auto *f = findFeature(*rit, coord);
		if ( f ) {
			return f;
		}
	}

	/*
	for ( size_t i = node->features.size(); i > 0; --i ) {
		Geo::GeoFeature *f = node->features[i-1];
		int rank = node->properties->rank < 0?f->rank():node->properties->rank;
		if ( rank <= canvas()->zoomLevel() ) {
			if ( f->bbox().contains(coord) && f->contains(coord) )
				return f;
		}
	}
	*/
	return node->quadtree.findLast(coord);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::toggleVisibility(CategoryNode *node, bool visible) {
	bool updateRequired = false;
	if ( node->properties->visible != visible ) {
		node->properties->visible = visible;
		updateRequired = true;
	}

	return updateRequired;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::toggleFeatureVisibility(bool checked) {
	auto *action = static_cast<QAction*>(sender());
	void *propertyPtr = action->data().value<void*>();
	auto *prop = reinterpret_cast<LayerProperties*>(propertyPtr);

	if ( !prop ) {
		return;
	}

	auto *node = _root->nodeForProperties(prop);
	if ( node && toggleVisibility(node, checked) ) {
		emit updateRequested(RasterLayer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureLayer::disableFeatureVisibility() {
	toggleFeatureVisibility(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::compareByIndex(const LayerProperties *p1,
                                     const LayerProperties *p2) {
	return p1->index < p2->index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::compareNodeByIndex(const GeoFeatureLayer::CategoryNode *n1,
                                         const GeoFeatureLayer::CategoryNode *n2) {
	return n1->properties->index < n2->properties->index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureLayer::compareNodeByName(const GeoFeatureLayer::CategoryNode *n1,
                                        const GeoFeatureLayer::CategoryNode *n2) {
	return n1->properties->name < n2->properties->name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
