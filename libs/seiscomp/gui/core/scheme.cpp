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



#include <seiscomp/gui/core/scheme.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/core/application.h>

#include <QTabBar>
#include <QTabWidget>

#include <boost/algorithm/string.hpp>

#define READ_BOOL(location) \
	try { location = SCApp->configGetBool("scheme."#location); } \
	catch ( ... ) {}

#define READ_STRING(location) \
	try { location = SCApp->configGetString("scheme."#location); } \
	catch ( ... ) {}


#define READ_COLOR(location) \
	location = SCApp->configGetColor("scheme."#location, location);

#define READ_BRUSH_COLOR(location) \
	location = SCApp->configGetColor("scheme."#location, location.color());

#define READ_BRUSH(location) \
	location = SCApp->configGetBrush("scheme."#location, location.color());

#define READ_COLOR_GRADIENT(location) \
	location = SCApp->configGetColorGradient("scheme."#location, location);

#define READ_FONT(location) \
	location = SCApp->configGetFont("scheme."#location, location);

#define READ_FONT_BY_NAME(var, location) \
	var = SCApp->configGetFont("scheme."#location, var);

#define READ_PEN(location) \
	location = SCApp->configGetPen("scheme."#location, location);

#define READ_INT(location) \
	try { location = SCApp->configGetInt("scheme."#location); } \
	catch ( ... ) {}

#define READ_DOUBLE(location) \
	try { location = SCApp->configGetDouble("scheme."#location); } \
	catch ( ... ) {}

#define READ_POINT(location) \
	{\
		int x = location.x();\
		int y = location.y();\
		try { x = SCApp->configGetInt("scheme."#location".x"); }\
		catch ( ... ) {}\
		try { y = SCApp->configGetInt("scheme."#location".y"); }\
		catch ( ... ) {}\
		location = QPoint(x,y);\
	}


using namespace std;

namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor blend(const QColor& c1, const QColor& c2, int percentOfC1) {
	return QColor((c1.red()*percentOfC1 + c2.red()*(100-percentOfC1)) / 100,
	              (c1.green()*percentOfC1 + c2.green()*(100-percentOfC1)) / 100,
	              (c1.blue()*percentOfC1 + c2.blue()*(100-percentOfC1)) / 100);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor blend(const QColor& c1, const QColor& c2) {
	int invertedAlpha = 255-c2.alpha();
	return QColor((c1.red()*invertedAlpha + c2.red()*c2.alpha()) / 255,
	              (c1.green()*invertedAlpha + c2.green()*c2.alpha()) / 255,
	              (c1.blue()*invertedAlpha + c2.blue()*c2.alpha()) / 255);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Scheme() {
	showMenu = true;
	showStatusBar = true;
	tabPosition = -1;
	distanceHypocentral = false;

	fonts.setBase(SCApp->font());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Scheme::applyTabPosition(QTabWidget *tab) {
	QTabBar *tabBar = tab->findChild<QTabBar*>();

	if ( tabPosition > 0 ) {
		tab->setTabPosition(static_cast<QTabWidget::TabPosition>(tabPosition-1));
		if ( tabBar )
			tabBar->setVisible(true);
	}
	else if ( tabPosition == 0 ) {
		if ( tabBar )
			tabBar->setVisible(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Colors() {
	spectrogram.setColorAt(0.0, QColor(255,   0, 255)); // pink
	spectrogram.setColorAt(0.2, QColor(  0,   0, 255)); // blue
	spectrogram.setColorAt(0.4, QColor(  0, 255, 255)); // cyan
	spectrogram.setColorAt(0.6, QColor(  0, 255,   0)); // green
	spectrogram.setColorAt(0.8, QColor(255, 255,   0)); // yellow
	spectrogram.setColorAt(1.0, QColor(255,   0,   0)); // red
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Splash::Splash()
: version(0,104,158,255), message(128,128,128,255) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Picks::Picks() {
	manual = Qt::green;
	automatic = Qt::red;
	undefined = Qt::gray;
	disabled = Qt::gray;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Arrivals::Arrivals() {
	manual = QColor(0, 160, 0);
	automatic = QColor(160, 0, 0);
	theoretical = QColor(0, 0, 160);
	undefined = QColor(160, 0, 0);
	disabled = Qt::gray;
	uncertainties = QPen(Qt::gray, 0.8);
	defaultUncertainties = QPen(QColor(128,128,128,64), 0.8);

	/* BERND Version
	residuals.setColorAt(-10, QColor(0,0,255));
	residuals.setColorAt(0, QColor(0,255,0));
	residuals.setColorAt(10, QColor(255,0,255));
	*/

	residuals.setColorAt(+8, QColor(100,0,0));
	residuals.setColorAt(+4, QColor(255,0,0));
	residuals.setColorAt(+3, QColor(255,100,100));
	residuals.setColorAt(+2, QColor(255,170,170));
	residuals.setColorAt(+1, QColor(255,220,220));
	residuals.setColorAt( 0, QColor(255,255,255));
	residuals.setColorAt(-1, QColor(220,220,255));
	residuals.setColorAt(-2, QColor(170,170,255));
	residuals.setColorAt(-3, QColor(100,100,255));
	residuals.setColorAt(-4, QColor(0,0,255));
	residuals.setColorAt(-8, QColor(0,0,100));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Magnitudes::Magnitudes() {
	set = QColor(0, 160, 0);
	unset = Qt::transparent;
	disabled = Qt::gray;

	residuals.setColorAt(+1.0, QColor(100,0,0));
	residuals.setColorAt(+0.6, QColor(255,0,0));
	residuals.setColorAt(+0.4, QColor(255,100,100));
	residuals.setColorAt(+0.2, QColor(255,170,170));
	residuals.setColorAt(+0.1, QColor(255,220,220));
	residuals.setColorAt( 0.0, QColor(255,255,255));
	residuals.setColorAt(-0.1, QColor(220,220,255));
	residuals.setColorAt(-0.2, QColor(170,170,255));
	residuals.setColorAt(-0.4, QColor(100,100,255));
	residuals.setColorAt(-0.6, QColor(0,0,255));
	residuals.setColorAt(-1.0, QColor(0,0,100));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::RecordStates::RecordStates() {
	unrequested = QColor(0,0,0,128);
	requested = QColor(255,255,0,128);
	inProgress = QColor(0,255,0,16);
	notAvailable = QColor(255,0,0,128);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::RecordBorders::RecordBorders() {
	QColor color = Qt::gray;
	standard.pen = QPen(color);
	color.setAlpha(16);
	standard.brush = color;

	color = Qt::green;
	signatureValid.pen = QPen(color);
	color.setAlpha(16);
	signatureValid.brush = QBrush(color);

	color = Qt::red;
	signatureInvalid.pen = QPen(color);
	color.setAlpha(16);
	signatureInvalid.brush = QBrush(color);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Records::Records() {
	alignment = Qt::red;
	foreground = QColor(128, 128, 128);
	alternateForeground = foreground;
	spectrogram = Qt::black;
	offset = QColor(192,192,255);
	gridPen = QPen(QColor(0,0,0,32), 1, Qt::DashLine);
	subGridPen = QPen(QColor(0,0,0,0), 1, Qt::DotLine);
	gaps = QColor(255, 255, 0, 64);
	overlaps = QColor(255, 0, 255, 64);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Stations::Stations() {
	text = Qt::white;
	associated = QColor(130, 173, 88);
	selected = QColor(77, 77, 184);
	triggering = QColor(Qt::red);
	triggered0 = QColor(0, 128, 255);
	triggered1 = QColor(0, 0, 255);
	triggered2 = QColor(0, 0, 128);
	disabled = QColor(102, 102, 102, 100);
	idle = QColor(102, 102, 102, 128);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::QC::QC() :
	delay0(0, 255, 255),
	delay1(0, 255, 0),
	delay2(255, 253, 0),
	delay3(255, 102, 51),
	delay4(255, 0, 0),
	delay5(204, 204, 204),
	delay6(153, 153, 153),
	delay7(102, 102, 102),
//	qcWarning(255, 255, 255),
	qcWarning(Qt::yellow),
	qcError(Qt::red),
	qcOk(Qt::green),
	qcNotSet(0, 0, 0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::OriginSymbol::OriginSymbol() {
	classic = false;
	depth.discrete = true;
	depth.gradient.setColorAt(0, Qt::red);
	depth.gradient.setColorAt(50, QColor(255, 165, 0));
	depth.gradient.setColorAt(100, Qt::yellow);
	depth.gradient.setColorAt(250, Qt::green);
	depth.gradient.setColorAt(600, Qt::blue);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::OriginStatus::OriginStatus() :
	automatic(Qt::red),
	manual(Qt::darkGreen){}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::GroundMotion::GroundMotion() :
	gmNotSet(0, 0, 0),
	gm0(0, 0, 255),
	gm1(0, 0, 255),
	gm2(0, 167, 255),
	gm3(0, 238, 255),
	gm4(0, 255,  0),
	gm5(255,255,0),
	gm6(255, 210, 0),
	gm7(255, 160, 0),
	gm8(255, 0, 0),
	gm9(160, 0, 60)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::RecordView::RecordView() :
	selectedTraceZoom(192, 192, 255, 192)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Map::Map()
: lines(255, 255, 255, 64)
, outlines(255, 255, 255)
, directivity(QColor(255 ,160, 0))
, grid(Qt::white, 1, Qt::DotLine)
, stationAnnotations(Qt::red)
, cityLabels(Qt::black)
, cityOutlines(Qt::black)
, cityCapital(255, 160, 122)
, cityNormal(Qt::white)
, cityHalo(Qt::white)
{
	annotations.normalText = QPen(QColor(192,192,192));
	annotations.normalBorder = QPen(QColor(160,160,164));
	annotations.normalBackground = QColor(32,32,32,192);

	annotations.highlightedText = QPen(QColor(0,0,0));
	annotations.highlightedBorder = QPen(QColor(160,160,164));
	annotations.highlightedBackground = QColor(255,255,255,192);

	annotations.textSize = 9;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Colors::Legend::Legend() :
	background(255, 255, 255, 224),
	border(160, 160, 160),
	text(64, 64, 64),
	headerText(0, 0, 0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Splash::Splash() {
	version.pos = QPoint(390, 145);
	version.align = Qt::AlignRight | Qt::AlignTop;
	message.pos = QPoint(200, 260);
	message.align = Qt::AlignHCenter | Qt::AlignBottom;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Map::Map() {
	stationSize = 12;
	originSymbolMinSize = 9;
	originSymbolMinMag = 1.2;
	originSymbolScaleMag = 4.9;
	vectorLayerAntiAlias = true;
	bilinearFilter = true;
	showGrid = true;
	showLayers = true;
	showCities = true;
	showLegends = false;
	cityPopulationWeight = 150;
	cityHaloWidth = 0;
	toBGR = false;
	polygonRoughness = 3;
	projection = "";
	maxZoom = 24;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Marker::Marker() {
	lineWidth = 1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::RecordBorders::RecordBorders() {
	drawMode = Gui::RecordWidget::Box;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Records::Records() {
	lineWidth = 1;
	antiAliasing = true;
	optimize = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Fonts::Fonts() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Scheme::Fonts::setBase(const QFont& f) {
	Q_UNUSED(f);

	normal = base;
	normal.setPointSize(base.pointSize());

	smaller = base;
	smaller.setPointSize(base.pointSize() - 2);

	large = base;
	large.setPointSize(base.pointSize() + 6);

	highlight = normal;
	highlight.setBold(true);

	heading1 = normal;
	heading1.setBold(true);
	heading1.setPointSize(normal.pointSize() * 2);

	heading2 = base;
	heading2.setBold(true);
	heading2.setPointSize(base.pointSize() * 2);

	heading3 = base;
	heading3.setBold(true);
	heading3.setPointSize(base.pointSize() + 4);

	cityLabels = base;
	splashVersion = base;
	splashVersion.setBold(true);
	splashVersion.setPixelSize(16);
	splashMessage = base;
	splashMessage.setPixelSize(12);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Precision::Precision() {
	depth = 0;
	distance = 1;
	location = 2;
	magnitude = 1;
	originTime = 0;
	pickTime = 1;
	traceValues = 1;
	rms = 1;
	uncertainties = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::Unit::Unit() {
	distanceInKM = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme::DateTime::DateTime() {
	useLocalTime = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Scheme::fetch() {
	READ_BOOL(showMenu);
	READ_BOOL(showStatusBar);

	std::string tabPosition = "";
	READ_STRING(tabPosition);

	if ( tabPosition == "off" )
		this->tabPosition = 0;
	else if ( tabPosition == "north" )
		this->tabPosition = 1;
	else if ( tabPosition == "south" )
		this->tabPosition = 2;
	else if ( tabPosition == "west" )
		this->tabPosition = 3;
	else if ( tabPosition == "east" )
		this->tabPosition = 4;

	READ_BOOL(distanceHypocentral);

	READ_COLOR(colors.background);

	READ_COLOR(colors.splash.version);
	READ_COLOR(colors.splash.message);

	READ_COLOR(colors.records.alignment);
	READ_COLOR(colors.records.background);
	READ_COLOR(colors.records.alternateBackground);
	READ_COLOR(colors.records.foreground);
	READ_COLOR(colors.records.alternateForeground);
	READ_COLOR(colors.records.spectrogram);
	READ_PEN(colors.records.offset);
	READ_PEN(colors.records.gridPen);
	READ_PEN(colors.records.subGridPen);
	READ_BRUSH_COLOR(colors.records.gaps);
	READ_BRUSH_COLOR(colors.records.overlaps);
	READ_COLOR(colors.records.states.unrequested);
	READ_COLOR(colors.records.states.requested);
	READ_COLOR(colors.records.states.inProgress);
	READ_COLOR(colors.records.states.notAvailable);
	READ_PEN(colors.records.borders.standard.pen);
	READ_BRUSH(colors.records.borders.standard.brush);
	READ_PEN(colors.records.borders.signatureValid.pen);
	READ_BRUSH(colors.records.borders.signatureValid.brush);
	READ_PEN(colors.records.borders.signatureInvalid.pen);
	READ_BRUSH(colors.records.borders.signatureInvalid.brush);

	READ_COLOR_GRADIENT(colors.spectrogram);

	READ_COLOR(colors.picks.manual);
	READ_COLOR(colors.picks.automatic);
	READ_COLOR(colors.picks.undefined);
	READ_COLOR(colors.picks.disabled);

	READ_COLOR(colors.arrivals.manual);
	READ_COLOR(colors.arrivals.automatic);
	READ_COLOR(colors.arrivals.theoretical);
	READ_COLOR(colors.arrivals.undefined);
	READ_COLOR(colors.arrivals.disabled);
	READ_PEN(colors.arrivals.uncertainties);
	READ_PEN(colors.arrivals.defaultUncertainties);
	READ_COLOR_GRADIENT(colors.arrivals.residuals);

	READ_COLOR(colors.magnitudes.set);
	READ_COLOR(colors.magnitudes.unset);
	READ_COLOR(colors.magnitudes.disabled);
	READ_COLOR_GRADIENT(colors.magnitudes.residuals);

	READ_COLOR(colors.stations.text);
	READ_COLOR(colors.stations.associated);
	READ_COLOR(colors.stations.triggering);
	READ_COLOR(colors.stations.triggered0);
	READ_COLOR(colors.stations.triggered1);
	READ_COLOR(colors.stations.triggered2);
	READ_COLOR(colors.stations.disabled);
	READ_COLOR(colors.stations.idle);

	READ_COLOR(colors.qc.delay0);
	READ_COLOR(colors.qc.delay1);
	READ_COLOR(colors.qc.delay2);
	READ_COLOR(colors.qc.delay3);
	READ_COLOR(colors.qc.delay4);
	READ_COLOR(colors.qc.delay5);
	READ_COLOR(colors.qc.delay6);
	READ_COLOR(colors.qc.delay7);
	READ_COLOR(colors.qc.qcWarning);
	READ_COLOR(colors.qc.qcError);
	READ_COLOR(colors.qc.qcOk);
	READ_COLOR(colors.qc.qcNotSet);

	READ_COLOR(colors.gm.gm0);
	READ_COLOR(colors.gm.gm1);
	READ_COLOR(colors.gm.gm2);
	READ_COLOR(colors.gm.gm3);
	READ_COLOR(colors.gm.gm4);
	READ_COLOR(colors.gm.gm5);
	READ_COLOR(colors.gm.gm6);
	READ_COLOR(colors.gm.gm7);
	READ_COLOR(colors.gm.gm8);
	READ_COLOR(colors.gm.gm9);
	READ_COLOR(colors.gm.gmNotSet);

	READ_COLOR(colors.recordView.selectedTraceZoom);

	READ_COLOR(colors.map.lines);
	READ_COLOR(colors.map.outlines);
	READ_PEN(colors.map.directivity);
	READ_PEN(colors.map.grid);
	READ_COLOR(colors.map.stationAnnotations);
	READ_COLOR(colors.map.cityLabels);
	READ_COLOR(colors.map.cityOutlines);
	READ_COLOR(colors.map.cityCapital);
	READ_COLOR(colors.map.cityNormal);
	READ_COLOR(colors.map.cityHalo);
	READ_PEN(colors.map.annotations.normalBorder);
	READ_PEN(colors.map.annotations.normalText);
	READ_BRUSH(colors.map.annotations.normalBackground);
	READ_PEN(colors.map.annotations.highlightedBorder);
	READ_PEN(colors.map.annotations.highlightedText);
	READ_BRUSH(colors.map.annotations.highlightedBackground);
	READ_INT(colors.map.annotations.textSize);

	READ_COLOR(colors.legend.background);
	READ_COLOR(colors.legend.border);
	READ_COLOR(colors.legend.text);
	READ_COLOR(colors.legend.headerText);

	READ_BOOL(colors.originSymbol.classic);
	READ_COLOR_GRADIENT(colors.originSymbol.depth.gradient);
	READ_BOOL(colors.originSymbol.depth.discrete);

	READ_COLOR(colors.originStatus.automatic);
	READ_COLOR(colors.originStatus.manual);

	try {
		vector<string> agencyColors = SCApp->configGetStrings("scheme.colors.agencies");
		for ( size_t i = 0; i < agencyColors.size(); ++i ) {
			size_t pos = agencyColors[i].rfind(':');
			if ( pos == std::string::npos ) continue;
			std::string value = agencyColors[i].substr(0, pos);
			std::string strColor = agencyColors[i].substr(pos+1);
			QColor color;
			if ( fromString(color, strColor) )
				colors.agencies[value] = color;
		}
	}
	catch ( ... ) {}

	READ_INT(marker.lineWidth);
	READ_INT(records.lineWidth);
	READ_BOOL(records.antiAliasing);
	READ_BOOL(records.optimize);

	try {
		string mode = SCApp->configGetString("scheme.records.borders.drawMode");
		boost::to_lower(mode);
		if ( mode == "box" ) {
			records.recordBorders.drawMode = Gui::RecordWidget::Box;
		}
		else if ( mode == "bottomline" ) {
			records.recordBorders.drawMode = Gui::RecordWidget::BottomLine;
		}
		else {
			records.recordBorders.drawMode = Gui::RecordWidget::TopLine;
		}
	}
	catch ( ... ) {}

	READ_FONT(fonts.base);
	fonts.setBase(fonts.base);
	READ_FONT_BY_NAME(fonts.smaller, fonts.small);
	READ_FONT(fonts.normal);
	READ_FONT(fonts.large);
	READ_FONT(fonts.highlight);
	READ_FONT(fonts.heading1);
	READ_FONT(fonts.heading2);
	READ_FONT(fonts.heading3);
	READ_FONT(fonts.cityLabels);
	READ_FONT(fonts.splashVersion);
	READ_FONT(fonts.splashMessage);
	SCApp->setFont(fonts.base);

	READ_INT(splash.version.align);
	READ_POINT(splash.version.pos);
	READ_INT(splash.message.align);
	READ_POINT(splash.message.pos);

	READ_INT(map.stationSize);
	READ_INT(map.originSymbolMinSize);
	READ_DOUBLE(map.originSymbolMinMag);
	READ_DOUBLE(map.originSymbolScaleMag);
	READ_BOOL(map.vectorLayerAntiAlias);
	READ_BOOL(map.bilinearFilter);
	READ_BOOL(map.showGrid);
	READ_BOOL(map.showLayers);
	READ_BOOL(map.showCities);
	READ_BOOL(map.showLegends);
	READ_INT(map.cityPopulationWeight);
	READ_INT(map.cityHaloWidth);
	READ_BOOL(map.toBGR);
	READ_INT(map.polygonRoughness);
	READ_STRING(map.projection);
	READ_INT(map.maxZoom);

	READ_INT(precision.depth);
	READ_INT(precision.distance);
	READ_INT(precision.location);
	READ_INT(precision.magnitude);
	READ_INT(precision.originTime);
	READ_INT(precision.pickTime);
	READ_INT(precision.traceValues);
	READ_INT(precision.rms);
	READ_INT(precision.uncertainties);

	READ_BOOL(unit.distanceInKM);
	READ_BOOL(dateTime.useLocalTime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QFont Scheme::font(int relativeFontSize, bool bold, bool italic) {
	QFont f = SCApp->font();
	f.setBold(bold);
	f.setItalic(italic);
	int newSize = f.pointSize() + relativeFontSize;
	if ( newSize < 1 ) newSize = 1;
	f.setPointSize(newSize);
	return f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
