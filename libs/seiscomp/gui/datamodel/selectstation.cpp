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



#include <seiscomp/client/inventory.h>
#include <seiscomp/datamodel/network.h>
#include <seiscomp/math/geo.h>

#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/datamodel/selectstation.h>

#include <string>

#include <QMessageBox>
#include <QSizePolicy>
#include <QHeaderView>
#include <QKeyEvent>
#include <QCompleter>


namespace Seiscomp {
namespace Gui {

namespace {


QString parse(QString s) {
	QString r;
	bool isDoubleQuote = false;
	bool isEscape = false;

	for ( int i = 0; i < s.count(); ++i ) {
		if ( s[i] == '\\' ) {
			if ( !isEscape ) {
				isEscape = true;
				continue;
			}
		}

		if ( s[i] == '"' ) {
			if ( !isEscape ) {
				isDoubleQuote = !isDoubleQuote;
				continue;
			}
		}

		isEscape = false;
		r += s[i];
	}

	return r;
}


class StationsModel : public QAbstractTableModel {
	public:
		struct Entry {
			DataModel::Station *station;
			QString code;
			QString networkType;
			QString stationType;
			QString sensorUnit;
			double distance;
			double azimuth;

			// auxilliary
			QStringList sensorUnits;
		};

		QStringList networkTypeOptions;
		QStringList stationTypeOptions;
		QStringList sensorUnitOptions;


	public:
		StationsModel(Core::Time time,
		              const QSet<QString> *blackList,
		              bool ignoreDisabledStations,
		              QObject *parent = 0) : QAbstractTableModel(parent) {
			auto *inv = Client::Inventory::Instance()->inventory();
			if ( inv ) {
				for ( size_t i = 0; i < inv->networkCount(); ++i ) {
					auto *n = inv->network(i);

					try {
						if ( n->end() <= time ) {
							continue;
						}
					}
					catch ( Core::ValueException& ) {}

					for ( size_t j = 0; j < n->stationCount(); ++j ) {
						auto *s = n->station(j);

						try {
							if ( s->end() <= time ) {
								continue;
							}
						}
						catch ( Core::ValueException& ) {}

						if ( ignoreDisabledStations
						  && !SCApp->isStationEnabled(n->code(), s->code()) ) {
							continue;
						}

						QString code = (n->code() + "." + s->code()).c_str();

						if ( blackList && blackList->contains(code) ) {
							continue;
						}

						Entry entry;

						for ( size_t l = 0; l < s->sensorLocationCount(); ++l ) {
							auto *loc = s->sensorLocation(l);
							for ( size_t c = 0; c < loc->streamCount(); ++c ) {
								auto *stream = loc->stream(c);
								if ( !entry.sensorUnits.contains(stream->gainUnit().data()) ) {
									entry.sensorUnits.append(stream->gainUnit().data());
								}
							}
						}

						entry.station = s;
						entry.code = code;
						entry.networkType = n->type().data();
						entry.stationType = s->type().data();
						entry.sensorUnit = entry.sensorUnits.join(", ");
						entry.distance = 0;
						entry.azimuth = 0;

						// collection drop down options
						if ( !networkTypeOptions.contains(entry.networkType) ) {
							networkTypeOptions.append(entry.networkType);
						}
						networkTypeOptions.sort(Qt::CaseInsensitive);

						if ( !stationTypeOptions.contains(entry.stationType) ) {
							stationTypeOptions.append(entry.stationType);
						}
						stationTypeOptions.sort(Qt::CaseInsensitive);

						foreach ( auto unit, entry.sensorUnits ) {
							if ( !sensorUnitOptions.contains(unit) ) {
								sensorUnitOptions.append(unit);
							}
						}
						sensorUnitOptions.sort(Qt::CaseInsensitive);

						_data.push_back(entry);
					}
				}
			}
		}

		void setReferenceLocation(double lat, double lon) {
			for ( int row = 0; row < _data.size(); ++row ) {
				auto *s = _data[row].station;
				double azi2;
				Math::Geo::delazi(lat, lon,
				                  s->latitude(), s->longitude(),
				                  &_data[row].distance,
				                  &_data[row].azimuth,
				                  &azi2);
			}
		}

		int rowCount(const QModelIndex &) const {
			return _data.size();
		}

		int columnCount(const QModelIndex &) const {
			return 6;
		}

		DataModel::Station *station(int row) const {
			return _data[row].station;
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
			if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
				switch ( section ) {
					case 0:
						return "ID";
					case 1:
						return "Distance";
					case 2:
						return "Azimuth";
					case 3:
						return "Network";
					case 4:
						return "Station";
					case 5:
						return "Sensor";
					default:
						break;
				}
			}
			return QVariant();
		}

		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
			switch ( role ) {
				case Qt::DisplayRole:
					switch ( index.column() ) {
						case 0:
							return _data[index.row()].code;
						case 1:
							return QString("%1").arg(_data[index.row()].distance, 0, 'f', 1);
						case 2:
							return QString("%1").arg(_data[index.row()].azimuth, 0, 'f', 1);
						case 3:
							return _data[index.row()].networkType;
						case 4:
							return _data[index.row()].stationType;
						case 5:
							return _data[index.row()].sensorUnit;
						default:
							break;
					}
					break;
				case Qt::ToolTipRole:
					switch ( index.column() ) {
						case 0:
							return _data[index.row()].code;
						case 1:
							return QString("%1").arg(_data[index.row()].distance, 0, 'f', 1);
						case 2:
							return QString("%1").arg(_data[index.row()].azimuth, 0, 'f', 1);
						case 3:
							return _data[index.row()].networkType;
						case 4:
							return _data[index.row()].stationType;
						case 5:
							return _data[index.row()].sensorUnit;
						default:
							break;
					}
					break;
				case Qt::UserRole:
					switch ( index.column() ) {
						case 1:
							return _data[index.row()].distance;
						case 2:
							return _data[index.row()].azimuth;
						default:
							break;
					}
					break;
				default:
					break;
			}

			return QVariant();
		}

		const Entry *row(int row) const {
			return &_data[row];
		}

	private:
		QVector<Entry> _data;
};


class StationsSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		StationsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {
			setFilterCaseSensitivity(Qt::CaseInsensitive);
		}

	public:
		void includeNSLC(bool f) {
			_includeNSLC = f;
		}

		void setNetworkType(const QString &type) {
			if ( type.isEmpty() ) {
				_networkType = Core::None;
			}
			else {
				_networkType = parse(type);
			}
		}

		void includeNetworkType(bool f) {
			_includeNetworkType = f;
		}

		void setStationType(const QString &type) {
			if ( type.isEmpty() ) {
				_stationType = Core::None;
			}
			else {
				_stationType = parse(type);
			}
		}

		void includeStationType(bool f) {
			_includeStationType = f;
		}

		void setSensorUnit(const QString &type) {
			if ( type.isEmpty() ) {
				_sensorUnit = Core::None;
			}
			else {
				_sensorUnit = parse(type);
			}
		}

		void includeSensorUnit(bool f) {
			_includeSensorUnit = f;
		}

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
			if ( (left.column() == 1 && right.column() == 1)
			  || (left.column() == 2 && right.column() == 2) ) {
				return sourceModel()->data(left, Qt::UserRole).toDouble() < sourceModel()->data(right, Qt::UserRole).toDouble();
			}
			else {
				return QSortFilterProxyModel::lessThan(left, right);
			}
		}

		bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
			auto entry = static_cast<StationsModel*>(sourceModel())->row(source_row);
#if QT_VERSION_MAJOR < 6
			bool nslcEmpty = filterRegExp().isEmpty();
			bool matchNSLC = nslcEmpty || filterRegExp().exactMatch(entry->code);
#else
			bool nslcEmpty = filterRegularExpression().pattern().isEmpty()
			bool matchNSLC = nslcEmpty || filterRegularExpression().match(entry->code).hasMatch();
#endif
			bool matchNetworkType = _networkType && (*_networkType == entry->networkType);
			bool matchStationType = _stationType && (*_stationType == entry->stationType);

			if ( (!nslcEmpty && (_includeNSLC != matchNSLC))
			  || (_networkType && (_includeNetworkType != matchNetworkType))
			  || (_stationType && (_includeStationType != matchStationType)) ) {
				return false;
			}

			if ( _sensorUnit ) {
				foreach ( auto unit, entry->sensorUnits ) {
					if ( unit == *_sensorUnit ) {
						return _includeSensorUnit;
					}
				}

				return !_includeSensorUnit;
			}

			return true;
		}

	private:
		bool         _includeNSLC{true};
		bool         _includeNetworkType{true};
		bool         _includeStationType{true};
		bool         _includeSensorUnit{true};
		OPT(QString) _networkType;
		OPT(QString) _stationType;
		OPT(QString) _sensorUnit;
};


}



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SelectStation::SelectStation(Core::Time time, bool ignoreDisabledStations,
                             QWidget* parent, Qt::WindowFlags f)
 : QDialog(parent, f) {
	init(time, ignoreDisabledStations, nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SelectStation::SelectStation(Core::Time time, bool ignoreDisabledStations,
                             const QSet<QString> &blackList,
                             QWidget* parent, Qt::WindowFlags f)
 : QDialog(parent, f) {
	init(time, ignoreDisabledStations, &blackList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::init(Core::Time time, bool ignoreDisabledStations,
                         const QSet<QString> *blackList) {
	_ui.setupUi(this);

	_ui.lineEditNSLC->setFocus(Qt::TabFocusReason);

	auto model = new StationsModel(time, blackList,
	                               ignoreDisabledStations, this);
	QSortFilterProxyModel *filterModel = new StationsSortFilterProxyModel(this);
	filterModel->setSourceModel(model);
	_ui.table->setModel(filterModel);

	// Configure widget
	connect(_ui.table->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
	        _ui.table, SLOT(sortByColumn(int, Qt::SortOrder)));
	_ui.table->horizontalHeader()->setSortIndicatorShown(true);
	_ui.table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	_ui.table->verticalHeader()->hide();

	_ui.table->hideColumn(1);
	_ui.table->hideColumn(2);

	_ui.table->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
	//_ui.table->horizontalHeader()->hide();

	foreach ( auto option, model->networkTypeOptions ) {
		_ui.comboNetworkType->addItem(option);
	}

	foreach ( auto option, model->stationTypeOptions ) {
		_ui.comboStationType->addItem(option);
	}

	foreach ( auto option, model->sensorUnitOptions ) {
		_ui.comboSensorUnit->addItem(option);
	}

	connect(_ui.lineEditNSLC, SIGNAL(textChanged(QString)),
	        this, SLOT(listMatchingStations()));
	connect(_ui.cbExcludeNSLC, SIGNAL(toggled(bool)),
	        this, SLOT(listMatchingStations()));

	connect(_ui.comboNetworkType, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(listMatchingStations()));
	connect(_ui.cbExcludeNetworkType, SIGNAL(toggled(bool)),
	        this, SLOT(listMatchingStations()));

	connect(_ui.comboStationType, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(listMatchingStations()));
	connect(_ui.cbExcludeStationType, SIGNAL(toggled(bool)),
	        this, SLOT(listMatchingStations()));

	connect(_ui.comboSensorUnit, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(listMatchingStations()));
	connect(_ui.cbExcludeSensorUnit, SIGNAL(toggled(bool)),
	        this, SLOT(listMatchingStations()));

	listMatchingStations();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::keyPressEvent(QKeyEvent *event) {
	if ( (event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return) ) {
		return;
	}

	QDialog::keyPressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::listMatchingStations() {
	auto *model = static_cast<StationsSortFilterProxyModel*>(_ui.table->model());
	model->includeNSLC(!_ui.cbExcludeNSLC->isChecked());

	if ( _ui.comboNetworkType->currentIndex() > 0 ) {
		model->setNetworkType(_ui.comboNetworkType->currentText().isEmpty() ? "\"\"" : _ui.comboNetworkType->currentText());
	}
	else {
		model->setNetworkType(QString());
	}
	model->includeNetworkType(!_ui.cbExcludeNetworkType->isChecked());

	if ( _ui.comboStationType->currentIndex() > 0 ) {
		model->setStationType(_ui.comboStationType->currentText().isEmpty() ? "\"\"" : _ui.comboStationType->currentText());
	}
	else {
		model->setStationType(QString());
	}
	model->includeStationType(!_ui.cbExcludeStationType->isChecked());

	if ( _ui.comboSensorUnit->currentIndex() > 0 ) {
		model->setSensorUnit(_ui.comboSensorUnit->currentText().isEmpty() ? "\"\"" : _ui.comboSensorUnit->currentText());
	}
	else {
		model->setSensorUnit(QString());
	}
	model->includeSensorUnit(!_ui.cbExcludeSensorUnit->isChecked());

	model->setFilterWildcard(_ui.lineEditNSLC->text().trimmed());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<DataModel::Station*> SelectStation::selectedStations() const {
	QModelIndexList list = _ui.table->selectionModel()->selectedIndexes();
	QList<DataModel::Station *> result;

	QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(_ui.table->model());
	StationsModel* model = static_cast<StationsModel*>(proxy->sourceModel());

	foreach ( const QModelIndex &index, list ) {
		result.push_back(model->station(proxy->mapToSource(index).row()));
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::setReferenceLocation(double lat, double lon) {
	static_cast<StationsModel*>(static_cast<QSortFilterProxyModel*>(_ui.table->model())->sourceModel())->setReferenceLocation(lat, lon);

	_ui.table->showColumn(1);
	_ui.table->showColumn(2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Gui
} // namespace Seiscomp
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
