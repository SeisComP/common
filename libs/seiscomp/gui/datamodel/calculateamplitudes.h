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



#ifndef SEISCOMP_GUI_CALCULATEAMPLITUDES_H
#define SEISCOMP_GUI_CALCULATEAMPLITUDES_H

#include <seiscomp/gui/core/recordstreamthread.h>
#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/timewindow.h>
#include <seiscomp/datamodel/waveformstreamid.h>
#include <seiscomp/processing/amplitudeprocessor.h>
#endif
#include <seiscomp/gui/qt.h>

#include <QDialog>
#include <map>
#include <set>

#include <seiscomp/gui/datamodel/ui_calculateamplitudes.h>


namespace Seiscomp {

class Record;

namespace DataModel {

DEFINE_SMARTPOINTER(Amplitude);
class Pick;
class Origin;
class DatabaseQuery;

}


namespace Gui {


class SC_GUI_API CalculateAmplitudes : public QDialog {
	Q_OBJECT

	public:
		typedef std::pair<DataModel::AmplitudePtr, bool> AmplitudeEntry;
		typedef std::multimap<std::string, AmplitudeEntry> PickAmplitudeMap;
		typedef PickAmplitudeMap::iterator iterator;
		typedef std::pair<iterator, iterator> iterator_range;
		typedef std::set<std::string> TypeSet;


	public:
		CalculateAmplitudes(DataModel::Origin *origin,
		                    QWidget * parent = 0, Qt::WindowFlags f = 0);
		~CalculateAmplitudes();

		void setOrigin(DataModel::Origin *origin);
		void setStreamURL(const std::string& streamURL);
		void setDatabase(DataModel::DatabaseQuery *);

		void setRecomputeAmplitudes(bool);
		void setAmplitudeCache(PickAmplitudeMap *cache);

		void setAmplitudeTypes(const TypeSet &types);


	public:
		int exec();
		void done(int r);

		bool process();

		//! Iterate over the computed/fetched amplitudes.
		iterator begin();
		iterator end();

		//! Returns the amplitudes belonging to a pick that has been
		//! fetched or calculated
		iterator_range pickAmplitudes(const std::string &pickID);

		iterator amplitude(const std::string &amplitudeID);

		bool isNewlyCreated(const iterator&) const;
		DataModel::AmplitudePtr amplitude(const iterator&) const;

		//! Sets the state of an amplitude that stand for 'newly created'
		void setState(iterator it, bool);

		void setSilentMode(bool f);


	private slots:
		void receivedRecord(Seiscomp::Record*);
		void finishedAcquisition();
		void filterStateChanged(int index);
		void filterTypeChanged(int index);


	private:
		void closeAcquisition();

		void addProcessor(const std::string &type,
		                  const DataModel::Pick *pick,
		                  const DataModel::SensorLocation *loc,
		                  double dist);

		void addProcessor(Processing::AmplitudeProcessor *,
		                  const DataModel::Pick *pick,
		                  int c);

		void subscribeData(Processing::AmplitudeProcessor *,
		                   const DataModel::Pick *pick,
		                   int c);

		void checkPriority(const AmplitudeEntry &newAmp);

		int addProcessingRow(const std::string &streamID, const std::string &type);

		void emitAmplitude(const Processing::AmplitudeProcessor *,
		                   const Processing::AmplitudeProcessor::Result &res);

		void setError(int row, QString text);
		void setInfo(int row, QString text);
		void setMessage(int row, QString text);
		void setProgress(int row, int progress);
		void setValue(int row, double value);

		void filterView(int startRow = 0, int cnt = -1);
		void updateTitle();


	private:
		typedef std::vector<Processing::AmplitudeProcessorPtr> ProcessorSlot;
		typedef std::map<std::string, ProcessorSlot> ProcessorMap;
		typedef std::multimap<Processing::AmplitudeProcessorCPtr, int> TableRowMap;
		typedef std::map<std::string, Seiscomp::Util::KeyValuesPtr> ParameterMap;
		typedef std::map<std::string, Seiscomp::Processing::StreamPtr> StreamMap;

		::Ui::CalculateAmplitudes _ui;

		ProcessorMap              _processors;
		ParameterMap              _parameters;
		StreamMap                 _streams;
		TableRowMap               _rows;
		PickAmplitudeMap          _amplitudes;
		PickAmplitudeMap         *_externalAmplitudeCache;
		Core::TimeWindow          _timeWindow;
		RecordStreamThread       *_thread;
		DataModel::DatabaseQuery *_query;
		DataModel::Origin        *_origin;
		TypeSet                   _amplitudeTypes;

		bool                      _recomputeAmplitudes;
		bool                      _computeSilently;
};


}

}


#endif
