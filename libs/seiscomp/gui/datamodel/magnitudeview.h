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


#ifndef SEISCOMP_GUI_MAGNITUDEVIEW_H
#define SEISCOMP_GUI_MAGNITUDEVIEW_H


#include <QComboBox>

#include <seiscomp/core/enumeration.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/gui/datamodel/amplitudeview.h>
#include <seiscomp/gui/datamodel/ui_magnitudeview_filter.h>
#include <seiscomp/gui/map/mapwidget.h>

#include <set>


namespace Seiscomp {
namespace Gui {


class ModelAbstractRowFilter {
	public:
		MAKEENUM(
			CompareOperation,
			EVALUES(
				Undefined,
				Less,
				LessEqual,
				Equal,
				NotEqual,
				Greater,
				GreaterEqual,
				Like
			),
			ENAMES(
				"None",
				"Less",
				"Less or equal",
				"Equal",
				"Not equal",
				"Greater",
				"Greater or equal",
				"Like"
			)
		);

	public:
		virtual ~ModelAbstractRowFilter() {}

		virtual int column() const = 0;
		virtual CompareOperation operation() const = 0;
		virtual QString value() const = 0;

		virtual QString toString() = 0;
		virtual bool fromString(const QString &) = 0;

		virtual bool passes(QAbstractItemModel *model, int row) = 0;
};


class SC_GUI_API MagnitudeRowFilter : public QDialog {
	Q_OBJECT

	public:
		MagnitudeRowFilter(ModelAbstractRowFilter **filter, QWidget * parent = 0,
		                   Qt::WindowFlags f = Qt::WindowFlags());

		virtual void accept();

	private slots:
		void addFilter();
		void removeFilter();

	private:
		struct Row {
			QLayout   *layout;
			QComboBox *column;
			QComboBox *operation;
			QLineEdit *value;
		};

		Row &addRow();
		void popRow();

		::Ui::MagnitudeRowFilter   _ui;
		QVector<Row>               _rows;
		ModelAbstractRowFilter   **_filter;
};


class MagnitudeViewPrivate;


class SC_GUI_API MagnitudeView : public QWidget {
	Q_OBJECT

	public:
		using AmplitudeSet = std::set<std::pair<DataModel::AmplitudePtr, bool> >;
		using StringSet = std::set<std::string>;


	public:
		MagnitudeView(const MapsDesc &maps,
		              Seiscomp::DataModel::DatabaseQuery* reader,
		              QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

		MagnitudeView(Map::ImageTree *mapTree,
		              Seiscomp::DataModel::DatabaseQuery* reader,
		              QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

		~MagnitudeView();

		void setDrawGridLines(bool);
		void setComputeMagnitudesSilently(bool);
		void setMagnitudeTypeSelectionEnabled(bool);

		void setAmplitudeConfig(const AmplitudeView::Config &config);
		const AmplitudeView::Config &amplitudeConfig() const;

		MapWidget *map() const;

		void setPreferredMagnitudeID(const std::string &);

		bool setDefaultAggregationType(const std::string &);


	signals:
		void localAmplitudesAvailable(Seiscomp::DataModel::Origin*, AmplitudeSet*, StringSet*);
		void magnitudeUpdated(const QString &, Seiscomp::DataModel::Object*);
		void magnitudeRemoved(const QString &, Seiscomp::DataModel::Object*);
		void magnitudeSelected(const QString &, Seiscomp::DataModel::Magnitude*);
		void requestClose();


	public slots:
		void drawStations(bool);
		void drawStationAnnotations(bool);

		void setOrigin(Seiscomp::DataModel::Origin *, Seiscomp::DataModel::Event *);

		bool showMagnitude(const std::string &);

		void addObject(const QString& parentID, Seiscomp::DataModel::Object*);
		void removeObject(const QString& parentID, Seiscomp::DataModel::Object*);
		void updateObject(const QString& parentID, Seiscomp::DataModel::Object*);

		//! Enables/disabled the rework of magnitudes, default is true
		void setReadOnly(bool);
		void disableRework();

		void computeMagnitudes();

		void reload();


	private slots:
		void objectDestroyed(QObject*);
		void magnitudeCreated(Seiscomp::DataModel::Magnitude*);
		void amplitudesConfirmed(Seiscomp::DataModel::Origin*, QList<Seiscomp::DataModel::AmplitudePtr>);
		void recalculateMagnitude();
		void selectChannels();
		void selectChannelsWithEdit();
		void activateChannels();
		void deactivateChannels();
		void openWaveforms();

		void magnitudesSelected();
		void hoverMagnitude(int id);
		void selectMagnitude(int id);
		void selectStation(const std::string &net, const std::string &code);
		void adjustMagnitudeRect(QRectF&);
		void tableStationMagnitudesContextMenuRequested(const QPoint &pos);
		void tableStationMagnitudesHeaderContextMenuRequested(const QPoint &pos);
		void changeMagnitudeState(int id, bool state);
		void changeStationState(int id, bool state);
		void dataChanged(const QModelIndex&, const QModelIndex&);
		void selectPreferredMagnitude(int idx);
		void tabStateChanged(int state);
		void updateContent();

		void closeTab(int idx);

		void debugCreateMagRef();
		void evaluationStatusChanged(int index);

		void magnitudeCommentChanged(QString);


	protected:
		void closeEvent(QCloseEvent *e);


	private:
		struct SourceReceiver {
			double distance;
			double azimuth;
		};

		void init(Seiscomp::DataModel::DatabaseQuery* reader);

		void setContent();
		void resetContent();

		int  addMagnitude(Seiscomp::DataModel::Magnitude*);
		void addStationMagnitude(Seiscomp::DataModel::StationMagnitude*, int);
		void updateMagnitudeLabels();
		void updateMinMaxMagnitude();

		// for calculating map boundaries from max sta dist
		DataModel::Station* getStation(DataModel::Pick* p);
		DataModel::Pick* getPick(DataModel::Arrival* a);
		void calcMinMax(Seiscomp::DataModel::Origin*, double& latMin, double& latMax, double& lonMin, double& lonMax );

		//! returns distance to origin
		SourceReceiver addStationMagnitude(DataModel::Magnitude* magnitude,
		                                   DataModel::StationMagnitude* stationMagnitude,
		                                   double weight);

		struct MagnitudeStatus {
			MagnitudeStatus(const  std::string &t,
			                const DataModel::Amplitude *a,
			                Processing::MagnitudeProcessor::Status s,
			                bool isWarning = false)
			: type(t), amplitude(a), status(s), warning(isWarning) {}

			std::string                             type;
			const DataModel::Amplitude             *amplitude;
			Processing::MagnitudeProcessor::Status  status;
			bool                                    warning;
		};

		typedef QList<MagnitudeStatus> MagnitudeStats;

		DataModel::Magnitude *
		computeStationMagnitudes(const std::string &magType,
		                         QList<DataModel::AmplitudePtr> *amps,
		                         MagnitudeStats *errors = nullptr);

		void computeMagnitude(DataModel::Magnitude *magnitude, const std::string &aggType);
		bool editSelectionFilter();
		void resetPreferredMagnitudeSelection();


	private:
		MagnitudeViewPrivate *_d_ptr;
};


}
}

#endif
