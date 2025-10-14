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


#define SEISCOMP_COMPONENT EventEdit

#include <seiscomp/client/inventory.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/gui/datamodel/eventedit.h>
#include <seiscomp/gui/datamodel/originsymbol.h>
#include <seiscomp/gui/datamodel/publicobjectevaluator.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/map/projection.h>
#include <seiscomp/gui/map/layers/annotationlayer.h>

#include <seiscomp/system/hostinfo.h>
#include <seiscomp/datamodel/journaling.h>
#include <seiscomp/datamodel/journalentry.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/datamodel/momenttensorstationcontribution.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/math/conversions.h>
#include <seiscomp/math/tensor.h>

#include <seiscomp/seismology/regions.h>

#include <QMenu>
#include <QMessageBox>

#include <algorithm>

#ifdef WIN32
#define snprintf _snprintf
#endif


using namespace std;
using namespace Seiscomp::DataModel;


namespace Seiscomp {
namespace Gui {


namespace {


MAKEENUM(
	OriginListColumns,
	EVALUES(
		OL_CREATED,
		OL_TIME,
		OL_PHASES,
		OL_LAT,
		OL_LON,
		OL_DEPTH,
		OL_DEPTH_TYPE,
		OL_RMS,
		OL_AZGAP,
		OL_STAT,
		OL_METHOD,
		OL_AGENCY,
		OL_AUTHOR,
		OL_REGION,
		OL_ID
	),
	ENAMES(
		"Created (%1)",
		"OT (%1)",
		"Phases",
		"Lat",
		"Lon",
		"Depth",
		"DType",
		"RMS",
		"AzGap",
		"Stat",
		"Method",
		"Agency",
		"Author",
		"Region",
		"ID"
	)
);

int OriginColAligns[OriginListColumns::Quantity] = {
	Qt::AlignLeft | Qt::AlignVCenter,
	Qt::AlignRight | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignRight | Qt::AlignVCenter,
	Qt::AlignRight | Qt::AlignVCenter,
	Qt::AlignRight | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignLeft | Qt::AlignVCenter,
	Qt::AlignLeft | Qt::AlignVCenter
};

bool OriginColBold[OriginListColumns::Quantity] = {
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	false,
	false,
	false
};


bool OriginColVisible[OriginListColumns::Quantity] = {
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true
};


MAKEENUM(
	MagnitudeListColumns,
	EVALUES(
		MLC_TIMESTAMP,
		MLC_TYPE,
		MLC_VALUE,
		MLC_NUM,
		MLC_RMS,
		MLC_STAT,
		MLC_AGENCY,
		MLC_AUTHOR,
		MLC_ID,
		MLC_DUMMY
	),
	ENAMES(
		"Created (%1)",
		"MType",
		"M",
		"Count",
		"RMS",
		"Stat",
		"Agency",
		"Author",
		"ID",
		""
	)
);

int MagnitudeColAligns[MagnitudeListColumns::Quantity] = {
	Qt::AlignLeft | Qt::AlignVCenter,
	Qt::AlignLeft | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter,
	Qt::AlignLeft | Qt::AlignVCenter,
	Qt::AlignHCenter | Qt::AlignVCenter
};

bool MagnitudeColBold[MagnitudeListColumns::Quantity] = {
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false
};

MAKEENUM(
	FMListColumns,
	EVALUES(
		FML_CREATED,
		FML_DEPTH,
		FML_MAG,
		FML_MCOUNT,
		FML_COUNT,
		FML_MISFIT,
		FML_STDR,
		FML_AZGAP,
		FML_STAT,
		FML_DC,
		FML_CLVD,
		FML_ISO,
		FML_STRIKE1,
		FML_DIP1,
		FML_RAKE1,
		FML_STRIKE2,
		FML_DIP2,
		FML_RAKE2,
		FML_AGENCY,
		FML_AUTHOR,
		FML_ID
	),
	ENAMES(
		"Created (%1)",
		"Depth",
		"M",
		"MCount",
		"Count",
		"Misfit",
		"STDR",
		"AzGap",
		"Stat",
		"DC",
		"CLVD",
		"ISO",
		"S1",
		"D1",
		"R1",
		"S2",
		"D2",
		"R2",
		"Agency",
		"Author",
		"ID"
	)
);

int FMColAligns[FMListColumns::Quantity] = {
	Qt::AlignLeft | Qt::AlignVCenter, // CREATED
	Qt::AlignCenter,                  // DEPTH
	Qt::AlignCenter,                  // MAG
	Qt::AlignCenter,                  // MCOUNT
	Qt::AlignCenter,                  // COUNT
	Qt::AlignCenter,                  // MISFIT
	Qt::AlignCenter,                  // STDR
	Qt::AlignCenter,                  // GAP
	Qt::AlignCenter,                  // STAT
	Qt::AlignCenter,                  // DC
	Qt::AlignCenter,                  // CLVD
	Qt::AlignCenter,                  // ISO
	Qt::AlignCenter,                  // STRIKE1
	Qt::AlignCenter,                  // DIP1
	Qt::AlignCenter,                  // RAKE1
	Qt::AlignCenter,                  // STRIKE2
	Qt::AlignCenter,                  // DIP2
	Qt::AlignCenter,                  // RAKE2
	Qt::AlignCenter,                  // AGENCY
	Qt::AlignCenter,                  // AUTHOR
	Qt::AlignLeft | Qt::AlignVCenter  // ID
};

bool FMColBold[FMListColumns::Quantity] = {
	false,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false
};


bool FMColVisible[FMListColumns::Quantity] = {
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true
};


#define RESET_COLUMN(ENUM) \
do {\
	item->setText(_fmColumnMap[ENUM], "-");\
	item->setData(_fmColumnMap[ENUM], Qt::UserRole, QVariant());\
} while (0)

#define POPULATE_COLUMN_INT(ENUM, DATA) \
do {\
	try {\
		item->setText(_fmColumnMap[ENUM], QString("%1").arg(DATA));\
		item->setData(_fmColumnMap[ENUM], Qt::UserRole, DATA);\
	}\
	catch ( Core::ValueException& ) {\
		item->setText(_fmColumnMap[ENUM], "-");\
		item->setData(_fmColumnMap[ENUM], Qt::UserRole, QVariant());\
	}\
} while (0)

#define POPULATE_COLUMN_DOUBLE(ENUM, DATA, PREC) \
do {\
	try {\
		item->setText(_fmColumnMap[ENUM], QString("%1").arg(DATA, 0, 'f', PREC));\
		item->setData(_fmColumnMap[ENUM], Qt::UserRole, DATA);\
	}\
	catch ( Core::ValueException& ) {\
		item->setText(_fmColumnMap[ENUM], "-");\
		item->setData(_fmColumnMap[ENUM], Qt::UserRole, QVariant());\
	}\
} while (0)

#define POPULATE_COLUMN(ENUM, TEXT, VALUE) \
do {\
	try {\
		item->setText(_fmColumnMap[ENUM], TEXT);\
		item->setData(_fmColumnMap[ENUM], Qt::UserRole, VALUE);\
	}\
	catch ( Core::ValueException& ) {\
		item->setText(_fmColumnMap[ENUM], "-");\
		item->setData(_fmColumnMap[ENUM], Qt::UserRole, QVariant());\
	}\
} while (0)

#define POPULATE_AGENCY(COL, VALUE) \
do {\
	try {\
		item->setText(COL, VALUE.c_str());\
		auto it = SCScheme.colors.agencies.find(VALUE);\
		if ( it != SCScheme.colors.agencies.end() )\
			item->setData(COL, Qt::ForegroundRole, it.value());\
		else \
			item->setData(COL, Qt::ForegroundRole, QVariant());\
	}\
	catch ( Seiscomp::Core::ValueException& ) {\
		item->setText(COL, QString());\
		item->setData(COL, Qt::ForegroundRole, QVariant());\
	}\
} while (0)



struct ConfigProcessColumn {
	int     pos;
	QString script;
	QString label;
};


typedef bool(*LessThan)(const QPair<QTreeWidgetItem*,int>&,const QPair<QTreeWidgetItem*,int>&);

bool itemLessThan(const QPair<QTreeWidgetItem*, int>& left, const QPair<QTreeWidgetItem*, int>& right) {
	return left.first->data(left.second, Qt::UserRole).toDouble() <
	       right.first->data(right.second, Qt::UserRole).toDouble();
}

bool itemGreaterThan(const QPair<QTreeWidgetItem*, int>& left, const QPair<QTreeWidgetItem*, int>& right) {
	return left.first->data(left.second, Qt::UserRole).toDouble() >
	       right.first->data(right.second, Qt::UserRole).toDouble();
}

bool itemTextLessThan(const QPair<QTreeWidgetItem*, int>& left, const QPair<QTreeWidgetItem*, int>& right) {
	return left.first->text(left.second) < right.first->text(right.second);
}

bool itemTextGreaterThan(const QPair<QTreeWidgetItem*, int>& left, const QPair<QTreeWidgetItem*, int>& right) {
	return left.first->text(left.second) > right.first->text(right.second);
}


class SquareSizeFilter : public QObject {
	public:
		SquareSizeFilter(QObject *parent = 0) : QObject(parent) {}

	protected:
		bool eventFilter(QObject *obj, QEvent *event) {
			if ( event->type() == QEvent::Resize ) {
				QResizeEvent *e = static_cast<QResizeEvent*>(event);
				static_cast<QWidget*>(obj)->setMaximumWidth(e->size().height());
				return true;
			}

			// standard event processing
			return QObject::eventFilter(obj, event);
		}
};

QString npToString(const NodalPlane &np) {
	return QString("%1/%2/%3").arg(np.strike().value(), 0, 'f', 2)
	                          .arg(np.dip().value(), 0, 'f', 2)
	                          .arg(np.rake().value(), 0, 'f', 2);
}

class OriginTreeWidget : public QTreeWidget {
	public:
		OriginTreeWidget(EventEdit *eventEdit, QWidget *p)
		: QTreeWidget(p), _eventEdit(eventEdit) {}

		bool dropMimeData(QTreeWidgetItem *parent, int index,
		                  const QMimeData *data, Qt::DropAction action) {

			QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
			QDataStream stream(&encoded, QIODevice::ReadOnly);

			qApp->changeOverrideCursor(Qt::ArrowCursor);

			QList<DataModel::Origin*> origins;
			DataModel::Origin *org;

			if ( parent != nullptr )
				org = DataModel::Origin::Find(parent->data(0,Qt::UserRole).toString().toStdString());
			else
				org = DataModel::Origin::Find(topLevelItem(index)->data(0,Qt::UserRole).toString().toStdString());

			if ( org != nullptr ) origins.append(org);

			while ( !stream.atEnd() ) {
				int row, col;
				QMap<int,  QVariant> roleDataMap;
				stream >> row >> col >> roleDataMap;
				if ( col == 0 ) {
					QString originID = roleDataMap[Qt::UserRole].toString();
					org = DataModel::Origin::Find(originID.toStdString());
					if ( org == nullptr )
						cerr << "Origin with id '" << qPrintable(originID) << "' not found" << endl;
					else if ( !origins.contains(org) )
						origins.append(org);
				}
			}

			if ( origins.count() > 1 ) _eventEdit->handleOrigins(origins);

			return false;
		}


	private:
		EventEdit *_eventEdit;
};


struct StationLayer : Map::Layer {
	StationLayer(FMMap *map)
	: Map::Layer(map) {}

	~StationLayer() override {
		clear();
	}

	void setReferenceSymbol(const ExtTensorSymbol *symbol) {
		refSymbol = symbol;
	}

	void setVisible(bool v) {
		Map::Layer::setVisible(v);
		if ( !v ) {
			for ( auto &entry : stations ) {
				if ( entry->annotation )
					entry->annotation->visible = false;
			}
		}
		else {
			for ( auto &entry : stations ) {
				if ( entry->annotation )
					entry->annotation->visible = entry->isVisible();
			}
		}
	}

	void calculateMapPosition(const Map::Canvas *canvas) override {
		for ( auto &entry : stations ) {
			entry->setVisible(true);
			entry->calculateMapPosition(canvas);

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
				setToolTip(static_cast<FMMap*>(parent())->toolTip());
				if ( toolTip().isEmpty() ) {
					if ( !stations[hoverId]->net.empty()
					  && !stations[hoverId]->code.empty() ) {
						setToolTip((stations[hoverId]->net + "." + stations[hoverId]->code).c_str());
					}
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

		if ( drawStationsLines && (canvas->symbolCollection()->count() > 0) && refSymbol ) {
			int cutOff = 4;

			if ( cutOff ) {
				p.setClipping(true);
				p.setClipRegion(
					QRegion(
						p.window()
					)
					-
					QRegion(
						QRect(
							refSymbol->pos().x() - cutOff / 2,
							refSymbol->pos().y() - cutOff / 2,
							cutOff, cutOff
						),
						QRegion::Ellipse
					)
				);
			}

			p.setPen(SCScheme.colors.map.lines);
			for ( auto &s : stations ) {
				canvas->drawLine(p, refSymbol->location(), s->location());
			}

			if ( cutOff ) {
				p.setClipping(false);
			}
		}

		for ( int i = stations.count() - 1; i >= 0; --i ) {
			if ( stations[i]->isClipped() || !stations[i]->isVisible() ) {
				continue;
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
		for ( auto &symbol : stations ) {
			delete symbol;
		}
		stations.clear();
	}

	struct Symbol : StationSymbol {
		Symbol() = delete;

		Symbol(QPointF loc, const std::string &nc,
		       const std::string &sc,
		       Map::AnnotationItem *annotation = nullptr)
		: StationSymbol(loc.y(), loc.x())
		, net(nc), code(sc)
		, annotation(annotation) {
			setOutlineColor(SCScheme.colors.map.outlines);
			setVisible(true);
		}

		std::string          net;
		std::string          code;
		Map::AnnotationItem *annotation{nullptr};
	};

	QVector<Symbol*>       stations;
	bool                   drawStationsLines{true};
	int                    hoverId{-1};
	const ExtTensorSymbol *refSymbol{nullptr};
};


#define SYMBOLLAYER static_cast<StationLayer*>(_symbolLayer)


double subGeo(double a, double b) {
	double s = a - b;
	if ( s < -180 )
		s += 360;
	else if ( s > 180 )
		s -= 360;
	return s;
}


QColor percentToColor(double value) {
	if ( Math::isNaN(value) ) {
		value = 0;
	}

	if ( value < 0 ) {
		value = 0;
	}
	else if ( value > 100 ) {
		value = 100;
	}

	if ( value < 50 ) {
		return blend(Qt::yellow, Qt::red, value * 2);
	}
	else {
		return blend(Qt::green, Qt::yellow, (value - 50) * 2);
	}
}


QSize FMDefaultSize = QSize(32, 32);
QSize FMSelectedSize = QSize(40, 40);


std::string TableCTimeFormat;
std::string TableOTimeFormat;
std::string PanelOTimeFormat;


} // namespace anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExtTensorSymbol::ExtTensorSymbol(const Math::Tensor2Sd &t,
                                 const FocalMechanism *fm,
                                 Map::Decorator* decorator)
 : TensorSymbol(t, decorator) {
	if ( !fm ) {
		return;
	}

	_fm = fm;

	// agencyID + created
	try {
		_agency = fm->creationInfo().agencyID().c_str();
		_created = fm->creationInfo().creationTime();
	}
	catch ( Seiscomp::Core::ValueException & ) {}

	if ( !fm->momentTensorCount() ) {
		return;
	}

	auto mt = fm->momentTensor(0);

	if ( _agency.isEmpty() ) {
		try { _agency = mt->creationInfo().agencyID().c_str(); }
		catch ( Seiscomp::Core::ValueException & ) {}
	}
	if ( !_created ) {
		try { _created = mt->creationInfo().creationTime(); }
		catch ( Seiscomp::Core::ValueException & ) {}
	}

	auto o = Origin::Find(mt->derivedOriginID());
	auto m = Magnitude::Find(mt->momentMagnitudeID());

	// depth
	if ( o ) {
		try { _depth = QString("%1 km").arg(o->depth().value(), 0, 'f', 0); }
		catch ( Seiscomp::Core::ValueException & ) {}
	}

	// magnitude
	if ( m ) {
		_magnitude = QString("%1 %2")
		             .arg(m->type().empty() ? "M" : m->type().c_str())
		             .arg(m->magnitude().value(), 0, 'f', 1);
	}

	_selected = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ExtTensorSymbol::customDraw(const Map::Canvas *c, QPainter &p) {
	p.save();

	if ( size() != _lastSize ) {
		_lastSize = size();
		resize(_lastSize.width(), _lastSize.height());
	}

	QPoint symbolPos;
	if ( _drawLocationConnector ) {
		c->projection()->project(symbolPos, _refPos);
		symbolPos += _offset;
		QColor color = _selected ? Qt::black : QColor(64, 64, 64);
		p.setPen(QPen(color, 1, _selected ? Qt::SolidLine : Qt::DashLine));
		p.drawLine(pos(), symbolPos);
		p.setPen(color);
		p.setBrush(QBrush(color));
		p.drawEllipse(QRect(pos().x() - 2, pos().y() - 2, 4, 4));
	}
	else {
		symbolPos = pos();
	}

	p.drawImage(symbolPos - QPoint(_size.width() / 2, _size.height() / 2), _buffer);

	// draw label
	QString text;
	int lines = 0;
	int width = 0;
	QFont f = SCScheme.fonts.smaller;
	QFontMetrics fm(f);
	if ( _drawAgency && !_agency.isEmpty() ) {
		text += _agency;
		++lines;
		width = QT_FM_WIDTH(fm, _agency);
	}
	if ( _drawMagnitude && !_magnitude.isEmpty() ) {
		if ( !text.isEmpty() ) text += "\n";
		text += _magnitude;
		++lines;
		width = max(width, QT_FM_WIDTH(fm, _magnitude));
	}
	if ( _drawDepth && !_depth.isEmpty() ) {
		if ( !text.isEmpty() ) text += "\n";
		text += _depth;
		++lines;
		width = max(width, QT_FM_WIDTH(fm, _depth));
	}

	if ( lines ) {
		p.setFont(f);
		p.setBrush(QColor(255, 255, 255, 192));
		p.setPen(Qt::black);

		int margin = 4;
		QRect r(0, 0, width + 2 * margin, lines * fm.height() + margin);
		p.translate(symbolPos - QPoint(r.width()/2, -FMDefaultSize.height()/2 - margin));

		p.drawRect(r);
		p.drawText(r, text, QTextOption(Qt::AlignCenter));
	}

	p.restore();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FMMap::~FMMap() {
	QSettings settings;
	settings.beginGroup("FocalMechanismMap");
	settings.setValue("tensorDrawAgency", _drawAgency);
	settings.setValue("tensorDrawMagnitude", _drawMagnitude);
	settings.setValue("tensorDrawDepth", _drawDepth);
	settings.setValue("tensorDrawStations", _drawStations);
	settings.setValue("tensorSmartLayout", _smartLayout);
	settings.setValue("tensorGroupByAgency", _groupByAgency);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::draw(QPainter& p) {
	if ( _smartLayout && _smartLayoutDirty && _originSymbol ) {
		_smartLayoutDirty = false;
		int items = 0;

		for ( auto &it : _fmSymbols ) {
			if ( it.second->isVisible() ) {
				++items;
			}
		}

		QSize itemSize(1.7 * FMDefaultSize.width(), 3 * FMDefaultSize.height());
		int xItems = items < 4 ? 0 : 2;
		int yItems = 0;
		int i = -1;
		int x, y;

		for ( FMSymbols::iterator it = _fmSymbols.begin();
		      it != _fmSymbols.end(); ++it ) {
			if ( !it->second->isVisible() ) continue;
			if ( i < 0 || i >= 2*xItems + 2*yItems ) {
				i = 0;
				xItems += 2; yItems += 2;
				x = -xItems / 2; y = yItems / 2;
			}
			else if ( i <= xItems ) ++x;
			else if ( i <= xItems + yItems ) --y;
			else if ( i <= 2*xItems + yItems ) --x;
			else ++y;

			it->second->setOffset(QPoint(x * itemSize.width(), y * itemSize.height()));
			++i;
		}
	}

	MapWidget::draw(p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::init() {
	_originSymbol = nullptr;
	_symbolLayer = new StationLayer(this);
	canvas().addLayer(_symbolLayer);
	_annotationLayer = new Map::AnnotationLayer(this, new Map::Annotations(this));
	_annotationLayer->setVisible(false);
	canvas().addLayer(_annotationLayer);

	QSettings settings;
	settings.beginGroup("FocalMechanismMap");
	_drawAgency = settings.value("tensorDrawAgency", false).toBool();
	_drawMagnitude = settings.value("tensorDrawMagnitude", false).toBool();
	_drawDepth = settings.value("tensorDrawDepth", false).toBool();
	_drawStations = settings.value("tensorDrawStations", false).toBool();
	_smartLayout = settings.value("tensorSmartLayout", false).toBool();
	_groupByAgency = settings.value("tensorGroupByAgency", false).toBool();
	_smartLayoutDirty = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::updateStations(const DataModel::FocalMechanism *fm) {
	SYMBOLLAYER->clear();
	_annotationLayer->annotations()->clear();

	// FocalMechanism Station Symbols
	if ( !fm || fm->momentTensorCount() == 0 ) {
		return;
	}

	auto inv = Client::Inventory::Instance();
	auto mt = fm->momentTensor(0);

	auto o = Origin::Find(mt->derivedOriginID());
	if ( !o ) {
		o = Origin::Find(fm->triggeringOriginID());
	}

	if ( !o ) {
		return;
	}

	SYMBOLLAYER->setReferenceSymbol(nullptr);
	{
		auto it = _fmSymbols.find(fm->publicID());
		if ( it != _fmSymbols.end() ) {
			SYMBOLLAYER->setReferenceSymbol(it->second);
		}
	}

	for ( size_t i = 0; i < mt->momentTensorStationContributionCount(); ++i ) {
		const auto *contrib = mt->momentTensorStationContribution(i);
		DataModel::WaveformStreamID wfid;

		try {
			wfid = contrib->waveformID();
		}
		catch ( Core::ValueException &) {
			continue;
		}

		auto *loc = inv->getSensorLocation(
			wfid.networkCode(), wfid.stationCode(),
			wfid.locationCode(), o->time()
		);

		if ( !loc ) {
			continue;
		}

		auto *symbol = new StationLayer::Symbol(
			QPointF(loc->longitude(), loc->latitude()),
			wfid.networkCode(), wfid.stationCode(),
			_annotationLayer->annotations()->add(
				(wfid.networkCode() + "." + wfid.stationCode()).c_str()
			)
		);
		symbol->annotation->highlighted = true;
		symbol->setID(wfid.networkCode() + "." + wfid.stationCode() + "." +
		              wfid.locationCode() + "." + wfid.channelCode());
		symbol->setOutlineColor(Qt::black);
		symbol->setColor(Qt::gray);

		double percent = 0, percentCount = 0;
		for ( size_t j = 0; j < contrib->momentTensorComponentContributionCount(); ++j ) {
			auto ccontrib = contrib->momentTensorComponentContribution(j);
			if ( ccontrib->active() ) {
				try {
					percent += (1.0 - max(min(ccontrib->misfit(), 1.0), 0.0)) * 100;
					percentCount += 1.0;
				}
				catch ( ... ) {}
			}
		}

		if ( percentCount > 0 ) {
			percent /= percentCount;
			symbol->setColor(percentToColor(percent));
		}

		SYMBOLLAYER->stations.append(symbol);
	}

	SYMBOLLAYER->update(Map::Layer::Position);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::addFM(const DataModel::FocalMechanism *fm) {
	Origin *o = nullptr;
	MomentTensor *mt = nullptr;

	if ( fm->momentTensorCount() > 0 ) {
		mt = fm->momentTensor(0);
		o = Origin::Find(mt->derivedOriginID());
	}

	if ( !o ) {
		o = Origin::Find(fm->triggeringOriginID());
	}

	const NodalPlane *np = nullptr;
	try {
		np = &(fm->nodalPlanes().nodalPlane1());
	}
	catch ( Core::ValueException& ) {}

	if ( !o || !np ) {
		return;
	}

	double depth = 10;
	try {
		depth = o->depth();
	}
	catch ( Core::ValueException& ) {}

	auto c = SCScheme.colors.originSymbol.depth.gradient.colorAt(
		depth, SCScheme.colors.originSymbol.depth.discrete
	);

	Math::Tensor2Sd tensor;
	Math::NODAL_PLANE plane;
	plane.str = np->strike();
	plane.dip = np->dip();
	plane.rake = np->rake();
	Math::np2tensor(plane, tensor);

	auto symbol = new ExtTensorSymbol(tensor, fm);
	symbol->setID(fm->publicID());
	symbol->setPosition(QPointF(o->longitude(), o->latitude()));
	symbol->setSize(FMDefaultSize);
	symbol->setPriority(Map::Symbol::HIGH);
	symbol->setShadingEnabled(true);
	symbol->setTColor(c);
	symbol->setBorderColor(Qt::black);
	symbol->setDrawAgency(_drawAgency);
	symbol->setDrawMagnitude(_drawMagnitude);
	symbol->setDrawDepth(_drawDepth);

	if ( _originSymbol ) {
		symbol->setReferencePosition(QPointF(_originSymbol->longitude(),
		                                     _originSymbol->latitude()));
		symbol->setDrawConnectorEnabled(_smartLayout);
	}

	canvas().symbolCollection()->add(symbol);

	if ( _groupByAgency ) {
		if ( symbol->agencyID().isEmpty() ) {
			symbol->setVisible(false);
		}
		else {
			for ( FMSymbols::iterator it = _fmSymbols.begin();
			      it != _fmSymbols.end(); ++it ) {
				if ( it->second->agencyID() != symbol->agencyID() ) {
					continue;
				}

				if ( it->second->created() > symbol->created() ) {
					symbol->setVisible(false);
					break;
				}
				else {
					it->second->setVisible(false);
				}
			}
		}
	}

	_fmSymbols[fm->publicID()] = symbol;

	if ( _fmBoundings.isNull() ) {
		_fmBoundings.setRect(o->longitude()-0.01, o->latitude()-0.01, 0.02, 0.02);
	}
	else {
		if ( o->latitude() < _fmBoundings.top() ) {
			_fmBoundings.setTop(o->latitude());
		}
		else if ( o->latitude() > _fmBoundings.bottom() ) {
			_fmBoundings.setBottom(o->latitude());
		}

		double dist = subGeo(o->longitude().value(), _fmBoundings.left());
		if ( dist < 0 ) {
			_fmBoundings.setLeft(_fmBoundings.left() + dist);
		}
		else if ( dist > _fmBoundings.width() ) {
			_fmBoundings.setRight(_fmBoundings.left() + dist);
		}
	}

	setEnabled(true);
	canvas().displayRect(_fmBoundings.adjusted(-0.5, -0.5, 0.5, 0.5));
	_smartLayoutDirty = true;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::clear() {
	updateStations(nullptr);
	canvas().symbolCollection()->clear();
	_fmBoundings = QRectF();
	_originSymbol = nullptr;
	_fmSymbols.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::setCurrentFM(const string &id) {
	if ( _currentFMID == id ) {
		return;
	}

	_currentFMID = id;

	for ( const auto &fmSymbolEntry : _fmSymbols ) {
		fmSymbolEntry.second->setSize(FMDefaultSize);
		fmSymbolEntry.second->setSelected(false);
	}

	auto it = _fmSymbols.find(id);
	if ( it != _fmSymbols.end() ) {
		it->second->setSize(FMSelectedSize);
		it->second->setSelected(true);
		if ( it->second->isVisible() ) {
			canvas().symbolCollection()->setTop(it->second);
		}
		updateStations(it->second->model());
	}
	else {
		updateStations(nullptr);
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::setEvent(const DataModel::Event *event) {
	Map::SymbolLayer *col = canvas().symbolCollection();
	if ( _originSymbol ) {
		col->remove(_originSymbol);
		_originSymbol = nullptr;
	}

	QPointF epicenter;
	Origin *o = nullptr;
	if ( event ) o = Origin::Find(event->preferredOriginID());
	if ( o ) {
		_originSymbol = new OriginSymbol;
		_originSymbol->setPriority(Map::Symbol::MEDIUM);
		_originSymbol->setID(o->publicID());
		_originSymbol->setLatitude(o->latitude());
		_originSymbol->setLongitude(o->longitude());
		try {
			_originSymbol->setDepth(o->depth());
		} catch ( Core::ValueException& ) {}
		col->add(_originSymbol);
		epicenter.setX(o->longitude());
		epicenter.setY(o->latitude());
	}

	bool refPosEnabled = _originSymbol && _smartLayout;
	for ( FMSymbols::iterator it = _fmSymbols.begin(); it != _fmSymbols.end(); ++it ) {
		it->second->setReferencePosition(epicenter);
		it->second->setDrawConnectorEnabled(refPosEnabled);
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::setDrawStations(bool draw) {
	_symbolLayer->setVisible(draw);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::setDrawStationAnnotations(bool draw) {
	_annotationLayer->setVisible(draw);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FMMap::contextMenuEvent(QContextMenuEvent *e) {
	QMenu menu(this);
	auto *fmMenu = menu.addMenu("Focal Mechanism");

	auto *actionDrawAgency = fmMenu->addAction("Draw agency id");
	actionDrawAgency->setCheckable(true);
	actionDrawAgency->setChecked(_drawAgency);

	auto *actionDrawMagnitude = fmMenu->addAction("Draw magnitude");
	actionDrawMagnitude->setCheckable(true);
	actionDrawMagnitude->setChecked(_drawMagnitude);

	auto *actionDrawDepth = fmMenu->addAction("Draw depth");
	actionDrawDepth->setCheckable(true);
	actionDrawDepth->setChecked(_drawDepth);

	auto *actionSmartLayout = fmMenu->addAction("Smart layout");
	actionSmartLayout->setEnabled(_originSymbol);
	actionSmartLayout->setCheckable(true);
	actionSmartLayout->setChecked(_smartLayout);

	auto *actionGroupByAgency = fmMenu->addAction("Group by agency");
	actionGroupByAgency->setCheckable(true);
	actionGroupByAgency->setChecked(_groupByAgency);

	updateContextMenu(&menu);

	auto *action = menu.exec(e->globalPos());
	if ( !action ) {
		return;
	}

	if ( action == actionDrawAgency ) {
		_drawAgency = !_drawAgency;
	}
	else if ( action == actionDrawMagnitude ) {
		_drawMagnitude = !_drawMagnitude;
	}
	else if ( action == actionDrawDepth ) {
		_drawDepth = !_drawDepth;
	}
	else if ( action == actionSmartLayout ) {
		_smartLayout = !_smartLayout;
	}
	else if ( action == actionGroupByAgency ) {
		_groupByAgency = !_groupByAgency;
		_smartLayoutDirty = true;

		FMSymbols agencyMap;
		for ( const auto &fmSymbolEntry : _fmSymbols ) {
			auto *fmSymbol = fmSymbolEntry.second;
			if ( _groupByAgency && !fmSymbol->agencyID().isEmpty() ) {
				auto a_it = agencyMap.find(fmSymbol->agencyID().toStdString());
				if ( a_it == agencyMap.end() ||
				     a_it->second->created() <= fmSymbol->created() ) {
					fmSymbol->setVisible(true);
					if ( a_it != agencyMap.end()) {
						a_it->second->setVisible(false);
					}
					agencyMap[fmSymbol->agencyID().toStdString()] = fmSymbol;
					continue;
				}
			}
			fmSymbol->setVisible(!_groupByAgency);
		}
	}
	else {
		executeContextMenuAction(action);
		return;
	}

	for ( const auto &fmSymbolEntry : _fmSymbols ) {
		auto *fmSymbol = fmSymbolEntry.second;
		fmSymbol->setDrawAgency(_drawAgency);
		fmSymbol->setDrawMagnitude(_drawMagnitude);
		fmSymbol->setDrawDepth(_drawDepth);
		fmSymbol->setDrawConnectorEnabled(_originSymbol && _smartLayout);
	}
	update();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventEdit::EventEdit(DatabaseQuery* reader,
                     Map::ImageTree *mapTree,
                     QWidget *parent)
: QWidget(parent) {
	_fmActivityMovie = nullptr;
	_reader = reader;
	_mapTreeOrigin = mapTree;
	_mapTreeFM = mapTree;

	if ( TableOTimeFormat.empty() ) {
		TableOTimeFormat = "%T";
		if ( SCScheme.precision.originTime > 0 ) {
			TableOTimeFormat += ".%";
			TableOTimeFormat += Core::toString(SCScheme.precision.originTime);
			TableOTimeFormat += "f";
		}
	}

	if ( PanelOTimeFormat.empty() ) {
		PanelOTimeFormat = "%F %T";
		if ( SCScheme.precision.originTime > 0 ) {
			PanelOTimeFormat += ".%";
			PanelOTimeFormat += Core::toString(SCScheme.precision.originTime);
			PanelOTimeFormat += "f";
		}
	}

	if ( TableCTimeFormat.empty() ) {
		TableCTimeFormat = "%F %T";
		if ( SCScheme.precision.originTime > 0 ) {
			TableCTimeFormat += ".%";
			TableCTimeFormat += Core::toString(SCScheme.precision.originTime);
			TableCTimeFormat += "f";
		}
	}

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventEdit::~EventEdit() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::init() {
	_ui.setupUi(this);

	Object::RegisterObserver(this);

	_blockObserver = false;

	_ui.frameMap->installEventFilter(new SquareSizeFilter(this));
	_originMap = new MapWidget(_mapTreeOrigin.get(), _ui.frameMap);
	_fmMap = new FMMap(_mapTreeFM.get(), _ui.fmMap);

	_fmActivity = new QLabel(_fmMap);
	_fmActivity->move(4,4);
	_fmActivity->resize(32,32);
	_fmActivity->hide();

	QHBoxLayout *l = new QHBoxLayout(_ui.frameMap);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(_originMap);

	l = new QHBoxLayout(_ui.fmMap);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(_fmMap);

	_updateLocalEPInstance = false;

	QFont f(font());
	f.setBold(true);

	_ui.labelDepthValue->setFont(f);
	_ui.labelDepthUnit->setFont(f);
	_ui.labelDepthError->setFont(f);

	_ui.labelLatitudeValue->setFont(f);
	_ui.labelLatitudeUnit->setFont(f);
	_ui.labelLatitudeError->setFont(f);

	_ui.labelLongitudeValue->setFont(f);
	_ui.labelLongitudeUnit->setFont(f);
	_ui.labelLongitudeError->setFont(f);

	_ui.labelMagnitudeTypeValue->setFont(f);

	QObject *drawFilter = new ElideFadeDrawer(this);
	_ui.labelRegionValue->installEventFilter(drawFilter);
	_ui.labelMagnitudeMethodValue->installEventFilter(drawFilter);

	try {
		std::vector<std::string> cols = SCApp->configGetStrings("eventedit.origin.visibleColumns");
		for ( int i = 0; i < OriginListColumns::Quantity; ++i )
			OriginColVisible[i] = false;

		for ( size_t i = 0; i < cols.size(); ++i ) {
			OriginListColumns v;
			if ( !v.fromString(cols[i]) ) {
				std::cerr << "ERROR: eventedit.origin.visibleColumns: invalid column name '"
				          << cols[i] << "' at index " << i << ", ignoring" << std::endl;
				continue;
			}

			OriginColVisible[v] = true;
		}

		// First columns are always visible
		OriginColVisible[OL_CREATED] = true;
		OriginColVisible[OL_TIME] = true;
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> cols = SCApp->configGetStrings("eventedit.fm.visibleColumns");
		for ( int i = 0; i < FMListColumns::Quantity; ++i )
			FMColVisible[i] = false;

		for ( size_t i = 0; i < cols.size(); ++i ) {
			FMListColumns v;
			if ( !v.fromString(cols[i]) ) {
				std::cerr << "ERROR: eventedit.fm.visibleColumns: invalid column name '"
				          << cols[i] << "' at index " << i << ", ignoring" << std::endl;
				continue;
			}

			FMColVisible[v] = true;
		}

		// First column is always visible
		FMColVisible[FML_CREATED] = true;
	}
	catch ( ... ) {}

	_customColumn = -1;
	_customDefaultText = "-";

	for ( int i = 0; i < OriginListColumns::Quantity; ++i )
		_originColumnMap.append(i);
	for ( int i = 0; i < FMListColumns::Quantity; ++i )
		_fmColumnMap.append(i);

	try {
		_customDefaultText = SCApp->configGetString("eventedit.origin.customColumn.default").c_str();
	}
	catch ( ... ) {
		try {
			_customDefaultText = SCApp->configGetString("eventedit.customColumn.default").c_str();
			SEISCOMP_WARNING("The parameter 'eventedit.customColumn.default' is deprecated and will be removed in future. "
			                 "Please replace with 'eventedit.origin.customColumn.default'.");
		}
		catch ( ... ) {}
	}

	try {
		_commentID = SCApp->configGetString("eventedit.origin.customColumn.originCommentID");
	}
	catch ( ... ) {
		try {
			_commentID = SCApp->configGetString("eventedit.customColumn.originCommentID");
			SEISCOMP_WARNING("The parameter 'eventedit.customColumn.originCommentID' is deprecated and will be removed in future. "
			                 "Please replace with 'eventedit.origin.customColumn.originCommentID'.");
		}
		catch ( ... ) {}
	}

	try {
		_customColumn = SCApp->configGetInt("eventedit.origin.customColumn.pos");
		try {
			_customColumn = SCApp->configGetInt("eventedit.customColumn.pos");
			SEISCOMP_WARNING("The parameter 'eventedit.customColumn.pos' is deprecated and will be removed in future. "
			                 "Please replace with 'eventedit.origin.customColumn.pos'.");
		}
		catch ( ... ) {
			_customColumn = 6;
		}
	}
	catch ( ... ) {}

	try {
		_customColumnLabel = SCApp->configGetString("eventedit.origin.customColumn.name").c_str();
	}
	catch ( ... ) {
		try {
			_customColumnLabel = SCApp->configGetString("eventedit.customColumn").c_str();
			SEISCOMP_WARNING("The parameter 'eventedit.customColumn' is deprecated and will be removed in future. "
			                 "Please replace with 'eventedit.origin.customColumn.name'.");
		}
		catch ( ... ) {
			_customColumn = -1;
		}
	}

	{
		std::vector<std::string> customColors;
		try {
			customColors = SCApp->configGetStrings("eventedit.origin.customColumn.colors");
		}
		catch ( ... ) {
			try {
				customColors = SCApp->configGetStrings("eventedit.customColumn.colors");
				SEISCOMP_WARNING("The parameter 'eventedit.customColumn.colors' is deprecated and will be removed in future. "
				                 "Please replace with 'eventedit.origin.customColumn.colors'.");
			}
			catch ( ... ) {}
		}

		for ( size_t i = 0; i < customColors.size(); ++i ) {
			size_t pos = customColors[i].rfind(':');
			if ( pos == std::string::npos ) continue;
			std::string value = customColors[i].substr(0, pos);
			std::string strColor = customColors[i].substr(pos+1);
			QColor color;
			if ( fromString(color, strColor) )
				_customColorMap[value] = color;
		}
	}

	if ( _customColumn >= 0 ) {
		if ( _customColumn < _originColumnMap.size() ) {
			for ( int i = _customColumn; i < _originColumnMap.size(); ++i )
				_originColumnMap[i] = i+1;
		}
		else
			_customColumn = OriginListColumns::Quantity;
	}

	_originTableHeader.clear();
	for ( int i = 0; i < OriginListColumns::Quantity; ++i ) {
		if ( i == _customColumn )
			_originTableHeader << _customColumnLabel;

		switch ( i ) {
			case OL_CREATED:
			case OL_TIME:
				if ( SCScheme.dateTime.useLocalTime )
					_originTableHeader << QString(EOriginListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
				else
					_originTableHeader << QString(EOriginListColumnsNames::name(i)).arg("UTC");
				break;
			case OL_AZGAP:
				_originTableHeader << QString("%1 (°)").arg(EOriginListColumnsNames::name(i));
				break;
			case OL_DEPTH:
				_originTableHeader << QString("%1").arg(EOriginListColumnsNames::name(i));
				break;
			case OL_RMS:
				_originTableHeader << QString("%1 (s)").arg(EOriginListColumnsNames::name(i));
				break;
			case OL_LAT:
				_originTableHeader << QString("%1 (°)").arg(EOriginListColumnsNames::name(i));
				break;
			case OL_LON:
				_originTableHeader << QString("%1 (°)").arg(EOriginListColumnsNames::name(i));
				break;
			default:
				_originTableHeader << EOriginListColumnsNames::name(i);
				break;
		}
	}

	if ( _customColumn == OriginListColumns::Quantity )
		_originTableHeader << _customColumnLabel;

	for ( int i = 0; i < FMListColumns::Quantity; ++i ) {
		switch ( i ) {
			case FML_CREATED:
				if ( SCScheme.dateTime.useLocalTime )
					_fmTableHeader << QString(EFMListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
				else
					_fmTableHeader << QString(EFMListColumnsNames::name(i)).arg("UTC");
				break;
			case FML_AZGAP:
				_fmTableHeader << QString("%1 (°)").arg(EFMListColumnsNames::name(i));
				break;
			case FML_DC:
			case FML_CLVD:
			case FML_ISO:
				_fmTableHeader << QString("%1 (%)").arg(EFMListColumnsNames::name(i));
				break;
			case FML_STRIKE1:
			case FML_STRIKE2:
			case FML_DIP1:
			case FML_DIP2:
			case FML_RAKE1:
			case FML_RAKE2:
				_fmTableHeader << QString("%1 (°)").arg(EFMListColumnsNames::name(i));
				break;
			default:
				_fmTableHeader << EFMListColumnsNames::name(i);
				break;
		}
	}

	// Custom event types
	try {
		vector<string> eventTypes = SCApp->configGetStrings("olv.commonEventTypes");
		for (  size_t i = 0; i < eventTypes.size(); ++i ) {
			DataModel::EventType type;
			if ( !type.fromString(eventTypes[i].c_str()) ) {
				SEISCOMP_WARNING("olv.commonEventTypes: invalid type, ignoring: %s",
				                 eventTypes[i].c_str());
			}
			else
				_eventTypesWhitelist.append(type);
		}
	}
	catch ( ... ) {}

	_ui.comboTypes->addItem("- unset -");

	if ( _eventTypesWhitelist.isEmpty() ) {
		QStringList types;
		for ( int i = 0; i < 4; ++i ) {
			_ui.comboTypes->addItem(QString());
		}
		for ( int i = int(EventType::First); i < int(EventType::Quantity); ++i ) {
			if ( EventType::Type(i) == EARTHQUAKE ) {
				_ui.comboTypes->setItemText(1, EventType::NameDispatcher::name(i));
				_ui.comboTypes->setItemData(1, EventType::NameDispatcher::name(i),
				                            Qt::ToolTipRole);
			}
			else if ( EventType::Type(i) == NOT_EXISTING ) {
				_ui.comboTypes->setItemText(2, EventType::NameDispatcher::name(i));
				_ui.comboTypes->setItemData(2, EventType::NameDispatcher::name(i),
				                            Qt::ToolTipRole);
			}
			else if ( EventType::Type(i) == NOT_LOCATABLE ) {
				_ui.comboTypes->setItemText(3, EventType::NameDispatcher::name(i));
				_ui.comboTypes->setItemData(3, EventType::NameDispatcher::name(i),
				                            Qt::ToolTipRole);
			}
			else if ( EventType::Type(i) == OUTSIDE_OF_NETWORK_INTEREST ) {
				_ui.comboTypes->setItemText(4, EventType::NameDispatcher::name(i));
				_ui.comboTypes->setItemData(4, EventType::NameDispatcher::name(i),
				                            Qt::ToolTipRole);
			}
			else {
				types << EventType::NameDispatcher::name(i);
			}
		}
		_ui.comboTypes->insertSeparator(5);
		types.sort();
		foreach (QString type, types) {
			_ui.comboTypes->addItem(type);
			_ui.comboTypes->setItemData(_ui.comboTypes->count()-1, type, Qt::ToolTipRole);
		}
	}
	else {
		bool usedFlags[DataModel::EventType::Quantity];
		for ( int i = 0; i < DataModel::EventType::Quantity; ++i )
			usedFlags[i] = false;

		for ( int i = 0; i < _eventTypesWhitelist.count(); ++i ) {
			if ( usedFlags[_eventTypesWhitelist[i]] ) continue;
			_ui.comboTypes->addItem(_eventTypesWhitelist[i].toString());
			_ui.comboTypes->setItemData(_ui.comboTypes->count()-1,
			                            _eventTypesWhitelist[i].toString(), Qt::ToolTipRole);
			usedFlags[_eventTypesWhitelist[i]] = true;
		}
		_ui.comboTypes->insertSeparator(_eventTypesWhitelist.count()+1);

		QColor reducedColor;
		reducedColor = blend(palette().color(QPalette::Text), palette().color(QPalette::Base), 50);

		QStringList types;
		for ( int i = 0; i < DataModel::EventType::Quantity; ++i ) {
			if ( usedFlags[i] ) continue;
			types << EventType::NameDispatcher::name(i);
		}
		types.sort();
		foreach (QString type, types) {
			_ui.comboTypes->addItem(type);
			_ui.comboTypes->setItemData(_ui.comboTypes->count()-1, reducedColor, Qt::ForegroundRole);
			_ui.comboTypes->setItemData(_ui.comboTypes->count()-1, type, Qt::ToolTipRole);
		}
	}

	_ui.comboTypeCertainties->addItem("- unset -");
	for ( int i = (int)EventTypeCertainty::First; i < (int)EventTypeCertainty::Quantity; ++i )
		_ui.comboTypeCertainties->addItem(EventTypeCertainty::NameDispatcher::name(i));

	QVector<ConfigProcessColumn> scriptColumns;

	// Read script columns
	try {
		std::vector<std::string> processProfiles =
			SCApp->configGetStrings("eventedit.scripts.columns");

		for ( size_t i = 0; i < processProfiles.size(); ++i ) {
			ConfigProcessColumn item;
			try {
				item.pos = SCApp->configGetInt("eventedit.scripts.column." + processProfiles[i] + ".pos");
			}
			catch ( ... ) {
				item.pos = -1;
			}

			try {
				item.script = Environment::Instance()->absolutePath(SCApp->configGetString("eventedit.scripts.column." + processProfiles[i] + ".script")).c_str();
			}
			catch ( ... ) {}

			if ( item.script.isEmpty() ) {
				std::cerr << "WARNING: eventedit.scripts.column."
				          << processProfiles[i] << ".script is not set: ignoring"
				          << std::endl;
				continue;
			}

			try {
				item.label = SCApp->configGetString("eventedit.scripts.column." + processProfiles[i] + ".label").c_str();
			}
			catch ( ... ) {
				std::cerr << "WARNING: eventedit.scripts.column."
				          << processProfiles[i] << ".label is not set: ignoring"
				          << std::endl;
				continue;
			}

			scriptColumns.append(item);
		}
	}
	catch ( ... ) {}

	// Apply process column configuration
	for ( int i = 0; i < scriptColumns.size(); ++i ) {
		ConfigProcessColumn &item = scriptColumns[i];

		if ( item.pos >= 0 && item.pos < _originTableHeader.size() ) {
			_originTableHeader.insert(item.pos, item.label);
			if ( item.pos <= _customColumn )
				++_customColumn;
		}
		else {
			_originTableHeader.append(item.label);
			item.pos = _originTableHeader.size()-1;
		}

		ProcessColumn col;
		col.pos = item.pos;
		col.script = item.script;

		if ( item.pos >= 0 && item.pos < _originColumnMap.size() ) {
			// Remap predefined columns
			for ( int i = 0; i < _originColumnMap.size(); ++i ) {
				if ( _originColumnMap[i] >= item.pos ) ++_originColumnMap[i];
			}

			// Remap process columns
			for ( int i = 0; i < _scriptColumns.size(); ++i ) {
				if ( _scriptColumns[i].pos >= item.pos )
					++_scriptColumns[i].pos;
			}
		}

		_scriptColumns.append(col);
		_scriptColumnMap[col.script] = col.pos;
	}

	if ( !_scriptColumns.isEmpty() ) {
		connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultAvailable(QString,QString,QString,QString)),
		        this, SLOT(evalResultAvailable(QString,QString,QString,QString)));
		connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultError(QString,QString,QString,int)),
		        this, SLOT(evalResultError(QString,QString,QString,int)));
	}

	_originTree = new OriginTreeWidget(this, _ui.frameOrigins);

	l = new QHBoxLayout(_ui.frameOrigins);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);
	l->addWidget(_originTree);

	_ui.frameOrigins->setLayout(l);

	_originTree->setContextMenuPolicy(Qt::CustomContextMenu);
	_originTree->setDragEnabled(true);
	_originTree->setAcceptDrops(true);
	_originTree->setRootIsDecorated(false);
	_originTree->setUniformRowHeights(true);
	_originTree->setAlternatingRowColors(true);
	_originTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_originTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	_originTree->setAutoScroll(true);

	QHeaderView* header = _originTree->header();
	header->setSortIndicatorShown(true);
	header->setSortIndicator(_originColumnMap[OL_CREATED], Qt::DescendingOrder);
	header->setSectionsClickable(true);
	connect(header, SIGNAL(sectionClicked(int)),
	        this, SLOT(sortOriginItems(int)));

	_ui.treeMagnitudes->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui.treeMagnitudes->setSelectionMode(QAbstractItemView::ExtendedSelection);
	header = _ui.treeMagnitudes->header();
	header->setSortIndicatorShown(true);
	header->setSortIndicator(MLC_TIMESTAMP, Qt::DescendingOrder);
	header->setSectionsClickable(true);
	connect(header, SIGNAL(sectionClicked(int)),
	        this, SLOT(sortMagnitudeItems(int)));

	_originTree->header()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(_originTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	        this, SLOT(currentOriginChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(_originTree, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(originTreeCustomContextMenu(QPoint)));
	connect(_originTree->header(), SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(originTreeHeaderCustomContextMenu(QPoint)));

	connect(_ui.treeMagnitudes, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	        this, SLOT(currentMagnitudeChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(_ui.treeMagnitudes, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(magnitudeTreeCustomContextMenu(QPoint)));

	connect(_ui.comboTypes, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(currentTypeChanged(int)));
	connect(_ui.comboTypeCertainties, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(currentTypeCertaintyChanged(int)));

	connect(_ui.buttonFixOrigin, SIGNAL(clicked()),
	        this, SLOT(fixOrigin()));
	connect(_ui.buttonReleaseOrigin, SIGNAL(clicked()),
	        this, SLOT(releaseOrigin()));
	connect(_ui.buttonFixMagnitudeType, SIGNAL(clicked()),
	        this, SLOT(fixMagnitudeType()));
	connect(_ui.buttonReleaseMagnitudeType, SIGNAL(clicked()),
	        this, SLOT(releaseMagnitudeType()));

	connect(_ui.buttonFixMw, SIGNAL(clicked()), this, SLOT(fixMw()));
	connect(_ui.buttonFixFmMw, SIGNAL(clicked()), this, SLOT(fixFmMw()));
	connect(_ui.buttonReleaseMw, SIGNAL(clicked()), this, SLOT(releaseMw()));

	connect(_originTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
	        this, SLOT(originSelected(QTreeWidgetItem*,int)));

	//_ui.comboFixOrigin->addItem("nothing");
	for ( int i = (int)EvaluationMode::First; i < (int)EvaluationMode::Quantity; ++i )
		_ui.comboFixOrigin->addItem(QString("%1 origins").arg(EvaluationMode::NameDispatcher::name(i)));

	_fixOriginDefaultActionCount = _ui.comboFixOrigin->count();

	_ui.fmTree->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui.fmTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

	header = _ui.fmTree->header();
	header->setSortIndicatorShown(true);
	header->setSortIndicator(_fmColumnMap[FML_CREATED], Qt::DescendingOrder);
	header->setSectionsClickable(true);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(sectionClicked(int)), this, SLOT(sortFMItems(int)));
	connect(header, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(fmTreeHeaderCustomContextMenu(QPoint)));

	connect(_ui.fmTree, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(fmTreeCustomContextMenu(QPoint)));
	connect(_ui.fmTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	        this, SLOT(currentFMChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(_ui.fmTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
	        this, SLOT(fmSelected(QTreeWidgetItem*,int)));
	connect(_ui.fmFixButton, SIGNAL(clicked()), this, SLOT(fixFM()));
	connect(_ui.fmAutoButton, SIGNAL(clicked()), this, SLOT(releaseFM()));
	connect(_ui.fmTriggerButton, SIGNAL(clicked()), this, SLOT(triggerMw()));

	for ( int i = (int)EvaluationMode::First; i < (int)EvaluationMode::Quantity; ++i )
		_ui.fmFixCombo->addItem(QString("%1 focal mechanisms").arg(EvaluationMode::NameDispatcher::name(i)));

	_fixFMDefaultActionCount = _ui.fmFixCombo->count();

	 // TODO: remove if FMAutoMode is supported
	_ui.fmFixCombo->setVisible(false);
	_ui.fmAutoButton->hide();

	_ui.fmTriggerButton->hide();

	try {
		if ( SCApp->configGetBool("eventedit.triggerFM") )
			_ui.fmTriggerButton->show();
	}
	catch ( ... ) {}

	_ui.fmTriggerButton->setEnabled(false);
	_ui.fmInfo->setMinimumWidth(270);

	_ui.mtOriginInfo->setMinimumWidth(270);

	_ui.splitter->setSizes(QList<int>() << 0 << 1);

	resetContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::setMessagingEnabled(bool e) {
	_updateLocalEPInstance = !e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::drawStations(bool e) {
	_fmMap->setDrawStations(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::drawStationAnnotations(bool e) {
	_fmMap->setDrawStationAnnotations(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Event *EventEdit::currentEvent() const {
	return _currentEvent.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::handleOrigins(const QList<DataModel::Origin*> &origins) {
	mergeOrigins(origins);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::onObjectAdded(Object* parent, Object* newChild) {
	if ( _blockObserver ) return;

	// Only allow modifications in this thread
	if ( QThread::currentThread() != thread() ) return;

	PublicObject *po = PublicObject::Cast(parent);
	addObject(po?po->publicID().c_str():"", newChild);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::onObjectRemoved(Object* parent, Object* oldChild) {
	if ( _blockObserver ) return;

	// Only allow modifications in this thread
	if ( QThread::currentThread() != thread() ) return;

	PublicObject *po = PublicObject::Cast(parent);
	removeObject(po?po->publicID().c_str():"", oldChild);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::onObjectModified(Object* object) {
	if ( _blockObserver ) return;

	// Only allow modifications in this thread
	if ( QThread::currentThread() != thread() ) return;

	updateObject(object->parent()?object->parent()->publicID().c_str():"", object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::addObject(const QString &parentID, Object *obj) {
	if ( !_currentEvent ) return;

	if ( parentID == _currentEvent->publicID().c_str() ) {
		OriginReference *ref = OriginReference::Cast(obj);
		if ( ref ) {
			for ( int i = 0; i < _originTree->topLevelItemCount(); ++i )
				if ( _originTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toStdString() == ref->originID() )
					return;

			OriginPtr org = Origin::Find(ref->originID());
			if ( !org && _reader )
				org = Origin::Cast(_reader->getObject(Origin::TypeInfo(), ref->originID()));

			if ( !org ) return;

			if ( org->magnitudeCount() == 0 && _reader )
				_reader->loadMagnitudes(org.get());

			storeOrigin(org.get());
			insertOriginRow(org.get());

			if ( _preferredOriginIdx != -1 )
				++_preferredOriginIdx;

			for (int i = 0; i < _originTree->columnCount(); i++)
				_originTree->resizeColumnToContents(i);

			sortOriginItems(_originTree->header()->sortIndicatorSection());

			return;
		}

		FocalMechanismReference *fmRef = FocalMechanismReference::Cast(obj);
		if ( fmRef ) {
			for ( int i = 0; i < _ui.fmTree->topLevelItemCount(); ++i )
				if ( _ui.fmTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toStdString() == fmRef->focalMechanismID() )
					return;

			FocalMechanismPtr fm = FocalMechanism::Find(fmRef->focalMechanismID());
			if ( !fm && _reader )
				fm = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), fmRef->focalMechanismID()));

			if ( !fm ) return;

			if ( fm->momentTensorCount() == 0 && _reader ) {
				_reader->loadMomentTensors(fm.get());
				for ( size_t i = 0; i < fm->momentTensorCount(); ++i ) {
					std::string derivedID = fm->momentTensor(i)->derivedOriginID();
					OriginPtr derived = Origin::Find(derivedID);
					if ( !derived )
						derived = Origin::Cast(_reader->getObject(Origin::TypeInfo(), derivedID));
					if ( derived ) {
						if ( derived->magnitudeCount() == 0 && _reader )
							_reader->loadMagnitudes(derived.get());
						storeDerivedOrigin(derived.get());
					}
				}
			}

			storeFM(fm.get());
			insertFMRow(fm.get());

			if ( _preferredFMIdx != -1 )
				++_preferredFMIdx;

			for (int i = 0; i < _ui.fmTree->columnCount(); i++)
				_ui.fmTree->resizeColumnToContents(i);

			sortFMItems(_ui.fmTree->header()->sortIndicatorSection());

			return;
		}

	}
	else if ( _currentOrigin && parentID == _currentOrigin->publicID().c_str() ) {
		Magnitude *mag = Magnitude::Cast(obj);
		if ( mag ) {
			addMagnitude(mag);

			for ( int i = 0; i < _ui.treeMagnitudes->columnCount(); i++ )
				_ui.treeMagnitudes->resizeColumnToContents(i);

			return;
		}
	}
	else if ( _currentFM && parentID == _currentFM->publicID().c_str() ) {
		MomentTensor *mt = MomentTensor::Cast(obj);
		if ( mt ) {
			std::string derivedID = mt->derivedOriginID();
			OriginPtr derived = Origin::Find(derivedID);
			if ( !derived && _reader )
				derived = Origin::Cast(_reader->getObject(Origin::TypeInfo(), derivedID));
			if ( derived ) {
				if ( derived->magnitudeCount() == 0 && _reader )
					_reader->loadMagnitudes(derived.get());
				storeDerivedOrigin(derived.get());
			}

			return;
		}
	}
	else {
		Comment *comment = Comment::Cast(obj);
		if ( comment ) {
			Origin *org = Origin::Find((const char*)parentID.toLatin1());
			if ( org ) updateOrigin(org);
			return;
		}

		JournalEntry *entry = JournalEntry::Cast(obj);
		if ( entry && entry->objectID() == _currentEvent->publicID() ) {
			addJournal(entry);
			return;
		}
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateObject(const QString &parentID, Object *obj) {
	if ( !_currentEvent ) return;

	Event *ev = Event::Cast(obj);
	if ( ev ) {
		if ( _currentEvent->publicID() == ev->publicID() ) {
			bool changePreferredOrigin = true;
			bool changePreferredMagnitude = true;
			bool changePreferredFM = true;

			if ( _preferredOriginIdx != -1 ) {
				if ( _currentEvent->preferredOriginID() == (const char*)_originTree->topLevelItem(
				         _preferredOriginIdx)->data(0, Qt::UserRole).toString().toLatin1() )
					changePreferredOrigin = false;
			}

			if ( changePreferredOrigin ) {
				updatePreferredOriginIndex();
				_fmMap->setEvent(_currentEvent.get());
			}

			if ( _preferredMagnitudeIdx != -1 ) {
				if ( _currentEvent->preferredMagnitudeID() == (const char*)_ui.treeMagnitudes->topLevelItem(
				         _preferredMagnitudeIdx)->data(0, Qt::UserRole).toString().toLatin1() )
					changePreferredMagnitude = false;
			}

			if ( changePreferredMagnitude )
				updatePreferredMagnitudeIndex();

			if ( _preferredFMIdx != -1 ) {
				if ( _currentEvent->preferredFocalMechanismID() == (const char*)_ui.fmTree->topLevelItem(
				         _preferredFMIdx)->data(0, Qt::UserRole).toString().toLatin1() )
					changePreferredFM = false;
			}

			if ( changePreferredFM )
				updatePreferredFMIndex();

			updateEvent();

			return;
		}
	}

	Origin *org = Origin::Cast(obj);
	if ( org ) {
		updateOrigin(org);
		return;
	}

	Magnitude *mag = Magnitude::Cast(obj);
	if ( mag ) {
		Magnitude *publicMag = Magnitude::Find(mag->publicID());
		if ( _currentOrigin && publicMag && publicMag->origin() == _currentOrigin ) {
			for ( int i = 0; i < _ui.treeMagnitudes->topLevelItemCount(); ++i ) {
				if ( mag->publicID() == (const char*)_ui.treeMagnitudes->topLevelItem(i)->data(0, Qt::UserRole).toString().toLatin1() ) {
					updateMagnitudeRow(i, mag);
					//_ui.treeMagnitudes->topLevelItem(i)->setText(0, QString("%1: %2").arg(mag->type().c_str()).arg(mag->magnitude().value(), 0, 'f', 2));
				}
			}
		}

		if ( _currentMagnitude && _currentMagnitude->publicID() == mag->publicID() )
			updateMagnitude();
		return;

		// TODO: update MT Magnitude??
	}

	FocalMechanism *fm = FocalMechanism::Cast(obj);
	if ( fm ) {
		updateFM(fm);
	}

	MomentTensor *mt = MomentTensor::Cast(obj);
	if ( mt ) {
		if ( _currentMT && _currentMT->publicID() == mt->publicID() )
			updateMT();
		return;
	}

	Comment *comment = Comment::Cast(obj);
	if ( comment ) {
		Origin *org = Origin::Find((const char*)parentID.toLatin1());
		if ( org ) updateOrigin(org);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::removeObject(const QString& parentID, Object* obj) {
	if ( !_currentEvent ) return;
	if ( parentID == _currentEvent->publicID().c_str() ) {
		OriginReference *ref = OriginReference::Cast(obj);
		if ( ref ) {
			bool foundRow = false;
			for ( int i = 0; i < _originTree->topLevelItemCount(); ++i ) {
				auto item = _originTree->topLevelItem(i);
				if ( item->data(0, Qt::UserRole).toString().toStdString() == ref->originID() ) {
					foundRow = true;
					delete item;
					break;
				}
			}

			if ( foundRow ) {
				for ( auto it = _origins.begin(); it != _origins.end(); ++it) {
					if ( (*it)->publicID() == ref->originID() ) {
						PublicObjectEvaluator::Instance().erase(this, (*it)->publicID().c_str());
						_origins.erase(it);
						break;
					}
				}

				currentOriginChanged(_originTree->currentItem(), nullptr);
			}

			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updatePreferredOriginIndex() {
	if ( _preferredOriginIdx != -1 ) {
		QTreeWidgetItem *item = _originTree->topLevelItem(_preferredOriginIdx);
		for ( int c = 0; c < item->columnCount(); ++c ) {
			QFont f = item->font(c);
			f.setBold(false);
			item->setFont(c, f);
		}
	}

	for ( int i = 0; i < _originTree->topLevelItemCount(); ++i ) {
		if ( _currentEvent->preferredOriginID() == (const char*)_originTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toLatin1() ) {
			QTreeWidgetItem *item = _originTree->topLevelItem(i);
			for ( int c = 0; c < _originColumnMap.count(); ++c ) {
				QFont f = item->font(_originColumnMap[c]);
				f.setBold(OriginColBold[c]);
				item->setFont(_originColumnMap[c], f);
			}
			_preferredOriginIdx = i;
			break;
		}
	}

	for ( int i = 0; i < _originTree->columnCount(); ++i )
		_originTree->resizeColumnToContents(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updatePreferredMagnitudeIndex() {
	if ( _preferredMagnitudeIdx != -1 ) {
		QTreeWidgetItem *item = _ui.treeMagnitudes->topLevelItem(_preferredMagnitudeIdx);
		for ( int c = 0; c < item->columnCount(); ++c ) {
			QFont f = item->font(c);
			f.setBold(false);
			item->setFont(c, f);
		}
	}

	for ( int i = 0; i < _ui.treeMagnitudes->topLevelItemCount(); ++i ) {
		if ( _currentEvent->preferredMagnitudeID() ==
		     (const char*)_ui.treeMagnitudes->topLevelItem(i)->data(0, Qt::UserRole).toString().toLatin1() ) {
			QTreeWidgetItem *item = _ui.treeMagnitudes->topLevelItem(i);
			for ( int c = 0; c < item->columnCount(); ++c ) {
				QFont f = item->font(c);
				f.setBold(MagnitudeColBold[c]);
				item->setFont(c, f);
			}
			_preferredMagnitudeIdx = i;
			break;
		}
	}

	for (int i = 0; i < _ui.treeMagnitudes->columnCount(); i++)
		_ui.treeMagnitudes->resizeColumnToContents(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updatePreferredFMIndex() {
	if ( _preferredFMIdx != -1 ) {
		QTreeWidgetItem *item = _ui.fmTree->topLevelItem(_preferredFMIdx);
		for ( int c = 0; c < item->columnCount(); ++c ) {
			QFont f = item->font(c);
			f.setBold(false);
			item->setFont(c, f);
		}
	}

	for ( int i = 0; i < _ui.fmTree->topLevelItemCount(); ++i ) {
		if ( _currentEvent->preferredFocalMechanismID() ==
		     (const char*)_ui.fmTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toLatin1() ) {
			QTreeWidgetItem *item = _ui.fmTree->topLevelItem(i);
			for ( int c = 0; c < _fmColumnMap.count(); ++c ) {
				QFont f = item->font(_fmColumnMap[c]);
				f.setBold(FMColBold[c]);
				item->setFont(_fmColumnMap[c], f);
			}
			_preferredFMIdx = i;
			break;
		}
	}

	for ( int i = 0; i < _ui.fmTree->columnCount(); ++i )
		_ui.fmTree->resizeColumnToContents(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::setFMActivity(bool e) {
	if ( e && _fmActivityMovie != nullptr ) return;
	if ( !e && _fmActivityMovie == nullptr ) return;

	if ( e ) {
		if ( _fmActivityMovie != nullptr ) delete _fmActivityMovie;

		_fmActivityMovie = new QMovie(this);
		_fmActivityMovie->setFileName(":/sc/assets/mt.mng");

		_fmActivity->show();
		_fmActivity->setMovie(_fmActivityMovie);
		_fmActivityMovie->start();
	}
	else {
		_fmActivity->hide();
		_fmActivity->setMovie(nullptr);

		if ( _fmActivityMovie != nullptr ) delete _fmActivityMovie;
		_fmActivityMovie = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::setEvent(Event *event, Origin *origin) {
	bool eventChanged = false;

	if ( _currentEvent != event ) {
		PublicObjectEvaluator::Instance().clear(this);

		setFMActivity(false);

		eventChanged = true;
		_currentEvent = event;

		clearOrigins();
		clearFMs();

		_currentOrigin = nullptr;
		_currentFM = nullptr;

		if ( _currentEvent == nullptr ) {
			resetContent();
			return;
		}

		_blockObserver = true;

		if ( _currentEvent->originReferenceCount() == 0 && _reader ) {
			_reader->loadOriginReferences(_currentEvent.get());
			clearOrigins();
			_currentOrigin = nullptr;
		}

		if ( _currentEvent->focalMechanismReferenceCount() == 0 && _reader ) {
			_reader->loadFocalMechanismReferences(_currentEvent.get());
			clearFMs();
			_currentFM = nullptr;
		}

		for ( size_t i = 0; i < _currentEvent->originReferenceCount(); ++i ) {
			OriginPtr org = Origin::Find(_currentEvent->originReference(i)->originID());
			if ( !org && _reader )
				org = Origin::Cast(_reader->getObject(Origin::TypeInfo(), _currentEvent->originReference(i)->originID()));

			if ( !org ) continue;

			if ( org->magnitudeCount() == 0 && _reader )
				_reader->loadMagnitudes(org.get());

			storeOrigin(org.get());
		}

		for ( int i = 0; i < _originTree->columnCount(); ++i )
			_originTree->resizeColumnToContents(i);

		for ( size_t i = 0; i < _currentEvent->focalMechanismReferenceCount(); ++i ) {
			FocalMechanismReference *ref = _currentEvent->focalMechanismReference(i);
			FocalMechanismPtr fm = FocalMechanism::Find(ref->focalMechanismID());
			if ( !fm && _reader )
				fm = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), ref->focalMechanismID()));

			if ( !fm ) continue;

			if ( fm->momentTensorCount() == 0 && _reader )
				_reader->loadMomentTensors(fm.get());

			for ( size_t i = 0; i < fm->momentTensorCount(); ++i ) {
				std::string derivedID = fm->momentTensor(i)->derivedOriginID();
				OriginPtr derived = Origin::Find(derivedID);
				if ( !derived && _reader )
					derived = Origin::Cast(_reader->getObject(Origin::TypeInfo(), derivedID));
				if ( derived ) {
					if ( derived->magnitudeCount() == 0 && _reader )
						_reader->loadMagnitudes(derived.get());
					storeDerivedOrigin(derived.get());
				}
			}

			storeFM(fm.get());
		}

		for ( int i = 0; i < _ui.fmTree->columnCount(); ++i )
			_ui.fmTree->resizeColumnToContents(i);

		_blockObserver = false;
		updateContent();
	}

	if ( _currentEvent == nullptr ) return;

	if ( origin == nullptr )
		origin = Origin::Find(_currentEvent->preferredOriginID());

	// Preselect the requested origin
	for ( int i = 0; i < _originTree->topLevelItemCount(); ++i ) {
		if ( _originTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toStdString() == origin->publicID() ) {
			_currentOrigin = origin;
			_originTree->setCurrentItem(_originTree->topLevelItem(i));
			break;
		}
	}

	// Preselect preferred FM, or if unset first in list
	if ( eventChanged && _ui.fmTree->topLevelItemCount() > 0 ) {
		FocalMechanism *fm = FocalMechanism::Find(_currentEvent->preferredFocalMechanismID());
		if ( fm ) {
			for ( int i = 0; i < _ui.fmTree->topLevelItemCount(); ++i ) {
				if ( _ui.fmTree->topLevelItem(i)->data(0, Qt::UserRole).toString().toStdString() == fm->publicID() ) {
					_ui.fmTree->setCurrentItem(_ui.fmTree->topLevelItem(i));
					break;
				}
			}
		}
		else
			_ui.fmTree->setCurrentItem(_ui.fmTree->topLevelItem(0));
	}

	_ui.tabWidget->setCurrentIndex(0);

	// TODO: enable/disalbe QTabBar if event has/has not fm references
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateOrigin(Origin *org) {
	if ( org == nullptr ) return;

	if ( _currentOrigin && _currentOrigin->publicID() == org->publicID() )
		updateOrigin();

	QString oid = org->publicID().c_str();
	for ( int i = 0; i < _originTree->topLevelItemCount(); ++i ) {
		if ( _originTree->topLevelItem(i)->data(0, Qt::UserRole).toString() == oid ) {
			updateOriginRow(i, org);
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateFM(FocalMechanism *fm) {
	if ( fm == nullptr ) return;

	if ( _currentFM && _currentFM->publicID() == fm->publicID() )
		updateFM();

	QString fmid = fm->publicID().c_str();
	for ( int i = 0; i < _ui.fmTree->topLevelItemCount(); ++i ) {
		if ( _ui.fmTree->topLevelItem(i)->data(0, Qt::UserRole).toString() == fmid ) {
			updateFMRow(i, fm);
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::showTab(int i) {
	_ui.tabWidget->setCurrentIndex(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::insertOriginRow(Origin *org) {
	QTreeWidgetItem *item = new QTreeWidgetItem;

	for ( int i = 0; i < OriginListColumns::Quantity; ++i )
		item->setTextAlignment(_originColumnMap[i], OriginColAligns[i]);
	if ( _customColumn >= 0 )
		item->setTextAlignment(_customColumn, Qt::AlignCenter);

	// Register script calls
	if ( !_scriptColumns.empty() ) {
		QStringList scripts;
		QHash<QString, int>::iterator it;
		for ( it = _scriptColumnMap.begin(); it != _scriptColumnMap.end(); ++it ) {
			item->setBackground(it.value(), SCScheme.colors.records.gaps);
			item->setTextAlignment(it.value(), Qt::AlignCenter);
			scripts << it.key();
		}

		PublicObjectEvaluator::Instance().prepend(this, org->publicID().c_str(),
		                                          org->typeInfo().className(), scripts);
	}

	_originTree->insertTopLevelItem(0, item);

	OriginSymbol *symbol = new OriginSymbol;
	symbol->setID(org->publicID());
	symbol->setLatitude(org->latitude());
	symbol->setLongitude(org->longitude());
	try {
		symbol->setDepth(org->depth());
	}
	catch ( Core::ValueException & ) {}


	if ( org->magnitudeCount() > 0 )
		symbol->setPreferredMagnitudeValue(org->magnitude(0)->magnitude());

	_originMap->canvas().symbolCollection()->add(symbol);

	if ( _originBoundings.isNull() )
		_originBoundings.setRect(org->longitude()-0.01, org->latitude()-0.01, 0.02, 0.02);
	else {
		if ( org->latitude() < _originBoundings.top() )
			_originBoundings.setTop(org->latitude());
		else if ( org->latitude() > _originBoundings.bottom() )
			_originBoundings.setBottom(org->latitude());

		double dist = subGeo(org->longitude().value(), _originBoundings.left());
		if ( dist < 0 )
			_originBoundings.setLeft(_originBoundings.left()+dist);
		else if ( dist > _originBoundings.width() )
			_originBoundings.setRight(_originBoundings.left()+dist);
	}

	_originMap->canvas().displayRect(_originBoundings.adjusted(-0.5,-0.5,0.5,0.5));

	updateOriginRow(0, org);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateOriginRow(int row, Origin *org) {
	QTreeWidgetItem *item = _originTree->topLevelItem(row);

	item->setData(0, Qt::UserRole, QString(org->publicID().c_str()));
	item->setText(_originColumnMap[OL_TIME], timeToString(org->time().value(), TableOTimeFormat.c_str()));
	item->setText(_originColumnMap[OL_LAT], QString("%1 %2").arg(fabs(org->latitude()), 0, 'f', SCScheme.precision.location).arg(org->latitude() < 0?"S":"N"));
	item->setData(_originColumnMap[OL_LAT], Qt::UserRole, org->latitude().value());
	item->setText(_originColumnMap[OL_LON], QString("%1 %2").arg(fabs(org->longitude()), 0, 'f', SCScheme.precision.location).arg(org->longitude() < 0?"W":"E"));
	item->setData(_originColumnMap[OL_LON], Qt::UserRole, org->longitude().value());

	try {
		item->setText(_originColumnMap[OL_DEPTH], depthToString(org->depth(), SCScheme.precision.depth) + " km");
		item->setData(_originColumnMap[OL_DEPTH], Qt::UserRole, org->depth().value());
	}
	catch ( ... ) {
		item->setText(_originColumnMap[OL_DEPTH], "-");
		item->setData(_originColumnMap[OL_DEPTH], Qt::UserRole, QVariant());
	}

	try {
		item->setText(_originColumnMap[OL_DEPTH_TYPE], org->depthType().toString());
	}
	catch ( ... ) {
		item->setText(_originColumnMap[OL_DEPTH_TYPE], "-");
	}

	char stat = objectStatusToChar(org);
	item->setText(_originColumnMap[OL_STAT], QString("%1").arg(stat));

	try {
		switch ( org->evaluationMode() ) {
			case DataModel::AUTOMATIC:
				item->setForeground(_originColumnMap[OL_STAT], SCScheme.colors.originStatus.automatic);
				break;
			case DataModel::MANUAL:
				item->setForeground(_originColumnMap[OL_STAT], SCScheme.colors.originStatus.manual);
				break;
			default:
				break;
		};
	}
	catch ( ... ) {
		item->setForeground(_originColumnMap[OL_STAT], SCScheme.colors.originStatus.automatic);
	}

	try {
		item->setText(_originColumnMap[OL_PHASES], QString("%1").arg(org->quality().usedPhaseCount()));
		item->setData(_originColumnMap[OL_PHASES], Qt::UserRole, org->quality().usedPhaseCount());
	}
	catch ( ... ) {
		item->setText(_originColumnMap[OL_PHASES], "-");
		item->setData(_originColumnMap[OL_PHASES], Qt::UserRole, QVariant());
	}
	try {
		item->setText(_originColumnMap[OL_RMS], QString("%1").arg(fabs(org->quality().standardError()), 0, 'f', 1));
		item->setData(_originColumnMap[OL_RMS], Qt::UserRole, org->quality().standardError());
	}
	catch ( ... ) {
		item->setText(_originColumnMap[OL_RMS], "-");
		item->setData(_originColumnMap[OL_RMS], Qt::UserRole, QVariant());
	}
	try {
		item->setText(_originColumnMap[OL_AZGAP], QString("%1").arg(org->quality().azimuthalGap(), 0, 'f', 0));
		item->setData(_originColumnMap[OL_AZGAP], Qt::UserRole, org->quality().azimuthalGap());
	}
	catch ( ... ) {
		item->setText(_originColumnMap[OL_AZGAP], "-");
		item->setData(_originColumnMap[OL_AZGAP], Qt::UserRole, QVariant());
	}
	try {
		item->setText(_originColumnMap[OL_CREATED], timeToString(org->creationInfo().creationTime(), TableCTimeFormat.c_str()));
	}
	catch ( ... ) {
		item->setText(_originColumnMap[OL_CREATED], "");
	}

	POPULATE_AGENCY(_originColumnMap[OL_AGENCY], org->creationInfo().agencyID());
	item->setText(_originColumnMap[OL_METHOD], org->methodID().c_str());
	item->setText(_originColumnMap[OL_AUTHOR], objectAuthor(org).c_str());
	item->setText(_originColumnMap[OL_REGION], Regions::getRegionName(org->latitude(),org->longitude()).c_str());
	item->setText(_originColumnMap[OL_ID], org->publicID().c_str());

	if ( _customColumn >= 0 ) {
		item->setText(_customColumn, _customDefaultText);
		for ( size_t i = 0; i < org->commentCount(); ++i ) {
			Comment *c = org->comment(i);
			if ( c->id() != _commentID ) continue;

			item->setText(_customColumn, c->text().c_str());
			auto it = _customColorMap.find(c->text());
			if ( it != _customColorMap.end() ) {
				item->setData(_customColumn, Qt::ForegroundRole, it.value());
			}
			break;
		}
	}

	item->setForeground(_originColumnMap[OL_CREATED], palette().color(QPalette::Disabled, QPalette::QPalette::Text));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateMagnitudeRow(int row, Magnitude *mag) {
	QTreeWidgetItem *item = _ui.treeMagnitudes->topLevelItem(row);

	item->setData(0, Qt::UserRole, QString(mag->publicID().c_str()));

	try {
		item->setText(MLC_TIMESTAMP, timeToString(mag->creationInfo().creationTime(), TableCTimeFormat.c_str()));
	}
	catch ( ... ) {
		item->setText(MLC_TIMESTAMP, "");
	}

	item->setText(MLC_VALUE, QString("%1").arg(mag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude));
	item->setText(MLC_TYPE, mag->type().c_str());

	try {
		item->setText(MLC_NUM, QString("%1").arg(mag->stationCount()));
	}
	catch ( ... ) {
		item->setText(MLC_NUM, "");
	}

	try {
		item->setText(MLC_RMS, QString("%1").arg(quantityUncertainty(mag->magnitude()), 0, 'f', 1));
	}
	catch ( ... ) {
		item->setText(MLC_RMS, "");
	}

	char stat = objectEvaluationStatusToChar(mag);
	if ( stat )
		item->setText(MLC_STAT, QString("%1").arg(stat));
	else
		item->setText(MLC_STAT, QString());
	item->setText(MLC_AGENCY, objectAgencyID(mag).c_str());
	item->setText(MLC_AUTHOR, objectAuthor(mag).c_str());
	item->setText(MLC_ID, mag->publicID().c_str());

	item->setForeground(MLC_TIMESTAMP, palette().color(QPalette::Disabled, QPalette::QPalette::Text));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::insertFMRow(FocalMechanism *fm) {
	QTreeWidgetItem *item = new QTreeWidgetItem;

	for ( int i = 0; i < FMListColumns::Quantity; ++i ) {
		item->setTextAlignment(_fmColumnMap[i], FMColAligns[i]);
	}

	_ui.fmTree->insertTopLevelItem(0, item);
	updateFMRow(0, fm);

	_fmMap->addFM(fm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateFMRow(int row, FocalMechanism *fm) {
	QTreeWidgetItem *item = _ui.fmTree->topLevelItem(row);

	item->setData(0, Qt::UserRole, QString(fm->publicID().c_str()));

	RESET_COLUMN(FML_DEPTH);
	RESET_COLUMN(FML_MAG);

	if ( fm->momentTensorCount() > 0 ) {
		auto mt = fm->momentTensor(0);
		auto o = Origin::Find(mt->derivedOriginID());
		if ( o ) {
			POPULATE_COLUMN(FML_DEPTH, (depthToString(o->depth(), SCScheme.precision.depth)), o->depth().value());
		}
		auto m = Magnitude::Find(mt->momentMagnitudeID());
		if ( m ) {
			POPULATE_COLUMN_DOUBLE(FML_MAG, m->magnitude().value(), SCScheme.precision.magnitude);
		}

		if ( !mt->momentTensorStationContributionCount() && _reader ) {
			_reader->loadMomentTensorStationContributions(mt);
			for ( size_t i = 0; i < mt->momentTensorStationContributionCount(); ++i ) {
				_reader->load(mt->momentTensorStationContribution(i));
			}
		}
	}

	POPULATE_COLUMN_DOUBLE(FML_AZGAP, fm->azimuthalGap(), 0);

	try {
		item->setText(_fmColumnMap[FML_COUNT], QString("%1").arg(fm->stationPolarityCount()));
		item->setData(_fmColumnMap[FML_COUNT], Qt::UserRole, fm->stationPolarityCount());
	}
	catch ( Core::ValueException& ) {
		try {
			if ( fm->momentTensorCount() > 0 ) {
				auto mt = fm->momentTensor(0);
				auto o = Origin::Find(mt->derivedOriginID());
				if ( o ) {
					item->setText(_fmColumnMap[FML_COUNT], QString("%1").arg(o->quality().usedPhaseCount()));
					item->setData(_fmColumnMap[FML_COUNT], Qt::UserRole, o->quality().usedPhaseCount());
				}
			}
			else {
				throw Core::ValueException();
			}
		}
		catch ( Core::ValueException& ) {
			item->setText(_fmColumnMap[FML_COUNT], "-");
			item->setData(_fmColumnMap[FML_COUNT], Qt::UserRole, QVariant());
		}
	}

	try {
		if ( fm->momentTensorCount() > 0 ) {
			auto mt = fm->momentTensor(0);
			auto m = Magnitude::Find(mt->momentMagnitudeID());
			if ( m ) {
				item->setText(_fmColumnMap[FML_MCOUNT], QString("%1").arg(m->stationCount()));
				item->setData(_fmColumnMap[FML_MCOUNT], Qt::UserRole, m->stationCount());
			}
			else {
				throw Core::ValueException();
			}
		}
		else {
			throw Core::ValueException();
		}
	}
	catch ( Core::ValueException& ) {
		item->setText(_fmColumnMap[FML_MCOUNT], "-");
		item->setData(_fmColumnMap[FML_MCOUNT], Qt::UserRole, QVariant());
	}

	// Strike, dip, rake
	POPULATE_COLUMN_DOUBLE(FML_STRIKE1, fm->nodalPlanes().nodalPlane1().strike().value(), 0);
	POPULATE_COLUMN_DOUBLE(FML_DIP1, fm->nodalPlanes().nodalPlane1().dip().value(), 0);
	POPULATE_COLUMN_DOUBLE(FML_RAKE1, fm->nodalPlanes().nodalPlane1().rake().value(), 0);

	POPULATE_COLUMN_DOUBLE(FML_STRIKE2, fm->nodalPlanes().nodalPlane2().strike().value(), 0);
	POPULATE_COLUMN_DOUBLE(FML_DIP2, fm->nodalPlanes().nodalPlane2().dip().value(), 0);
	POPULATE_COLUMN_DOUBLE(FML_RAKE2, fm->nodalPlanes().nodalPlane2().rake().value(), 0);

	// DC, ISO, CLVD
	if ( fm->momentTensorCount() > 0 ) {
		POPULATE_COLUMN_DOUBLE(FML_DC, fm->momentTensor(0)->doubleCouple()*100.0, 0);
		POPULATE_COLUMN_DOUBLE(FML_CLVD, fm->momentTensor(0)->clvd()*100.0, 0);
		POPULATE_COLUMN_DOUBLE(FML_ISO, fm->momentTensor(0)->iso()*100.0, 0);
	}
	else {
		item->setText(_fmColumnMap[FML_DC], "-");
		item->setData(_fmColumnMap[FML_DC], Qt::UserRole, QVariant());
		item->setText(_fmColumnMap[FML_ISO], "-");
		item->setData(_fmColumnMap[FML_ISO], Qt::UserRole, QVariant());
		item->setText(_fmColumnMap[FML_CLVD], "-");
		item->setData(_fmColumnMap[FML_CLVD], Qt::UserRole, QVariant());
	}

	// Misfit
	POPULATE_COLUMN_DOUBLE(FML_MISFIT, fm->misfit(), 2);
	POPULATE_COLUMN_DOUBLE(FML_STDR, fm->stationDistributionRatio(), 2);

	char stat = objectStatusToChar(fm);
	item->setText(_fmColumnMap[FML_STAT], QString("%1").arg(stat));

	try {
		switch ( fm->evaluationMode() ) {
			case DataModel::AUTOMATIC:
				item->setForeground(_fmColumnMap[FML_STAT], SCScheme.colors.originStatus.automatic);
				break;
			case DataModel::MANUAL:
				item->setForeground(_fmColumnMap[FML_STAT], SCScheme.colors.originStatus.manual);
				break;
			default:
				break;
		};
	}
	catch ( ... ) {
		item->setForeground(_fmColumnMap[FML_STAT], SCScheme.colors.originStatus.automatic);
	}

	try {
		item->setText(_fmColumnMap[FML_CREATED], timeToString(fm->creationInfo().creationTime(), TableCTimeFormat.c_str()));
	}
	catch ( ... ) {
		item->setText(_fmColumnMap[FML_CREATED], "");
	}

	POPULATE_AGENCY(_fmColumnMap[FML_AGENCY], fm->creationInfo().agencyID());
	item->setText(_fmColumnMap[FML_AUTHOR], objectAuthor(fm).c_str());
	item->setText(_fmColumnMap[FML_ID], fm->publicID().c_str());

	item->setForeground(_fmColumnMap[OL_CREATED], palette().color(QPalette::Disabled, QPalette::QPalette::Text));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateContent() {
	if ( _currentEvent == nullptr ) {
		resetContent();
		return;
	}

	setEnabled(true);

	// origin
	_originTree->blockSignals(true);
	_preferredOriginIdx = -1;

	_originBoundings = QRectF();
	_originMap->canvas().symbolCollection()->clear();
	_originTree->clear();
	_originTree->setColumnCount(OriginListColumns::Quantity);
	_originTree->setHeaderLabels(_originTableHeader);
	//_originTree->header()->setResizeMode(QHeaderView::Stretch);

	for ( int i = 0; i < OriginListColumns::Quantity; ++i )
		_originTree->header()->setSectionHidden(_originColumnMap[i], !OriginColVisible[i]);

	for ( OriginList::iterator it = _origins.begin(); it != _origins.end(); ++it )
		insertOriginRow(it->get());

	updatePreferredOriginIndex();
	_originTree->blockSignals(false);
	sortOriginItems(_originTree->header()->sortIndicatorSection());

	// focal mechanism
	_ui.fmTriggerButton->setEnabled(true);
	_ui.fmTree->blockSignals(true);
	_preferredFMIdx = -1;

	_fmMap->clear();
	_ui.fmTree->clear();
	_ui.fmTree->setColumnCount(FMListColumns::Quantity);
	_ui.fmTree->setHeaderLabels(_fmTableHeader);

	for ( int i = 0; i < FMListColumns::Quantity; ++i ) {
		_ui.fmTree->header()->setSectionHidden(_fmColumnMap[i], !FMColVisible[i]);
	}

	for ( FMList::iterator it = _fms.begin(); it != _fms.end(); ++it ) {
		insertFMRow(it->get());
	}

	updatePreferredFMIndex();
	_ui.fmTree->blockSignals(false);
	sortFMItems(_ui.fmTree->header()->sortIndicatorSection());
	_fmMap->setEvent(_currentEvent.get());

	updateEvent();
	updateJournal();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::resetContent() {
	_ui.labelTimeValue->setText("-");
	_preferredOriginIdx = -1;
	_preferredFMIdx = -1;

	resetOrigin();
	resetFM();

	_originBoundings = QRectF();
	_originMap->canvas().symbolCollection()->clear();
	_fmMap->clear();
	_fmMap->setEvent(nullptr);
	_originTree->clear();
	_ui.fmTree->clear();
	_ui.listJournal->clear();
	_ui.fmTriggerButton->setEnabled(false);

	_fmActivity->setMovie(nullptr);
	_fmActivity->hide();

	setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::resetOrigin() {
	_ui.labelRegionValue->setText("-");
	_ui.labelDepthValue->setText("-");
	_ui.labelDepthError->setText("");

	_ui.labelLatitudeValue->setText("-");
	_ui.labelLatitudeUnit->setText("");
	_ui.labelLatitudeError->setText("");

	_ui.labelLongitudeValue->setText("-");
	_ui.labelLongitudeUnit->setText("");
	_ui.labelLongitudeError->setText("");

	_ui.labelPhasesValue->setText("-");
	_ui.labelRMSValue->setText("-");
	_ui.labelAgencyValue->setText("");
	_ui.labelOriginStatusValue->setText("");

	_ui.treeMagnitudes->clear();

	_ui.buttonFixOrigin->setEnabled(false);
	_ui.comboFixOrigin->setEnabled(false);
	while ( _ui.comboFixOrigin->count() > _fixOriginDefaultActionCount ) {
		_ui.comboFixOrigin->setCurrentIndex(0);
		_ui.comboFixOrigin->removeItem(_ui.comboFixOrigin->count()-1);
	}

	_preferredMagnitudeIdx = -1;

	resetMagnitude();

	Map::SymbolLayer::iterator begin = _originMap->canvas().symbolCollection()->begin();
	Map::SymbolLayer::iterator end = _originMap->canvas().symbolCollection()->end();
	Map::SymbolLayer::iterator it = begin;

	_originMap->canvas().symbolCollection()->setTop(nullptr);
	for ( ; it != end; ++it )
		static_cast<OriginSymbol*>(*it)->setFilled(false);

	_originMap->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::resetMagnitude() {
	_ui.labelMagnitudeTypeValue->setText("-");
	_ui.labelMagnitudeValue->setText("-");
	_ui.labelMagnitudeError->setText("");
	_ui.labelMagnitudeCountValue->setText("-");
	_ui.labelMagnitudeMethodValue->setText("");
	_ui.labelMagnitudeStatus->setText(QString());

	_ui.buttonFixMagnitudeType->setEnabled(false);
	_ui.buttonReleaseMagnitudeType->setEnabled(false);

	_currentMagnitude = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::resetFM() {
	_ui.fmNP1->setText("-");
	_ui.fmNP2->setText("-");
	_ui.fmGap->setText("-");
	_ui.fmCount->setText("-");
	_ui.fmMisfit->setText("-");
	_ui.fmDist->setText("-");
	_ui.fmMethod->setText("-");
	_ui.fmMode->setText("-");
	_ui.fmStatus->setText("-");

	_ui.fmFixButton->setEnabled(false);
	_ui.fmFixCombo->setEnabled(false);
	while ( _ui.fmFixCombo->count() > _fixFMDefaultActionCount )
		_ui.fmFixCombo->removeItem(_ui.fmFixCombo->count()-1);

	resetMT(true);

	_fmMap->setCurrentFM("");
	_fmMap->canvas().displayRect(QRectF(-180,-90,360,180));
	_fmMap->setEnabled(false);
	_fmMap->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::resetMT(bool resetCurrent) {
	if ( resetCurrent ) {
		_currentMT = nullptr;
		_ui.mtOriginInfo->setEnabled(false);
		_ui.mtMagInfo->setEnabled(false);
	}

	_ui.mtOriginTime->setText("-");
	_ui.mtOriginRegion->setText("-");
	_ui.mtOriginDepth->setText("-");
	_ui.mtOriginDepthUnit->setText("");
	_ui.mtOriginDepthError->setText("");
	_ui.mtOriginLat->setText("-");
	_ui.mtOriginLatUnit->setText("");
	_ui.mtOriginLatError->setText("");
	_ui.mtOriginLon->setText("-");
	_ui.mtOriginLonUnit->setText("");
	_ui.mtOriginLonError->setText("");
	_ui.mtOriginPhases->setText("-");
	_ui.mtOriginAgency->setText("-");
	_ui.mtOriginStatus->setText("-");

	_ui.mtMagInfo->setEnabled(false);
	_ui.mtMagType->setText("-");
	_ui.mtMag->setText("-");
	_ui.mtMagError->setText("");
	_ui.mtMagCount->setText("-");
	_ui.mtMagMethod->setText("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventEdit::sendJournal(const std::string &action,
                            const std::string &params) {

	//std::cout << "Journal: " << action << "(" << params << ")" << std::endl;
	//return true;

	if ( _updateLocalEPInstance ) {
		NotifierPtr notifier;

		if ( action == "EvType" ) {
			EventType et;
			if ( et.fromString(params) )
				_currentEvent->setType(et);
			else
				_currentEvent->setType(Core::None);

			notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
		}
		else if ( action == "EvPrefOrgID" ) {
			if ( params.empty() )
				QMessageBox::critical(this, "Error", "Releasing the preferred origin in offline mode is not supported.");
			else if ( _currentEvent->preferredOriginID() != params ) {
				_currentEvent->setPreferredOriginID(params);
				_currentEvent->setPreferredMagnitudeID("");
				notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
			}
		}
		else if ( action == "EvPrefMagType" ) {
			if ( params.empty() )
				QMessageBox::critical(this, "Error", "Releasing the preferred magnitude type in offline mode is not supported.");
			else {
				if ( _currentMagnitude ) {
					if ( _currentEvent->preferredMagnitudeID() != _currentMagnitude->publicID() ) {
						_currentEvent->setPreferredMagnitudeID(_currentMagnitude->publicID());
						notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
					}
				}
				else
					QMessageBox::critical(this, "Error",
					                      QString("Unable to find a magnitude with type %1 within origin %2.")
					                      .arg(params.c_str(),
					                           _currentEvent->preferredOriginID().c_str()));
			}
		}

		if ( notifier ) SCApp->emitNotifier(notifier.get());
	}
	else {
		JournalEntryPtr entry = new JournalEntry;
		entry->setObjectID(_currentEvent->publicID());
		entry->setAction(action);
		entry->setParameters(params);
		entry->setSender(SCApp->author());
		entry->setCreated(Core::Time::UTC());

		NotifierPtr n = new Notifier("Journaling", OP_ADD, entry.get());
		NotifierMessagePtr nm = new NotifierMessage;
		nm->attach(n.get());
		if ( SCApp->sendMessage(SCApp->messageGroups().event.c_str(), nm.get()) ) {
			addJournal(entry.get());
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::addJournal(JournalEntry *entry) {
	QString created;
	try {
		created = timeToString(entry->created(), "%F %T");
	}
	catch ( ... ) {
		created = "--/--/---- --:--:--";
	}

	if ( entry->action() == "MT.finished" )
		setFMActivity(false);
	else if ( entry->action() == "MT.started" || entry->action() == "MT.queued" )
		setFMActivity(true);

	/*
	QLabel *label = new QLabel;
	label->setText(QString("%1 [%2] %3(%4)")
		 .arg(created)
		 .arg(entry->sender().c_str())
		 .arg(entry->action().c_str())
		 .arg(entry->parameters().c_str()));
	label->setAutoFillBackground(true);

	QListWidgetItem *item = new QListWidgetItem;
	item->setText("test");
	_ui.listJournal->insertItem(0,item);

	_ui.listJournal->setItemWidget(item, label);
	*/
	QString action = entry->action().c_str();
	if ( action.endsWith("OK") )
		action = QString("<b><font color=green>%1</font></b>").arg(action);
	else if ( action.endsWith("Failed") || action.endsWith("failed") )
		action = QString("<b><font color=red>%1</font></b>").arg(action);
	else
		action = QString("<b>%1</b>").arg(action);

	_ui.listJournal->append(
		QString("<font color=gray>%1</font> %3(<i>%4</i>) <font color=gray>from %2</font>")
		 .arg(created,
		      entry->sender().c_str(),
		      action,
		      entry->parameters().c_str())
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::addMagnitude(Magnitude *mag) {
	for ( int i = 0; i < _ui.treeMagnitudes->topLevelItemCount(); ++i ) {
		if ( mag->publicID() == (const char*)_ui.treeMagnitudes->topLevelItem(i)->data(0, Qt::UserRole).toString().toLatin1() )
			return;
	}

	QTreeWidgetItem *item = new QTreeWidgetItem;
	for ( int i = 0; i < MagnitudeListColumns::Quantity; ++i )
		item->setTextAlignment(i, MagnitudeColAligns[i]);

	_ui.treeMagnitudes->addTopLevelItem(item);

	updateMagnitudeRow(_ui.treeMagnitudes->topLevelItemCount()-1, mag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::storeOrigin(Origin *org) {
	_origins.push_back(org);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::clearOrigins() {
	_origins.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::mergeOrigins(const QList<Seiscomp::DataModel::Origin*> &origins) {
	emit originMergeRequested(origins);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::storeFM(FocalMechanism *fm) {
	_fms.push_back(fm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::storeDerivedOrigin(Origin *o) {
	_derivedOrigins.push_back(o);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::clearFMs() {
	_fms.clear();
	_derivedOrigins.clear();
	resetFM();
	resetMT(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateEvent() {
	_ui.comboTypes->blockSignals(true);
	try {
		int idx = _ui.comboTypes->findText(_currentEvent->type().toString());
		_ui.comboTypes->setCurrentIndex(idx != -1?idx:0);
	}
	catch ( ... ) {
		_ui.comboTypes->setCurrentIndex(0);
	}
	_ui.comboTypes->blockSignals(false);

	_ui.comboTypeCertainties->blockSignals(true);
	try {
		int idx = _ui.comboTypeCertainties->findText(_currentEvent->typeCertainty().toString());
		_ui.comboTypeCertainties->setCurrentIndex(idx != -1?idx:0);
	}
	catch ( ... ) {
		_ui.comboTypeCertainties->setCurrentIndex(0);
	}
	_ui.comboTypeCertainties->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateOrigin() {
	timeToLabel(_ui.labelTimeValue, _currentOrigin->time().value(), PanelOTimeFormat.c_str());

	_ui.labelLatitudeValue->setText(latitudeToString(_currentOrigin->latitude(), true, false, SCScheme.precision.location));
	_ui.labelLatitudeUnit->setText(latitudeToString(_currentOrigin->latitude(), false, true));

	try {
		_ui.labelLatitudeError->setText(QString("+/- %1 km").arg(_currentOrigin->latitude().lowerUncertainty(), 0, 'f', 0));
	}
	catch ( Core::ValueException& ) {
		_ui.labelLatitudeError->setText("");
	}

	_ui.labelLongitudeValue->setText(longitudeToString(_currentOrigin->longitude(), true, false, SCScheme.precision.location));
	_ui.labelLongitudeUnit->setText(longitudeToString(_currentOrigin->longitude(), false, true));

	try {
		_ui.labelLongitudeError->setText(QString("+/- %1 km").arg(_currentOrigin->longitude().lowerUncertainty(), 0, 'f', 0));
	}
	catch ( Core::ValueException& ) {
		_ui.labelLongitudeError->setText("");
	}

	try {
		_ui.labelDepthValue->setText(depthToString(_currentOrigin->depth(), std::max(3, SCScheme.precision.depth)));
		_ui.labelDepthUnit->setText(" km");
	}
	catch ( Core::ValueException& ) {
		_ui.labelDepthValue->setText("-");
		_ui.labelDepthUnit->setText("""");
	}

	try {
		double err_z_l = _currentOrigin->depth().lowerUncertainty();
		double err_z_u = _currentOrigin->depth().upperUncertainty();

		double err_z = std::max(err_z_l, err_z_u);
		if (err_z == 0.0)
			_ui.labelDepthError->setText(QString("fixed"));
		else
			_ui.labelDepthError->setText(QString("+/- %1 km").arg(err_z, 0, 'f', 0));
	}
	catch ( Core::ValueException& ) {
		_ui.labelDepthError->setText("");
	}

	_ui.labelRegionValue->setText(Regions::getRegionName(_currentOrigin->latitude(), _currentOrigin->longitude()).c_str());

	try {
		_ui.labelPhasesValue->setText(QString("%1/%2")
			.arg(_currentOrigin->quality().usedPhaseCount())
			.arg(_currentOrigin->quality().associatedPhaseCount()));
	}
	catch ( ... ) {
		_ui.labelPhasesValue->setText("-");
	}

	try {
		_ui.labelRMSValue->setText(QString("%1").arg(_currentOrigin->quality().standardError(), 0, 'f', SCScheme.precision.rms));
	}
	catch ( Core::ValueException& ) {
		_ui.labelRMSValue->setText("-");
	}

	_ui.labelAgencyValue->setText(objectAgencyID(_currentOrigin.get()).c_str());

	try {
		_ui.labelOriginStatusValue->setText(_currentOrigin->evaluationMode().toString());
	}
	catch ( Core::ValueException& ) {
		_ui.labelOriginStatusValue->setText("");
	}

	// Update map symbol (fill the current origin)
	Map::SymbolLayer::iterator begin = _originMap->canvas().symbolCollection()->begin();
	Map::SymbolLayer::iterator end = _originMap->canvas().symbolCollection()->end();
	Map::SymbolLayer::iterator it = begin;

	for ( ; it != end; ++it ) {
		static_cast<OriginSymbol*>(*it)->setFillColor(QColor());
		static_cast<OriginSymbol*>(*it)->setFilled(false);
	}

	for ( it = begin; it != end; ++it ) {
		if ( (*it)->id() == _currentOrigin->publicID() ) {
			static_cast<OriginSymbol*>(*it)->setFilled(true);
			static_cast<OriginSymbol*>(*it)->setFillColor(Qt::black);
			_originMap->canvas().symbolCollection()->setTop(*it);
			break;
		}
	}

	_originMap->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateMagnitude() {
	_ui.labelMagnitudeTypeValue->setText(_currentMagnitude->type().c_str());
	_ui.labelMagnitudeValue->setText(QString("%1").arg(_currentMagnitude->magnitude().value(), 0, 'f', SCScheme.precision.magnitude));
	_ui.labelMagnitudeError->setText("");

	try {
		_ui.labelMagnitudeCountValue->setText(QString("%1").arg(_currentMagnitude->stationCount()));
	}
	catch ( Core::ValueException& ) {
		_ui.labelMagnitudeCountValue->setText("-");
	}

	try {
		double rms = quantityUncertainty(_currentMagnitude->magnitude());
		_ui.labelMagnitudeError->setText(QString("+/- %1").arg(rms, 0, 'f', SCScheme.precision.magnitude));
	}
	catch ( ... ) {
		_ui.labelMagnitudeError->setText("");
	}

	_ui.labelMagnitudeMethodValue->setText(_currentMagnitude->methodID().c_str());

	try {
		_ui.labelMagnitudeStatus->setText(_currentMagnitude->evaluationStatus().toString());
	}
	catch ( ... ) {
		_ui.labelMagnitudeStatus->setText(QString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateFM() {
	try { _ui.fmNP1->setText(npToString(_currentFM->nodalPlanes().nodalPlane1())); }
	catch ( Core::ValueException& ) { _ui.fmNP1->setText("-"); }

	try { _ui.fmNP2->setText(npToString(_currentFM->nodalPlanes().nodalPlane2())); }
	catch ( Core::ValueException& ) { _ui.fmNP2->setText("-"); }

	try { _ui.fmGap->setText(QString("%1").arg(_currentFM->azimuthalGap(), 0, 'f', 2)); }
	catch ( Core::ValueException& ) { _ui.fmGap->setText("-"); }

	try { _ui.fmCount->setText(QString("%1").arg(_currentFM->stationPolarityCount())); }
	catch ( Core::ValueException& ) { _ui.fmCount->setText("-"); }

	try { _ui.fmMisfit->setText(QString("%1").arg(_currentFM->misfit(), 0, 'f', 2)); }
	catch ( Core::ValueException& ) { _ui.fmMisfit->setText("-"); }

	try { _ui.fmDist->setText(QString("%1").arg(_currentFM->stationDistributionRatio(), 0, 'f', 2)); }
	catch ( Core::ValueException& ) { _ui.fmDist->setText("-"); }

	_ui.fmMethod->setText(_currentFM->methodID().c_str());

	try { _ui.fmMode->setText(_currentFM->evaluationMode().toString()); }
	catch ( Core::ValueException& ) { _ui.fmMode->setText("-"); }

	try { _ui.fmStatus->setText(_currentFM->evaluationStatus().toString()); }
	catch ( Core::ValueException& ) { _ui.fmStatus->setText("-"); }

	// Update map symbol (bring current MT to top)
	_fmMap->setCurrentFM(_currentFM->publicID());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateMT() {
	Origin *o = Origin::Find(_currentMT->derivedOriginID());
	Magnitude *m = Magnitude::Find(_currentMT->momentMagnitudeID());

	resetMT();
	if ( o ) {
		_ui.mtOriginInfo->setEnabled(true);

		// time
		timeToLabel(_ui.mtOriginTime, o->time().value(), PanelOTimeFormat.c_str());

		// region
		_ui.mtOriginRegion->setText(Regions::getRegionName(o->latitude(), o->longitude()).c_str());

		// depth
		try {
			_ui.mtOriginDepth->setText(depthToString(o->depth(), SCScheme.precision.depth));
			_ui.mtOriginDepthUnit->setText(" km");
		}
		catch ( Core::ValueException& ) {}
		try {
			double err_z_l = o->depth().lowerUncertainty();
			double err_z_u = o->depth().upperUncertainty();
			double err_z = std::max(err_z_l, err_z_u);
			if (err_z == 0.0)
				_ui.mtOriginDepthError->setText(QString("fixed"));
			else
				_ui.mtOriginDepthError->setText(QString("+/- %1 km").arg(err_z, 0, 'f', 0));
		}
		catch ( Core::ValueException& ) {}

		// latitude
		_ui.mtOriginLat->setText(latitudeToString(o->latitude(), true, false, SCScheme.precision.location));
		_ui.mtOriginLatUnit->setText(latitudeToString(o->latitude(), false, true));
		try { _ui.mtOriginLatError->setText(QString("+/- %1 km").arg(o->latitude().lowerUncertainty(), 0, 'f', 0)); }
		catch ( Core::ValueException& ) {}

		// longitude
		_ui.mtOriginLon->setText(longitudeToString(o->longitude(), true, false, SCScheme.precision.location));
		_ui.mtOriginLonUnit->setText(longitudeToString(o->longitude(), false, true));
		try { _ui.mtOriginLonError->setText(QString("+/- %1 km").arg(o->longitude().lowerUncertainty(), 0, 'f', 0)); }
		catch ( Core::ValueException& ) {}

		// phases
		try {
			_ui.mtOriginPhases->setText(QString("%1/%2")
				.arg(o->quality().usedPhaseCount())
				.arg(o->quality().associatedPhaseCount()));
		}
		catch ( ... ) {
			try {
				_ui.mtOriginPhases->setText(QString("%1")
					.arg(o->quality().usedPhaseCount()));
			}
			catch ( ... ) {}
		}

		// Agency + Status
		_ui.mtOriginAgency->setText(objectAgencyID(o).c_str());
		char stat = objectStatusToChar(o);
		_ui.mtOriginStatus->setText(QString("%1").arg(stat));
	}

	if ( m ) {
		_ui.mtMagInfo->setEnabled(true);

		_ui.mtMagType->setText(m->type().c_str());
		_ui.mtMag->setText(QString("%1").arg(m->magnitude().value(), 0, 'f', SCScheme.precision.magnitude));

		try {
			double rms = quantityUncertainty(m->magnitude());
			_ui.mtMagError->setText(QString("+/- %1").arg(rms, 0, 'f', SCScheme.precision.magnitude));
		}
		catch ( ... ) {
			_ui.mtMagError->setText("");
		}

		try { _ui.mtMagCount->setText(QString("%1").arg(m->stationCount())); }
		catch ( Core::ValueException& ) { _ui.mtMagCount->setText("-"); }

		_ui.mtMagMethod->setText(m->methodID().c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::updateJournal() {
	_ui.listJournal->clear();

	auto instance = static_cast<Journaling*>(PublicObject::Find(Journaling::ClassName()));
	if ( instance ) {
		auto count = instance->journalEntryCount();
		for ( size_t i = 0; i < count; ++i ) {
			auto entry = instance->journalEntry(i);
			if ( entry->objectID() == _currentEvent->publicID() ) {
				addJournal(entry);
			}
		}
		return;
	}

	if ( !_reader ) {
		return;
	}

	DatabaseIterator it = _reader->getJournal(_currentEvent->publicID());
	while ( it.get() ) {
		addJournal(JournalEntry::Cast(*it));
		++it;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::sortOriginItems(int column) {
	QHeaderView* header =  _originTree->header();
	if ( header == nullptr ) return;

	Qt::SortOrder order = header->sortIndicatorOrder();

	QTreeWidgetItem *prefItem = nullptr;
	if ( _preferredOriginIdx != -1 )
		prefItem = _originTree->topLevelItem(_preferredOriginIdx);

	_originTree->blockSignals(true);

	QList<QTreeWidgetItem*> selectedItems = _originTree->selectedItems();

	int count = _originTree->topLevelItemCount();

	QVector<QPair<QTreeWidgetItem*, int> > items(count);
	for ( int i = 0; i < items.count(); ++i ) {
		items[i].first = _originTree->takeTopLevelItem(0);
		items[i].second = column;
	}

	LessThan compare;

	if ( column == _originColumnMap[OL_PHASES] || column == _originColumnMap[OL_LAT] ||
	     column == _originColumnMap[OL_LON] || column == _originColumnMap[OL_DEPTH] ||
	     column == _originColumnMap[OL_RMS] || column == _originColumnMap[OL_AZGAP] )
		compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
	else
		compare = (order == Qt::AscendingOrder ? &itemTextLessThan : &itemTextGreaterThan);

	stable_sort(items.begin(), items.end(), compare);

	for ( int i = 0; i < items.count(); ++i ) {
		if ( prefItem == items[i].first )
			_preferredOriginIdx = _originTree->topLevelItemCount();
		_originTree->addTopLevelItem(items[i].first);
	}

	foreach ( QTreeWidgetItem *item, selectedItems )
		item->setSelected(true);

	_originTree->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::sortFMItems(int column) {
	QHeaderView* header =  _ui.fmTree->header();
	if ( header == nullptr ) return;

	Qt::SortOrder order = header->sortIndicatorOrder();

	QTreeWidgetItem *prefItem = nullptr;
	if ( _preferredFMIdx != -1 )
		prefItem = _ui.fmTree->topLevelItem(_preferredFMIdx);

	_ui.fmTree->blockSignals(true);

	QList<QTreeWidgetItem*> selectedItems = _ui.fmTree->selectedItems();

	int count = _ui.fmTree->topLevelItemCount();

	QVector<QPair<QTreeWidgetItem*, int> > items(count);
	for ( int i = 0; i < items.count(); ++i ) {
		items[i].first = _ui.fmTree->takeTopLevelItem(0);
		items[i].second = column;
	}

	LessThan compare;

	if ( column == _fmColumnMap[FML_AZGAP] ||  column == _fmColumnMap[FML_COUNT] ||
	     column == _fmColumnMap[FML_MISFIT] || column == _fmColumnMap[FML_STDR] ||
	     column == _fmColumnMap[FML_DEPTH] || column == _fmColumnMap[FML_MAG] )
		compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
	else
		compare = (order == Qt::AscendingOrder ? &itemTextLessThan : &itemTextGreaterThan);

	stable_sort(items.begin(), items.end(), compare);

	for ( int i = 0; i < items.count(); ++i ) {
		if ( prefItem == items[i].first )
			_preferredFMIdx = _ui.fmTree->topLevelItemCount();
		_ui.fmTree->addTopLevelItem(items[i].first);
	}

	foreach ( QTreeWidgetItem *item, selectedItems )
		item->setSelected(true);

	_ui.fmTree->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::originSelected(QTreeWidgetItem *, int) {
	if ( !_currentEvent || !_currentOrigin ) return;
	emit originSelected(_currentOrigin.get(), _currentEvent.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fmSelected(QTreeWidgetItem *, int) {
	if ( !_currentEvent || !_currentFM ) return;
	emit fmSelected(_currentFM.get(), _currentEvent.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::sortMagnitudeItems(int c) {
	QHeaderView* header =  _ui.treeMagnitudes->header();
	if ( header == nullptr ) return;

	Qt::SortOrder order = header->sortIndicatorOrder();

	QTreeWidgetItem *prefItem = nullptr;
	if ( _preferredMagnitudeIdx != -1 )
		prefItem = _ui.treeMagnitudes->topLevelItem(_preferredMagnitudeIdx);

	_originTree->blockSignals(true);
	_ui.treeMagnitudes->sortItems(c,order);
	if ( prefItem )
		_preferredMagnitudeIdx = _ui.treeMagnitudes->indexOfTopLevelItem(prefItem);
	_originTree->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::currentTypeChanged(int row) {
	std::string type;

	if ( row > 0 )
		type = _ui.comboTypes->itemText(row).toStdString();

	if ( !sendJournal("EvType", type) )
		updateEvent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::currentTypeCertaintyChanged(int row) {
	std::string typeCertainty;

	if ( row > 0 )
		typeCertainty = _ui.comboTypeCertainties->itemText(row).toStdString();

	if ( !sendJournal("EvTypeCertainty", typeCertainty) )
		updateEvent();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::currentOriginChanged(QTreeWidgetItem* item, QTreeWidgetItem*) {
	if ( item == nullptr ) {
		resetOrigin();
		return;
	}

	// Update origin information
	_currentOrigin = Origin::Find(item->data(0, Qt::UserRole).toString().toStdString());

	if ( !_currentOrigin ) {
		resetOrigin();
		return;
	}

	updateOrigin();
	resetMagnitude();

	_ui.treeMagnitudes->blockSignals(true);
	_ui.treeMagnitudes->clear();
	_ui.treeMagnitudes->setColumnCount(MagnitudeListColumns::Quantity);

	QStringList headerLabels;
	for ( int i = 0; i < MagnitudeListColumns::Quantity; ++i ) {
		if ( i == MLC_TIMESTAMP ) {
			if ( SCScheme.dateTime.useLocalTime )
				headerLabels << QString(EMagnitudeListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
			else
				headerLabels << QString(EMagnitudeListColumnsNames::name(i)).arg("UTC");
		}
		else
			headerLabels << EMagnitudeListColumnsNames::name(i);
	}

	_ui.treeMagnitudes->setHeaderLabels(headerLabels);

	_preferredMagnitudeIdx = -1;

	for ( size_t i = 0; i < _currentOrigin->magnitudeCount(); ++i )
		addMagnitude(_currentOrigin->magnitude(i));

	updatePreferredMagnitudeIndex();

	_ui.treeMagnitudes->blockSignals(false);

	sortMagnitudeItems(_ui.treeMagnitudes->sortColumn());

	_ui.treeMagnitudes->setCurrentItem(_preferredMagnitudeIdx == -1?nullptr:_ui.treeMagnitudes->topLevelItem(_preferredMagnitudeIdx));

	_ui.buttonFixOrigin->setEnabled(true);
	_ui.comboFixOrigin->setEnabled(true);

	if ( _ui.comboFixOrigin->count() == _fixOriginDefaultActionCount ) {
		_ui.comboFixOrigin->addItem("selected origin");
		_ui.comboFixOrigin->setCurrentIndex(_ui.comboFixOrigin->count()-1);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::currentFMChanged(QTreeWidgetItem* item, QTreeWidgetItem*) {
	if ( item == nullptr ) {
		resetFM();
		return;
	}

	// Update focal mechanism information
	_currentFM = FocalMechanism::Find(item->data(0, Qt::UserRole).toString().toStdString());

	if ( !_currentFM ) {
		resetFM();
		resetMT(true);
		return;
	}

	updateFM();

	_ui.fmFixButton->setEnabled(true);
	_ui.fmFixCombo->setEnabled(true);

	if ( _ui.fmFixCombo->count() == _fixFMDefaultActionCount )
		_ui.fmFixCombo->addItem("selected focal mechanism");

	if ( _currentFM->momentTensorCount() > 0 ) {
		_currentMT = _currentFM->momentTensor(0);
		updateMT();
	}
	else
		resetMT(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::currentMagnitudeChanged(QTreeWidgetItem *item, QTreeWidgetItem*) {
	if ( item == nullptr ) {
		resetMagnitude();
		return;
	}

	// Update magnitude information
	_currentMagnitude = Magnitude::Find(item->data(0, Qt::UserRole).toString().toStdString());

	if ( !_currentMagnitude ) {
		resetMagnitude();
		return;
	}

	updateMagnitude();

	_ui.buttonFixMagnitudeType->setEnabled(true);
	_ui.buttonReleaseMagnitudeType->setEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::originTreeCustomContextMenu(const QPoint &pos) {
	if ( !_originTree->selectionModel() ) return;
	if ( !_originTree->selectionModel()->hasSelection() ) return;

	QModelIndexList selection = _originTree->selectionModel()->selectedRows();
	QMenu menu;

	QTreeWidgetItem *item = _originTree->itemAt(pos);
	int column = _originTree->header()->visualIndexAt(pos.x());

	QAction *actionMerge = nullptr;
	if ( selection.count() >= 2 ) {
		actionMerge = menu.addAction("Merge selected origins");
		menu.addSeparator();
	}
	QAction *actionCopyCell = item ? menu.addAction("Copy cell to clipboard") : nullptr;
	QAction *actionCopyRows = menu.addAction("Copy selected rows to clipboard");

	QAction *result = menu.exec(_originTree->mapToGlobal(pos));
	if ( actionMerge && result == actionMerge ) {
		QList<DataModel::Origin*> origins;
		DataModel::Origin *org;

		foreach ( const QModelIndex &idx, selection ) {
			if ( idx.column() != 0 ) continue;
			QString originID = idx.data(Qt::UserRole).toString();
			org = DataModel::Origin::Find(originID.toStdString());
			if ( !org ) {
				cerr << "Origin with id '" << qPrintable(originID) << "' not found" << endl;
			}
			else if ( !origins.contains(org) ) {
				origins.append(org);
			}
		}

		if ( !origins.isEmpty() ) handleOrigins(origins);
	}
	else if ( result == actionCopyRows )
		SCApp->copyToClipboard(_originTree);
	else if ( actionCopyCell && result == actionCopyCell ) {
		QClipboard *cb = QApplication::clipboard();
		if ( cb )
			cb->setText(item->text(column));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::originTreeHeaderCustomContextMenu(const QPoint &pos) {
	int count = _originTree->header()->count();
	QAbstractItemModel *model = _originTree->header()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!_originTree->header()->isSectionHidden(i));
	}

	QAction *result = menu.exec(_originTree->header()->mapToGlobal(pos));
	if ( result == NULL ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	for ( int i = 0; i < count; ++i )
		OriginColVisible[i] = actions[i]->isChecked();

	_originTree->header()->setSectionHidden(section, !OriginColVisible[section]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fmTreeCustomContextMenu(const QPoint &pos) {
	if ( !_ui.fmTree->selectionModel() ) return;
	if ( !_ui.fmTree->selectionModel()->hasSelection() ) return;

	QMenu menu;

	QTreeWidgetItem *item = _ui.fmTree->itemAt(pos);
	int column = _ui.fmTree->header()->visualIndexAt(pos.x());

	QAction *actionCopyCell = item ? menu.addAction("Copy cell to clipboard") : nullptr;
	QAction *actionCopyRows = menu.addAction("Copy selected rows to clipboard");

	QAction *result = menu.exec(_ui.fmTree->mapToGlobal(pos));
	if ( result == actionCopyRows )
		SCApp->copyToClipboard(_ui.fmTree);
	else if ( actionCopyCell && result == actionCopyCell ) {
		QClipboard *cb = QApplication::clipboard();
		if ( cb )
			cb->setText(item->text(column));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fmTreeHeaderCustomContextMenu(const QPoint &pos) {
	int count = _ui.fmTree->header()->count();
	QAbstractItemModel *model = _ui.fmTree->header()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!_ui.fmTree->header()->isSectionHidden(i));
	}

	QAction *result = menu.exec(_ui.fmTree->header()->mapToGlobal(pos));
	if ( result == NULL ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	for ( int i = 0; i < count; ++i )
		FMColVisible[i] = actions[i]->isChecked();

	_ui.fmTree->header()->setSectionHidden(section, !FMColVisible[section]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::magnitudeTreeCustomContextMenu(const QPoint &pos) {
	if ( !_ui.treeMagnitudes->selectionModel() ) return;
	if ( !_ui.treeMagnitudes->selectionModel()->hasSelection() ) return;

	QTreeWidgetItem *item = _ui.treeMagnitudes->itemAt(pos);
	int column = _ui.treeMagnitudes->header()->visualIndexAt(pos.x());

	QMenu menu;
	QAction *actionCopyCell = item ? menu.addAction("Copy cell to clipboard") : nullptr;
	QAction *actionCopyRows = menu.addAction("Copy selected rows to clipboard");

	QAction *result = menu.exec(_ui.treeMagnitudes->mapToGlobal(pos));
	if ( result == actionCopyRows )
		SCApp->copyToClipboard(_ui.treeMagnitudes);
	else if ( actionCopyCell && result == actionCopyCell ) {
		QClipboard *cb = QApplication::clipboard();
		if ( cb )
			cb->setText(item->text(column));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fixOrigin() {
	if ( _ui.comboFixOrigin->currentText() == "nothing" ) {
		sendJournal("EvPrefOrgAutomatic", "");
	}
	else if ( _ui.comboFixOrigin->currentText() == "selected origin" ) {
		if ( !_currentOrigin ) {
			QMessageBox::critical(this, "Error", "No origin selected.");
			return;
		}

		sendJournal("EvPrefOrgID", _currentOrigin->publicID());
	}
	else {
		int sep = _ui.comboFixOrigin->currentText().lastIndexOf(' ');
		if ( sep != -1 )
			sendJournal("EvPrefOrgEvalMode", _ui.comboFixOrigin->currentText().mid(0, sep).toStdString());
		else
			QMessageBox::critical(this, "Error", "Internal error.");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fixFM() {
	if ( !_currentFM ) {
		QMessageBox::critical(this, "Error", "No focal mechanism selected.");
		return;
	}

	sendJournal("EvPrefFocMecID", _currentFM->publicID());

//	if ( _ui.fmFixCombo->currentText() == "nothing" ) {
//		sendJournal("EvPrefFocMecAutomatic", "");
//	}
//	else if ( _ui.fmFixCombo->currentText() == "selected focal mechanism" ) {
//		if ( !_currentFM ) {
//			QMessageBox::critical(this, "Error", "No focal mechanism selected.");
//			return;
//		}

//		sendJournal("EvPrefFocMecID", _currentFM->publicID());
//	}
//	else {
//		int sep = _ui.fmFixCombo->currentText().lastIndexOf(' ');
//		if ( sep != -1 )
//			sendJournal("EvPrefFocMecEvalMode", _ui.fmFixCombo->currentText().mid(0, sep).toStdString());
//		else
//			QMessageBox::critical(this, "Error", "Internal error.");
//	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::releaseOrigin() {
	sendJournal("EvPrefOrgID", "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::releaseFM() {
	sendJournal("EvPrefFocMecID", "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fixMagnitudeType() {
	QTreeWidgetItem *item = _ui.treeMagnitudes->currentItem();
	if ( !item ) return;

	sendJournal("EvPrefMagType", _ui.labelMagnitudeTypeValue->text().toStdString());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::releaseMagnitudeType() {
	sendJournal("EvPrefMagType", "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fixMw() {
	sendJournal("EvPrefMw", "true");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::fixFmMw() {
	if ( !_currentFM ) {
		QMessageBox::critical(this, "Error", "No focal mechanism selected.");
		return;
	}

	sendJournal("EvPrefFocMecID", _currentFM->publicID());
	sendJournal("EvPrefMw", "true");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::releaseMw() {
	sendJournal("EvPrefMw", "false");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::triggerMw() {
	if ( !_currentEvent ) return;
	sendJournal("MT.start", "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::evalResultAvailable(const QString &oid,
                                    const QString &className,
                                    const QString &script,
                                    const QString &result) {
	for ( int i = 0; i < _originTree->topLevelItemCount(); ++i ) {
		QTreeWidgetItem *item = _originTree->topLevelItem(i);
		if ( item->data(0, Qt::UserRole).toString() == oid ) {
			QHash<QString,int>::iterator it = _scriptColumnMap.find(script);
			if ( it == _scriptColumnMap.end() ) return;

			item->setText(it.value(), result);
			item->setBackground(it.value(), Qt::NoBrush);
			item->setForeground(it.value(), Qt::NoBrush);
			item->setToolTip(it.value(), "");
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventEdit::evalResultError(const QString &oid,
                                const QString &className,
                                const QString &script,
                                int error) {
	for ( int i = 0; i < _originTree->topLevelItemCount(); ++i ) {
		QTreeWidgetItem *item = _originTree->topLevelItem(i);
		if ( item->data(0, Qt::UserRole).toString() == oid ) {
			QHash<QString,int>::iterator it = _scriptColumnMap.find(script);
			if ( it == _scriptColumnMap.end() ) return;

			item->setText(it.value(), "!");
			item->setBackground(it.value(), Qt::NoBrush);
			item->setForeground(it.value(), Qt::darkRed);
			item->setToolTip(it.value(),
			                 QString("%1\n\n%2")
			                 .arg(script)
			                 .arg(PublicObjectEvaluator::Instance().errorMsg(error)));
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
