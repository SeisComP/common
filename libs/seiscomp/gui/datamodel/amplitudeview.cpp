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



#define SEISCOMP_COMPONENT Gui::AmplitudeView

#include "amplitudeview.h"
#include <seiscomp/core/platform/platform.h>
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/gui/datamodel/selectstation.h>
#include <seiscomp/gui/datamodel/origindialog.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/recordstreamthread.h>
#include <seiscomp/gui/core/timescale.h>
#include <seiscomp/gui/core/uncertainties.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/client/configdb.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/parameter.h>
#include <seiscomp/datamodel/stationmagnitudecontribution.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/utils/misc.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/logging/log.h>

#include <QMessageBox>
#include <functional>
#include <numeric>
#include <fstream>
#include <limits>
#include <set>

#include "./amplitudeview_p.h"


#ifdef MACOSX
#include <seiscomp/gui/core/osx.h>
#endif

#define NO_FILTER_STRING       "Raw"
#define DEFAULT_FILTER_STRING  "Default"

#define ITEM_DISTANCE_INDEX  0
#define ITEM_AZIMUTH_INDEX  1
#define ITEM_PRIORITY_INDEX  2
//#define ITEM_ARRIVALID_INDEX 2

#define THEORETICAL_POSTFIX  "  "

#define SC_D (*_d_ptr)


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;
using namespace Seiscomp::Util;
using namespace Seiscomp::Gui;
using namespace Seiscomp::Gui::PrivateAmplitudeView;


namespace {


char COMPS[3] = {'Z', '1', '2'};

struct StationItem {
	AmplitudePtr amp;
	PickPtr      pick;
	bool         isTrigger;
};


class TraceList : public RecordView {
	public:
		TraceList(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		 : RecordView(parent, f) {}

		TraceList(const Seiscomp::Core::TimeWindow& tw,
		          QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		 : RecordView(tw, parent, f) {}

		TraceList(const Seiscomp::Core::TimeSpan& ts,
		          QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		 : RecordView(ts, parent, f) {}

	protected:
		RecordLabel* createLabel(RecordViewItem *item) const {
			return new AmplitudeRecordLabel;
		}

		void dropEvent(QDropEvent *event) {
			if ( event->mimeData()->hasFormat("text/plain") ) {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
				QString strFilter = event->mimeData()->text();
#else
				QString strFilter = event->mimeData()->data("text/plain");
#endif
				auto f = RecordWidget::Filter::Create(strFilter.toStdString());

				if ( !f ) {
					QMessageBox::critical(
						this, "Create filter",
						QString("Invalid filter: %1").arg(strFilter)
					);
					return;
				}

				delete f;
				emit filterChanged(strFilter);
			}
		}
};


class TraceDecorator : public RecordWidgetDecorator {
	public:
		TraceDecorator(QObject *parent, AmplitudeRecordLabel *itemLabel)
		: RecordWidgetDecorator(parent), _itemLabel(itemLabel) {}

		AmplitudeRecordLabel *label() { return _itemLabel; }

		void drawDecoration(QPainter *painter, RecordWidget *widget) {
			if ( _itemLabel->processor ) {
				painter->setClipRect(widget->canvasRect());

				int nbegin = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().noiseBegin));
				int nend = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().noiseEnd));
				int sbegin = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().signalBegin));
				int send = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().signalEnd));

				// Draw unused area before noise
				painter->fillRect(0,0,nbegin,widget->height(), QColor(0,0,0,92));
				// Draw noise area
				painter->fillRect(nbegin,0,nend-nbegin,widget->height(), QColor(0,0,0,64));
				// Draw unused area between noise and signal
				painter->fillRect(nend,0,sbegin-nend,widget->height(), QColor(0,0,0,92));
				// Draw signal area
				painter->fillRect(send,0,widget->width()-send,widget->height(), QColor(0,0,0,92));

				if ( !_itemLabel->infoText.isEmpty() ) {
					QRect boundingRect =
						widget->fontMetrics().boundingRect(painter->window(),
						                                   Qt::AlignRight|Qt::AlignTop,
						                                   _itemLabel->infoText);
					boundingRect.adjust(-9,0,-1,8);

					if ( _itemLabel->isError ) {
						painter->setPen(Qt::white);
						painter->setBrush(QColor(128,0,0,192));
					}
					else {
						painter->setPen(qApp->palette().color(QPalette::WindowText));
						painter->setBrush(QColor(255,255,255,192));
					}

					painter->drawRect(boundingRect);

					if ( _itemLabel->processor->status() > Processing::WaveformProcessor::Finished )
						painter->setPen(Qt::white);

					painter->drawText(boundingRect, Qt::AlignHCenter|Qt::AlignVCenter, _itemLabel->infoText);
				}

				painter->setClipping(false);
			}
		}

	private:
		AmplitudeRecordLabel *_itemLabel;
};


class MyRecordWidget : public RecordWidget {
	public:
		MyRecordWidget() {
			if ( SCScheme.colors.records.background.isValid() ) {
				QPalette p = palette();
				p.setColor(QPalette::Base, SCScheme.colors.records.background);
				setPalette(p);
				setAutoFillBackground(true);
			}
		}

		void setSelected(const Core::Time &t1, const Core::Time &t2) {
			_t1 = t1;
			_t2 = t2;
			update();
		}

	protected:
		void drawCustomBackground(QPainter &painter) {
			if ( !_t1 || !_t2 ) return;

			// The painter transform is set up that the upper left coordinate
			// lies on the upper left coordinate of the trace canvas

			int x1 = mapCanvasTime(_t1);
			int x2 = mapCanvasTime(_t2);

			//int y1 = streamYPos(currentRecords());
			//int h = streamHeight(currentRecords());
			int y1 = 0;
			int h = height();

			QColor col = palette().color(QPalette::Highlight);
			col.setAlpha(64);
			painter.fillRect(x1,y1,x2-x1+1,h, col);
		}

	private:
		Core::Time _t1, _t2;
};


class AmplitudeViewMarker : public RecordMarker {
	public:
		enum Type {
			UndefinedType, /* Something undefined */
			Reference,     /* The amplitude reference marker (first P arrival) */
			Amplitude,     /* An amplitude */
			Theoretical    /* A theoretical marker */
		};

	public:
		AmplitudeViewMarker(RecordWidget *parent,
		                    const Seiscomp::Core::Time& pos,
		                    Type type, bool newAmplitude)
		: RecordMarker(parent, pos),
		  _type(type),
		  _slot(-1) {
			setMovable(newAmplitude);
			init();
		}

		AmplitudeViewMarker(RecordWidget *parent,
		                    const Seiscomp::Core::Time& pos,
		                    const QString& text,
		                    Type type, bool newAmplitude)
		: RecordMarker(parent, pos, text),
		  _type(type),
		  _slot(-1) {
			setMovable(newAmplitude);
			init();
		}

		AmplitudeViewMarker(RecordWidget *parent,
		                    const AmplitudeViewMarker& m)
		: RecordMarker(parent, m)
		, _referencedAmplitude(m._referencedAmplitude)
		, _manualAmplitude(m._manualAmplitude)
		, _type(m._type)
		, _slot(m._slot)
		{
			init();
			_time = m._time;
		}

		virtual ~AmplitudeViewMarker() {}


	private:
		void init() {
			_twBegin = _twEnd = 0;
			_manualAmplitude = nullptr;
			setMoveCopy(false);
			updateVisual();
		}


	public:
		void setEnabled(bool enable) {
			RecordMarker::setEnabled(enable);
			updateVisual();
		}

		void setSlot(int s) {
			_slot = s;
		}

		char slot() const {
			return _slot;
		}

		void setType(Type t) {
			_type = t;
			updateVisual();
		}

		Type type() const {
			return _type;
		}

		void setTimeWindow(float begin, float end) {
			_twBegin = begin;
			_twEnd = end;
		}

		float timeWindowBegin() const {
			return _twBegin;
		}

		float timeWindowEnd() const {
			return _twEnd;
		}

		void setMagnitude(OPT(double) mag, const QString &error) {
			_magnitude = mag;
			_magnitudeError = error;

			if ( _magnitude )
				setDescription(QString("%1: %2").arg(text()).arg(*_magnitude, 0, 'f', 2));
			else
				setDescription("");
		}

		void setAmplitude(DataModel::Amplitude *a) {
			_referencedAmplitude = a;
			_time = a->timeWindow().reference();

			try {
				setTimeWindow(a->timeWindow().begin(), a->timeWindow().end());
			}
			catch ( ... ) {
				_twBegin = _twEnd = 0;
			}

			if ( _referencedAmplitude )
				_manualAmplitude = nullptr;

			updateVisual();
		}

		void setAmplitudeResult(DataModel::Amplitude *a) {
			_manualAmplitude = a;
		}

		void setFilterID(const std::string &filterID) {
			_filterID = filterID;
		}

		const std::string &filterID() const {
			return _filterID;
		}

		DataModel::Amplitude *amplitudeResult() const {
			return _manualAmplitude.get();
		}

		void setPick(DataModel::Pick *p) {
			_pick = p;
		}

		void convertToManualAmplitude() {
			if ( !_referencedAmplitude ) return;
			_referencedAmplitude = nullptr;
			setMovable(true);
			setDescription("");
			updateVisual();
		}

		DataModel::Amplitude *amplitude() const {
			return _referencedAmplitude.get();
		}

		DataModel::Amplitude *manualAmplitude() const {
			return _manualAmplitude.get();
		}

		bool equalsAmplitude(DataModel::Amplitude *amp) const {
			if ( !amp ) {
				return false;
			}

			// Time + uncertainties do not match: not equal
			if ( correctedTime() != amp->timeWindow().reference() ) {
				return false;
			}

			if ( !_manualAmplitude ) {
				return false;
			}

			try {
				if ( _manualAmplitude->amplitude().value() != amp->amplitude().value() ) {
					return false;
				}
			}
			catch ( ... ) {
				return false;
			}

			return true;
		}

		bool isAmplitude() const {
			return _type == Amplitude;
		}

		bool isNewAmplitude() const {
			return _type == Amplitude && _manualAmplitude;
		}

		bool isReference() const {
			return _type == Reference;
		}

		bool isTheoretical() const {
			return _type == Theoretical;
		}

		RecordMarker *copy() { return new AmplitudeViewMarker(nullptr, *this); }

		void draw(QPainter &painter, RecordWidget *context, int x, int y1, int y2,
		          QColor color, qreal lineWidth) {
			// Adjust vertical position to current slot
			if ( _slot >= 0 ) {
				y1 = context->streamYPos(_slot);
				y2 = y1 + context->streamHeight(_slot);
			}
			else {
				y1 = 0;
				y2 = context->height();
			}

			int twb = (int)(_twBegin * context->timeScale());
			int twe = (int)(_twEnd * context->timeScale());

			if ( twe-twb > 0 )
				painter.fillRect(x+twb,y2-10,twe-twb+1,10, QColor(color.red(), color.green(), color.blue(), 64));

			if ( _slot >= 0 ) {
				painter.setPen(QPen(color, lineWidth, Qt::DashLine));
				painter.drawLine(x, 0, x, y1);
				painter.drawLine(x, y2, x, context->height());
			}

			RecordMarker::draw(painter, context, x, y1, y2, color, lineWidth);

			if ( !_magnitudeError.isEmpty() ) {
				painter.save();

				static QPoint marker[3] = {QPoint(-1,2), QPoint(1,2), QPoint(0,0)};

				int fh = 24;
				x += fh/4;
				y1 += 2;

				painter.setRenderHint(QPainter::Antialiasing);
				painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
				painter.setBrush(Qt::yellow);

				marker[0] = QPoint(x-fh/2, y1);
				marker[1] = QPoint(x+fh/2, y1);
				marker[2] = QPoint(x,y1+fh);

				painter.drawPolygon(marker, 3);

				y2 = y1+fh*3/4;
				painter.drawLine(x,y1+4,x,y2-8);
				painter.drawLine(x,y2-6,x,y2-5);

				painter.restore();
			}
		}

		QString toolTip() const {
			QString text;

			if ( (!_referencedAmplitude) && !isAmplitude() )
				return text;

			if ( _magnitude )
				text += QString("magnitude: %1").arg(*_magnitude,0,'f',2);
			else
				text += "magnitude: -";

			if ( !_magnitudeError.isEmpty() )
				text += QString(" (%1)").arg(_magnitudeError);

			if ( !text.isEmpty() )
				text += "\n\n";

			if ( _referencedAmplitude ) {
				try {
					switch ( _referencedAmplitude->evaluationMode() ) {
						case MANUAL:
							text += "manual ";
							break;
						case AUTOMATIC:
							text += "automatic ";
							break;
						default:
							break;
					}
				}
				catch ( ... ) {}

				text += "amplitude";

				try {
					text += QString(" created by %1").arg(_referencedAmplitude->creationInfo().author().c_str());
				}
				catch ( ... ) {}

				try {
					text += QString(" at %1").arg(timeToString(_referencedAmplitude->creationInfo().creationTime(), "%F %T"));
				}
				catch ( ... ) {}

				try {
					text += QString("\nvalue: %1").arg(_referencedAmplitude->amplitude().value());
				}
				catch ( ... ) {}

				try {
					text += QString("\nuncertainty: -%1, +%2")
					        .arg(_referencedAmplitude->amplitude().lowerUncertainty())
					        .arg(_referencedAmplitude->amplitude().upperUncertainty());
				}
				catch ( ... ) {
					try {
						text += QString("\nuncertainty: %1").arg(_referencedAmplitude->amplitude().uncertainty());
					}
					catch ( ... ) {}
				}

				try {
					text += QString("\nperiod: %1").arg(_referencedAmplitude->period());
				}
				catch ( ... ) {}

				try {
					text += QString("\nsnr: %1").arg(_referencedAmplitude->snr());
				}
				catch ( ... ) {}

				if ( !_referencedAmplitude->filterID().empty() )
					text += QString("\nfilter: %1").arg(_referencedAmplitude->filterID().c_str());

				if ( !_referencedAmplitude->methodID().empty() )
					text += QString("\nmethod: %1").arg(_referencedAmplitude->methodID().c_str());
			}
			else if ( _manualAmplitude ){
				text += "amplitude\n";
				text += QString("value: %1").arg(_manualAmplitude->amplitude().value());

				try {
					text += QString("\nuncertainty: -%1, +%2")
					        .arg(_manualAmplitude->amplitude().lowerUncertainty())
					        .arg(_manualAmplitude->amplitude().upperUncertainty());
				}
				catch ( ... ) {
					try {
						text += QString("\nuncertainty: %1").arg(_manualAmplitude->amplitude().uncertainty());
					}
					catch ( ... ) {}
				}

				try {
					if ( _manualAmplitude->period() > 0 )
						text += QString("\nperiod: %1").arg(_manualAmplitude->period());
				}
				catch ( ... ) {}

				try {
					if ( _manualAmplitude->snr() >= 0 )
						text += QString("\nsnr: %1").arg(_manualAmplitude->snr());
				}
				catch ( ... ) {}

				if ( !_filterID.empty() )
					text += QString("\nfilter: %1").arg(_filterID.c_str());
			}

			return text;
		}

	private:
		void updateVisual() {
			QColor col = SCScheme.colors.picks.disabled;
			Qt::Alignment al = Qt::AlignVCenter;
			EvaluationMode state = AUTOMATIC;

			DataModel::Amplitude *a = amplitude();
			if ( a ) {
				try { state = a->evaluationMode(); } catch ( ... ) {}
			}
			
			if ( isMovable() )
				state = MANUAL;

			switch ( _type ) {
				case Amplitude:
					if ( isEnabled() ) {
						switch ( state ) {
							case MANUAL:
								col = SCScheme.colors.arrivals.manual;
								break;
							case AUTOMATIC:
							default:
								col = SCScheme.colors.arrivals.automatic;
								break;
						}
					}
					else
						col = SCScheme.colors.arrivals.disabled;

					al = Qt::AlignVCenter;
					break;

				case Reference:
					col = SCScheme.colors.arrivals.undefined;
					al = Qt::AlignTop;
					break;

				case Theoretical:
					col = SCScheme.colors.arrivals.theoretical;
					al = Qt::AlignBottom;
					break;

				default:
					break;
			}

			setColor(col);
			setAlignment(al);
		}


	private:
		AmplitudePtr _referencedAmplitude;
		AmplitudePtr _manualAmplitude;
		PickPtr      _pick;
		TimeQuantity _time;
		Type         _type;
		int          _slot;
		float        _twBegin;
		float        _twEnd;
		OPT(double)  _magnitude;
		QString      _magnitudeError;
		std::string  _filterID;
};


bool isTraceUsed(Seiscomp::Gui::RecordWidget *w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(w->marker(i));
		if ( !m->isEnabled() ) continue;
		if ( m->type() == AmplitudeViewMarker::Amplitude ) return true;
	}

	return false;
}

bool isTracePicked(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(w->marker(i));
		if ( m->type() == AmplitudeViewMarker::Amplitude ) return true;
	}

	return false;
}

SensorLocation *findSensorLocation(Station *station, const std::string &code,
                                   const Seiscomp::Core::Time &time) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		if ( loc->code() == code )
			return loc;
	}

	return nullptr;
}

Stream* findStream(Station *station, const std::string &code,
                   const Seiscomp::Core::Time &time) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			if ( stream->code().substr(0, code.size()) != code ) continue;

			return stream;
		}
	}

	return nullptr;
}

Stream* findStream(Station *station, const std::string &code, const std::string &locCode,
                   const Seiscomp::Core::Time &time) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		if ( loc->code() != locCode ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			if ( stream->code().substr(0, code.size()) != code ) continue;

			return stream;
		}
	}

	return nullptr;
}

Stream* findStream(Station *station, const Seiscomp::Core::Time &time,
                   Processing::WaveformProcessor::SignalUnit requestedUnit) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			Sensor *sensor = Sensor::Find(stream->sensor());
			if ( !sensor ) continue;

			Processing::WaveformProcessor::SignalUnit unit;

			// Unable to retrieve the unit enumeration from string
			if ( !unit.fromString(sensor->unit().c_str()) ) continue;
			if ( unit != requestedUnit ) continue;

			return stream;
		}
	}

	return nullptr;
}

Stream* findConfiguredStream(Station *station, const Seiscomp::Core::Time &time) {
	DataModel::Stream *stream = nullptr;
	DataModel::ConfigModule *module = SCApp->configModule();
	if ( module ) {
		for ( size_t ci = 0; ci < module->configStationCount(); ++ci ) {
			DataModel::ConfigStation* cs = module->configStation(ci);
			if ( cs->networkCode() == station->network()->code() &&
			     cs->stationCode() == station->code() ) {

				for ( size_t si = 0; si < cs->setupCount(); ++si ) {
					DataModel::Setup* setup = cs->setup(si);

					DataModel::ParameterSet* ps = nullptr;
					try {
						ps = DataModel::ParameterSet::Find(setup->parameterSetID());
					}
					catch ( Core::ValueException & ) {
						continue;
					}

					if ( !ps ) {
						SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
						continue;
					}

					std::string net, sta, loc, cha;
					net = cs->networkCode();
					sta = cs->stationCode();
					for ( size_t pi = 0; pi < ps->parameterCount(); ++pi ) {
						DataModel::Parameter* par = ps->parameter(pi);
						if ( par->name() == "detecLocid" )
							loc = par->value();
						else if ( par->name() == "detecStream" )
							cha = par->value();
					}

					// No channel defined
					if ( cha.empty() ) continue;
					stream = findStream(station, cha, loc, time);
					if ( stream ) return stream;
				}
			}
		}
	}

	return stream;
}

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

std::string adjustChannelCode(const std::string& channelCode, bool allComponents) {
	if ( channelCode.size() < 3 )
		return channelCode + (allComponents?'?':'Z');
	else
		return allComponents?channelCode.substr(0,2) + '?':channelCode;
}

WaveformStreamID setWaveformIDComponent(const WaveformStreamID& id, char component) {
	return WaveformStreamID(id.networkCode(), id.stationCode(), id.locationCode(),
	                        id.channelCode().substr(0,2) + component, id.resourceURI());
}

WaveformStreamID adjustWaveformStreamID(const WaveformStreamID& id) {
	return WaveformStreamID(id.networkCode(), id.stationCode(), id.locationCode(),
	                        adjustChannelCode(id.channelCode(), true), id.resourceURI());
}

std::string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}

bool isLinkedItem(RecordViewItem *item) {
	return static_cast<AmplitudeRecordLabel*>(item->label())->isLinkedItem();
}

void unlinkItem(RecordViewItem *item) {
	static_cast<AmplitudeRecordLabel*>(item->label())->unlink();
	item->disconnect(SIGNAL(firstRecordAdded(const Seiscomp::Record*)));
}

void selectFirstVisibleItem(RecordView *view) {
	for ( int i = 0; i < view->rowCount(); ++i ) {
		view->setCurrentItem(view->itemAt(i));
		view->ensureVisible(i);
		break;
	}
}


const TravelTime *findPhase(const TravelTimeList &ttt, const QString &phase, double delta) {
	if ( phase == "P" || phase == "P1" ) {
		return firstArrivalP(&ttt);
	}

	// First pass -> exact match
	for ( const auto &tt : ttt ) {
		if ( delta > 115 ) { // skip Pdiff et al.
			if ( tt.phase ==  "Pdiff" ) continue;
			if ( tt.phase == "pPdiff" ) continue;
			if ( tt.phase == "sPdiff" ) continue;
		}

		QString ph(tt.phase.c_str());

		if ( phase == ph ) {
			return &tt;
		}

		if ( phase == "P" &&
		     (tt.phase == "Pn" || tt.phase == "Pg" || tt.phase == "Pb") ) {
			return &tt;
		}
	}

	if ( phase != "P" && phase != "S" )
		return nullptr;

	// Second pass -> find first phase that represents a
	// P or S phase
	for ( const auto &tt : ttt ) {
		if ( delta > 115 ) { // skip Pdiff et al.
			if ( tt.phase ==  "Pdiff" ) continue;
			if ( tt.phase == "pPdiff" ) continue;
			if ( tt.phase == "sPdiff" ) continue;
		}

		if ( phase[0] == getShortPhaseName(tt.phase) ) {
			return &tt;
		}
	}

	return nullptr;
}


}


namespace Seiscomp {
namespace Gui {
namespace PrivateAmplitudeView {


ThreeComponentTrace::~ThreeComponentTrace() {
	for ( int i = 0; i < 3; ++i ) {
		if ( traces[i].raw ) delete traces[i].raw;
		if ( widget ) widget->setRecords(traces[i].recordSlot, nullptr);
		if ( traces[i].transformed ) delete traces[i].transformed;
		if ( traces[i].processed ) delete traces[i].processed;
		if ( traces[i].filter ) delete traces[i].filter;
	}
}


void ThreeComponentTrace::setFilter(RecordWidget::Filter *f, const std::string &filterID_) {
	if ( f )
		filterID = filterID_;
	else
		filterID.clear();

	for ( int i = 0; i < 3; ++i ) {
		if ( traces[i].filter ) delete traces[i].filter;
		traces[i].filter = f?f->clone():nullptr;

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = nullptr;

			if ( widget && !showProcessed ) widget->setRecords(traces[i].recordSlot, nullptr);
		}

		removeProcessedData(i);
	}

	try {
		transform();
	}
	catch ( std::exception &e ) {
		label->infoText = e.what();
	}
}


void ThreeComponentTrace::setRecordWidget(RecordWidget *w) {
	if ( widget ) {
		for ( int i = 0; i < 3; ++i )
			widget->setRecords(traces[i].recordSlot, nullptr);
		widget->disconnect(this);
	}

	widget = w;

	if ( widget ) {
		for ( int i = 0; i < 3; ++i )
			widget->setRecords(traces[i].recordSlot, showProcessed?traces[i].processed:traces[i].transformed, false);
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));
	}
}


void ThreeComponentTrace::widgetDestroyed(QObject *obj) {
	if ( obj == widget )
		widget = nullptr;
}


void ThreeComponentTrace::setTransformationEnabled(bool f) {
	enableTransformation = f;

	for ( int i = 0; i < 3; ++i ) {
		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = nullptr;

			if ( widget && !showProcessed ) widget->setRecords(traces[i].recordSlot, nullptr);
		}
	}

	transform();
}


void ThreeComponentTrace::showProcessedData(bool e) {
	showProcessed = e;
	if ( !widget ) return;
	for ( int i = 0; i < 3; ++i )
		widget->setRecords(traces[i].recordSlot, showProcessed?traces[i].processed:traces[i].transformed, false);
}


bool ThreeComponentTrace::setProcessedData(int comp,
                                           const std::string &networkCode,
                                           const std::string &stationCode,
                                           const std::string &locationCode,
                                           const Core::Time &startTime,
                                           double samplingFrequency,
                                           DoubleArrayPtr data) {
	GenericRecordPtr prec = new GenericRecord(networkCode,
	                                          stationCode,
	                                          locationCode,
	                                          traces[comp].channelCode,
	                                          startTime, samplingFrequency);
	prec->setData(data.get());
	prec->dataUpdated();

	try {
		prec->endTime();
	}
	catch ( ... ) {
		return false;
	}

	if ( traces[comp].processed == nullptr )
		traces[comp].processed = new RingBuffer(0);
	else
		traces[comp].processed->clear();

	traces[comp].processed->feed(prec.get());

	if ( widget && showProcessed )
		widget->setRecords(traces[comp].recordSlot, traces[comp].processed, false);

	return true;
}


void ThreeComponentTrace::removeProcessedData(int comp) {
	if ( traces[comp].processed ) {
		delete traces[comp].processed;
		traces[comp].processed = nullptr;
	}

	if ( widget && showProcessed )
		widget->setRecords(traces[comp].recordSlot, traces[comp].processed, false);
}


bool ThreeComponentTrace::transform(int comp, Record *rec) {
	Core::Time minStartTime;
	Core::Time maxStartTime;
	Core::Time minEndTime;
	bool gotRecords = false;

	if ( enableTransformation ) {
		// Not all traces available, nothing to do
		for ( int i = 0; i < 3; ++i ) {
			// Delete current transformed records
			if ( traces[i].transformed ) {
				//delete traces[i].transformed;
				//traces[i].transformed = nullptr;
				Core::Time endTime = traces[i].transformed->back()->endTime();
				if ( endTime > minStartTime )
					minStartTime = endTime;
			}

			if ( !traces[i].raw || traces[i].raw->empty() )
				return false;
		}

		// Find common start time for all three components
		RecordSequence::iterator it[3];
		RecordSequence::iterator it_end[3];
		int maxStartComponent;
		int skips;
		double samplingFrequency, timeTolerance;

		// Initialize iterators for each component
		for ( int i = 0; i < 3; ++i )
			it[i] = traces[i].raw->begin();

		// Store sampling frequency of first record of first component
		// All records must match this sampling frequency
		samplingFrequency = (*it[0])->samplingFrequency();
		timeTolerance = 0.5 / samplingFrequency;

		while ( true ) {
			if ( minStartTime ) {
				for ( int i = 0; i < 3; ++i ) {
					while ( it[i] != traces[i].raw->end() ) {
						if ( (*it[i])->endTime() <= minStartTime ) {
							++it[i];
						}
						else
							break;
					}

					// End of stream?
					if ( it[i] == traces[i].raw->end() ) {
						return gotRecords;
					}
				}
			}

			// Advance all other components to first record matching
			// the first sampling frequency found
			for ( int i = 0; i < 3; ++i ) {
				while ( ((*it[i])->samplingFrequency() != samplingFrequency) ) {
					++it[i];
					// No matching sampling frequency found?
					if ( it[i] == traces[i].raw->end() )
						return gotRecords;
				}
			}

			// Find maximum start time of all three records
			skips = 1;
			while ( skips ) {
				for ( int i = 0; i < 3; ++i ) {
					if ( !i || maxStartTime < (*it[i])->startTime() ) {
						maxStartTime = (*it[i])->startTime();
						maxStartComponent = i;
					}
				}

				skips = 0;

				// Check all other components against maxStartTime
				for ( int i = 0; i < 3; ++i ) {
					if ( i == maxStartComponent ) continue;
					while ( (*it[i])->samplingFrequency() != samplingFrequency ||
					        (*it[i])->endTime() <= maxStartTime ) {
						++it[i];

						// End of sequence? Nothing can be done anymore
						if ( it[i] == traces[i].raw->end() )
							return gotRecords;

						// Increase skip counter
						++skips;
					}
				}
			}

			// Advance all iterators to last non-gappy record
			for ( int i = 0; i < 3; ++i ) {
				RecordSequence::iterator tmp = it[i];
				it_end[i] = it[i];
				++it_end[i];
				while ( it_end[i] != traces[i].raw->end() ) {
					const Record *rec = it_end[i]->get();

					// Skip records with wrong sampling frequency
					if ( rec->samplingFrequency() != samplingFrequency ) break;

					double diff = (double)(rec->startTime()-(*tmp)->endTime());
					if ( fabs(diff) > timeTolerance ) break;

					tmp = it_end[i];
					++it_end[i];
				}

				it_end[i] = tmp;
			}

			// Find minimum end time of all three records
			for ( int i = 0; i < 3; ++i ) {
				if ( !i || minEndTime > (*it_end[i])->endTime() )
					minEndTime = (*it_end[i])->endTime();
			}


			GenericRecordPtr comps[3];
			int minLen = 0;

			// Clip maxStartTime to minStartTime
			if ( maxStartTime < minStartTime )
				maxStartTime = minStartTime;

			// Rotate records
			for ( int i = 0; i < 3; ++i ) {
				float tq = 0;
				int tqCount = 0;
				GenericRecordPtr rec = new GenericRecord((*it[i])->networkCode(),
				                                         (*it[i])->stationCode(),
				                                         (*it[i])->locationCode(),
				                                         (*it[i])->channelCode(),
				                                         maxStartTime, samplingFrequency);

				DoubleArrayPtr data = new DoubleArray;
				RecordSequence::iterator seq_end = it_end[i];
				++seq_end;

				for ( RecordSequence::iterator rec_it = it[i]; rec_it != seq_end; ++rec_it ) {
					const Array *rec_data = (*rec_it)->data();
					if ( rec_data == nullptr ) {
						SEISCOMP_ERROR("%s: no data for record", (*rec_it)->streamID().c_str());
						return gotRecords;
					}

					if ( (*rec_it)->startTime() > minEndTime )
						break;

					++it[i];

					const DoubleArray *srcData = DoubleArray::ConstCast(rec_data);
					DoubleArrayPtr tmp;
					if ( srcData == nullptr ) {
						tmp = static_cast<DoubleArray*>(data->copy(Array::DOUBLE));
						srcData = tmp.get();
					}

					int startIndex = 0;
					int endIndex = srcData->size();

					if ( (*rec_it)->startTime() < maxStartTime )
						startIndex += (int)(double(maxStartTime-(*rec_it)->startTime())*(*rec_it)->samplingFrequency());

					if ( (*rec_it)->endTime() > minEndTime )
						endIndex -= (int)(double((*rec_it)->endTime()-minEndTime)*(*rec_it)->samplingFrequency());

					int len = endIndex-startIndex;
					// Skip empty records
					if ( len <= 0 ) continue;

					if ( (*rec_it)->timingQuality() >= 0 ) {
						tq += (*rec_it)->timingQuality();
						++tqCount;
					}

					data->append(len, srcData->typedData()+startIndex);
				}

				if ( tqCount > 0 )
					rec->setTimingQuality((int)(tq / tqCount));

				minLen = i==0?data->size():std::min(minLen, data->size());

				rec->setData(data.get());

				comps[i] = rec;
			}

			// Create record sequences
			for ( int i = 0; i < 3; ++i ) {
				DoubleArray *data = static_cast<DoubleArray*>(comps[i]->data());
				if ( data->size() > minLen ) {
					data->resize(minLen);
					comps[i]->dataUpdated();
				}

				// Create ring buffer without limit if needed
				if ( traces[i].transformed == nullptr ) {
					traces[i].transformed = new RingBuffer(0);
					if ( widget && !showProcessed )
						widget->setRecords(traces[i].recordSlot, traces[i].transformed, false);
					if ( traces[i].filter )
						traces[i].filter->setSamplingFrequency(comps[i]->samplingFrequency());
				}

				transformedRecord(i, comps[i].get());
			}

			gotRecords = true;

			double *dataZ = static_cast<DoubleArray*>(comps[0]->data())->typedData();
			double *data1 = static_cast<DoubleArray*>(comps[1]->data())->typedData();
			double *data2 = static_cast<DoubleArray*>(comps[2]->data())->typedData();

			// Rotate finally
			for ( int i = 0; i < minLen; ++i ) {
				Math::Vector3d v = transformation * Math::Vector3d(*data2, *data1, *dataZ);
				*dataZ = v.z;
				*data1 = v.y;
				*data2 = v.x;

				++dataZ; ++data1; ++data2;
			}

			// And filter
			for ( int i = 0; i < 3; ++i ) {
				if ( traces[i].filter )
					traces[i].filter->apply(*static_cast<DoubleArray*>(comps[i]->data()));
			}

			minStartTime = minEndTime;
		}
	}
	else {
		// Record passed that needs filtering?
		if ( rec ) {
			if ( traces[comp].transformed == nullptr ) {
				traces[comp].transformed = new RingBuffer(0);
				if ( widget && !showProcessed )
					widget->setRecords(traces[comp].recordSlot, traces[comp].transformed, false);
				if ( traces[comp].filter )
					traces[comp].filter->setSamplingFrequency(rec->samplingFrequency());
			}

			RecordPtr toFeed;
			if ( traces[comp].filter ) {
				GenericRecordPtr grec = new GenericRecord(rec->networkCode(),
				                                          rec->stationCode(),
				                                          rec->locationCode(),
				                                          rec->channelCode(),
				                                          rec->startTime(),
				                                          rec->samplingFrequency(),
				                                          rec->timingQuality());

				DoubleArrayPtr data = static_cast<DoubleArray*>(rec->data()->copy(Array::DOUBLE));
				traces[comp].filter->apply(*data);
				grec->setData(data.get());
				toFeed = grec;
			}
			else
				toFeed = rec;

			transformedRecord(comp, toFeed.get());
			gotRecords = true;
		}
		else {
			// Just copy the records and filter them if activated
			for ( int i = 0; i < 3; ++i ) {
				if ( traces[i].raw == nullptr || traces[i].raw->empty() ) continue;

				RecordSequence::iterator it;
				if ( traces[i].transformed == nullptr )
					it = traces[i].raw->begin();
				else {
					Core::Time endTime = traces[i].transformed->back()->endTime();
					for ( RecordSequence::iterator s_it = traces[i].raw->begin();
					      s_it != traces[i].raw->end(); ++s_it ) {
						if ( (*s_it)->startTime() >= endTime ) {
							it = s_it;
							break;
						}
					}
				}

				for ( RecordSequence::iterator s_it = it;
				      s_it != traces[i].raw->end(); ++s_it ) {

					RecordCPtr s_rec = s_it->get();

					if ( traces[i].transformed == nullptr ) {
						traces[i].transformed = new RingBuffer(0);
						if ( widget && !showProcessed )
							widget->setRecords(traces[i].recordSlot, traces[i].transformed, false);
						if ( traces[i].filter )
							traces[i].filter->setSamplingFrequency(s_rec->samplingFrequency());
					}

					if ( traces[i].filter ) {
						GenericRecordPtr grec = new GenericRecord(s_rec->networkCode(),
						                                          s_rec->stationCode(),
						                                          s_rec->locationCode(),
						                                          s_rec->channelCode(),
						                                          s_rec->startTime(),
						                                          s_rec->samplingFrequency(),
						                                          s_rec->timingQuality());

						DoubleArrayPtr data = static_cast<DoubleArray*>(s_rec->data()->copy(Array::DOUBLE));
						traces[i].filter->apply(*data);
						grec->setData(data.get());
						s_rec = grec;
					}

					transformedRecord(i, s_rec.get());
					gotRecords = true;
				}
			}
		}
	}

	return gotRecords;
}


void ThreeComponentTrace::transformedRecord(int comp, const Record *rec) {
	traces[comp].transformed->feed(rec);
	if ( widget && !showProcessed ) widget->fed(traces[comp].recordSlot, rec);

	if ( label->processor ) {
		Processing::WaveformProcessor::StreamComponent c = label->processor->usedComponent();
		switch ( c ) {
			case Processing::WaveformProcessor::Vertical:
				if ( comp == 0 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::FirstHorizontal:
				if ( comp == 1 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::SecondHorizontal:
				if ( comp == 2 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::Horizontal:
				if ( comp == 1 || comp == 2 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::Any:
				label->processor->feed(rec);
				label->updateProcessingInfo();
				break;
			default:
				break;
		}

		if ( label->processor->isFinished() ) {
			for ( int i = 0; i < 3; ++i ) {
				const Processing::AmplitudeProcessor *compProc = label->processor->componentProcessor((Processing::WaveformProcessor::Component)i);
				if ( compProc == nullptr ) continue;
				const DoubleArray *processedData = compProc->processedData((Processing::WaveformProcessor::Component)i);
				if ( traces[i].processed == nullptr && processedData )
					setProcessedData(
						i, rec->networkCode(),
						rec->stationCode(),
						rec->locationCode(),
						compProc->dataTimeWindow().startTime(),
						compProc->samplingFrequency(),
						DoubleArray::Cast(processedData->copy(Array::DOUBLE))
					);
			}
		}
	}
}


AmplitudeRecordLabel::AmplitudeRecordLabel(int items, QWidget *parent, const char* name)
	: StandardRecordLabel(items, parent, name), _isLinkedItem(false), _isExpanded(false) {
	_btnExpand = nullptr;
	_linkedItem = nullptr;

	latitude = 999;
	longitude = 999;
	isError = false;
	data.label = this;

	hasGotData = false;
	isEnabledByConfig = false;
}

AmplitudeRecordLabel::~AmplitudeRecordLabel() {}

void AmplitudeRecordLabel::setLinkedItem(bool li) {
	_isLinkedItem = li;
}

void AmplitudeRecordLabel::setControlledItem(RecordViewItem *controlledItem) {
	_linkedItem = controlledItem;
	static_cast<AmplitudeRecordLabel*>(controlledItem->label())->_linkedItem = recordViewItem();
}

RecordViewItem *AmplitudeRecordLabel::controlledItem() const {
	return _linkedItem;
}

void AmplitudeRecordLabel::enabledExpandButton(RecordViewItem *controlledItem) {
	if ( _btnExpand ) return;

	_btnExpand = new QPushButton(this);
	_btnExpand->resize(16,16);
	_btnExpand->move(width() - _btnExpand->width(), height() - _btnExpand->height());
	_btnExpand->setIcon(QIcon(QString::fromUtf8(":/icons/icons/arrow_down.png")));
	_btnExpand->setFlat(true);
	_btnExpand->show();

	connect(_btnExpand, SIGNAL(clicked()),
	        this, SLOT(extentButtonPressed()));

	if ( !_linkedItem )
		setControlledItem(controlledItem);

	_isExpanded = false;
}

void AmplitudeRecordLabel::disableExpandButton() {
	if ( _btnExpand ) {
		delete _btnExpand;
		_btnExpand = nullptr;
	}

	_linkedItem = nullptr;
}

void AmplitudeRecordLabel::unlink() {
	if ( _linkedItem ) {
		static_cast<AmplitudeRecordLabel*>(_linkedItem->label())->disableExpandButton();
		_linkedItem = nullptr;
	}
}

bool AmplitudeRecordLabel::isLinkedItem() const {
	return _isLinkedItem;
}

bool AmplitudeRecordLabel::isExpanded() const {
	return _isExpanded;
}


void AmplitudeRecordLabel::visibilityChanged(bool v) {
	if ( _linkedItem && !_isLinkedItem ) {
		if ( !v )
			_linkedItem->setVisible(false);
		else if ( _isExpanded )
			_linkedItem->setVisible(true);
	}
}

void AmplitudeRecordLabel::resizeEvent(QResizeEvent *e) {
	StandardRecordLabel::resizeEvent(e);
	if ( _btnExpand ) {
		_btnExpand->move(e->size().width() - _btnExpand->width(),
		                 e->size().height() - _btnExpand->height());
	}
}

void AmplitudeRecordLabel::enableExpandable(const Seiscomp::Record *rec) {
	enabledExpandButton(static_cast<RecordViewItem*>(sender()));
}

void AmplitudeRecordLabel::extentButtonPressed() {
	_isExpanded = !_isExpanded;
	_btnExpand->setIcon(QIcon(QString::fromUtf8(_isExpanded?":/icons/icons/arrow_up.png":":/icons/icons/arrow_down.png")));
	if ( _linkedItem ) {
		if ( !_isExpanded ) {
			recordViewItem()->recordView()->setCurrentItem(recordViewItem());
			_linkedItem->recordView()->setItemSelected(_linkedItem, false);
		}
		_linkedItem->setVisible(_isExpanded);
	}
}

void AmplitudeRecordLabel::paintEvent(QPaintEvent *e) {
	QPainter p(this);

	if ( _hasLabelColor ) {
		QRect r(rect());

		r.setLeft(r.right()-16);

		QColor bg = palette().color(QPalette::Window);
		QLinearGradient gradient(r.left(), 0, r.right(), 0);
		gradient.setColorAt(0, bg);
		gradient.setColorAt(1, _labelColor);

		p.fillRect(r, gradient);
	}

	if ( _items.count() == 0 ) return;

	int fontSize = p.fontMetrics().ascent();

	int w = width();
	int h = height();

	int posX = 0;
	int posY = (h - fontSize * 2 - 4)/2;

	for ( int i = 0; i < _items.count()-1; ++i ) {
		if ( _items[i].text.isEmpty() ) continue;
		p.setFont(_items[i].font);
		p.setPen(
			isEnabled()
			?
			(_items[i].color.isValid() ? _items[i].color : palette().color(QPalette::Text))
			:
			palette().color(QPalette::Disabled, QPalette::Text)
		);

		p.drawText(posX,posY, w, fontSize, _items[i].align, _items[i].text);

		if ( _items[i].width < 0 )
			posX += p.fontMetrics().boundingRect(_items[i].text).width();
		else
			posX += _items[i].width;
	}

	posY += fontSize + 4;

	p.setPen(
		isEnabled()
		?
		(_items.last().color.isValid() ? _items.last().color : palette().color(QPalette::Text))
		:
		palette().color(QPalette::Disabled, QPalette::Text)
	);

	p.drawText(
		0, posY,
		_items.last().width < 0 ? w - 18 : std::min(_items.last().width, w - 18),
		fontSize, _items.last().align,
		_items.last().text
	);
}

void AmplitudeRecordLabel::setLabelColor(QColor c) {
	_labelColor = c;
	_hasLabelColor = true;
	update();
}

void AmplitudeRecordLabel::removeLabelColor() {
	_hasLabelColor = false;
	update();
}


void AmplitudeRecordLabel::updateProcessingInfo() {
	if ( !processor ) {
		infoText = QString();
		return;
	}

	switch ( processor->status() ) {
		case Processing::WaveformProcessor::WaitingForData:
			infoText = processor->status().toString();
			break;
		case Processing::WaveformProcessor::Finished:
			infoText = QString();
			break;
		case Processing::WaveformProcessor::Terminated:
			infoText = processor->status().toString();
			break;
		case Processing::WaveformProcessor::InProgress:
			infoText = QString("%1: %2%")
			           .arg(processor->status().toString())
			           .arg(processor->statusValue(),0,'f',1);
			break;
		case Processing::WaveformProcessor::LowSNR:
			infoText = QString("%1: %2 < %3")
			           .arg(processor->status().toString())
			           .arg(processor->statusValue(),0,'f',1)
			           .arg(processor->config().snrMin,0,'f',1);
			break;
		default:
			infoText = QString("%1(%2)")
			           .arg(processor->status().toString())
			           .arg(processor->statusValue(),0,'f',1);
			break;
	}

	isError = processor->status() > Processing::WaveformProcessor::Finished;
}



}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeView::Config::Config() {
	timingQualityLow = Qt::darkRed;
	timingQualityMedium = Qt::yellow;
	timingQualityHigh = Qt::darkGreen;
	ignoreDisabledStations = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeView::AmplitudeView(QWidget *parent, Qt::WindowFlags f)
: QMainWindow(parent,f)
, _d_ptr(new AmplitudeViewPrivate) {
	SC_D.recordView = new TraceList();
	SC_D.phases.append("P");
	SC_D.phases.append("S");
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeView::~AmplitudeView() {
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i )
		SC_D.recordView->itemAt(i)->widget()->setShadowWidget(nullptr, false);

	if ( SC_D.currentFilter ) delete SC_D.currentFilter;

	closeThreads();

	QList<int> sizes = SC_D.ui.splitter->sizes();

	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());

		SCApp->settings().setValue("geometry", saveGeometry());
		SCApp->settings().setValue("state", saveState());

		if ( sizes.count() >= 2 ) {
			SCApp->settings().setValue("splitter/upper", sizes[0]);
			SCApp->settings().setValue("splitter/lower", sizes[1]);
		}

		SCApp->settings().endGroup();
	}

	delete _d_ptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordLabel* AmplitudeView::createLabel(RecordViewItem *item) const {
	return new AmplitudeRecordLabel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::figureOutTravelTimeTable() {
	if ( !SC_D.origin ) return;

	int idx = SC_D.comboTTT->findText(SC_D.origin->methodID().c_str());
	if ( idx < 0 ) return;

	SC_D.ttTableName = SC_D.origin->earthModelID();
	SC_D.comboTTT->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::init() {
	setObjectName("Amplitudes");

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	SC_D.ui.setupUi(this);

	QFont f(font());
	f.setBold(true);
	SC_D.ui.labelStationCode->setFont(f);

	SC_D.settingsRestored = false;
	SC_D.currentSlot = -1;
	SC_D.currentFilter = nullptr;
	SC_D.currentFilterStr = "";
	SC_D.autoScaleZoomTrace = true;
	SC_D.showProcessedData = true;

	SC_D.reader = nullptr;

	SC_D.zoom = 1.0;
	SC_D.currentAmplScale = 1.0;

	SC_D.centerSelection = false;
	SC_D.checkVisibility = true;

	insertToolBarBreak(SC_D.ui.toolBarFilter);

	SC_D.recordView->setSelectionMode(RecordView::SingleSelection);
	SC_D.recordView->setMinimumRowHeight(fontMetrics().ascent()*2+6);
	SC_D.recordView->setDefaultRowHeight(fontMetrics().ascent()*2+6);
	SC_D.recordView->setSelectionEnabled(false);
	SC_D.recordView->setRecordUpdateInterval(1000);

	connect(SC_D.recordView, SIGNAL(currentItemChanged(RecordViewItem*, RecordViewItem*)),
	        this, SLOT(itemSelected(RecordViewItem*, RecordViewItem*)));

	connect(SC_D.recordView, SIGNAL(fedRecord(RecordViewItem*, const Seiscomp::Record*)),
	        this, SLOT(updateTraceInfo(RecordViewItem*, const Seiscomp::Record*)));

	connect(SC_D.recordView, SIGNAL(filterChanged(const QString&)),
	        this, SLOT(addNewFilter(const QString&)));

	connect(SC_D.recordView, SIGNAL(progressStarted()),
	        this, SLOT(beginWaitForRecords()));

	connect(SC_D.recordView, SIGNAL(progressChanged(int)),
	        this, SLOT(doWaitForRecords(int)));

	connect(SC_D.recordView, SIGNAL(progressFinished()),
	        this, SLOT(endWaitForRecords()));

	SC_D.recordView->setAlternatingRowColors(true);
	SC_D.recordView->setAutoInsertItem(false);
	SC_D.recordView->setAutoScale(true);
	SC_D.recordView->setRowSpacing(2);
	SC_D.recordView->setHorizontalSpacing(6);
	SC_D.recordView->setFramesEnabled(false);
	//SC_D.recordView->setDefaultActions();

	SC_D.recordView->timeWidget()->setSelectionHandleCount(4);
	SC_D.recordView->timeWidget()->setSelectionHandleEnabled(2, false);

	SC_D.connectionState = new ConnectionStateLabel(this);
	connect(SC_D.connectionState, SIGNAL(customInfoWidgetRequested(const QPoint &)),
	        this, SLOT(openConnectionInfo(const QPoint &)));

	QWidget *wrapper = new QWidget;
	wrapper->setBackgroundRole(QPalette::Base);
	wrapper->setAutoFillBackground(true);

	QBoxLayout* layout = new QVBoxLayout(SC_D.ui.frameTraces);
	layout->setMargin(2);
	layout->setSpacing(0);
	layout->addWidget(wrapper);

	layout = new QVBoxLayout(wrapper);
	layout->setMargin(SC_D.ui.frameZoom->layout()->margin());
	layout->setSpacing(6);
	layout->addWidget(SC_D.recordView);

	SC_D.searchStation = new QLineEdit();
	SC_D.searchStation->setVisible(false);

	SC_D.searchBase = SC_D.searchStation->palette().color(QPalette::Base);
	SC_D.searchError = blend(Qt::red, SC_D.searchBase, 50);

	SC_D.searchLabel = new QLabel();
	SC_D.searchLabel->setVisible(false);
	SC_D.searchLabel->setText(tr("Type the station code to search for"));

	connect(SC_D.searchStation, SIGNAL(textChanged(const QString&)),
	        this, SLOT(search(const QString&)));

	connect(SC_D.searchStation, SIGNAL(returnPressed()),
	        this, SLOT(nextSearch()));

	statusBar()->addPermanentWidget(SC_D.searchStation, 1);
	statusBar()->addPermanentWidget(SC_D.searchLabel, 5);
	statusBar()->addPermanentWidget(SC_D.connectionState);

	SC_D.currentRecord = new MyRecordWidget();
	SC_D.currentRecord->showScaledValues(SC_D.ui.actionShowTraceValuesInNmS->isChecked());
	SC_D.currentRecord->setClippingEnabled(SC_D.ui.actionClipComponentsToViewport->isChecked());
	SC_D.currentRecord->setMouseTracking(true);
	SC_D.currentRecord->setContextMenuPolicy(Qt::CustomContextMenu);
	SC_D.currentRecord->setRowSpacing(6);
	SC_D.currentRecord->setAxisSpacing(6);
	SC_D.currentRecord->setDrawAxis(true);
	SC_D.currentRecord->setDrawSPS(true);
	SC_D.currentRecord->setAxisPosition(RecordWidget::Left);

	//_currentRecord->setFocusPolicy(Qt::StrongFocus);

	//_currentRecord->setDrawMode(RecordWidget::Single);
	//_currentRecord->setDrawMode(RecordWidget::InRows);
	/*
	SC_D.currentRecord->setRecordColor(0, Qt::red);
	SC_D.currentRecord->setRecordColor(1, Qt::green);
	SC_D.currentRecord->setRecordColor(2, Qt::blue);
	*/

	/*
	connect(SC_D.currentRecord, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(openRecordContextMenu(const QPoint &)));
	*/

	layout = new QVBoxLayout(SC_D.ui.frameCurrentRow);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(SC_D.currentRecord);

	SC_D.timeScale = new TimeScale();
	SC_D.timeScale->setSelectionEnabled(true);
	SC_D.timeScale->setRangeSelectionEnabled(true);
	SC_D.timeScale->setAbsoluteTimeEnabled(true);
	SC_D.timeScale->setSelectionHandleCount(4);
	SC_D.timeScale->setSelectionHandleEnabled(2, false);

	layout = new QVBoxLayout(SC_D.ui.frameTimeScale);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(SC_D.timeScale);

	connect(SC_D.timeScale, SIGNAL(dragged(double)),
	        this, SLOT(move(double)));
	connect(SC_D.timeScale, SIGNAL(dragStarted()),
	        this, SLOT(disableAutoScale()));
	connect(SC_D.timeScale, SIGNAL(dragFinished()),
	        this, SLOT(enableAutoScale()));
	connect(SC_D.timeScale, SIGNAL(rangeChangeRequested(double,double)),
	        this, SLOT(applyTimeRange(double,double)));
	connect(SC_D.timeScale, SIGNAL(selectionHandleMoved(int,double,Qt::KeyboardModifiers)),
	        this, SLOT(zoomSelectionHandleMoved(int,double,Qt::KeyboardModifiers)));
	connect(SC_D.timeScale, SIGNAL(selectionHandleMoveFinished()),
	        this, SLOT(zoomSelectionHandleMoveFinished()));

	connect(SC_D.recordView->timeWidget(), SIGNAL(dragged(double)),
	        this, SLOT(moveTraces(double)));
	connect(SC_D.recordView->timeWidget(), SIGNAL(selectionHandleMoved(int,double,Qt::KeyboardModifiers)),
	        this, SLOT(selectionHandleMoved(int,double,Qt::KeyboardModifiers)));
	connect(SC_D.recordView->timeWidget(), SIGNAL(selectionHandleMoveFinished()),
	        this, SLOT(selectionHandleMoveFinished()));

	connect(SC_D.recordView, SIGNAL(updatedRecords()),
	        SC_D.currentRecord, SLOT(updateRecords()));

	// add actions
	addAction(SC_D.ui.actionIncreaseAmplitudeScale);
	addAction(SC_D.ui.actionDecreaseAmplitudeScale);
	addAction(SC_D.ui.actionTimeScaleUp);
	addAction(SC_D.ui.actionTimeScaleDown);
	addAction(SC_D.ui.actionClipComponentsToViewport);

	addAction(SC_D.ui.actionIncreaseRowHeight);
	addAction(SC_D.ui.actionDecreaseRowHeight);
	addAction(SC_D.ui.actionIncreaseRowTimescale);
	addAction(SC_D.ui.actionDecreaseRowTimescale);

	addAction(SC_D.ui.actionScrollLeft);
	addAction(SC_D.ui.actionScrollFineLeft);
	addAction(SC_D.ui.actionScrollRight);
	addAction(SC_D.ui.actionScrollFineRight);
	addAction(SC_D.ui.actionSelectNextTrace);
	addAction(SC_D.ui.actionSelectPreviousTrace);
	addAction(SC_D.ui.actionSelectFirstRow);
	addAction(SC_D.ui.actionSelectLastRow);

	addAction(SC_D.ui.actionDefaultView);

	addAction(SC_D.ui.actionSortAlphabetically);
	addAction(SC_D.ui.actionSortByDistance);

	addAction(SC_D.ui.actionShowZComponent);
	addAction(SC_D.ui.actionShowNComponent);
	addAction(SC_D.ui.actionShowEComponent);
	
	addAction(SC_D.ui.actionAlignOnOriginTime);
	addAction(SC_D.ui.actionAlignOnPArrival);

	addAction(SC_D.ui.actionToggleFilter);
	addAction(SC_D.ui.actionMaximizeAmplitudes);

	addAction(SC_D.ui.actionCreateAmplitude);
	addAction(SC_D.ui.actionSetAmplitude);
	addAction(SC_D.ui.actionConfirmAmplitude);
	addAction(SC_D.ui.actionDeleteAmplitude);

	addAction(SC_D.ui.actionShowZComponent);
	addAction(SC_D.ui.actionShowNComponent);
	addAction(SC_D.ui.actionShowEComponent);

	addAction(SC_D.ui.actionGotoNextMarker);
	addAction(SC_D.ui.actionGotoPreviousMarker);

	addAction(SC_D.ui.actionComputeMagnitudes);
	addAction(SC_D.ui.actionSwitchFullscreen);
	addAction(SC_D.ui.actionAddStations);
	addAction(SC_D.ui.actionSearchStation);

	addAction(SC_D.ui.actionRecalculateAmplitude);
	addAction(SC_D.ui.actionRecalculateAmplitudes);

	SC_D.lastFilterIndex = -1;

	SC_D.comboFilter = new QComboBox;
	//SC_D.comboFilter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	SC_D.comboFilter->setDuplicatesEnabled(false);
	SC_D.comboFilter->addItem(NO_FILTER_STRING);
	SC_D.comboFilter->addItem(DEFAULT_FILTER_STRING);

	SC_D.comboFilter->setCurrentIndex(1);
	changeFilter(SC_D.comboFilter->currentIndex());

	SC_D.spinSNR = new QDoubleSpinBox;
	SC_D.spinSNR->setRange(0, 10000);
	SC_D.spinSNR->setDecimals(2);
	SC_D.spinSNR->setSingleStep(1);
	//SC_D.spinSNR->setPrefix("Min. SNR ");
	SC_D.spinSNR->setSpecialValueText("Disabled");
	SC_D.checkOverrideSNR = new QCheckBox;
	SC_D.checkOverrideSNR->setToolTip(tr("Enable to override the minimum SNR"));
	SC_D.checkOverrideSNR->setChecked(false);
	SC_D.spinSNR->setEnabled(SC_D.checkOverrideSNR->isChecked());

	connect(SC_D.checkOverrideSNR, SIGNAL(toggled(bool)), SC_D.spinSNR, SLOT(setEnabled(bool)));

	SC_D.comboAmpType = new QComboBox;
	SC_D.comboAmpType->setEnabled(false);

	SC_D.comboAmpCombiner = new QComboBox;
	SC_D.comboAmpCombiner->setEnabled(false);

	connect(SC_D.ui.actionRecalculateAmplitude, SIGNAL(triggered()),
	        this, SLOT(recalculateAmplitude()));
	connect(SC_D.ui.actionRecalculateAmplitudes, SIGNAL(triggered()),
	        this, SLOT(recalculateAmplitudes()));

	SC_D.ui.toolBarFilter->insertWidget(SC_D.ui.actionToggleFilter, SC_D.comboFilter);
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, SC_D.checkOverrideSNR);
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, new QLabel("Min SNR:"));
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, SC_D.spinSNR);
	SC_D.ui.toolBarSetup->insertSeparator(SC_D.ui.actionRecalculateAmplitude);
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, SC_D.labelAmpType = new QLabel("Amp.type:"));
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, SC_D.comboAmpType);
	SC_D.ui.toolBarSetup->insertSeparator(SC_D.ui.actionRecalculateAmplitude);
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, SC_D.labelAmpCombiner = new QLabel("Amp.combiner:"));
	SC_D.ui.toolBarSetup->insertWidget(SC_D.ui.actionRecalculateAmplitude, SC_D.comboAmpCombiner);

	// TTT selection
	SC_D.comboTTT = new QComboBox;
	SC_D.ui.toolBarTTT->addWidget(SC_D.comboTTT);

	SC_D.comboTTT->setToolTip(tr("Select one of the supported travel time table backends."));
	TravelTimeTableInterfaceFactory::ServiceNames *ttServices = TravelTimeTableInterfaceFactory::Services();
	if ( ttServices ) {
		TravelTimeTableInterfaceFactory::ServiceNames::iterator it;
		int currentIndex = -1;
		for ( it = ttServices->begin(); it != ttServices->end(); ++it ) {
			SC_D.comboTTT->addItem((*it).c_str());
			if ( SC_D.ttInterface == *it )
				currentIndex = SC_D.comboTTT->count()-1;
		}
		delete ttServices;

		if ( currentIndex >= 0 )
			SC_D.comboTTT->setCurrentIndex(currentIndex);
	}

	if ( SC_D.comboTTT->count() > 0 ) {
		connect(SC_D.comboTTT, SIGNAL(currentIndexChanged(QString)), this, SLOT(ttInterfaceChanged(QString)));
		SC_D.comboTTTables = new QComboBox;
		SC_D.comboTTTables->setToolTip(tr("Select one of the supported tables for the current travel time table backend."));
		SC_D.ui.toolBarTTT->addWidget(SC_D.comboTTTables);
		ttInterfaceChanged(SC_D.comboTTT->currentText());
		connect(SC_D.comboTTTables, SIGNAL(currentIndexChanged(QString)), this, SLOT(ttTableChanged(QString)));
	}
	else {
		delete SC_D.comboTTT;
		SC_D.comboTTT = nullptr;
	}

	SC_D.labelAmpType->setEnabled(false);
	SC_D.labelAmpCombiner->setEnabled(false);

	connect(SC_D.comboFilter, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeFilter(int)));

	connect(SC_D.ui.actionLimitFilterToZoomTrace, SIGNAL(triggered(bool)),
	        this, SLOT(limitFilterToZoomTrace(bool)));

	SC_D.spinDistance = new QDoubleSpinBox;
	SC_D.spinDistance->setValue(15);

	if ( SCScheme.unit.distanceInKM ) {
		SC_D.spinDistance->setRange(0, 25000);
		SC_D.spinDistance->setDecimals(0);
		SC_D.spinDistance->setSuffix("km");
	}
	else {
		SC_D.spinDistance->setRange(0, 180);
		SC_D.spinDistance->setDecimals(1);
		SC_D.spinDistance->setSuffix(degrees);
	}

	SC_D.ui.toolBarStations->insertWidget(SC_D.ui.actionShowAllStations, SC_D.spinDistance);

	/*
	connect(SC_D.spinDistance, SIGNAL(editingFinished()),
	        this, SLOT(loadNextStations()));
	*/

	// connect actions
	connect(SC_D.ui.actionDefaultView, SIGNAL(triggered(bool)),
	        this, SLOT(setDefaultDisplay()));
	connect(SC_D.ui.actionSortAlphabetically, SIGNAL(triggered(bool)),
	        this, SLOT(sortAlphabetically()));
	connect(SC_D.ui.actionSortByDistance, SIGNAL(triggered(bool)),
	        this, SLOT(sortByDistance()));

	connect(SC_D.ui.actionShowZComponent, SIGNAL(triggered(bool)),
	        this, SLOT(showZComponent()));
	connect(SC_D.ui.actionShowNComponent, SIGNAL(triggered(bool)),
	        this, SLOT(showNComponent()));
	connect(SC_D.ui.actionShowEComponent, SIGNAL(triggered(bool)),
	        this, SLOT(showEComponent()));
	
	connect(SC_D.ui.actionAlignOnOriginTime, SIGNAL(triggered(bool)),
	        this, SLOT(alignOnOriginTime()));
	connect(SC_D.ui.actionAlignOnPArrival, SIGNAL(triggered(bool)),
	        this, SLOT(alignOnPArrivals()));

	connect(SC_D.ui.actionIncreaseAmplitudeScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleAmplUp()));
	connect(SC_D.ui.actionDecreaseAmplitudeScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleAmplDown()));
	connect(SC_D.ui.actionTimeScaleUp, SIGNAL(triggered(bool)),
	        this, SLOT(scaleTimeUp()));
	connect(SC_D.ui.actionTimeScaleDown, SIGNAL(triggered(bool)),
	        this, SLOT(scaleTimeDown()));
	connect(SC_D.ui.actionClipComponentsToViewport, SIGNAL(triggered(bool)),
	        SC_D.currentRecord, SLOT(setClippingEnabled(bool)));
	connect(SC_D.ui.actionScrollLeft, SIGNAL(triggered(bool)),
	        this, SLOT(scrollLeft()));
	connect(SC_D.ui.actionScrollFineLeft, SIGNAL(triggered(bool)),
	        this, SLOT(scrollFineLeft()));
	connect(SC_D.ui.actionScrollRight, SIGNAL(triggered(bool)),
	        this, SLOT(scrollRight()));
	connect(SC_D.ui.actionScrollFineRight, SIGNAL(triggered(bool)),
	        this, SLOT(scrollFineRight()));
	/*
	connect(SC_D.ui.actionGotoNextMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoNextMarker()));
	connect(SC_D.ui.actionGotoPreviousMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoPreviousMarker()));
	*/
	connect(SC_D.ui.actionSelectNextTrace, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(selectNextRow()));
	connect(SC_D.ui.actionSelectPreviousTrace, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(selectPreviousRow()));
	connect(SC_D.ui.actionSelectFirstRow, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(selectFirstRow()));
	connect(SC_D.ui.actionSelectLastRow, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(selectLastRow()));
	connect(SC_D.ui.actionIncreaseRowHeight, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(verticalZoomIn()));
	connect(SC_D.ui.actionDecreaseRowHeight, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(verticalZoomOut()));
	connect(SC_D.ui.actionIncreaseRowTimescale, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(horizontalZoomIn()));
	connect(SC_D.ui.actionDecreaseRowTimescale, SIGNAL(triggered(bool)),
	        SC_D.recordView, SLOT(horizontalZoomOut()));
	connect(SC_D.ui.actionShowTraceValuesInNmS, SIGNAL(triggered(bool)),
	        this, SLOT(showTraceScaleToggled(bool)));
	connect(SC_D.ui.actionShowTheoreticalArrivals, SIGNAL(triggered(bool)),
	        this, SLOT(showTheoreticalArrivals(bool)));

	connect(SC_D.ui.actionToggleFilter, SIGNAL(triggered(bool)),
	        this, SLOT(toggleFilter()));

	connect(SC_D.ui.actionMaximizeAmplitudes, SIGNAL(triggered(bool)),
	        this, SLOT(scaleVisibleAmplitudes()));

	connect(SC_D.ui.actionPickAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(pickAmplitudes(bool)));
	connect(SC_D.ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(pickNone(bool)));
	connect(SC_D.ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(abortSearchStation()));

	connect(SC_D.ui.actionCreateAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(createAmplitude()));
	connect(SC_D.ui.actionSetAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(setAmplitude()));
	connect(SC_D.ui.actionConfirmAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(confirmAmplitude()));
	connect(SC_D.ui.actionDeleteAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(deleteAmplitude()));

	connect(SC_D.ui.actionComputeMagnitudes, SIGNAL(triggered(bool)),
	        this, SLOT(commit()));

	connect(SC_D.ui.actionShowAllStations, SIGNAL(triggered(bool)),
	        this, SLOT(loadNextStations()));

	connect(SC_D.ui.actionShowUsedStations, SIGNAL(triggered(bool)),
	        this, SLOT(showUsedStations(bool)));

	/*
	connect(SC_D.ui.btnAmplScaleUp, SIGNAL(clicked()),
	        this, SLOT(scaleAmplUp()));
	connect(SC_D.ui.btnAmplScaleDown, SIGNAL(clicked()),
	        this, SLOT(scaleAmplDown()));
	connect(SC_D.ui.btnTimeScaleUp, SIGNAL(clicked()),
	        this, SLOT(scaleTimeUp()));
	connect(SC_D.ui.btnTimeScaleDown, SIGNAL(clicked()),
	        this, SLOT(scaleTimeDown()));
	connect(SC_D.ui.btnScaleReset, SIGNAL(clicked()),
	        this, SLOT(scaleReset()));
	*/

	connect(SC_D.ui.btnRowAccept, SIGNAL(clicked()),
	        this, SLOT(confirmAmplitude()));
	connect(SC_D.ui.btnRowRemove, SIGNAL(clicked(bool)),
	        this, SLOT(setCurrentRowDisabled(bool)));
	connect(SC_D.ui.btnRowRemove, SIGNAL(clicked(bool)),
	        SC_D.recordView, SLOT(selectNextRow()));
	connect(SC_D.ui.btnRowReset, SIGNAL(clicked(bool)),
	        this, SLOT(deleteAmplitude()));
	connect(SC_D.ui.btnRowReset, SIGNAL(clicked(bool)),
	        SC_D.recordView, SLOT(selectNextRow()));

	connect(SC_D.currentRecord, SIGNAL(cursorUpdated(RecordWidget*,int)),
	        this, SLOT(updateSubCursor(RecordWidget*,int)));

	connect(SC_D.currentRecord, SIGNAL(clickedOnTime(Seiscomp::Core::Time)),
	        this, SLOT(updateRecordValue(Seiscomp::Core::Time)));

	connect(SC_D.ui.frameZoom, SIGNAL(lineDown()),
	        SC_D.recordView, SLOT(selectNextRow()));
	connect(SC_D.ui.frameZoom, SIGNAL(lineUp()),
	        SC_D.recordView, SLOT(selectPreviousRow()));

	connect(SC_D.ui.frameZoom, SIGNAL(verticalZoomIn()),
	        this, SLOT(scaleAmplUp()));
	connect(SC_D.ui.frameZoom, SIGNAL(verticalZoomOut()),
	        this, SLOT(scaleAmplDown()));

	connect(SC_D.ui.frameZoom, SIGNAL(horizontalZoomIn()),
	        this, SLOT(scaleTimeUp()));
	connect(SC_D.ui.frameZoom, SIGNAL(horizontalZoomOut()),
	        this, SLOT(scaleTimeDown()));

	connect(SC_D.ui.actionSwitchFullscreen, SIGNAL(triggered(bool)),
	        this, SLOT(showFullscreen(bool)));

	connect(SC_D.timeScale, SIGNAL(changedInterval(double, double, double)),
	        SC_D.currentRecord, SLOT(setGridSpacing(double, double, double)));
	connect(SC_D.recordView, SIGNAL(toggledFilter(bool)),
	        SC_D.currentRecord, SLOT(enableFiltering(bool)));
	connect(SC_D.recordView, SIGNAL(scaleChanged(double, double)),
	        this, SLOT(changeScale(double, double)));
	connect(SC_D.recordView, SIGNAL(timeRangeChanged(double, double)),
	        this, SLOT(changeTimeRange(double, double)));
	connect(SC_D.recordView, SIGNAL(selectionChanged(double, double)),
	        SC_D.currentRecord, SLOT(setSelected(double, double)));
	connect(SC_D.recordView, SIGNAL(alignmentChanged(const Seiscomp::Core::Time&)),
	        this, SLOT(setAlignment(Seiscomp::Core::Time)));
	connect(SC_D.recordView, SIGNAL(amplScaleChanged(double)),
	        SC_D.currentRecord, SLOT(setAmplScale(double)));

	connect(SC_D.ui.actionAddStations, SIGNAL(triggered(bool)),
	        this, SLOT(addStations()));

	connect(SC_D.ui.actionSearchStation, SIGNAL(triggered(bool)),
	        this, SLOT(searchStation()));

	connect(SC_D.recordView, SIGNAL(selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)));

	connect(SC_D.currentRecord, SIGNAL(selectedTime(Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Core::Time)));

	connect(SC_D.currentRecord, SIGNAL(selectedTimeRangeChanged(Seiscomp::Core::Time, Seiscomp::Core::Time)),
	        this, SLOT(onChangingTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time)));
	connect(SC_D.currentRecord, SIGNAL(selectedTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time)));

	connect(SC_D.recordView, SIGNAL(addedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)),
	        this, SLOT(onAddedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)));

	connect(&RecordStreamState::Instance(), SIGNAL(firstConnectionEstablished()),
	        this, SLOT(firstConnectionEstablished()));
	connect(&RecordStreamState::Instance(), SIGNAL(lastConnectionClosed()),
	        this, SLOT(lastConnectionClosed()));

	SC_D.ui.frameZoom->setBackgroundRole(QPalette::Base);
	SC_D.ui.frameZoom->setAutoFillBackground(true);

	if ( RecordStreamState::Instance().connectionCount() )
		firstConnectionEstablished();
	else
		lastConnectionClosed();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::setConfig(const Config &c, QString *error) {
	SC_D.config = c;

	if ( SCScheme.unit.distanceInKM )
		SC_D.spinDistance->setValue(Math::Geo::deg2km(SC_D.config.defaultAddStationsDistance));
	else
		SC_D.spinDistance->setValue(SC_D.config.defaultAddStationsDistance);

	//_config.filters.append(Config::FilterEntry("4 pole HP @2s", "BW_HP(4,0.5)"));

	if ( SC_D.comboFilter ) {
		SC_D.comboFilter->blockSignals(true);
		SC_D.comboFilter->clear();
		SC_D.comboFilter->addItem(NO_FILTER_STRING);
		SC_D.comboFilter->addItem(DEFAULT_FILTER_STRING);

		SC_D.lastFilterIndex = -1;

		int defaultIndex = -1;
		for ( int i = 0; i < SC_D.config.filters.count(); ++i ) {
			if ( SC_D.config.filters[i].first.isEmpty() ) continue;

			if ( SC_D.config.filters[i].first[0] == '@' ) {
				if ( defaultIndex == -1 ) defaultIndex = SC_D.comboFilter->count();
				addFilter(SC_D.config.filters[i].first.mid(1), SC_D.config.filters[i].second);
			}
			else
				addFilter(SC_D.config.filters[i].first, SC_D.config.filters[i].second);
		}

		SC_D.comboFilter->blockSignals(false);

		SC_D.comboFilter->setCurrentIndex(defaultIndex != -1?defaultIndex:1);
	}

	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( item && SC_D.currentRecord ) {
		if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
			if ( SC_D.config.showAllComponents &&
				SC_D.config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
				SC_D.currentRecord->setDrawMode(RecordWidget::InRows);
			else
				SC_D.currentRecord->setDrawMode(RecordWidget::Single);
		}
		else
			SC_D.currentRecord->setDrawMode(RecordWidget::Single);
	}

	if ( SC_D.config.hideStationsWithoutData ) {
		bool reselectCurrentItem = false;

		for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
			RecordViewItem* item = SC_D.recordView->itemAt(r);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
			if ( isLinkedItem(item) ) continue;

			if ( !isTracePicked(item->widget()) && !label->hasGotData ) {
				item->forceInvisibilty(true);
				if ( item == SC_D.recordView->currentItem() )
					reselectCurrentItem = true;
			}
		}

		if ( SC_D.recordView->currentItem() == nullptr ) reselectCurrentItem = true;

		if ( reselectCurrentItem )
			selectFirstVisibleItem(SC_D.recordView);
	}
	else {
		for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
			RecordViewItem* item = SC_D.recordView->itemAt(r);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
			if ( isLinkedItem(item) ) continue;

			if ( !isTracePicked(item->widget()) && !label->hasGotData )
				item->forceInvisibilty(!label->isEnabledByConfig);
		}
	}

	acquireStreams();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setDatabase(Seiscomp::DataModel::DatabaseQuery* reader) {
	SC_D.reader = reader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setStrongMotionCodes(const std::vector<std::string> &codes) {
	SC_D.strongMotionCodes = codes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &AmplitudeView::currentMagnitudeType() const {
	return SC_D.magnitudeType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showEvent(QShowEvent *e) {
	// avoid truncated distance labels
	int w1 = SC_D.ui.frameZoomControls->sizeHint().width();
	int w2 = 0;
	QFont f(SC_D.ui.labelDistance->font()); // hack to get default font size
	QFontMetrics fm(f);
	w2 += fm.boundingRect("WW ").width();
	f.setBold(true);
	w2 += fm.boundingRect("WWWWW 100").width();

	if ( SCScheme.unit.distanceInKM )
		w2 = std::max(w2, fm.boundingRect(QString("%1 km").arg(299999.0/3.0, 0, 'f', SCScheme.precision.distance)).width());
	else
		w2 = std::max(w2, fm.boundingRect(QString("155.5%1").arg(degrees)).width());

	if ( w2 < w1 )
		w2 = w1;

	if ( !SC_D.settingsRestored ) {
		QList<int> sizes;

		if ( SCApp ) {
			SCApp->settings().beginGroup(objectName());
			restoreGeometry(SCApp->settings().value("geometry").toByteArray());
			restoreState(SCApp->settings().value("state").toByteArray());

#ifdef MACOSX
			Mac::addFullscreen(this);
#endif

			QVariant splitterUpperSize = SCApp->settings().value("splitter/upper");
			QVariant splitterLowerSize = SCApp->settings().value("splitter/lower");

			if ( !splitterUpperSize.isValid() || !splitterLowerSize.isValid() ) {
				sizes.append(200);
				sizes.append(400);
			}
			else {
				sizes.append(splitterUpperSize.toInt());
				sizes.append(splitterLowerSize.toInt());
			}

			SCApp->settings().endGroup();
		}
		else {
			sizes.append(200);
			sizes.append(400);
		}

		SC_D.ui.splitter->setSizes(sizes);

		SC_D.settingsRestored = true;
	}

	SC_D.ui.frameZoomControls->setFixedWidth(w2);
	SC_D.recordView->setLabelWidth(w2);
	SC_D.currentRecord->setAxisWidth(w2 + SC_D.currentRecord->axisSpacing());

	QWidget::showEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onSelectedTime(Seiscomp::Core::Time time) {
	//setPhaseMarker(SC_D.currentRecord, time);
	if ( SC_D.recordView->currentItem() ) {
		setPhaseMarker(SC_D.recordView->currentItem()->widget(), time);
		SC_D.currentRecord->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onChangingTimeRange(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2) {
	static_cast<MyRecordWidget*>(SC_D.currentRecord)->setSelected(t1, t2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onSelectedTimeRange(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2) {
	static_cast<MyRecordWidget*>(SC_D.currentRecord)->setSelected(Core::Time(), Core::Time());

	RecordViewItem *item = SC_D.recordView->currentItem();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	if ( t1 == t2 ) return;

	if ( SC_D.currentSlot < 0 ) return;
	if ( label->processor == nullptr ) return;

	double smin = t1-label->processor->trigger();
	double smax = t2-label->processor->trigger();

	if ( SC_D.checkOverrideSNR->isChecked() )
		label->processor->setMinSNR(SC_D.spinSNR->value());
	else
		label->processor->setMinSNR(label->initialMinSNR);

	if ( SC_D.comboAmpType->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, SC_D.comboAmpType->currentText().toStdString());
	if ( SC_D.comboAmpCombiner->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::Combiner, SC_D.comboAmpCombiner->currentText().toStdString());

	label->processor->setPublishFunction(bind(&AmplitudeView::newAmplitudeAvailable, this, placeholders::_1, placeholders::_2));
	label->processor->reprocess(smin, smax);
	label->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
	label->updateProcessingInfo();

	QString statusText = label->processor->status().toString();

	switch ( label->processor->status() ) {
		case Processing::WaveformProcessor::WaitingForData:
		case Processing::WaveformProcessor::Finished:
		case Processing::WaveformProcessor::Terminated:
			break;
		case Processing::WaveformProcessor::InProgress:
			statusText += QString(": %1%")
			              .arg(label->processor->statusValue(),0,'f',1);
			break;
		case Processing::WaveformProcessor::LowSNR:
			statusText += QString(": %1 < %2")
			              .arg(label->processor->statusValue(),0,'f',1)
			              .arg(label->processor->config().snrMin,0,'f',1);
			break;
		default:
			statusText += QString("(%1)")
			              .arg(label->processor->statusValue(),0,'f',1);
			break;
	}

	statusBar()->showMessage(statusText);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onSelectedTime(Seiscomp::Gui::RecordWidget* widget,
                                   Seiscomp::Core::Time time) {
	if ( widget == SC_D.currentRecord ) return;
	setPhaseMarker(widget, time);
	//setPhaseMarker(SC_D.currentRecord, time);
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setPhaseMarker(Seiscomp::Gui::RecordWidget* widget,
                                   const Seiscomp::Core::Time& time) {
	if ( widget != SC_D.recordView->currentItem()->widget() ) return;
	if ( widget->cursorText().isEmpty() ) return;

	RecordViewItem *item = SC_D.recordView->currentItem();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	if ( SC_D.currentSlot < 0 ) return;
	if ( label->processor == nullptr ) return;

	double smin = double(time-label->processor->trigger())-0.5;
	double smax = smin+1.0;

	if ( SC_D.checkOverrideSNR->isChecked() )
		label->processor->setMinSNR(SC_D.spinSNR->value());
	else
		label->processor->setMinSNR(label->initialMinSNR);

	if ( SC_D.comboAmpType->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, SC_D.comboAmpType->currentText().toStdString());
	if ( SC_D.comboAmpCombiner->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::Combiner, SC_D.comboAmpCombiner->currentText().toStdString());

	label->processor->setPublishFunction(bind(&AmplitudeView::newAmplitudeAvailable, this, placeholders::_1, placeholders::_2));
	label->processor->reprocess(smin, smax);
	label->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
	label->updateProcessingInfo();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* AmplitudeView::updatePhaseMarker(Seiscomp::Gui::RecordViewItem *item,
                                               const Processing::AmplitudeProcessor *proc,
                                               const Processing::AmplitudeProcessor::Result &res) {
	RecordWidget *widget = item->widget();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	OPT(double) mag;
	QString magError;

	int slot = -1;
	if ( res.component < Processing::WaveformProcessor::Horizontal )
		slot = SC_D.componentMap[res.component];

	// Create amplitude
	WaveformStreamID s = item->streamID();
	AmplitudePtr a = Amplitude::Create();

	if ( res.component <= Processing::WaveformProcessor::SecondHorizontal )
		a->setWaveformID(
			WaveformStreamID(
				s.networkCode(), s.stationCode(),
				s.locationCode(), label->data.traces[res.component].channelCode, ""
			)
		);
	else
		a->setWaveformID(
			WaveformStreamID(
				s.networkCode(), s.stationCode(),
				s.locationCode(), s.channelCode().substr(0,2), ""
			)
		);

	a->setAmplitude(
		RealQuantity(res.amplitude.value, Core::None,
		             res.amplitude.lowerUncertainty,
		             res.amplitude.upperUncertainty, Core::None)
	);

	if ( res.period > 0 ) a->setPeriod(RealQuantity(res.period));
	if ( res.snr >= 0 ) a->setSnr(res.snr);
	a->setType(label->processor->type());
	a->setUnit(label->processor->unit());
	a->setTimeWindow(
		TimeWindow(res.time.reference, res.time.begin, res.time.end)
	);
	a->setPickID(label->processor->referencingPickID());
	a->setFilterID(label->data.filterID);
	a->setEvaluationMode(EvaluationMode(MANUAL));

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());
	a->setCreationInfo(ci);

	proc->finalizeAmplitude(a.get());

	if ( label->magnitudeProcessor ) {
		double m;
		Processing::MagnitudeProcessor::Status status;
		status = label->magnitudeProcessor->computeMagnitude(
			res.amplitude.value, label->processor->unit(),
			res.period, res.snr, item->value(ITEM_DISTANCE_INDEX),
			SC_D.origin->depth(), SC_D.origin.get(), label->location, a.get(), m);
		if ( status == Processing::MagnitudeProcessor::OK )
			mag = m;
		else {
			if ( label->magnitudeProcessor->treatAsValidMagnitude() )
				mag = m;
			magError = status.toString();
		}
	}

	AmplitudeViewMarker *marker = static_cast<AmplitudeViewMarker*>(widget->marker(proc->type().c_str(), true));
	// Marker found?
	if ( marker ) {
		// Set the marker time to the new picked time
		marker->setCorrectedTime(res.time.reference);
		// and set its component to the currently displayed component
		marker->setSlot(slot);
		//marker->setWidth(res.amplitudeLeftWidth, res.amplitudeRightWidth);
		marker->setTimeWindow(res.time.begin, res.time.end);
		marker->setMagnitude(mag, magError);
		marker->setAmplitudeResult(a.get());
		marker->setFilterID(label->data.filterID);

		widget->update();
	}
	else {
		// Valid phase code?
		if ( !proc->type().empty() ) {
			// Create a new marker for the phase
			marker = new AmplitudeViewMarker(widget, res.time.reference, proc->type().c_str(),
			                                 AmplitudeViewMarker::Amplitude, true);
			marker->setSlot(slot);
			//marker->setWidth(res.amplitudeLeftWidth, res.amplitudeRightWidth);
			marker->setTimeWindow(res.time.begin, res.time.end);
			marker->setMagnitude(mag, magError);
			marker->setAmplitudeResult(a.get());
			marker->setFilterID(label->data.filterID);
			marker->setEnabled(true);
	
			for ( int i = 0; i < widget->markerCount(); ++i ) {
				RecordMarker* marker2 = widget->marker(i);
				if ( marker == marker2 ) {
					continue;
				}

				if ( marker2->text() == marker->text() && !marker2->isMovable() ) {
					marker2->setEnabled(false);
				}
			}

			widget->update();
		}
	}

	return marker;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onAddedItem(const Record* rec, RecordViewItem* item) {
	// NOTE: Dynamic item insertion is not yet used
	/*
	setupItem(item);
	addTheoreticalArrivals(item, rec->networkCode(), rec->stationCode());
	sortByDistance();
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCursorText(const QString& text) {
	SC_D.recordView->setCursorText(text);
	SC_D.currentRecord->setCursorText(text);
	SC_D.currentRecord->setActive(text != "");

	if ( SC_D.currentRecord->isActive() ) {
		//_centerSelection = true;
		RecordMarker* m = SC_D.currentRecord->marker(text);
		if ( m )
			setCursorPos(m->correctedTime());
		else if ( SC_D.recordView->currentItem() )
			setCursorPos(SC_D.recordView->currentItem()->widget()->visibleTimeWindow().startTime() +
			             Core::TimeSpan(SC_D.recordView->currentItem()->widget()->visibleTimeWindow().length()*0.5));
	}

	updateCurrentRowState();
	componentByState();

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::loadNextStations() {
	float distance = SC_D.spinDistance->value();

	if ( SCScheme.unit.distanceInKM )
		distance = Math::Geo::km2deg(distance);

	std::vector<Seiscomp::DataModel::WaveformStreamID>::iterator it;

	SC_D.recordView->setUpdatesEnabled(false);

	loadNextStations(distance);

	sortByState();
	alignByState();
	componentByState();

	// Load all required components
	for ( int i = 0; i < 3; ++i )
		if ( SC_D.componentMap[i] >= 0 )
			fetchComponent(COMPS[i]);

	if ( SC_D.recordView->currentItem() == nullptr ) {
		selectFirstVisibleItem(SC_D.recordView);
	}
	setCursorText(SC_D.currentRecord->cursorText());

	SC_D.recordView->setUpdatesEnabled(true);
	SC_D.recordView->setFocus();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::sortByState() {
	if ( SC_D.ui.actionSortByDistance->isChecked() )
		sortByDistance();
	else if ( SC_D.ui.actionSortAlphabetically->isChecked() )
		sortAlphabetically();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::alignByState() {
	if ( SC_D.ui.actionAlignOnPArrival->isChecked() )
		alignOnPArrivals();
	else if ( SC_D.ui.actionAlignOnOriginTime->isChecked() )
		alignOnOriginTime();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::componentByState() {
	if ( SC_D.ui.actionShowZComponent->isChecked() )
		showComponent('Z');
	else if ( SC_D.ui.actionShowNComponent->isChecked() )
		showComponent('1');
	else if ( SC_D.ui.actionShowEComponent->isChecked() )
		showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::resetState() {
	for ( int i = 0; i < 3; ++i )
		if ( SC_D.componentMap[i] >= 0 )
			showComponent(COMPS[i]);
	//showComponent('Z');
	//alignOnOriginTime();
	alignOnPArrivals();
	pickNone(true);
	sortByDistance();
	SC_D.ui.actionShowUsedStations->setChecked(false);
	showUsedStations(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateLayoutFromState() {
	sortByState();
	alignByState();
	componentByState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::firstConnectionEstablished() {
	SC_D.connectionState->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::lastConnectionClosed() {
	SC_D.connectionState->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::beginWaitForRecords() {
	qApp->setOverrideCursor(Qt::WaitCursor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::doWaitForRecords(int value) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::endWaitForRecords() {
	qApp->restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showFullscreen(bool e) {
	if ( e )
		showFullScreen();
	else {
		showNormal();
#ifdef MACOSX
		Mac::addFullscreen(this);
#endif
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::recalculateAmplitude() {
	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( item == nullptr ) return;

	AmplitudeRecordLabel *l = static_cast<AmplitudeRecordLabel*>(item->label());
	if ( !l->processor ) return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if ( SC_D.checkOverrideSNR->isChecked() )
		l->processor->setMinSNR(SC_D.spinSNR->value());
	else
		l->processor->setMinSNR(l->initialMinSNR);

	resetAmplitude(item, SC_D.amplitudeType.c_str(), false);

	if ( SC_D.comboAmpType->isEnabled() )
		l->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, SC_D.comboAmpType->currentText().toStdString());
	if ( SC_D.comboAmpCombiner->isEnabled() )
		l->processor->setParameter(Processing::AmplitudeProcessor::Combiner, SC_D.comboAmpCombiner->currentText().toStdString());

	if ( l->processor->isFinished() ) {
		l->processor->setPublishFunction(bind(&AmplitudeView::newAmplitudeAvailable, this, placeholders::_1, placeholders::_2));
		l->processor->reprocess();

		for ( int i = 0; i < 3; ++i ) {
			const Processing::AmplitudeProcessor *compProc = l->processor->componentProcessor((Processing::WaveformProcessor::Component)i);
			if ( compProc == nullptr ) continue;

			const DoubleArray *processedData = compProc->processedData((Processing::WaveformProcessor::Component)i);

			if ( processedData ) {
				l->data.setProcessedData(
					i, item->streamID().networkCode(),
					item->streamID().stationCode(),
					item->streamID().locationCode(),
					compProc->dataTimeWindow().startTime(),
					compProc->samplingFrequency(),
					DoubleArray::Cast(processedData->copy(Array::DOUBLE))
				);

				//l->processor->writeData();
			}
			else
				l->data.removeProcessedData(i);
		}

		l->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
	}

	l->updateProcessingInfo();
	//l->setEnabled(!l->isError);

	SC_D.currentRecord->update();
	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::recalculateAmplitudes() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem *item = SC_D.recordView->itemAt(r);
		AmplitudeRecordLabel *l = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( !l->processor ) continue;

		// NOTE: Ignore items that are children of other items and not
		//       expanded (eg SM channels)
		if ( l->isLinkedItem() ) {
			AmplitudeRecordLabel *controllerLabel = static_cast<AmplitudeRecordLabel*>(l->controlledItem()->label());
			if ( !controllerLabel->isExpanded() ) continue;
		}

		if ( SC_D.checkOverrideSNR->isChecked() )
			l->processor->setMinSNR(SC_D.spinSNR->value());
		else
			l->processor->setMinSNR(l->initialMinSNR);

		resetAmplitude(item, SC_D.amplitudeType.c_str(), false);

		if ( SC_D.comboAmpType->isEnabled() )
			l->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, SC_D.comboAmpType->currentText().toStdString());
		if ( SC_D.comboAmpCombiner->isEnabled() )
			l->processor->setParameter(Processing::AmplitudeProcessor::Combiner, SC_D.comboAmpCombiner->currentText().toStdString());

		if ( l->processor->isFinished() ) {
			l->processor->setPublishFunction(bind(&AmplitudeView::newAmplitudeAvailable, this, placeholders::_1, placeholders::_2));
			l->processor->reprocess();

			for ( int i = 0; i < 3; ++i ) {
				const Processing::AmplitudeProcessor *compProc = l->processor->componentProcessor((Processing::WaveformProcessor::Component)i);
				if ( compProc == nullptr ) continue;

				const DoubleArray *processedData = compProc->processedData((Processing::WaveformProcessor::Component)i);

				if ( processedData ) {
					l->data.setProcessedData(
						i, item->streamID().networkCode(),
						item->streamID().stationCode(),
						item->streamID().locationCode(),
						compProc->dataTimeWindow().startTime(),
						compProc->samplingFrequency(),
						DoubleArray::Cast(processedData->copy(Array::DOUBLE))
					);

					//l->processor->writeData();
				}
				else
					l->data.removeProcessedData(i);
			}

			l->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
		}

		l->updateProcessingInfo();
		//l->setEnabled(!l->isError);
	}

	SC_D.currentRecord->update();
	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::newAmplitudeAvailable(const Processing::AmplitudeProcessor *proc,
                                          const Processing::AmplitudeProcessor::Result &res) {
	std::string streamID = res.record->streamID();
	auto it = SC_D.recordItemLabels.find(streamID);
	if ( it == SC_D.recordItemLabels.end() )
		return;

	AmplitudeRecordLabel *label = it->second;
	RecordViewItem *item = label->recordViewItem();

	if ( label->processor.get() != proc ) return;
	if ( proc->type() != SC_D.amplitudeType ) return;

	updatePhaseMarker(item, proc, res);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::fetchComponent(char componentCode) {
	for ( auto it = SC_D.allStreams.begin(); it != SC_D.allStreams.end(); ) {
		char queuedComponent = COMPS[it->component];
		if ( queuedComponent == componentCode || queuedComponent == '?' ) {
			// Cut the needed timewindow
			RecordViewItem* item = SC_D.recordView->item(it->streamID);
			if ( item ) {
				AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
				it->timeWindow = label->timeWindow;
			}

			SC_D.nextStreams.push_back(*it);
			it = SC_D.allStreams.erase(it);
		}
		else
			++it;
	}

	acquireStreams();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showComponent(char componentCode) {
	int newSlot;

	switch ( componentCode ) {
		default:
		case 'Z':
			newSlot = 0;
			break;
		case '1':
			newSlot = 1;
			break;
		case '2':
			newSlot = 2;
			break;
	}

	if ( SC_D.componentMap[newSlot] >= 0 ) {
		fetchComponent(componentCode);
		SC_D.currentSlot = newSlot;
	}

	SC_D.recordView->showSlot(SC_D.componentMap[SC_D.currentSlot]);
	SC_D.ui.actionShowZComponent->setChecked(SC_D.currentSlot == 0);
	SC_D.ui.actionShowNComponent->setChecked(SC_D.currentSlot == 1);
	SC_D.ui.actionShowEComponent->setChecked(SC_D.currentSlot == 2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showUsedStations(bool usedOnly) {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		if ( !isLinkedItem(item) ) {
			if ( usedOnly )
				item->setVisible(isTraceUsed(item->widget()));
			else
				item->setVisible(true);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::loadNextStations(float distance) {
	DataModel::Inventory* inv = Client::Inventory::Instance()->inventory();

	if ( inv != nullptr ) {

		for ( size_t i = 0; i < inv->networkCount(); ++i ) {
			Network* n = inv->network(i);
			for ( size_t j = 0; j < n->stationCount(); ++j ) {
				Station* s = n->station(j);
	
				QString code = (n->code() + "." + s->code()).c_str();
	
				if ( SC_D.stations.contains(code) ) continue;
	
				try {
					if ( s->end() <= SC_D.origin->time() )
						continue;
				}
				catch ( Core::ValueException& ) {}
	
				double lat = s->latitude();
				double lon = s->longitude();
				double delta, az1, az2;
	
				Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
				                  lat, lon, &delta, &az1, &az2);
	
				if ( delta > distance ) continue;

				// Skip stations out of amplitude processors range
				if ( delta < SC_D.minDist || delta > SC_D.maxDist ) continue;
	
				// try to get the configured location and stream code
				Stream *stream = findConfiguredStream(s, SC_D.origin->time());

				// Try to get a default stream
				if ( stream == nullptr ) {
					// Preferred channel code is BH. If not available use either SH or skip.
					for ( size_t c = 0; c < SC_D.broadBandCodes.size(); ++c ) {
						stream = findStream(s, SC_D.broadBandCodes[c], SC_D.origin->time());
						if ( stream ) break;
					}
				}

				if ( stream == nullptr )
					stream = findStream(s, SC_D.origin->time(), Processing::WaveformProcessor::MeterPerSecond);
	
				if ( stream ) {
					WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");

					try {
						TravelTime ttime =
							SC_D.ttTable->computeFirst(SC_D.origin->latitude(), SC_D.origin->longitude(),
							                           SC_D.origin->depth(), lat, lon);

						Core::Time referenceTime = SC_D.origin->time().value() + Core::TimeSpan(ttime.time);

						RecordViewItem* item = addStream(stream->sensorLocation(), streamID, referenceTime, false);
						if ( item ) {
							SC_D.stations.insert(code);
							item->setVisible(!SC_D.ui.actionShowUsedStations->isChecked());
							if ( SC_D.config.hideStationsWithoutData )
								item->forceInvisibilty(true);
						}
					}
					catch ( ... ) {}
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addAmplitude(Gui::RecordViewItem *item,
                                 DataModel::Amplitude *amp,
                                 DataModel::Pick *pick, Core::Time reference,
                                 int id) {
	RecordWidget *widget = item->widget();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	// Store referencing pickID
	if ( label->processor && pick ) {
		label->processor->setReferencingPickID(pick->publicID());
		label->processor->setPick(pick);
	}

	if ( amp ) {
		AmplitudeViewMarker *marker;
		marker = new AmplitudeViewMarker(widget, amp->timeWindow().reference(), AmplitudeViewMarker::Amplitude, false);
		marker->setAmplitude(amp);
		marker->setText(SC_D.magnitudeType.c_str());
		marker->setId(id);

		if ( amp->waveformID().channelCode().size() > 2 )
			marker->setSlot(item->mapComponentToSlot(*amp->waveformID().channelCode().rbegin()));

		if ( label->magnitudeProcessor ) {
			double m, per = 0, snr = 0;

			try { per = amp->period().value(); } catch ( ... ) {}
			try { snr = amp->snr(); } catch ( ... ) {}

			Processing::MagnitudeProcessor::Status stat;
			stat = label->magnitudeProcessor->computeMagnitude(
				amp->amplitude().value(), label->processor->unit(),
				per, snr, item->value(ITEM_DISTANCE_INDEX),
				SC_D.origin->depth(), SC_D.origin.get(), label->location, amp, m);
			if ( stat == Processing::MagnitudeProcessor::OK )
				marker->setMagnitude(m, QString());
			else {
				if ( label->magnitudeProcessor->treatAsValidMagnitude() )
					marker->setMagnitude(m, stat.toString());
				else
					marker->setMagnitude(Core::None, stat.toString());
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::setOrigin(Seiscomp::DataModel::Origin* origin,
                              const std::string &magType) {
	if ( (origin == SC_D.origin) && (SC_D.magnitudeType == magType) ) return true;

	SEISCOMP_DEBUG("stopping record acquisition");
	stop();

	SC_D.recordView->clear();
	SC_D.recordItemLabels.clear();

	SC_D.labelAmpType->setEnabled(false);
	SC_D.comboAmpType->clear();
	SC_D.comboAmpType->setEnabled(false);

	SC_D.labelAmpCombiner->setEnabled(false);
	SC_D.comboAmpCombiner->clear();
	SC_D.comboAmpCombiner->setEnabled(false);

	SC_D.origin = origin;
	SC_D.magnitude = nullptr;

	figureOutTravelTimeTable();
	updateOriginInformation();

	if ( SC_D.comboFilter->currentIndex() == 0 )
		SC_D.comboFilter->setCurrentIndex(1);

	if ( SC_D.origin == nullptr )
		return false;

	Processing::MagnitudeProcessorPtr procMag = Processing::MagnitudeProcessorFactory::Create(magType.c_str());
	if ( !procMag ) {
		QMessageBox::critical(this, "Amplitude waveform review",
		                      QString("Failed to create magnitude processor with type %1").arg(magType.c_str()));
		return false;
	}

	SC_D.magnitudeType = magType;
	SC_D.amplitudeType = procMag->amplitudeType();
	Processing::AmplitudeProcessorPtr procAmp = Processing::AmplitudeProcessorFactory::Create(SC_D.amplitudeType.c_str());
	if ( !procAmp ) {
		QMessageBox::critical(this, "Amplitude waveform review",
		                      QString("Failed to create amplitude processor with type %1").arg(SC_D.amplitudeType.c_str()));
		return false;
	}

	if ( procAmp->supports(Processing::AmplitudeProcessor::MeasureType) ) {
		Processing::AmplitudeProcessor::IDList params = procAmp->capabilityParameters(Processing::AmplitudeProcessor::MeasureType);
		for ( size_t i = 0; i < params.size(); ++i )
			SC_D.comboAmpType->addItem(params[i].c_str());
	}
	SC_D.comboAmpType->setEnabled(SC_D.comboAmpType->count() > 0);
	SC_D.labelAmpType->setEnabled(SC_D.comboAmpType->isEnabled());

	if ( procAmp->supports(Processing::AmplitudeProcessor::Combiner) ) {
		SC_D.comboAmpCombiner->setEnabled(true);
		Processing::AmplitudeProcessor::IDList params = procAmp->capabilityParameters(Processing::AmplitudeProcessor::Combiner);
		for ( size_t i = 0; i < params.size(); ++i )
			SC_D.comboAmpCombiner->addItem(params[i].c_str());
	}
	SC_D.comboAmpCombiner->setEnabled(SC_D.comboAmpCombiner->count() > 0);
	SC_D.labelAmpCombiner->setEnabled(SC_D.comboAmpCombiner->isEnabled());

	// Default map from component to trace slot in RecordWidget
	SC_D.componentMap[0] = -1;
	SC_D.componentMap[1] = -1;
	SC_D.componentMap[2] = -1;
	SC_D.slotCount = 0;

	Processing::WaveformProcessor::StreamComponent c = procAmp->usedComponent();
	switch ( c ) {
		case Processing::WaveformProcessor::Vertical:
			SC_D.componentMap[0] = 0;
			SC_D.slotCount = 1;
			break;
		case Processing::WaveformProcessor::FirstHorizontal:
			SC_D.componentMap[1] = 0;
			SC_D.slotCount = 1;
			break;
		case Processing::WaveformProcessor::SecondHorizontal:
			SC_D.componentMap[2] = 0;
			SC_D.slotCount = 1;
			break;
		case Processing::WaveformProcessor::Horizontal:
			SC_D.componentMap[1] = 0;
			SC_D.componentMap[2] = 1;
			SC_D.slotCount = 2;
			break;
		case Processing::WaveformProcessor::Any:
			SC_D.componentMap[0] = 0;
			SC_D.componentMap[1] = 1;
			SC_D.componentMap[2] = 2;
			SC_D.slotCount = 3;
			break;
		default:
			return false;
	}

	SC_D.spinSNR->setValue(procAmp->config().snrMin);

	SC_D.minDist = procAmp->config().minimumDistance;
	SC_D.maxDist = procAmp->config().maximumDistance;

	if ( SCScheme.unit.distanceInKM ) {
		SC_D.spinDistance->setMinimum(Math::Geo::deg2km(SC_D.minDist));
		SC_D.spinDistance->setMaximum(Math::Geo::deg2km(SC_D.maxDist));
	}
	else {
		SC_D.spinDistance->setMinimum(SC_D.minDist);
		SC_D.spinDistance->setMaximum(SC_D.maxDist);
	}

	setUpdatesEnabled(false);

	SC_D.stations.clear();

	Core::Time originTime = SC_D.origin->time();
	if ( !originTime ) originTime = Core::Time::GMT();	

	// Default is 1h travel time for 180deg
	double maxTravelTime = 3600.0;

	try {
		TravelTime tt = SC_D.ttTable->computeFirst(0.0,0.0, SC_D.origin->depth(), 180.0,0.0);
		maxTravelTime = tt.time;
	}
	catch ( ... ) {}

	SEISCOMP_DEBUG("MaxTravelTime = %.2f", maxTravelTime);

	SC_D.minTime = procAmp->config().noiseBegin;
	SC_D.maxTime = maxTravelTime+procAmp->config().signalEnd;
	Core::TimeWindow timeWindow(originTime+Core::TimeSpan(SC_D.minTime),
	                            originTime+Core::TimeSpan(SC_D.maxTime));
	SC_D.recordView->setTimeWindow(timeWindow);
	SC_D.recordView->setTimeRange(SC_D.minTime, SC_D.maxTime);

	for ( size_t i = 0; i < SC_D.origin->magnitudeCount(); ++i ) {
		if ( SC_D.origin->magnitude(i)->type() == magType ) {
			SC_D.magnitude = SC_D.origin->magnitude(i);
			break;
		}
	}

	map<string, StationItem> items;

	if ( SC_D.magnitude ) {
		for ( size_t i = 0; i < SC_D.magnitude->stationMagnitudeContributionCount(); ++i ) {
			StationMagnitudeContribution *staMagRef = SC_D.magnitude->stationMagnitudeContribution(i);
			StationMagnitude *staMag = StationMagnitude::Find(staMagRef->stationMagnitudeID());
			if ( !staMag ) continue;
			StationItem item;

			item.amp = Amplitude::Find(staMag->amplitudeID());
			if ( !item.amp && SC_D.reader )
				item.amp = Amplitude::Cast(SC_D.reader->getObject(Amplitude::TypeInfo(), staMag->amplitudeID()));

			if ( !item.amp ) continue;

			item.pick = Pick::Cast(PublicObject::Find(item.amp->pickID()));
			item.isTrigger = true;

			if ( !item.amp->pickID().empty() && !item.pick && SC_D.reader )
				item.pick = Pick::Cast(SC_D.reader->getObject(Pick::TypeInfo(), item.amp->pickID()));

			string itemID = item.amp->waveformID().networkCode() + "." + item.amp->waveformID().stationCode();
			items[itemID] = item;
		}
	}

	for ( size_t i = 0; i < SC_D.origin->arrivalCount(); ++i ) {
		Arrival* arrival = origin->arrival(i);

		StationItem item;

		item.pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( !item.pick ) {
			//std::cout << "pick not found" << std::endl;
			continue;
		}

		item.isTrigger = getShortPhaseName(arrival->phase().code()) == 'P';

		string itemID = item.pick->waveformID().networkCode() + "." + item.pick->waveformID().stationCode();
		map<string, StationItem>::iterator it = items.find(itemID);

		if ( it != items.end() ) {
			// Entry with amplitude already registered?
			if ( it->second.amp ) continue;
			if ( !it->second.pick ) continue;

			// If this pick is earlier than the already registered one
			// use it
			if ( it->second.pick->time().value() > item.pick->time().value() ) {
				it->second.pick = item.pick;
				it->second.isTrigger = item.isTrigger;
			}
		}
		else {
			items[itemID] = item;
		}
	}

	for ( auto it : items ) {
		WaveformStreamID streamID;
		Core::Time reference;

		if ( it.second.amp )
			streamID = adjustWaveformStreamID(it.second.amp->waveformID());
		else if ( it.second.pick )
			streamID = adjustWaveformStreamID(it.second.pick->waveformID());
		else
			continue;

		SensorLocation *loc = nullptr;

		Station *sta = Client::Inventory::Instance()->getStation(
		               streamID.networkCode(), streamID.stationCode(), SC_D.origin->time());

		if ( sta )
			loc = findSensorLocation(sta, streamID.locationCode(), SC_D.origin->time());
		if ( loc == nullptr ) {
			SEISCOMP_ERROR("skipping station %s.%s: sensor location %s not found",
			               streamID.networkCode().c_str(),
			               streamID.stationCode().c_str(),
			               streamID.locationCode().c_str());
			continue;
		}

		if ( it.second.pick && it.second.isTrigger )
			reference = it.second.pick->time().value();
		else /*if ( it->second.amp )*/ {
			try {
				TravelTime ttime =
					SC_D.ttTable->computeFirst(SC_D.origin->latitude(), SC_D.origin->longitude(),
					                           SC_D.origin->depth(), loc->latitude(), loc->longitude());

				reference = SC_D.origin->time().value() + Core::TimeSpan(ttime.time);
			}
			catch ( ... ) {
				SEISCOMP_ERROR("skipping station %s.%s: unable to compute trigger time",
				               streamID.networkCode().c_str(),
				               streamID.stationCode().c_str());
				continue;
			}
		}

		if ( it.second.amp == nullptr ) {
			double delta, az, baz;
			Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
			                  loc->latitude(), loc->longitude(), &delta, &az, &baz);

			if ( delta < SC_D.minDist || delta > SC_D.maxDist ) {
				SEISCOMP_INFO("skipping station %s.%s: out of range",
				              streamID.networkCode().c_str(), streamID.stationCode().c_str());
				continue;
			}
		}

		RecordViewItem *item = addStream(loc, streamID, reference, true);
		// A new item has been inserted
		if ( item != nullptr ) {
			addAmplitude(item, it.second.amp.get(), it.second.pick.get(), reference, -1);
			SC_D.stations.insert((streamID.networkCode() + "." + streamID.stationCode()).c_str());
		}
		else {
			// Try to find the existing item for the stream
			item = SC_D.recordView->item(streamID);

			// If not found ignore this stream, we can't do anything else
			if ( item == nullptr ) continue;

			// If the stream is a strong motion stream, we need to unlink
			// it from its broadband stream (disconnect the "expand button" feature)
			if ( isLinkedItem(item) )
				unlinkItem(item);
		}
	}

	if ( SC_D.recordView->rowCount() == 0 )
		loadNextStations();

	SC_D.recordView->setAlignment(originTime);

	SC_D.ui.actionShowZComponent->setEnabled(SC_D.componentMap[0] >= 0);
	SC_D.ui.actionShowZComponent->setVisible(SC_D.componentMap[0] >= 0);
	SC_D.ui.actionShowNComponent->setEnabled(SC_D.componentMap[1] >= 0);
	SC_D.ui.actionShowNComponent->setVisible(SC_D.componentMap[1] >= 0);
	SC_D.ui.actionShowEComponent->setEnabled(SC_D.componentMap[2] >= 0);
	SC_D.ui.actionShowEComponent->setVisible(SC_D.componentMap[2] >= 0);

	resetState();
	updateLayoutFromState();

	selectFirstVisibleItem(SC_D.recordView);

	setUpdatesEnabled(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateOriginInformation() {
	QString title;

	if ( SC_D.origin ) {
		QString depth;
		try {
			depth = QString("%1").arg(static_cast<int>(SC_D.origin->depth()));
		}
		catch ( Core::ValueException& ) {
			depth = "nullptr";
		}

		title = QString("ID: %1, Lat/Lon: %2 | %3, Depth: %4 km")
		                 .arg(SC_D.origin->publicID().c_str())
		                 .arg(SC_D.origin->latitude(), 0, 'f', 2)
		                 .arg(SC_D.origin->longitude(), 0, 'f', 2)
		                 .arg(depth);
	}
	else {
		title = "";
	}

	setWindowTitle(title);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setStationEnabled(const std::string& networkCode,
                                      const std::string& stationCode,
                                      bool state) {
	QList<RecordViewItem*> streams = SC_D.recordView->stationStreams(networkCode, stationCode);
	foreach ( RecordViewItem* item, streams ) {
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

		label->isEnabledByConfig = state;

		// Force state to false if item has no data yet and should be hidden
		if ( SC_D.config.hideStationsWithoutData && !label->hasGotData && !isTracePicked(item->widget()) )
			state = false;

		item->forceInvisibilty(!state);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCurrentStation(const std::string& networkCode,
                                      const std::string& stationCode) {
	QList<RecordViewItem*> streams = SC_D.recordView->stationStreams(networkCode, stationCode);
	if ( streams.isEmpty() ) return;
	SC_D.recordView->setCurrentItem(streams.front());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::queueStream(const DataModel::WaveformStreamID& streamID,
                                int component) {
	SC_D.allStreams.push_back(
		AmplitudeViewPrivate::WaveformRequest(Core::TimeWindow(), streamID, component)
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* AmplitudeView::addStream(const DataModel::SensorLocation *sloc,
                                         const WaveformStreamID& streamID,
                                         const Core::Time &referenceTime,
                                         bool showDisabled) {
	bool isEnabled = true;
	if ( !showDisabled ) {
		isEnabled = SCApp->isStationEnabled(streamID.networkCode(), streamID.stationCode());
		if ( !isEnabled )
			return nullptr;
	}

	// HACK: Add strong motion
	WaveformStreamID smStreamID(streamID);
	SensorLocation *smsloc = nullptr;
	bool hasStrongMotion = false;

	if ( SC_D.config.loadStrongMotionData ) {

		Station *sta = Client::Inventory::Instance()->getStation(
			streamID.networkCode(),
			streamID.stationCode(),
			SC_D.origin->time());

		if ( sta ) {
			// Find the stream with given code priorities
			Stream *stream = nullptr;
			for ( size_t c = 0; c < SC_D.strongMotionCodes.size(); ++c ) {
				stream = findStream(sta, SC_D.strongMotionCodes[c], SC_D.origin->time());
				if ( stream ) break;
			}

			if ( stream == nullptr )
				stream = findStream(sta, SC_D.origin->time(), Processing::WaveformProcessor::MeterPerSecondSquared);

			if ( stream ) {
				smsloc = stream->sensorLocation();
				smStreamID.setLocationCode(smsloc->code());

				smStreamID.setChannelCode(stream->code());
				smStreamID = adjustWaveformStreamID(smStreamID);

				hasStrongMotion = true;
			}
		}

	}

	RecordViewItem *item = addRawStream(sloc, streamID, referenceTime);
	if ( item == nullptr ) return nullptr;

	item->setValue(ITEM_PRIORITY_INDEX, 0);
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	label->isEnabledByConfig = isEnabled;

	item->forceInvisibilty(!label->isEnabledByConfig);

	if ( hasStrongMotion ) {
		// Try to find a corresponding StrongMotion stream and add
		// it to the view
		RecordViewItem *sm_item = addRawStream(smsloc, smStreamID, referenceTime);
		if ( sm_item ) {
			label = static_cast<AmplitudeRecordLabel*>(sm_item->label());
			label->setLinkedItem(true);
			label->isEnabledByConfig = isEnabled;
			label->hasGotData = false;
			sm_item->setValue(ITEM_PRIORITY_INDEX, 1);
			sm_item->forceInvisibilty(!label->isEnabledByConfig);
			sm_item->setVisible(false);

			// Start showing the expandable button when the first record arrives
			connect(sm_item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
			        static_cast<AmplitudeRecordLabel*>(item->label()), SLOT(enableExpandable(const Seiscomp::Record*)));

			static_cast<AmplitudeRecordLabel*>(item->label())->setControlledItem(sm_item);

			sm_item->label()->setBackgroundColor(QColor(192,192,255));
		}
	}

	addTheoreticalArrivals(item, streamID.networkCode(), streamID.stationCode(), streamID.locationCode());

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::openConnectionInfo(const QPoint &p) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::ttInterfaceChanged(QString interface) {
	SC_D.comboTTTables->blockSignals(true);
	SC_D.comboTTTables->clear();

	SC_D.ttInterface = interface.toStdString();

	try {
		vector<string> models = SCApp->configGetStrings("ttt." + interface.toStdString() + ".tables");
		int currentIndex = -1;
		for ( size_t i = 0; i < models.size(); ++i ) {
			SC_D.comboTTTables->addItem(models[i].c_str());
			if ( SC_D.ttTableName == models[i] )
				currentIndex = SC_D.comboTTTables->count()-1;
		}

		if ( currentIndex >= 0 )
			SC_D.comboTTTables->setCurrentIndex(currentIndex);
	}
	catch ( ... ) {}

	SC_D.comboTTTables->setEnabled(SC_D.comboTTTables->count() > 0);
	SC_D.comboTTTables->blockSignals(false);

	ttTableChanged(SC_D.comboTTTables->currentText());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::ttTableChanged(QString tables) {
	SC_D.ttTableName = tables.toStdString();

	SC_D.ttTable = TravelTimeTableInterfaceFactory::Create(SC_D.ttInterface.c_str());
	if ( !SC_D.ttTable ) {
		QMessageBox::critical(this, tr("Error"),
		                      tr("Error creating travel time table backend %1")
		                      .arg(SC_D.ttInterface.c_str()));
	}
	else if ( !SC_D.ttTable->setModel(SC_D.ttTableName.c_str()) ) {
		QMessageBox::critical(this, tr("Error"),
		                      tr("Failed to set table %1")
		                      .arg(SC_D.ttTableName.c_str()));
	}

	fillTheoreticalArrivals();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* AmplitudeView::addRawStream(const DataModel::SensorLocation *loc,
                                            const WaveformStreamID& sid,
                                            const Core::Time &referenceTime) {
	WaveformStreamID streamID(sid);

	if ( loc == nullptr ) return nullptr;

	double delta, az, baz;
	Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
	                  loc->latitude(), loc->longitude(), &delta, &az, &baz);

	// Skip stations out of range
	//if ( delta < SC_D.minDist || delta > SC_D.maxDist ) return nullptr;

	Processing::AmplitudeProcessorPtr proc = Processing::AmplitudeProcessorFactory::Create(SC_D.amplitudeType.c_str());
	if ( proc == nullptr ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": unable to create processor "
		     << SC_D.amplitudeType << ": ignoring station" << endl;
		return nullptr;
	}

	proc->setTrigger(referenceTime);

	bool allComponents = true;
	ThreeComponents tc;
	char comps[3] = {COMPS[0],COMPS[1],COMPS[2]};

	// Fetch all three components
	getThreeComponents(tc, loc, streamID.channelCode().substr(0, streamID.channelCode().size()-1).c_str(), SC_D.origin->time());
	if ( tc.comps[ThreeComponents::Vertical] ) {
		comps[0] = *tc.comps[ThreeComponents::Vertical]->code().rbegin();
		Processing::Stream stream;
		stream.init(sid.networkCode(),
		            sid.stationCode(),
		            sid.locationCode(),
		            tc.comps[ThreeComponents::Vertical]->code(),
		            referenceTime);
		proc->streamConfig(Processing::WaveformProcessor::VerticalComponent) = stream;
	}
	else
		allComponents = false;

	if ( tc.comps[ThreeComponents::FirstHorizontal] ) {
		comps[1] = *tc.comps[ThreeComponents::FirstHorizontal]->code().rbegin();
		Processing::Stream stream;
		stream.init(sid.networkCode(),
		            sid.stationCode(),
		            sid.locationCode(),
		            tc.comps[ThreeComponents::FirstHorizontal]->code(),
		            referenceTime);
		proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent) = stream;
	}
	else
		allComponents = false;

	if ( tc.comps[ThreeComponents::SecondHorizontal] ) {
		comps[2] = *tc.comps[ThreeComponents::SecondHorizontal]->code().rbegin();
		Processing::Stream stream;
		stream.init(sid.networkCode(),
		            sid.stationCode(),
		            sid.locationCode(),
		            tc.comps[ThreeComponents::SecondHorizontal]->code(),
		            referenceTime);
		proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent) = stream;
	}
	else
		allComponents = false;


	Util::KeyValuesPtr keys = getParams(sid.networkCode(), sid.stationCode());

	if ( !proc->setup(
		Processing::Settings(
			SCApp->configModuleName(),
			sid.networkCode(), sid.stationCode(),
			sid.locationCode(), sid.channelCode().substr(0,2),
			&SCCoreApp->configuration(), keys.get())) ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": setup processor failed"
		     << ": ignoring station" << endl;
		return nullptr;
	}

	Processing::MagnitudeProcessorPtr magProc = Processing::MagnitudeProcessorFactory::Create(SC_D.magnitudeType.c_str());
	if ( magProc == nullptr ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": unable to create magnitude processor "
		     << SC_D.magnitudeType << ": ignoring station" << endl;
		return nullptr;
	}

	if ( proc->config().minimumDistance < SC_D.minDist ) {
		SC_D.minDist = proc->config().minimumDistance;
		if ( SCScheme.unit.distanceInKM )
			SC_D.spinDistance->setMinimum(Math::Geo::deg2km(SC_D.minDist));
		else
			SC_D.spinDistance->setMinimum(SC_D.minDist);
	}

	if ( proc->config().maximumDistance > SC_D.maxDist ) {
		SC_D.maxDist = proc->config().maximumDistance;
		if ( SCScheme.unit.distanceInKM )
			SC_D.spinDistance->setMaximum(Math::Geo::deg2km(SC_D.maxDist));
		else
			SC_D.spinDistance->setMaximum(SC_D.maxDist);
	}

	try {
		proc->setHint(Processing::WaveformProcessor::Depth, SC_D.origin->depth());
	}
	catch ( ... ) {}

	proc->setHint(Processing::WaveformProcessor::Distance, delta);

	try {
		proc->setHint(Processing::WaveformProcessor::Time, (double) SC_D.origin->time().value());
	}
	catch ( ... ) {}

	proc->setEnvironment(SC_D.origin.get(), loc, proc->pick());
	proc->computeTimeWindow();

	if ( proc->isFinished() ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": setup amplitude processor failed"
		     << ": " << proc->status().toString() << " (" << proc->statusValue() << "): ignoring station" << endl;
		return nullptr;
	}

	if ( !magProc->setup(
		Processing::Settings(
			SCApp->configModuleName(),
			sid.networkCode(), sid.stationCode(),
			sid.locationCode(), sid.channelCode().substr(0,2),
			&SCCoreApp->configuration(), keys.get())) ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": setup magnitude processor failed"
		     << ": ignoring station" << endl;
		return nullptr;
	}

	RecordViewItem* item = SC_D.recordView->addItem(adjustWaveformStreamID(streamID), sid.stationCode().c_str());
	if ( item == nullptr ) return nullptr;

	if ( SC_D.currentRecord )
		item->widget()->setCursorText(SC_D.currentRecord->cursorText());

	item->label()->setText(sid.stationCode().c_str(), 0);
	QFont f(item->label()->font(0));
	f.setBold(true);
	item->label()->setFont(f, 0);

	QFontMetrics fm(f);
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	label->setWidth(fm.boundingRect("WWWW ").width(), 0);

	label->setText(QString("%1").arg(sid.networkCode().c_str()), 1);
	label->processor = proc;
	label->magnitudeProcessor = magProc;
	label->initialMinSNR = proc->config().snrMin;

	label->location = loc;
	label->latitude = loc->latitude();
	label->longitude = loc->longitude();

	label->orientationZRT.loadRotateZ(deg2rad(baz + 180.0));

	item->setValue(ITEM_DISTANCE_INDEX, delta);
	item->setValue(ITEM_AZIMUTH_INDEX, az);

	if ( SCScheme.unit.distanceInKM )
		label->setText(QString("%1 km").arg(Math::Geo::deg2km(delta),0,'f',SCScheme.precision.distance), 2);
	else
		label->setText(QString("%1%2").arg(delta,0,'f',1).arg(degrees), 2);

	label->setAlignment(Qt::AlignRight, 2);
	label->setColor(palette().color(QPalette::Disabled, QPalette::WindowText), 2);

	label->timeWindow.set(referenceTime+Core::TimeSpan(label->processor->config().noiseBegin - SC_D.config.preOffset),
	                      referenceTime+Core::TimeSpan(label->processor->config().signalEnd + SC_D.config.postOffset));
	//label->timeWindow = label->processor->safetyTimeWindow();

	if ( !allComponents )
		SEISCOMP_WARNING("Unable to fetch all components of stream %s.%s.%s.%s",
		                 streamID.networkCode().c_str(), streamID.stationCode().c_str(),
		                 streamID.locationCode().c_str(), streamID.channelCode().substr(0,streamID.channelCode().size()-1).c_str());

	item->setData(QVariant(QString(sid.stationCode().c_str())));
	setupItem(comps, item);

	// Compute and set rotation matrix
	if ( allComponents ) {
		//cout << "[" << streamID.stationCode() << "]" << endl;
		try {
			Math::Vector3f n;
			n.fromAngles(+deg2rad(tc.comps[ThreeComponents::Vertical]->azimuth()),
			             -deg2rad(tc.comps[ThreeComponents::Vertical]->dip())).normalize();
			label->orientationZNE.setColumn(2, n);
			//cout << tc.comps[ThreeComponents::Vertical]->code() << ": dip=" << tc.comps[ThreeComponents::Vertical]->dip()
			//     << ", az=" << tc.comps[ThreeComponents::Vertical]->azimuth() << endl;

			n.fromAngles(+deg2rad(tc.comps[ThreeComponents::FirstHorizontal]->azimuth()),
			             -deg2rad(tc.comps[ThreeComponents::FirstHorizontal]->dip())).normalize();
			label->orientationZNE.setColumn(1, n);
			//cout << tc.comps[ThreeComponents::FirstHorizontal]->code() << ": dip=" << tc.comps[ThreeComponents::FirstHorizontal]->dip()
			//     << ", az=" << tc.comps[ThreeComponents::FirstHorizontal]->azimuth() << endl;

			n.fromAngles(+deg2rad(tc.comps[ThreeComponents::SecondHorizontal]->azimuth()),
			             -deg2rad(tc.comps[ThreeComponents::SecondHorizontal]->dip())).normalize();
			label->orientationZNE.setColumn(0, n);
			//cout << tc.comps[ThreeComponents::SecondHorizontal]->code() << ": dip=" << tc.comps[ThreeComponents::SecondHorizontal]->dip()
			//     << ", az=" << tc.comps[ThreeComponents::SecondHorizontal]->azimuth() << endl;
		}
		catch ( ... ) {
			SEISCOMP_WARNING("Unable to fetch orientation of stream %s.%s.%s.%s",
		                 streamID.networkCode().c_str(), streamID.stationCode().c_str(),
		                 streamID.locationCode().c_str(), streamID.channelCode().substr(0,streamID.channelCode().size()-1).c_str());
			allComponents = false;
		}
	}

	if ( !allComponents )
		// Set identity matrix
		label->orientationZNE.identity();

	for ( int i = 0; i < 3; ++i ) {
		WaveformStreamID componentID = setWaveformIDComponent(streamID, comps[i]);
		label->data.traces[i].channelCode = componentID.channelCode();
		label->data.traces[i].recordSlot = SC_D.componentMap[i];
		// Map waveformID to recordviewitem label
		SC_D.recordItemLabels[waveformIDToStdString(componentID)] = label;

		if ( label->data.traces[i].recordSlot >= 0 )
			queueStream(setWaveformIDComponent(streamID, comps[i]), i);
	}

	label->data.setRecordWidget(item->widget());
	label->updateProcessingInfo();
	label->hasGotData = false;

	applyFilter(item);

	AmplitudeViewMarker *marker =
		new AmplitudeViewMarker(item->widget(), referenceTime, AmplitudeViewMarker::Reference, false);
	Q_UNUSED(marker);

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char AmplitudeView::currentComponent() const {
	if ( SC_D.ui.actionShowZComponent->isChecked() )
		return 'Z';
	else if ( SC_D.ui.actionShowNComponent->isChecked() )
		return '1';
	else if ( SC_D.ui.actionShowEComponent->isChecked() )
		return '2';

	return '\0';
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setupItem(const char comps[3],
                              RecordViewItem *item) {
	connect(item->widget(), SIGNAL(cursorUpdated(RecordWidget*,int)),
	        this, SLOT(updateMainCursor(RecordWidget*,int)));

	connect(item, SIGNAL(componentChanged(RecordViewItem*, char)),
	        this, SLOT(updateItemLabel(RecordViewItem*, char)));

	connect(item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
	        this, SLOT(updateItemRecordState(const Seiscomp::Record*)));

	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	item->widget()->setDecorator(new TraceDecorator(item->widget(), label));
	item->widget()->setSelected(0,0);

	label->setOrientation(Qt::Horizontal);
	label->setToolTip("Timing quality: undefined");

	QPalette pal = item->widget()->palette();
	pal.setColor(QPalette::WindowText, QColor(128,128,128));
	//pal.setColor(QPalette::HighlightedText, QColor(128,128,128));
	item->widget()->setPalette(pal);

	item->widget()->setCustomBackgroundColor(SCScheme.colors.records.states.unrequested);

	item->widget()->setSlotCount(SC_D.slotCount);

	for ( int i = 0; i < 3; ++i ) {
		if ( SC_D.componentMap[i] < 0 ) continue;
		item->insertComponent(comps[i], SC_D.componentMap[i]);
	}

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( inv ) {
		std::string channelCode = item->streamID().channelCode().substr(0,2);
		double gain;

		for ( int i = 0; i < 3; ++i ) {
			if ( SC_D.componentMap[i] < 0 ) continue;
			try {
				gain = inv->getGain(item->streamID().networkCode(), item->streamID().stationCode(),
				                    item->streamID().locationCode(), channelCode + comps[i],
				                    SC_D.origin->time().value());
				item->widget()->setRecordScale(SC_D.componentMap[i], 1E9 / gain);
			}
			catch ( ... ) {}
		}
	}

	item->widget()->showScaledValues(SC_D.ui.actionShowTraceValuesInNmS->isChecked());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::addTheoreticalArrivals(RecordViewItem *item,
                                           const std::string &netCode,
                                           const std::string &staCode,
                                           const std::string &locCode) {
	if ( !SC_D.origin ) return false;

	// First clear all theoretical arrivals
	for ( int i = 0; i < item->widget()->markerCount(); ) {
		auto m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(i));
		if ( m->type() == AmplitudeViewMarker::Theoretical )
			item->widget()->removeMarker(i);
		else
			++i;
	}

	item->widget()->update();

	if ( !SC_D.ttTable ) return false;

	try {
		DataModel::SensorLocation *loc =
			Client::Inventory::Instance()->getSensorLocation(
				netCode, staCode, locCode, SC_D.origin->time()
		);

		if ( loc == nullptr ) {
			SEISCOMP_ERROR("SensorLocation %s.%s.%s not found",
			               netCode.c_str(), staCode.c_str(), locCode.c_str());
			return false;
		}

		double delta, az1, az2;
		double elat = SC_D.origin->latitude();
		double elon = SC_D.origin->longitude();
		double slat, slon;
		double salt = loc->elevation();

		try {
			slat = loc->latitude(); slon = loc->longitude();
		}
		catch ( ... ) {
			SEISCOMP_WARNING("SensorLocation %s.%s.%s has no valid coordinates",
			                 netCode.c_str(), staCode.c_str(), locCode.c_str());
			return false;
		}

		Math::Geo::delazi(elat, elon, slat, slon, &delta, &az1, &az2);

		item->setValue(ITEM_DISTANCE_INDEX, delta);
		item->setValue(ITEM_AZIMUTH_INDEX, az1);

		/*
		if ( SC_D.config.showAllComponents && SC_D.config.allComponentsMaximumStationDistance >= delta )
			item->widget()->setDrawMode(RecordWidget::InRows);
		else
			item->widget()->setDrawMode(RecordWidget::Single);
		*/

		/*
		item->label()->setText(QApplication::translate("PickerView", "D: %1\302\260 A: %2\302\260", 0, QApplication::UnicodeUTF8)
		                        .arg(delta,0,'f',1).arg(az1,0,'f',1), 1);
		*/

		item->label()->setText(QString("%1").arg(netCode.c_str()), 1);
		QFontMetrics fm(item->label()->font(1));
		item->label()->setWidth(fm.boundingRect("WW  ").width(), 1);

		if ( SCScheme.unit.distanceInKM )
			item->label()->setText(QString("%1 km").arg(Math::Geo::deg2km(delta),0,'f',SCScheme.precision.distance), 2);
		else
			item->label()->setText(QString("%1%2").arg(delta,0,'f',1).arg(degrees), 2);
		item->label()->setAlignment(Qt::AlignRight, 2);
		item->label()->setColor(palette().color(QPalette::Disabled, QPalette::WindowText), 2);

		double depth;
		try {
			depth = SC_D.origin->depth();
		}
		catch ( ... ) {
			depth = 0.0;
		}

		TravelTimeList* ttt = SC_D.ttTable->compute(elat, elon, depth, slat, slon, salt);

		if ( ttt ) {
			QMap<QString, RecordMarker*> currentPhases;

			foreach ( const QString &phase, SC_D.phases ) {
				// Find the TravelTime from the TravelTimeList that corresponds
				// the current phase
				const TravelTime *tt = findPhase(*ttt, phase, delta);
				if ( !tt ) continue;

				// If there is already a theoretical marker for the given
				// TravelTime the current item gets another alias
				if ( currentPhases.contains(tt->phase.c_str()) ) {
					currentPhases[tt->phase.c_str()]->addAlias(phase + THEORETICAL_POSTFIX);
					continue;
				}

				auto marker = new AmplitudeViewMarker(
					item->widget(),
					static_cast<Core::Time>(SC_D.origin->time()) + Core::TimeSpan(tt->time),
					phase + THEORETICAL_POSTFIX,
					AmplitudeViewMarker::Theoretical,
					false
				);

				marker->setVisible(SC_D.ui.actionShowTheoreticalArrivals->isChecked());

				// Set the description of the marker that is used as display text
				marker->setDescription(tt->phase.c_str());

				// Remember the phase
				currentPhases[tt->phase.c_str()] = marker;
			}

			delete ttt;
		}

		return true;
	}
	catch ( std::exception &exc ) {
		SEISCOMP_ERROR("%s", exc.what());
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::fillTheoreticalArrivals() {
	bool stationSet = false;

	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		WaveformStreamID stream_id = SC_D.recordView->streamID(i);

		RecordViewItem* item = SC_D.recordView->itemAt(i);
		if ( addTheoreticalArrivals(item, stream_id.networkCode(), stream_id.stationCode(), stream_id.locationCode()) )
			stationSet = true;
	}

	SC_D.currentRecord->update();

	return stationSet;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateMainCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 && slot != SC_D.currentSlot )
		showComponent(comps[slot]);

	setCursorPos(w->cursorPos(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateSubCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 ) {
		if ( SC_D.componentMap[0] == slot ) slot = 0;
		else if ( SC_D.componentMap[1] == slot ) slot = 1;
		else if ( SC_D.componentMap[2] == slot ) slot = 2;
		else slot = -1;
	}

	if ( slot != -1 && slot != SC_D.currentSlot )
		showComponent(comps[slot]);

	SC_D.recordView->currentItem()->widget()->blockSignals(true);
	SC_D.recordView->currentItem()->widget()->setCursorPos(w->cursorPos());
	SC_D.recordView->currentItem()->widget()->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateRecordValue(Seiscomp::Core::Time t) {
	if ( !statusBar() ) return;

	const double *v = SC_D.currentRecord->value(t);

	if ( v == nullptr )
		statusBar()->clearMessage();
	else
		statusBar()->showMessage(QString("value = %1").arg(*v, 0, 'f', 2));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showTraceScaleToggled(bool e) {
	SC_D.currentRecord->showScaledValues(e);
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordViewItem* item = SC_D.recordView->itemAt(i);
		item->widget()->showScaledValues(e);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showTheoreticalArrivals(bool v) {
	for ( int i = 0; i < SC_D.currentRecord->markerCount(); ++i ) {
		auto m = static_cast<AmplitudeViewMarker*>(SC_D.currentRecord->marker(i));
		if ( m->isTheoretical() ) {
			m->setVisible(v);
		}
	}

	// Since all markers are just proxies of the real traces we need
	// to update the zoom trace explicitly.
	SC_D.currentRecord->update();

	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordWidget *w = SC_D.recordView->itemAt(i)->widget();

		for ( int i = 0; i < w->markerCount(); ++i ) {
			auto m = static_cast<AmplitudeViewMarker*>(w->marker(i));
			if ( m->isTheoretical() ) {
				m->setVisible(v);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateItemLabel(RecordViewItem* item, char component) {
	/*
	if ( item == SC_D.recordView->currentItem() )
		SC_D.currentRecord->setRecords(item->widget()->records(component), component, false);
	*/

	/*
	QString text = item->label()->text(0);
	int index = text.lastIndexOf('.');
	if ( index < 0 ) return;

	if ( text.size() - index > 2 )
		text[text.size()-1] = component;
	else
		text += component;
	item->label()->setText(text, 0);
	*/

	if ( item == SC_D.recordView->currentItem() ) {
		QString text = SC_D.ui.labelCode->text();

		int index = text.lastIndexOf(' ');
		if ( index < 0 ) return;
	
		if ( text.size() - index > 2 )
			text[text.size()-1] = component;
		else
			text += component;

		SC_D.ui.labelCode->setText(text);
	}

	updateTraceInfo(item, nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateItemRecordState(const Seiscomp::Record *rec) {
	// Reset acquisition related coloring since the first record
	// arrived already
	RecordViewItem *item = static_cast<RecordViewItem *>(sender());
	RecordWidget *widget = item->widget();
	int slot = item->mapComponentToSlot(*rec->channelCode().rbegin());
	widget->setRecordBackgroundColor(slot, SCScheme.colors.records.states.inProgress);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCursorPos(const Seiscomp::Core::Time& t, bool always) {
	SC_D.currentRecord->setCursorPos(t);

	if ( !always && SC_D.currentRecord->cursorText() == "" ) return;

	float offset = 0;

	if ( SC_D.centerSelection ) {
		float len = SC_D.recordView->currentItem()?
			SC_D.recordView->currentItem()->widget()->width() / SC_D.currentRecord->timeScale():
			SC_D.currentRecord->tmax() - SC_D.currentRecord->tmin();

		float pos = float(t - SC_D.currentRecord->alignment()) - len/2;
		offset = pos - SC_D.currentRecord->tmin();
	}
	else {
		if ( t > SC_D.currentRecord->rightTime() )
			offset = t - SC_D.currentRecord->rightTime();
		else if ( t < SC_D.currentRecord->leftTime() )
			offset = t - SC_D.currentRecord->leftTime();
	}

	move(offset);

	SC_D.centerSelection = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setTimeRange(float tmin, float tmax) {
	auto amplScale = SC_D.currentRecord->amplScale();
	SC_D.currentRecord->setTimeRange(tmin, tmax);

	if ( SC_D.autoScaleZoomTrace )
		SC_D.currentRecord->setNormalizationWindow(SC_D.currentRecord->visibleTimeWindow());

	/*
	std::cout << "ScaleWindow: " << Core::toString(SC_D.currentRecord->visibleTimeWindow().startTime()) << ", "
	          << Core::toString(SC_D.currentRecord->visibleTimeWindow().endTime()) << std::endl;
	*/

	SC_D.currentRecord->setAmplScale(amplScale);
	SC_D.timeScale->setTimeRange(tmin, tmax);

	/*
	std::cout << "[setTimeRange]" << std::endl;
	std::cout << "new TimeRange(" << SC_D.currentRecord->tmin() << ", " << SC_D.currentRecord->tmax() << ")" << std::endl;
	std::cout << "current TimeScale = " << SC_D.currentRecord->timeScale() << std::endl;
	*/

	if ( SC_D.recordView->currentItem() )
		SC_D.recordView->currentItem()->widget()->setSelected(SC_D.currentRecord->tmin(), SC_D.currentRecord->tmax());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::enableAutoScale() {
	SC_D.autoScaleZoomTrace = true;
	if ( SC_D.currentRecord ) {
		auto amplScale = SC_D.currentRecord->amplScale();
		SC_D.currentRecord->setNormalizationWindow(SC_D.currentRecord->visibleTimeWindow());
		SC_D.currentRecord->setAmplScale(amplScale);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::disableAutoScale() {
	SC_D.autoScaleZoomTrace = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::zoomSelectionHandleMoved(int idx, double v, Qt::KeyboardModifiers mods) {
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(SC_D.recordView->currentItem()->label());
	if ( label->processor ) {
		double relTime = v - (double)(label->processor->trigger() - SC_D.timeScale->alignment());

		switch ( idx ) {
			case 0:
				label->processor->setNoiseStart(relTime);
				SC_D.recordView->timeWidget()->setSelectionHandle(0, v);
				break;
			case 1:
				label->processor->setNoiseEnd(relTime);
				SC_D.recordView->timeWidget()->setSelectionHandle(1, v);
				if ( !mods.testFlag(Qt::ShiftModifier) ) {
					label->processor->setSignalStart(relTime);
					SC_D.timeScale->setSelectionHandle(2, SC_D.timeScale->selectionHandlePos(1));
					SC_D.timeScale->setSelectionHandleEnabled(2, false);
					SC_D.recordView->timeWidget()->setSelectionHandle(2, SC_D.recordView->timeWidget()->selectionHandlePos(1));
				}
				else {
					SC_D.timeScale->setSelectionHandleEnabled(2, true);
				}
				break;
			case 2:
				label->processor->setSignalStart(relTime);
				SC_D.recordView->timeWidget()->setSelectionHandle(2, v);
				if ( !mods.testFlag(Qt::ShiftModifier) ) {
					label->processor->setNoiseEnd(relTime);
					SC_D.timeScale->setSelectionHandle(1, SC_D.timeScale->selectionHandlePos(2));
					SC_D.recordView->timeWidget()->setSelectionHandle(1, SC_D.recordView->timeWidget()->selectionHandlePos(2));
					SC_D.timeScale->setSelectionHandleEnabled(2, false);
				}
				break;
			case 3:
				label->processor->setSignalEnd(relTime);
				SC_D.recordView->timeWidget()->setSelectionHandle(3, v);
				break;
			default:
				return;
		}

		SC_D.currentRecord->update();
		SC_D.recordView->currentItem()->widget()->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::zoomSelectionHandleMoveFinished() {
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(SC_D.recordView->currentItem()->label());
	if ( label->processor ) label->processor->computeTimeWindow();

	applyFilter(SC_D.recordView->currentItem());
	SC_D.recordView->currentItem()->widget()->update();
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectionHandleMoved(int idx, double v, Qt::KeyboardModifiers mods) {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( label->processor ) {
			double relTime = v;

			switch ( idx ) {
				case 0:
					label->processor->setNoiseStart(relTime);
					break;
				case 1:
					label->processor->setNoiseEnd(relTime);
					if ( !mods.testFlag(Qt::ShiftModifier) ) {
						label->processor->setSignalStart(relTime);
						SC_D.recordView->timeWidget()->setSelectionHandleEnabled(2, false);
						SC_D.recordView->timeWidget()->setSelectionHandle(2, SC_D.recordView->timeWidget()->selectionHandlePos(1));
					}
					else
						SC_D.recordView->timeWidget()->setSelectionHandleEnabled(2, true);
					break;
				case 2:
					label->processor->setSignalStart(relTime);
					if ( !mods.testFlag(Qt::ShiftModifier) ) {
						SC_D.recordView->timeWidget()->setSelectionHandleEnabled(2, false);
						label->processor->setNoiseEnd(relTime);
					}
					break;
				case 3:
					label->processor->setSignalEnd(relTime);
					break;
				default:
					return;
			}

			item->widget()->update();
		}
	}

	SC_D.currentRecord->update();

	if ( SC_D.recordView->currentItem() == nullptr ) return;

	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(SC_D.recordView->currentItem()->label());
	if ( label->processor ) {
		SC_D.timeScale->setSelectionEnabled(true);
		SC_D.timeScale->setSelectionHandle(0, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().noiseBegin);
		SC_D.timeScale->setSelectionHandle(1, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().noiseEnd);
		SC_D.timeScale->setSelectionHandle(2, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().signalBegin);
		SC_D.timeScale->setSelectionHandle(3, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().signalEnd);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectionHandleMoveFinished() {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( label->processor ) label->processor->computeTimeWindow();
	}

	applyFilter();
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setAlignment(Seiscomp::Core::Time t) {
	double offset = SC_D.currentRecord->alignment() - t;
	SC_D.currentRecord->setAlignment(t);

	// Because selection handle position are relative to the alignment
	// move them
	if ( SC_D.timeScale->isSelectionEnabled() ) {
		for ( int i = 0; i < SC_D.timeScale->selectionHandleCount(); ++i )
			SC_D.timeScale->setSelectionHandle(i, SC_D.timeScale->selectionHandlePos(i)+offset);
	}

	SC_D.timeScale->setAlignment(t);

	auto tmin = SC_D.currentRecord->tmin() + offset;
	auto tmax = SC_D.currentRecord->tmax() + offset;

	if ( SC_D.checkVisibility ) {
		ensureVisibility(tmin, tmax);
	}

	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::ensureVisibility(double &tmin, double &tmax) {
	if ( SC_D.recordView->currentItem() ) {
		RecordWidget* w = SC_D.recordView->currentItem()->widget();
		auto leftOffset = tmin - w->tmin();
		auto rightOffset = tmax - w->tmax();
		if ( leftOffset < 0 ) {
			tmin = w->tmin();
			tmax -= leftOffset;
		}
		else if ( rightOffset > 0 ) {
			auto usedOffset = std::min(leftOffset, rightOffset);
			tmin -= usedOffset;
			tmax -= usedOffset;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin) {
	Core::Time left = time - Core::TimeSpan(pixelMargin / SC_D.currentRecord->timeScale());
	Core::Time right = time + Core::TimeSpan(pixelMargin / SC_D.currentRecord->timeScale());

	double offset = 0;
	if ( right > SC_D.currentRecord->rightTime() )
		offset = right - SC_D.currentRecord->rightTime();
	else if ( left < SC_D.currentRecord->leftTime() )
		offset = left - SC_D.currentRecord->leftTime();

	if ( offset != 0 )
		setTimeRange(SC_D.currentRecord->tmin() + offset, SC_D.currentRecord->tmax() + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::moveTraces(double offset) {
	if ( fabs(offset) < 0.001 ) return;

	SC_D.recordView->move(offset);

	float tmin = SC_D.recordView->timeRangeMin();
	float tmax = SC_D.recordView->timeRangeMax();

	if ( tmin > SC_D.currentRecord->tmin() ) {
		offset = tmin - SC_D.currentRecord->tmin();
	}
	else if ( tmax < SC_D.currentRecord->tmax() ) {
		float length = tmax - tmin;
		float cr_length = SC_D.currentRecord->tmax() - SC_D.currentRecord->tmin();

		offset = tmax - SC_D.currentRecord->tmax();

		if ( cr_length > length )
			offset += cr_length - length;
	}
	else
		offset = 0;

	setTimeRange(SC_D.currentRecord->tmin() + offset,
	             SC_D.currentRecord->tmax() + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::move(double offset) {
	if ( fabs(offset) < 0.001 ) return;

	float tmin = SC_D.currentRecord->tmin() + offset;
	float tmax = SC_D.currentRecord->tmax() + offset;

	if ( tmin < SC_D.recordView->timeRangeMin() ) {
		offset = tmin - SC_D.recordView->timeRangeMin();
	}
	else if ( tmax > SC_D.recordView->timeRangeMax() ) {
		float length = tmax - tmin;
		float rv_length = SC_D.recordView->timeRangeMax() - SC_D.recordView->timeRangeMin();

		offset = tmax - SC_D.recordView->timeRangeMax();

		if ( length > rv_length )
			offset -= length - rv_length;
	}
	else
		offset = 0;

	SC_D.recordView->move(offset);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::itemSelected(RecordViewItem* item, RecordViewItem* lastItem) {
	float smin = 0;
	float smax = 0;

	Core::TimeSpan relSelectedTime;

	if ( lastItem ) {
		smin = lastItem->widget()->smin();
		smax = lastItem->widget()->smax();
		lastItem->widget()->setSelected(0,0);
		lastItem->widget()->setShadowWidget(nullptr, false);
		lastItem->widget()->setCurrentMarker(nullptr);

		disconnect(lastItem->label(), SIGNAL(statusChanged(bool)),
		           this, SLOT(setCurrentRowEnabled(bool)));

		relSelectedTime = lastItem->widget()->cursorPos() - lastItem->widget()->alignment();
	}

	if ( !item ) {
		SC_D.currentRecord->setDecorator(nullptr);
		SC_D.currentRecord->clearRecords();
		SC_D.currentRecord->setEnabled(false);
		SC_D.currentRecord->setMarkerSourceWidget(nullptr);
		SC_D.timeScale->setSelectionEnabled(false);
		return;
	}

	//_centerSelection = true;

	Core::Time cursorPos;
	RecordMarker* m = item->widget()->enabledMarker(item->widget()->cursorText());
	if ( m )
		cursorPos = m->correctedTime();
	else
		cursorPos = item->widget()->alignment() + relSelectedTime;

	//item->widget()->setCursorPos(cursorPos);
	//_currentRecord->setCursorPos(cursorPos);
	SC_D.currentRecord->setEnabled(item->widget()->isEnabled());
	SC_D.currentRecord->setDecorator(item->widget()->decorator());

	connect(item->label(), SIGNAL(statusChanged(bool)),
	        this, SLOT(setCurrentRowEnabled(bool)));

	auto amplScale = SC_D.currentRecord->amplScale();

	SC_D.currentRecord->setNormalizationWindow(item->widget()->normalizationWindow());
	SC_D.currentRecord->setAlignment(item->widget()->alignment());

	SC_D.timeScale->setAlignment(item->widget()->alignment());

	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	if ( label->processor ) {
		SC_D.timeScale->setSelectionEnabled(true);
		SC_D.timeScale->setSelectionHandle(0, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().noiseBegin);
		SC_D.timeScale->setSelectionHandle(1, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().noiseEnd);
		SC_D.timeScale->setSelectionHandle(2, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().signalBegin);
		SC_D.timeScale->setSelectionHandle(3, double(label->processor->trigger() - SC_D.timeScale->alignment()) + label->processor->config().signalEnd);
		SC_D.timeScale->setSelectionHandleEnabled(2, true);

		SC_D.recordView->timeWidget()->setSelectionHandle(0, label->processor->config().noiseBegin);
		SC_D.recordView->timeWidget()->setSelectionHandle(1, label->processor->config().noiseEnd);
		SC_D.recordView->timeWidget()->setSelectionHandle(2, label->processor->config().signalBegin);
		SC_D.recordView->timeWidget()->setSelectionHandle(3, label->processor->config().signalEnd);
		SC_D.recordView->timeWidget()->setSelectionHandleEnabled(2, true);
	}
	else
		SC_D.timeScale->setSelectionEnabled(false);

	if ( smax - smin > 0 )
		setTimeRange(smin, smax);
	else
		setTimeRange(SC_D.recordView->timeRangeMin(), SC_D.recordView->timeRangeMax());

	//_currentRecord->setAmplScale(item->widget()->amplScale());
	SC_D.currentRecord->setAmplScale(amplScale);

	item->widget()->setShadowWidget(SC_D.currentRecord, false);
	SC_D.currentRecord->setMarkerSourceWidget(item->widget());

	if ( SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter(item);

	/*
	if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
		if ( SC_D.config.showAllComponents &&
		     SC_D.config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
			SC_D.currentRecord->setDrawMode(RecordWidget::InRows);
		else
			SC_D.currentRecord->setDrawMode(RecordWidget::Single);
	}
	else
		SC_D.currentRecord->setDrawMode(RecordWidget::Single);
	*/
	SC_D.currentRecord->setDrawMode(RecordWidget::InRows);

	/*
	SC_D.currentRecord->clearMarker();
	for ( int i = 0; i < item->widget()->markerCount(); ++i )
		new AmplitudeViewMarker(SC_D.currentRecord, *static_cast<AmplitudeViewMarker*>(item->widget()->marker(i)));
	*/

	//SC_D.ui.labelCode->setText(item->label()->text(0));
	//SC_D.ui.labelInfo->setText(item->label()->text(1));

	if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
		if ( SCScheme.unit.distanceInKM )
			SC_D.ui.labelDistance->setText(QString("%1 km").arg(Math::Geo::deg2km(item->value(ITEM_DISTANCE_INDEX)),0,'f',SCScheme.precision.distance));
		else
			SC_D.ui.labelDistance->setText(QString("%1%2").arg(item->value(ITEM_DISTANCE_INDEX),0,'f',1).arg(degrees));
		SC_D.ui.labelAzimuth->setText(QString("%1%2").arg(item->value(ITEM_AZIMUTH_INDEX),0,'f',1).arg(degrees));
	}

	WaveformStreamID streamID = SC_D.recordView->streamID(item->row());
	std::string cha = streamID.channelCode();
	char component = item->currentComponent();

	for ( int i = 0; i < SC_D.currentRecord->slotCount(); ++i )
		SC_D.currentRecord->setRecordID(i, QString("%1").arg(item->mapSlotToComponent(i)));

	if ( cha.size() > 2 )
		cha[cha.size()-1] = component;
	else
		cha += component;

	SC_D.ui.labelStationCode->setText(streamID.stationCode().c_str());
	SC_D.ui.labelCode->setText(QString("%1  %2%3")
	                        .arg(streamID.networkCode().c_str())
	                        .arg(streamID.locationCode().c_str())
	                        .arg(cha.c_str()));
	/*
	const RecordSequence* seq = SC_D.currentRecord->records();
	if ( seq && !seq->empty() )
		SC_D.ui.labelCode->setText((*seq->begin())->streamID().c_str());
	else
		SC_D.ui.labelCode->setText("NO DATA");
	*/

	//_centerSelection = true;
	//_currentRecord->enableFiltering(item->widget()->isFilteringEnabled());
	setCursorPos(cursorPos);
	SC_D.currentRecord->update();

	updateCurrentRowState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCurrentRowEnabled(bool enabled) {
	SC_D.currentRecord->setEnabled(enabled);
	updateCurrentRowState();

	RecordWidget* w = SC_D.recordView->currentItem()->widget();

	if ( w ) {
		for ( int i = 0; i < w->markerCount(); ++i ) {
			if ( w->marker(i)->id() >= 0 ) {
				/*
				emit arrivalChanged(w->marker(i)->id(), enabled?w->marker(i)->isEnabled():false);
				// To disable an arrival (trace) we have to keep this information
				// somewhere else not just in the picker
				emit arrivalEnableStateChanged(w->marker(i)->id(), enabled);
				*/
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCurrentRowDisabled(bool disabled) {
	//setCurrentRowEnabled(!disabled);
	if ( SC_D.currentRecord->cursorText().isEmpty() ||
	     (!disabled && !SC_D.currentRecord->isEnabled()) ) {
		SC_D.currentRecord->setEnabled(!disabled);
		if ( SC_D.recordView->currentItem() )
			SC_D.recordView->currentItem()->label()->setEnabled(!disabled);
	}
	else {
		setMarkerState(SC_D.currentRecord, !disabled);
		if ( SC_D.recordView->currentItem() )
			setMarkerState(SC_D.recordView->currentItem()->widget(), !disabled);
	}

	updateCurrentRowState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setMarkerState(Seiscomp::Gui::RecordWidget* w, bool enabled) {
	bool foundManual = false;
	int arid = -1;

	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->text() == w->cursorText() ) {
			if ( marker->isMovable() ) foundManual = true;
			if ( marker->id() >= 0 ) arid = marker->id();
		}
	}

	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->text() == w->cursorText() ) {
			if ( marker->isEnabled() != enabled && arid >= 0 ) {
				//emit arrivalChanged(arid, enabled);
				arid = -1;
			}

			if ( marker->isMovable() || !foundManual ) {
				marker->setEnabled(enabled);
				marker->update();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateCurrentRowState() {
	//setMarkerState(SC_D.currentRecord, enabled);
	//_currentRecord->setEnabled(enabled);

	bool enabled = true;

	if ( !SC_D.currentRecord->isEnabled() )
		enabled = false;
	else if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		RecordMarker* m = SC_D.currentRecord->marker(SC_D.currentRecord->cursorText(), true);
		if ( !m ) m = SC_D.currentRecord->marker(SC_D.currentRecord->cursorText(), false);
		enabled = m?m->isEnabled():true;
	}

	SC_D.ui.btnRowAccept->setChecked(false);
	SC_D.ui.btnRowAccept->setEnabled(enabled);
	SC_D.ui.btnRowReset->setEnabled(enabled);

	SC_D.ui.btnRowRemove->setChecked(!enabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateTraceInfo(RecordViewItem* item,
                                 const Seiscomp::Record* rec) {
	float timingQuality = item->widget()->timingQuality(SC_D.componentMap[SC_D.currentSlot]);
	if ( timingQuality >= 0 ) {
		if ( timingQuality > 100 ) timingQuality = 100;

		if ( timingQuality < 50 )
			static_cast<AmplitudeRecordLabel*>(item->label())->setLabelColor(blend(SC_D.config.timingQualityMedium, SC_D.config.timingQualityLow, (int)(timingQuality*2)));
		else
			static_cast<AmplitudeRecordLabel*>(item->label())->setLabelColor(blend(SC_D.config.timingQualityHigh, SC_D.config.timingQualityMedium, (int)((timingQuality-50)*2)));

		item->label()->setToolTip(QString("Timing quality: %1").arg((int)timingQuality));
	}
	else {
		static_cast<AmplitudeRecordLabel*>(item->label())->removeLabelColor();
		item->label()->setToolTip("Timing quality: undefined");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::toggleFilter() {
	if ( SC_D.comboFilter->currentIndex() > 1 )
		SC_D.comboFilter->setCurrentIndex(1);
	else {
		if ( SC_D.lastFilterIndex < 0 )
			SC_D.lastFilterIndex = std::min(SC_D.comboFilter->count()-1,2);

		SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addNewFilter(const QString& filter) {
	SC_D.lastFilterIndex = SC_D.comboFilter->findData(filter);

	if ( SC_D.lastFilterIndex == -1 ) {
		SC_D.comboFilter->addItem(filter, filter);
		SC_D.lastFilterIndex = SC_D.comboFilter->count()-1;
	}

	SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
	SC_D.currentRecord->setFilter(SC_D.recordView->filter());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleVisibleAmplitudes() {
	SC_D.recordView->scaleVisibleAmplitudes();

	SC_D.currentRecord->setNormalizationWindow(SC_D.currentRecord->visibleTimeWindow());
	SC_D.currentAmplScale = 1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeScale(double, double) {
	zoom(1.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeTimeRange(double, double) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::sortAlphabetically() {
	SC_D.recordView->sortByTextAndValue(0, ITEM_PRIORITY_INDEX);

	SC_D.ui.actionSortAlphabetically->setChecked(true);
	SC_D.ui.actionSortByDistance->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::sortByDistance() {
	SC_D.recordView->sortByValue(ITEM_DISTANCE_INDEX, ITEM_PRIORITY_INDEX);

	SC_D.ui.actionSortAlphabetically->setChecked(false);
	SC_D.ui.actionSortByDistance->setChecked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showZComponent() {
	showComponent('Z');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showNComponent() {
	showComponent('1');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showEComponent() {
	showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::alignOnOriginTime() {
	SC_D.checkVisibility = false;
	SC_D.recordView->setAbsoluteTimeEnabled(true);
	SC_D.recordView->setTimeRange(SC_D.minTime, SC_D.maxTime);
	SC_D.recordView->setSelectionEnabled(false);
	SC_D.checkVisibility = true;

	SC_D.recordView->setAlignment(SC_D.origin->time());

	SC_D.ui.actionAlignOnPArrival->setChecked(false);
	SC_D.ui.actionAlignOnOriginTime->setChecked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::alignOnPArrivals() {
	int used = 0;
	double minTime = -10;
	double maxTime = 60;

	SC_D.ui.actionAlignOnPArrival->setChecked(true);
	SC_D.ui.actionAlignOnOriginTime->setChecked(false);

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem *item = SC_D.recordView->itemAt(r);
		AmplitudeRecordLabel *l = static_cast<AmplitudeRecordLabel*>(item->label());

		RecordWidget* w = item->widget();

		// Find modified arrivals for phase of controller item
		RecordMarker* m = w->marker("");

		if ( m ) {
			w->setAlignment(m->correctedTime());
			++used;
		}

		if ( l->processor ) {
			if ( l->processor->config().noiseBegin < minTime )
				minTime = l->processor->config().noiseBegin;
			if ( l->processor->config().signalEnd > maxTime )
				maxTime = l->processor->config().signalEnd;
		}
	}

	if ( !used ) return;

	SC_D.recordView->setAbsoluteTimeEnabled(false);
	SC_D.recordView->setTimeRange(minTime-5, maxTime+5);
	SC_D.recordView->setSelectionEnabled(true);

	if ( SC_D.recordView->currentItem() ) {
		RecordWidget* w = SC_D.recordView->currentItem()->widget();
		setAlignment(w->alignment());
		setCursorPos(w->alignment(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::pickAmplitudes(bool) {
	setCursorText(SC_D.amplitudeType.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::pickNone(bool) {
	setCursorText("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleAmplUp() {
	double scale = SC_D.currentRecord->amplScale();
	//if ( scale >= 1 ) scale = SC_D.currentAmplScale;
	double value = (scale == 0 ? 1.0 : scale) * SC_D.recordView->zoomFactor();
	if ( value > 1000 ) value = 1000;
	if ( /*value < 1*/true ) {
		SC_D.currentRecord->setAmplScale(value);
		SC_D.currentAmplScale = 1;
	}
	else {
		SC_D.currentRecord->setAmplScale(1);
		SC_D.currentAmplScale = value;
	}

	//_currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleAmplDown() {
	double scale = SC_D.currentRecord->amplScale();
	//if ( scale >= 1 ) scale = SC_D.currentAmplScale;
	double value = (scale == 0 ? 1.0 : scale) / SC_D.recordView->zoomFactor();
	//if ( value < 1 ) value = 1;
	if ( value < 0.001 ) value = 0.001;

	//_currentRecord->setAmplScale(value);
	if ( /*value < 1*/true ) {
		SC_D.currentRecord->setAmplScale(value);
		SC_D.currentAmplScale = 1;
	}
	else {
		SC_D.currentRecord->setAmplScale(1);
		SC_D.currentAmplScale = value;
	}

	//_currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleReset() {
	SC_D.currentRecord->setAmplScale(1.0);
	SC_D.currentAmplScale = 1.0;
	zoom(0.0);

	//_currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleTimeUp() {
	zoom(SC_D.recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleTimeDown() {
	zoom(1.0 / SC_D.recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::zoom(float factor) {
	SC_D.zoom *= factor;
	if ( SC_D.zoom < 1.0 )
		SC_D.zoom = 1.0;

	if ( SC_D.zoom > 100 )
		SC_D.zoom = 100;

	auto currentScale = SC_D.currentRecord->timeScale();
	auto newScale = SC_D.recordView->timeScale() * SC_D.zoom;

	factor = newScale / currentScale;

	auto tmin = SC_D.currentRecord->tmin();
	auto tmax = SC_D.recordView->currentItem()?
		tmin + SC_D.recordView->currentItem()->widget()->width() / SC_D.currentRecord->timeScale():
		SC_D.currentRecord->tmax();
	auto tcen = tmin + (tmax - tmin) * 0.5;

	tmin = tcen - (tcen - tmin) / factor;
	tmax = tcen + (tmax - tcen) / factor;

	SC_D.currentRecord->setTimeScale(newScale);
	SC_D.timeScale->setScale(newScale);
	
	if ( SC_D.checkVisibility ) {
		ensureVisibility(tmin, tmax);
	}

	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::applyTimeRange(double rmin, double rmax) {
	auto tmin = rmin;
	auto tmax = rmax;

	auto newScale = SC_D.currentRecord->width() / (tmax-tmin);
	if ( newScale < SC_D.recordView->timeScale() )
		newScale = SC_D.recordView->timeScale();

	if ( tmin < SC_D.recordView->currentItem()->widget()->tmin() )
		tmin = SC_D.recordView->currentItem()->widget()->tmin();

	SC_D.currentRecord->setTimeScale(newScale);
	SC_D.timeScale->setScale(newScale);

	// Calculate zoom
	SC_D.zoom = newScale / SC_D.recordView->timeScale();

	if ( SC_D.checkVisibility ) {
		ensureVisibility(tmin, tmax);
	}

	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollLeft() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp -= Core::TimeSpan((float)width()/(20 * SC_D.currentRecord->timeScale()));
		setCursorPos(cp);
		return;
	}
	
	float offset = -(float)width()/(8 * SC_D.currentRecord->timeScale());
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollFineLeft() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp -= Core::TimeSpan(1.0 / SC_D.currentRecord->timeScale());
		setCursorPos(cp);
		return;
	}
	
	float offset = -1.0 / SC_D.currentRecord->timeScale();
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollRight() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp += Core::TimeSpan((float)width()/(20 * SC_D.currentRecord->timeScale()));
		setCursorPos(cp);
		return;
	}

	float offset = (float)width()/(8 * SC_D.currentRecord->timeScale());
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollFineRight() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp += Core::TimeSpan(1.0 / SC_D.currentRecord->timeScale());
		setCursorPos(cp);
		return;
	}

	float offset = 1.0 / SC_D.currentRecord->timeScale();
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::applyAmplitudes() {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem *rvi = SC_D.recordView->itemAt(r);
		RecordWidget *widget = rvi->widget();

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker *marker = widget->marker(m);
			marker->apply();
		}
	}

	SC_D.changedAmplitudes.clear();
	SC_D.recordView->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::hasModifiedAmplitudes() const {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem *rvi = SC_D.recordView->itemAt(r);
		RecordWidget *widget = rvi->widget();
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker *marker = widget->marker(m);
			if ( marker->isModified() )
				return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::getChangedAmplitudes(ObjectChangeList<DataModel::Amplitude> &list) const {
	list = SC_D.changedAmplitudes;
	SC_D.changedAmplitudes.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::fetchManualAmplitudes(std::vector<RecordMarker*>* markers) const {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = SC_D.recordView->itemAt(r);

		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(rvi->label());
		RecordWidget* widget = rvi->widget();

		if ( !widget->isEnabled() ) continue;

		// NOTE: Ignore items that are children of other items and not
		//       expanded (eg SM channels)
		if ( label->isLinkedItem() ) {
			AmplitudeRecordLabel *controllerLabel = static_cast<AmplitudeRecordLabel*>(label->controlledItem()->label());
			if ( !controllerLabel->isExpanded() ) continue;
		}

		bool hasManualMarker = false;

		// Count the number of interesting markers for a particular phase
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			AmplitudeViewMarker *marker = static_cast<AmplitudeViewMarker*>(widget->marker(m));
			if ( !marker->isEnabled() ) continue;
			if ( marker->isNewAmplitude() ) {
				hasManualMarker = true;
				break;
			}
		}

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			AmplitudeViewMarker *marker = static_cast<AmplitudeViewMarker*>(widget->marker(m));

			// If the marker is not an amplitude do nothing
			if ( !marker->isAmplitude() ) continue;
			if ( !marker->isEnabled() ) continue;

			AmplitudePtr amp = marker->amplitude();

			// Picked marker and we've got an manual replacement: do nothing
			if ( hasManualMarker && !marker->isNewAmplitude() ) {
				//SEISCOMP_DEBUG("   - ignore amplitude to be replaced");
				marker->setId(-1);
				continue;
			}

			if ( amp ) {
				if ( !marker->isModified() ) {
					if ( markers )
						markers->push_back(marker);
					SEISCOMP_DEBUG("   - reuse existing amplitude");
					continue;
				}
			}

			auto a = marker->manualAmplitude();
			if ( !a ) {
				continue;
			}

			SC_D.changedAmplitudes.push_back(ObjectChangeList<DataModel::Amplitude>::value_type(a, true));
			SEISCOMP_DEBUG("   - created new amplitude");

			if ( markers ) {
				markers->push_back(marker);
			}

			marker->setAmplitude(a);
		}
	}

	/*
	// Remove all automatic amplitudes if configured
	if ( SC_D.config.removeAutomaticPicks ) {
		for ( std::vector<RecordMarker*>::iterator it = markers->begin();
		      it != markers->end(); ) {
			try {
				if ( static_cast<PickerMarker*>(*it)->pick()->evaluationMode() == MANUAL ) {
					++it;
					continue;
				}
			}
			catch ( ... ) {}

			it = markers->erase(it);
		}
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::commit() {
	std::vector<RecordMarker*> markers;
	fetchManualAmplitudes(&markers);

	QList<AmplitudePtr> amps;

	for ( size_t i = 0; i < markers.size(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(markers[i]);
		if ( !m->isEnabled() ) continue;
		AmplitudePtr amp = m->amplitude();
		if ( amp == nullptr ) {
			SEISCOMP_ERROR("Amplitude not set in marker");
			continue;
		}

		amps.append(amp);
	}

	if ( !amps.isEmpty() )
		emit amplitudesConfirmed(SC_D.origin.get(), amps);

	return;


	SEISCOMP_DEBUG("Origin.stationMags before: %d",
	               static_cast<int>(SC_D.origin->stationMagnitudeCount()));

	// Remove all station magnitudes of origin with requested type
	for ( size_t i = 0; i < SC_D.origin->stationMagnitudeCount(); ) {
		if ( SC_D.origin->stationMagnitude(i)->type() == SC_D.magnitudeType )
			SC_D.origin->removeStationMagnitude(i);
		else
			++i;
	}

	SEISCOMP_DEBUG("Origin.stationMags after: %d",
	               static_cast<int>(SC_D.origin->stationMagnitudeCount()));

	if ( !SC_D.magnitude )
		SC_D.magnitude = Magnitude::Create();
	else {
		// Remove all stationmagnitude references from magnitude
		while ( SC_D.magnitude->stationMagnitudeContributionCount() > 0 )
			SC_D.magnitude->removeStationMagnitudeContribution(0);

		SEISCOMP_DEBUG("Mag.stationMagRefs after: %d",
		               static_cast<int>(SC_D.magnitude->stationMagnitudeContributionCount()));
	}

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());

	SC_D.magnitude->setCreationInfo(ci);
	SC_D.magnitude->setType(SC_D.magnitudeType);
	SC_D.magnitude->setEvaluationStatus(EvaluationStatus(CONFIRMED));
	SC_D.magnitude->setOriginID("");

	vector<double> mags;
	vector<double> azimuths;
	set<string> stations;

	for ( size_t i = 0; i < markers.size(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(markers[i]);
		//if ( !m->isEnabled() ) continue;

		AmplitudePtr amp = m->amplitude();
		if ( !amp ) {
			SEISCOMP_ERROR("Amplitude not set in marker");
			continue;
		}

		RecordWidget *w = m->parent();
		AmplitudeRecordLabel *label = static_cast<TraceDecorator*>(w->decorator())->label();
		RecordViewItem *item = label->recordViewItem();

		if ( !label->magnitudeProcessor ) {
			SEISCOMP_ERROR("No magnitude processor attached for station %s",
			               item->data().toString().toStdString().c_str());
			continue;
		}

		double magValue;
		double period = 0, snr = 0;

		try { period = amp->period().value(); } catch ( ... ) {}
		try { snr = amp->snr(); } catch ( ... ) {}

		Processing::MagnitudeProcessor::Status stat =
			label->magnitudeProcessor->computeMagnitude(
				amp->amplitude().value(), label->processor->unit(), period, snr,
				item->value(ITEM_DISTANCE_INDEX), SC_D.origin->depth(),
				SC_D.origin.get(), label->location, amp.get(), magValue
			);

		if ( stat != Processing::MagnitudeProcessor::OK ) {
			SEISCOMP_ERROR("Failed to compute magnitude for station %s: %s",
			               item->data().toString().toStdString().c_str(),
			               stat.toString());
			continue;
		}

		mags.push_back(magValue);
		azimuths.push_back(item->value(ITEM_AZIMUTH_INDEX));

		StationMagnitudePtr staMag = StationMagnitude::Create();
		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::GMT());
		staMag->setType(SC_D.magnitude->type());
		staMag->setCreationInfo(ci);
		staMag->setWaveformID(amp->waveformID());
		staMag->setMagnitude(magValue);
		staMag->setAmplitudeID(amp->publicID());

		label->magnitudeProcessor->finalizeMagnitude(staMag.get());

		SC_D.origin->add(staMag.get());

		StationMagnitudeContributionPtr ref = new StationMagnitudeContribution;
		ref->setStationMagnitudeID(staMag->publicID());
		ref->setWeight(m->isEnabled()?1.0:0.0);

		SC_D.magnitude->add(ref.get());

		stations.insert(amp->waveformID().networkCode() + "." + amp->waveformID().stationCode());
	}

	// Magnitudes are averaged by the handler of this signal
	emit magnitudeCreated(SC_D.magnitude.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setDefaultDisplay() {
	//alignByState();
	alignOnPArrivals();
	selectFirstVisibleItem(SC_D.recordView);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
bool AmplitudeView::start() {
	stop();

	if ( SC_D.recordView->start() ) {
		connect(SC_D.recordView->recordStreamThread(), SIGNAL(finished()),
		        this, SLOT(acquisitionFinished()));
		return true;
	}

	return false;
}
*/
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::stop() {
	SC_D.recordView->stop();
	closeThreads();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid) {
	RecordViewItem *item = SC_D.recordView->item(wid);
	if ( item ) {
		SC_D.recordView->setCurrentItem(item);
		SC_D.recordView->ensureVisible(item->row());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectTrace(const std::string &code) {
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		if ( SC_D.recordView->itemAt(i)->streamID().stationCode() == code ) {
			SC_D.recordView->setCurrentItem(SC_D.recordView->itemAt(i));
			SC_D.recordView->ensureVisible(i);
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::closeThreads() {
	foreach ( RecordStreamThread* t, SC_D.acquisitionThreads) {
		t->stop(true);
		SEISCOMP_DEBUG("removed finished thread %d from list", t->ID());
		delete t;
	}

	SC_D.acquisitionThreads.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::acquisitionFinished() {
	QObject* s = sender();
	if ( s ) {
		RecordStreamThread* t = static_cast<RecordStreamThread*>(s);
		int index = SC_D.acquisitionThreads.indexOf(t);
		if ( index != -1 ) {
			SC_D.acquisitionThreads.remove(index);
			SEISCOMP_DEBUG("removed finished thread %d from list", t->ID());
			delete t;
		}

		// Update color states
		for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
			RecordViewItem *item = SC_D.recordView->itemAt(i);
			RecordWidget *widget = item->widget();

			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

			for ( int i = 0; i < 3; ++i ) {
				if ( label->data.traces[i].thread != t ) continue;
				if ( label->data.traces[i].raw && !label->data.traces[i].raw->empty() )
					widget->removeRecordBackgroundColor(label->data.traces[i].recordSlot);
				else
					widget->setRecordBackgroundColor(label->data.traces[i].recordSlot, SCScheme.colors.records.states.notAvailable);
				// Reset the thread
				label->data.traces[i].thread = nullptr;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::acquireStreams() {
	if ( SC_D.nextStreams.empty() ) return;

	RecordStreamThread *t = new RecordStreamThread(SC_D.config.recordURL.toStdString());

	if ( !t->connect() ) {
		if ( SC_D.config.recordURL != SC_D.lastRecordURL ) {
			QMessageBox::critical(this, "Waveform acquisition",
			                      QString("Unable to open recordstream '%1'").arg(SC_D.config.recordURL));
		}

		SC_D.lastRecordURL = SC_D.config.recordURL;
		delete t;
		return;
	}

	connect(t, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));

	connect(t, SIGNAL(finished()),
	        this, SLOT(acquisitionFinished()));

	for ( auto it = SC_D.nextStreams.begin(); it != SC_D.nextStreams.end(); ++it ) {
		if ( it->timeWindow ) {
			if ( !t->addStream(it->streamID.networkCode(),
				               it->streamID.stationCode(),
				               it->streamID.locationCode(),
				               it->streamID.channelCode(),
				               it->timeWindow.startTime(), it->timeWindow.endTime()) )
				t->addStream(it->streamID.networkCode(),
				             it->streamID.stationCode(),
				             it->streamID.locationCode(),
				             it->streamID.channelCode());
		}
		else {
			SEISCOMP_WARNING("Not time window for stream %s set: ignoring", waveformIDToStdString(it->streamID).c_str());
			continue;
		}

		RecordViewItem *item = SC_D.recordView->item(adjustWaveformStreamID(it->streamID));
		if ( item ) {
			int slot = item->mapComponentToSlot(*it->streamID.channelCode().rbegin());
			item->widget()->setRecordBackgroundColor(slot, SCScheme.colors.records.states.requested);
			item->widget()->setRecordUserData(slot, QVariant::fromValue((void*)t));
			// Store the acquisition thread as user data
			static_cast<AmplitudeRecordLabel*>(item->label())->data.traces[it->component].thread = t;
		}
	}

	SC_D.nextStreams.clear();

	SC_D.acquisitionThreads.push_back(t);
	t->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::receivedRecord(Seiscomp::Record *rec) {
	Seiscomp::RecordPtr tmp(rec);
	if ( !rec->data() ) return;

	std::string streamID = rec->streamID();
	auto it = SC_D.recordItemLabels.find(streamID);
	if ( it == SC_D.recordItemLabels.end() )
		return;

	AmplitudeRecordLabel *label = it->second;
	RecordViewItem *item = label->recordViewItem();

	int i;
	for ( i = 0; i < 3; ++i ) {
		if ( label->data.traces[i].channelCode == rec->channelCode() ) {
			if ( label->data.traces[i].raw == nullptr )
				label->data.traces[i].raw = new TimeWindowBuffer(label->timeWindow);
			break;
		}
	}

	if ( i == 3 ) return;

	bool firstRecord = label->data.traces[i].raw->empty();
	if ( !label->data.traces[i].raw->feed(rec) ) return;

	// Check for out-of-order records
	if ( (label->data.traces[i].filter || label->data.enableTransformation) &&
	     label->data.traces[i].raw->back() != (const Record*)rec ) {
//		SEISCOMP_DEBUG("%s.%s.%s.%s: out of order record, reinitialize trace",
//		               rec->networkCode().c_str(),
//		               rec->stationCode().c_str(),
//		               rec->locationCode().c_str(),
//		               rec->channelCode().c_str());
		RecordWidget::Filter *f = label->data.traces[i].filter->clone();
		label->data.setFilter(f, label->data.filterID);
		delete f;
	}
	else {
		try {
			label->data.transform(i, rec);
		}
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s: %s", streamID.c_str(), e.what());
			label->infoText = e.what();
		}
	}

	if ( firstRecord ) {
		item->widget()->setRecordBackgroundColor(SC_D.componentMap[i], SCScheme.colors.records.states.inProgress);
		label->hasGotData = true;

		if ( SC_D.config.hideStationsWithoutData )
			item->forceInvisibilty(!label->isEnabledByConfig);

		//item->widget()->setRecords(i, label->traces[i].raw, false);

		// If this item is linked to another item, enable the expand button of
		// the controller
		if ( label->isLinkedItem() && label->_linkedItem != nullptr )
			static_cast<AmplitudeRecordLabel*>(label->_linkedItem->label())->enabledExpandButton(item);
	}
	else {
		// Tell the widget to rebuild its traces
		//item->widget()->fed(i, rec);
		updateTraceInfo(item, rec);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addStations() {
	if ( !SC_D.origin ) return;

	SelectStation dlg(SC_D.origin->time(), SC_D.config.ignoreDisabledStations, SC_D.stations, this);
	dlg.setReferenceLocation(SC_D.origin->latitude(), SC_D.origin->longitude());
	if ( dlg.exec() != QDialog::Accepted ) return;

	QList<DataModel::Station *> stations = dlg.selectedStations();
	if ( stations.isEmpty() ) return;

	SC_D.recordView->setUpdatesEnabled(false);

	foreach ( DataModel::Station *s, stations ) {
		DataModel::Network *n = s->network();

		QString code = (n->code() + "." + s->code()).c_str();

		if ( SC_D.stations.contains(code) ) continue;

		double delta, az1, az2;
		Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
		                  s->latitude(), s->longitude(), &delta, &az1, &az2);

		// Skip stations out of amplitude processors range
		if ( delta < SC_D.minDist || delta > SC_D.maxDist ) continue;

		Stream *stream = nullptr;
		// Preferred channel code is BH. If not available use either SH or skip.
		for ( size_t c = 0; c < SC_D.broadBandCodes.size(); ++c ) {
			stream = findStream(s, SC_D.broadBandCodes[c], SC_D.origin->time());
			if ( stream ) break;
		}

		if ( stream == nullptr )
			stream = findStream(s, SC_D.origin->time(), Processing::WaveformProcessor::MeterPerSecond);

		if ( stream ) {
			WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");

			try {
				TravelTime ttime =
					SC_D.ttTable->computeFirst(SC_D.origin->latitude(), SC_D.origin->longitude(),
				                               SC_D.origin->depth(), s->latitude(), s->longitude());

				Core::Time referenceTime = SC_D.origin->time().value() + Core::TimeSpan(ttime.time);

				RecordViewItem* item = addStream(stream->sensorLocation(), streamID, referenceTime, false);
				if ( item ) {
					SC_D.stations.insert(code);
					item->setVisible(!SC_D.ui.actionShowUsedStations->isChecked());
					if ( SC_D.config.hideStationsWithoutData )
						item->forceInvisibilty(true);
				}
			}
			catch ( ... ) {}
		}
	}

	sortByState();
	alignByState();
	componentByState();

	if ( SC_D.recordView->currentItem() == nullptr )
		selectFirstVisibleItem(SC_D.recordView);

	setCursorText(SC_D.currentRecord->cursorText());

	SC_D.recordView->setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::searchStation() {
	SC_D.searchStation->selectAll();
	SC_D.searchStation->setVisible(true);
	SC_D.searchLabel->setVisible(true);

	//_searchStation->grabKeyboard();
	SC_D.searchStation->setFocus();
	SC_D.recordView->setFocusProxy(SC_D.searchStation);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::searchByText(const QString &text) {
	if ( text.isEmpty() ) return;

	QRegExp rx(text + "*");
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	int row = SC_D.recordView->findByText(0, rx, SC_D.lastFoundRow+1);
	if ( row != -1 ) {
		SC_D.recordView->setCurrentItem(SC_D.recordView->itemAt(row));
		SC_D.lastFoundRow = row;

		QPalette pal = SC_D.searchStation->palette();
		pal.setColor(QPalette::Base, SC_D.searchBase);
		SC_D.searchStation->setPalette(pal);

		SC_D.recordView->ensureVisible(SC_D.lastFoundRow);
	}
	else {
		QPalette pal = SC_D.searchStation->palette();
		pal.setColor(QPalette::Base, SC_D.searchError);
		SC_D.searchStation->setPalette(pal);
		SC_D.lastFoundRow = -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::search(const QString &text) {
	SC_D.lastFoundRow = -1;

	searchByText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::nextSearch() {
	searchByText(SC_D.searchStation->text());
	if ( SC_D.lastFoundRow == -1 )
		searchByText(SC_D.searchStation->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::abortSearchStation() {
	SC_D.recordView->setFocusProxy(nullptr);
	SC_D.searchStation->releaseKeyboard();

	SC_D.searchStation->setVisible(false);
	SC_D.searchLabel->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
void AmplitudeView::emitPick(const Processing::Picker *,
                             const Processing::Picker::Result &res) {
	setCursorPos(res.time);
}
*/
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::createAmplitude() {
	RecordViewItem* item = SC_D.recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(SC_D.currentRecord, SC_D.currentRecord->cursorPos());

		SC_D.recordView->selectNextRow();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setAmplitude() {
	RecordViewItem* item = SC_D.recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(SC_D.currentRecord, SC_D.currentRecord->cursorPos());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::confirmAmplitude() {
	RecordViewItem* item = SC_D.recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(SC_D.currentRecord, SC_D.currentRecord->cursorPos());

		int row = item->row() + 1;

		item = nullptr;

		for ( int i = 0; i < SC_D.recordView->rowCount(); ++i, ++row ) {
			if ( row >= SC_D.recordView->rowCount() ) row -= SC_D.recordView->rowCount();

			RecordViewItem* nextItem = SC_D.recordView->itemAt(row);

			// ignore disabled rows
			if ( !nextItem->widget()->isEnabled() ) continue;

			RecordMarker* m = nextItem->widget()->marker(nextItem->widget()->cursorText());
			if ( m ) {
				item = nextItem;
				SC_D.recordView->setCurrentItem(nextItem);
				SC_D.recordView->ensureVisible(row);
				break;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::deleteAmplitude() {
	RecordViewItem* item = SC_D.recordView->currentItem();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	if ( item ) {
		label->isError = false;
		label->infoText = QString();
		if ( item->widget()->cursorText().isEmpty() )
			resetAmplitude(item, SC_D.amplitudeType.c_str(), true);
		else
			resetAmplitude(item, item->widget()->cursorText(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::resetAmplitude(RecordViewItem *item, const QString &text, bool enable) {
	AmplitudeViewMarker* m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(text, true));
	if ( m ) {
		if ( m->isMoveCopyEnabled() ) {
			m->reset();
			m->setEnabled(enable);
		}
		else {
			delete m;
			m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(text));
			if ( m )
				m->setEnabled(enable);
		}
	}
	else {
		m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(text));
		if ( m )
			m->setEnabled(enable);
	}

	item->widget()->update();
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addFilter(const QString& name, const QString& filter) {
	if ( SC_D.comboFilter ) {
		if ( SC_D.comboFilter->findText(name) != -1 )
			return;

		SC_D.comboFilter->addItem(name, filter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::limitFilterToZoomTrace(bool e) {
	changeFilter(SC_D.comboFilter->currentIndex(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeFilter(int index) {
	changeFilter(index, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::applyFilter(RecordViewItem *item) {
	if ( item == nullptr ) {
		for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
			RecordViewItem* rvi = SC_D.recordView->itemAt(i);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(rvi->label());
			if ( label->processor ) {
				Core::Time t = label->processor->trigger();
				label->processor->reset();
				label->processor->setTrigger(t);
			}
			label->data.setFilter(SC_D.currentFilter, SC_D.currentFilterStr);
			label->data.showProcessedData(SC_D.showProcessedData);
		}
	}
	else {
		for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
			RecordViewItem* rvi = SC_D.recordView->itemAt(i);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(rvi->label());
			label->data.showProcessedData(SC_D.showProcessedData);
		}

		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( label->processor ) {
			Core::Time t = label->processor->trigger();
			label->processor->reset();
			label->processor->setTrigger(t);
		}
		label->data.setFilter(SC_D.currentFilter, SC_D.currentFilterStr);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeFilter(int index, bool force) {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QString name = SC_D.comboFilter->itemText(index);
	QString filter = SC_D.comboFilter->itemData(index).toString();

	SC_D.showProcessedData = false;

	if ( name == NO_FILTER_STRING ) {
		if ( SC_D.currentFilter ) delete SC_D.currentFilter;
		SC_D.currentFilter = nullptr;
		SC_D.currentFilterStr = "";

		if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
			applyFilter();
		else
			applyFilter(SC_D.recordView->currentItem());

		QApplication::restoreOverrideCursor();
		return;
	}
	else if ( name == DEFAULT_FILTER_STRING ) {
		if ( SC_D.currentFilter ) delete SC_D.currentFilter;
		SC_D.currentFilter = nullptr;
		SC_D.currentFilterStr = "";

		SC_D.showProcessedData = true;
		applyFilter();

		QApplication::restoreOverrideCursor();
		return;
	}

	SC_D.showProcessedData = true;
	RecordWidget::Filter *newFilter = RecordWidget::Filter::Create(filter.toStdString());

	if ( newFilter == nullptr ) {
		QMessageBox::critical(this, "Invalid filter",
		                      QString("Unable to create filter: %1\nFilter: %2").arg(name).arg(filter));

		SC_D.comboFilter->blockSignals(true);
		SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
		SC_D.comboFilter->blockSignals(false);

		QApplication::restoreOverrideCursor();
		return;
	}

	if ( SC_D.currentFilter ) delete SC_D.currentFilter;
	SC_D.currentFilter = newFilter;
	SC_D.currentFilterStr = filter.toStdString();

	if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter();
	else
		applyFilter(SC_D.recordView->currentItem());

	SC_D.lastFilterIndex = index;
	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setArrivalState(int arrivalId, bool state) {
	setArrivalState(SC_D.currentRecord, arrivalId, state);

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		if ( setArrivalState(item->widget(), arrivalId, state) ) {
			item->setVisible(!(SC_D.ui.actionShowUsedStations->isChecked() &&
			                   item->widget()->hasMovableMarkers()));
			if ( state )
				item->label()->setEnabled(true);
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::setArrivalState(RecordWidget* w, int arrivalId, bool state) {
	if ( !w->isEnabled() ) return false;

	bool foundManual = false;
	QString phase;

	// Find phase for arrival
	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->id() == arrivalId )
			phase = marker->text();
	}

	// Find manual marker for arrival
	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->text() == phase && marker->isMovable() )
			foundManual = true;
	}

	// Update state
	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);

		if ( marker->text() == phase ) {
			if ( marker->isMovable() || !foundManual ) {
				marker->setEnabled(state);
				marker->update();
				return true;
			}
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
