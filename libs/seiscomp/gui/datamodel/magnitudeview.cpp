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



#define SEISCOMP_COMPONENT Gui::MagnitudeView

#include "magnitudeview.h"
#include "magnitudeview_p.h"

#include <seiscomp/gui/core/diagramwidget.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/processing/amplitudeprocessor.h>
#include <seiscomp/processing/magnitudeprocessor.h>
#include <seiscomp/seismology/regions.h>
#include <seiscomp/utils/misc.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/datamodel/amplitudeview.h>
#include <seiscomp/gui/datamodel/utils.h>

#include <QMessageBox>

#include <functional>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;


#define INVALID_MAG 0.0
#define SC_D (*_d_ptr)


namespace {


struct TabData {
	TabData()
	: valid(true), selected(false) {}

	TabData(const string &pid)
	: publicID(pid), valid(true), selected(false) {}

	string publicID;
	bool   valid;
	bool   selected;
};


struct MagnitudeCommentProfile {
	string         id;
	string         value; // Output value
	string         label;
	vector<string> options;
	bool           allowFreeText{false};
};


}


namespace Seiscomp {
namespace Gui {


namespace {

MAKEENUM(
	StaMagsListColumns,
	EVALUES(
		USED,
		NETWORK,
		STATION,
		CHANNEL,
		MAGNITUDE,
		RESIDUAL,
		DISTANCE,
		AZIMUTH,
		AMP,
		SNR,
		PERIOD,
		CREATED,
		UPDATED
	),
	ENAMES(
		"Sel",
		"Net",
		"Sta",
		"Loc/Cha",
		"Mag",
		"Res",
		"Dist",
		"Az",
		"Amp",
		"SNR",
		"Per (s)",
		"Created",
		"Updated"
	)
);


bool colVisibility[StaMagsListColumns::Quantity] = {
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	false,
	false
};


QVariant colAligns[StaMagsListColumns::Quantity] = {
	{},
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight| Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter)
};


class StaMagsSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		StaMagsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
			if ( (left.column() == MAGNITUDE && right.column() == MAGNITUDE) ||
			     (left.column() == RESIDUAL && right.column() == RESIDUAL) ||
			     (left.column() == DISTANCE && right.column() == DISTANCE) ||
			     (left.column() == AZIMUTH && right.column() == AZIMUTH) ||
			     (left.column() == SNR && right.column() == SNR) ||
			     (left.column() == PERIOD && right.column() == PERIOD) )
				return sourceModel()->data(left, Qt::UserRole).toDouble() < sourceModel()->data(right, Qt::UserRole).toDouble();
			else
				return QSortFilterProxyModel::lessThan(left, right);
		}
};


Util::KeyValuesPtr getParams(const string &net, const string &sta) {
	ConfigModule *module = SCApp->configModule();
	if ( module == nullptr ) return nullptr;

	for ( size_t ci = 0; ci < module->configStationCount(); ++ci ) {
		ConfigStation* cs = module->configStation(ci);
		if ( cs->networkCode() != net || cs->stationCode() != sta ) continue;
		Setup *setup = findSetup(cs, SCApp->name());
		if ( setup == nullptr ) continue;
		if ( !setup->enabled() ) continue;

		DataModel::ParameterSet *ps = DataModel::ParameterSet::Find(setup->parameterSetID());
		if ( ps == nullptr ) {
			SEISCOMP_WARNING("Cannot find parameter set %s for station %s.%s",
			                 setup->parameterSetID().data(),
			                 net.data(), sta.data());
			continue;
		}

		Util::KeyValuesPtr keys = new Util::KeyValues;
		keys->init(ps);
		return keys;
	}

	return nullptr;
}


int findType(QTabBar *tab, const char *text) {
	for ( int i = 0; i < tab->count(); ++i ) {
		Magnitude *mag = Magnitude::Find(tab->tabData(i).value<TabData>().publicID);
		if ( mag && mag->type() == text )
			return i;
	}

	return -1;
}


int findData(QTabBar *tab, const string &publicID) {
	for ( int i = 0; i < tab->count(); ++i ) {
		if ( tab->tabData(i).value<TabData>().publicID == publicID )
			return i;
	}

	return -1;
}


string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}


int usedStationCount(const Magnitude *mag) {
	try {
		return mag->stationCount();
	}
	catch ( ... ) {
		int cnt = 0;
		for ( size_t i = 0; i < mag->stationMagnitudeContributionCount(); ++i ) {
			try {
				if ( mag->stationMagnitudeContribution(i)->weight() > 0.0 )
					++cnt;
			}
			catch ( ... ) {
				++cnt;
			}
		}
		return cnt;
	}
}


int totalStationCount(const Magnitude *mag) {
	return mag->stationMagnitudeContributionCount();
}


void updateTab(QTabBar *tabBar, const Magnitude *mag) {
	int idx = findData(tabBar, mag->publicID());
	if ( idx != -1 ) {
		QString text = QString("%1 %2 (%3/%4)")
		        .arg(mag->type().c_str())
		        .arg(mag->magnitude().value(), 0,
		            'f', SCScheme.precision.magnitude)
		        .arg(usedStationCount(mag))
		        .arg(totalStationCount(mag));
		tabBar->setTabText(idx, text);
	}
}


template <typename T>
struct like {
	bool operator()(const T &lhs, const T &rhs) const {
		return false;
	}
};


template <>
struct like<QString> {
	bool operator()(const QString &lhs, const QString &rhs) const {
		return Core::wildcmp(rhs.toLatin1(), lhs.toLatin1());
	}
};


template <typename T>
class ModelFieldValueFilter : public ModelAbstractRowFilter {
	public:
		explicit ModelFieldValueFilter(int column, CompareOperation op, T value) : _column(column), _op(op), _value(value) {}

		virtual int column() const { return _column; }
		virtual CompareOperation operation() const { return _op; }
		virtual QString value() const { return QVariant(_value).toString(); }

		virtual QString toString() {
			StaMagsListColumns c = (EStaMagsListColumns)_column;
			return QString("%1,%2,%3").arg(c.toString(), _op.toString()).arg(_value);
		}

		virtual bool fromString(const QString &) { return false; }

		virtual bool passes(QAbstractItemModel *model, int row) {
			return check(model, model->index(row, _column), _value);
		}

	protected:
		bool check(QAbstractItemModel *model, const QModelIndex &idx, const T &v) {
			// Actually this->_op is not necessary but due to a bug of clang
			// this line is required as a hotfix, see
			// https://stackoverflow.com/questions/55359614/clang-complains-about-constexpr-function-in-case-for-switch-statement
			switch ( this->_op ) {
				case Less:
					return std::less<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case LessEqual:
					return std::less_equal<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case Equal:
					return std::equal_to<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case NotEqual:
					return std::not_equal_to<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case Greater:
					return std::greater<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case GreaterEqual:
					return std::greater_equal<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case Like:
					return like<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				default:
					break;
			}

			return false;
		}

	protected:
		int              _column;
		CompareOperation _op;
		T                _value;
};


class ModelDistanceFilter : public ModelFieldValueFilter<double> {
	public:
		explicit ModelDistanceFilter(int column, CompareOperation op, double value)
		: ModelFieldValueFilter<double>(column, op, value) {
			if ( SCScheme.unit.distanceInKM )
				_baseValue = Math::Geo::km2deg(value);
			else
				_baseValue = _value;
		}

		virtual bool passes(QAbstractItemModel *model, int row) {
			return check(model, model->index(row, _column), _baseValue);
		}

	private:
		double _baseValue;
};


template <class FUNC>
class ModelRowFilterMultiOperation : public ModelAbstractRowFilter {
	public:
		// Takes ownership
		explicit ModelRowFilterMultiOperation() {}
		~ModelRowFilterMultiOperation() {
			foreach ( ModelAbstractRowFilter *f, _filters ) delete f;
		}

	public:
		int column() const { return -1; }
		virtual CompareOperation operation() const { return Undefined; }
		virtual QString value() const { return QString(); }

		bool passes(QAbstractItemModel *model, int row) {
			if ( _filters.empty() ) return true;

			bool ret = _filters[0]->passes(model,row);

			for ( int i = 1; i < _filters.size(); ++i )
				ret = _func(ret, _filters[i]->passes(model,row));

			return ret;
		}

		virtual QString toString() {
			QString str;

			for ( int i = 0; i < _filters.size(); ++i ) {
				if ( i ) str += QString(" %1 ").arg(FUNC::str());
				str += _filters[i]->toString();
			}

			return str;
		}

		virtual bool fromString(const QString &s) {
			QStringList items = s.split(FUNC::str());
			foreach ( const QString &item, items ) {
				QStringList toks = item.trimmed().split(",");
				if ( toks.size() != 3 ) return false;
				StaMagsListColumns c;
				CompareOperation op;
				QString value;
				if ( !c.fromString(toks[0].trimmed().toStdString()) )
					return false;
				if ( !op.fromString(toks[1].trimmed().toStdString()) )
					return false;
				value = toks[2].trimmed();

				ModelAbstractRowFilter *stage = nullptr;

				switch ( c ) {
					case CHANNEL:
					{
						stage = new ModelFieldValueFilter<QString>(c, op, value);
						break;
					}
					case MAGNITUDE:
					case RESIDUAL:
					case DISTANCE:
					{
						bool ok;
						double v = value.toDouble(&ok);
						if ( ok ) {
							if ( c == DISTANCE )
								stage = new ModelDistanceFilter(c, op, v);
							else
								stage = new ModelFieldValueFilter<double>(c, op, v);
						}
						break;
					}
					default:
						break;
				}

				if ( stage != nullptr )
					add(stage);
				else
					return false;
			}

			return true;
		}

		void add(ModelAbstractRowFilter *f) { _filters.append(f); }
		int count() const { return _filters.count(); }
		ModelAbstractRowFilter *filter(int i) const { return _filters[i]; }

	private:
		FUNC                              _func;
		QVector<ModelAbstractRowFilter *> _filters;
};


struct opAnd {
	static const char *str() { static const char *s = "&&"; return s; }
	bool operator()(bool lhs, bool rhs) const {
		return lhs && rhs;
	}
};

struct opOr {
	static const char *str() { static const char *s = "||"; return s; }
	bool operator()(bool lhs, bool rhs) const {
		return lhs || rhs;
	}
};

struct opXor {
	static const char *str() { static const char *s = "^^"; return s; }
	bool operator()(bool lhs, bool rhs) const {
		return lhs != rhs;
	}
};


typedef ModelRowFilterMultiOperation<opOr> ModelRowFilter;

QColor failedQCColor(255,160,0,64);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// Implementation of StationMagnitudeModel
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class StationMagnitudeModel : public QAbstractTableModel {
	public:
		StationMagnitudeModel(DataModel::PublicObjectCache *cache = nullptr,
		                      QObject *parent = 0);

		void setOrigin(DataModel::Origin *origin,
		               DataModel::Magnitude *netMag);

		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;

		bool insertRows(int row, int count, const QModelIndex &parent) override;

		QVariant data(const QModelIndex &index, int role) const override;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override;

		Qt::ItemFlags flags(const QModelIndex &index) const override;
		bool setData(const QModelIndex &index, const QVariant &value,
		             int role = Qt::EditRole) override;

		bool useMagnitude(int row) const;


	private:
		DataModel::PublicObjectCache *_cache;
		DataModel::Origin            *_origin;
		DataModel::Magnitude         *_magnitude;
		QVector<Qt::CheckState>       _used;
		QVector<double>               _distance;
		QVector<double>               _azimuth;
		QStringList                   _header;
		int                           _rowCount;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationMagnitudeModel::StationMagnitudeModel(PublicObjectCache *cache,
                                             QObject *parent)
: QAbstractTableModel(parent), _cache(cache) {
	for ( int i = 0; i < StaMagsListColumns::Quantity; ++i ) {
		if ( i == DISTANCE ) {
			if ( SCScheme.unit.distanceInKM ) {
				_header << QString("%1 (km)").arg(EStaMagsListColumnsNames::name(i));
			}
			else {
				_header << QString("%1 (°)").arg(EStaMagsListColumnsNames::name(i));
			}
		}
		else if ( i == AZIMUTH ) {
			_header << QString("%1 (°)").arg(EStaMagsListColumnsNames::name(i));
		}
		else if ( i == CREATED ) {
			_header << QString("%1 (UTC)").arg(EStaMagsListColumnsNames::name(i));
		}
		else if ( i == UPDATED ) {
			_header << QString("%1 (UTC)").arg(EStaMagsListColumnsNames::name(i));
		}
		else {
			_header << EStaMagsListColumnsNames::name(i);
		}
	}

	setOrigin(nullptr, nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationMagnitudeModel::setOrigin(DataModel::Origin* origin,
                                      DataModel::Magnitude* magnitude) {
	_origin = origin;
	_magnitude = magnitude;
	_rowCount = 0;
	if ( _origin ) {
		if ( _magnitude ) {
			_rowCount = _magnitude->stationMagnitudeContributionCount();
			_distance.fill(-1.0, _rowCount);
			_azimuth.fill(999.0, _rowCount);
			_used.fill(Qt::Checked, _rowCount);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationMagnitudeModel::rowCount(const QModelIndex &) const {
	/*
	if ( _magnitude ) {
		cout << "------ REQUESTED ROWCOUNT: " << _magnitude->stationMagnitudeContributionCount() << endl;
		return _magnitude->stationMagnitudeContributionCount();
	}
	return 0;
	*/
	return _rowCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationMagnitudeModel::columnCount(const QModelIndex &) const {
	return _header.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationMagnitudeModel::insertRows(int row, int count, const QModelIndex &parent) {
	if ( count <= 0 ) {
		return false;
	}

	if ( row < _rowCount ) {
		// Only append is supported
		return false;
	}

	beginInsertRows(parent, row, row + count - 1);

	_rowCount += count;

	_distance.insert(_distance.end(), _rowCount, -1.0);
	_azimuth.insert(_azimuth.end(), _rowCount, 999.0);
	_used.insert(_used.end(), _rowCount, Qt::Checked);

	endInsertRows();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant StationMagnitudeModel::data(const QModelIndex &index, int role) const {
	if ( !index.isValid() ) {
		return {};
	}

	if ( !_magnitude ) {
		return {};
	}

	if ( index.row() >= static_cast<int>(_magnitude->stationMagnitudeContributionCount()) ) {
		return {};
	}


	if ( role == Qt::CheckStateRole && index.column() == 0 ) {
		if ( index.row() < _used.size() ) {
			return _used[index.row()];
		}
		else {
			return {};
		}
	}

	StationMagnitude *sm = StationMagnitude::Find(_magnitude->stationMagnitudeContribution(index.row())->stationMagnitudeID());

	if ( role == Qt::DisplayRole ) {
		if ( !sm ) {
			return {};
		}

		char buf[10];
		double smval = sm->magnitude().value();

		switch ( index.column() ) {
			case USED:
				try {
					snprintf(buf, 10, "   %.3f", _magnitude->stationMagnitudeContribution(index.row())->weight());
					return QString::fromUtf8(buf);
				}
				catch ( Core::ValueException& ) {}
				break;

			case NETWORK:
				try{
					return sm->waveformID().networkCode().c_str();
				}
				catch ( ... ) {
					return {};
				}

			case STATION:
				try {
					return sm->waveformID().stationCode().c_str();
				}
				catch ( ... ) {
					return {};
				}

			case CHANNEL:
				try {
					if ( sm->waveformID().locationCode().empty() ) {
						return sm->waveformID().channelCode().c_str();
					}
					else {
						return (sm->waveformID().locationCode() + '.' + sm->waveformID().channelCode()).c_str();
					}
				}
				catch ( ValueException& ) {}
				break;

			case MAGNITUDE:
				snprintf(buf, 10, "%.*f", SCScheme.precision.magnitude, smval);
				return QString::fromUtf8(buf);

			case RESIDUAL:
				if ( _magnitude ) {
					try {
						snprintf(buf, 10, "%.2f", _magnitude->stationMagnitudeContribution(index.row())->residual());
						return QString::fromUtf8(buf);
					}
					catch ( ... ) {}
				}
				break;

			case DISTANCE: // dist
				if ( index.row() < _distance.size() ) {
					double distance = _distance[index.row()];
					if ( distance >= 0 ) {
						if ( SCScheme.unit.distanceInKM ) {
							snprintf(buf, 10, "%.*f", SCScheme.precision.distance, Math::Geo::deg2km(distance));
						}
						else {
							snprintf(buf, 10, distance<10 ? "%.2f" : "%.1f", distance);
						}
						return QString::fromUtf8(buf);
					}
				}
				break;

			case AZIMUTH: // azimuth
				if ( index.row() < _azimuth.size() ) {
					double az = _azimuth[index.row()];
					if ( az < 999.0 ) {
						return static_cast<int>(az);
					}
				}
				break;

			case AMP:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return QString("%1").arg(amp->amplitude().value());
					}
					catch ( ... ) {}
				}
				break;
			}

			case SNR:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return QString("%1").arg(amp->snr(), 0, 'f', 1);
					}
					catch ( ... ) {}
				}
				break;
			}

			case PERIOD:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return QString("%1").arg(amp->period().value(), 0, 'f', 2);
					}
					catch ( ... ) {}
				}
				break;
			}

			case CREATED:
				try {
					return timeToString(sm->creationInfo().creationTime(), "%T.%1f");
				}
				catch ( ValueException& ) {}
				break;

			case UPDATED:
				try {
					return timeToString(sm->creationInfo().modificationTime(), "%T.%1f");
				}
				catch ( ValueException& ) {}
				break;

			default:
				break;
		}
	}
	else if ( role == Qt::BackgroundRole ) {
		if ( sm ) {
			try {
				if ( !sm->passedQC() ) {
					return failedQCColor;
				}
			}
			catch ( ... ) {}
		}
	}
	else if ( role == Qt::UserRole ) {
		switch ( index.column() ) {
			case NETWORK:
				if ( sm ) {
					try {
						return sm->waveformID().networkCode().c_str();
					}
					catch ( ... ) {}
				}
				break;

			case STATION:
				if ( sm ) {
					try {
						return sm->waveformID().stationCode().c_str();
					}
					catch ( ... ) {}
				}
				break;

			case CHANNEL:
				if ( sm ) {
					try {
						return sm->waveformID().channelCode().c_str();
					}
					catch ( ValueException& ) {}
				}
				break;

			// Magnitude
			case MAGNITUDE:
				if ( sm ) {
					try { return sm->magnitude().value(); }
					catch ( ValueException& ) {}
				}
				break;
			// Residual
			case RESIDUAL:
				try {
					return std::abs(_magnitude->stationMagnitudeContribution(index.row())->residual());
				}
				catch ( ValueException& ) {}
				break;
			// Distance
			case DISTANCE:
				if ( index.row() < _distance.size() ) {
					double distance = _distance[index.row()];
					if ( distance >= 0 ) {
						return distance;
					}
				}
				break;
			// Azimuth
			case AZIMUTH:
				if ( index.row() < _azimuth.size() ) {
					double az = _azimuth[index.row()];
					if ( az < 999.0 ) {
						return az;
					}
				}
				break;
			case SNR:
			{
				if ( sm ) {
					AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
					if ( amp ) {
						try {
							return amp->snr();
						}
						catch ( ... ) {}
					}
				}
				return -1;
			}
			case PERIOD:
			{
				if ( sm ) {
					AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
					if ( amp ) {
						try {
							return amp->period().value();
						}
						catch ( ... ) {}
					}
				}
				return -1;
			}
			default:
				break;
		}
	}
	/*
	else if ( role == Qt::ForegroundRole ) {
		switch ( index.column() ) {
			// Residual
			case RESIDUAL:
				try {
					double smval = sm->magnitude().value(),
					nmval = _magnitude ? _magnitude->magnitude().value() : 0;
					double res = smval = nmval;
					return res < 0?Qt::darkRed:Qt::darkGreen;
				}
				catch ( ValueException& ) {}
				break;
			default:
				break;
		}
	}
	*/
	else if ( role == Qt::TextAlignmentRole ) {
		return colAligns[index.column()];
	}
	else if ( role == Qt::ToolTipRole ) {
		switch ( index.column() ) {
			case CREATED:
				if ( sm ) {
					try {
						return sm->creationInfo().creationTime().toString("%F %T.%f").c_str();
					}
					catch ( ValueException& ) {}
				}
				break;

			case UPDATED:
				if ( sm ) {
					try {
						return sm->creationInfo().modificationTime().toString("%F %T.%f").c_str();
					}
					catch ( ValueException& ) {}
				}
				break;
		}
	}

	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant StationMagnitudeModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
	if ( section < 0 ) return {};

	if ( orientation == Qt::Horizontal ) {
		switch ( role ) {
			case Qt::DisplayRole:
				if ( section >= _header.count() )
					return QString("%1").arg(section);
				else
					return _header[section];
				break;
			case Qt::TextAlignmentRole:
				return colAligns[section];
		}
	}
	else
		return section;

	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::ItemFlags StationMagnitudeModel::flags(const QModelIndex &index) const {
	if ( !index.isValid() ) {
		return Qt::ItemIsEnabled;
	}

	if ( index.column() == 0 ) {
		return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
	}

	return QAbstractTableModel::flags(index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationMagnitudeModel::setData(const QModelIndex &index, const QVariant &value,
                                    int role) {
	// set checkBox
	if ( index.isValid() && role == Qt::CheckStateRole ) {
		if ( _used.count() <= index.row() )
			_used.resize(index.row()+1);

		_used[index.row()] = (Qt::CheckState)value.toInt();
		emit dataChanged(index, index);
		return true;
	}

	// set data fields
	if ( index.isValid() && role == Qt::DisplayRole ) {
		// distance
		if ( index.column() == DISTANCE ) {
			if ( _distance.size() <= index.row() ) {
				_distance.resize(index.row() + 1);
			}

			//_distance[index.row()] = QString("%1").arg(value.toDouble(), 0, 'f', 1).toDouble();
			_distance[index.row()] = value.toDouble();
			emit dataChanged(index, index);
			return true;
		}
		else if ( index.column() == AZIMUTH ) {
			if ( _azimuth.size() <= index.row() ) {
				_azimuth.resize(index.row() + 1);
			}

			//_distance[index.row()] = QString("%1").arg(value.toDouble(), 0, 'f', 1).toDouble();
			_azimuth[index.row()] = value.toDouble();
			emit dataChanged(index, index);
			return true;
		}
	}

	return QAbstractTableModel::setData(index, value, role);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationMagnitudeModel::useMagnitude(int row) const {
	return _used[row] == Qt::Checked;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeRowFilter::MagnitudeRowFilter(ModelAbstractRowFilter **filter_ptr, QWidget * parent,
                                       Qt::WindowFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	_filter = filter_ptr;

	if ( SCScheme.unit.distanceInKM )
		_ui.labelInfo->setText(tr("NOTE: Distance is specified in km."));
	else
		_ui.labelInfo->setText(tr("NOTE: Distance is specified in degree."));

	ModelRowFilter *filter = nullptr;

	if ( _filter != nullptr )
		filter = reinterpret_cast<ModelRowFilter*>(*_filter);

	if ( filter != nullptr ) {
		for ( int i = 0; i < filter->count(); ++i ) {
			ModelAbstractRowFilter *stage = filter->filter(i);
			Row &row = addRow();

			switch ( stage->column() ) {
				case CHANNEL:
					row.column->setCurrentIndex(0);
					break;
				case MAGNITUDE:
					row.column->setCurrentIndex(1);
					break;
				case RESIDUAL:
					row.column->setCurrentIndex(2);
					break;
				case DISTANCE:
					row.column->setCurrentIndex(3);
					break;
				default:
					break;
			}

			if ( stage->operation() != ModelAbstractRowFilter::Undefined )
				row.operation->setCurrentIndex((int)stage->operation()-1);
			row.value->setText(stage->value());
		}
	}
	else {
		// At least one row is mandatory
		addRow();
	}

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	_ui.frameFilters->setLayout(layout);

	for ( int i = 0; i < _rows.count(); ++i ) {
		layout->addLayout(_rows[i].layout);
	}

	connect(_ui.btnAdd, SIGNAL(clicked()), this, SLOT(addFilter()));
	connect(_ui.btnRemove, SIGNAL(clicked()), this, SLOT(removeFilter()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeRowFilter::Row &MagnitudeRowFilter::addRow() {
	Row row;
	row.layout = new QHBoxLayout;
	row.column = new QComboBox(_ui.frameFilters);
	row.operation = new QComboBox(_ui.frameFilters);
	row.value = new QLineEdit(_ui.frameFilters);

	row.layout->addWidget(row.column);
	row.layout->addWidget(row.operation);
	row.layout->addWidget(row.value);

	row.column->addItem(tr("Channel"));
	row.column->addItem(tr("Magnitude"));
	row.column->addItem(tr("Residual"));
	row.column->addItem(tr("Distance"));

	for ( int i = 1; i < ModelAbstractRowFilter::CompareOperation::Quantity; ++i )
		row.operation->addItem(tr(ModelAbstractRowFilter::ECompareOperationNames::name(i)));

	_rows.append(row);
	return _rows.back();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeRowFilter::accept() {
	if ( _filter != nullptr ) {
		ModelRowFilter *filter = new ModelRowFilter();

		foreach ( Row r, _rows ) {
			ModelAbstractRowFilter::CompareOperation op;
			int column;

			op.fromInt(r.operation->currentIndex()+1);

			switch ( r.column->currentIndex() ) {
				case 0:
					column = CHANNEL;
					break;
				case 1:
					column = MAGNITUDE;
					break;
				case 2:
					column = RESIDUAL;
					break;
				case 3:
					column = DISTANCE;
					break;
				default:
					QMessageBox::critical(this, tr("Error"), tr("Internal error: invalid column %1").arg(r.column->currentIndex()));
					delete filter;
					return;
			}

			ModelAbstractRowFilter *stage = nullptr;

			switch ( column ) {
				case CHANNEL:
				{
					QString value = r.value->text();
					stage = new ModelFieldValueFilter<QString>(column, op, value);
					break;
				}
				case MAGNITUDE:
				case RESIDUAL:
				case DISTANCE:
				{
					bool ok;
					double value = r.value->text().toDouble(&ok);
					if ( ok ) {
						if ( column == DISTANCE )
							stage = new ModelDistanceFilter(column, op, value);
						else
							stage = new ModelFieldValueFilter<double>(column, op, value);
					}
					else {
						QMessageBox::critical(this, tr("Error"), tr("Expected double value"));
						delete filter;
						return;
					}
					break;
				}
				default:
					QMessageBox::critical(this, tr("Error"), tr("Internal error: invalid target column %1").arg(column));
					delete filter;
					return;
			}

			filter->add(stage);
		}

		if ( *_filter != nullptr )
			delete *_filter;

		*_filter = filter;

		QSettings &s = SCApp->settings();
		s.beginGroup("MagnitudeView");
		s.setValue("selectionFilter", filter->toString());
		s.endGroup();
	}

	QDialog::accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeRowFilter::addFilter() {
	static_cast<QVBoxLayout*>(_ui.frameFilters->layout())->addLayout(addRow().layout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeRowFilter::removeFilter() {
	if ( _rows.size() <= 1 ) return;

	delete _rows.back().layout;
	delete _rows.back().column;
	delete _rows.back().operation;
	delete _rows.back().value;
	_rows.pop_back();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


ModelAbstractRowFilter *&selectionFilter() {
	static ModelAbstractRowFilter *selectionFilter = nullptr;
	static bool filterReadFromSettings = false;

	if ( !filterReadFromSettings ) {
		QSettings &s = SCApp->settings();
		s.beginGroup("MagnitudeView");
		QString f_str = s.value("selectionFilter").toString();
		if ( !f_str.isEmpty() ) {
			ModelRowFilter *filter = new ModelRowFilter;
			if ( !filter->fromString(f_str) ) {
				delete filter;
				QMessageBox::warning(nullptr, SCApp->tr("Settings"), SCApp->tr("Could not restore magnitude selection filter"), QMessageBox::Ok);
			}
			else {
				if ( selectionFilter != nullptr )
					delete selectionFilter;
				selectionFilter = filter;
			}
		}
		s.endGroup();
		filterReadFromSettings = true;
	}

	return selectionFilter;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! Implementation of MagnitudeView
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::MagnitudeView(const MapsDesc &maps,
                             Seiscomp::DataModel::DatabaseQuery *reader,
                             QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
, _d_ptr(new MagnitudeViewPrivate(reader, new Map::ImageTree(maps))) {
	SC_D.modelStationMagnitudes = new StationMagnitudeModel(&SC_D.objCache, this);
	SC_D.modelStationMagnitudesProxy = nullptr;
	init(reader);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::MagnitudeView(Map::ImageTree *mapTree,
                             Seiscomp::DataModel::DatabaseQuery *reader,
                             QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
, _d_ptr(new MagnitudeViewPrivate(reader, mapTree)) {
	SC_D.modelStationMagnitudes = new StationMagnitudeModel(&SC_D.objCache, this);
	SC_D.modelStationMagnitudesProxy = nullptr;
	init(reader);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::closeEvent(QCloseEvent *e) {
	if ( SC_D.amplitudeView ) {
		SC_D.amplitudeView->close();
	}
	QWidget::closeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::closeTab(int idx) {
	std::string magID = SC_D.tabMagnitudes->tabData(idx).value<TabData>().publicID;
	MagnitudePtr mag = Magnitude::Find(magID);

	if ( mag) {
		if ( mag->detach() ) {
			emit magnitudeRemoved(SC_D.origin->publicID().c_str(), mag.get());
			SC_D.tabMagnitudes->removeTab(idx);
		}
		else {
			QMessageBox::critical(this, "Error", tr("An error occured while removing magnitude %1").arg(magID.c_str()));
		}
	}
	else {
		QMessageBox::critical(this, "Error", tr("Did not find magnitude %1").arg(magID.c_str()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::debugCreateMagRef() {
	StationMagnitudePtr staMag = StationMagnitude::Create();
	staMag->setMagnitude(10.0);
	staMag->setType(SC_D.netMag->type());
	staMag->setWaveformID(WaveformStreamID("II", "KAPI", "00", "BH", ""));

	StationMagnitudeContributionPtr magRef = new StationMagnitudeContribution(staMag->publicID());
	SC_D.netMag->add(magRef.get());
	addObject(SC_D.netMag->publicID().c_str(), magRef.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::init(Seiscomp::DataModel::DatabaseQuery *) {
	SC_D.ui.setupUi(this);

	SC_D.amplitudeView = nullptr;
	SC_D.computeMagnitudesSilently = false;
	SC_D.enableMagnitudeTypeSelection = true;

	QObject *drawFilter = new ElideFadeDrawer(this);

	SC_D.ui.labelRegion->setFont(SCScheme.fonts.heading3);
	SC_D.ui.labelRegion->installEventFilter(drawFilter);
	//SC_D.ui.comboMagType->setFont(SCScheme.fonts.highlight);

	SC_D.ui.label_7->setFont(SCScheme.fonts.normal);
	SC_D.ui.label->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_8->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_3->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_2->setFont(SCScheme.fonts.normal);

	SC_D.ui.labelMagnitude->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelRMS->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelNumStaMags->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelMinMag->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelMaxMag->setFont(SCScheme.fonts.normal);

	/*
	_ui.lbAgencyID->setFont(SCScheme.fonts.normal);
	_ui.lbAuthor->setFont(SCScheme.fonts.normal);
	_ui.labelAgencyID->setFont(SCScheme.fonts.highlight);
	_ui.labelAuthor->setFont(SCScheme.fonts.highlight);
	_ui.lbPublicID->setFont(SCScheme.fonts.normal);
	_ui.labelMethod->setFont(SCScheme.fonts.highlight);
	*/

	setBold(SC_D.ui.labelAgencyID, true);
	setBold(SC_D.ui.labelAuthor, true);
	setBold(SC_D.ui.labelMethod, true);

	ElideFadeDrawer *elider = new ElideFadeDrawer(this);

	fixWidth(SC_D.ui.labelMethod, 8);
	SC_D.ui.labelMethod->installEventFilter(elider);
	//fixWidth(_ui.labelAgencyID, 8);
	SC_D.ui.labelAgencyID->installEventFilter(elider);
	SC_D.ui.labelAuthor->installEventFilter(elider);

	SC_D.ui.btnCommit->setFont(SCScheme.fonts.highlight);

	SC_D.ui.cbEvalStatus->addItem("- unset -");
	for ( int i = 0; i < 6; ++i ) {
		SC_D.ui.cbEvalStatus->addItem(QString());
	}
	for ( size_t i = EvaluationStatus::First; i < EvaluationStatus::Quantity; ++i ) {
		if ( EvaluationStatus::Type(i) == FINAL ) {
			SC_D.ui.cbEvalStatus->setItemText(1, EvaluationStatus::NameDispatcher::name(i));
		}
		else if ( EvaluationStatus::Type(i) == REVIEWED ) {
			SC_D.ui.cbEvalStatus->setItemText(2, EvaluationStatus::NameDispatcher::name(i));
		}
		else if ( EvaluationStatus::Type(i) == CONFIRMED ) {
			SC_D.ui.cbEvalStatus->setItemText(3, EvaluationStatus::NameDispatcher::name(i));
		}
		else if ( EvaluationStatus::Type(i) == PRELIMINARY ) {
			SC_D.ui.cbEvalStatus->setItemText(4, EvaluationStatus::NameDispatcher::name(i));
		}
		else if ( EvaluationStatus::Type(i) == REPORTED ) {
			SC_D.ui.cbEvalStatus->setItemText(5, EvaluationStatus::NameDispatcher::name(i));
		}
		else if ( EvaluationStatus::Type(i) == REJECTED ) {
			SC_D.ui.cbEvalStatus->setItemText(6, EvaluationStatus::NameDispatcher::name(i));
		}
		else {
			SC_D.ui.cbEvalStatus->addItem(EvaluationStatus::NameDispatcher::name(i));
		}
	}

	/*
	QAction* debugAction = new QAction(this);
	debugAction->setShortcut(Qt::Key_C);
	addAction(debugAction);
	connect(debugAction, SIGNAL(triggered()),
	        this, SLOT(debugCreateMagRef()));
	*/

	SC_D.stamagnitudes = new DiagramWidget(SC_D.ui.groupMagnitudes);
	if ( SCScheme.unit.distanceInKM ) {
		SC_D.stamagnitudes->setAbscissaName("Epi. distance (km)");
	}
	else {
		SC_D.stamagnitudes->setAbscissaName("Epi. distance (°)");
	}
	SC_D.stamagnitudes->setOrdinateName("Residual");
	SC_D.stamagnitudes->setMarkerDistance(10, 0.1);
	SC_D.stamagnitudes->setDisplayRect(QRectF(0,-2,180,4));
	SC_D.stamagnitudes->setValueDisabledColor(SCScheme.colors.magnitudes.disabled);
	SC_D.ui.groupMagnitudes->layout()->addWidget(SC_D.stamagnitudes);

	SC_D.map = new MagnitudeMap(SC_D.maptree.get(), SC_D.ui.frameMap);

	if ( SC_D.map ) {
		connect(SC_D.map, SIGNAL(magnitudeChanged(int,bool)),
		        this, SLOT(changeStationState(int,bool)));
		connect(SC_D.map, SIGNAL(clickedMagnitude(int)), this, SLOT(selectMagnitude(int)));
		connect(SC_D.map, SIGNAL(clickedStation(const std::string &, const std::string &)), this, SLOT(selectStation(const std::string &, const std::string &)));
	}

	auto hboxLayout = new QHBoxLayout(SC_D.ui.frameMap);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	if ( SC_D.map ) {
		SC_D.map->setMouseTracking(true);
		hboxLayout->addWidget(SC_D.map);

		try {
			SC_D.map->setStationsMaxDist(SCApp->configGetDouble("olv.map.stations.unassociatedMaxDist"));
		}
		catch ( ... ) {}
	}

	hboxLayout = new QHBoxLayout(SC_D.ui.frameMagnitudeTypes);
	hboxLayout->setContentsMargins(0, 0, 0, 0);

	SC_D.tabMagnitudes = new QTabBar(SC_D.ui.frameMagnitudeTypes);
	SC_D.tabMagnitudes->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
	SC_D.tabMagnitudes->setShape(QTabBar::RoundedNorth);
	SC_D.tabMagnitudes->setUsesScrollButtons(true);

	connect(SC_D.tabMagnitudes, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

	hboxLayout->addWidget(SC_D.tabMagnitudes);

	// reset GUI to default
	resetContent();

	SC_D.ui.tableStationMagnitudes->horizontalHeader()->setSortIndicatorShown(true);
	SC_D.ui.tableStationMagnitudes->horizontalHeader()->setSortIndicator(DISTANCE, Qt::AscendingOrder);
	//SC_D.ui.tableStationMagnitudes->horizontalHeader()->setStretchLastSection(true);
	SC_D.ui.tableStationMagnitudes->setContextMenuPolicy(Qt::CustomContextMenu);
	SC_D.ui.tableStationMagnitudes->setSelectionMode(QAbstractItemView::ExtendedSelection);
	SC_D.ui.tableStationMagnitudes->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(SC_D.ui.tableStationMagnitudes->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableStationMagnitudesHeaderContextMenuRequested(const QPoint &)));

	connect(SC_D.ui.tableStationMagnitudes->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
	        SC_D.ui.tableStationMagnitudes, SLOT(sortByColumn(int, Qt::SortOrder)));
	connect(SC_D.ui.tableStationMagnitudes, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableStationMagnitudesContextMenuRequested(const QPoint &)));

	// on selection in  diagram
	connect(SC_D.stamagnitudes, SIGNAL(valueActiveStateChanged(int, bool)), this, SLOT(changeMagnitudeState(int, bool)));
	connect(SC_D.stamagnitudes, SIGNAL(endSelection()), this, SLOT(magnitudesSelected()));
	connect(SC_D.stamagnitudes, SIGNAL(adjustZoomRect(QRectF&)), this, SLOT(adjustMagnitudeRect(QRectF&)));
	connect(SC_D.stamagnitudes, SIGNAL(hover(int)), this, SLOT(hoverMagnitude(int)));
	connect(SC_D.stamagnitudes, SIGNAL(clicked(int)), this, SLOT(selectMagnitude(int)));

	// on selection in table
	connect(SC_D.modelStationMagnitudes, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

	connect(SC_D.ui.btnRecalculate, SIGNAL(clicked()), this, SLOT(recalculateMagnitude()));
	connect(SC_D.ui.btnSelect, SIGNAL(clicked()), this, SLOT(selectChannels()));
	connect(SC_D.ui.btnActivate, SIGNAL(clicked()), this, SLOT(activateChannels()));
	connect(SC_D.ui.btnDeactivate, SIGNAL(clicked()), this, SLOT(deactivateChannels()));
	connect(SC_D.ui.btnWaveforms, SIGNAL(clicked()), this, SLOT(openWaveforms()));
	connect(SC_D.ui.cbEvalStatus, SIGNAL(currentIndexChanged(int)), this, SLOT(evaluationStatusChanged(int)));

	QMenu *selectMenu = new QMenu(SC_D.ui.btnSelect);
	QAction *editSelectionFilter = new QAction(tr("Edit"), this);
	editSelectionFilter->setShortcut(QKeySequence("shift+s"));
	selectMenu->addAction(editSelectionFilter);

	SC_D.ui.btnSelect->setMenu(selectMenu);
	connect(editSelectionFilter, SIGNAL(triggered()), this, SLOT(selectChannelsWithEdit()));

	//NOTE: Set to visible if you want to activate magnitude recalculation and
	//      comitting: Very alpha and comitting does still not work
	SC_D.ui.btnCommit->setVisible(false);
	//_ui.btnWaveforms->setVisible(false);

	SC_D.ui.groupReview->setEnabled(false);

	try {
		SC_D.magnitudeTypes = SCApp->configGetStrings("magnitudes");
	}
	catch ( ... ) {
		SC_D.magnitudeTypes.push_back("MLv");
		SC_D.magnitudeTypes.push_back("mb");
		SC_D.magnitudeTypes.push_back("mB");
		SC_D.magnitudeTypes.push_back("Mwp");
	}

	// Gather available magnitudes types
	SC_D.availableMagTypes = Processing::MagnitudeProcessorFactory::Services();
	if ( SC_D.availableMagTypes != nullptr ) {
		for ( size_t i = 0; i < SC_D.magnitudeTypes.size(); ) {
			if ( std::find(SC_D.availableMagTypes->begin(), SC_D.availableMagTypes->end(), SC_D.magnitudeTypes[i])
			     == SC_D.availableMagTypes->end() ) {
				SEISCOMP_WARNING("Removing unavailable magnitude: %s", SC_D.magnitudeTypes[i]);
				SC_D.magnitudeTypes.erase(SC_D.magnitudeTypes.begin() + i);
			}
			else
				++i;
		}
	}

	SC_D.currentMagnitudeTypes = SC_D.magnitudeTypes;

	try {
		int baseIdx = SC_D.ui.layoutActions->indexOf(SC_D.ui.cbEvalStatus);
		if ( baseIdx < 0 ) {
			baseIdx = SC_D.ui.layoutActions->count() - 1;
		}

		auto profiles = SCApp->configGetStrings("olv.magnitudeComments");
		set<string> ids;

		for ( const auto &profile : profiles ) {
			MagnitudeCommentProfile commentProfile;

			try {
				commentProfile.id = SCApp->configGetString("olv.magnitudeComments." + profile + ".id");
				if ( commentProfile.id.empty() ) {
					SEISCOMP_WARNING("olv.magnitudeComments.%s.id is empty: ignoring",
					                 profile);
					continue;
				}

				if ( ids.find(commentProfile.id) != ids.end() ) {
					SEISCOMP_WARNING("Duplicate olv.magnitudeComments.%s.id: ignoring",
					                 profile);
					continue;
				}

				commentProfile.label = SCApp->configGetString("olv.magnitudeComments." + profile + ".label");
				if ( commentProfile.label.empty() ) {
					SEISCOMP_WARNING("olv.magnitudeComments.%s.label is empty: ignoring",
					                 profile);
					continue;
				}

				commentProfile.options = SCApp->configGetStrings("olv.magnitudeComments." + profile + ".options");
			}
			catch ( exception &e ) {
				SEISCOMP_WARNING("olv.magnitudeComments: %s, ignoring %s",
				                 e.what(), profile);
				continue;
			}

			try {
				commentProfile.allowFreeText = SCApp->configGetBool("olv.magnitudeComments." + profile + ".allowFreeText");
			}
			catch ( ... ) {}

			ids.insert(commentProfile.id);

			SC_D.ui.layoutActions->insertWidget(baseIdx + 1, new QLabel((commentProfile.label + ":").c_str()));

			auto comboComment = new QComboBox;
			comboComment->setObjectName(QString("comboBox/magnitude/comment/%1").arg(commentProfile.id.c_str()));
			comboComment->setProperty("id", QString(commentProfile.id.c_str()));
			comboComment->setEditable(commentProfile.allowFreeText);
			comboComment->setToolTip(
				tr("Populates comment with id '%1'")
				.arg(commentProfile.id.c_str())
			);
			for ( auto &option : commentProfile.options ) {
				comboComment->addItem(option.c_str());
			}
			auto idx = comboComment->findText(commentProfile.value.c_str());
			if ( idx >= 0 ) {
				comboComment->setCurrentIndex(idx);
			}
			else if ( commentProfile.allowFreeText ) {
				comboComment->setEditText(commentProfile.value.c_str());
			}

			connect(comboComment, SIGNAL(currentTextChanged(QString)),
			        this, SLOT(magnitudeCommentChanged(QString)));

			SC_D.ui.layoutActions->insertWidget(baseIdx + 2, comboComment);

			baseIdx += 2;
		}
	}
	catch ( ... ) {}

	setReadOnly(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::~MagnitudeView() {
	if ( SC_D.availableMagTypes ) {
		delete SC_D.availableMagTypes;
	}

	delete _d_ptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget *MagnitudeView::map() const {
	return SC_D.map;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setPreferredMagnitudeID(const string &id) {
	SC_D.preferredMagnitudeID = id;

	//QColor col = palette().color(QPalette::WindowText);
	for ( int i = 0; i < SC_D.tabMagnitudes->count(); ++i ) {
		TabData d = SC_D.tabMagnitudes->tabData(i).value<TabData>();
		if ( d.publicID == SC_D.preferredMagnitudeID ) {
			//SC_D.tabMagnitudes->setTabTextColor(i, Qt::green);
			SC_D.tabMagnitudes->setTabIcon(i, icon("tab_ok"));
			resetPreferredMagnitudeSelection();
		}
		else {
			//SC_D.tabMagnitudes->setTabTextColor(i, col);
			SC_D.tabMagnitudes->setTabIcon(i, QIcon());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeView::setDefaultAggregationType(const std::string &type) {
	if ( type == "mean"
	  || type == "trimmedMean"
	  || type == "median"
	  || type == "medianTrimmedMean" ) {
		SC_D.defaultMagnitudeAggregation = type;
		return true;
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::recalculateMagnitude() {
	// Recalculate the magnitudes according the defined settings
	if ( !SC_D.netMag ) {
		return;
	}

	vector<double> mags;
	vector<double> weights;

	for ( int i = 0; i < SC_D.modelStationMagnitudes->rowCount(); ++i ) {
		if ( static_cast<StationMagnitudeModel*>(SC_D.modelStationMagnitudes)->useMagnitude(i) ) {
			StationMagnitude* sm = StationMagnitude::Find(SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
			if ( !sm ) {
				QMessageBox::critical(this, "Error", QString("StationMagnitude %1 not found").arg(SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID().c_str()));
				return;
			}

			mags.push_back(sm->magnitude().value());
		}
	}

	if ( mags.empty() ) {
		QMessageBox::critical(this, "Error", "At least one station magnitude must be selected");
		return;
	}

	double netmag, stdev;

	if ( SC_D.ui.btnDefault->isChecked() ) {
		if ( mags.size() < 4 ) {
			Math::Statistics::computeMean(mags, netmag, stdev);
			weights.resize(mags.size(), 1.);
			SC_D.netMag->setMethodID("mean");
		}
		else {
			Math::Statistics::computeTrimmedMean(mags, 25.0, netmag, stdev, &weights);
			SC_D.netMag->setMethodID("trimmed mean(25)");
		}
	}
	else if ( SC_D.ui.btnMean->isChecked() ) {
		if ( !Math::Statistics::computeMean(mags, netmag, stdev) ) {
			QMessageBox::critical(this, "Error", "Recalculating the magnitude using the mean failed for unknown reason");
			return;
		}

		weights.resize(mags.size(), 1.);

		SC_D.netMag->setMethodID("mean");
	}
	else if ( SC_D.ui.btnTrimmedMean->isChecked() ) {
		if ( !Math::Statistics::computeTrimmedMean(mags, SC_D.ui.spinTrimmedMeanValue->value(), netmag, stdev, &weights) ) {
			QMessageBox::critical(this, "Error", "Recalculating the magnitude using the trimmed mean failed for unknown reason");
			return;
		}

		SC_D.netMag->setMethodID(Core::stringify("trimmed mean(%d)", SC_D.ui.spinTrimmedMeanValue->value()));
	}
	else if ( SC_D.ui.btnMedian->isChecked() ) {
		netmag = Math::Statistics::median(mags);
		if ( mags.size() > 1 ) {
			stdev = 0;
			for ( size_t i = 0; i < mags.size(); ++i )
				stdev += (mags[i] - netmag) * (mags[i] - netmag);
			stdev /= mags.size()-1;
			stdev = sqrt(stdev);
		}

		weights.resize(mags.size(), 1.);

		SC_D.netMag->setMethodID("median");
	}
	else if ( SC_D.ui.btnTrimmedMedian->isChecked() ) {
		if ( !Math::Statistics::computeMedianTrimmedMean(mags, SC_D.ui.spinTrimmedMedianValue->value(), netmag, stdev, &weights) ) {
			QMessageBox::critical(this, "Error", "Recalculating the magnitude using the median trimmed mean failed for unknown reason");
			return;
		}

		SC_D.netMag->setMethodID(Core::stringify("median trimmed mean(%f)", SC_D.ui.spinTrimmedMedianValue->value()));
	}
	else {
		QMessageBox::critical(this, "Error", "Please select a method to recalculate the magnitude.");
		return;
	}

	SC_D.netMag->setMagnitude(DataModel::RealQuantity(netmag, stdev, Core::None, Core::None, Core::None));
	SC_D.netMag->setEvaluationStatus(EvaluationStatus(CONFIRMED));

	SC_D.ui.cbEvalStatus->blockSignals(true);
	SC_D.ui.cbEvalStatus->setCurrentIndex(SC_D.netMag->evaluationStatus().toInt()+1);
	SC_D.ui.cbEvalStatus->blockSignals(false);

	int idx = findType(SC_D.tabMagnitudes, SC_D.netMag->type().c_str());
	SC_D.tabMagnitudes->setTabTextColor(idx, QColor());
	SC_D.tabMagnitudes->setTabIcon(idx, QIcon());

	idx = 0;
	int staCount = 0;
	for ( int i = 0; i < SC_D.modelStationMagnitudes->rowCount(); ++i ) {
		if ( static_cast<StationMagnitudeModel*>(SC_D.modelStationMagnitudes)->useMagnitude(i) ) {
			if ( weights[idx] > 0.0 ) {
				++staCount;
			}

			SC_D.netMag->stationMagnitudeContribution(i)->setWeight(weights[idx]);
			++idx;
		}
		else {
			SC_D.netMag->stationMagnitudeContribution(i)->setWeight(0.);
		}
	}

	for ( int i = 0; i < SC_D.modelStationMagnitudes->rowCount(); ++i ) {
		if ( staCount ) {
			StationMagnitude* sm = StationMagnitude::Find(SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
			SC_D.netMag->stationMagnitudeContribution(i)->setResidual(sm->magnitude().value() - netmag);
		}
		else {
			SC_D.netMag->stationMagnitudeContribution(i)->setResidual(Core::None);
		}
	}

	SC_D.netMag->setStationCount(staCount);

	// Update corresponding Mw estimation
	Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(SC_D.netMag->type().c_str());
	if ( proc ) {
		double Mw, MwError;
		string type = proc->typeMw();

		if ( proc->estimateMw(&SCCoreApp->configuration(), netmag, Mw, MwError) == Processing::MagnitudeProcessor::OK ) {
			idx = findType(SC_D.tabMagnitudes, type.c_str());
			//int idx = _ui.comboMagType->findText(type.c_str());
			if ( idx != -1 ) {
				MagnitudePtr magMw =
					//Magnitude::Find(_ui.comboMagType->itemData(idx).value<QString>().toStdString());
					Magnitude::Find(SC_D.tabMagnitudes->tabData(idx).value<TabData>().publicID);

				if ( magMw && magMw != SC_D.netMag ) {
					stdev = stdev > MwError?stdev:MwError;
					magMw->setMagnitude(RealQuantity(Mw, stdev, Core::None, Core::None, Core::None));
					try {
						magMw->setStationCount(SC_D.netMag->stationCount());
					}
					catch ( ... ) {
						magMw->setStationCount(Core::None);
					}
					magMw->setEvaluationStatus(EvaluationStatus(CONFIRMED));
					emit magnitudeUpdated(SC_D.origin->publicID().c_str(), magMw.get());
				}

				SC_D.tabMagnitudes->setTabText(idx, QString("%1 %2 (%3/%4)")
				                           .arg(magMw->type().c_str())
				                           .arg(magMw->magnitude().value(), 0, 'f', SCScheme.precision.magnitude)
				                           .arg(usedStationCount(magMw.get()))
				                           .arg(totalStationCount(magMw.get())));
			}
			else {
				MagnitudePtr magMw;

				CreationInfo ci;
				ci.setAgencyID(SCApp->agencyID());
				ci.setAuthor(SCApp->author());
				ci.setCreationTime(Core::Time::UTC());
				stdev = stdev > MwError ? stdev : MwError;

				for ( size_t m = 0; m < SC_D.origin->magnitudeCount(); ++m ) {
					if ( SC_D.origin->magnitude(m)->type() == proc->typeMw() ) {
						magMw = SC_D.origin->magnitude(m);
						break;
					}
				}

				if ( !magMw ) {
					magMw = Magnitude::Create();
					SC_D.origin->add(magMw.get());
				}

				magMw->setCreationInfo(ci);
				magMw->setType(proc->typeMw());
				magMw->setMagnitude(RealQuantity(Mw, stdev, Core::None, Core::None, Core::None));

				try {
					magMw->setStationCount(SC_D.netMag->stationCount());
				}
				catch ( ... ) {
					magMw->setStationCount(Core::None);
				}

				addMagnitude(magMw.get());
			}
		}
		else {
			idx = findType(SC_D.tabMagnitudes, type.c_str());
			if ( idx != -1 ) {
				SC_D.tabMagnitudes->removeTab(idx);
			}
		}
	}

	updateTab(SC_D.tabMagnitudes, SC_D.netMag.get());

	updateMagnitudeLabels();
	SC_D.ui.tableStationMagnitudes->reset();

	emit magnitudeUpdated(SC_D.origin->publicID().c_str(), SC_D.netMag.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectChannels() {
	if ( selectionFilter() == nullptr ) {
		if ( !editSelectionFilter() )
			return;
	}

	ModelAbstractRowFilter *filter = selectionFilter();

	if ( filter == nullptr )
		return;

	SC_D.ui.tableStationMagnitudes->selectionModel()->clear();

	int rowCount = SC_D.modelStationMagnitudesProxy->rowCount();
	for ( int i = 0; i < rowCount; ++i ) {
		if ( reinterpret_cast<ModelAbstractRowFilter*>(filter)->passes(SC_D.modelStationMagnitudesProxy, i) )
			SC_D.ui.tableStationMagnitudes->selectionModel()->select(SC_D.modelStationMagnitudesProxy->index(i, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectChannelsWithEdit() {
	if ( !editSelectionFilter() )
		return;

	selectChannels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::activateChannels() {
	QModelIndexList rows = SC_D.ui.tableStationMagnitudes->selectionModel()->selectedRows();
	foreach ( const QModelIndex &idx, rows )
		changeStationState(SC_D.modelStationMagnitudesProxy->mapToSource(idx).row(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::deactivateChannels() {
	QModelIndexList rows = SC_D.ui.tableStationMagnitudes->selectionModel()->selectedRows();
	foreach ( const QModelIndex &idx, rows )
		changeStationState(SC_D.modelStationMagnitudesProxy->mapToSource(idx).row(), false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::openWaveforms() {
	if ( !SC_D.netMag ) return;

	if ( SC_D.amplitudeView ) {
		if ( SC_D.amplitudeView->currentMagnitudeType() == SC_D.netMag->type() ) {
			SC_D.amplitudeView->activateWindow();
			SC_D.amplitudeView->raise();
			return;
		}
		else {
			if ( QMessageBox::question(this, "Waveform review",
			                           QString("A waveform review window for type %1 is still active.\n"
			                                   "Do you want to replace it with current type %2?")
			                           .arg(SC_D.amplitudeView->currentMagnitudeType().c_str(),
			                                SC_D.netMag->type().c_str()),
			                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
			     == QMessageBox::No )
				return;
		}
	}
	else {
		SC_D.amplitudeView = new AmplitudeView(nullptr, Qt::Window);
		SC_D.amplitudeView->setAttribute(Qt::WA_DeleteOnClose);
		SC_D.amplitudeView->setDatabase(SC_D.reader);

		try {
			SC_D.amplitudeView->setStrongMotionCodes(SCApp->configGetStrings("picker.accelerationChannelCodes"));
		}
		catch ( ... ) {}

		connect(SC_D.amplitudeView, SIGNAL(magnitudeCreated(Seiscomp::DataModel::Magnitude*)),
		        this, SLOT(magnitudeCreated(Seiscomp::DataModel::Magnitude*)));
		connect(SC_D.amplitudeView, SIGNAL(amplitudesConfirmed(Seiscomp::DataModel::Origin*, QList<Seiscomp::DataModel::AmplitudePtr>)),
		        this, SLOT(amplitudesConfirmed(Seiscomp::DataModel::Origin*, QList<Seiscomp::DataModel::AmplitudePtr>)));
		connect(SC_D.amplitudeView, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
	}

	SC_D.amplitudeView->setConfig(SC_D.amplitudeConfig);

	if ( !SC_D.amplitudeView->setOrigin(SC_D.origin.get(), SC_D.netMag->type()) ) {
		delete SC_D.amplitudeView;
		SC_D.amplitudeView = nullptr;
		return;
	}

	SC_D.amplitudeView->show();
	SC_D.amplitudeView->raise();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::objectDestroyed(QObject *o) {
	if ( o == SC_D.amplitudeView ) {
		SC_D.amplitudeView = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::computeMagnitudes() {
	try {
		SC_D.origin->depth();
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Origin.depth not set: recompute magnitudes failed");
		if ( !SC_D.computeMagnitudesSilently ) {
			QMessageBox::critical(this, tr("Compute magnitudes"),
			                      tr("Origin.depth is not set, cannot compute "
			                      "magnitudes."));
		}
		return;
	}

	if ( SC_D.availableMagTypes == nullptr ) {
		QMessageBox::critical(this, tr("Compute magnitudes"),
		                      tr("No magnitude processors available."));
		return;
	}

	std::vector<std::string> magnitudeTypes;

	if ( SC_D.enableMagnitudeTypeSelection && !SC_D.computeMagnitudesSilently ) {
		QDialog setupTypesDlg;
		setupTypesDlg.setWindowTitle(tr("Select magnitude types"));

		QVBoxLayout *ml = new QVBoxLayout;
		QHBoxLayout *hl;
		QVBoxLayout *vl;

		// Center layout
		hl = new QHBoxLayout;

		// Right tool buttons
		vl = new QVBoxLayout;
		QPushButton *selectAll = new QPushButton(tr("Select all"));
		vl->addWidget(selectAll);
		QPushButton *deselectAll = new QPushButton(tr("Deselect all"));
		vl->addWidget(deselectAll);
		vl->addStretch();

		hl->addLayout(vl);

		// Magnitude type grid
		vl = new QVBoxLayout;

		QGridLayout *grid = new QGridLayout;
		QVector<QCheckBox*> typeChecks;

		for ( size_t i = 0; i < SC_D.availableMagTypes->size(); ++i ) {
			CheckBox *check = new CheckBox;
			check->setChecked(std::find(SC_D.currentMagnitudeTypes.begin(), SC_D.currentMagnitudeTypes.end(), SC_D.availableMagTypes->at(i)) != SC_D.currentMagnitudeTypes.end());
			connect(selectAll, &CheckBox::clicked, check, &CheckBox::check);
			connect(deselectAll, &CheckBox::clicked, check, &CheckBox::unCheck);

			QLabel *label = new QLabel;
			label->setText(SC_D.availableMagTypes->at(i).c_str());

			grid->addWidget(check, i, 0);
			grid->addWidget(label, i, 1);

			typeChecks.append(check);
		}

		QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
		grid->addItem(spacer, SC_D.availableMagTypes->size(), 0);
		grid->setColumnStretch(0, 0);
		grid->setColumnStretch(1, 1);
		grid->setContentsMargins(0, 0, 0, 0);

		QWidget *typeSelections = new QWidget;
		typeSelections->setLayout(grid);

		QScrollArea *scrollArea = new QScrollArea;
		scrollArea->setFrameShape(QFrame::NoFrame);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidget(typeSelections);
		scrollArea->setWidgetResizable(true);

		vl->addWidget(scrollArea);

		hl->insertLayout(0, vl);

		QLabel *info = new QLabel;
		info->setText(tr("Select the magnitudes that should be computed."));
		ml->addWidget(info);

		QFrame *hline = new QFrame;
		hline->setFrameShape(QFrame::HLine);
		ml->addWidget(hline);

		ml->addLayout(hl);

		// Main buttons
		hl = new QHBoxLayout;

		QPushButton *ok = new QPushButton(tr("OK"));
		connect(ok, &QPushButton::clicked, &setupTypesDlg, &QDialog::accept);
		QPushButton *cancel = new QPushButton(tr("Cancel"));
		connect(cancel, &QPushButton::clicked, &setupTypesDlg, &QDialog::reject);

		hl->addStretch();
		hl->addWidget(ok);
		hl->addWidget(cancel);

		ml->addLayout(hl);

		ok->setDefault(true);
		ok->setAutoDefault(true);
		ok->setFocus();

		setupTypesDlg.setLayout(ml);

		if ( setupTypesDlg.exec() != QDialog::Accepted ) {
			return;
		}

		for ( int i = 0; i < typeChecks.size(); ++i ) {
			QCheckBox *check = typeChecks[i];
			if ( check->isChecked() ) {
				magnitudeTypes.push_back(SC_D.availableMagTypes->at(i));
			}
		}
	}
	else {
		magnitudeTypes = SC_D.magnitudeTypes;
	}

	// Save the last choice
	SC_D.currentMagnitudeTypes = magnitudeTypes;

	// Gather all amplitude types required to compute the magnitudes
	CalculateAmplitudes::TypeSet ampTypes;
	for ( size_t i = 0; i < magnitudeTypes.size(); ++i ) {
		Processing::MagnitudeProcessorPtr proc;
		proc = Processing::MagnitudeProcessorFactory::Create(magnitudeTypes[i].c_str());
		if ( proc ) {
			ampTypes.insert(proc->amplitudeType());
			SEISCOMP_DEBUG("+ %s : %s", magnitudeTypes[i], proc->amplitudeType());
		}
	}

	CalculateAmplitudes dlg(SC_D.origin.get(), this);
	static QByteArray geomCalculateAmplitudes;

	dlg.restoreGeometry(geomCalculateAmplitudes);
	// TODO: This should go into the settings panel
	dlg.setRecomputeAmplitudes(false);
	dlg.setAmplitudeTypes(ampTypes);
	dlg.setDatabase(SC_D.reader);
	dlg.setStreamURL(SC_D.amplitudeConfig.recordURL.toStdString());
	dlg.setAmplitudeCache(&SC_D.amplitudes);
	dlg.setSilentMode(SC_D.computeMagnitudesSilently);
	if ( dlg.exec() != QDialog::Accepted ) {
		geomCalculateAmplitudes = dlg.saveGeometry();
		return;
	}

	geomCalculateAmplitudes = dlg.saveGeometry();

	QList<Seiscomp::DataModel::AmplitudePtr> sessionAmplitudes;

	CalculateAmplitudes::iterator it;
	for ( it = dlg.begin(); it != dlg.end(); ++it ) {
		sessionAmplitudes.append(it->second.first);

		// Find all cached amplitudes for the current pick
		auto itp = SC_D.amplitudes.equal_range(it->first);

		// Check if this pick-amplitude association has been registered already
		bool found = false;
		for ( auto ita = itp.first; ita != itp.second; ++ita ) {
			if ( ita->second.first == it->second.first ) {
				found = true;
				break;
			}
		}

		if ( !found ) {
			SC_D.amplitudes.insert(MagnitudeViewPrivate::PickAmplitudeMap::value_type(it->first, it->second));
		}
	}

	SEISCOMP_DEBUG("Amplitude cache size: %d", SC_D.amplitudes.size());

	MagnitudeStats magErrors;

	for ( size_t i = 0; i < magnitudeTypes.size(); ++i ) {
		Magnitude *mag = computeStationMagnitudes(magnitudeTypes[i], &sessionAmplitudes, &magErrors);
		if ( mag ) {
			computeMagnitude(mag, SC_D.defaultMagnitudeAggregation?*SC_D.defaultMagnitudeAggregation:"");

			Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(mag->type().c_str());
			if ( proc ) {
				double stddev = 0;
				double Mw, MwError;
				if ( proc->estimateMw(&SCCoreApp->configuration(), mag->magnitude(), Mw, MwError) == Processing::MagnitudeProcessor::OK ) {
					try {
						stddev = mag->magnitude().uncertainty();
					}
					catch ( ... ) {}

					CreationInfo ci;
					ci.setAgencyID(SCApp->agencyID());
					ci.setAuthor(SCApp->author());
					ci.setCreationTime(Core::Time::UTC());
					stddev = stddev > MwError ? stddev : MwError;
					MagnitudePtr MagMw;

					for ( size_t m = 0; m < SC_D.origin->magnitudeCount(); ++m ) {
						if ( SC_D.origin->magnitude(m)->type() == proc->typeMw() ) {
							MagMw = SC_D.origin->magnitude(m);
							break;
						}
					}

					if ( !MagMw ) MagMw = Magnitude::Create();

					MagMw->setCreationInfo(ci);
					MagMw->setType(proc->typeMw());
					MagMw->setMagnitude(RealQuantity(Mw, stddev, Core::None, Core::None, Core::None));
					try {
						MagMw->setStationCount(mag->stationCount());
					}
					catch ( ... ) {
						MagMw->setStationCount(Core::None);

					}
					SC_D.origin->add(MagMw.get());
				}
			}
		}
		else {
			MagnitudePtr mag = Magnitude::Create();
			mag->setType(magnitudeTypes[i]);
			mag->setEvaluationStatus(EvaluationStatus(REJECTED));
			mag->setMagnitude(RealQuantity(INVALID_MAG));

			CreationInfo ci;
			ci.setAgencyID(SCApp->agencyID());
			ci.setAuthor(SCApp->author());
			ci.setCreationTime(Core::Time::UTC());

			mag->setCreationInfo(ci);

			SC_D.origin->add(mag.get());
		}
	}

	if ( !magErrors.isEmpty() && !SC_D.computeMagnitudesSilently ) {
		static QByteArray geomCalculateMagnitudes;

		QDialog dlg;
		dlg.setWindowTitle(tr("Compute magnitudes"));
		dlg.restoreGeometry(geomCalculateMagnitudes);

		QTableWidget *table = new QTableWidget;
		QVBoxLayout *vlayout = new QVBoxLayout;
		dlg.setLayout(vlayout);
		vlayout->addWidget(table);

		QHBoxLayout *hlayout = new QHBoxLayout;
		hlayout->addStretch();
		QPushButton *btnOK = new QPushButton(tr("OK"));
		hlayout->addWidget(btnOK);
		connect(btnOK, SIGNAL(pressed()), &dlg, SLOT(accept()));
		vlayout->addLayout(hlayout);

		table->setColumnCount(3);

		table->horizontalHeader()->setStretchLastSection(true);
		table->horizontalHeader()->setVisible(true);
		table->setHorizontalHeaderLabels(QStringList() << tr("Channel") << tr("Type") << tr("Info"));

		QColor orange(255,128,0);

		// Populate errors
		foreach ( const MagnitudeStatus &s, magErrors ) {
			QTableWidgetItem *itemStream = new QTableWidgetItem(waveformIDToStdString(s.amplitude->waveformID()).c_str());
			itemStream->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QTableWidgetItem *itemType = new QTableWidgetItem(s.type.c_str());
			itemType->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QTableWidgetItem *itemState = new QTableWidgetItem(s.status.toString());
			if ( s.warning ) {
				itemState->setData(Qt::ForegroundRole, orange);
			}
			itemState->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

			int row = table->rowCount();
			table->insertRow(row);

			table->setItem(row, 0, itemStream);
			table->setItem(row, 1, itemType);
			table->setItem(row, 2, itemState);
		}

		table->resizeColumnsToContents();
		table->setSortingEnabled(true);

		dlg.exec();
		geomCalculateMagnitudes = dlg.saveGeometry();
	}

	// Synchronize local amplitudes for commit
	set<string> ampIDs;

	for ( size_t i = 0; i < SC_D.origin->stationMagnitudeCount(); ++i ) {
		if ( SC_D.origin->stationMagnitude(i)->amplitudeID().empty() ) continue;
		ampIDs.insert(SC_D.origin->stationMagnitude(i)->amplitudeID());
	}

	AmplitudeSet ampSet;

	for ( auto it = SC_D.amplitudes.begin(); it != SC_D.amplitudes.end(); ++it ) {
		// Skip external amplitudes
		if ( !it->second.second ) {
			continue;
		}

		// Skip unrequested amplitudes
		if ( ampIDs.find(it->second.first->publicID()) == ampIDs.end() ) {
			continue;
		}

		ampSet.insert(AmplitudeSet::value_type(it->second.first, true));
	}

	emit localAmplitudesAvailable(SC_D.origin.get(), &ampSet, &ampIDs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::magnitudeCreated(Seiscomp::DataModel::Magnitude *netMag) {
	AmplitudeView *view = (AmplitudeView*)sender();
	ObjectChangeList<DataModel::Amplitude> changedAmps;
	view->getChangedAmplitudes(changedAmps);

	if ( netMag->origin() != SC_D.origin ) return;

	if ( SC_D.ui.btnMean->isChecked() ) {
		computeMagnitude(netMag, "mean");
	}
	else if ( SC_D.ui.btnTrimmedMean->isChecked() ) {
		computeMagnitude(netMag, "trimmedMean");
	}
	else if ( SC_D.ui.btnMedian->isChecked() ) {
		computeMagnitude(netMag, "median");
	}
	else if ( SC_D.ui.btnTrimmedMedian->isChecked() ) {
		computeMagnitude(netMag, "medianTrimmedMean");
	}
	else {
		computeMagnitude(netMag, "");
	}

	for ( auto it = changedAmps.begin(); it != changedAmps.end(); ++it ) {
		SC_D.amplitudes.insert(
			MagnitudeViewPrivate::PickAmplitudeMap::value_type(
				it->first->pickID(), MagnitudeViewPrivate::AmplitudeEntry(it->first, true)
			)
		);
	}

	SEISCOMP_DEBUG("Amplitude cache size: %d", SC_D.amplitudes.size());

	// Estimate Mw
	Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(netMag->type().c_str());
	if ( proc ) {
		double Mw, MwError;
		string type = proc->typeMw();
		int idx = findType(SC_D.tabMagnitudes, type.c_str());
		if ( proc->estimateMw(&SCCoreApp->configuration(), netMag->magnitude().value(), Mw, MwError) == Processing::MagnitudeProcessor::OK ) {
			//int idx = SC_D.ui.comboMagType->findText(type.c_str());
			if ( idx != -1 ) {
				MagnitudePtr magMw =
					Magnitude::Find(SC_D.tabMagnitudes->tabData(idx).value<TabData>().publicID);

				if ( magMw ) {
					if ( MwError < netMag->magnitude().uncertainty() )
						MwError = netMag->magnitude().uncertainty();
					magMw->setMagnitude(RealQuantity(Mw, MwError, Core::None, Core::None, Core::None));
					magMw->setStationCount(netMag->stationCount());
					emit magnitudeUpdated(SC_D.origin->publicID().c_str(), magMw.get());
				}

				SC_D.tabMagnitudes->setTabText(idx, QString("%1 %2 (%3/%4)")
				                           .arg(magMw->type().c_str())
				                           .arg(magMw->magnitude().value(), 0, 'f', SCScheme.precision.magnitude)
				                           .arg(usedStationCount(magMw.get()))
				                           .arg(totalStationCount(magMw.get())));
			}
		}
		else if ( idx != -1 )
			SC_D.tabMagnitudes->removeTab(idx);
	}

	int typeIdx = findType(SC_D.tabMagnitudes, netMag->type().c_str());
	if ( typeIdx == -1 ) {
		typeIdx = addMagnitude(netMag);
		SC_D.tabMagnitudes->setCurrentIndex(typeIdx);
		emit magnitudeUpdated(SC_D.origin->publicID().c_str(), netMag);
		return;
	}

	/*
	data.setValue(QString(netMag->publicID().c_str()));
	int idx = findData(SC_D.tabMagnitudes, data);
	if ( idx != -1 ) {
		updateObject(SC_D.origin->publicID().c_str(), netMag);
		SC_D.tabMagnitudes->setCurrentIndex(idx);
		emit magnitudeUpdated(SC_D.origin->publicID().c_str(), netMag);
		return;
	}
	*/

	// Replace magnitude
	updateTab(SC_D.tabMagnitudes, netMag);
	SC_D.tabMagnitudes->setTabData(typeIdx, QVariant::fromValue<TabData>(netMag->publicID()));
	if ( SC_D.tabMagnitudes->currentIndex() != typeIdx ) {
		SC_D.tabMagnitudes->setCurrentIndex(typeIdx);
	}
	else {
		updateContent();
	}

	emit magnitudeUpdated(SC_D.origin->publicID().c_str(), netMag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::amplitudesConfirmed(Origin *origin,
                                        QList<Seiscomp::DataModel::AmplitudePtr> amps) {
	AmplitudeView *view = (AmplitudeView*)sender();
	ObjectChangeList<DataModel::Amplitude> changedAmps;
	view->getChangedAmplitudes(changedAmps);

	if ( origin != SC_D.origin ) return;
	try {
		SC_D.origin->depth();
	}
	catch ( ... ) {
		QMessageBox::critical(this, "Compute magnitudes",
		                      "Origin.depth is not set, cannot compute "
		                      "magnitudes.");
		return;
	}

	ObjectChangeList<DataModel::Amplitude>::iterator it;
	for ( it = changedAmps.begin(); it != changedAmps.end(); ++it ) {
		SC_D.amplitudes.insert({ it->first->pickID(), { it->first, true } }	);
	}

	SEISCOMP_DEBUG("Amplitude cache size: %d", SC_D.amplitudes.size());

	MagnitudePtr mag = computeStationMagnitudes(SC_D.amplitudeView->currentMagnitudeType(), &amps);
	if ( !mag ) {
		return;
	}

	int typeIdx = findType(SC_D.tabMagnitudes, mag->type().c_str());

	// Type not yet available, add it
	if ( typeIdx == -1 ) {
		typeIdx = addMagnitude(mag.get());
	}
	// Update magnitude for type
	else {
		SC_D.tabMagnitudes->setTabData(typeIdx, QVariant::fromValue<TabData>(mag->publicID()));
	}

	if ( SC_D.tabMagnitudes->currentIndex() != typeIdx ) {
		showMagnitude(mag->publicID());
	}
	else {
		updateContent();
	}

	recalculateMagnitude();

	set<string> ampIDs;
	for ( size_t i = 0; i < SC_D.origin->stationMagnitudeCount(); ++i ) {
		if ( SC_D.origin->stationMagnitude(i)->amplitudeID().empty() ) {
			continue;
		}
		ampIDs.insert(SC_D.origin->stationMagnitude(i)->amplitudeID());
	}

	AmplitudeSet ampSet;

	for ( auto it = SC_D.amplitudes.begin(); it != SC_D.amplitudes.end(); ++it ) {
		// Skip external amplitudes
		if ( !it->second.second ) {
			continue;
		}

		// Skip unrequested amplitudes
		if ( ampIDs.find(it->second.first->publicID()) == ampIDs.end() ) {
			continue;
		}

		ampSet.insert({ it->second.first, true });
	}

	emit localAmplitudesAvailable(SC_D.origin.get(), &ampSet, &ampIDs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Magnitude *
MagnitudeView::computeStationMagnitudes(const string &magType,
                                        QList<Seiscomp::DataModel::AmplitudePtr> *amps,
                                        MagnitudeStats *errors) {
	Processing::MagnitudeProcessorPtr magProc = Processing::MagnitudeProcessorFactory::Create(magType.c_str());
	if ( !magProc ) {
	        return nullptr;
	}

	string ampType = magProc->amplitudeType();
	bool addMag = false;

	MagnitudePtr mag;
	QList<Seiscomp::DataModel::AmplitudePtr> tmpAmps;

	SEISCOMP_DEBUG("Origin.stationMags before: %d", (int)SC_D.origin->stationMagnitudeCount());

	// Remove all station magnitudes of origin with requested type
	for ( size_t i = 0; i < SC_D.origin->stationMagnitudeCount(); ) {
		if ( SC_D.origin->stationMagnitude(i)->type() == magType )
			SC_D.origin->removeStationMagnitude(i);
		else
			++i;
	}

	SEISCOMP_DEBUG("Origin.stationMags after: %d", (int)SC_D.origin->stationMagnitudeCount());

	for ( size_t i = 0; i < SC_D.origin->magnitudeCount(); ++i ) {
		if ( SC_D.origin->magnitude(i)->type() == magType ) {
			mag = SC_D.origin->magnitude(i);
			break;
		}
	}

	if ( !mag ) {
		mag = Magnitude::Create();
		mag->setType(magType);
		addMag = true;
	}
	else {
		// Remove all stationmagnitude references from magnitude
		while ( mag->stationMagnitudeContributionCount() > 0 )
			mag->removeStationMagnitudeContribution(0);

		SEISCOMP_DEBUG("Mag.stationMagRefs after: %d",
		               (int)mag->stationMagnitudeContributionCount());
	}

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::UTC());

	mag->setCreationInfo(ci);
	mag->setEvaluationStatus(Core::None);
	mag->setOriginID("");

	bool considerUnusedArrivals = false;
	try {
		SCApp->configGetBool(fmt::format("amplitudes.{}.considerUnusedArrivals", ampType));
	}
	catch ( ... ) {
		considerUnusedArrivals = true;
	}

	if ( !amps ) {
		// Typedef a pickmap that maps a streamcode to a pick
		using PickStreamMap = QMap<string, PickCPtr>;

		// This map is needed to find the earliest P pick of
		// a certain stream
		PickStreamMap pickStreamMap;

		// Fetch all picks to get amplitudes for
		for ( size_t i = 0; i < SC_D.origin->arrivalCount(); ++i ) {
			Arrival *ar = SC_D.origin->arrival(i);

			double weight = 1.;
			try { weight = ar->weight(); } catch (Seiscomp::Core::ValueException &) {}

			if ( Util::getShortPhaseName(ar->phase().code()) != 'P' ) {
				continue;
			}

			if ( (weight < 0.5) && !considerUnusedArrivals ) {
				continue;
			}

			Pick *pick = Pick::Find(ar->pickID());
			if ( !pick ) {
				continue;
			}

			DataModel::WaveformStreamID wfid = pick->waveformID();
			// Strip the component code because every AmplitudeProcessor
			// will use its own component to pick the amplitude on
			wfid.setChannelCode(wfid.channelCode().substr(0,2));

			string streamID = waveformIDToStdString(wfid);
			PickCPtr p = pickStreamMap[streamID];

			// When there is already a pick registered for this stream which has
			// been picked earlier, ignore the current pick
			if ( p && p->time().value() < pick->time().value() ) {
				continue;
			}

			pickStreamMap[streamID] = pick;
		}

		// Fetch all amplitudes for all picks
		for ( auto it = pickStreamMap.begin(); it != pickStreamMap.end(); ++it ) {
			PickCPtr pick = it.value();

			// The amplitude map contains always the newest manual amplitudes
			auto itp = SC_D.amplitudes.equal_range(pick->publicID());

			bool gotAmplitude = false;

			for ( auto it = itp.first; it != itp.second; ++it ) {
				AmplitudePtr amp = it->second.first;

				// Skip unrequestes amplitude types
				if ( amp->type() != ampType ) {
					continue;
				}

				// Use this amplitude
				tmpAmps.append(amp);
				gotAmplitude = true;

				break;
			}

			if ( gotAmplitude ) {
				continue;
			}

			if ( SC_D.reader ) {
				auto it = SC_D.reader->getAmplitudesForPick(pick->publicID());
				for ( ; *it; ++it ) {
					AmplitudePtr amp = Amplitude::Cast(*it);
					if ( !amp ) {
						continue;
					}

					// Save to cache
					SC_D.amplitudes.insert({ amp->pickID(), { amp, false } });

					// Skip unrequestes amplitude types
					if ( amp->type() != ampType ) {
						continue;
					}

					// Use this amplitude
					tmpAmps.append(amp);
					gotAmplitude = true;
				}
			}

			if ( gotAmplitude ) {
				continue;
			}

			auto ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
			if ( ep ) {
				for ( size_t i = 0; i < ep->amplitudeCount(); ++i ) {
					Amplitude *amp = ep->amplitude(i);
					if ( amp->pickID() != pick->publicID() ) {
						continue;
					}
					if ( amp->type() != ampType ) {
						continue;
					}

					// Use this amplitude
					tmpAmps.append(amp);
					gotAmplitude = true;
				}
			}
		}

		amps = &tmpAmps;
	}

	if ( amps ) {
		QList<Seiscomp::DataModel::AmplitudePtr>::iterator it;
		for ( it = amps->begin(); it != amps->end(); ++it ) {
			AmplitudePtr amp = *it;
			if ( amp->type() != ampType ) {
				continue;
			}

			// Create a magnitude processor for each amplitude. Basically this is
			// done for safety reasons since each station might have another configuration
			// set after setup is called.
			magProc = Processing::MagnitudeProcessorFactory::Create(magType.c_str());
			if ( !magProc ) {
				cerr << amp->waveformID().networkCode() << "."
				     << amp->waveformID().stationCode() << ": unable to create magnitude processor "
				     << magType << ": ignoring station" << endl;
				continue;
			}

			Util::KeyValuesPtr keys = getParams(amp->waveformID().networkCode(),
			                                    amp->waveformID().stationCode());
			if ( !magProc->setup(
				Processing::Settings(
					SCApp->configModuleName(),
					amp->waveformID().networkCode(), amp->waveformID().stationCode(),
					amp->waveformID().locationCode(), amp->waveformID().channelCode().substr(0,2),
					&SCCoreApp->configuration(), keys.get())) ) {
				cerr << amp->waveformID().networkCode() << "."
				     << amp->waveformID().stationCode() << ": setup magnitude processor failed"
				     << ": ignoring station" << endl;
				continue;
			}

			double dist, az;

			// Compute station distance
			SensorLocation *loc =  Client::Inventory::Instance()->getSensorLocation(
				amp->waveformID().networkCode(), amp->waveformID().stationCode(),
				amp->waveformID().locationCode(), amp->timeWindow().reference());

			if ( !loc ) {
				SEISCOMP_ERROR("Failed to get meta data for %s.%s.%s",
				               amp->waveformID().networkCode(),
				               amp->waveformID().stationCode(),
				               amp->waveformID().locationCode().empty() ? "--" : amp->waveformID().locationCode());
				continue;
			}

			try {
				double baz;
				Math::Geo::delazi(loc->latitude(), loc->longitude(),
				                  SC_D.origin->latitude(), SC_D.origin->longitude(),
				                  &dist, &az, &baz);
			}
			catch ( Core::GeneralException &e ) {
				SEISCOMP_ERROR("Magnitude distance computation: %s", e.what());
				continue;
			}

			double magValue;
			double period = 0, snr = 0;
			bool passedQC = true;

			try { period = amp->period().value(); } catch ( ... ) {}
			try { snr = amp->snr(); } catch ( ... ) {}

			Processing::MagnitudeProcessor::Status stat =
				magProc->computeMagnitude(
					amp->amplitude().value(), amp->unit(), period, snr,
					dist, SC_D.origin->depth(), SC_D.origin.get(), loc, amp.get(), magValue
				);

			if ( stat != Processing::MagnitudeProcessor::OK ) {
				SEISCOMP_DEBUG("Do not compute %s magnitude for station %s: %d (%s)",
				               mag->type(),
				               amp->waveformID().stationCode(),
				               (int)stat, stat.toString());

				if ( !magProc->treatAsValidMagnitude() ) {
					if ( errors ) {
						errors->append(MagnitudeStatus(magProc->type(), amp.get(), stat));
					}
					continue;
				}
				else {
					passedQC = false;
					if ( errors ) {
						errors->append(MagnitudeStatus(magProc->type(), amp.get(), stat, true));
					}
				}
			}

			StationMagnitudePtr staMag = StationMagnitude::Create();
			CreationInfo ci;
			ci.setAgencyID(SCApp->agencyID());
			ci.setAuthor(SCApp->author());
			ci.setCreationTime(Core::Time::UTC());
			staMag->setPassedQC(passedQC);
			staMag->setType(mag->type());
			staMag->setCreationInfo(ci);
			staMag->setWaveformID(amp->waveformID());
			staMag->setMagnitude(magValue);
			staMag->setAmplitudeID(amp->publicID());

			magProc->finalizeMagnitude(staMag.get());

			SC_D.origin->add(staMag.get());

			StationMagnitudeContributionPtr ref = new StationMagnitudeContribution;
			ref->setStationMagnitudeID(staMag->publicID());
			ref->setWeight(passedQC ? 1.0 : 0.0);

			mag->add(ref.get());
		}
	}

	if ( addMag ) {
		if ( mag->stationMagnitudeContributionCount() > 0 ) {
			SC_D.origin->add(mag.get());
		}
		else {
			return nullptr;
		}
	}

	return mag.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::computeMagnitude(DataModel::Magnitude *magnitude,
                                     const std::string &aggType) {
	vector<double> mags;
	vector<double> weights;
	vector<StationMagnitudeContribution*> stamags, stamagsZeroWeight;

	for ( size_t i = 0; i < magnitude->stationMagnitudeContributionCount(); ++i ) {
		StationMagnitude *sm = StationMagnitude::Find(magnitude->stationMagnitudeContribution(i)->stationMagnitudeID());

		try {
			if ( !sm->passedQC() ) {
				stamagsZeroWeight.push_back(magnitude->stationMagnitudeContribution(i));
				continue;
			}
		}
		catch ( ... ) {}

		stamags.push_back(magnitude->stationMagnitudeContribution(i));
		mags.push_back(sm->magnitude().value());
	}

	int staCount = 0;
	double netmag = INVALID_MAG;
	OPT(double) stdev = 0.0;

	if ( !stamags.empty() ) {
		bool fallback = true;

		if ( aggType == "mean" ) {
			Math::Statistics::computeMean(mags, netmag, *stdev);
			weights.resize(mags.size(), 1.);
			magnitude->setMethodID("mean");
			fallback = false;
		}
		else if ( aggType == "trimmedMean" ) {
			Math::Statistics::computeTrimmedMean(mags, 25.0, netmag, *stdev, &weights);
			magnitude->setMethodID("trimmed mean(25)");
			fallback = false;
		}
		else if ( aggType == "median" ) {
			netmag = Math::Statistics::median(mags);
			if ( mags.size() > 1 ) {
				*stdev = 0;
				for ( size_t i = 0; i < mags.size(); ++i )
					*stdev += (mags[i] - netmag) * (mags[i] - netmag);
				*stdev /= mags.size()-1;
				*stdev = sqrt(*stdev);
			}

			weights.resize(mags.size(), 1.);
			magnitude->setMethodID("median");
			fallback = false;
		}
		else if ( aggType == "medianTrimmedMean" ) {
			Math::Statistics::computeMedianTrimmedMean(mags, 0.5, netmag, *stdev, &weights);
			magnitude->setMethodID("median trimmed mean(0.5)");
			fallback = false;
		}

		if ( fallback ) {
			if ( mags.size() < 4 ) {
				Math::Statistics::computeMean(mags, netmag, *stdev);
				weights.resize(mags.size(), 1.);
				magnitude->setMethodID("mean");
			}
			else {
				Math::Statistics::computeTrimmedMean(mags, 25.0, netmag, *stdev, &weights);
				magnitude->setMethodID("trimmed mean(25)");
			}
		}

		for ( size_t i = 0; i < stamags.size(); ++i ) {
			StationMagnitudeContribution *ref = stamags[i];
			ref->setWeight(weights[i]);
			ref->setResidual(mags[i]-netmag);

			if ( weights[i] > 0.0 )
				++staCount;
		}
	}
	else {
		magnitude->setEvaluationStatus(EvaluationStatus(REJECTED));
	}

	magnitude->setMagnitude(DataModel::RealQuantity(netmag, stdev, Core::None, Core::None, Core::None));
	magnitude->setStationCount(staCount);

	for ( size_t i = 0; i < stamagsZeroWeight.size(); ++i ) {
		StationMagnitudeContribution *ref = stamagsZeroWeight[i];
		StationMagnitude *sm = StationMagnitude::Find(ref->stationMagnitudeID());

		ref->setWeight(0.0);
		if ( staCount )
			ref->setResidual(sm->magnitude().value()-netmag);
		else
			ref->setResidual(Core::None);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeView::editSelectionFilter() {
	MagnitudeRowFilter dlg(&selectionFilter());
	return dlg.exec() == QDialog::Accepted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! called, after selection made in diagram
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::magnitudesSelected() {
	// get selected Rect from diagram
	QRectF brect = SC_D.stamagnitudes->getSelectedValuesRect();

	// if nothing selected --> unCheck all everywhere
	if ( brect.isEmpty() && !brect.isNull() ) {
		for ( int i = 0; i < SC_D.modelStationMagnitudes->rowCount(); ++i )
			changeStationState(i, false);
		return;
	}

	// prevent loop, when changing table values [BEGIN]
	disconnect(SC_D.modelStationMagnitudes, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

	// set "used" checkBox in table and change color in map
	QVector<int> selectedIds = SC_D.stamagnitudes->getSelectedValues();
	int startIndex = 0;
	for ( int i = 0; i < selectedIds.count(); ++i ) {
		for ( int j = startIndex; j < selectedIds[i]; ++j ){
			SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(j, USED), Qt::Unchecked, Qt::CheckStateRole);
			if ( SC_D.map ) {
				SC_D.map->setMagnitudeState(j, false);
			}
		}
		SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(selectedIds[i], USED), Qt::Checked, Qt::CheckStateRole);
		if ( SC_D.map ) {
			SC_D.map->setMagnitudeState(selectedIds[i], true);
		}
		startIndex = selectedIds[i]+1;
	}

	for ( int j = startIndex; j < SC_D.modelStationMagnitudes->rowCount(); ++j ){
		SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(j, USED), Qt::Unchecked, Qt::CheckStateRole);
		if ( SC_D.map ) {
			SC_D.map->setMagnitudeState(j, false);
		}
	}

	// prevent loop, when changing table values [END]
	connect(SC_D.modelStationMagnitudes, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::hoverMagnitude(int id) {
	QWidget *w = (QWidget*)sender();
	if ( id == -1 )
		w->setToolTip("");
	else
		w->setToolTip(
			SC_D.modelStationMagnitudes->data(SC_D.modelStationMagnitudes->index(id, NETWORK), Qt::DisplayRole).toString() + "." +
			SC_D.modelStationMagnitudes->data(SC_D.modelStationMagnitudes->index(id, STATION), Qt::DisplayRole).toString());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectMagnitude(int id) {
	if ( id == -1 ) return;

	QModelIndex idx = SC_D.modelStationMagnitudesProxy->mapFromSource(SC_D.modelStationMagnitudes->index(id, 0));
	SC_D.ui.tableStationMagnitudes->setCurrentIndex(idx);
	SC_D.ui.tableStationMagnitudes->scrollTo(idx);

	if ( SC_D.amplitudeView ) {
		SC_D.amplitudeView->setCurrentStation(
			SC_D.modelStationMagnitudes->data(SC_D.modelStationMagnitudes->index(id, NETWORK), Qt::DisplayRole).toString().toStdString(),
			SC_D.modelStationMagnitudes->data(SC_D.modelStationMagnitudes->index(id, STATION), Qt::DisplayRole).toString().toStdString()
		);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectStation(const std::string &net, const std::string &code) {
	if ( SC_D.amplitudeView )
		SC_D.amplitudeView->setCurrentStation(net, code);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::adjustMagnitudeRect(QRectF& rect) {
	//rect.setLeft(max(0.0, floor(rect.left()*0.1) * 10));
	//rect.setRight((int)(ceil(rect.right()*0.1)) * 10);
	rect.setLeft(max(0.0, double(floor(rect.left()))));
	rect.setRight((int)(ceil(rect.right())));
	rect.setBottom(max(double(max(ceil(abs(rect.bottom())), ceil(abs(rect.top())))), 1.0));
	rect.setTop(-rect.bottom());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::tableStationMagnitudesContextMenuRequested(const QPoint &pos) {
	if ( !SC_D.ui.tableStationMagnitudes->selectionModel() ) return;
	if ( !SC_D.ui.tableStationMagnitudes->selectionModel()->hasSelection() ) return;
	QMenu menu;
	QAction *actionCopy = menu.addAction("Copy selected rows to clipboard");
	QAction *result = menu.exec(SC_D.ui.tableStationMagnitudes->mapToGlobal(pos));
	if ( result == actionCopy )
		SCApp->copyToClipboard(SC_D.ui.tableStationMagnitudes);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::tableStationMagnitudesHeaderContextMenuRequested(const QPoint &pos) {
	int count = SC_D.ui.tableStationMagnitudes->horizontalHeader()->count();
	QAbstractItemModel *model = SC_D.ui.tableStationMagnitudes->horizontalHeader()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!SC_D.ui.tableStationMagnitudes->horizontalHeader()->isSectionHidden(i));
	}

	QAction *result = menu.exec(SC_D.ui.tableStationMagnitudes->horizontalHeader()->mapToGlobal(pos));
	if ( result == nullptr ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	for ( int i = 0; i < count; ++i )
		colVisibility[i] = actions[i]->isChecked();

	SC_D.ui.tableStationMagnitudes->horizontalHeader()->setSectionHidden(section, !colVisibility[section]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::changeMagnitudeState(int id, bool state) {
	SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(id, USED), state?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! DISABLED: called, after selection made in map
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::changeStationState(int id, bool state) {
	// set state (un/checked) in table
	SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(id, USED), state?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! called, after selection made in table
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::dataChanged(const QModelIndex& topLeft, const QModelIndex&){
	if ( topLeft.column() != 0 ) return;

	bool state = SC_D.modelStationMagnitudes->data(topLeft, Qt::CheckStateRole).toInt() == Qt::Checked;

	// set state (color) in diagram
	SC_D.stamagnitudes->setValueSelected(topLeft.row(), state);

	// set state (color) in map
	if ( SC_D.map ) {
		SC_D.map->setMagnitudeState(topLeft.row(), state);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectPreferredMagnitude(int idx) {
	Magnitude *mag = nullptr;

	for ( int i = 0; i < SC_D.tabMagnitudes->count(); ++i ) {
		TabData d = SC_D.tabMagnitudes->tabData(i).value<TabData>();
		d.selected = d.valid && (i == idx);
		SC_D.tabMagnitudes->setTabData(i, QVariant::fromValue(d));

		if ( d.selected )
			mag = Magnitude::Find(d.publicID);

		QCheckBox *cb = static_cast<QCheckBox*>(SC_D.tabMagnitudes->tabButton(i, QTabBar::LeftSide));
		if ( cb ) {
			cb->blockSignals(true);
			cb->setCheckState(d.selected ? Qt::Checked : Qt::Unchecked);
			cb->blockSignals(false);
		}
	}

	emit magnitudeSelected(SC_D.origin->publicID().c_str(), mag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::tabStateChanged(int state) {
	if ( state == Qt::Checked ) {
		for ( int i = 0; i < SC_D.tabMagnitudes->count(); ++i ) {
			QCheckBox *cb = static_cast<QCheckBox*>(SC_D.tabMagnitudes->tabButton(i, QTabBar::LeftSide));
			if ( cb == sender() ) {
				selectPreferredMagnitude(i);
				return;
			}
		}
	}

	selectPreferredMagnitude(-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setDrawGridLines(bool f) {
	SC_D.stamagnitudes->setDrawGridLines(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setComputeMagnitudesSilently(bool f) {
	SC_D.computeMagnitudesSilently = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setMagnitudeTypeSelectionEnabled(bool f) {
	SC_D.enableMagnitudeTypeSelection = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::drawStations(bool enable) {
	SC_D.map->setDrawStations(enable);
	SC_D.map->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::drawStationAnnotations(bool enable) {
	SC_D.map->setDrawStationAnnotations(enable);
	SC_D.map->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setAmplitudeConfig(const AmplitudeView::Config &config) {
	SC_D.amplitudeConfig = config;

	if ( SC_D.amplitudeView )
		SC_D.amplitudeView->setConfig(SC_D.amplitudeConfig);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AmplitudeView::Config &MagnitudeView::amplitudeConfig() const {
	return SC_D.amplitudeConfig;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! set origin & event (and if applicable, the preferred magnitude)
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setOrigin(Origin* o, Event *e) {
	if ( SC_D.origin == o ) {
		return;
	}

	bool eventChanged = (SC_D.event != e) || !e;

	SC_D.origin = o;
	SC_D.event = e;
	SC_D.netMag = nullptr;
	SC_D.amplitudes.clear();

	if ( SC_D.amplitudeView ) {
		SC_D.amplitudeView->close();
	}

	if ( !SC_D.origin ) {
		SC_D.currentMagnitudeTypes = SC_D.magnitudeTypes;
		resetContent();
		return;
	}

	// If a new event is set reset the selected magnitudes to default
	if ( eventChanged ) {
		SC_D.currentMagnitudeTypes = SC_D.magnitudeTypes;
	}

	reload();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeView::showMagnitude(const string &id) {
	for ( int i = 0; i < SC_D.tabMagnitudes->count(); ++i ) {
		if ( SC_D.tabMagnitudes->tabData(i).value<TabData>().publicID == id ) {
			SC_D.tabMagnitudes->setCurrentIndex(i);
			return true;
		}
	}

	/*
	for ( int i = 0; i < SC_D.ui.comboMagType->count(); ++i ) {
		if ( SC_D.ui.comboMagType->itemData(i).toString() == id.c_str() ) {
			SC_D.ui.comboMagType->setCurrentIndex(i);
			return true;
		}
	}
	*/

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::reload() {
	// otherwise display the first magnitude
	SC_D.netMag = nullptr;

	for ( size_t i = 0; i < SC_D.origin->magnitudeCount(); ++i ) {
		MagnitudePtr mag = SC_D.origin->magnitude(i);
		SC_D.netMag = mag;
		break;
	}

	setContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::addObject(const QString &parentID, Seiscomp::DataModel::Object* o) {
	// Check whether a new stationmagnitude has been inserted into the current networkmagnitude
	StationMagnitudeContribution* magRef = StationMagnitudeContribution::Cast(o);
	if ( magRef ) {
		auto pid = parentID.toStdString();
		Magnitude *netMag = Magnitude::Find(pid);
		if ( netMag ) {
			// Update the corresponding tab header
			updateTab(SC_D.tabMagnitudes, netMag);
		}

		// It does not influence the currently displayed magnitude -> dont care
		if ( !SC_D.netMag || pid != SC_D.netMag->publicID() )
			return;

		StationMagnitude* staMag = StationMagnitude::Find(magRef->stationMagnitudeID());
		if ( staMag == nullptr ) {
			SEISCOMP_DEBUG("Received stationMagnitudeContribution for magnitude '%s' that has not been found",
			               magRef->stationMagnitudeID());
			return;
		}

		//cout << "Add StationStationMagnitudeContribution for Magnitude '" << SC_D.netMag->publicID() << "'" << endl;
		SEISCOMP_DEBUG("NetMag '%s' has %lu StaMags", SC_D.netMag->publicID(), (unsigned long)SC_D.netMag->stationMagnitudeContributionCount());
		addStationMagnitude(staMag, SC_D.netMag->stationMagnitudeContributionCount()-1);

		return;
	}

	// Check whether a new networkmagnitude has been inserted into the current origin
	Magnitude* netMag = Magnitude::Cast(o);
	if ( netMag ) {
		// Not for now
		if ( !SC_D.origin || SC_D.origin->publicID() != parentID.toStdString() ) {
			return;
		}

		if ( SC_D.tabMagnitudes->count() == 0 ) {
			if ( SC_D.map ) {
				SC_D.map->update();
			}
		}

		addMagnitude(netMag);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::removeObject(const QString &, Seiscomp::DataModel::Object *) {
	// this shouldn't happen
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateObject(const QString &parentID, Seiscomp::DataModel::Object* o) {
	// Check whether a networkmagnitude has been updated
	Magnitude *netMag = Magnitude::Cast(o);
	if ( netMag ) {
		// Resolve the currently cached version which also holds the
		// station magnitude contributions
		netMag = Magnitude::Find(netMag->publicID());

		if ( !netMag ) {
			return;
		}

		if ( !SC_D.origin || SC_D.origin->publicID() != parentID.toStdString() ) {
			return;
		}

		updateTab(SC_D.tabMagnitudes, netMag);

		if ( !SC_D.netMag || SC_D.netMag->publicID() != netMag->publicID() ) {
			return;
		}

		SEISCOMP_INFO("Updating magnitude %s", netMag->publicID());
		updateMagnitudeLabels();
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setReadOnly(bool e) {
	SC_D.ui.groupReview->setVisible(!e);

	SC_D.tabMagnitudes->setTabsClosable(!e);

	if ( SC_D.amplitudeView && e )
		SC_D.amplitudeView->close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::resetPreferredMagnitudeSelection() {
	selectPreferredMagnitude(-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::disableRework() {
	resetPreferredMagnitudeSelection();
	setReadOnly(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MagnitudeView::addMagnitude(Seiscomp::DataModel::Magnitude* netMag) {
	//for ( int i = 0; i < SC_D.ui.comboMagType->count(); ++i ) {
	for ( int i = 0; i < SC_D.tabMagnitudes->count(); ++i ) {
		if ( SC_D.tabMagnitudes->tabData(i).value<TabData>().publicID == netMag->publicID() ) {
		//if ( SC_D.ui.comboMagType->itemData(i).toString() == netMag->publicID().c_str() ) {
			SEISCOMP_DEBUG("Magnitude '%s' has been added already", netMag->publicID());
			return i;
		}
	}

	//SC_D.ui.comboMagType->addItem(QString("%1").arg(netMag->type().c_str()), data);
	int tabIndex = SC_D.tabMagnitudes->addTab(
		QString("%1 %2 (%3/%4)")
		.arg(netMag->type().c_str())
		.arg(netMag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude)
		.arg(usedStationCount(netMag))
		.arg(totalStationCount(netMag))
	);
	TabData data(netMag->publicID());

	try {
		if ( netMag->evaluationStatus() == REJECTED ) {
			SC_D.tabMagnitudes->setTabText(
				tabIndex, QString("%1 -.-- (%2/%3)")
				.arg(netMag->type().c_str())
				.arg(usedStationCount(netMag))
				.arg(totalStationCount(netMag))
			);
			SC_D.tabMagnitudes->setTabTextColor(tabIndex, palette().color(QPalette::Disabled, QPalette::WindowText));
			SC_D.tabMagnitudes->setTabIcon(tabIndex, icon("tab_disabled"));
			data.valid = false;
		}
	}
	catch ( ... ) {}

	SC_D.tabMagnitudes->setTabData(tabIndex, QVariant::fromValue<TabData>(data));

	if ( data.valid ) {
		QCheckBox *btn = new QCheckBox;
		btn->setToolTip(tr("Select this magnitude type as preferred magnitude "
		                   "type when the event will be committed either "
		                   "with additional options or with custom commit profiles."));
		btn->setProperty("tabIndex", tabIndex);
		SC_D.tabMagnitudes->setTabButton(tabIndex, QTabBar::LeftSide, btn);
		connect(btn, SIGNAL(stateChanged(int)), this, SLOT(tabStateChanged(int)));
	}

	if ( tabIndex == SC_D.tabMagnitudes->currentIndex() )
		updateContent();

	if ( !SC_D.ui.frameMagnitudeTypes->isVisible() )
		SC_D.ui.frameMagnitudeTypes->setVisible(true);

	return tabIndex;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! set magnitude combo box and go on
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setContent() {
	// fill MagComboBox with all magnitudes from origin
	//disconnect(SC_D.ui.comboMagType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateContent()));
	disconnect(SC_D.tabMagnitudes, SIGNAL(currentChanged(int)), this, SLOT(updateContent()));
	//SC_D.ui.comboMagType->clear();
	while ( SC_D.tabMagnitudes->count() > 0 ) SC_D.tabMagnitudes->removeTab(0);
	emit magnitudeSelected(SC_D.origin ? QString(SC_D.origin->publicID().c_str()) : QString(), 0);

	SC_D.ui.frameMagnitudeTypes->setVisible(false);

	if ( SC_D.origin ) {
		for (size_t i = 0; i < SC_D.origin->magnitudeCount(); i++)
			addMagnitude(SC_D.origin->magnitude(i));

		if ( SC_D.netMag ) {
			// set combo box item according to desired netMag
			SC_D.tabMagnitudes->setCurrentIndex(findData(SC_D.tabMagnitudes, SC_D.netMag->publicID()));
		}
	}

	//connect(SC_D.ui.comboMagType,SIGNAL(currentIndexChanged(int)), this, SLOT(updateContent()));
	connect(SC_D.tabMagnitudes, SIGNAL(currentChanged(int)), this, SLOT(updateContent()));

	// setup map, show all stations in red; associated with netMag in green
	if ( SC_D.map ) {
		SC_D.map->setOrigin(SC_D.origin.get());

		int iLatitude = (int)SC_D.origin->latitude();
		int iLongitude = (int)SC_D.origin->longitude();

		if (SC_D.origin->arrivalCount() > 0){ // if true --> calculate map boundary so, that ALL stations are visible

			// min/max stations coordinates incl. Origin
			double latMin, latMax, lonMin, lonMax;
			calcMinMax(SC_D.origin.get(), latMin, latMax, lonMin, lonMax);

			// lower left corner of displaying rectangle, currentOrigin is inside
			double x = lonMin;
			double y = latMin;
			// width and height of displaying rectangle
			double dx = lonMax-x;
			double dy = latMax-y;

			if (dy == 0.0) dx = dy = 1.0;

			double displayRectAspectRatio;
			try{displayRectAspectRatio = dx/dy;}
			catch(...){displayRectAspectRatio = 1.0;}

			double frameMapAspectRatio;
			try {frameMapAspectRatio = (double)SC_D.ui.frameMap->width()/(double)SC_D.ui.frameMap->height();}
			catch(...){frameMapAspectRatio = 1.0;}

			double ndx, ndy;
			double frame = 0.05; // *100=% of rect width/height

			if ( (displayRectAspectRatio / frameMapAspectRatio) > 1.0)
				ndy = dx / frameMapAspectRatio;
			else
				ndy = dy;

			if ( ndy < 180.0 ){
				SC_D.map->canvas().displayRect(QRectF(x-(frame*dx), (y-((ndy-dy)/2))-(frame*ndy), dx+(2*frame*dx), ndy+(2*frame*ndy)));
			}
			else{
				ndx = 180.0 * frameMapAspectRatio;
				SC_D.map->canvas().displayRect(QRectF(iLongitude-ndx/2, -90.0, ndx, 180));
			}
		}
		else{ // if false --> center map around origin
			SC_D.map->canvas().displayRect(QRectF(iLongitude-30, iLatitude-30, 60, 60));
		}

		SC_D.map->update();
	}
	else
		SEISCOMP_ERROR("no Map");

	// Set default aggregation type
	if ( SC_D.defaultMagnitudeAggregation ) {
		if ( *SC_D.defaultMagnitudeAggregation == "median" ) {
			SC_D.ui.btnMedian->setChecked(true);
		}
		else if ( *SC_D.defaultMagnitudeAggregation == "mean" ) {
			SC_D.ui.btnMean->setChecked(true);
		}
		else if ( *SC_D.defaultMagnitudeAggregation == "trimmedMean" ) {
			SC_D.ui.btnTrimmedMean->setChecked(true);
		}
		else if ( *SC_D.defaultMagnitudeAggregation == "medianTrimmedMean" ) {
			SC_D.ui.btnTrimmedMedian->setChecked(true);
		}
	}
	else {
		SC_D.ui.btnDefault->setChecked(true);
	}

	updateContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! reset content to default, disconnect signals
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::resetContent() {
	SC_D.ui.frameMagnitudeTypes->setVisible(false);

	// remove all entries in comboBox
	//disconnect(SC_D.ui.comboMagType,SIGNAL(currentIndexChanged(int)), this, SLOT(updateContent()));
	disconnect(SC_D.tabMagnitudes, SIGNAL(currentChanged(int)), this, SLOT(updateContent()));
	//SC_D.ui.comboMagType->clear();
	while ( SC_D.tabMagnitudes->count() > 0 ) SC_D.tabMagnitudes->removeTab(0);
	emit magnitudeSelected(SC_D.origin ? QString(SC_D.origin->publicID().c_str()) : QString(), 0);

	SC_D.stamagnitudes->clear();
	SC_D.stamagnitudes->update();

 	static_cast<StationMagnitudeModel*>(SC_D.modelStationMagnitudes)->setOrigin(nullptr, nullptr);

	QAbstractItemModel* m = SC_D.ui.tableStationMagnitudes->model();
	if ( m ) delete m;

	SC_D.modelStationMagnitudesProxy = new StaMagsSortFilterProxyModel(this);
	SC_D.modelStationMagnitudesProxy->setSourceModel(SC_D.modelStationMagnitudes);
	SC_D.ui.tableStationMagnitudes->setModel(SC_D.modelStationMagnitudesProxy);

	if ( SC_D.map ) {
		SC_D.map->setOrigin(nullptr);
		SC_D.map->canvas().displayRect(QRectF(-180.0,-90.0, 360.0, 180.0));
	}

	SC_D.ui.labelRegion->setText("Region");

	updateMagnitudeLabels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! update map, diagram, table and labels
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateContent() {
	SC_D.ui.labelRegion->setText("");

	if ( !SC_D.origin ) {
		return;
	}

	SC_D.ui.labelRegion->setText(Regions::getRegionName(SC_D.origin->latitude(), SC_D.origin->longitude()).c_str());

	//  use selection from comboBox for netmagType
	//SC_D.netMag = SC_D.origin->findMagnitude((SC_D.ui.comboMagType->itemData(SC_D.ui.comboMagType->currentIndex()).value<QString>()).toAscii().data());
	SC_D.netMag = SC_D.origin->findMagnitude(SC_D.tabMagnitudes->tabData(SC_D.tabMagnitudes->currentIndex()).value<TabData>().publicID);

	if ( SC_D.netMag ) {
		for ( size_t i = 0; i < SC_D.netMag->stationMagnitudeContributionCount(); ++i ) {
			if ( !StationMagnitude::Find(SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID()) ) {
				StationMagnitudePtr stamag = (StationMagnitude*)SC_D.reader->getObject(StationMagnitude::TypeInfo(), SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
				if ( stamag ) {
					SC_D.origin->add(stamag.get());
				}
			}
		}
	}

	if ( SC_D.map ) {
		SC_D.map->setMagnitude(SC_D.netMag.get());
		SC_D.map->update();
	}

	// clear diagram
	SC_D.stamagnitudes->clear();

	// setup model/view table
	static_cast<StationMagnitudeModel*>(SC_D.modelStationMagnitudes)->setOrigin(SC_D.origin.get(), SC_D.netMag.get());

	QAbstractItemModel *m = SC_D.ui.tableStationMagnitudes->model();
	if ( m ) {
		delete m;
	}

	SC_D.modelStationMagnitudesProxy = new StaMagsSortFilterProxyModel(this);
	SC_D.modelStationMagnitudesProxy->setSourceModel(SC_D.modelStationMagnitudes);
	SC_D.ui.tableStationMagnitudes->setModel(SC_D.modelStationMagnitudesProxy);

	SC_D.minStationMagnitude = 999.99;
	SC_D.maxStationMagnitude = -999.99;

	if ( SC_D.netMag ) {
		StationMagnitude* staMagnitude;
		for ( size_t i = 0; i < SC_D.netMag->stationMagnitudeContributionCount(); ++i ) {
			staMagnitude = StationMagnitude::Find(SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
			// set distance in distanceList
			if ( staMagnitude ) {
				addStationMagnitude(staMagnitude, i);
			}
		}
	}

	// set labels ...
	updateMagnitudeLabels();

	auto regExp = QRegularExpression("^comboBox/magnitude/comment/.*$");
	auto magnitudeComments = findChildren<QComboBox*>(regExp);

	if ( !SC_D.netMag ) {
		SC_D.ui.cbEvalStatus->setCurrentIndex(0);
		for ( auto comboBox : magnitudeComments ) {
			comboBox->setCurrentIndex(0);
		}

		SC_D.ui.groupReview->setEnabled(false);

		// set dist column in table & add Net/Sta-Mag to diagram: dist = addStationMagnitude()
		updateMinMaxMagnitude();
		update();
		return;
	}
	else {
		SC_D.ui.cbEvalStatus->blockSignals(true);
		try {
			int idx = SC_D.ui.cbEvalStatus->findText(SC_D.netMag->evaluationStatus().toString());
			if ( idx != -1 ) {
				SC_D.ui.cbEvalStatus->setCurrentIndex(idx);
			}
			else {
				SC_D.ui.cbEvalStatus->setCurrentIndex(0);
			}
		}
		catch ( ... ) {
			SC_D.ui.cbEvalStatus->setCurrentIndex(0);
		}
		SC_D.ui.cbEvalStatus->blockSignals(false);

		for ( QComboBox *comboBox : magnitudeComments ) {
			comboBox->blockSignals(true);
			auto id = comboBox->property("id").toString().toStdString();
			auto comment = SC_D.netMag->comment(id);
			if ( comment ) {
				auto idx = comboBox->findText(comment->text().c_str());
				if ( idx >= 0 ) {
					comboBox->setCurrentIndex(idx);
				}
				else if ( comboBox->isEditable() ) {
					comboBox->setEditText(comment->text().c_str());
				}
			}
			else {
				if ( comboBox->isEditable() ) {
					comboBox->setEditText(QString());
				}
				else {
					comboBox->setCurrentIndex(0);
				}
			}
			comboBox->blockSignals(false);
		}
	}

	SEISCOMP_DEBUG("selected magnitude: %s with %lu magRefs ", SC_D.netMag->publicID(), (unsigned long)SC_D.netMag->stationMagnitudeContributionCount() );
	SEISCOMP_DEBUG("selected Origin          : %s with %lu arrivals", SC_D.origin->publicID(), (unsigned long)SC_D.origin->arrivalCount() );

	for ( int i = 0; i < StaMagsListColumns::Quantity; ++i )
		SC_D.ui.tableStationMagnitudes->setColumnHidden(i, !colVisibility[i]);

	// update column width in table view
	SC_D.ui.tableStationMagnitudes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	//SC_D.ui.tableStationMagnitudes->resizeColumnsToContents();
	SC_D.ui.tableStationMagnitudes->resizeRowsToContents();
	SC_D.ui.tableStationMagnitudes->sortByColumn(
	            SC_D.ui.tableStationMagnitudes->horizontalHeader()->sortIndicatorSection(),
	            SC_D.ui.tableStationMagnitudes->horizontalHeader()->sortIndicatorOrder());

	if ( SC_D.netMag->stationMagnitudeContributionCount() == 0 ) {
		SC_D.ui.groupReview->setEnabled(false);
		updateMinMaxMagnitude();

		try {
			if ( SC_D.netMag->evaluationStatus() == REJECTED ) {
				SC_D.ui.groupReview->setEnabled(true);
			}
		}
		catch ( ... ) {}
	}
	else {
		SC_D.ui.groupReview->setEnabled(true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateMinMaxMagnitude() {
	SC_D.ui.labelMinMag->setText("-");
	SC_D.ui.labelMaxMag->setText("-");
	if ( SC_D.minStationMagnitude <= SC_D.maxStationMagnitude
	  && SC_D.minStationMagnitude > -10 && SC_D.maxStationMagnitude < 15 ) {
		char buf[10];
		snprintf(buf, 10, "%.*f", SCScheme.precision.magnitude, SC_D.minStationMagnitude);
		SC_D.ui.labelMinMag->setText(buf);
		snprintf(buf, 10, "%.*f", SCScheme.precision.magnitude, SC_D.maxStationMagnitude);
		SC_D.ui.labelMaxMag->setText(buf);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateMagnitudeLabels() {
	if ( SC_D.netMag ) {
		char buf[10] = "-";
		double netmagval = SC_D.netMag->magnitude().value();
		if ( netmagval < 12 ) {
			snprintf(buf, 10, "%.*f", SCScheme.precision.magnitude, netmagval);
		}
		else if ( netmagval < 1000000000 ) {
			snprintf(buf, 10, "%d", (int)netmagval);
		}

		SC_D.ui.labelMethod->setText(SC_D.netMag->methodID().c_str());
		SC_D.ui.labelMethod->setToolTip(SC_D.netMag->methodID().c_str());
		try {
			SC_D.ui.labelAgencyID->setText(SC_D.netMag->creationInfo().agencyID().c_str());
			SC_D.ui.labelAgencyID->setToolTip(SC_D.netMag->creationInfo().agencyID().c_str());
		}
		catch(Core::ValueException &) {
			SC_D.ui.labelAgencyID->setText("");
			SC_D.ui.labelAgencyID->setToolTip("");
		}

		try {
			SC_D.ui.labelAuthor->setText(SC_D.netMag->creationInfo().author().c_str());
			SC_D.ui.labelAuthor->setToolTip(SC_D.netMag->creationInfo().author().c_str());
		}
		catch(Core::ValueException &) {
			SC_D.ui.labelAuthor->setText("");
			SC_D.ui.labelAuthor->setToolTip("");
		}

		try {
			SC_D.ui.labelEvaluation->setText(SC_D.netMag->evaluationStatus().toString());
		}
		catch(Core::ValueException &) {
			SC_D.ui.labelEvaluation->setText("-");
		}

		SC_D.ui.labelMagnitude->setText(buf);
		try {
			SC_D.ui.labelNumStaMags->setText(QString("%1 (%2)").arg(SC_D.netMag->stationCount()).arg(SC_D.netMag->stationMagnitudeContributionCount()));
		}
		catch(Core::ValueException &) {
			SC_D.ui.labelNumStaMags->setText(QString("-"));
		}

		strcpy(buf, "-");
		try {
			double rms = quantityUncertainty(SC_D.netMag->magnitude());
			if ( rms < 10 )
				snprintf(buf, 10, "%.*f", SCScheme.precision.magnitude, rms);
		}
		catch ( ... ) {}
		SC_D.ui.labelRMS->setText(buf);

		if ( (size_t)SC_D.stamagnitudes->count() == SC_D.netMag->stationMagnitudeContributionCount() ) {
			SC_D.stamagnitudes->setUpdatesEnabled(false);
			for ( size_t i = 0; i < SC_D.netMag->stationMagnitudeContributionCount(); ++i ) {
				StationMagnitudePtr staMagnitude = StationMagnitude::Find(SC_D.netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
				// set distance in distanceList
				if ( staMagnitude ) {
					QPointF p = SC_D.stamagnitudes->value(i);
					double residual = staMagnitude->magnitude().value() - netmagval;
					p.setY(residual);
					SC_D.stamagnitudes->setValue(i, p);
					try {
						SC_D.stamagnitudes->setValueSelected(i, SC_D.netMag->stationMagnitudeContribution(i)->weight() > 0.0);
					}
					catch ( ... ) {
						SC_D.stamagnitudes->setValueSelected(i, true);
					}

					changeMagnitudeState(i, SC_D.stamagnitudes->isValueSelected(i));
				}
			}

			SC_D.stamagnitudes->updateBoundingRect();
			QRectF rect(SC_D.stamagnitudes->boundingRect());
			QRectF newRect(rect);
			adjustMagnitudeRect(newRect);
			newRect.setLeft(max(0.0, double(rect.left())));

			SC_D.stamagnitudes->setDisplayRect(newRect);
			SC_D.stamagnitudes->setUpdatesEnabled(true);

			SC_D.map->setMagnitude(SC_D.netMag.get());

			for ( int i = 0; i < SC_D.modelStationMagnitudes->rowCount(); ++i ) {
				bool state = SC_D.modelStationMagnitudes->data(SC_D.modelStationMagnitudes->index(i, USED), Qt::CheckStateRole).toInt() == Qt::Checked;
				SC_D.map->setMagnitudeState(i, state);
			}

			SC_D.map->update();
		}
	}
	else {
		SC_D.ui.labelNumStaMags->setText("0");
		SC_D.ui.labelMagnitude->setText("-");
		SC_D.ui.labelMinMag->setText("-");
		SC_D.ui.labelMaxMag->setText("-");
		SC_D.ui.labelRMS->setText("-");

		SC_D.ui.labelAgencyID->setText("");
		SC_D.ui.labelAgencyID->setToolTip("");

		SC_D.ui.labelAuthor->setText("");
		SC_D.ui.labelAuthor->setToolTip("");

		SC_D.ui.labelMethod->setText("");
		SC_D.ui.labelMethod->setToolTip("");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::addStationMagnitude(StationMagnitude *stationMagnitude, int index) {
	//SEISCOMP_DEBUG("Adding stationMagnitude at index #%d", index);

	// StationMagnitude has been inserted already -> skip it
	if ( index < SC_D.stamagnitudes->count() ) {
		SEISCOMP_WARNING("Index out of bounds (%d >= %d), has been added already", index, SC_D.stamagnitudes->count());
		return;
	}

	//SEISCOMP_DEBUG("Current model rowCount = %d", SC_D.modelStationMagnitudes->rowCount());

	if ( SC_D.map ) {
		SC_D.map->addStationMagnitude(stationMagnitude, index);
	}

	while ( SC_D.modelStationMagnitudes->rowCount() <= index ) {
		SC_D.modelStationMagnitudes->insertRow(SC_D.modelStationMagnitudes->rowCount());
		updateMagnitudeLabels();
	}

	double weight = 1.0;
	try {
		weight = SC_D.netMag->stationMagnitudeContribution(index)->weight();
	}
	catch ( ... ) {}

	auto sr = addStationMagnitude(SC_D.netMag.get(), stationMagnitude, weight);
	SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(index, DISTANCE),
	                                sr.distance, Qt::DisplayRole);
	SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(index, AZIMUTH),
	                                sr.azimuth, Qt::DisplayRole);
	SC_D.modelStationMagnitudes->setData(SC_D.modelStationMagnitudes->index(index, USED),
	                                Qt::Checked, Qt::CheckStateRole);

	SC_D.ui.tableStationMagnitudes->resizeRowToContents(index);

	double value = stationMagnitude->magnitude().value();

	if ( value < SC_D.minStationMagnitude ) {
		SC_D.minStationMagnitude = value;
	}

	if ( value > SC_D.maxStationMagnitude ) {
		SC_D.maxStationMagnitude = value;
	}

	updateMinMaxMagnitude();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


//! add StationMag to distance-MagResidual diagram
//! returns the distance: origin-station
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::SourceReceiver
MagnitudeView::addStationMagnitude(DataModel::Magnitude *magnitude,
                                   DataModel::StationMagnitude *stationMagnitude,
                                   double weight) {
	double distance = -1.0, az = 999.0;

	try {
		const WaveformStreamID &wfsID = stationMagnitude->waveformID();

		try {
			StationLocation loc = Client::Inventory::Instance()->stationLocation(
				wfsID.networkCode(), wfsID.stationCode(), SC_D.origin->time().value());

			Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
			                  loc.latitude, loc.longitude,
			                  &distance, &az);
		}
		catch ( ValueException& ) {
			SEISCOMP_ERROR("MagnitudeView::addStationMagnitude: Station %s.%s "
			               "not found. Not added.", wfsID.networkCode(),
			               wfsID.stationCode());
		}
	}
	catch ( ValueException& ) {
		SEISCOMP_ERROR("MagnitudeView::addStationMagnitude: WaveformID in "
		               "magnitude '%s' not set",
		               stationMagnitude->publicID());
	}

	double residual = stationMagnitude->magnitude().value() - magnitude->magnitude().value();

	QColor c = SCScheme.colors.arrivals.automatic;
	AmplitudePtr amp = SC_D.objCache.get<Amplitude>(stationMagnitude->amplitudeID());
	if ( amp ) {
		try {
			if ( amp->evaluationMode() == DataModel::MANUAL )
				c = SCScheme.colors.arrivals.manual;
		}
		catch ( ... ) {}
	}

	if ( SCScheme.unit.distanceInKM ) {
		SC_D.stamagnitudes->addValue(QPointF(Math::Geo::deg2km(distance), residual), c);
	}
	else {
		SC_D.stamagnitudes->addValue(QPointF(distance, residual), c);
	}

	SC_D.stamagnitudes->setValueSelected(SC_D.stamagnitudes->count() - 1, weight > 0.0);
	changeMagnitudeState(
		SC_D.stamagnitudes->count() - 1,
		SC_D.stamagnitudes->isValueSelected(SC_D.stamagnitudes->count() - 1)
	);

	QRectF rect(SC_D.stamagnitudes->boundingRect());
	adjustMagnitudeRect(rect);
	rect.setLeft(max(0.0, double(rect.left())));

	if ( rect != SC_D.stamagnitudes->displayRect() ) {
		SC_D.stamagnitudes->setDisplayRect(rect);
		SC_D.stamagnitudes->update();
	}

	return { distance, az };
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! return pick for arrival
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Pick *MagnitudeView::getPick(DataModel::Arrival* arrival){
	if ( arrival ){
		DataModel::Pick* pick = DataModel::Pick::Cast(DataModel::PublicObject::Find(arrival->pickID()));

		if ( !pick && SC_D.reader ) {
			pick = DataModel::Pick::Cast(SC_D.reader->getObject(DataModel::Pick::TypeInfo(), arrival->pickID() ));
		}

		return pick;
	}
	else {
		return nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! calculate the max/min coordinates for all stations incl. origin
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::calcMinMax(Seiscomp::DataModel::Origin* o, double& latMin, double& latMax,
                               double& lonMin, double& lonMax ){

	latMin = o->latitude();
	latMax = o->latitude();
	lonMin = o->longitude();
	lonMax = o->longitude();

	double lat, lon;

	for (size_t i = 0; i < o->arrivalCount(); i++){

		DataModel::PickPtr p = getPick(o->arrival(i));

		if (p) {
			try {
				StationLocation loc = Client::Inventory::Instance()->stationLocation(
				p->waveformID().networkCode(),
				p->waveformID().stationCode(),
				p->time() );
				try{lat = loc.latitude;}catch(...){lat = -9999.9;}
				try{lon = loc.longitude;}catch(...){lon = -9999.9;}

				if (lat != -9999.9 && lon != -9999.9){
					latMax = lat>latMax?lat:latMax;
					latMin = lat<latMin?lat:latMin;
					lonMax = lon>lonMax?lon:lonMax;
					lonMin = lon<lonMin?lon:lonMin;
				}
			}
			catch(Core::ValueException& e){
				SEISCOMP_ERROR("While fetching the station location an error occured: %s", e.what());
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::evaluationStatusChanged(int index) {
	if ( index < 0 || !SC_D.netMag ) return;

	if ( !index ) {
		SC_D.netMag->setEvaluationStatus(Core::None);
	}
	else {
		EvaluationStatus stat;
		if ( !stat.fromString(SC_D.ui.cbEvalStatus->itemText(index).toStdString()) ) {
			return;
		}
		SC_D.netMag->setEvaluationStatus(stat);
	}

	emit magnitudeUpdated(SC_D.origin->publicID().c_str(), SC_D.netMag.get());

	// Update linked Mw estimate
	Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(SC_D.netMag->type().c_str());
	if ( proc ) {
		string type = proc->typeMw();
		int idx = findType(SC_D.tabMagnitudes, type.c_str());
		if ( idx != -1 ) {
			MagnitudePtr magMw =
				//Magnitude::Find(SC_D.ui.comboMagType->itemData(idx).value<QString>().toStdString());
				Magnitude::Find(SC_D.tabMagnitudes->tabData(idx).value<TabData>().publicID);

			if ( magMw && magMw != SC_D.netMag ) {
				try {
					magMw->setEvaluationStatus(SC_D.netMag->evaluationStatus());
				}
				catch ( ... ) {
					magMw->setEvaluationStatus(Core::None);
				}
				emit magnitudeUpdated(SC_D.origin->publicID().c_str(), magMw.get());
			}
		}
	}

	updateMagnitudeLabels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::magnitudeCommentChanged(QString) {
	if ( !SC_D.netMag ) {
		return;
	}

	auto comboComment = static_cast<QComboBox*>(sender());
	auto id = comboComment->property("id").toString().toStdString();
	auto text = comboComment->currentText().toStdString();

	CommentPtr comment = SC_D.netMag->comment(id);
	if ( comment ) {
		if ( comment->text() == text ) {
			return;
		}

		comment->setText(text);
	}
	else {
		comment = new Comment;
		comment->setId(id);
		comment->setText(text);
		SC_D.netMag->add(comment.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}
}


Q_DECLARE_METATYPE(TabData)
