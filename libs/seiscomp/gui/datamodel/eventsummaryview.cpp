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


#define SEISCOMP_COMPONENT EventSummaryView

#include "eventsummaryview.h"
#include <seiscomp/gui/datamodel/ui_eventsummaryview.h>
#include <seiscomp/gui/datamodel/ui_eventsummaryview_hypocenter.h>

#include <seiscomp/logging/log.h>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/originquality.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/momenttensor.h>
#include <seiscomp/datamodel/journalentry.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/system/hostinfo.h>
#include <seiscomp/config/config.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/math/conversions.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/seismology/regions.h>
#include <seiscomp/utils/replace.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/datamodel/tensorsymbol.h>
#include <seiscomp/gui/datamodel/utils.h>

#include <QMessageBox>
#include <QScrollArea>
#include <QStringList>

#ifdef WIN32
#undef min
#undef max
#endif

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;


namespace {


#define SET_COLOR(w,c) {\
	QPalette p = w->palette();\
	p.setColor(QPalette::WindowText, c);\
	w->setPalette(p);\
}\


struct PoiResolver : public Seiscomp::Util::VariableResolver {
	PoiResolver(const double& dist,
	            const std::string& dir,
	            const std::string& name,
	            double lat, double lon)
	 : _lat(lat), _lon(lon), _dist(dist), _dir(dir), _name(name) {}

	bool resolve(std::string& variable) const {
		if ( VariableResolver::resolve(variable) )
			return true;

		if ( variable == "dist" )
			variable = toString(_dist);
		else if ( variable == "dir" )
			variable = _dir;
		else if ( variable == "poi" )
			variable = _name;
		else if ( variable == "region" )
			variable = Regions::getRegionName(_lat, _lon);
		else
			return false;

		return true;
	}

	double _lat, _lon;
	const double& _dist;
	const std::string& _dir;
	const std::string& _name;
};


}



namespace Seiscomp {
namespace Gui {


// <---------------------------------------------------------------------------------------------->


MagList::MagList(QWidget* parent)
 : QWidget(parent), _space(true)
{
	_referenceMagsVisible = false;
// 	setGeometry(0, 0, 10, 10);

//     QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
//     sizePolicy.setHorizontalStretch(0);
//     sizePolicy.setVerticalStretch(0);
// //     sizePolicy.setHeightForWidth(_widget->sizePolicy().hasHeightForWidth());
//     setSizePolicy(sizePolicy);
//
// //     setMinimumSize(QSize(100, 100));
//      setMaximumSize(QSize(500, 400));
//
//
// 	_widget = new QWidget(this);

	_mainLayout = new QGridLayout(this);
	_mainLayout->setSpacing(0);
	_mainLayout->setMargin(0);
// 	setLayout(_mainLayout);

// 	setWidget(_widget);
// 	setFrameStyle(QFrame::NoFrame);
// 	setWidgetResizable(true);
// 	setAttribute( Qt::WA_StaticContents );

// 	updateGeometry();


	// set Header Line
	_header = new MagRow(nullptr, false, this);
	_header->setVisible(false);
	_mainLayout->addWidget(_header->_type, 0, 0);
	_mainLayout->addWidget(_header->_magnitude, 0, 1, 1, 2);
	_mainLayout->addWidget(_header->_stdev, 0, 3, 1, 2);
	_mainLayout->addWidget(_header->_quality, 0, 5, 1, 2);
}

// QSize MagList::sizeHint() const{
//
// return _widget->size();
// // return QSize(300,10);
// }


MagList::~MagList(){
	clear();
}


void MagList::clear(){
	// close and delete magnitude rows
	for (int i = 0; i < _magRows.size(); i++) {
		delete _magRows.at(i);
	}
	_magRows.clear();
}


void MagList::reset() {
	foreach(MagRow* row, _magRows) {
		row->setMagnitude(nullptr);
		row->setBold(false);
	}
}


void MagList::addMag(DataModel::Magnitude* netMag, bool bold, bool visible){
	// create new magnitude display row
	MagRow *magRow = nullptr;

	if ( netMag )
		magRow = row(netMag->type());

	if ( !magRow ) {
		magRow = new MagRow(netMag, bold, this);
		magRow->setReferenceMagnitudeVisible(_referenceMagsVisible);
		magRow->setReferenceMagnitudeColor(_referenceColor);
		_magRows.push_back(magRow);
		//_mainLayout->addWidget(_magRows.back());
		int row = _mainLayout->rowCount();
		_mainLayout->addWidget(_magRows.back()->_type, row, 0);
		_mainLayout->addWidget(_magRows.back()->_magnitude, row, 1);
		_mainLayout->addWidget(_magRows.back()->_magnitudeReference, row, 2);
		_mainLayout->addWidget(_magRows.back()->_stdev, row, 3);
		_mainLayout->addWidget(_magRows.back()->_stdevReference, row, 4);
		_mainLayout->addWidget(_magRows.back()->_quality, row, 5);
		_mainLayout->addWidget(_magRows.back()->_qualityReference, row, 6);

		if ( visible )
			_header->setVisible(true);
	}
	else {
		magRow->setMagnitude(netMag);
		magRow->setBold(bold);
	}

	magRow->setVisible(visible);
}


void MagList::addMag(const std::string& type, bool bold, bool visible) {
	MagRow *magRow = row(type);
	if ( !magRow ) {
		magRow = new MagRow(type, bold, this);
		magRow->setReferenceMagnitudeVisible(_referenceMagsVisible);
		magRow->setReferenceMagnitudeColor(_referenceColor);
		_magRows.push_back(magRow);
		//_mainLayout->addWidget(_magRows.back());
		int row = _mainLayout->rowCount();
		_mainLayout->addWidget(_magRows.back()->_type, row, 0);
		_mainLayout->addWidget(_magRows.back()->_magnitude, row, 1);
		_mainLayout->addWidget(_magRows.back()->_magnitudeReference, row, 2);
		_mainLayout->addWidget(_magRows.back()->_stdev, row, 3);
		_mainLayout->addWidget(_magRows.back()->_stdevReference, row, 4);
		_mainLayout->addWidget(_magRows.back()->_quality, row, 5);
		_mainLayout->addWidget(_magRows.back()->_qualityReference, row, 6);

		if ( visible )
			_header->setVisible(true);
	}
	else
		magRow->setBold(bold);
	
	magRow->setVisible(visible);
}


void MagList::updateMag(DataModel::Magnitude* netMag) {
	foreach(MagRow* row, _magRows) {
		if ( row->magnitude() && (row->magnitude()->publicID() == netMag->publicID()) ) {
			row->updateContent();
			return;
		}
	}
}


void MagList::updateReferenceMag(DataModel::Magnitude* netMag) {
	foreach(MagRow* row, _magRows) {
		if ( row->referenceMagnitude() && (row->referenceMagnitude()->publicID() == netMag->publicID()) ) {
			row->updateContent();
			return;
		}
	}
}


void MagList::selectMagnitude(const char *id) {
	foreach(MagRow* row, _magRows) {
		if ( row->magnitude() &&
		     row->magnitude()->publicID() == id )
			row->setBold(true);
		else
			row->setBold(false);
	}
}


MagRow* MagList::row(const std::string& type) const {
	foreach(MagRow* row, _magRows) {
		if ( row->_type->text() == type.c_str() )
			return row;
	}

	return nullptr;
}


void MagList::showAll() {
	_header->setVisible(true);
	foreach(MagRow* row, _magRows)
		row->setVisible(true);
}


void MagList::hideTypes(const std::set<std::string>& types) {
	bool allHidden = true;
	foreach(MagRow* row, _magRows) {
		row->setVisible(types.find(row->_type->text().toStdString()) != types.end());
		if ( row->isVisible() )
			allHidden = false;
	}

	_header->setVisible(!allHidden);
}


void MagList::setReferenceMagnitudesVisible(bool v) {
	_referenceMagsVisible = v;
	foreach(MagRow* row, _magRows) {
		row->setReferenceMagnitudeVisible(v);
	}
}


void MagList::setReferenceMagnitudesColor(QColor c) {
	_referenceColor = c;

	foreach(MagRow* row, _magRows) {
		row->setReferenceMagnitudeColor(c);
	}
}


// <---------------------------------------------------------------------------------------------->



MagRow::MagRow(DataModel::Magnitude* netMag, bool bold, QWidget* parent)
 : QWidget(parent), _netMag(netMag), _netMagReference(nullptr)
{
	_header = _netMag == nullptr;
	_referenceMagVisible = false;
	init();
	setBold(bold);
}


MagRow::MagRow(const std::string& type, bool bold, QWidget *parent)
 : QWidget(parent), _netMag(nullptr), _netMagReference(nullptr)
{
	_header = false;
	_referenceMagVisible = false;
	init();
	_type->setText(type.c_str());
	setBold(bold);
}


MagRow::~MagRow() {
}


void MagRow::init() {
	_type      = new QLabel();
	_magnitude = new QLabel();
	_stdev     = new QLabel();
	_quality   = new QLabel();

	/*
	_type->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	_magnitude->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	_stdev->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	_quality->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	*/

	if ( !_header ) {
		_magnitudeReference = new QLabel();
		_stdevReference     = new QLabel();
		_qualityReference   = new QLabel();
	}
	else {
		_magnitudeReference = nullptr;
		_stdevReference     = nullptr;
		_qualityReference   = nullptr;
	}

	_type->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	_magnitude->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_stdev->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_quality->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	_magnitude->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_stdev->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_quality->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	if ( !_header ) {
		_magnitudeReference->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		_stdevReference->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		_qualityReference->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		_magnitudeReference->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		_stdevReference->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		_qualityReference->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	}

	updateContent();

	_rowsLayout = new QHBoxLayout(this);
	_rowsLayout->setSpacing(0);
	_rowsLayout->setMargin(0);
	_rowsLayout->addWidget(_type);

	if ( !_header ) {
		QHBoxLayout *l = new QHBoxLayout;
		l->setMargin(0);
		l->addWidget(_magnitude);
		//l->addWidget(_magnitudeReference);
		_rowsLayout->addLayout(l);
	}
	else {
		_rowsLayout->addWidget(_magnitude);
	}

	if ( !_header ) {
		QHBoxLayout *l = new QHBoxLayout;
		l->setMargin(0);
		l->addWidget(_stdev);
		//l->addWidget(_stdevReference);
		_rowsLayout->addLayout(l);
	}
	else {
		_rowsLayout->addWidget(_stdev);
	}

	if ( !_header ) {
		QHBoxLayout *l = new QHBoxLayout;
		l->setMargin(0);
		l->addWidget(_quality);
		//l->addWidget(_qualityReference);
		_rowsLayout->addLayout(l);
	}
	else {
		_rowsLayout->addWidget(_quality);
	}

	setReferenceMagnitudeVisible(_referenceMagVisible);
}


void MagRow::setMagnitude(DataModel::Magnitude* nm) {
	_netMag = nm;
	updateContent();
}


void MagRow::setReferenceMagnitude(DataModel::Magnitude* nm) {
	_netMagReference = nm;
	updateContent();
}


void MagRow::setReferenceMagnitudeVisible(bool v) {
	_referenceMagVisible = v;

	if ( _magnitudeReference ) _magnitudeReference->setVisible(v && isVisible());
	if ( _qualityReference ) _qualityReference->setVisible(v && isVisible());
	if ( _stdevReference ) _stdevReference->setVisible(v && isVisible());

	if ( v ) updateContent();
}


void MagRow::setReferenceMagnitudeColor(QColor c) {
	if ( _magnitudeReference )
		SET_COLOR(_magnitudeReference, c);

	if ( _qualityReference )
		SET_COLOR(_qualityReference, c);

	if ( _stdevReference )
		SET_COLOR(_stdevReference, c);
}


void MagRow::setBold(bool bold) {
	QFont font(SCScheme.fonts.normal);
	if ( bold ) font.setBold(true);

	_magnitude->setFont(font);
	_type->setFont(font);
	_quality->setFont(font);
	_stdev->setFont(font);

	font.setBold(false);
	if ( _magnitudeReference ) _magnitudeReference->setFont(font);
	if ( _qualityReference ) _qualityReference->setFont(font);
	if ( _stdevReference ) _stdevReference->setFont(font);
}


void MagRow::setVisible(bool v) {
	QWidget::setVisible(v);

	_magnitude->setVisible(v);
	_type->setVisible(v);
	_quality->setVisible(v);
	_stdev->setVisible(v);

	if ( _magnitudeReference ) _magnitudeReference->setVisible(_referenceMagVisible && v);
	if ( _qualityReference ) _qualityReference->setVisible(_referenceMagVisible && v);
	if ( _stdevReference ) _stdevReference->setVisible(_referenceMagVisible && v);
}


void MagRow::updateContent() {
	if (_netMag) { // set Magnitude Row
		QString text;
		double netmagval = _netMag->magnitude().value();
		if ( netmagval < 12 ) {
			text = QString("%1").arg(netmagval, 0, 'f', SCScheme.precision.magnitude);
		}
		else {
			text = "-";
		}
		_magnitude->setText(text);
		_type->setText(QString("%1").arg(_netMag->type().c_str()));

		try {
			int count = _netMag->stationCount();
			_quality->setText(QString("%1").arg(count));
		}
		catch ( ... ) {
			_quality->setText("-");
		}

		double stdev = 100;
		try {
			stdev = _netMag->magnitude().uncertainty();
		}
		catch ( ... ) {
			try {
				stdev = 0.5*(_netMag->magnitude().lowerUncertainty() + _netMag->magnitude().upperUncertainty());
			}
			catch ( ... ) {}
		}

		if ( stdev < 10 ) {
			_stdev->setText(QString("%1").arg(stdev, 0, 'f', SCScheme.precision.magnitude));
		}
		else {
			_stdev->setText("-");
		}
	}
	else if ( _header ){ // set Header Row
		_magnitude->setText(QString("Value"));
		_type->setText(QString("Type"));
		_quality->setText(QString("Count"));
		_stdev->setText(QString("+/-"));
	}
	else {
		_magnitude->setText(QString("-"));
		_quality->setText(QString("-"));
		_stdev->setText(QString("-"));
	}

	if ( _referenceMagVisible ) {
		if ( _netMagReference ) {
			QString text;
			double netmagval = _netMagReference->magnitude().value();
			if ( netmagval < 12 ) {
				text = QString("%1").arg(netmagval, 0, 'f', SCScheme.precision.magnitude);
			}
			else {
				text = "-";
			}
			_magnitudeReference->setText(text);

			try {
				int count = _netMagReference->stationCount();
				_qualityReference->setText(QString("%1").arg(count));
			}
			catch ( ... ) {
				_qualityReference->setText("-");
			}

			double stdev = 100;
			try {
				stdev = _netMagReference->magnitude().uncertainty();
			}
			catch ( ... ) {
				try {
					stdev = 0.5*(_netMagReference->magnitude().lowerUncertainty() + _netMagReference->magnitude().upperUncertainty());
				}
				catch ( ... ) {}
			}

			if ( stdev < 10 ) {
				_stdevReference->setText(QString("%1").arg(stdev, 0, 'f', SCScheme.precision.magnitude));
			}
			else {
				_stdevReference->setText(QString("-"));
			}
		}
		else {
			_magnitudeReference->setText(QString("-"));
			_qualityReference->setText(QString("-"));
			_stdevReference->setText(QString("-"));
		}
	}

	update();
}

// <---------------------------------------------------------------------------------------------->


EventSummaryView::EventSummaryView(const MapsDesc &maps,
                                   DataModel::DatabaseQuery* reader,
                                   QWidget *parent)
: QWidget(parent)
, _ui(new Ui::EventSummaryView)
, _uiHypocenter(new Ui::Hypocenter)
, _reader(reader) {
	_maptree = new Gui::Map::ImageTree(maps);
	init();
}


EventSummaryView::EventSummaryView(Map::ImageTree* mapTree,
                                   DataModel::DatabaseQuery* reader,
                                   QWidget *parent)
: QWidget(parent)
, _ui(new Ui::EventSummaryView)
, _uiHypocenter(new Ui::Hypocenter)
, _reader(reader) {
	_maptree = mapTree;
	init();
}


EventSummaryView::~EventSummaryView(){
	delete _ui;
	delete _uiHypocenter;
}


#define SET_FONT(labelName, f1, f2) \
	_ui->label##labelName->setFont(SCScheme.fonts.f1); \
	_ui->label##labelName##Value->setFont(SCScheme.fonts.f2)


#define SET_VALUE(labelName, txt) \
	_ui->label##labelName##Value->setText(txt)


#define DISABLE_FRAME(f) \
	f->setFrameShape(QFrame::NoFrame); \
	if ( f->layout() ) \
		f->layout()->setMargin(0)


#define ENABLE_FRAME(f) \
	f->setFrameStyle(QFrame::StyledPanel | QFrame::Raised); \
	if ( f->layout() ) f->layout()->setMargin(4); \
	if ( f->layout() ) f->layout()->setSpacing(4)


class ScrollArea : public QScrollArea {
	public:
		ScrollArea(QWidget *parent)
		: QScrollArea(parent) {
			viewport()->setAutoFillBackground(false);
		}

		QSize sizeHint() const {
			if ( widget() == nullptr )
				return QScrollArea::sizeHint();

			return QSize(widget()->minimumWidth(), 0);
		}
};


class ScrollPanelWidget : public QWidget {
	public:
		ScrollPanelWidget(QScrollArea *forward) : _widget(forward) {}

	protected:
		void resizeEvent(QResizeEvent *e) {
			QWidget::resizeEvent(e);
			//_widget->setFixedWidth(e->size().width());
			//_widget->setMinimumWidth(e->size().width());
			_widget->setMinimumWidth(layout()->minimumSize().width());
		}


	private:
		QScrollArea *_widget;
};


void EventSummaryView::init() {
	_ui->setupUi(this);

	_enableFullTensor = false;
	_displayFocMechs = true;

	ScrollArea *area = new ScrollArea(_ui->frameEpicenterInformation);
	area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QWidget *infoPanel = new ScrollPanelWidget(area);
	QSizePolicy sp = infoPanel->sizePolicy();
	//sp.setHorizontalPolicy(QSizePolicy::Maximum);
	sp.setVerticalPolicy(QSizePolicy::Maximum);
	infoPanel->setSizePolicy(sp);

	QVBoxLayout *l = new QVBoxLayout(infoPanel);
	l->setMargin(0);

	QWidget *hypoCenterInfo = new QWidget(infoPanel);
	_uiHypocenter->setupUi(hypoCenterInfo);
	l->addWidget(hypoCenterInfo);

	/*
	QWidget *focalMechanismInfo = new QWidget(infoPanel);
	_uiHypocenter->setupUi(focalMechanismInfo);
	l->addWidget(focalMechanismInfo);
	//focalMechanismInfo->setVisible(false);
	*/

	setFMParametersVisible(false);

	area->setWidget(infoPanel);
	// Set by setWidget, reset it again
	infoPanel->setAutoFillBackground(false);
	area->setWidgetResizable(true);
	//area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	area->setFrameShape(QFrame::NoFrame);
	if ( area->layout() )
		area->layout()->setMargin(0);

	l = new QVBoxLayout(_ui->frameEpicenterInformation);
	l->setMargin(0);
	l->addWidget(area);

	_automaticOriginEnabledColor = Qt::darkRed;
	_automaticOriginDisabledColor = palette().color(QPalette::Disabled, QPalette::WindowText);
	_automaticOriginColor = palette().color(QPalette::WindowText);
	_automaticFMColor = palette().color(QPalette::WindowText);

	QObject *drawFilter = new ElideFadeDrawer(this);

	// Set the font sizes
	//_ui->frameInformation->setVisible(false);
	bool withBorders = false;
	try { withBorders = SCApp->configGetBool("summary.borders"); } catch ( ... ) {}

	try { _ignoreOtherEvents = SCApp->configGetBool("ignoreOtherEvents"); }
	catch ( ... ) { _ignoreOtherEvents = true; }

	double lonmin = -180;
	double lonmax = 180;
	double latmin = -90;
	double latmax = 90;

	try { lonmin = SCApp->configGetDouble("display.lonmin"); } catch (Config::Exception &) {}
	try { lonmax = SCApp->configGetDouble("display.lonmax"); } catch (Config::Exception &) {}
	try { latmin = SCApp->configGetDouble("display.latmin"); } catch (Config::Exception &) {}
	try { latmax = SCApp->configGetDouble("display.latmax"); } catch (Config::Exception &) {}

	QRectF displayRect;
	displayRect.setRect(lonmin, latmin, lonmax-lonmin, latmax-latmin);

	_uiHypocenter->labelVDistance->setText("");
	_uiHypocenter->labelVDistanceAutomatic->setText("");
	_uiHypocenter->labelFrameInfoSpacer->setText("");
	_uiHypocenter->labelFrameInfoSpacer->setFixedWidth(16);
	_uiHypocenter->fmLabelFrameInfoSpacer->setText("");
	_uiHypocenter->fmLabelFrameInfoSpacer->setFixedWidth(16);
	_uiHypocenter->labelFMSeparator->setText("\n\nFocalMechanism\n");

	_uiHypocenter->fmLabelVDistance->setText("");
	_uiHypocenter->fmLabelVDistanceAutomatic->setText("");

	if ( !withBorders ) {
		QFontMetrics fm(SCScheme.fonts.normal);
		QRect rect = fm.boundingRect('M');

		_ui->frameVDistance->setFixedHeight(rect.height()*2);
		_ui->frameHDistance->setFixedWidth(rect.width());
		_ui->frameMagnitudeDistance->setFixedHeight(rect.height());

		DISABLE_FRAME(_ui->frameRegion);
		DISABLE_FRAME(_ui->frameMagnitudes);
		DISABLE_FRAME(_uiHypocenter->frameInformation);
		DISABLE_FRAME(_uiHypocenter->frameInformationAutomatic);
		DISABLE_FRAME(_uiHypocenter->fmFrameInformation);
		DISABLE_FRAME(_uiHypocenter->fmFrameInformationAutomatic);
		DISABLE_FRAME(_ui->frameProcessing);
		DISABLE_FRAME(_ui->framePlugable);
	}
	else {
		_ui->frameVDistance->setVisible(false);
		_ui->frameHDistance->setVisible(false);
		_ui->frameMagnitudeDistance->setVisible(false);

		ENABLE_FRAME(_ui->frameRegion);
		ENABLE_FRAME(_ui->frameMagnitudes);
		ENABLE_FRAME(_uiHypocenter->frameInformation);
		ENABLE_FRAME(_uiHypocenter->frameInformationAutomatic);
		ENABLE_FRAME(_uiHypocenter->fmFrameInformation);
		ENABLE_FRAME(_uiHypocenter->fmFrameInformationAutomatic);
		ENABLE_FRAME(_ui->frameProcessing);
	}

	_ui->_lbOriginTime->setFont(SCScheme.fonts.heading1);
	_ui->_lbOriginTimeAutomatic->setFont(SCScheme.fonts.heading1);
	_ui->_lbTimeAgo->setFont(SCScheme.fonts.heading2);
	_ui->_lbRegion->setFont(SCScheme.fonts.highlight);
	_ui->_lbRegion->installEventFilter(drawFilter);
	_ui->_lbRegionExtra->setFont(SCScheme.fonts.normal);
	_ui->_lbRegionExtra->installEventFilter(drawFilter);

	_ui->_lbPreMagType->setFont(SCScheme.fonts.heading1);
	_ui->_lbPreMagVal->setFont(SCScheme.fonts.heading1);
	_ui->labelDepth->setFont(SCScheme.fonts.heading1);
	_ui->labelCustomName->setFont(SCScheme.fonts.heading1);
	_ui->labelCustomValue->setFont(SCScheme.fonts.heading1);
	_ui->labelCustomValue->installEventFilter(drawFilter);

	_uiHypocenter->_lbLatitudeTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbLatitude->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLatitudeUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLatError->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbLongitudeTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbLongitude->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLongitudeUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLongError->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbDepthTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbDepth->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbDepthUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbDepthError->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbNoPhasesTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbNoPhases->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbRMSTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbGapTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbComment->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbCommentTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbRMS->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbAzGap->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbAgencyTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbAgency->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbOriginStatusTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbOriginStatus->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbFirstLocTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbFirstLocation->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbThisLocTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbThisLocation->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbEventIDTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbEventID->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbEventID->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
	_uiHypocenter->_lbEventID->installEventFilter(drawFilter);

	_uiHypocenter->_lbLatitudeAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLatitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLatErrorAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbLongitudeAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLongitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbLongErrorAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbDepthAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbDepthUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->_lbDepthErrorAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbNoPhasesAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbRMSAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbAzGapAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbCommentAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbAgencyAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->_lbOriginStatusAutomatic->setFont(SCScheme.fonts.normal);

	_uiHypocenter->labelFMSeparator->setFont(SCScheme.fonts.highlight);

	_uiHypocenter->labelLatitudeTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelLatitude->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLatitudeUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLatitudeError->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelLongitudeTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelLongitude->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLongitudeUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLongitudeError->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelDepthTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelDepth->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelDepthUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelDepthError->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMwTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMw->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelMomentTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMoment->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelMomentUnit->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelPhasesTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelPhases->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMisfitTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMisfit->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelCLVDTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelCLVD->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMinDistTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMinDist->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMinDistUnit->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMaxDistTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMaxDist->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMaxDistUnit->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelNPTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelNP0->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelNP1->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelTypeTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelType->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelAgencyTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelAgency->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelStatusTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelStatus->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelThisSolutionTxt->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelThisSolution->setFont(SCScheme.fonts.normal);

	_uiHypocenter->labelLatitudeAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLatitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLatitudeErrorAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelLongitudeAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLongitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelLongitudeErrorAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelDepthAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelDepthUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelDepthErrorAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMwAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelMomentAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelMomentUnitAutomatic->setFont(SCScheme.fonts.highlight);
	_uiHypocenter->labelPhasesAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMisfitAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelCLVDAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMinDistAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMinDistUnitAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMaxDistAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelMaxDistUnitAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelNP0Automatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelNP1Automatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelTypeAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelAgencyAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelStatusAutomatic->setFont(SCScheme.fonts.normal);
	_uiHypocenter->labelThisSolutionAutomatic->setFont(SCScheme.fonts.normal);

	_displayEventComment = false;
	try {
		_displayEventCommentID = SCApp->configGetString("display.event.comment.id");
		_displayEventComment = true;
	}
	catch ( ... ) {}

	try {
		_displayEventCommentDefault = SCApp->configGetString("display.event.comment.default");
	}
	catch ( ... ) {
		_displayEventCommentDefault = _ui->labelCustomValue->text().toStdString();
	}

	try {
		_ui->labelCustomName->setText(QString("%1").arg(SCApp->configGetString("display.event.comment.label").c_str()));
	}
	catch ( ... ) {
		_ui->labelCustomName->setText(_displayEventCommentID.c_str());
	}

	if ( !_displayEventComment ) {
		_ui->labelCustomName->setVisible(false);
		_ui->labelCustomValue->setVisible(false);
	}

	_displayComment = false;
	try {
		_displayCommentID = SCApp->configGetString("display.origin.comment.id");
		_displayComment = true;
	}
	catch ( ... ) {}

	_uiHypocenter->_lbCommentTxt->setVisible(_displayComment);
	_uiHypocenter->_lbComment->setVisible(_displayComment);
	_uiHypocenter->_lbCommentAutomatic->setVisible(_displayComment);

	try {
		_displayCommentDefault = SCApp->configGetString("display.origin.comment.default");
	}
	catch ( ... ) {
		_displayCommentDefault = _uiHypocenter->_lbComment->text().toStdString();
	}

	try {
		_uiHypocenter->_lbCommentTxt->setText(QString("%1:").arg(SCApp->configGetString("display.origin.comment.label").c_str()));
	}
	catch ( ... ) {
		_uiHypocenter->_lbCommentTxt->setText(_displayCommentID.c_str());
	}

	_maxHotspotDist = 20.0;
	_minHotspotPopulation = 50000;
	_recenterMap = true;

	_showLastAutomaticSolution = false;
	_showOnlyMostRecentEvent = true;
	_ui->btnSwitchToAutomatic->setVisible(false);

	try { _maxHotspotDist = SCApp->configGetDouble("poi.maxDist"); } catch ( ... ) {}
	try { _hotSpotDescriptionPattern = SCApp->configGetString("poi.message"); } catch ( ... ) {}
	try { _minHotspotPopulation = SCApp->configGetDouble("poi.minPopulation"); } catch ( ... ) {}
	try { _showLastAutomaticSolution = SCApp->configGetBool("showLastAutomaticSolution"); } catch ( ... ) {}
	try { _showOnlyMostRecentEvent = SCApp->configGetBool("showOnlyMostRecentEvent"); } catch ( ... ) {}
	try { _recenterMapConfig = SCApp->configGetBool("recenterMap"); } catch ( ... ) {
		_recenterMapConfig = true;
	}
	try {
		if ( SCApp->configGetBool("enableFixAutomaticSolutions") )
			_ui->btnSwitchToAutomatic->setVisible(true);
	}
	catch ( ... ) {}

	_maxMinutesSecondDisplay = -1;
	try { _maxMinutesSecondDisplay = SCApp->configGetInt("displayAgoSecondsUpToMaximumMinutes"); }
	catch ( ... ) {}

	try {
		std::vector<std::string> mags = SCApp->configGetStrings("visibleMagnitudes");
		std::copy(mags.begin(), mags.end(), std::inserter(_visibleMagnitudes, _visibleMagnitudes.end()));
	} catch ( ... ) {
		_visibleMagnitudes.insert("M");
		_visibleMagnitudes.insert("MLv");
		_visibleMagnitudes.insert("mb");
		_visibleMagnitudes.insert("mB");
		_visibleMagnitudes.insert("Mw(mB)");
	}

	try {
		_ui->btnPlugable0->setText(SCApp->configGetString("button0").c_str());
	} catch ( ... ) {}

	try {
		_ui->btnPlugable1->setText(SCApp->configGetString("button1").c_str());
	} catch ( ... ) {}

	_ui->btnPlugable0->setVisible(false);
	_ui->btnPlugable1->setVisible(false);

	_map = new OriginLocatorMap(_maptree.get(), _ui->frameMap);
	_map->setStationsInteractive(false);
	_map->setMouseTracking(true);
	_map->canvas().displayRect(displayRect);

	QHBoxLayout* hboxLayout = new QHBoxLayout(_ui->frameMap);
	hboxLayout->setObjectName("hboxLayoutMap");
	hboxLayout->setSpacing(6);
	hboxLayout->setMargin(0);
	hboxLayout->addWidget(_map);

	QAction* refreshAction = new QAction(this);
	refreshAction->setObjectName(QString::fromUtf8("refreshAction"));

	refreshAction->setShortcut(QApplication::translate("EventSummaryView", "F5", 0));
	addAction(refreshAction);

	_magList = new MagList();
	_ui->frameMagnitudes->layout()->addWidget(_magList);

	_currentEvent = DataModel::Event::Create("nullptr");

//	QFont f(_uiHypocenter->_lbSystem->font());
//	f.setBold(true);
//	_uiHypocenter->_lbSystem->setFont(f);

	_autoSelect = true;

	addAction(_ui->actionShowInvisibleMagnitudes);

	connect(_ui->actionShowInvisibleMagnitudes, SIGNAL(triggered(bool)),
	        this, SLOT(showVisibleMagnitudes(bool)));

	connect(_ui->btnShowDetails, SIGNAL(clicked()), this, SIGNAL(toolButtonPressed()));
	connect(_ui->btnSwitchToAutomatic, SIGNAL(clicked()), this, SLOT(switchToAutomaticPressed()));
	connect(_ui->btnPlugable0, SIGNAL(clicked()), this, SLOT(runScript0()));
	connect(_ui->btnPlugable1, SIGNAL(clicked()), this, SLOT(runScript1()));

	connect(refreshAction, SIGNAL(triggered(bool)), this,  SLOT(updateEvent()));

	QTimer* TimeAgoTimer = new QTimer();
	connect(TimeAgoTimer, SIGNAL(timeout()), this, SLOT(updateTimeAgoLabel()));
	TimeAgoTimer->start(1000);

	_ui->_lbPreMagType->setText(" --");

	_ui->btnShowDetails->setEnabled(false);
	_ui->btnPlugable0->setEnabled(false);
	_ui->btnPlugable1->setEnabled(false);

	_mapTimer = new QTimer(this);
	_mapTimer->setSingleShot(true);

	setInteractiveEnabled(!SCApp->nonInteractive());

	setLastAutomaticOriginColor(_automaticOriginDisabledColor);
	setLastAutomaticFMColor(_automaticOriginDisabledColor);
	setLastAutomaticOriginVisible(_showLastAutomaticSolution);
	clearOriginParameter();
}


void EventSummaryView::setToolButtonText(const QString& text) {
	_ui->btnShowDetails->setText(text);
}


void EventSummaryView::setScript0(const std::string& script, bool oldStyle,
                                  bool exportMap) {
	_script0 = script;
	_scriptStyle0 = oldStyle;
	_scriptExportMap0 = exportMap;
	bool visible0 = !_script0.empty() && _interactive;
	bool visible1 = !_script1.empty() && _interactive;
	_ui->btnPlugable0->setVisible(visible0);
	_ui->framePlugable->setVisible(visible0 || visible1);
}


void EventSummaryView::setScript1(const std::string& script, bool oldStyle,
                                  bool exportMap) {
	_script1 = script;
	_scriptStyle1 = oldStyle;
	_scriptExportMap1 = exportMap;
	bool visible0 = !_script0.empty() && _interactive;
	bool visible1 = !_script1.empty() && _interactive;
	_ui->btnPlugable1->setVisible(visible1);
	_ui->framePlugable->setVisible(visible0 || visible1);
}


OriginLocatorMap* EventSummaryView::map() const {
	return _map;
}


bool EventSummaryView::ignoreOtherEvents() const {
	return _ignoreOtherEvents;
}


Seiscomp::DataModel::Event* EventSummaryView::currentEvent() const {
	return _currentEvent.get();
}

Seiscomp::DataModel::Origin* EventSummaryView::currentOrigin() const {

	if (_currentOrigin)
		return _currentOrigin.get();
	else
		return nullptr;
}

Seiscomp::DataModel::Magnitude* EventSummaryView::currentMagnitude() const {

	if (_currentNetMag)
		return _currentNetMag.get();
	else
		return nullptr;
}


void EventSummaryView::updateEvent(){

	if (!_reader) return;

	if (_currentEvent){
		emit showInStatusbar(QString("update event received: %1").arg(_currentEvent->publicID().c_str()), 1000);

		setOriginParameter(_currentEvent->preferredOriginID());
		setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
	}
}


void EventSummaryView::addObject(const QString& parentID, Seiscomp::DataModel::Object* obj){
	DataModel::EventPtr event = DataModel::Event::Cast(obj);
	if ( event ) {
		if ( _ignoreOtherEvents ) {
			try {
				if ( event->type() == DataModel::OTHER_EVENT ||
					event->type() == DataModel::NOT_EXISTING ) {
					emit showInStatusbar(QString("filtered new event (type: '%1'): %2")
					                     .arg(event->type().toString())
					                     .arg(event->publicID().c_str()), 10000);
					return;
				}
			}
			catch ( ... ) {}
		}

		emit showInStatusbar(QString("new event received: %1").arg(event->publicID().c_str()), 10000);

		if ( _autoSelect ) {
			if ( checkAndDisplay(event.get()) ) {
				_mapTimer->stop();
			}
		}
		else {
			emit showInStatusbar(QString("a new event has arrived: %1 [event displayed is %2]")
			                     .arg(event->publicID().c_str()).arg(_currentEvent->publicID().c_str()), 10000);
		}
		return;
	}

	DataModel::MagnitudePtr netMag = DataModel::Magnitude::Cast(obj);
	if ( netMag ) {
		if ( !_currentOrigin ) return;

		if ( parentID.toStdString() == _currentOrigin->publicID() ) {
			Magnitude* mag = Magnitude::Find(netMag->publicID());
			if ( mag ) {
				bool visibleType = (_visibleMagnitudes.find(mag->type()) != _visibleMagnitudes.end()) || _ui->actionShowInvisibleMagnitudes->isChecked();
				_magList->addMag(mag, false, visibleType);
				if ( visibleType ) {
					QString display;
					try {
						display =
							QString("%1 %2 (%3)")
								.arg(mag->type().c_str())
								.arg(mag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude)
								.arg(mag->stationCount());
					}
					catch ( ... ) {
						display =
							QString("%1 %2 (-)")
								.arg(mag->type().c_str())
								.arg(mag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude);
					}
				}
			}
			else
				SEISCOMP_DEBUG("Could not find NetMag %s in Origin %s",
				               netMag->publicID().c_str(), _currentOrigin->publicID().c_str());
		}
		else if ( _showLastAutomaticSolution && _lastAutomaticOrigin && parentID.toStdString() == _lastAutomaticOrigin->publicID() ) {
			Magnitude* mag = Magnitude::Find(netMag->publicID());
			if ( mag ) {
				MagRow *row = _magList->row(mag->type());
				if ( row ) row->setReferenceMagnitude(mag);
			}
			else
				SEISCOMP_DEBUG("Could not find NetMag %s in Origin %s",
				               netMag->publicID().c_str(), _currentOrigin->publicID().c_str());
		}

		return;
	}

	DataModel::OriginReferencePtr oref = DataModel::OriginReference::Cast(obj);
	if ( oref ) {
		if ( !_showLastAutomaticSolution ) return;
		if ( _currentEvent && parentID.toStdString() == _currentEvent->publicID() ) {
			OriginPtr o = Origin::Find(oref->originID());
			if (!o && _reader)
				o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), oref->originID()));

			if ( o ) {
				if ( updateLastAutomaticOrigin(o.get()) )
					setAutomaticOrigin(_lastAutomaticOrigin.get());
			}
		}
		return;
	}

	DataModel::FocalMechanismReferencePtr fmref = DataModel::FocalMechanismReference::Cast(obj);
	if ( fmref ) {
		if ( !_showLastAutomaticSolution ) return;
		if ( _currentEvent && parentID.toStdString() == _currentEvent->publicID() ) {
			FocalMechanismPtr fm = FocalMechanism::Find(fmref->focalMechanismID());
			if (!fm && _reader)
				fm = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), fmref->focalMechanismID()));

			if ( fm ) {
				if ( updateLastAutomaticFM(fm.get()) ) {
					setAutomaticFM(_lastAutomaticFocalMechanism.get());
					updateMap(false);
				}
			}
		}
		return;
	}

	DataModel::CommentPtr comment = DataModel::Comment::Cast(obj);
	if ( comment ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() )
			updateEventComment();
		return;
	}

	DataModel::EventDescriptionPtr ed = DataModel::EventDescription::Cast(obj);
	if ( ed ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() && ed->type() == EARTHQUAKE_NAME )
			updateEventName();
		return;
	}
}


void EventSummaryView::updateObject(const QString& parentID, Seiscomp::DataModel::Object* obj) {
	DataModel::EventPtr event = DataModel::Event::Cast(obj);
	if ( event ) {
		if ( _ignoreOtherEvents ) {
			try {
				if ( event->type() == DataModel::OTHER_EVENT ||
					event->type() == DataModel::NOT_EXISTING ) {

					// Remove current event and set the last one
					if ( _currentEvent && event->publicID() == _currentEvent->publicID() ) {
						_currentEvent = nullptr;
						emit requestNonFakeEvent();
						if ( _currentEvent == nullptr )
							showEvent(nullptr, nullptr);
					}

					emit showInStatusbar(QString("filtered new event (type: '%1'): %2")
					                     .arg(event->type().toString())
					                     .arg(event->publicID().c_str()), 10000);
					return;
				}
			}
			catch ( ... ) {}
		}

		emit showInStatusbar(QString("event update received: %1").arg(event->publicID().c_str()), 10000);

		if ( _autoSelect ) {
			EventPtr registeredEvent = Event::Find(event->publicID());
			if ( registeredEvent )
				event = registeredEvent;

			checkAndDisplay(event.get());
		}
		else if ( _currentEvent ) {
			if ( event->publicID() == _currentEvent->publicID() ) {
				//setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
				processEventMsg(event.get());
			}
			else
				emit showInStatusbar(QString("an event update has arrived: %1 [event displayed is %2]")
				                     .arg(event->publicID().c_str()).arg(_currentEvent->publicID().c_str()), 60000);
		}
		return;
	}

	DataModel::MagnitudePtr netMag = DataModel::Magnitude::Cast(obj);
	if ( netMag ) {
		if ( _currentOrigin && parentID == _currentOrigin->publicID().c_str() ) {
			updateMagnitude(netMag.get());
			if ( _currentEvent && netMag->publicID() == _currentEvent->preferredMagnitudeID() )
				setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
		}
		else if ( _showLastAutomaticSolution && _lastAutomaticOrigin && parentID.toStdString() == _lastAutomaticOrigin->publicID() ) {
			_magList->updateReferenceMag(netMag.get());
		}
		return;
	}

	DataModel::OriginPtr origin = DataModel::Origin::Cast(obj);
	if ( origin ) {
		if ( !_showLastAutomaticSolution ) return;
		if ( _lastAutomaticOrigin && origin->publicID() == _lastAutomaticOrigin->publicID() ) {
			// Origin status changed -> lookup a new last automatic origin
			try {
				if ( origin->evaluationMode() != AUTOMATIC ) {
					_lastAutomaticOrigin = nullptr;
					for ( size_t i = 0; i < _currentEvent->originReferenceCount(); ++i ) {
						OriginReference *ref = _currentEvent->originReference(i);
						OriginPtr o = Origin::Find(ref->originID());
						if ( !o && _reader )
							o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), ref->originID()));

						if ( !o ) continue;

						updateLastAutomaticOrigin(o.get());
					}

					setAutomaticOrigin(_lastAutomaticOrigin.get());
				}
			}
			catch ( ... ) {}
		}
		return;
	}

	DataModel::CommentPtr comment = DataModel::Comment::Cast(obj);
	if ( comment ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() )
			updateEventComment();
	}

	DataModel::EventDescriptionPtr ed = DataModel::EventDescription::Cast(obj);
	if ( ed ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() && ed->type() == EARTHQUAKE_NAME )
			updateEventName();
		return;
	}
}


void EventSummaryView::removeObject(const QString &parentID, Seiscomp::DataModel::Object *obj) {
	DataModel::EventPtr event = DataModel::Event::Cast(obj);
	if ( event ) {
		// Remove current event and set the last one
		if ( _currentEvent && event->publicID() == _currentEvent->publicID() ) {
			_currentEvent = nullptr;
			emit requestNonFakeEvent();
			if ( _currentEvent == nullptr )
				showEvent(nullptr, nullptr);
		}

		emit showInStatusbar(QString("event %1 removed")
		                     .arg(event->publicID().c_str()), 10000);
		return;
	}
}


void EventSummaryView::showEvent(Seiscomp::DataModel::Event* event, Seiscomp::DataModel::Origin* org){
	if ( event )
		emit showInStatusbar(QString("selected event: %1").arg(event->publicID().c_str()), 1000);
	else
		_currentEvent = DataModel::Event::Create("nullptr");

// 	clearLastMagnitudes();
	_mapTimer->stop();
	processEventMsg(event, org);
}


void EventSummaryView::showOrigin(Seiscomp::DataModel::Origin* origin){

	emit showInStatusbar(QString("selected origin: %1").arg(origin->publicID().c_str()), 1000);

// 	clearLastMagnitudes();
	_mapTimer->stop();

	processEventMsg(_currentEvent.get(), origin);
}


void EventSummaryView::processEventMsg(DataModel::Event* event, Seiscomp::DataModel::Origin* org){
	if ( event == nullptr ) {
		_currentOrigin = nullptr;
		_currentFocalMechanism = nullptr;
		_lastAutomaticOrigin = nullptr;
		_lastAutomaticFocalMechanism = nullptr;
		clearOriginParameter();
		clearMagnitudeParameter();
		clearMap();
		updateTimeAgoLabel();
		return;
	}

	if ( event->preferredOriginID().empty() ) return;

	bool changedEvent = true;
	if ( event && _currentEvent )
		changedEvent = _currentEvent->publicID() != event->publicID();
	else
		changedEvent = true;

	_recenterMap = changedEvent;

	_currentEvent = event;

	if ( _currentEvent ) {
		SEISCOMP_DEBUG("pe  publicID: %s ", _currentEvent->publicID().c_str());
	}

	_currentFocalMechanism = FocalMechanism::Find(_currentEvent->preferredFocalMechanismID());
	if ( !_currentFocalMechanism && _reader )
		_currentFocalMechanism = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), _currentEvent->preferredFocalMechanismID()));
	if ( _currentFocalMechanism && _reader )
		_reader->loadMomentTensors(_currentFocalMechanism.get());


	if ( org )
		setOrigin(org);
	else
		setOriginParameter(_currentEvent->preferredOriginID());

	if ( _showLastAutomaticSolution ) {
		if ( changedEvent ) {
			_lastAutomaticOrigin = nullptr;
			_lastAutomaticFocalMechanism = nullptr;
			if ( _showLastAutomaticSolution && _reader ) {
				DatabaseIterator it = _reader->getOriginsDescending(_currentEvent->publicID());
				for ( ; *it; ++it ) {
					OriginPtr o = Origin::Cast(*it);
					if ( updateLastAutomaticOrigin(o.get()) )
						break;
				}
				it.close();

				it = _reader->getFocalMechanismsDescending(_currentEvent->publicID());
				for ( ; *it; ++it ) {
					FocalMechanismPtr fm = FocalMechanism::Cast(*it);
					if ( updateLastAutomaticFM(fm.get()) )
						break;
				}
				it.close();
			}
		}

		setAutomaticOrigin(_lastAutomaticOrigin.get());
		setAutomaticFM(_lastAutomaticFocalMechanism.get());
		updateMap(false);
	}

	if ( _currentEvent )
		setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
	else
		setPrefMagnitudeParameter("");

	updateEventComment();
	updateEventName();
}


void EventSummaryView::updateEventComment() {
	_ui->labelCustomValue->setText(_displayEventCommentDefault.c_str());
	_ui->labelCustomValue->setToolTip("- no information available -");

	if ( !_currentEvent ) return;
	if ( !_displayEventComment ) return;

	if ( _reader && _currentEvent->commentCount() == 0 )
		_reader->loadComments(_currentEvent.get());

	for ( size_t i = 0; i < _currentEvent->commentCount(); ++i ) {
		if ( _currentEvent->comment(i)->id() == _displayEventCommentID ) {
			if ( !_currentEvent->comment(i)->text().empty() ) {
				_ui->labelCustomValue->setText(_currentEvent->comment(i)->text().c_str());
				_ui->labelCustomValue->setToolTip(_currentEvent->comment(i)->text().c_str());
			}
			break;
		}
	}
}


void EventSummaryView::updateEventName() {
	if ( !_currentEvent ) return;

	return;

	if ( _reader && _currentEvent->eventDescriptionCount() == 0 )
		_reader->loadEventDescriptions(_currentEvent.get());

	for ( size_t i = 0; i < _currentEvent->eventDescriptionCount(); ++i ) {
		if ( _currentEvent->eventDescription(i)->type() == EARTHQUAKE_NAME ) {
			if ( !_currentEvent->eventDescription(i)->text().empty() ) {
				//_ui->_lbName->setText(_currentEvent->eventDescription(i)->text().c_str());
				return;
			}
			break;
		}
	}

	//_ui->_lbName->setText("-");
}


static void elapsedTimeString(const Core::TimeSpan &dt, QString &str)
{
	int d=0, h=0, m=0, s=0;
	QLatin1Char fill('0');
	dt.elapsedTime(&d, &h, &m, &s);
	if (d)
		str = QString("O.T. +%1d %2h").arg(d,2).arg(h, 2, 10, fill);
	else if (h)
		str = QString("O.T. +%1h %2m").arg(h,2).arg(m, 2, 10, fill);
	else
		str = QString("O.T. +%1m %2s").arg(m,2).arg(s, 2, 10, fill);
}


bool EventSummaryView::setOriginParameter(std::string OriginID){
	OriginPtr origin = Origin::Find(OriginID);
	if (!origin && _reader)
		origin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), OriginID));

	if ( origin == nullptr ){
		_currentOrigin = nullptr;
		_currentFocalMechanism = nullptr;
		SEISCOMP_DEBUG("scesv: setOriginParameter:  origin not found %s ", OriginID.c_str());
		clearOriginParameter();
		clearMap();
	 	clearMagnitudeParameter();
		return false;
	}

	if ( !origin->arrivalCount() && _reader )
		_reader->loadArrivals(origin.get());

	if ( !origin->magnitudeCount() && _reader ) {
		// We do not need the station magnitude references
		//_reader->loadMagnitudes(_currentOrigin.get());
		DatabaseIterator it = _reader->getObjects(origin.get(), Magnitude::TypeInfo());
		while ( *it ) {
			origin->add(Magnitude::Cast(*it));
			++it;
		}
	}

	setOrigin(origin.get());
	return true;
}


void EventSummaryView::setOrigin(Seiscomp::DataModel::Origin* origin) {
	_currentOrigin = origin;

	calcOriginDistances();

	std::string desc = description(_currentOrigin.get());
	if ( desc.empty() )
		//desc = _currentEvent->description();
		_ui->_lbRegionExtra->setVisible(false);
	else {
		_ui->_lbRegionExtra->setVisible(true);
		_ui->_lbRegionExtra->setText(desc.c_str());
	}

	_ui->_lbRegion->setVisible(true);
	std::string region = _currentEvent?eventRegion(_currentEvent.get()):"";
	if ( _currentEvent && !region.empty() )
		_ui->_lbRegion->setText(region.c_str());
	else {
		_ui->_lbRegion->setText(Regions::getRegionName(_currentOrigin->latitude(), _currentOrigin->longitude()).c_str());
	}

	updateTimeAgoLabel();

	// set "not preferred" magnitudes
	setMagnitudeParameter(_currentOrigin.get());

	// set origin parameters
	_uiHypocenter->_lbLatitude->setText(latitudeToString(_currentOrigin->latitude(), true, false, SCScheme.precision.location));
	_uiHypocenter->_lbLatitudeUnit->setText(latitudeToString(_currentOrigin->latitude(), false, true));

	_uiHypocenter->_lbLongitude->setText(longitudeToString(_currentOrigin->longitude(), true, false, SCScheme.precision.location));
	_uiHypocenter->_lbLongitudeUnit->setText(longitudeToString(_currentOrigin->longitude(), false, true));

	try { // depth error
		double err_z = quantityUncertainty(_currentOrigin->depth());
		if (err_z == 0.0)
			_uiHypocenter->_lbDepthError->setText(QString("  fixed"));
		else
			_uiHypocenter->_lbDepthError->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.uncertainties));
	}
	catch(...) {
		_uiHypocenter->_lbDepthError->setText(QString(""));
	}
	try { // depth
		_uiHypocenter->_lbDepth->setText(depthToString(_currentOrigin->depth(), SCScheme.precision.depth));
		_ui->labelDepth->setText(depthToString(_currentOrigin->depth(), SCScheme.precision.depth) + " km");
		_uiHypocenter->_lbDepthUnit->setText("km");

	} catch(...) {
		_uiHypocenter->_lbDepth->setText(QString("---"));
		_uiHypocenter->_lbDepthUnit->setText("");
		_uiHypocenter->_lbDepthError->setText(QString(""));
		_ui->labelDepth->setText("");
	}

	string timeFormat = "%F %T";
	if ( SCScheme.precision.originTime > 0 ) {
		timeFormat += ".%";
		timeFormat += Core::toString(SCScheme.precision.originTime);
		timeFormat += "f";
	}
	timeToLabel(_ui->_lbOriginTime, _currentOrigin->time().value(), timeFormat.c_str(), true);

	try {
		_uiHypocenter->_lbOriginStatus->setText(_currentOrigin->evaluationMode().toString());
	} catch(...){
		_uiHypocenter->_lbOriginStatus->setText("---");
	}

	try {
		_uiHypocenter->_lbLatError->setText(QString("+/-%1 km").arg(quantityUncertainty(_currentOrigin->latitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		_uiHypocenter->_lbLatError->setText("");
	}

	try {
		_uiHypocenter->_lbLongError->setText(QString("+/-%1 km").arg(quantityUncertainty(_currentOrigin->longitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		_uiHypocenter->_lbLongError->setText("");
	}

	try {
		DataModel::OriginQuality quality = _currentOrigin->quality();
		try{
			_uiHypocenter->_lbNoPhases->setText(QString("%1").arg(quality.usedPhaseCount(), 0, 'd', 0, ' '));
		}
		catch(Core::ValueException&) {
			_uiHypocenter->_lbNoPhases->setText("--");
		}

		try {
			_uiHypocenter->_lbRMS->setText(QString("%1").arg(quality.standardError(), 0, 'f', 1));
		}
		catch(Core::ValueException&) {
			_uiHypocenter->_lbRMS->setText("--");
		}

		try {
			_uiHypocenter->_lbAzGap->setText(QString("%1").arg(quality.azimuthalGap(), 0, 'f', 0));
		}
		catch(Core::ValueException&) {
			_uiHypocenter->_lbAzGap->setText("--");
		}
	}
	catch ( ... ) {
		_uiHypocenter->_lbNoPhases->setText("--");
		_uiHypocenter->_lbRMS->setText("--");
		_uiHypocenter->_lbAzGap->setText("--");
	}

	if ( _displayComment ) {
		if ( _reader && _currentOrigin->commentCount() == 0 )
			_reader->loadComments(_currentOrigin.get());

		_uiHypocenter->_lbComment->setText(_displayCommentDefault.c_str());
		for ( size_t i = 0; i < _currentOrigin->commentCount(); ++i ) {
			if ( _currentOrigin->comment(i)->id() == _displayCommentID ) {
				_uiHypocenter->_lbComment->setText(_currentOrigin->comment(i)->text().c_str());
				break;
			}
		}
	}

	try {
		_uiHypocenter->_lbAgency->setText(_currentOrigin->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		_uiHypocenter->_lbAgency->setText("");
	}

	// get the time of first location of an origin belonging to this Event
	QString str("-");
	try {
		if ( _currentEvent ) {
			Core::TimeSpan dt = _currentEvent->creationInfo().creationTime() - _currentOrigin->time().value();
			elapsedTimeString(dt, str);
		}
	}
	catch (...) {}
	_uiHypocenter->_lbFirstLocation->setText(str);

	str = "-";
	// get the time of the current location
	try {
		Core::TimeSpan dt = _currentOrigin->creationInfo().creationTime() - _currentOrigin->time().value();
		elapsedTimeString(dt, str);
	}
	catch (...) {}
	_uiHypocenter->_lbThisLocation->setText(str);

	if ( _currentEvent )
		_uiHypocenter->_lbEventID->setText(_currentEvent->publicID().c_str());
	else
		_uiHypocenter->_lbEventID->setText("");

	// set map
	if ( !_mapTimer->isActive() ){
		SEISCOMP_DEBUG("updating map ...");
		updateMap(true);
		_mapTimer->start(2000); //! minimum time between two map updates
		disconnect (_mapTimer, 0, this, 0);
	}
	else{
		SEISCOMP_WARNING("updating map deferred!");
		connect (_mapTimer, SIGNAL(timeout()), this, SLOT(deferredMapUpdate()));
	}


	if ( _currentFocalMechanism ) {
		setFMParametersVisible(true);
		setFM(_currentFocalMechanism.get());
		setAutomaticFM(_currentFocalMechanism.get());
	}
	else
		setFMParametersVisible(false);


	_ui->btnShowDetails->setEnabled(true);
	_ui->btnPlugable0->setEnabled(true);
	_ui->btnPlugable1->setEnabled(true);

	SEISCOMP_DEBUG("***** Setting origin %s", _currentOrigin->publicID().c_str());
}


void EventSummaryView::setAutomaticOrigin(DataModel::Origin* origin) {
	if ( origin == nullptr ) {
		clearAutomaticOriginParameter();
		return;
	}

	if ( origin->magnitudeCount() == 0 && _reader )
		_reader->loadMagnitudes(origin);

	if ( _currentOrigin && _currentOrigin->publicID() == origin->publicID() ) {
		_ui->btnSwitchToAutomatic->setEnabled(false);
		setLastAutomaticOriginColor(_automaticOriginDisabledColor);
	}
	else {
		_ui->btnSwitchToAutomatic->setEnabled(true);
		setLastAutomaticOriginColor(_automaticOriginEnabledColor);
	}

	setAutomaticMagnitudeParameter(origin);

	// set origin parameters
	_uiHypocenter->_lbLatitudeAutomatic->setText(latitudeToString(origin->latitude(), true, false, SCScheme.precision.location));
	_uiHypocenter->_lbLatitudeUnitAutomatic->setText(latitudeToString(origin->latitude(), false, true));

	_uiHypocenter->_lbLongitudeAutomatic->setText(longitudeToString(origin->longitude(), true, false, SCScheme.precision.location));
	_uiHypocenter->_lbLongitudeUnitAutomatic->setText(longitudeToString(origin->longitude(), false, true));

	try { // depth error
		double err_z = quantityUncertainty(origin->depth());
		if (err_z == 0.0)
			_uiHypocenter->_lbDepthErrorAutomatic->setText(QString("  fixed"));
		else
			_uiHypocenter->_lbDepthErrorAutomatic->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.uncertainties));
	}
	catch(...) {
		_uiHypocenter->_lbDepthErrorAutomatic->setText(QString(""));
	}
	try { // depth
		_uiHypocenter->_lbDepthAutomatic->setText(depthToString(origin->depth(), SCScheme.precision.depth));
		_uiHypocenter->_lbDepthUnitAutomatic->setText("km");

	} catch(...) {
		_uiHypocenter->_lbDepthAutomatic->setText(QString("---"));
		_uiHypocenter->_lbDepthUnitAutomatic->setText("");
		_uiHypocenter->_lbDepthErrorAutomatic->setText(QString(""));
	}

	string timeFormat = "%F %T";
	if ( SCScheme.precision.originTime > 0 ) {
		timeFormat += ".%";
		timeFormat += Core::toString(SCScheme.precision.originTime);
		timeFormat += "f";
	}
	timeToLabel(_ui->_lbOriginTimeAutomatic, origin->time().value(), timeFormat.c_str(), true);

	try {
		_uiHypocenter->_lbOriginStatusAutomatic->setText(origin->evaluationMode().toString());
	} catch(...){
		_uiHypocenter->_lbOriginStatusAutomatic->setText("---");
	}

	try {
		_uiHypocenter->_lbLatErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(origin->latitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		_uiHypocenter->_lbLatErrorAutomatic->setText("");
	}

	try {
		_uiHypocenter->_lbLongErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(origin->longitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		_uiHypocenter->_lbLongErrorAutomatic->setText("");
	}

	try {
		DataModel::OriginQuality quality = origin->quality();
		try{
			_uiHypocenter->_lbNoPhasesAutomatic->setText(QString("%1").arg(quality.usedPhaseCount(), 0, 'd', 0, ' '));
		}
		catch(Core::ValueException&) {
			_uiHypocenter->_lbNoPhasesAutomatic->setText("--");
		}

		try {
			_uiHypocenter->_lbRMSAutomatic->setText(QString("%1").arg(quality.standardError(), 0, 'f', 1));
		}
		catch(Core::ValueException&) {
			_uiHypocenter->_lbRMSAutomatic->setText("--");
		}

		try {
			_uiHypocenter->_lbAzGapAutomatic->setText(QString("%1").arg(quality.azimuthalGap(), 0, 'f', 0));
		}
		catch(Core::ValueException&) {
			_uiHypocenter->_lbAzGapAutomatic->setText("--");
		}
	}
	catch ( ... ) {
		_uiHypocenter->_lbNoPhasesAutomatic->setText("--");
		_uiHypocenter->_lbRMSAutomatic->setText("--");
		_uiHypocenter->_lbAzGapAutomatic->setText("--");
	}


	if ( _displayComment ) {
		if ( _reader && origin->commentCount() == 0 )
			_reader->loadComments(origin);

		_uiHypocenter->_lbCommentAutomatic->setText(_displayCommentDefault.c_str());
		for ( size_t i = 0; i < origin->commentCount(); ++i ) {
			if ( origin->comment(i)->id() == _displayCommentID ) {
				_uiHypocenter->_lbCommentAutomatic->setText(origin->comment(i)->text().c_str());
				break;
			}
		}
	}


	try {
		_uiHypocenter->_lbAgencyAutomatic->setText(origin->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		_uiHypocenter->_lbAgencyAutomatic->setText("");
	}
}


void EventSummaryView::setFM(DataModel::FocalMechanism *fm) {
	OriginPtr derivedOrigin = nullptr;

	try {
		_uiHypocenter->labelMisfit->setText(QString("%1").arg(fm->misfit(), 0, 'f', 2));
	}
	catch ( ... ) {
		_uiHypocenter->labelMisfit->setText("-");
	}

	try {
		_uiHypocenter->labelNP0->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane1().strike())
			.arg((int)fm->nodalPlanes().nodalPlane1().dip())
			.arg((int)fm->nodalPlanes().nodalPlane1().rake()));
	}
	catch ( ... ) {
		_uiHypocenter->labelNP0->setText("S: -, D: -, R: -");
	}

	try {
		_uiHypocenter->labelNP1->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane2().strike())
			.arg((int)fm->nodalPlanes().nodalPlane2().dip())
			.arg((int)fm->nodalPlanes().nodalPlane2().rake()));
	}
	catch ( ... ) {
		_uiHypocenter->labelNP1->setText("S: -, D: -, R: -");
	}

	try {
		_uiHypocenter->labelAgency->setText(fm->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		_uiHypocenter->labelAgency->setText("-");
	}

	try {
		_uiHypocenter->labelStatus->setText(fm->evaluationMode().toString());
	}
	catch ( ... ) {
		_uiHypocenter->labelStatus->setText("-");
	}

	QString str = "-";
	try {
		Core::TimeSpan dt = fm->creationInfo().creationTime() - _currentOrigin->time().value();
		elapsedTimeString(dt, str);
	}
	catch ( ... ) {}
	_uiHypocenter->labelThisSolution->setText(str);

	if ( fm->momentTensorCount() > 0 ) {
		MomentTensor *mt = fm->momentTensor(0);
		derivedOrigin = Origin::Find(mt->derivedOriginID());
		if (!derivedOrigin && _reader)
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), mt->derivedOriginID()));

		try {
			_uiHypocenter->labelCLVD->setText(QString("%1").arg(mt->clvd(), 0, 'f', 2));
		}
		catch ( ... ) {
			_uiHypocenter->labelCLVD->setText("-");
		}

		MagnitudePtr mag = Magnitude::Find(mt->momentMagnitudeID());
		if ( !mag && _reader)
			mag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), mt->momentMagnitudeID()));

		if ( mag )
			_uiHypocenter->labelMw->setText(QString("%1").arg(mag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude));
		else
			_uiHypocenter->labelMw->setText("-");

		try {
			_uiHypocenter->labelMoment->setText(QString("%1").arg(mt->scalarMoment(), 0, 'E', 2));
		}
		catch ( ... ) {
			_uiHypocenter->labelMoment->setText("-");
		}
	}
	else {
		_uiHypocenter->labelCLVD->setText("-");
		_uiHypocenter->labelMw->setText("-");
		_uiHypocenter->labelMoment->setText("-");

		derivedOrigin = Origin::Find(fm->triggeringOriginID());
	}

	if ( derivedOrigin ) {
		_uiHypocenter->labelLatitude->setText(latitudeToString(derivedOrigin->latitude(), true, false, SCScheme.precision.location));
		_uiHypocenter->labelLatitudeUnit->setText(latitudeToString(derivedOrigin->latitude(), false, true));

		_uiHypocenter->labelLongitude->setText(longitudeToString(derivedOrigin->longitude(), true, false, SCScheme.precision.location));
		_uiHypocenter->labelLongitudeUnit->setText(longitudeToString(derivedOrigin->longitude(), false, true));

		try {
			_uiHypocenter->labelLatitudeError->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->latitude()), 4, 'f', SCScheme.precision.depth));
		}
		catch ( ... ) {
			_uiHypocenter->labelLatitudeError->setText("");
		}

		try {
			_uiHypocenter->labelLongitudeError->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->longitude()), 4, 'f', SCScheme.precision.depth));
		}
		catch ( ... ) {
			_uiHypocenter->labelLongitudeError->setText("");
		}
		
		try { // depth error
			double err_z = quantityUncertainty(derivedOrigin->depth());
			if (err_z == 0.0)
				_uiHypocenter->labelDepthError->setText(QString("  fixed"));
			else
				_uiHypocenter->labelDepthError->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.depth));
		}
		catch ( ... ) {
			_uiHypocenter->labelDepthError->setText(QString(""));
		}

		try { // depth
			_uiHypocenter->labelDepth->setText(depthToString(derivedOrigin->depth(), SCScheme.precision.depth));
			_uiHypocenter->labelDepthUnit->setText("km");

		}
		catch ( ... ) {
			_uiHypocenter->labelDepth->setText(QString("---"));
			_uiHypocenter->labelDepthUnit->setText("");
			_uiHypocenter->labelDepthError->setText(QString(""));
		}

		try {
			_uiHypocenter->labelPhases->setText(
				QString("%1").arg(derivedOrigin->quality().usedPhaseCount()));
		}
		catch ( ... ) {
			_uiHypocenter->labelPhases->setText("-");
		}

		try {
			_uiHypocenter->labelType->setText(derivedOrigin->type().toString());
		}
		catch ( ... ) {
			_uiHypocenter->labelType->setText("-");
		}

		try {
			_uiHypocenter->labelMinDist->setText(
				QString("%1").arg(derivedOrigin->quality().minimumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			_uiHypocenter->labelMinDist->setText("-");
		}

		try {
			_uiHypocenter->labelMaxDist->setText(
				QString("%1").arg(derivedOrigin->quality().maximumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			_uiHypocenter->labelMaxDist->setText("-");
		}
	}
	else {
		_uiHypocenter->labelLatitude->setText("---.--");
		_uiHypocenter->labelLatitudeUnit->setText("");
		_uiHypocenter->labelLongitude->setText("---.--");
		_uiHypocenter->labelLongitudeUnit->setText("");
		_uiHypocenter->labelDepth->setText("---");
		_uiHypocenter->labelDepthUnit->setText("");
		_uiHypocenter->labelPhases->setText("-");
		_uiHypocenter->labelType->setText("-");
		_uiHypocenter->labelMinDist->setText("-");
		_uiHypocenter->labelMaxDist->setText("-");
	}
}


void EventSummaryView::clearAutomaticFMParameter() {
	setLastAutomaticFMColor(_automaticOriginDisabledColor);

	_uiHypocenter->labelMisfitAutomatic->setText("-");
	_uiHypocenter->labelNP0Automatic->setText("S: -, D: -, R: -");
	_uiHypocenter->labelNP1Automatic->setText("S: -, D: -, R: -");
	_uiHypocenter->labelAgencyAutomatic->setText("-");
	_uiHypocenter->labelStatusAutomatic->setText("-");
	_uiHypocenter->labelThisSolutionAutomatic->setText("-");
	_uiHypocenter->labelCLVDAutomatic->setText("-");
	_uiHypocenter->labelMwAutomatic->setText("-");
	_uiHypocenter->labelMomentAutomatic->setText("-");
	_uiHypocenter->labelLatitudeAutomatic->setText("---.--");
	_uiHypocenter->labelLatitudeUnitAutomatic->setText("");
	_uiHypocenter->labelLongitudeAutomatic->setText("---.--");
	_uiHypocenter->labelLongitudeUnitAutomatic->setText("");
	_uiHypocenter->labelDepthAutomatic->setText("---");
	_uiHypocenter->labelDepthUnitAutomatic->setText("");
	_uiHypocenter->labelPhasesAutomatic->setText("-");
	_uiHypocenter->labelTypeAutomatic->setText("-");
}


void EventSummaryView::setAutomaticFM(DataModel::FocalMechanism *fm) {
	if ( fm == nullptr ) {
		clearAutomaticFMParameter();
		return;
	}

	if ( _currentFocalMechanism && _currentFocalMechanism->publicID() == fm->publicID() )
		setLastAutomaticFMColor(_automaticOriginDisabledColor);
	else
		setLastAutomaticFMColor(_automaticOriginEnabledColor);

	if ( _reader && fm->momentTensorCount() == 0 )
		_reader->loadMomentTensors(fm);

	OriginPtr derivedOrigin = nullptr;

	try {
		_uiHypocenter->labelMisfitAutomatic->setText(QString("%1").arg(fm->misfit(), 0, 'f', 2));
	}
	catch ( ... ) {
		_uiHypocenter->labelMisfitAutomatic->setText("-");
	}

	try {
		_uiHypocenter->labelNP0Automatic->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane1().strike())
			.arg((int)fm->nodalPlanes().nodalPlane1().dip())
			.arg((int)fm->nodalPlanes().nodalPlane1().rake()));
	}
	catch ( ... ) {
		_uiHypocenter->labelNP0Automatic->setText("S: -, D: -, R: -");
	}

	try {
		_uiHypocenter->labelNP1Automatic->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane2().strike())
			.arg((int)fm->nodalPlanes().nodalPlane2().dip())
			.arg((int)fm->nodalPlanes().nodalPlane2().rake()));
	}
	catch ( ... ) {
		_uiHypocenter->labelNP1Automatic->setText("S: -, D: -, R: -");
	}

	try {
		_uiHypocenter->labelAgencyAutomatic->setText(fm->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		_uiHypocenter->labelAgencyAutomatic->setText("-");
	}

	try {
		_uiHypocenter->labelStatusAutomatic->setText(fm->evaluationMode().toString());
	}
	catch ( ... ) {
		_uiHypocenter->labelStatusAutomatic->setText("-");
	}

	QString str = "-";
	try {
		Core::TimeSpan dt = fm->creationInfo().creationTime() - _currentOrigin->time().value();
		elapsedTimeString(dt, str);
	}
	catch ( ... ) {}
	_uiHypocenter->labelThisSolutionAutomatic->setText(str);

	if ( fm->momentTensorCount() > 0 ) {
		MomentTensor *mt = fm->momentTensor(0);
		derivedOrigin = Origin::Find(mt->derivedOriginID());
		if (!derivedOrigin && _reader)
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), mt->derivedOriginID()));

		try {
			_uiHypocenter->labelCLVDAutomatic->setText(QString("%1").arg(mt->clvd(), 0, 'f', 2));
		}
		catch ( ... ) {
			_uiHypocenter->labelCLVDAutomatic->setText("-");
		}

		MagnitudePtr mag = Magnitude::Find(mt->momentMagnitudeID());
		if ( !mag && _reader)
			mag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), mt->momentMagnitudeID()));

		if ( mag )
			_uiHypocenter->labelMwAutomatic->setText(QString("%1").arg(mag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude));
		else
			_uiHypocenter->labelMwAutomatic->setText("-");

		try {
			_uiHypocenter->labelMomentAutomatic->setText(QString("%1").arg(mt->scalarMoment(), 0, 'E', 2));
		}
		catch ( ... ) {
			_uiHypocenter->labelMomentAutomatic->setText("-");
		}
	}
	else {
		_uiHypocenter->labelCLVDAutomatic->setText("-");
		_uiHypocenter->labelMwAutomatic->setText("-");
		_uiHypocenter->labelMomentAutomatic->setText("-");
	}

	if ( derivedOrigin ) {
		_uiHypocenter->labelLatitudeAutomatic->setText(latitudeToString(derivedOrigin->latitude(), true, false, SCScheme.precision.location));
		_uiHypocenter->labelLatitudeUnitAutomatic->setText(latitudeToString(derivedOrigin->latitude(), false, true));

		_uiHypocenter->labelLongitudeAutomatic->setText(longitudeToString(derivedOrigin->longitude(), true, false, SCScheme.precision.location));
		_uiHypocenter->labelLongitudeUnitAutomatic->setText(longitudeToString(derivedOrigin->longitude(), false, true));

		try {
			_uiHypocenter->labelLatitudeErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->latitude()), 4, 'f', SCScheme.precision.uncertainties));
		}
		catch ( ... ) {
			_uiHypocenter->labelLatitudeErrorAutomatic->setText("");
		}

		try {
			_uiHypocenter->labelLongitudeErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->longitude()), 4, 'f', SCScheme.precision.uncertainties));
		}
		catch ( ... ) {
			_uiHypocenter->labelLongitudeErrorAutomatic->setText("");
		}
		
		try { // depth error
			double err_z = quantityUncertainty(derivedOrigin->depth());
			if (err_z == 0.0)
				_uiHypocenter->labelDepthErrorAutomatic->setText(QString("  fixed"));
			else
				_uiHypocenter->labelDepthErrorAutomatic->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.uncertainties));
		}
		catch ( ... ) {
			_uiHypocenter->labelDepthErrorAutomatic->setText(QString(""));
		}

		try { // depth
			_uiHypocenter->labelDepthAutomatic->setText(depthToString(derivedOrigin->depth(), SCScheme.precision.depth));
			_uiHypocenter->labelDepthUnitAutomatic->setText("km");

		}
		catch ( ... ) {
			_uiHypocenter->labelDepthAutomatic->setText(QString("---"));
			_uiHypocenter->labelDepthUnitAutomatic->setText("");
			_uiHypocenter->labelDepthErrorAutomatic->setText(QString(""));
		}

		try {
			_uiHypocenter->labelPhasesAutomatic->setText(
				QString("%1").arg(derivedOrigin->quality().usedPhaseCount()));
		}
		catch ( ... ) {
			_uiHypocenter->labelPhasesAutomatic->setText("-");
		}

		try {
			_uiHypocenter->labelTypeAutomatic->setText(derivedOrigin->type().toString());
		}
		catch ( ... ) {
			_uiHypocenter->labelTypeAutomatic->setText("-");
		}

		try {
			_uiHypocenter->labelMinDistAutomatic->setText(
				QString("%1").arg(derivedOrigin->quality().minimumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			_uiHypocenter->labelMinDistAutomatic->setText("-");
		}

		try {
			_uiHypocenter->labelMaxDistAutomatic->setText(
				QString("%1").arg(derivedOrigin->quality().maximumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			_uiHypocenter->labelMaxDist->setText("-");
		}

	}
	else {
		_uiHypocenter->labelLatitudeAutomatic->setText("---.--");
		_uiHypocenter->labelLatitudeUnitAutomatic->setText("");
		_uiHypocenter->labelLongitudeAutomatic->setText("---.--");
		_uiHypocenter->labelLongitudeUnitAutomatic->setText("");
		_uiHypocenter->labelDepthAutomatic->setText("---");
		_uiHypocenter->labelDepthUnitAutomatic->setText("");
		_uiHypocenter->labelPhasesAutomatic->setText("-");
		_uiHypocenter->labelTypeAutomatic->setText("-");
		_uiHypocenter->labelMinDistAutomatic->setText("-");
		_uiHypocenter->labelMaxDistAutomatic->setText("-");
	}
}


void EventSummaryView::deferredMapUpdate(){

	disconnect (_mapTimer, 0, this, 0);
	SEISCOMP_DEBUG("processing deferred map update...");
	updateMap(true);
	_mapTimer->start(2000); //! minimum time between two map updates

}


void EventSummaryView::updateMap(bool realignView){
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if ( _currentOrigin && !_currentOrigin->arrivalCount() && _reader )
		_reader->loadArrivals(_currentOrigin.get());

	_map->setOrigin(_currentOrigin.get());

	if ( _currentOrigin && realignView ) {
		if ( _recenterMap && _recenterMapConfig ) {
			double radius = 30;
			try { radius = std::min(radius, _currentOrigin->quality().maximumDistance()+0.1); }
			catch ( ... ) {}
			_map->canvas().displayRect(QRectF(_currentOrigin->longitude()-radius, _currentOrigin->latitude()-radius, radius*2, radius*2));
		}
		else {
			if ( !_map->canvas().isVisible(_currentOrigin->longitude(), _currentOrigin->latitude()) ) {
				_map->canvas().setView(QPointF(_currentOrigin->longitude(), _currentOrigin->latitude()), _map->canvas().zoomLevel());
			}
		}
	}

	if ( _displayFocMechs ) {
		if ( _currentFocalMechanism )
			showFocalMechanism(_currentFocalMechanism.get(), -80, -80, palette().color(QPalette::WindowText));

		// Only show the last automatic solution if there is a preferred
		// solution and if both are different
		if ( _lastAutomaticFocalMechanism && _currentFocalMechanism &&
			 _lastAutomaticFocalMechanism != _currentFocalMechanism )
			showFocalMechanism(_lastAutomaticFocalMechanism.get(), 80, -80, _automaticOriginEnabledColor);
	}

	_map->update();

	QApplication::restoreOverrideCursor();
}


void EventSummaryView::showFocalMechanism(DataModel::FocalMechanism *fm,
                                          int ofsX, int ofsY, QColor borderColor) {
	Math::NODAL_PLANE np;
	Math::Tensor2Sd tensor;
	bool hasTensor = false;
	DataModel::OriginPtr derivedOrigin;

	QColor c = Qt::black;
	if ( fm->momentTensorCount() > 0 ) {
		DataModel::MomentTensor *mt = fm->momentTensor(0);

		if ( _enableFullTensor ) {
			try {
				tensor._33 = +mt->tensor().Mrr();
				tensor._11 = +mt->tensor().Mtt();
				tensor._22 = +mt->tensor().Mpp();
				tensor._13 = +mt->tensor().Mrt();
				tensor._23 = -mt->tensor().Mrp();
				tensor._12 = -mt->tensor().Mtp();

				hasTensor = true;
			}
			catch ( ... ) {}
		}

		derivedOrigin = DataModel::Origin::Find(mt->derivedOriginID());
		if ( !derivedOrigin && _reader )
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), mt->derivedOriginID()));
	}
	else {
		derivedOrigin = DataModel::Origin::Find(fm->triggeringOriginID());
		if ( !derivedOrigin && _reader )
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), fm->triggeringOriginID()));
	}

	if ( derivedOrigin ) {
		try {
			float depth = derivedOrigin->depth().value();

			if ( depth < 50 )
				c = Qt::red;
			else if ( depth < 100 )
				c = QColor(255, 165, 0);
			else if ( depth < 250 )
				c = Qt::yellow;
			else if ( depth < 600 )
				c = Qt::green;
			else
				c = Qt::blue;
		}
		catch ( ... ) {}
	}

	try {
		if ( !hasTensor ) {
			np.str = fm->nodalPlanes().nodalPlane1().strike();
			np.dip = fm->nodalPlanes().nodalPlane1().dip();
			np.rake = fm->nodalPlanes().nodalPlane1().rake();

			Math::np2tensor(np, tensor);
		}
	}
	catch ( ... ) {
		return;
	}

	try {
		TensorSymbol *symbol = new TensorSymbol(tensor);
		symbol->setSize(QSize(64, 64));
		if ( derivedOrigin )
			symbol->setPosition(QPointF(derivedOrigin->longitude(), derivedOrigin->latitude()));
		else
			symbol->setPosition(QPointF(_currentOrigin->longitude(), _currentOrigin->latitude()));
		symbol->setOffset(QPoint(ofsX, ofsY));
		symbol->setPriority(Map::Symbol::HIGH);
		symbol->setShadingEnabled(true);
		symbol->setTColor(c);
		symbol->setBorderColor(borderColor);
		_map->canvas().symbolCollection()->add(symbol);
	}
	catch ( ... ) {}
}


void EventSummaryView::updateMagnitude(DataModel::Magnitude *mag) {
	_magList->updateMag(mag);
}


void EventSummaryView::setPrefMagnitudeParameter(std::string MagnitudeID){
	DataModel::MagnitudePtr Magnitude = Magnitude::Find(MagnitudeID);
	if (!Magnitude && _reader)
		Magnitude = DataModel::Magnitude::Cast(_reader->getObject(DataModel::Magnitude::TypeInfo(), MagnitudeID));

	if (Magnitude == nullptr){
		clearPrefMagnitudeParameter();
		emit showInStatusbar(QString("no magnitude %1").arg(MagnitudeID.c_str()), 1000);
		return;
	}

	_ui->_lbPreMagType->setText((Magnitude->type()).c_str());
	double premagval = Magnitude->magnitude().value();
	QString text;
	if ( premagval < 12 ) {
		text = QString("%1").arg(premagval, 0, 'f', SCScheme.precision.magnitude);
	}
	else {
		text = "-";
	}
	_ui->_lbPreMagVal->setText(text);

	_magList->selectMagnitude(MagnitudeID.c_str());

	return;
}


void EventSummaryView::setMagnitudeParameter(DataModel::Origin* origin){

	// clear all mag parameters incl. buttons
	clearMagnitudeParameter();

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {

		// if preferred Magnitude --> text style BOLD
		bool pref = false;
		if (origin->magnitude(i)->publicID() == _currentEvent->preferredMagnitudeID() )
			pref = true;

		bool typeEnabled =
			(_visibleMagnitudes.find(origin->magnitude(i)->type()) != _visibleMagnitudes.end())
			|| _ui->actionShowInvisibleMagnitudes->isChecked();

		// create new magnitudeList display row
		_magList->addMag(origin->magnitude(i), pref, typeEnabled);
	}
}


void EventSummaryView::setAutomaticMagnitudeParameter(DataModel::Origin* origin) {
	for ( int i = 0; i < _magList->rowCount(); ++i )
		_magList->rowAt(i)->setReferenceMagnitude(nullptr);

	if ( !origin ) return;

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		Magnitude *mag = origin->magnitude(i);

		MagRow *row = _magList->row(mag->type());
		if ( row )
			row->setReferenceMagnitude(mag);
	}
}


void EventSummaryView::clearPrefMagnitudeParameter(){

	// clear preferred magnitude parameter
	_ui->_lbPreMagType->setText("--");
	_ui->_lbPreMagVal->setText("-.-");

}


void EventSummaryView::clearMagnitudeParameter(){
	// clear magnitudeList display rows
	_magList->reset();

	for ( std::set<std::string>::iterator it = _visibleMagnitudes.begin();
	      it != _visibleMagnitudes.end(); ++it ) {
		_magList->addMag(*it, false, true);
	}

	_ui->_lbPreMagType->setText("");
	_ui->_lbPreMagVal->setText("");
}


void EventSummaryView::clearOriginParameter(){
	_ui->labelDepth->setText("");
	_uiHypocenter->_lbAgency->setText("");
	_uiHypocenter->_lbFirstLocation->setText("");
	_uiHypocenter->_lbThisLocation->setText("");
	_uiHypocenter->_lbEventID->setText("");

	_uiHypocenter->_lbLatitude->setText("---.--");
	_uiHypocenter->_lbLatitudeUnit->setText("");
	_uiHypocenter->_lbLongitude->setText("---.--");
	_uiHypocenter->_lbLongitudeUnit->setText("");
	_uiHypocenter->_lbDepth->setText("---");
	_uiHypocenter->_lbDepthUnit->setText("---");

	_uiHypocenter->_lbNoPhases->setText("--");
	_uiHypocenter->_lbRMS->setText("--");
	_uiHypocenter->_lbAzGap->setText("--");
//	_uiHypocenter->_lbMinDist->setText("--");

	_uiHypocenter->_lbLatError->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	_uiHypocenter->_lbLongError->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	_uiHypocenter->_lbDepthError->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));

	_ui->_lbOriginTime->setText("0000/00/00  00:00:00");
	_ui->_lbTimeAgo->setVisible(false);

	_ui->_lbRegion->setText(""); _ui->_lbRegion->setVisible(false);
	_ui->_lbRegionExtra->setText(""); _ui->_lbRegionExtra->setVisible(false);
	_uiHypocenter->_lbOriginStatus->setText("");

	clearAutomaticOriginParameter();

	_ui->btnShowDetails->setEnabled(false);
	_ui->btnPlugable0->setEnabled(false);
	_ui->btnPlugable1->setEnabled(false);

	_ui->btnSwitchToAutomatic->setEnabled(false);

	setFMParametersVisible(false);

	if ( _map )
		_map->canvas().setSelectedCity(nullptr);
}


void EventSummaryView::clearAutomaticOriginParameter() {
	setLastAutomaticOriginColor(_automaticOriginDisabledColor);

	_ui->_lbOriginTimeAutomatic->setText("0000/00/00  00:00:00");
	_uiHypocenter->_lbLatitudeAutomatic->setText("---.--");
	_uiHypocenter->_lbLatitudeUnitAutomatic->setText("");
	_uiHypocenter->_lbLongitudeAutomatic->setText("---.--");
	_uiHypocenter->_lbLongitudeUnitAutomatic->setText("");
	_uiHypocenter->_lbDepthAutomatic->setText("---");
	_uiHypocenter->_lbDepthUnitAutomatic->setText("");
	_uiHypocenter->_lbNoPhasesAutomatic->setText("--");
	_uiHypocenter->_lbRMSAutomatic->setText("--");
	_uiHypocenter->_lbAzGapAutomatic->setText("--");
	_uiHypocenter->_lbCommentAutomatic->setText(_displayCommentDefault.c_str());
	_uiHypocenter->_lbLatErrorAutomatic->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	_uiHypocenter->_lbLongErrorAutomatic->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	_uiHypocenter->_lbDepthErrorAutomatic->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	_uiHypocenter->_lbOriginStatusAutomatic->setText("");
	_uiHypocenter->_lbAgencyAutomatic->setText("");

	setAutomaticMagnitudeParameter(nullptr);
}


void EventSummaryView::clearMap(){

	if (_map){
		_map->canvas().symbolCollection()->clear();
		_map->canvas().displayRect(QRectF(-180.0, -90.0, 360.0, 180.0));
		_map->update();
	}

}


bool EventSummaryView::updateLastAutomaticOrigin(DataModel::Origin *origin) {
	try {
		if ( origin->evaluationMode() != AUTOMATIC ) return false;
	}
	catch ( ... ) {}

	if ( !_lastAutomaticOrigin ) {
		_lastAutomaticOrigin = origin;
		return true;
	}

	Core::Time created;
	try {
		created = origin->creationInfo().creationTime();
	}
	catch ( ... ) {
		return false;
	}

	try {
		if ( created > _lastAutomaticOrigin->creationInfo().creationTime() ) {
			_lastAutomaticOrigin = origin;
			return true;
		}
	}
	catch ( ... ) {}

	return false;
}


bool EventSummaryView::updateLastAutomaticFM(DataModel::FocalMechanism *fm) {
	try {
		if ( fm->evaluationMode() != AUTOMATIC ) return false;
	}
	catch ( ... ) {}

	if ( !_lastAutomaticFocalMechanism ) {
		_lastAutomaticFocalMechanism = fm;
		return true;
	}

	Core::Time created;
	try {
		created = fm->creationInfo().creationTime();
	}
	catch ( ... ) {
		return false;
	}

	try {
		if ( created > _lastAutomaticFocalMechanism->creationInfo().creationTime() ) {
			_lastAutomaticFocalMechanism = fm;
			return true;
		}
	}
	catch ( ... ) {}

	return false;
}


void EventSummaryView::updateTimeAgoLabel(){

// 	emit showInStatusbar(QString("%1").arg(Core::BaseObject::ObjectCount(), 0, 'd', 0, ' '), 0);

	if (!_currentOrigin){
		_ui->_lbTimeAgo->setVisible(false);
		return;
	}

	if ( _map && _map->waveformPropagation() )
		_map->update();

	Core::TimeSpan ts;
	Core::Time ct;

	ct.gmt();
	ts = ct - _currentOrigin->time();

	if ( !_ui->_lbTimeAgo->isVisible() )
		_ui->_lbTimeAgo->setVisible(true);

	int sec = ts.seconds();
	int days = sec / 86400;
	int hours = (sec - days*86400) / 3600;
	int minutes = (sec - days*86400 - hours*3600) / 60;
	int seconds = sec - days*86400 - hours*3600 - 60*minutes;

	QString text;

	if (days>0)
		text = QString("%1 days and %2 hours ago").arg(days, 0, 'd', 0, ' ').arg(hours, 0, 'd', 0, ' ');
	else if ((days==0)&&(hours>0))
		text = QString("%1 hours and %2 minutes ago").arg(hours, 0, 'd', 0, ' ').arg(minutes, 0, 'd', 0, ' ');
	else if ((days==0)&&(hours==0)&&(minutes>0)) {
		if ( _maxMinutesSecondDisplay >= 0 && minutes > _maxMinutesSecondDisplay )
			text = QString("%1 minutes").arg(minutes, 0, 'd', 0, ' ');
		else
			text = QString("%1 minutes and %2 seconds ago").arg(minutes, 0, 'd', 0, ' ').arg(seconds, 0, 'd', 0, ' ');
	}
	else if ((days==0)&&(hours==0)&&(minutes==0)&&(seconds>0))
		text = QString("%1 seconds ago").arg(seconds, 0, 'd', 0, ' ');

	if ( text != _ui->_lbTimeAgo->text() )
		_ui->_lbTimeAgo->setText(text);

	/*
	double tsd = ts;

	while ( _originStationsIndex < _originStations.size() ) {
		if ( _originStations[_originStationsIndex].second > tsd )
			break;

		++_originStationsIndex;
	}

	if ( _originStationsIndex >= _originStations.size() )
		return;

	std::cout << "--------" << std::endl;
	std::cout << "Arrivals: " << _currentOrigin->arrivalCount() << std::endl;
	std::cout << "Passed picks: " << _originStationsIndex << std::endl;
	std::cout << "Awaited picks: " << _originStations.size() - _originStationsIndex << std::endl;
	*/
}


void EventSummaryView::drawStations(bool enable) {
	_map->setDrawStations(enable);
	_map->update();
}


void EventSummaryView::drawBeachballs(bool enable) {
	if ( _displayFocMechs == enable ) return;
	_displayFocMechs = enable;
	updateMap(false);
}


void EventSummaryView::drawFullTensor(bool enable) {
	if ( _enableFullTensor == enable ) return;
	_enableFullTensor = enable;
	updateMap(false);
}


void EventSummaryView::setWaveformPropagation(bool enable) {
	_map->setWaveformPropagation(enable);
}


void EventSummaryView::setAutoSelect(bool s) {
	_autoSelect = s;
}


void EventSummaryView::setInteractiveEnabled(bool e) {
	_interactive = e;

	_ui->frameProcessing->setVisible(_interactive);
	//_ui->btnShowDetails->setVisible(_interactive);

	setScript0(_script0, _scriptStyle0, _scriptExportMap0);
	setScript1(_script1, _scriptStyle1, _scriptExportMap1);
}


void EventSummaryView::runScript(const QString& script, const QString& name, bool oldStyle,
                                 bool exportMap) {
	if ( QMessageBox::question(this, "Run action",
	                           tr("Do you really want to continue (%1)?").arg(name),
	                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
		return;

	QString cmd;

	if ( oldStyle ) {
		Magnitude *nm = Magnitude::Find(_currentEvent->preferredMagnitudeID());

		std::string extraDescription = _currentOrigin?description(_currentOrigin.get()):"";

		cmd = QString("%1 %2 %3 %4 \"%5\"")
		      .arg(script)
		      .arg(_currentEvent->publicID().c_str())
		      .arg(_currentOrigin->arrivalCount())
		      .arg(nm?QString("%1").arg(nm->magnitude().value(), 0, 'f', SCScheme.precision.magnitude):"")
		      .arg(extraDescription.c_str());
	}
	else {
		cmd = QString("%1 %2 \"%3\" \"%4\" \"%5\"")
		      .arg(script)
		      .arg(_currentEvent->publicID().c_str())
		      .arg(_currentEvent->preferredOriginID().c_str())
		      .arg(_currentEvent->preferredMagnitudeID().c_str())
		      .arg(_currentEvent->preferredFocalMechanismID().c_str());
	}

	if ( exportMap ) {
		QString templ = QDir::toNativeSeparators(QDir::tempPath() + "/XXXXXXXX.png");
		QTemporaryFile tempFile(templ);
		tempFile.setAutoRemove(false);
		if ( tempFile.open() ) {
			QImage img = QImage(_map->canvas().size(), QImage::Format_ARGB32);
			img.fill(Qt::transparent);

			QPainter p(&img);
			_map->canvas().draw(p);
			img.save(tempFile.fileName(), "PNG");
			cmd += QString(" %1").arg(tempFile.fileName());
			SEISCOMP_DEBUG("Stored screenshot of the current map as file %s",
			               qPrintable(tempFile.fileName()));
		}
		else {
			QMessageBox::warning(this, "Export event",
			                     tr("Unable to wite map "
			                        "content to temporary file."));
			return;
		}

	}

	QString command = QString(cmd);
	SEISCOMP_DEBUG("%s", qPrintable(cmd));
	// start as background process w/o any communication channel
	if( !QProcess::startDetached(command) ) {
		QMessageBox::warning(this, "Export event", tr("Can't execute script"));
	}
}


void EventSummaryView::switchToAutomaticPressed() {
	if ( _currentEvent == nullptr ) return;

	JournalEntryPtr entry = new JournalEntry;
	entry->setObjectID(_currentEvent->publicID());
	entry->setAction("EvPrefOrgEvalMode");
	entry->setParameters("automatic");
	entry->setSender(SCApp->name() + "@" + System::HostInfo().name());
	entry->setCreated(Core::Time::GMT());

	NotifierPtr n = new Notifier("Journaling", OP_ADD, entry.get());
	NotifierMessagePtr nm = new NotifierMessage;
	nm->attach(n.get());
	SCApp->sendMessage(SCApp->messageGroups().event.c_str(), nm.get());
}


void EventSummaryView::runScript0() {
	runScript(_script0.c_str(), _ui->btnPlugable0->text(), _scriptStyle0,
	          _scriptExportMap0);
}

void EventSummaryView::runScript1() {
	runScript(_script1.c_str(), _ui->btnPlugable1->text(), _scriptStyle1,
	          _scriptExportMap1);
}

std::string EventSummaryView::description(Origin* origin) const {
	double dist, azi;
	const Math::Geo::CityD* coord =
		Math::Geo::nearestCity(origin->latitude(), origin->longitude(), _maxHotspotDist, _minHotspotPopulation,
		                       SCApp->cities(), &dist, &azi);

	if ( _map ) _map->canvas().setSelectedCity(coord);

	if ( !coord )
		return "";

	dist = (int)Math::Geo::deg2km(dist);
	std::string dir;

	if ( azi < 22.5 || azi > 360.0-22.5 )
		dir = "N";
	else if ( azi >= 22.5 && azi <= 90.0-22.5 )
		dir = "NE";
	else if ( azi > 90.0-22.5 && azi < 90.0+22.5 )
		dir = "E";
	else if ( azi >= 90.0+22.5 && azi <= 180.0-22.5 )
		dir = "SE";
	else if ( azi > 180.0-22.5 && azi < 180.0+22.5 )
		dir = "S";
	else if ( azi >= 180.0+22.5 && azi <= 270.0-22.5 )
		dir = "SW";
	else if ( azi > 270.0-22.5 && azi < 270.0+22.5 )
		dir = "W";
	else if ( azi >= 270.0+22.5 && azi <= 360.0-22.5 )
		dir = "NW";
	else
		dir = "?";

	return Util::replace(_hotSpotDescriptionPattern,
	                      PoiResolver(dist, dir, coord->name(), origin->latitude(), origin->longitude()));
}


bool EventSummaryView::checkAndDisplay(DataModel::Event *e) {
	// If no current event set display the requested events
	if ( !_currentEvent ) {
		processEventMsg(e);
		return true;
	}

	// If no constraints in terms of origin time exists,
	// display the requested event
	if ( !_showOnlyMostRecentEvent ) {
		processEventMsg(e);
		return true;
	}

	// If current origin is empty, display the event
	if ( !_currentOrigin ) {
		processEventMsg(e);
		return true;
	}

	// Otherwise check the origin time
	if ( _currentEvent->publicID() != e->publicID() ) {
		OriginPtr o = Origin::Find(e->preferredOriginID());
		if ( !o && _reader )
			o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), e->preferredOriginID()));

		if ( !o ) return false;

		try {
			if ( o->time().value() < _currentOrigin->time().value() )
				return false;
		}
		catch ( ... ) {
			return false;
		}
	}

	processEventMsg(e);
	return true;
}


void EventSummaryView::calcOriginDistances() {
	/*
	_originStations.clear();
	_originStationsIndex = 0;

	if ( !_currentOrigin ) return;

	double depth = 0;
	try { depth = _currentOrigin->depth(); } catch (...) {}

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( inv == nullptr ) return;

	Inventory *inventory = inv->inventory();
	if ( inventory == nullptr ) return;

	TravelTimeTable ttt;

	for ( size_t ni = 0; ni < inventory->networkCount(); ++ni ) {
		Network *n = inventory->network(ni);
		try { if ( n->end() ) continue; } catch (...) {}

		for ( size_t si = 0; si < n->stationCount(); ++si ) {
			Station *s = n->station(si);
			try { if ( s->end() ) continue; } catch (...) {}

			try {
				double lat = s->latitude();
				double lon = s->longitude();

				TravelTime tt =
				ttt.computeFirst(_currentOrigin->latitude(), _currentOrigin->longitude(), depth,
				                 lat, lon);

				_originStations.push_back(StationDistances::value_type(s, tt.time));
			}
			catch (... ) {}
		}
	}

	std::sort(_originStations.begin(), _originStations.end(), lessThan);
	*/
}


void EventSummaryView::setLastAutomaticOriginColor(QColor c) {
	if ( _automaticOriginColor == c ) return;

	SET_COLOR(_ui->_lbOriginTimeAutomatic, c);
	SET_COLOR(_uiHypocenter->frameInformationAutomatic, c);

	_magList->setReferenceMagnitudesColor(c);

	_automaticOriginColor = c;
}


void EventSummaryView::setLastAutomaticFMColor(QColor c) {
	if ( _automaticFMColor == c ) return;

	SET_COLOR(_uiHypocenter->fmFrameInformationAutomatic, c);

	_automaticFMColor = c;
}


void EventSummaryView::setLastAutomaticOriginVisible(bool v) {
	_uiHypocenter->frameInformationAutomatic->setVisible(v);
	_uiHypocenter->labelFrameInfoSpacer->setVisible(v);
	_ui->_lbOriginTimeAutomatic->setVisible(v);
	_magList->setReferenceMagnitudesVisible(v);
}


void EventSummaryView::setFMParametersVisible(bool v) {
	_uiHypocenter->labelFMSeparator->setVisible(v);
	_uiHypocenter->fmFrameInformation->setVisible(v);
	_uiHypocenter->fmFrameInformationAutomatic->setVisible(v && _showLastAutomaticSolution);
	_uiHypocenter->fmLabelFrameInfoSpacer->setVisible(v && _showLastAutomaticSolution);
}


void EventSummaryView::showVisibleMagnitudes(bool e) {
	if ( e )
		_magList->showAll();
	else
		_magList->hideTypes(_visibleMagnitudes);
}


void EventSummaryView::showOnlyMostRecentEvent(bool e) {
	_showOnlyMostRecentEvent = e;
}


void EventSummaryView::ignoreOtherEvents(bool e) {
	_ignoreOtherEvents = e;
}


}
}
