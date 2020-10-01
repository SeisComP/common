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


#ifndef SEISCOMP_GUI_CORE_STREAMWIDGET_H
#define SEISCOMP_GUI_CORE_STREAMWIDGET_H

#include <vector>
#include <map>
#include <list>

#include <QWidget>
#include <QGroupBox>

#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/recordsequence.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/gui/core/recordstreamthread.h>
#include <seiscomp/gui/core/recordwidget.h>
#include <seiscomp/gui/core/timescale.h>
#include <seiscomp/gui/qt.h>
#endif


namespace Seiscomp { 
namespace Gui {


class SC_GUI_API StreamWidget : public QWidget {

	Q_OBJECT


public:
	StreamWidget(const std::string& recordStreamURL,
                 const std::string& waveformStreamID,
                 const double windowLength=600,
                 QWidget* parent=0);

	~StreamWidget();
	

protected:
	virtual void closeEvent(QCloseEvent*);
	virtual void showEvent(QShowEvent*);
	virtual void resizeEvent(QResizeEvent* evt);
	
	
signals:
	void StreamWidgetClosed(StreamWidget* widget);
	
	
private slots:
	void updateRecordWidget(Seiscomp::Record* record);
	void updateRecordWidgetAlignment();

	
private:
	void startWaveformDataAcquisition();
	void stopWaveformDataAcquisition();

	
private:
		
	QTimer* _timer;
	
	QGroupBox*                          _groupBox;
	std::unique_ptr<RecordStreamThread> _thread;
	RecordWidget*                       _recordWidget;
	RecordSequence*                     _recordSequence;
	Core::TimeSpan                      _ringBufferSize;
	TimeScale*                          _timeScale;
	std::string                         _recordStreamURL;
	std::string                         _waveformStreamID;
};


} // namespace Gui
} // namespace Seiscomp

#endif
