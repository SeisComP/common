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


#define SEISCOMP_COMPONENT Gui::PickerView


#include "pickerview.h"
#include "pickerview_p.h"

#include <seiscomp/core/platform/platform.h>
#include <seiscomp/gui/datamodel/selectstation.h>
#include <seiscomp/gui/datamodel/origindialog.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/recordstreamthread.h>
#include <seiscomp/gui/core/timescale.h>
#include <seiscomp/gui/core/uncertainties.h>
#include <seiscomp/gui/core/spectrogramrenderer.h>
#include <seiscomp/gui/core/spectrumwidget.h>
#include <seiscomp/gui/plot/axis.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/client/configdb.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/parameter.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/math/fft.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/const.h>
#include <seiscomp/math/filter/chainfilter.h>
#include <seiscomp/math/filter/iirdifferentiate.h>
#include <seiscomp/math/filter/iirintegrate.h>
#include <seiscomp/math/windows/cosine.h>
#include <seiscomp/math/windows/hann.h>
#include <seiscomp/math/windows/hamming.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/utils/misc.h>
#include <seiscomp/utils/keyvalues.h>
#include <seiscomp/logging/log.h>

#include <QMessageBox>
#include <QToolButton>

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>
#include <fstream>
#include <limits>
#include <set>


#ifdef MACOSX
#include <seiscomp/gui/core/osx.h>
#endif

#define NO_FILTER_STRING     "No filter"

#define ITEM_DISTANCE_INDEX  0
#define ITEM_RESIDUAL_INDEX  1
#define ITEM_AZIMUTH_INDEX  2
//#define ITEM_ARRIVALID_INDEX 2
#define ITEM_PRIORITY_INDEX  3

#define AUTOMATIC_POSTFIX    " "
#define THEORETICAL_POSTFIX  "  "

#define SET_PICKED_COMPONENT
//#define CENTER_SELECTION

#define COMP_NO_METADATA '\0'

#define SC_D (*_d_ptr)


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Util;
using namespace Seiscomp::Gui;
using namespace Seiscomp::Gui::PrivatePickerView;


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Gui::PickerMarkerActionPlugin, SC_GUI_API);
Q_DECLARE_METATYPE(Seiscomp::Core::Time)


namespace {


char ZNE_COMPS[3] = {'Z', 'N', 'E'};
char ZRT_COMPS[3] = {'Z', 'R', 'T'};
char ZH_COMPS[3] = {'Z', 'H', '-'};
char Z12_COMPS[3] = {'Z', '1', '2'};


MAKEENUM(
	RotationType,
	EVALUES(
		RT_123,
		RT_ZNE,
		RT_ZRT,
		RT_ZH
	),
	ENAMES(
		"123",
		"ZNE",
		"ZRT",
		"ZH(L2)"
	)
);


MAKEENUM(
	UnitType,
	EVALUES(
		UT_RAW,
		UT_ACC,
		UT_VEL,
		UT_DISP
	),
	ENAMES(
		"Sensor",
		"Acceleration",
		"Velocity",
		"Displacement"
	)
);


const char *Units[3] = {
	"m/s**2",
	"m/s",
	"m"
};


UnitType fromGainUnit(const std::string &gainUnit) {
	if ( !strcasecmp(gainUnit.c_str(), Units[0]) )
		return UT_ACC;
	else if ( !strcasecmp(gainUnit.c_str(), Units[1]) )
		return UT_VEL;
	else if ( !strcasecmp(gainUnit.c_str(), Units[2]) )
		return UT_DISP;
	return UT_RAW;
}


class ZoomRecordWidget : public RecordWidget {
	public:
		ZoomRecordWidget() {
			maxLower = maxUpper = 0;
			currentIndex = -1;
			crossHair = false;
			showSpectrogram = false;
			traces = nullptr;

			Gradient gradient;
			gradient.setColorAt(0.0, QColor(255,   0, 255,   0));
			gradient.setColorAt(0.2, QColor(  0,   0, 255, 255));
			gradient.setColorAt(0.4, QColor(  0, 255, 255, 255));
			gradient.setColorAt(0.6, QColor(  0, 255,   0, 255));
			gradient.setColorAt(0.8, QColor(255, 255,   0, 255));
			gradient.setColorAt(1.0, QColor(255,   0,   0, 255));

			for ( int i = 0; i < 3; ++i ) {
				spectrogram[i].setOptions(spectrogram[i].options());
				spectrogram[i].setGradient(gradient);
			}

			spectrogramAxis.setLabel(tr("f [1/T] in Hz"));
			spectrogramAxis.setPosition(Seiscomp::Gui::Axis::Right);

			if ( SCScheme.colors.records.background.isValid() ) {
				QPalette p = palette();
				p.setColor(QPalette::Base, SCScheme.colors.records.background);
				setPalette(p);
				setAutoFillBackground(true);
			}
		}

		void setUncertainties(const PickerView::Config::UncertaintyList &list) {
			uncertainties = list;
			maxLower = maxUpper = 0;
			currentIndex = -1;

			for ( int i = 0; i < uncertainties.count(); ++i ) {
				if ( i == 0 ) {
					maxLower = uncertainties[i].first;
					maxUpper = uncertainties[i].second;
				}
				else {
					maxLower = std::max(maxLower, (double)uncertainties[i].first);
					maxUpper = std::max(maxUpper, (double)uncertainties[i].second);
				}
			}
		}

		void setCurrentUncertaintyIndex(int idx) {
			currentIndex = idx;
			update();
		}

		int currentUncertaintyIndex() const {
			return currentIndex;
		}

		void setCrossHairEnabled(bool enable) {
			crossHair = enable;
			update();
		}

		void setShowSpectrogram(bool enable) {
			if ( showSpectrogram == enable ) return;

			showSpectrogram = enable;
			updateTraceColor();

			resetSpectrogram();
			update();
		}

		void setLogSpectrogram(bool enable) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setLogScale(enable);
			spectrogramAxis.setLogScale(enable);
			update();
		}

		void setSmoothSpectrogram(bool enable) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setSmoothTransform(enable);
			update();
		}

		void setMinSpectrogramRange(double v) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setGradientRange(v, spectrogram[i].gradientUpperBound());
			update();
		}

		void setMaxSpectrogramRange(double v) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setGradientRange(spectrogram[i].gradientLowerBound(), v);
			update();
		}

		void setSpectrogramTimeWindow(double tw) {
			for ( int i = 0; i < 3; ++i ) {
				IO::Spectralizer::Options opts = spectrogram[i].options();
				opts.windowLength = tw;
				spectrogram[i].setOptions(opts);
			}

			if ( showSpectrogram ) {
				resetSpectrogram();
				update();
			}
		}

		void setTraces(ThreeComponentTrace::Component *t) {
			traces = t;
			resetSpectrogram();
			updateTraceColor();
		}

		void feedRaw(int slot, const Seiscomp::Record *rec) {
			if ( showSpectrogram && (slot >= 0) && (slot < 3))
				spectrogram[slot].feed(rec);
		}

	private:
		void resetSpectrogram() {
			if ( showSpectrogram ) {
				qApp->setOverrideCursor(Qt::WaitCursor);
				for ( int i = 0; i < 3; ++i ) {
					const double *scale = recordScale(i);
					// Scale is is nm and needs to be converted to m
					if ( scale != nullptr ) spectrogram[i].setScale(*scale * 1E-9);
					spectrogram[i].setRecords(traces != nullptr ? traces[i].raw : nullptr);
					spectrogram[i].renderSpectrogram();
				}
				qApp->restoreOverrideCursor();
			}
		}

		void drawSpectrogram(QPainter &painter, int slot) {
			QRect r(0, 0, canvasRect().width(), canvasRect().height());
			r.setHeight(streamHeight(slot));
			r.moveTop(streamYPos(slot));
			spectrogram[slot].setAlignment(alignment());
			spectrogram[slot].setTimeRange(tmin(), tmax());
			painter.save();
			painter.setClipRect(r);
			spectrogram[slot].render(painter, r, false, false);
			painter.restore();
		}

		void drawSpectrogramAxis(QPainter &painter, int slot) {
			QRect r(canvasRect());
			r.setHeight(streamHeight(slot));
			r.moveTop(streamYPos(slot));

			painter.save();

			QPair<double, double> range = spectrogram[slot].range();
			spectrogramAxis.setRange(Seiscomp::Gui::Range(range.first, range.second));

			r.setLeft(r.right());
			r.setWidth(0);
			spectrogramAxis.updateLayout(painter, r);
			r.setRight(canvasRect().right());
			painter.fillRect(r.adjusted(-axisSpacing(),0,0,0), palette().color(backgroundRole()));
			spectrogramAxis.draw(painter, r, true);

			painter.restore();
		}

		void updateTraceColor() {
			if ( showSpectrogram ) {
				for ( int i = 0; i < slotCount(); ++i )
					setRecordPen(i, QPen(SCScheme.colors.records.spectrogram, SCScheme.records.lineWidth));
			}
			else {
				for ( int i = 0; i < slotCount(); ++i )
					setRecordPen(i, QPen(SCScheme.colors.records.foreground, SCScheme.records.lineWidth));
			}
		}

	protected:
		void paintEvent(QPaintEvent *p) override {
			RecordWidget::paintEvent(p);

			if ( showSpectrogram ) {
				QPainter painter(this);
				painter.setBrush(palette().brush(QPalette::Base));

				switch ( drawMode() ) {
					case InRows:
						for ( int i = 0; i < 3; ++i )
							drawSpectrogramAxis(painter, i);
						break;
					case Single:
						if ( (currentRecords() >= 0) && (currentRecords() < 3) )
							drawSpectrogramAxis(painter, currentRecords());
						break;
					default:
						break;
				}
			}
		}

		void drawCustomBackground(QPainter &painter) override {
			if ( showSpectrogram ) {
				painter.setBrush(palette().brush(QPalette::Base));

				switch ( drawMode() ) {
					case InRows:
						for ( int i = 0; i < 3; ++i )
							drawSpectrogram(painter, i);
						break;
					case Single:
						if ( (currentRecords() >= 0) && (currentRecords() < 3) )
							drawSpectrogram(painter, currentRecords());
						break;
					default:
						break;
				}
			}
		}

		void drawActiveCursor(QPainter &painter, int x, int y) override {
			RecordWidget::drawActiveCursor(painter, x, y);

			if ( !crossHair ) return;
			if ( maxLower <= 0 && maxUpper <= 0 ) return;

			int xl = (int)(maxLower*timeScale());
			int xu = (int)(maxUpper*timeScale());
			painter.drawLine(x-xl+1,y,x+xu,y);

			painter.setPen(palette().color(QPalette::WindowText));
			for ( int i = 0; i < uncertainties.size(); ++i ) {
				double lower = uncertainties[i].first;
				double upper = uncertainties[i].second;

				if ( lower > 0 && xl > 0 ) {
					int x0 = (int)(lower*timeScale());
					int h = 12-10*x0/xl;
					painter.drawLine(x-x0, y-h, x-x0, y+h);
				}

				if ( upper > 0 && xu > 0 ) {
					int x0 = (int)(upper*timeScale());
					int h = 12-10*x0/xu;
					painter.drawLine(x+x0, y-h, x+x0, y+h);
				}
			}

			if ( currentIndex >= 0 ) {
				painter.setPen(QPen(palette().color(QPalette::Highlight), 2));
				double lower = uncertainties[currentIndex].first;
				double upper = uncertainties[currentIndex].second;

				if ( lower > 0 && xl > 0 ) {
					int x0 = (int)(lower*timeScale());
					int h = 12-10*x0/xl;
					painter.drawLine(x-x0, y-h, x-x0, y+h);
				}

				if ( upper > 0 && xu > 0 ) {
					int x0 = (int)(upper*timeScale());
					int h = 12-10*x0/xu;
					painter.drawLine(x+x0, y-h, x+x0, y+h);
				}
			}
		}

	public:
		PickerView::Config::UncertaintyList uncertainties;

	private:
		bool                            crossHair;
		double                          maxLower, maxUpper;
		int                             currentIndex;
		SpectrogramRenderer             spectrogram[3];
		bool                            showSpectrogram;
		Seiscomp::Gui::Axis             spectrogramAxis;
		ThreeComponentTrace::Component *traces;
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
			//return new PickerLabel(item->widget());
			return new PickerRecordLabel;
		}

		void dropEvent(QDropEvent *event) {
			if ( event->mimeData()->hasFormat("text/plain") ) {
				QString strFilter = event->mimeData()->data("text/plain");
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


class PickerMarker : public RecordMarker {
	public:
		enum Type {
			UndefinedType, /* Something undefined */
			Arrival,       /* An associated pick */
			Pick,          /* Just a pick */
			Theoretical    /* A theoretical marker */
		};

	public:
		PickerMarker(RecordWidget *parent,
		             const Seiscomp::Core::Time& pos,
		             Type type, bool newPick)
		: RecordMarker(parent, pos)
		, _type(type)
		, _slot(-1), _rot(RT_123) {
			setMovable(newPick);
			init();
		}

		PickerMarker(RecordWidget *parent,
		             const Seiscomp::Core::Time& pos,
		             const QString& text,
		             Type type, bool newPick)
		: RecordMarker(parent, pos, text)
		, _type(type)
		, _slot(-1), _rot(RT_123) {
			setMovable(newPick);
			init();
		}

		PickerMarker(RecordWidget *parent,
		             const PickerMarker& m)
		: RecordMarker(parent, m)
		, _referencedPick(m._referencedPick)
		, _polarity(m._polarity)
		, _onset(m._onset)
		, _backAzimuth(m._backAzimuth)
		, _slowness(m._slowness)
		, _type(m._type)
		, _slot(m._slot)
		, _rot(m._rot)
		, _channelCode(m._channelCode)
		{
			init();
			_time = m._time;
		}

		virtual ~PickerMarker() {}


	private:
		void init() {
			setMoveCopy(false);
			updateVisual();
			_drawUncertaintyValues = false;
		}


	public:
		void setPhaseCode(const QString &code) {
			if ( pick() ) {
				QString text = code;
				if ( text.isEmpty() ) {
					try {
						text = pick()->phaseHint().code().c_str();
						setText(QString("%1" AUTOMATIC_POSTFIX).arg(text));
					}
					catch (...) {}
				}
				else
					setText(text);

				if ( !pick()->methodID().empty() ) {
					setDescription(QString("%1<%2>")
					               .arg(text)
					               .arg((char)toupper(pick()->methodID()[0])));
				}
				else
					setDescription(QString());
			}
			else if ( !code.isEmpty() )
				setText(code);
		}

		void setEnabled(bool enable) {
			RecordMarker::setEnabled(enable);
			updateVisual();
		}

		void setSlot(int s) {
			_channelCode = std::string();
			_slot = s;
		}

		int slot() const {
			return _slot;
		}

		void setRotation(int r) {
			_channelCode = std::string();
			_rot = r;
		}

		int rotation() const {
			return _rot;
		}

		void setFilter(const QString &filter) {
			_filter = filter;
		}

		const QString &filter() const {
			return _filter;
		}

		void setType(Type t) {
			_type = t;
			if ( _type == Pick ) {
				setPhaseCode("");
				setEnabled(true);
			}
			updateVisual();
		}

		Type type() const {
			return _type;
		}

		bool hasBeenAssociated() const {
			return id() >= 0;
		}

		void setPick(DataModel::Pick *p) {
			_referencedPick = p;
			_channelCode = std::string();
			try { _polarity = p->polarity(); }
			catch ( ... ) { _polarity = Core::None; }
			try { _onset = p->onset(); }
			catch ( ... ) { _onset = Core::None; }
			try { _backAzimuth = p->backazimuth(); }
			catch ( ... ) { _backAzimuth = Core::None; }
			try { _slowness = p->horizontalSlowness(); }
			catch ( ... ) { _slowness = Core::None; }

			_time = p->time();

			updateVisual();
		}

		void convertToManualPick() {
			if ( !_referencedPick ) return;
			_channelCode = _referencedPick->waveformID().channelCode();
			_referencedPick = nullptr;
			setMovable(true);
			setDescription("");
			updateVisual();
		}

		DataModel::Pick *pick() const {
			return _referencedPick.get();
		}

		bool equalsPick(DataModel::Pick *pick) const {
			if ( pick == nullptr ) return false;

			OPT(PickPolarity) pol;
			try { pol = pick->polarity(); } catch ( ... ) {}

			// Polarities do not match: not equal
			if ( pol != _polarity ) return false;

			OPT(PickOnset) onset;
			try { onset = pick->onset(); } catch ( ... ) {}

			// Onsets do not match: not equal
			if ( onset != _onset ) return false;

			// Time + uncertainties do not match: not equal
			if ( _time != pick->time() ) return false;

			return true;
		}

		const std::string &channelCode() const {
			return _channelCode;
		}

		void setPolarity(OPT(PickPolarity) p) {
			_polarity = p;
		}

		OPT(PickPolarity) polarity() const {
			return _polarity;
		}

		void setPickOnset(OPT(PickOnset) o) {
			_onset = o;
		}

		OPT(PickOnset) onset() const {
			return _onset;
		}

		void setBackazimuth(OPT(RealQuantity) baz) {
			_backAzimuth = baz;
		}

		const OPT(RealQuantity) &backazimuth() const {
			return _backAzimuth;
		}

		void setHorizontalSlowness(OPT(RealQuantity) slow) {
			_slowness = slow;
		}

		const OPT(RealQuantity) &horizontalSlowness() const {
			return _slowness;
		}

		void setUncertainty(double lower, double upper) {
			if ( lower == upper ) {
				if ( lower >= 0 )
					_time.setUncertainty(lower);
				else
					_time.setUncertainty(Core::None);

				_time.setLowerUncertainty(Core::None);
				_time.setUpperUncertainty(Core::None);
			}
			else {
				_time.setUncertainty(Core::None);

				if ( lower >= 0 )
					_time.setLowerUncertainty(lower);
				else
					_time.setLowerUncertainty(Core::None);

				if ( upper >= 0 )
					_time.setUpperUncertainty(upper);
				else
					_time.setUpperUncertainty(Core::None);
			}
		}

		double lowerUncertainty() const {
			try {
				return _time.lowerUncertainty();
			}
			catch ( ... ) {
				try {
					return _time.uncertainty();
				}
				catch ( ... ) {}
			}

			return -1;
		}

		double upperUncertainty() const {
			try {
				return _time.upperUncertainty();
			}
			catch ( ... ) {
				try {
					return _time.uncertainty();
				}
				catch ( ... ) {}
			}

			return -1;
		}

		bool hasUncertainty() const {
			return lowerUncertainty() >= 0 && upperUncertainty() >= 0;
		}

		void setDrawUncertaintyValues(bool e) {
			_drawUncertaintyValues = e;
		}


		bool isArrival() const {
			return _type == Arrival;
		}

		bool isPick() const {
			return _type == Pick;
		}

		bool isTheoretical() const {
			return _type == Theoretical;
		}

		RecordMarker *copy() { return new PickerMarker(nullptr, *this); }

		void drawBackground(QPainter &painter, Seiscomp::Gui::RecordWidget *context,
		                    int x, int y1, int y2,
		                    QColor color, qreal lineWidth) {
			double loUncert = lowerUncertainty();
			double hiUncert = upperUncertainty();

			if ( loUncert > 0 || hiUncert > 0 ) {
				QColor barColor(color);
				barColor.setAlpha(64);
				int l = (int)(std::max(loUncert,0.0)*context->timeScale());
				int h = (int)(std::max(hiUncert,0.0)*context->timeScale());

				painter.fillRect(x-l,0,l+h+1,context->height(), barColor);

				if ( _drawUncertaintyValues && context->markerSourceWidget() ) {
					QString str;
					QRect rct;

					QFont font = painter.font();
					font.setBold(false);
					painter.setFont(font);

					if ( loUncert >= 0 ) {
						str.setNum(loUncert, 'G', 4);
						rct = painter.fontMetrics().boundingRect(str);
						rct.adjust(0,0,4,4);
						rct.moveBottomRight(QPoint(x-l,context->height()-1));
						painter.fillRect(rct, context->palette().color(QPalette::Window));
						painter.setPen(context->palette().color(QPalette::WindowText));
						painter.drawRect(rct);
						painter.drawText(rct, Qt::AlignCenter, str);
					}

					if ( hiUncert >= 0 ) {
						str.setNum(hiUncert, 'G', 4);
						rct = painter.fontMetrics().boundingRect(str);
						rct.adjust(0,0,4,4);
						rct.moveBottomLeft(QPoint(x+h,context->height()-1));
						painter.fillRect(rct, context->palette().color(QPalette::Window));
						painter.setPen(context->palette().color(QPalette::WindowText));
						painter.drawRect(rct);
						painter.drawText(rct, Qt::AlignCenter, str);
					}
				}
			}
		}

		void draw(QPainter &painter, RecordWidget *context, int x, int y1, int y2,
		          QColor color, qreal lineWidth) {
			static QPoint poly[3];
			int em = painter.fontMetrics().height();

			painter.setPen(QPen(color, lineWidth));
			painter.drawLine(x, y1, x, y2);

			int onsetOffset = -2;

			if ( _polarity ) {
				int y = y1 + em + 2;
				int height = y2 - y + 1;

				int h = std::min(height, std::max(8, std::min(24, height * 30 / 100)));
				int w = h * 9 / 32;

				switch ( *_polarity ) {
					case POSITIVE:
					{
						bool hasAA = painter.renderHints() & QPainter::Antialiasing;
						painter.setRenderHint(QPainter::Antialiasing, true);
						poly[0] = QPoint(x, y);
						poly[1] = QPoint(x + w, y + h);
						poly[2] = QPoint(x - w, y + h);
						painter.setBrush(color);
						painter.drawPolygon(poly, 3);
						painter.setBrush(Qt::NoBrush);
						painter.setRenderHint(QPainter::Antialiasing, hasAA);
						onsetOffset -= w;
						break;
					}
					case NEGATIVE:
					{
						bool hasAA = painter.renderHints() & QPainter::Antialiasing;
						painter.setRenderHint(QPainter::Antialiasing, true);
						poly[0] = QPoint(x, y + h);
						poly[1] = QPoint(x - w, y);
						poly[2] = QPoint(x + w, y);
						painter.setBrush(color);
						painter.drawPolygon(poly, 3);
						painter.setBrush(Qt::NoBrush);
						painter.setRenderHint(QPainter::Antialiasing, hasAA);
						onsetOffset -= w;
						break;
					}
					case UNDECIDABLE:
						painter.save();
						{
							QFont f = painter.font();
							f.setPixelSize(h);
							f.setBold(true);
							painter.setFont(f);
							painter.drawText(x + 2, y + h, "X");
						}
						painter.restore();
						break;
					default:
						break;
				}
			}

			if ( _onset ) {
				int y = y1 + em + 2;
				int height = y2 - y + 1;
				int h = std::min(height, std::max(8, std::min(24, height * 30 / 100)));

				painter.save();
				QFont f = painter.font();
				f.setPixelSize(h);
				f.setBold(true);
				painter.setFont(f);

				switch ( *_onset ) {
					case EMERGENT:
						painter.drawText(0, y, x + onsetOffset, h, Qt::AlignRight | Qt::AlignTop, "/");
						break;
					case IMPULSIVE:
						painter.drawText(0, y, x + onsetOffset, h, Qt::AlignRight | Qt::AlignTop, "|");
						break;
					case QUESTIONABLE:
						painter.drawText(0, y, x + onsetOffset, h, Qt::AlignRight | Qt::AlignTop, "?");
						break;
					default:
						break;
				}

				painter.restore();
			}

			if ( _backAzimuth ) {
				painter.drawText(x - em - 2, y1, em, em, Qt::AlignRight | Qt::AlignTop, "B");
			}
		}

		QString toolTip() const {
			if ( _type != Pick && _type != Arrival )
				return RecordMarker::toolTip();

			if ( _referencedPick == nullptr )
				return QString("manual %1 pick (local)\n"
				               "filter: %2\n"
				               "arrival: %3")
				       .arg(text())
				       .arg(_filter.isEmpty()?"None":_filter)
				       .arg(isArrival()?"yes":"no");

			QString text;

			try {
				switch ( _referencedPick->evaluationMode() ) {
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

			try {
				text += _referencedPick->phaseHint().code().c_str();
				text += " ";
			}
			catch ( ... ) {
				text += "?? ";
			}

			text += "pick";

			try {
				text += QString(" created by %1").arg(_referencedPick->creationInfo().author().c_str());
			}
			catch ( ... ) {}

			try {
				text += QString(" at %1").arg(timeToString(_referencedPick->creationInfo().creationTime(), "%F %T"));
			}
			catch ( ... ) {}

			if ( !_referencedPick->methodID().empty() )
				text += QString("\nmethod: %1").arg(_referencedPick->methodID().c_str());
			if ( !_referencedPick->filterID().empty() )
				text += QString("\nfilter: %1").arg(_referencedPick->filterID().c_str());
			try {
				double baz = _referencedPick->backazimuth().value();
				text += QString("\nbackazimuth: %1°").arg(baz);
			}
			catch ( ... ) {}
			try {
				double hs = _referencedPick->horizontalSlowness().value();
				text += QString("\nhoriz. slowness: %1 deg/s").arg(hs);
			}
			catch ( ... ) {}

			text += QString("\narrival: %1").arg(isArrival()?"yes":"no");

			return text;
		}

	private:
		void updateVisual() {
			QColor col = SCScheme.colors.picks.disabled;
			Qt::Alignment al = Qt::AlignVCenter;
			EvaluationMode state = AUTOMATIC;

			DataModel::Pick *p = pick();
			if ( p ) {
				try { state = p->evaluationMode(); } catch ( ... ) {}
			}

			if ( isMovable() )
				state = MANUAL;

			switch ( _type ) {
				case Arrival:
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

					al = isMovable()?Qt::AlignVCenter:Qt::AlignTop;
					break;

				case Pick:
					if ( isEnabled() ) {
						switch ( state ) {
							case MANUAL:
								col = SCScheme.colors.picks.manual;
								break;
							case AUTOMATIC:
							default:
								col = SCScheme.colors.picks.automatic;
								break;
						}
					}
					else
						col = SCScheme.colors.picks.disabled;

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
		PickPtr           _referencedPick;
		OPT(PickPolarity) _polarity;
		OPT(PickOnset)    _onset;
		OPT(RealQuantity) _backAzimuth;
		OPT(RealQuantity) _slowness;
		TimeQuantity      _time;
		QString           _filter;
		Type              _type;
		int               _slot;
		int               _rot;
		bool              _drawUncertaintyValues;
		std::string       _channelCode;
};


class SpectrumView : public SpectrumViewBase {
	public:
		enum WindowFunc {
			None,
			Cosine,
			Hamming,
			Hann
		};

		SpectrumView(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		: SpectrumViewBase(parent, f)
		, _windowFunc(None)
		, _windowWidth(5) {
			QFrame *frame = new QFrame;
			QHBoxLayout *hl = new QHBoxLayout;

			spectrumWidget = new SpectrumWidget;
			_infoLabel = new QLabel;

			QComboBox *windowCombo = new QComboBox;
			windowCombo->addItem(tr("Boxcar window"));
			windowCombo->addItem(tr("Cosine window"));
			windowCombo->addItem(tr("Hamming window"));
			windowCombo->addItem(tr("Hann window"));
			connect(windowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(windowFuncChanged(int)));

			QDoubleSpinBox *spinWindowWidth = new QDoubleSpinBox;
			spinWindowWidth->setToolTip(tr("The data portion in percent at either side where the window function is applied on."));
			spinWindowWidth->setRange(0, 50);
			spinWindowWidth->setSuffix("%");
			spinWindowWidth->setValue(_windowWidth);
			connect(spinWindowWidth, SIGNAL(valueChanged(double)), this, SLOT(windowWidthChanged(double)));

			QComboBox *modeCombo = new QComboBox;
			modeCombo->addItem(tr("Amplitude spectrum"));
			modeCombo->addItem(tr("Power spectrum"));
			modeCombo->addItem(tr("Phase spectrum"));
			connect(modeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));

			QToolButton *toggleLogX = new QToolButton;
			toggleLogX->setText(tr("Log scale X"));
			toggleLogX->setCheckable(true);
			toggleLogX->setChecked(true);
			connect(toggleLogX, SIGNAL(toggled(bool)), spectrumWidget, SLOT(setLogScaleX(bool)));

			_toggleLogY = new QToolButton;
			_toggleLogY->setText(tr("Log scale Y"));
			_toggleLogY->setCheckable(true);
			_toggleLogY->setChecked(true);
			connect(_toggleLogY, SIGNAL(toggled(bool)), spectrumWidget, SLOT(setLogScaleY(bool)));

			hl->addWidget(toggleLogX);
			hl->addWidget(_toggleLogY);
			hl->addWidget(windowCombo);
			hl->addWidget(spinWindowWidth);
			hl->addWidget(modeCombo);
			hl->addStretch();

			QToolButton *toggleSpec = new QToolButton;
			toggleSpec->setText(tr("Raw spectrum"));
			toggleSpec->setCheckable(true);
			toggleSpec->setChecked(true);
			connect(toggleSpec, SIGNAL(toggled(bool)), spectrumWidget, SLOT(setShowSpectrum(bool)));

			QToolButton *toggleCorrSpec = new QToolButton;
			toggleCorrSpec->setText(tr("Corrected spectrum"));
			toggleCorrSpec->setCheckable(true);
			toggleCorrSpec->setChecked(false);
			connect(toggleCorrSpec, SIGNAL(toggled(bool)), spectrumWidget, SLOT(setShowCorrected(bool)));

			QToolButton *toggleResp = new QToolButton;
			toggleResp->setText(tr("Response"));
			toggleResp->setCheckable(true);
			toggleResp->setChecked(false);
			connect(toggleResp, SIGNAL(toggled(bool)), spectrumWidget, SLOT(setShowResponse(bool)));

			hl->addWidget(toggleSpec);
			hl->addWidget(toggleCorrSpec);
			hl->addWidget(toggleResp);

			QVBoxLayout *vl = new QVBoxLayout;

			vl->addWidget(_infoLabel);
			vl->addWidget(frame);
			vl->addLayout(hl);

			hl = new QHBoxLayout;
			QPushButton *exportButton = new QPushButton;
			exportButton->setText(tr("Export"));
			connect(exportButton, SIGNAL(clicked()), spectrumWidget, SLOT(exportSpectra()));
			hl->addWidget(exportButton);
			hl->addStretch();
			QPushButton *closeButton = new QPushButton;
			closeButton->setText(tr("Close"));
			connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
			hl->addWidget(closeButton);
			vl->addLayout(hl);

			setLayout(vl);

			frame->setFrameShadow(QFrame::Sunken);
			frame->setFrameShape(QFrame::StyledPanel);

			vl = new QVBoxLayout;
			vl->setMargin(0);
			vl->addWidget(spectrumWidget);
			frame->setLayout(vl);

			spectrumWidget->setLogScaleX(toggleLogX->isChecked());
			spectrumWidget->setLogScaleY(_toggleLogY->isChecked());
			spectrumWidget->setShowSpectrum(toggleSpec->isChecked());
			spectrumWidget->setShowCorrected(toggleCorrSpec->isChecked());
			spectrumWidget->setShowResponse(toggleResp->isChecked());
		}

		void setData(Record *rec, Processing::Sensor *sensor) {
			_trace = rec;
			_response = sensor && sensor->response() ? sensor->response() : nullptr;

			setInfo(sensor);
			updateData();
		}


		void setInfo(Processing::Sensor *sensor) {
			if ( sensor == nullptr ) {
				_infoLabel->setText(QString());
				return;
			}

			_infoLabel->setText(QString("%1, %2, %3, %4")
			                    .arg(sensor->manufacturer().c_str())
			                    .arg(sensor->model().c_str())
			                    .arg(sensor->type().c_str())
			                    .arg(sensor->unit().c_str()));
		}


	private:
		void windowFuncChanged(int index) {
			switch ( index ) {
				case 0:
					_windowFunc = None;
					break;
				case 1:
					_windowFunc = Cosine;
					break;
				case 2:
					_windowFunc = Hamming;
					break;
				case 3:
					_windowFunc = Hann;
					break;
				default:
					break;
			}

			updateData();
		}


		void windowWidthChanged(double value) {
			_windowWidth = value;
			if ( _windowWidth < 0 )
				_windowWidth = 0;
			if ( _windowWidth > 50 )
				_windowWidth = 50;

			updateData();
		}


		void modeChanged(int index) {
			switch ( index ) {
				case 0:
					spectrumWidget->setAmplitudeSpectrum();
					_toggleLogY->setChecked(true);
					break;
				case 1:
					spectrumWidget->setPowerSpectrum();
					_toggleLogY->setChecked(true);
					break;
				case 2:
					spectrumWidget->setPhaseSpectrum();
					_toggleLogY->setChecked(false);
					break;
				default:
					break;
			}
		}


		void updateData() {
			if ( !_trace ) return;

			std::vector<double> data(static_cast<const DoubleArray*>(_trace->data())->impl());
			double wwidth = _windowWidth * 0.01;

			switch ( _windowFunc ) {
				case Cosine:
					Math::CosineWindow<double>().apply(data, wwidth);
					break;
				case Hamming:
					Math::HammingWindow<double>().apply(data, wwidth);
					break;
				case Hann:
					Math::HannWindow<double>().apply(data, wwidth);
					break;
				default:
					break;
			}

			Math::ComplexArray spectrum;
			Math::fft(spectrum, data);

			spectrumWidget->setSpectrum(_trace->samplingFrequency()*0.5,
			                            spectrum, _response.get(),
			                            _trace->streamID().c_str());
		}


	private:
		QLabel                  *_infoLabel;
		QToolButton             *_toggleLogY;
		RecordPtr                _trace;
		Processing::ResponsePtr  _response;
		WindowFunc               _windowFunc;
		double                   _windowWidth;


	public:
		SpectrumWidget *spectrumWidget;
};



class PickerTimeWindowDecorator : public RecordWidgetDecorator {
	public:
		PickerTimeWindowDecorator(QObject *parent = 0)
		: RecordWidgetDecorator(parent) {}

	public:
		void setVisible(bool visible) {
			_visible = visible;
		}

		void setTimeWindowAndSNR(const Core::TimeWindow &tw, double snr) {
			_timeWindow = tw;
			_snr = snr;
		}

		void drawDecoration(QPainter *painter, RecordWidget *widget) override {
			if ( !_visible ) {
				return;
			}

			painter->setPen(SCScheme.colors.arrivals.theoretical);

			int x0 = widget->mapCanvasTime(_timeWindow.startTime());
			int x1 = widget->mapCanvasTime(_timeWindow.endTime());

			bool clippedX0 = false;
			bool clippedX1 = false;

			if ( x0 < 0 ) {
				x0 = 0;
				clippedX0 = true;
			}

			if ( x1 >= widget->canvasRect().width() ) {
				x1 = widget->canvasRect().width() - 1;
				clippedX1 = true;
			}

			int left = widget->canvasRect().left() + x0;
			int right = widget->canvasRect().left() + x1;
			int bottom = widget->canvasRect().bottom();
			int top = widget->canvasRect().top();

			QColor c = SCScheme.colors.arrivals.theoretical;
			c.setAlpha(32);
			painter->fillRect(left, top, right-left, bottom-top, c);
			painter->drawLine(left, top, right, top);
			painter->drawLine(left, bottom, right, bottom);

			if ( !clippedX0 ) {
				painter->drawLine(left, bottom, left, top);
			}

			if ( !clippedX1 ) {
				painter->drawLine(right, bottom, right, top);
			}

			painter->drawText(widget->canvasRect().left() + x0 + 4,
			                  widget->canvasRect().top(),
			                  x1 - x0,
			                  widget->canvasRect().height() - 4,
			                  Qt::AlignLeft | Qt::AlignBottom,
			                  QString("SNR: %1").arg(_snr, 0, 'f', 1));
		}

	private:
		bool             _visible{false};
		Core::TimeWindow _timeWindow;
		double           _snr{-1};
};


bool isTraceUsed(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		if ( !m->isEnabled() ) continue;
		if ( m->type() == PickerMarker::Arrival ) return true;
	}

	return false;
}


bool isTracePicked(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		if ( m->type() == PickerMarker::Arrival ) return true;
	}

	return false;
}


bool isArrivalTrace(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		if ( m->pick() && m->id() >= 0 ) return true;
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


Stream *findStream(Station *station, const std::string &code,
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


Stream *findStream(Station *station, const Seiscomp::Core::Time &time,
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

			//Sensor *sensor = Sensor::Find(stream->sensor());
			//if ( !sensor ) continue;

			Processing::WaveformProcessor::SignalUnit unit;

			// Unable to retrieve the unit enumeration from string
			//if ( !unit.fromString(sensor->unit().c_str()) ) continue;
			std::string gainUnit = stream->gainUnit();
			std::transform(gainUnit.begin(), gainUnit.end(), gainUnit.begin(), ::toupper);
			if ( !unit.fromString(gainUnit.c_str()) ) continue;
			if ( unit != requestedUnit ) continue;

			return stream;
		}
	}

	return nullptr;
}


Stream *findConfiguredStream(Station *station, const Seiscomp::Core::Time &time) {
	DataModel::Stream *stream = nullptr;
	DataModel::ConfigModule *module = SCApp->configModule();
	if ( module ) {
		for ( size_t ci = 0; ci < module->configStationCount(); ++ci ) {
			DataModel::ConfigStation* cs = module->configStation(ci);
			if ( cs->networkCode() == station->network()->code() &&
			     cs->stationCode() == station->code() ) {
				DataModel::Setup *setup = findSetup(cs, SCApp->name(), true);
				if ( setup ) {
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
					if ( !cha.empty() ) {
						stream = findStream(station, cha, loc, time);
						if ( stream )
							return stream;
					}
				}
			}
		}
	}

	return stream;
}


Pick *findPick(Seiscomp::Gui::RecordWidget *w, const Seiscomp::Core::Time &t) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		Pick* p = m->pick();
		if ( p && p->time() == t )
			return p;
	}

	return nullptr;
}


std::string adjustChannelCode(const std::string &channelCode, bool allComponents) {
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

QString waveformIDToQString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode()).c_str();
}


std::string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}


bool isLinkedItem(RecordViewItem *item) {
	return static_cast<PickerRecordLabel*>(item->label())->isLinkedItem();
}


void unlinkItem(RecordViewItem *item) {
	static_cast<PickerRecordLabel*>(item->label())->unlink();
	item->disconnect(SIGNAL(firstRecordAdded(const Seiscomp::Record*)));
}


void selectFirstVisibleItem(RecordView *view) {
	for ( int i = 0; i < view->rowCount(); ++i ) {
		if ( !static_cast<PickerRecordLabel*>(view->itemAt(i)->label())->isLinkedItem() ) {
			view->setCurrentItem(view->itemAt(i));
			view->ensureVisible(i);
			break;
		}
	}
}


#define CFG_LOAD_PICKS SC_D.ui.actionShowUnassociatedPicks->isChecked()


}


namespace Seiscomp {
namespace Gui {
namespace PrivatePickerView {


ThreeComponentTrace::~ThreeComponentTrace() {
	for ( int i = 0; i < 3; ++i ) {
		if ( traces[i].raw ) delete traces[i].raw;
		if ( widget ) widget->setRecords(i, nullptr);
		if ( traces[i].transformed ) delete traces[i].transformed;
	}
}


void ThreeComponentTrace::reset() {
	for ( int i = 0; i < 3; ++i ) {
		traces[i].filter.reset();

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = nullptr;

			if ( widget ) {
				widget->setRecords(i, nullptr);
				widget->setRecordStatus(i, false, QString());
			}
		}
	}

	transform();
}


void ThreeComponentTrace::setFilter(RecordWidget::Filter *f) {
	for ( int i = 0; i < 3; ++i ) {
		traces[i].filter = f?f->clone():nullptr;

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = nullptr;

			if ( widget ) {
				widget->setRecords(i, nullptr);
				widget->setRecordStatus(i, false, QString());
			}
		}
	}

	transform();
}


void ThreeComponentTrace::setRecordWidget(RecordWidget *w) {
	if ( widget ) {
		for ( int i = 0; i < 3; ++i ) widget->setRecords(i, nullptr);
		widget->disconnect(this);
	}

	widget = w;

	if ( widget ) {
		for ( int i = 0; i < 3; ++i ) {
			widget->setRecords(i, traces[i].transformed, false);
			widget->setRecordStatus(i, false, traces[i].filter.lastError().c_str());
		}
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));
	}
}


void ThreeComponentTrace::widgetDestroyed(QObject *obj) {
	if ( obj == widget )
		widget = nullptr;
}


void ThreeComponentTrace::setTransformationEnabled(bool f) {
	bool needTransformation = false;

	enableTransformation = f;

	for ( int i = 0; i < 3; ++i ) {
		traces[i].filter.reset();

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = nullptr;

			if ( widget ) widget->setRecords(i, nullptr);
		}

		if ( enableTransformation ) {
			Math::Vector3d r = transformation.row(2-i);
			bool passthrough = true;

			if ( enableL2Horizontals && i > 0 )
				passthrough = false;
			else {
				for ( int j = 0; j < 3; ++j ) {
					if ( j == 2-i ) {
						if ( fabs(r[j]-1.0) > 1E-6 )
							passthrough = false;
					}
					else {
						if ( fabs(r[j]) > 1E-6 )
							passthrough = false;
					}
				}
			}

			setPassThrough(i, passthrough);
			if ( !passthrough )
				needTransformation = true;
		}
	}

	enableTransformation = needTransformation;

	transform();
}


void ThreeComponentTrace::setL2Horizontals(bool f) {
	enableL2Horizontals = f;
}


void ThreeComponentTrace::setPassThrough(int component, bool enable) {
	traces[component].passthrough = enable;
}


bool ThreeComponentTrace::transform(int comp, Seiscomp::Record *rec) {
	bool gotRecords = false;

	// Record passed that needs filtering?
	if ( rec ) {
		if ( traces[comp].passthrough || !enableTransformation ) {
			if ( traces[comp].transformed == nullptr ) {
				traces[comp].transformed = new RingBuffer(0);
				if ( widget ) {
					widget->setRecords(comp, traces[comp].transformed, false);
				}
			}

			RecordPtr grec = traces[comp].filter.feed(rec);

			if ( grec ) {
				traces[comp].transformed->feed(grec.get());
				if ( widget ) {
					widget->fed(comp, grec.get());
				}
				gotRecords = true;
			}
			else {
				if ( widget ) {
					widget->setRecordStatus(comp, false, traces[comp].filter.lastError().c_str());
				}
			}
		}
	}
	else {
		// Just copy the records and filter them if activated
		for ( int i = 0; i < 3; ++i ) {
			if ( enableTransformation && !traces[i].passthrough ) continue;
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

				const Record *s_rec = s_it->get();

				if ( traces[i].transformed == nullptr ) {
					traces[i].transformed = new RingBuffer(0);
					if ( widget ) {
						widget->setRecords(i, traces[i].transformed, false);
					}
				}

				RecordPtr grec = traces[i].filter.feed(s_rec);
				if ( grec ) {
					traces[i].transformed->feed(grec.get());
					if ( widget ) {
						widget->fed(i, grec.get());
					}
					gotRecords = true;
				}
				else {
					if ( widget ) {
						widget->setRecordStatus(i, false, traces[i].filter.lastError().c_str());
					}
				}
			}
		}
	}

	if ( enableTransformation ) {
		Core::Time minStartTime;
		Core::Time maxStartTime;
		Core::Time minEndTime;

		// Not all traces available, nothing to do
		for ( int i = 0; i < 3; ++i ) {
			if ( !traces[i].passthrough && traces[i].transformed ) {
				Core::Time endTime = traces[i].transformed->back()->endTime();
				if ( endTime > minStartTime )
					minStartTime = endTime;
			}

			if ( !traces[i].raw || traces[i].raw->empty() )
				return gotRecords;
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

				for ( auto rec_it = it[i]; rec_it != seq_end; ++rec_it ) {
					const Array *rec_data = (*rec_it)->data();
					if ( !rec_data ) {
						SEISCOMP_ERROR("%s: no data for record", (*rec_it)->streamID().c_str());
						return gotRecords;
					}

					if ( (*rec_it)->startTime() > minEndTime )
						break;

					++it[i];

					const DoubleArray *srcData = DoubleArray::ConstCast(rec_data);
					DoubleArrayPtr tmp;
					if ( !srcData ) {
						tmp = static_cast<DoubleArray*>(rec_data->copy(Array::DOUBLE));
						srcData = tmp.get();
					}

					int startIndex = 0;
					int endIndex = srcData->size();

					if ( (*rec_it)->startTime() < maxStartTime )
						startIndex += (int)(double(maxStartTime-(*rec_it)->startTime())*(*rec_it)->samplingFrequency()+0.5);

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

			// Trim record sizes
			for ( int i = 0; i < 3; ++i ) {
				DoubleArray *data = static_cast<DoubleArray*>(comps[i]->data());
				if ( data->size() > minLen ) {
					data->resize(minLen);
					comps[i]->dataUpdated();
				}
			}

			gotRecords = true;

			auto dataZ = static_cast<DoubleArray*>(comps[0]->data())->typedData();
			auto data1 = static_cast<DoubleArray*>(comps[1]->data())->typedData();
			auto data2 = static_cast<DoubleArray*>(comps[2]->data())->typedData();

			// Rotate finally
			for ( int i = 0; i < minLen; ++i ) {
				Math::Vector3d v = transformation * Math::Vector3d(*data2, *data1, *dataZ);
				*dataZ = v.z;
				*data1 = v.y;
				*data2 = v.x;

				++dataZ; ++data1; ++data2;
			}

			if ( enableL2Horizontals ) {
				auto data1 = static_cast<DoubleArray*>(comps[1]->data())->typedData();
				auto data2 = static_cast<DoubleArray*>(comps[2]->data())->typedData();

				for ( int i = 0; i < minLen; ++i ) {
					double rms = sqrt(*data1 * *data1 + *data2 * *data2);
					*data1 = rms;
					*data2 = 0;

					++data1; ++data2;
				}
			}

			// And filter
			for ( int i = 0; i < 3; ++i ) {
				if ( traces[i].passthrough ) continue;
				if ( !traces[i].filter.apply(comps[i].get()) ) {
					comps[i] = nullptr;
					if ( widget ) {
						widget->setRecordStatus(i, false, traces[i].filter.lastError().c_str());
					}
				}
			}

			// Create record sequences
			for ( int i = 0; i < 3; ++i ) {
				if ( traces[i].passthrough ) continue;
				if ( !comps[i] ) {
					if ( traces[i].transformed ) {
						delete traces[i].transformed;
						traces[i].transformed = nullptr;
					}

					if ( widget ) {
						widget->setRecords(i, traces[i].transformed, false);
					}

					continue;
				}

				// Create ring buffer without limit if needed
				if ( !traces[i].transformed ) {
					traces[i].transformed = new RingBuffer(0);
					if ( widget ) {
						widget->setRecords(i, traces[i].transformed, false);
					}
				}

				traces[i].transformed->feed(comps[i].get());
				if ( widget ) {
					widget->fed(i, comps[i].get());
				}
			}

			minStartTime = minEndTime;
		}
	}

	return gotRecords;
}


PickerRecordLabel::PickerRecordLabel(int items, QWidget *parent, const char* name)
: StandardRecordLabel(items, parent, name), _isLinkedItem(false), _isExpanded(false) {
	_btnExpand = nullptr;
	_linkedItem = nullptr;

	latitude = 999;
	longitude = 999;

	unit = UT_RAW;

	hasGotData = false;
	isEnabledByConfig = false;
}

PickerRecordLabel::~PickerRecordLabel() {}

void PickerRecordLabel::setConfigState(bool state) {
	isEnabledByConfig = state;
}

void PickerRecordLabel::setLinkedItem(bool li) {
	_isLinkedItem = li;
}

void PickerRecordLabel::setControlledItem(RecordViewItem *controlledItem) {
	_linkedItem = controlledItem;
	static_cast<PickerRecordLabel*>(controlledItem->label())->_linkedItem = recordViewItem();
}

RecordViewItem *PickerRecordLabel::controlledItem() const {
	return _linkedItem;
}

void PickerRecordLabel::enabledExpandButton(RecordViewItem *controlledItem) {
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

void PickerRecordLabel::disableExpandButton() {
	if ( _btnExpand ) {
		delete _btnExpand;
		_btnExpand = nullptr;
	}

	_linkedItem = nullptr;
}

void PickerRecordLabel::unlink() {
	if ( _linkedItem ) {
		static_cast<PickerRecordLabel*>(_linkedItem->label())->disableExpandButton();
		_linkedItem = nullptr;
	}
}

bool PickerRecordLabel::isLinkedItem() const {
	return _isLinkedItem;
}

bool PickerRecordLabel::isExpanded() const {
	return _isExpanded;
}


void PickerRecordLabel::visibilityChanged(bool v) {
	if ( _linkedItem && !_isLinkedItem ) {
		if ( !v )
			_linkedItem->setVisible(false);
		else if ( _isExpanded )
			_linkedItem->setVisible(true);
	}
}

void PickerRecordLabel::resizeEvent(QResizeEvent *e) {
	StandardRecordLabel::resizeEvent(e);
	if ( _btnExpand ) {
		_btnExpand->move(e->size().width() - _btnExpand->width(),
		                 e->size().height() - _btnExpand->height());
	}
}

void PickerRecordLabel::paintEvent(QPaintEvent *e) {
	QPainter p(this);

	int fontSize = p.fontMetrics().ascent();

	if ( _hasLabelColor ) {
		QRect r(rect());

		r.setLeft(r.right()-16);

		QLinearGradient gradient(r.left(), 0, r.right(), 0);
		gradient.setColorAt(0, palette().color(QPalette::Window));
		gradient.setColorAt(1, _labelColor);

		p.fillRect(r, gradient);
	}

	if ( !isEnabledByConfig ) {
		QRect r(rect());

		r.setRight(r.left()+16);
		r.setTop(r.bottom()-fontSize);

		QLinearGradient gradient(r.left(), 0, r.right(), 0);
		gradient.setColorAt(0, QColor(192,0,0));
		gradient.setColorAt(1, palette().color(QPalette::Window));

		p.fillRect(r, gradient);
	}

	if ( _items.count() == 0 ) return;

	int w = width();
	int h = height();

	int posX = 0;
	int posY = (h - fontSize * 2 - 4) / 2;

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

void PickerRecordLabel::enableExpandable(const Seiscomp::Record *rec) {
	enabledExpandButton(static_cast<RecordViewItem*>(sender()));
}

void PickerRecordLabel::extentButtonPressed() {
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

void PickerRecordLabel::setLabelColor(QColor c) {
	_labelColor = c;
	_hasLabelColor = true;
	update();
}


void PickerRecordLabel::removeLabelColor() {
	_hasLabelColor = false;
	update();
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::Config::Config() {
	timingQualityLow = Qt::darkRed;
	timingQualityMedium = Qt::yellow;
	timingQualityHigh = Qt::darkGreen;

	defaultDepth = 10;
	alignmentPosition = 0.5;
	offsetWindowStart = 0;
	offsetWindowEnd = 0;

	hideDisabledStations = false;

	onlyApplyIntegrationFilterOnce = true;
	ignoreDisabledStations = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::Config::getPickPhases(StringList &phases) const {
	getPickPhases(phases, phaseGroups);
	foreach ( const QString &ph, favouritePhases ) {
		if ( !phases.contains(ph) ) phases.append(ph);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::Config::getPickPhases(StringList &phases, const QList<PhaseGroup> &groups) const {
	foreach ( const PhaseGroup &g, groups ) {
		if ( g.childs.empty() ) {
			if ( !phases.contains(g.name) ) phases.append(g.name);
		}
		else
			getPickPhases(phases, g.childs);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::PickerView(QWidget *parent, Qt::WindowFlags f)
: QMainWindow(parent,f)
, _d_ptr(new PickerViewPrivate) {
	SC_D.recordView = new TraceList();
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::PickerView(const Seiscomp::Core::TimeWindow& tw,
                       QWidget *parent, Qt::WindowFlags f)
: QMainWindow(parent, f)
, _d_ptr(new PickerViewPrivate) {
	SC_D.recordView = new TraceList(tw);
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::PickerView(const Seiscomp::Core::TimeSpan& ts,
                       QWidget *parent, Qt::WindowFlags f)
: QMainWindow(parent, f)
, _d_ptr(new PickerViewPrivate) {

	SC_D.recordView = new TraceList(ts);
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::~PickerView() {
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i )
		SC_D.recordView->itemAt(i)->widget()->setShadowWidget(nullptr, false);

	if ( SC_D.currentFilter ) {
		delete SC_D.currentFilter;
	}

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
RecordLabel* PickerView::createLabel(RecordViewItem *item) const {
	//return new PickerLabel(item->widget());
	return new PickerRecordLabel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::init() {
	setObjectName("Picker");

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	SC_D.ui.setupUi(this);

	QFont f(font());
	f.setBold(true);
	SC_D.ui.labelStationCode->setFont(f);

	//setContextMenuPolicy(Qt::ActionsContextMenu);
	//SC_D.recordView->setMinimumRowHeight(70);

	//SC_D.ttTable.setBranch("P");

	SC_D.currentRotationMode = RT_123;
	SC_D.currentUnitMode = UT_RAW;
	SC_D.settingsRestored = false;
	SC_D.currentSlot = -1;
	SC_D.currentFilter = nullptr;
	SC_D.autoScaleZoomTrace = true;
	SC_D.loadedPicks = false;

	SC_D.reader = nullptr;

	SC_D.zoom = 1.0;
	SC_D.currentAmplScale = 1.0;

	SC_D.centerSelection = false;
	SC_D.checkVisibility = true;

	SC_D.spectrumView = nullptr;

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

	SC_D.connectionState = new ConnectionStateLabel(this);
	connect(SC_D.connectionState, SIGNAL(customInfoWidgetRequested(const QPoint &)),
	        this, SLOT(openConnectionInfo(const QPoint &)));

	QWidget *wrapper = new QWidget;
	wrapper->setBackgroundRole(QPalette::Base);
	wrapper->setAutoFillBackground(true);

	QBoxLayout* layout = new QVBoxLayout(SC_D.ui.framePickList);
	layout->setMargin(0);
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

	SC_D.currentRecord = new ZoomRecordWidget();
	SC_D.currentRecord->showScaledValues(SC_D.ui.actionShowTraceValuesInNmS->isChecked());
	SC_D.currentRecord->setClippingEnabled(SC_D.ui.actionClipComponentsToViewport->isChecked());
	SC_D.currentRecord->setMouseTracking(true);
	SC_D.currentRecord->setContextMenuPolicy(Qt::CustomContextMenu);
	SC_D.currentRecord->setRowSpacing(6);
	SC_D.currentRecord->setAxisSpacing(6);
	SC_D.currentRecord->setDrawAxis(true);
	SC_D.currentRecord->setDrawSPS(true);
	SC_D.currentRecord->setAxisPosition(RecordWidget::Left);

	//SC_D.currentRecord->setFocusPolicy(Qt::StrongFocus);

	//SC_D.currentRecord->setDrawMode(RecordWidget::Single);
	//SC_D.currentRecord->setDrawMode(RecordWidget::InRows);
	/*
	SC_D.currentRecord->setRecordColor(0, Qt::red);
	SC_D.currentRecord->setRecordColor(1, Qt::green);
	SC_D.currentRecord->setRecordColor(2, Qt::blue);
	*/

	connect(SC_D.currentRecord, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(openRecordContextMenu(const QPoint &)));
	connect(SC_D.currentRecord, SIGNAL(currentMarkerChanged(Seiscomp::Gui::RecordMarker*)),
	        this, SLOT(currentMarkerChanged(Seiscomp::Gui::RecordMarker*)));

	layout = new QVBoxLayout(SC_D.ui.frameCurrentRow);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(SC_D.currentRecord);

	SC_D.timeScale = new TimeScale();
	SC_D.timeScale->setSelectionEnabled(false);
	SC_D.timeScale->setSelectionHandleCount(2);
	SC_D.timeScale->setAbsoluteTimeEnabled(true);
	SC_D.timeScale->setRangeSelectionEnabled(true);

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
	connect(SC_D.timeScale, SIGNAL(selectionHandleMoved(int, double, Qt::KeyboardModifiers)),
	        this, SLOT(zoomSelectionHandleMoved(int, double, Qt::KeyboardModifiers)));
	connect(SC_D.timeScale, SIGNAL(selectionHandleMoveFinished()),
	        this, SLOT(zoomSelectionHandleMoveFinished()));

	connect(SC_D.recordView->timeWidget(), SIGNAL(dragged(double)),
	        this, SLOT(moveTraces(double)));

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
	addAction(SC_D.ui.actionSortByAzimuth);
	addAction(SC_D.ui.actionSortByResidual);

	addAction(SC_D.ui.actionShowZComponent);
	addAction(SC_D.ui.actionShowNComponent);
	addAction(SC_D.ui.actionShowEComponent);

	addAction(SC_D.ui.actionAlignOnOriginTime);
	addAction(SC_D.ui.actionAlignOnPArrival);
	addAction(SC_D.ui.actionAlignOnSArrival);

	addAction(SC_D.ui.actionToggleFilter);
	addAction(SC_D.ui.actionNextFilter);
	addAction(SC_D.ui.actionPreviousFilter);
	addAction(SC_D.ui.actionMaximizeAmplitudes);

	addAction(SC_D.ui.actionPickP);
	addAction(SC_D.ui.actionPickS);
	addAction(SC_D.ui.actionDisablePicking);

	addAction(SC_D.ui.actionCreatePick);
	addAction(SC_D.ui.actionConfirmPick);
	addAction(SC_D.ui.actionSetPick);
	addAction(SC_D.ui.actionDeletePick);

	addAction(SC_D.ui.actionShowZComponent);
	addAction(SC_D.ui.actionShowNComponent);
	addAction(SC_D.ui.actionShowEComponent);

	addAction(SC_D.ui.actionRepickAutomatically);
	addAction(SC_D.ui.actionGotoNextMarker);
	addAction(SC_D.ui.actionGotoPreviousMarker);

	addAction(SC_D.ui.actionSetPolarityPositive);
	addAction(SC_D.ui.actionSetPolarityNegative);
	addAction(SC_D.ui.actionSetPolarityUndecidable);
	addAction(SC_D.ui.actionSetPolarityUnset);

	addAction(SC_D.ui.actionSetPickOnsetEmergent);
	addAction(SC_D.ui.actionSetPickOnsetImpulsive);
	addAction(SC_D.ui.actionSetPickOnsetQuestionable);
	addAction(SC_D.ui.actionSetPickOnsetUnset);

	addAction(SC_D.ui.actionRelocate);
	addAction(SC_D.ui.actionSwitchFullscreen);
	addAction(SC_D.ui.actionAddStations);
	addAction(SC_D.ui.actionSearchStation);

	addAction(SC_D.ui.actionShowSpectrogram);

	SC_D.lastFilterIndex = 0;

	SC_D.comboFilter = new QComboBox;
	//SC_D.comboFilter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	SC_D.comboFilter->setDuplicatesEnabled(false);
	SC_D.comboFilter->addItem(NO_FILTER_STRING);

	SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
	changeFilter(SC_D.comboFilter->currentIndex());

	SC_D.ui.toolBarFilter->insertWidget(SC_D.ui.actionToggleFilter, SC_D.comboFilter);

	SC_D.comboRotation = new QComboBox;
	//SC_D.comboRotation->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	SC_D.comboRotation->setDuplicatesEnabled(false);
	for ( int i = 0; i < RotationType::Quantity; ++i )
		SC_D.comboRotation->addItem(ERotationTypeNames::name(i));
	SC_D.comboRotation->setCurrentIndex(SC_D.currentRotationMode);

	SC_D.ui.toolBarFilter->insertWidget(SC_D.ui.actionToggleFilter, SC_D.comboRotation);

	SC_D.comboUnit = new QComboBox;
	SC_D.comboUnit->setDuplicatesEnabled(false);
	for ( int i = 0; i < UnitType::Quantity; ++i )
		SC_D.comboUnit->addItem(EUnitTypeNames::name(i));
	SC_D.comboUnit->setCurrentIndex(SC_D.currentUnitMode);

	SC_D.ui.toolBarFilter->insertWidget(SC_D.ui.actionToggleFilter, SC_D.comboUnit);

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

	connect(SC_D.ui.actionSetPolarityPositive, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));
	connect(SC_D.ui.actionSetPolarityNegative, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));
	connect(SC_D.ui.actionSetPolarityUndecidable, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));
	connect(SC_D.ui.actionSetPolarityUnset, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));

	connect(SC_D.ui.actionSetPickOnsetEmergent, SIGNAL(triggered(bool)),
	        this, SLOT(setPickOnset()));
	connect(SC_D.ui.actionSetPickOnsetImpulsive, SIGNAL(triggered(bool)),
	        this, SLOT(setPickOnset()));
	connect(SC_D.ui.actionSetPickOnsetQuestionable, SIGNAL(triggered(bool)),
	        this, SLOT(setPickOnset()));
	connect(SC_D.ui.actionSetPickOnsetUnset, SIGNAL(triggered(bool)),
	        this, SLOT(setPickOnset()));

	connect(SC_D.comboFilter, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeFilter(int)));
	connect(SC_D.comboRotation, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeRotation(int)));
	connect(SC_D.comboUnit, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeUnit(int)));

	connect(SC_D.ui.actionLimitFilterToZoomTrace, SIGNAL(triggered(bool)),
	        this, SLOT(limitFilterToZoomTrace(bool)));

	connect(SC_D.ui.actionShowTheoreticalArrivals, SIGNAL(triggered(bool)),
	        this, SLOT(showTheoreticalArrivals(bool)));
	connect(SC_D.ui.actionShowUnassociatedPicks, SIGNAL(triggered(bool)),
	        this, SLOT(showUnassociatedPicks(bool)));
	connect(SC_D.ui.actionShowSpectrogram, SIGNAL(triggered(bool)),
	        this, SLOT(showSpectrogram(bool)));

	connect(SC_D.ui.actionOpenSpectrum, SIGNAL(triggered(bool)),
	        this, SLOT(showSpectrum()));

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

	QCheckBox *cb = new QCheckBox;
	cb->setObjectName("spec.log");
	cb->setText(tr("Logscale"));
	cb->setChecked(false);
	connect(cb, SIGNAL(toggled(bool)), this, SLOT(specLogToggled(bool)));
	specLogToggled(cb->isChecked());

	SC_D.ui.toolBarSpectrogram->addWidget(cb);

	cb = new QCheckBox;
	cb->setObjectName("spec.smooth");
	cb->setText(tr("Smoothing"));
	cb->setChecked(true);
	connect(cb, SIGNAL(toggled(bool)), this, SLOT(specSmoothToggled(bool)));
	specSmoothToggled(cb->isChecked());

	SC_D.ui.toolBarSpectrogram->addWidget(cb);

	SC_D.specOpts.minRange = -15;
	SC_D.specOpts.maxRange = -5;
	SC_D.specOpts.tw = 5;

	QDoubleSpinBox *spinLower = new QDoubleSpinBox;
	spinLower->setMinimum(-100);
	spinLower->setMaximum(100);
	spinLower->setValue(SC_D.specOpts.minRange);
	connect(spinLower, SIGNAL(valueChanged(double)), this, SLOT(specMinValue(double)));
	specMinValue(spinLower->value());

	SC_D.ui.toolBarSpectrogram->addSeparator();
	SC_D.ui.toolBarSpectrogram->addWidget(spinLower);

	QDoubleSpinBox *spinUpper = new QDoubleSpinBox;
	spinUpper->setMinimum(-100);
	spinUpper->setMaximum(100);
	spinUpper->setValue(SC_D.specOpts.maxRange);
	connect(spinUpper, SIGNAL(valueChanged(double)), this, SLOT(specMaxValue(double)));
	specMaxValue(spinUpper->value());

	SC_D.ui.toolBarSpectrogram->addSeparator();
	SC_D.ui.toolBarSpectrogram->addWidget(spinUpper);

	QDoubleSpinBox *spinTW = new QDoubleSpinBox;
	spinTW->setMinimum(0.1);
	spinTW->setMaximum(600);
	spinTW->setValue(SC_D.specOpts.tw);
	spinTW->setSuffix("s");
	spinTW->setToolTip(tr("Sets the time window length of raw data to be used to compute a column of the spectrogram."));
	connect(spinTW, SIGNAL(valueChanged(double)), this, SLOT(specTimeWindow(double)));
	specTimeWindow(spinTW->value());
	specApply();

	SC_D.ui.toolBarSpectrogram->addSeparator();
	SC_D.ui.toolBarSpectrogram->addWidget(spinTW);

	QToolButton *btnSpecUpdate = new QToolButton;
	btnSpecUpdate->setToolTip(tr("Applies the time window changes to the current spectrogram (if active)."));
	btnSpecUpdate->setText(tr("Apply"));
	connect(btnSpecUpdate, SIGNAL(clicked()), this, SLOT(specApply()));
	SC_D.ui.toolBarSpectrogram->addWidget(btnSpecUpdate);

	// connect actions
	connect(SC_D.ui.actionDefaultView, SIGNAL(triggered(bool)),
	        this, SLOT(setDefaultDisplay()));
	connect(SC_D.ui.actionSortAlphabetically, SIGNAL(triggered(bool)),
	        this, SLOT(sortAlphabetically()));
	connect(SC_D.ui.actionSortByDistance, SIGNAL(triggered(bool)),
	        this, SLOT(sortByDistance()));
	connect(SC_D.ui.actionSortByAzimuth, SIGNAL(triggered(bool)),
	        this, SLOT(sortByAzimuth()));
	connect(SC_D.ui.actionSortByResidual, SIGNAL(triggered(bool)),
	        this, SLOT(sortByResidual()));

	connect(SC_D.ui.actionShowAllComponents, SIGNAL(triggered(bool)),
	        this, SLOT(showAllComponents(bool)));
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
	connect(SC_D.ui.actionAlignOnSArrival, SIGNAL(triggered(bool)),
	        this, SLOT(alignOnSArrivals()));

	connect(SC_D.ui.actionIncreaseAmplitudeScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleAmplUp()));
	connect(SC_D.ui.actionDecreaseAmplitudeScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleAmplDown()));
	connect(SC_D.ui.actionTimeScaleUp, SIGNAL(triggered(bool)),
	        this, SLOT(scaleTimeUp()));
	connect(SC_D.ui.actionTimeScaleDown, SIGNAL(triggered(bool)),
	        this, SLOT(scaleTimeDown()));
	connect(SC_D.ui.actionResetScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleReset()));
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
	connect(SC_D.ui.actionRepickAutomatically, SIGNAL(triggered(bool)),
	        this, SLOT(automaticRepick()));
	connect(SC_D.ui.actionGotoNextMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoNextMarker()));
	connect(SC_D.ui.actionGotoPreviousMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoPreviousMarker()));
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

	connect(SC_D.ui.actionToggleFilter, SIGNAL(triggered(bool)),
	        this, SLOT(toggleFilter()));
	connect(SC_D.ui.actionNextFilter, SIGNAL(triggered(bool)),
	        this, SLOT(nextFilter()));
	connect(SC_D.ui.actionPreviousFilter, SIGNAL(triggered(bool)),
	        this, SLOT(previousFilter()));

	connect(SC_D.ui.actionMaximizeAmplitudes, SIGNAL(triggered(bool)),
	        this, SLOT(scaleVisibleAmplitudes()));

	connect(SC_D.ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(pickNone(bool)));
	connect(SC_D.ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(abortSearchStation()));
	connect(SC_D.ui.actionPickP, SIGNAL(triggered(bool)),
	        this, SLOT(pickP(bool)));
	connect(SC_D.ui.actionPickS, SIGNAL(triggered(bool)),
	        this, SLOT(pickS(bool)));

	connect(SC_D.ui.actionCreatePick, SIGNAL(triggered(bool)),
	        this, SLOT(createPick()));
	connect(SC_D.ui.actionSetPick, SIGNAL(triggered(bool)),
	        this, SLOT(setPick()));
	connect(SC_D.ui.actionConfirmPick, SIGNAL(triggered(bool)),
	        this, SLOT(confirmPick()));
	connect(SC_D.ui.actionDeletePick, SIGNAL(triggered(bool)),
	        this, SLOT(deletePick()));

	connect(SC_D.ui.actionRelocate, SIGNAL(triggered(bool)),
	        this, SLOT(relocate()));

	connect(SC_D.ui.actionModifyOrigin, SIGNAL(triggered(bool)),
	        this, SLOT(modifyOrigin()));

	connect(SC_D.ui.actionShowAllStations, SIGNAL(triggered(bool)),
	        this, SLOT(loadNextStations()));

	connect(SC_D.ui.actionShowUsedStations, SIGNAL(triggered(bool)),
	        this, SLOT(showUsedStations(bool)));

	/* TODO: Remove me
	connect(SC_D.ui.btnScaleReset, SIGNAL(clicked()),
	        this, SLOT(scaleReset()));
	*/

	connect(SC_D.ui.btnRowAccept, SIGNAL(clicked()),
	        this, SLOT(confirmPick()));
	connect(SC_D.ui.btnRowRemove, SIGNAL(clicked(bool)),
	        this, SLOT(setCurrentRowDisabled(bool)));
	connect(SC_D.ui.btnRowRemove, SIGNAL(clicked(bool)),
	        SC_D.recordView, SLOT(selectNextRow()));
	connect(SC_D.ui.btnRowReset, SIGNAL(clicked(bool)),
	        this, SLOT(resetPick()));
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
	/*
	connect(SC_D.recordView, SIGNAL(cursorTextChanged(const QString&)),
	        SC_D.currentRecord, SLOT(setCursorText(const QString&)));
	*/

	SC_D.ui.frameZoom->setBackgroundRole(QPalette::Base);
	SC_D.ui.frameZoom->setAutoFillBackground(true);

	SC_D.actionsUncertainty = nullptr;
	SC_D.actionsPickGroupPhases = nullptr;
	SC_D.actionsPickFavourites = nullptr;
	SC_D.actionsAlignOnFavourites = nullptr;
	SC_D.actionsAlignOnGroupPhases = nullptr;

	SC_D.minTime = -SC_D.config.minimumTimeWindow;
	SC_D.maxTime = SC_D.config.minimumTimeWindow;

	/*
	pal = palette();
	pal.setColor(QPalette::Highlight, QColor(224,255,224));
	//pal.setColor(QPalette::Light, QColor(240,255,240));
	SC_D.recordView->setPalette(pal);
	*/

	/*
	//SC_D.wantedPhases.push_back("Pg");
	//SC_D.wantedPhases.push_back("Pn");
	SC_D.wantedPhases.push_back("P");
	//SC_D.wantedPhases.push_back("pP");
	SC_D.wantedPhases.push_back("S");
	//SC_D.wantedPhases.push_back("sS");
	*/

	/*
	SC_D.broadBandCodes.push_back("BH");
	SC_D.broadBandCodes.push_back("SH");
	SC_D.broadBandCodes.push_back("HH");

	SC_D.strongMotionCodes.push_back("BL");
	SC_D.strongMotionCodes.push_back("SL");
	SC_D.strongMotionCodes.push_back("HL");
	SC_D.strongMotionCodes.push_back("HN");
	SC_D.strongMotionCodes.push_back("LN");
	*/

	connect(SC_D.recordView, SIGNAL(selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)));

	connect(SC_D.currentRecord, SIGNAL(selectedTime(Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Core::Time)));

	connect(SC_D.recordView, SIGNAL(addedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)),
	        this, SLOT(onAddedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)));

	connect(&RecordStreamState::Instance(), SIGNAL(firstConnectionEstablished()),
	        this, SLOT(firstConnectionEstablished()));
	connect(&RecordStreamState::Instance(), SIGNAL(lastConnectionClosed()),
	        this, SLOT(lastConnectionClosed()));

	if ( RecordStreamState::Instance().connectionCount() )
		firstConnectionEstablished();
	else
		lastConnectionClosed();

	Processing::PickerFactory::ServiceNames *pickers = Processing::PickerFactory::Services();
	if ( pickers ) {
		std::sort(pickers->begin(), pickers->end());
		SC_D.comboPicker = new QComboBox(this);
		Processing::PickerFactory::ServiceNames::iterator it;
		for ( it = pickers->begin(); it != pickers->end(); ++it )
			SC_D.comboPicker->addItem(it->c_str());

		SC_D.ui.toolBarPicking->addSeparator();
		SC_D.ui.toolBarPicking->addWidget(SC_D.comboPicker);
		delete pickers;
	}
	else
		SC_D.comboPicker = nullptr;

	SC_D.ui.menuPicking->addAction(SC_D.ui.actionGotoNextMarker);
	SC_D.ui.menuPicking->addAction(SC_D.ui.actionGotoPreviousMarker);
	SC_D.ui.menuPicking->addSeparator();
	SC_D.ui.menuPicking->addAction(SC_D.ui.actionDisablePicking);
	SC_D.ui.menuPicking->addAction(SC_D.ui.actionPickP);
	SC_D.ui.menuPicking->addAction(SC_D.ui.actionPickS);

	/*
	QDockWidget *dock = new QDockWidget(tr("Filter picks"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	SC_D.pickInfoList = new QWidget(dock);
	QVBoxLayout *l = new QVBoxLayout;
	SC_D.pickInfoList->setLayout(l);
	dock->setWidget(SC_D.pickInfoList);
	addDockWidget(Qt::RightDockWidgetArea, dock);

	QMenu *viewWindows = SC_D.ui.menuView->addMenu(tr("Windows"));
	viewWindows->addAction(dock->toggleViewAction());
	*/

	initPhases();

	PickerMarkerActionPluginFactory::ServiceNames *plugins = PickerMarkerActionPluginFactory::Services();
	if ( plugins != nullptr ) {
		PickerMarkerActionPluginFactory::ServiceNames::iterator it;
		for ( it = plugins->begin(); it != plugins->end(); ++it ) {
			PickerMarkerActionPlugin *plugin = PickerMarkerActionPluginFactory::Create(it->c_str());
			if ( plugin != nullptr ) {
				plugin->setParent(this);
				SC_D.markerPlugins.append(plugin);
			}
		}

		delete plugins;
	}

	try {
		SC_D.componentFollowsMouse = SCApp->configGetBool("picker.componentFollowsMouse");
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

void createPhaseMenus(QActionGroup *actionGroup, QList<QMenu*> &menus,
                      const PickerView::Config::GroupList &list,
                      QMenu *root = nullptr, int depth = 0)
{
	QMenu *actionRoot = depth == 0?nullptr:root;

	foreach ( const PickerView::Config::PhaseGroup &group, list ) {
		if ( group.childs.empty() ) {
			if ( actionRoot == nullptr ) {
				if ( root == nullptr ) {
					actionRoot = new QMenu(group.name);
					menus.append(actionRoot);
				}
				else
					actionRoot = root->addMenu("unnamed");

				// Store top-level menus
				if ( depth == 0 ) menus.append(actionRoot);
			}

			QAction *action = new QAction(group.name, actionGroup);
			actionRoot->addAction(action);
		}
		else {
			QMenu *subMenu;

			if ( root == nullptr )
				subMenu = new QMenu(group.name);
			else
				subMenu = root->addMenu(group.name);

			// Store top-level menus
			if ( depth == 0 ) menus.append(subMenu);

			createPhaseMenus(actionGroup, menus, group.childs, subMenu, depth+1);
		}
	}
}


void createAlignPhaseMenus(QActionGroup *actionGroup, QList<QMenu*> &menus,
                           const PickerView::Config::GroupList &list,
                           QMenu *root = nullptr, int depth = 0)
{
	QMenu *actionRoot = depth == 0?nullptr:root;

	foreach ( const PickerView::Config::PhaseGroup &group, list ) {
		if ( group.childs.empty() ) {
			if ( actionRoot == nullptr ) {
				if ( root == nullptr ) {
					actionRoot = new QMenu(group.name);
					menus.append(actionRoot);
				}
				else
					actionRoot = root->addMenu("unnamed");

				// Store top-level menus
				if ( depth == 0 ) menus.append(actionRoot);
			}

			QAction *action = new QAction(group.name, actionGroup);
			// Flag as phase
			action->setData(QVariant(false));
			actionRoot->addAction(action);

			action = new QAction(QString("%1 (ttt)").arg(group.name), actionGroup);
			// Flag as theoretical phase
			action->setData(QVariant(true));
			actionRoot->addAction(action);
		}
		else {
			QMenu *subMenu;

			if ( root == nullptr )
				subMenu = new QMenu(group.name);
			else
				subMenu = root->addMenu(group.name);

			// Store top-level menus
			if ( depth == 0 ) menus.append(subMenu);

			createAlignPhaseMenus(actionGroup, menus, group.childs, subMenu, depth+1);
		}
	}
}


}


void PickerView::initPhases() {
	SC_D.phases.clear();
	SC_D.showPhases.clear();

	// Remove pick phase group submenus
	foreach ( QMenu *menu, SC_D.menusPickGroups )
		delete menu;
	SC_D.menusPickGroups.clear();

	// Remove align phase group submenus
	foreach ( QMenu *menu, SC_D.menusAlignGroups )
		delete menu;
	SC_D.menusAlignGroups.clear();

	// Remove pick group phases
	if ( SC_D.actionsPickGroupPhases ) {
		delete SC_D.actionsPickGroupPhases;
		SC_D.actionsPickGroupPhases = nullptr;
	}

	// Remove align group phases
	if ( SC_D.actionsAlignOnGroupPhases ) {
		delete SC_D.actionsAlignOnGroupPhases;
		SC_D.actionsAlignOnGroupPhases = nullptr;
	}

	// Remove pick favourite phases
	if ( SC_D.actionsPickFavourites ) {
		delete SC_D.actionsPickFavourites;
		SC_D.actionsPickFavourites = nullptr;
	}

	// Remove align favourite phases
	if ( SC_D.actionsAlignOnFavourites ) {
		delete SC_D.actionsAlignOnFavourites;
		SC_D.actionsAlignOnFavourites = nullptr;
	}

	// Create favourite phase actions + shortcuts
	if ( !SC_D.config.favouritePhases.empty() ) {
		SC_D.ui.menuPicking->addSeparator();
		SC_D.ui.menuAlignArrival->addSeparator();

		SC_D.actionsPickFavourites = new QActionGroup(this);
		SC_D.actionsAlignOnFavourites = new QActionGroup(this);

		if ( SC_D.config.favouritePhases.count() > 9 )
			SEISCOMP_WARNING("More than 9 favourite phases defined: shortcuts are only "
			                 "assigned to the first 9 phases");

		int i = 0;
		foreach ( const QString &ph, SC_D.config.favouritePhases ) {
			QAction *pickAction = new QAction(SC_D.actionsPickFavourites);
			pickAction->setText(ph);

			QAction *alignAction = new QAction(SC_D.actionsAlignOnFavourites);
			alignAction->setText(ph);

			QAction *alignTheoreticalAction = new QAction(SC_D.actionsAlignOnFavourites);
			alignTheoreticalAction->setText(QString("%1 (ttt)").arg(ph));

			// Set flag to use pick time (theoretical == false)
			alignAction->setData(QVariant(false));
			// Set flag to use theoretical pick time (theoretical == true)
			alignTheoreticalAction->setData(QVariant(true));

			if ( i < 9 ) {
				pickAction->setShortcut(Qt::Key_1 + i);
				alignAction->setShortcut(Qt::CTRL + Qt::Key_1 + i);
				alignTheoreticalAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_1 + i);
			}

			SC_D.ui.menuPicking->addAction(pickAction);
			SC_D.ui.menuAlignArrival->addAction(alignAction);
			SC_D.ui.menuAlignArrival->addAction(alignTheoreticalAction);

			++i;
		}

		connect(SC_D.actionsPickFavourites, SIGNAL(triggered(QAction*)),
		        this, SLOT(setPickPhase(QAction*)));

		connect(SC_D.actionsAlignOnFavourites, SIGNAL(triggered(QAction*)),
		        this, SLOT(alignOnPhase(QAction*)));
	}

	if ( !SC_D.config.phaseGroups.empty() ) {
		SC_D.ui.menuPicking->addSeparator();

		SC_D.actionsPickGroupPhases = new QActionGroup(this);

		createPhaseMenus(SC_D.actionsPickGroupPhases, SC_D.menusPickGroups,
		                 SC_D.config.phaseGroups, SC_D.ui.menuPicking);

		connect(SC_D.actionsPickGroupPhases, SIGNAL(triggered(QAction*)),
		        this, SLOT(setPickPhase(QAction*)));


		SC_D.ui.menuAlignArrival->addSeparator();
		SC_D.actionsAlignOnGroupPhases = new QActionGroup(this);

		createAlignPhaseMenus(SC_D.actionsAlignOnGroupPhases, SC_D.menusAlignGroups,
		                      SC_D.config.phaseGroups, SC_D.ui.menuAlignArrival);

		connect(SC_D.actionsAlignOnGroupPhases, SIGNAL(triggered(QAction*)),
		        this, SLOT(alignOnPhase(QAction*)));
	}


	QSet<QString> phases;
	phases.insert("P");
	phases.insert("S");

	if ( SC_D.actionsPickGroupPhases ) {
		foreach ( QAction *action, SC_D.actionsPickGroupPhases->actions() )
			phases.insert(action->text());
	}

	SC_D.phases = phases.values();

	foreach ( const QString &ph, SC_D.config.showPhases ) {
		if ( !phases.contains(ph) ) {
			SC_D.showPhases.append(ph);
			phases.insert(ph);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::setConfig(const Config &c, QString *error) {
	SC_D.config = c;

	SC_D.uncertainties.clear();

	Config::UncertaintyProfiles::iterator it;
	it = SC_D.config.uncertaintyProfiles.find(SC_D.config.uncertaintyProfile);
	if ( it != SC_D.config.uncertaintyProfiles.end() )
		SC_D.uncertainties = it.value();

	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setUncertainties(SC_D.uncertainties);
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setCrossHairEnabled(SC_D.config.showCrossHair);

	if ( SCScheme.unit.distanceInKM )
		SC_D.spinDistance->setValue(Math::Geo::deg2km(SC_D.config.defaultAddStationsDistance));
	else
		SC_D.spinDistance->setValue(SC_D.config.defaultAddStationsDistance);

	if ( SC_D.comboFilter ) {
		SC_D.comboFilter->blockSignals(true);
		SC_D.comboFilter->clear();
		SC_D.comboFilter->addItem(NO_FILTER_STRING);

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

		SC_D.comboFilter->setCurrentIndex(defaultIndex != -1?defaultIndex:(SC_D.comboFilter->count() > 1?1:0));
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

		SC_D.ui.actionShowAllComponents->setEnabled(true);
		SC_D.ui.actionShowAllComponents->setChecked(SC_D.currentRecord->drawMode() == RecordWidget::InRows);
	}
	else {
		SC_D.ui.actionShowAllComponents->setChecked(false);
		SC_D.ui.actionShowAllComponents->setEnabled(false);
	}

	/*
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordViewItem *item = SC_D.recordView->itemAt(i);
		if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
			if ( SC_D.config.showAllComponents &&
				SC_D.config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
				item->widget()->setDrawMode(RecordWidget::InRows);
			else
				item->widget()->setDrawMode(RecordWidget::Single);
		}
		else
			item->widget()->setDrawMode(RecordWidget::Single);
	}
	*/


	if ( SC_D.actionsUncertainty ) {
		delete SC_D.actionsUncertainty;
		SC_D.actionsUncertainty = nullptr;
	}

	SC_D.actionsUncertainty = new QActionGroup(this);

	QAction *action = new QAction(SC_D.actionsUncertainty);
	action->setText("unset");
	action->setShortcut(Qt::SHIFT + Qt::Key_0);
	action->setData(-1);
	addAction(action);

	connect(action, SIGNAL(triggered()), this, SLOT(setPickUncertainty()));

	if ( !SC_D.uncertainties.isEmpty() ) {
		if ( SC_D.uncertainties.count() > 8 ) {
			SEISCOMP_WARNING("more than 8 uncertainty profiles defined, shortcuts are "
			                 "assigned to the first 8 profiles only.");
		}

		for ( int i = 0; i < SC_D.uncertainties.count(); ++i ) {
			action = new QAction(SC_D.actionsUncertainty);
			QString text;

			Config::Uncertainty value = SC_D.uncertainties[i];
			if ( value.first == value.second )
				text = QString("+/-%1s").arg(value.first);
			else
				text = QString("-%1s;+%2s").arg(value.first).arg(value.second);

			action->setText(text);
			action->setData(i);

			if ( i < 9 )
				action->setShortcut(Qt::SHIFT + (Qt::Key_1 + i));

			connect(action, SIGNAL(triggered()), this, SLOT(setPickUncertainty()));

			addAction(action);
		}
	}

	bool reselectCurrentItem = false;

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
		if ( isLinkedItem(item) ) continue;

		// Force state to false if item has no data yet and should be hidden
		item->forceInvisibilty(!isTracePicked(item->widget())
		                    && ((SC_D.config.hideStationsWithoutData && !label->hasGotData)
		                     || (SC_D.config.hideDisabledStations && !label->isEnabledByConfig)));

		if ( item == SC_D.recordView->currentItem() )
			reselectCurrentItem = true;
	}

	if ( SC_D.recordView->currentItem() == nullptr ) reselectCurrentItem = true;

	if ( reselectCurrentItem )
		selectFirstVisibleItem(SC_D.recordView);

	SC_D.ui.actionShowUnassociatedPicks->setChecked(SC_D.config.loadAllPicks);

	initPhases();
	acquireStreams();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setDatabase(Seiscomp::DataModel::DatabaseQuery* reader) {
	SC_D.reader = reader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setBroadBandCodes(const std::vector<std::string> &codes) {
	SC_D.broadBandCodes = codes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setStrongMotionCodes(const std::vector<std::string> &codes) {
	SC_D.strongMotionCodes = codes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setAuxiliaryChannels(const std::vector<std::string> &patterns,
                                      double minimumDistance, double maximumDistance) {
	SC_D.auxiliaryStreamIDPatterns = patterns;
	SC_D.auxiliaryMinDistance = minimumDistance;
	SC_D.auxiliaryMaxDistance = maximumDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showEvent(QShowEvent *e) {
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
			QVariant geometry = SCApp->settings().value("geometry");
			restoreState(SCApp->settings().value("state").toByteArray());
			restoreGeometry(geometry.toByteArray());
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
void PickerView::onSelectedTime(Seiscomp::Core::Time time) {
	//updatePhaseMarker(SC_D.currentRecord, time);
	if ( SC_D.recordView->currentItem() ) {
		updatePhaseMarker(SC_D.recordView->currentItem()->widget(), time);
		SC_D.currentRecord->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::onSelectedTime(Seiscomp::Gui::RecordWidget* widget,
                                Seiscomp::Core::Time time) {
	if ( widget == SC_D.currentRecord ) return;
	updatePhaseMarker(widget, time);
	//updatePhaseMarker(SC_D.currentRecord, time);
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updatePhaseMarker(Seiscomp::Gui::RecordWidget* widget,
                                   const Seiscomp::Core::Time& time) {
	ZoomRecordWidget *zoomRecord = static_cast<ZoomRecordWidget*>(SC_D.currentRecord);
	int uncertaintyIndex = zoomRecord->currentUncertaintyIndex();

	PickerMarker *marker = (PickerMarker*)widget->marker(widget->cursorText(), true);
	// Marker found?
	if ( marker ) {
		marker->setType(PickerMarker::Arrival);

		// Set the marker time to the new picked time
		marker->setCorrectedTime(time);
		// and set its component to the currently displayed component
		marker->setSlot(SC_D.currentRecord->currentRecords());
		marker->setRotation(SC_D.currentRotationMode);
		marker->setFilter(SC_D.currentFilterID);

		if ( uncertaintyIndex >= 0 ) {
			marker->setUncertainty(
				SC_D.uncertainties[uncertaintyIndex].first,
				SC_D.uncertainties[uncertaintyIndex].second
			);
		}

		marker->setEnabled(true);
		widget->setCurrentMarker(marker);

		/*
		// If there exists another marker with a different phase on the same time, delete it
		for ( int i = 0; i < widget->markerCount(); ++i ) {
			if ( widget->marker(i)->text() != marker->text()
			     && ((PickerMarker*)widget->marker(i))->isArrival()
			     && widget->marker(i)->correctedTime() == marker->correctedTime() ) {
				// NOTE: Better check whether the underlaying marker is an arrival or a manual pick
				//       to decide whether to delete or to rename the pick
				if ( widget->removeMarker(i) )
					--i;
			}
		}
		*/

		// Disable all other arrivals of the same phase
		for ( int i = 0; i < widget->markerCount(); ++i ) {
			PickerMarker* m = (PickerMarker*)widget->marker(i);
			if ( marker == m ) continue;
			if ( m->text() != widget->cursorText() ) continue;
			if ( m->isArrival() ) m->setType(PickerMarker::Pick);
		}

		widget->update();
	}
	else {
		// Valid phase code?
		if ( !widget->cursorText().isEmpty() ) {
			PickerMarker *reusedMarker = nullptr;

			// Look for a marker that is on the same position
			for ( int i = 0; i < widget->markerCount(); ++i ) {
				if ( /*widget->marker(i)->text() != widget->cursorText()
				     &&*/
				     widget->marker(i)->correctedTime() == time ) {
					marker = static_cast<PickerMarker*>(widget->marker(i));

#if 0
					if ( !marker->isMovable() /*&& !marker->isPick()*/ ) continue;
#endif
					if ( !marker->isMovable() && !marker->isArrival() && !marker->isPick() ) continue;
					if ( !marker->isPick() && !marker->isArrival() ) continue;

					reusedMarker = marker;
					break;
				}
			}

			if ( reusedMarker == nullptr ) {
				// Create a new marker for the phase
				marker = new PickerMarker(widget, time, widget->cursorText(),
				                          PickerMarker::Arrival, true);

				if ( uncertaintyIndex >= 0 ) {
					marker->setUncertainty(
						SC_D.uncertainties[uncertaintyIndex].first,
						SC_D.uncertainties[uncertaintyIndex].second
					);
				}

				marker->setSlot(SC_D.currentRecord->currentRecords());
				marker->setRotation(SC_D.currentRotationMode);
				marker->setFilter(SC_D.currentFilterID);

				for ( int i = 0; i < widget->markerCount(); ++i ) {
					PickerMarker *marker2 = (PickerMarker*)widget->marker(i);
					if ( marker == marker2 ) continue;

					if ( marker2->text() == marker->text() && marker2->isArrival() ) {
						// Set type back to pick. The phase code is updated
						// automatically
						marker2->setType(PickerMarker::Pick);
						// Copy orientation
						marker->setBackazimuth(marker2->backazimuth());
						marker->setHorizontalSlowness(marker2->horizontalSlowness());
					}
				}

				widget->setCurrentMarker(marker);
			}
			else {
				declareArrival(reusedMarker, widget->cursorText(), false);
				widget->setCurrentMarker(reusedMarker);
			}

			widget->update();
		}
	}

	if ( SC_D.recordView->currentItem()->widget() == widget &&
	     widget->cursorText() == "P" && marker ) {
		RecordMarker* marker2 = widget->marker("P" THEORETICAL_POSTFIX);
		if ( marker2 )
			SC_D.recordView->currentItem()->setValue(ITEM_RESIDUAL_INDEX,
				-fabs((double)(marker->correctedTime() - marker2->correctedTime())));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::declareArrival(RecordMarker *m_, const QString &phase,
                                bool updateResidual) {
	PickerMarker *m = (PickerMarker*)m_;
	if ( m->isPick() ) m->setType(PickerMarker::Arrival);
	m->setPhaseCode(phase);
	m->setEnabled(true);

	RecordWidget *w = m->parent();
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker *marker = (PickerMarker*)w->marker(i);
		if ( marker == m ) continue;
		if ( marker->text() != phase ) continue;
		if ( marker->isArrival() ) {
			if ( marker->isMovable() )
				delete marker;
			else
				marker->setType(PickerMarker::Pick);
			break;
		}
	}

	if ( !updateResidual ) return;

	if ( SC_D.recordView->currentItem()->widget() == w &&
	     w->cursorText() == "P" && m ) {
		RecordMarker* marker2 = w->marker("P" THEORETICAL_POSTFIX);
		if ( marker2 )
			SC_D.recordView->currentItem()->setValue(ITEM_RESIDUAL_INDEX,
				-fabs((double)(m->correctedTime() - marker2->correctedTime())));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::onAddedItem(const Record* rec, RecordViewItem* item) {
	// NOTE: Dynamic item insertion is not yet used
	/*
	setupItem(item);
	addTheoreticalArrivals(item, rec->networkCode(), rec->stationCode(), rec->locationCode());
	sortByDistance();
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::apply(QAction* action) {
	int id = action->data().toInt() - 1;

	if ( id < 0 ) {
		switch ( id ) {
			case -2:
				alignOnOriginTime();
				break;
			case -3:
				sortByDistance();
				break;
			case -4:
				pickNone(true);
				break;
			case -5:
				confirmPick();
				break;
			case -6:
				deletePick();
				break;
			case -7:
				relocate();
				break;
		}
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickPhase(QAction *action) {
	setCursorText(action->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnPhase(QAction *action) {
	alignOnPhase(action->text().left(action->text().indexOf(' ')), action->data().toBool());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCursorText(const QString &text) {
	SC_D.recordView->setCursorText(text);
	SC_D.currentRecord->setCursorText(text);
	SC_D.currentRecord->setActive(text != "");
	auto d = static_cast<PickerTimeWindowDecorator*>(SC_D.currentRecord->decorator());
	if ( d ) {
		d->setVisible(false);
		SC_D.currentRecord->update();
	}

	if ( SC_D.currentRecord->isActive() ) {
#ifdef CENTER_SELECTION
		SC_D.centerSelection = true;
#endif
		RecordMarker* m = SC_D.currentRecord->marker(text, true);
		if ( !m ) m = SC_D.currentRecord->marker(text);
		if ( m )
			setCursorPos(m->correctedTime());
#ifdef CENTER_SELECTION
		else if ( SC_D.recordView->currentItem() )
			setCursorPos(SC_D.recordView->currentItem()->widget()->visibleTimeWindow().startTime() +
			             Core::TimeSpan(SC_D.recordView->currentItem()->widget()->visibleTimeWindow().length()*0.5));
#endif
	}

	updateCurrentRowState();
	componentByState();

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnPhase(const QString& phase, bool theoretical) {
	int used = 0;

	SC_D.alignedOnOT = false;
	QString phaseId = phase;
	//QString shortPhaseId = QString("%1").arg(getShortPhaseName(phase.toStdString()));

	if ( theoretical ) {
		phaseId += THEORETICAL_POSTFIX;
		//shortPhaseId += THEORETICAL_POSTFIX;
	}

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem *item = SC_D.recordView->itemAt(r);
		PickerRecordLabel *l = static_cast<PickerRecordLabel*>(item->label());

		// Is the item an linked (controlled) item, ignore it
		// The alignment is done by the controller item
		if ( l->isLinkedItem() ) continue;

		RecordViewItem *controlledItem = l->controlledItem();

		RecordWidget* w1 = item->widget();
		RecordWidget* w2 = controlledItem?controlledItem->widget():nullptr;

		// Find modified arrivals for phase of controller item
		RecordMarker* m = w1->marker(phaseId, true);
		// Find arrivals for phase
		if ( !m ) m = w1->marker(phaseId);
		//if ( !m ) m = w1->marker(shortPhaseId, true);

		// No pick found on controller item?
		if ( w2 && !m ) {
			m = w2->marker(phaseId, true);
			if ( !m ) m = w2->marker(phaseId);
		}

		if ( !theoretical ) {
			// Find theoretical arrivals for phase
			if ( !m ) m = w1->marker(phase + THEORETICAL_POSTFIX);

			// Find automatic picks for phase
			if ( !m ) m = w1->marker(phase + AUTOMATIC_POSTFIX);

			if ( w2 && !m ) {
				// Find automatic picks for phase
				m = w2->marker(phase + AUTOMATIC_POSTFIX);
			}
		}

		if ( m ) {
			w1->setAlignment(m->correctedTime());
			if ( w2 ) w2->setAlignment(m->correctedTime());

			++used;
		}
	}

	if ( !used ) return;

	SC_D.checkVisibility = false;

	SC_D.recordView->setAbsoluteTimeEnabled(false);

	SC_D.recordView->setJustification(SC_D.config.alignmentPosition);

	double timeRange = SC_D.recordView->timeRangeMax() - SC_D.recordView->timeRangeMin();
	double leftTime = -timeRange*SC_D.config.alignmentPosition;
	double rightTime = timeRange*(1.0-SC_D.config.alignmentPosition);
	SC_D.recordView->setTimeRange(leftTime, rightTime);

	SC_D.checkVisibility = true;

	if ( SC_D.recordView->currentItem() ) {
		RecordWidget* w = SC_D.recordView->currentItem()->widget();
		setAlignment(w->alignment());
//#ifdef CENTER_SELECTION
		SC_D.centerSelection = true;
//#endif
		setCursorPos(w->alignment(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::loadNextStations() {
	float distance = SC_D.spinDistance->value();

	if ( SCScheme.unit.distanceInKM )
		distance = Math::Geo::km2deg(distance);

	std::vector<Seiscomp::DataModel::WaveformStreamID>::iterator it;

	SC_D.recordView->setUpdatesEnabled(false);

	/*
	for ( it = SC_D.nextStations.begin(); it != SC_D.nextStations.end(); ++it ) {
		RecordViewItem* item = SC_D.recordView->item(waveformIDToString(*it));
		if ( item ) {
			item->setVisible(item->value(0) <= distance);
			//SC_D.recordView->removeItem(item);
			//SC_D.stations.remove((it->networkCode() + "." + it->stationCode()).c_str());
		}
	}
	*/

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		bool show = false;

		if ( !isLinkedItem(item) ) {
			if ( isArrivalTrace(item->widget()) )
			//if ( item->widget()->hasMovableMarkers() )
				show = true;
			else
				show = item->value(ITEM_DISTANCE_INDEX) <= distance;

			if ( SC_D.ui.actionShowUsedStations->isChecked() )
				show = show && isTraceUsed(item->widget());

			item->setVisible(show);
		}
	}

	loadNextStations(distance);
	fillRawPicks();

	sortByState();
	alignByState();
	componentByState();

	if ( SC_D.recordView->currentItem() == nullptr ) {
		selectFirstVisibleItem(SC_D.recordView);
	}
	setCursorText(SC_D.currentRecord->cursorText());

	SC_D.recordView->setUpdatesEnabled(true);
	SC_D.recordView->setFocus();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByState() {
	if ( SC_D.ui.actionSortByDistance->isChecked() )
		sortByDistance();
	else if ( SC_D.ui.actionSortByAzimuth->isChecked() )
		sortByAzimuth();
	else if ( SC_D.ui.actionSortAlphabetically->isChecked() )
		sortAlphabetically();
	else if ( SC_D.ui.actionSortByResidual->isChecked() )
		sortByResidual();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignByState() {
	if ( SC_D.alignedOnOT && SC_D.origin )
		SC_D.recordView->setAlignment(SC_D.origin->time());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::componentByState() {
	if ( SC_D.ui.actionShowZComponent->isChecked() )
		showComponent('Z');
	else if ( SC_D.ui.actionShowNComponent->isChecked() )
		showComponent('1');
	else if ( SC_D.ui.actionShowEComponent->isChecked() )
		showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::resetState() {
	if ( SC_D.comboRotation->currentIndex() > RT_123 )
		changeRotation(SC_D.comboRotation->currentIndex());

	showComponent('Z');
	alignOnOriginTime();
	pickNone(true);
	sortByDistance();
	SC_D.ui.actionShowUsedStations->setChecked(false);
	showUsedStations(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateLayoutFromState() {
	sortByState();
	alignByState();
	componentByState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::firstConnectionEstablished() {
	SC_D.connectionState->start(SC_D.config.recordURL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::lastConnectionClosed() {
	SC_D.connectionState->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::beginWaitForRecords() {
	qApp->setOverrideCursor(Qt::WaitCursor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::doWaitForRecords(int value) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::endWaitForRecords() {
	qApp->restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showFullscreen(bool e) {
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
void PickerView::fetchComponent(char componentCode) {
	for ( auto it = SC_D.allStreams.begin(); it != SC_D.allStreams.end(); ) {
		char queuedComponent = it->component;
		if ( queuedComponent == componentCode || queuedComponent == '?' ||
		     SC_D.config.loadAllComponents ) {
			if ( SC_D.config.usePerStreamTimeWindows ) {
				// Cut the needed timewindow
				RecordViewItem* item = SC_D.recordView->item(it->streamID);
				if ( item ) {
					RecordWidget *w = item->widget();
					Core::TimeWindow tw;
					for ( int i = 0; i < w->markerCount(); ++i ) {
						PickerMarker *m = static_cast<PickerMarker*>(w->marker(i));
						if ( (m->type() == PickerMarker::Arrival || m->type() == PickerMarker::Theoretical) &&
						     getShortPhaseName(m->text().toStdString()) == 'P' &&
						     m->text().left(3) != "PcP" ) {
							Core::Time start = m->time() - Core::TimeSpan(SC_D.config.preOffset);
							Core::Time end = m->time() + Core::TimeSpan(SC_D.config.postOffset);

							if ( !tw.startTime().valid() || tw.startTime() > start )
								tw.setStartTime(start);

							if ( !tw.endTime().valid() || tw.endTime() < end )
								tw.setEndTime(end);
						}
					}

					it->timeWindow = tw;
				}
			}

			SC_D.nextStreams.push_back(*it);
			it = SC_D.allStreams.erase(it);
		}
		else
			++it;
	}

	// Sort by distance
	SC_D.nextStreams.sort();

	acquireStreams();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showComponent(char componentCode) {
	fetchComponent(componentCode);

	switch ( componentCode ) {
		default:
		case 'Z':
			SC_D.currentSlot = 0;
			break;
		case '1':
			SC_D.currentSlot = 1;
			break;
		case '2':
			SC_D.currentSlot = 2;
			break;
	}

	SC_D.recordView->showSlot(SC_D.currentSlot);
	SC_D.ui.actionShowZComponent->setChecked(componentCode == 'Z');
	SC_D.ui.actionShowNComponent->setChecked(componentCode == '1');
	SC_D.ui.actionShowEComponent->setChecked(componentCode == '2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showUsedStations(bool usedOnly) {
	//float distance = SC_D.spinDistance->value();

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
void PickerView::loadNextStations(float distance) {
	DataModel::Inventory* inv = Client::Inventory::Instance()->inventory();

	if ( inv != nullptr ) {

		for ( size_t i = 0; i < inv->networkCount(); ++i ) {
			Network *n = inv->network(i);
			for ( size_t j = 0; j < n->stationCount(); ++j ) {
				Station *s = n->station(j);

				QString code = (n->code() + "." + s->code()).c_str();

				if ( SC_D.stations.contains(code) ) continue;

				try {
					if ( s->end() <= SC_D.origin->time() )
						continue;
				}
				catch ( Core::ValueException & ) {}

				double lat, lon;
				double delta, az1, az2;

				try {
					lat = s->latitude(); lon = s->longitude();
				}
				catch ( Core::ValueException & ) {
					SEISCOMP_WARNING("Station %s.%s has no valid coordinates",
					                 n->code().c_str(), s->code().c_str());
					continue;
				}

				Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
				                  lat, lon, &delta, &az1, &az2);

				if ( delta > distance ) continue;

				// try to get the configured location and stream code
				Stream *stream = findConfiguredStream(s, SC_D.origin->time());
				if ( stream ) {
					SEISCOMP_DEBUG("Adding configured stream %s.%s.%s.%s",
					               stream->sensorLocation()->station()->network()->code().c_str(),
					               stream->sensorLocation()->station()->code().c_str(),
					               stream->sensorLocation()->code().c_str(),
					               stream->code().c_str());
				}

				// Try to get a default stream
				if ( !stream ) {
					// Preferred channel code is BH. If not available use either SH or skip.
					for ( size_t c = 0; c < SC_D.broadBandCodes.size(); ++c ) {
						stream = findStream(s, SC_D.broadBandCodes[c], SC_D.origin->time());
						if ( stream ) break;
					}
				}

				if ( !stream && !SC_D.config.ignoreUnconfiguredStations ) {
					stream = findStream(s, SC_D.origin->time(), Processing::WaveformProcessor::MeterPerSecond);
					if ( stream != nullptr ) {
						SEISCOMP_DEBUG("Adding velocity stream %s.%s.%s.%s",
						               stream->sensorLocation()->station()->network()->code().c_str(),
						               stream->sensorLocation()->station()->code().c_str(),
						               stream->sensorLocation()->code().c_str(),
						               stream->code().c_str());
					}
				}

				if ( stream ) {
					try {
						stream->sensorLocation()->latitude();
						stream->sensorLocation()->longitude();
					}
					catch ( ... ) {
						SEISCOMP_WARNING("SensorLocation %s.%s.%s has no valid coordinates",
						                 stream->sensorLocation()->station()->network()->code().c_str(),
						                 stream->sensorLocation()->station()->code().c_str(),
						                 stream->sensorLocation()->code().c_str());
						continue;
					}

					WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");
					auto sid = waveformIDToStdString(streamID);
					bool isAuxilliary = false;
					for ( const auto &pattern : SC_D.auxiliaryStreamIDPatterns ) {
						if ( Core::wildcmp(pattern, sid) ) {
							isAuxilliary = true;
							break;
						}
					}

					if ( isAuxilliary ) {
						// Auxilliary station will only be added (unless associated)
						// if they are within a configured distance range.
						if ( delta < SC_D.auxiliaryMinDistance ) {
							SEISCOMP_DEBUG("Auxilliary channel %s rejected, too close (%f < %f)",
							               sid.c_str(), delta, SC_D.auxiliaryMinDistance);
							continue;
						}

						if ( delta > SC_D.auxiliaryMaxDistance ) {
							SEISCOMP_DEBUG("Auxilliary channel %s rejected, too far away (%f > %f)",
							               sid.c_str(), delta, SC_D.auxiliaryMaxDistance);
							continue;
						}
					}

					RecordViewItem* item = addStream(stream->sensorLocation(), streamID, delta, streamID.stationCode().c_str(), false, true, stream);
					if ( item ) {
						SC_D.stations.insert(code);
						item->setVisible(!SC_D.ui.actionShowUsedStations->isChecked());
						if ( SC_D.config.hideStationsWithoutData )
							item->forceInvisibilty(true);
					}
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addArrival(Seiscomp::Gui::RecordWidget* widget,
                            Seiscomp::DataModel::Arrival* arrival, int id) {
	Pick* pick = Pick::Find(arrival->pickID());
	if ( !pick ) return;

	if ( !arrival->phase().code().empty() ) {
		//NOTE: Because autoloc's associates e.g. PP phases automatically
		//      it is important to insert all arrivals here otherwise the
		//      trace wont be shown (unused)
		//if ( !SC_D.phases.contains(arrival->phase().code().c_str()) )
		//	return;

		PickerMarker *marker = new PickerMarker(widget,
		                                        pick->time(),
		                                        arrival->phase().code().c_str(),
		                                        PickerMarker::Arrival, false);

		marker->setPick(pick);
		marker->setId(id);

		if ( !pick->methodID().empty() ) {
			marker->setDescription(QString("%1<%2>")
			                       .arg(arrival->phase().code().c_str())
			                       .arg((char)toupper(pick->methodID()[0])));
		}

		try {
			if ( arrival->weight() <= 0.5 ) {
				marker->setEnabled(false);
				//marker->setMovable(true);
			}
		}
		catch ( ... ) {}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::figureOutTravelTimeTable() {
	if ( !SC_D.origin ) return;

	int idx = SC_D.comboTTT->findText(SC_D.origin->methodID().c_str());
	if ( idx < 0 ) return;

	SC_D.ttTableName = SC_D.origin->earthModelID();
	SC_D.comboTTT->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::setOrigin(Seiscomp::DataModel::Origin* origin,
                           double relTimeWindowStart,
                           double relTimeWindowEnd) {
	if ( origin == SC_D.origin ) return false;

	SEISCOMP_DEBUG("stopping record acquisition");
	stop();

	SC_D.recordView->clear();
	SC_D.recordItemLabels.clear();

	SC_D.origin = origin;
	figureOutTravelTimeTable();

	updateOriginInformation();
	if ( SC_D.comboFilter->currentIndex() == 0 && SC_D.lastFilterIndex > 0 )
		SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);

	if ( SC_D.origin == nullptr )
		return false;

	setUpdatesEnabled(false);

	SC_D.stations.clear();

	Core::Time originTime = SC_D.origin->time();
	if ( !originTime ) originTime = Core::Time::GMT();

	Core::Time minTime = originTime;
	Core::Time maxTime = originTime;

	// Find the minimal and maximal pick time
	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		Arrival* arrival = origin->arrival(i);
		Pick* pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( !pick )
			continue;

		if ( (Core::Time)pick->time() < minTime )
			minTime = pick->time();

		if ( (Core::Time)pick->time() > maxTime )
			maxTime = pick->time();
	}

	if ( minTime > maxTime )
		std::swap(minTime, maxTime);

	// Add a buffer of one minute to the time span
	minTime -= SC_D.config.preOffset;
	maxTime += SC_D.config.postOffset;

	if ( (maxTime - minTime) < Core::TimeSpan(0,100000) ) {
		minTime -= Core::TimeSpan(10 * SC_D.config.alignmentPosition,0);
		maxTime += Core::TimeSpan(10 * (1-SC_D.config.alignmentPosition),0);
	}

	relTimeWindowStart = minTime - originTime;
	relTimeWindowEnd = maxTime - originTime;

	double timeWindowLength = relTimeWindowEnd - relTimeWindowStart;

	SC_D.minTime = relTimeWindowStart;
	SC_D.maxTime = relTimeWindowEnd;

	SEISCOMP_DEBUG("setting time range to: [%.2f,%.2f]", SC_D.minTime, SC_D.maxTime);

	timeWindowLength = std::max(timeWindowLength, (double)SC_D.config.minimumTimeWindow);
	SC_D.timeWindow = Core::TimeWindow((Core::Time)originTime + Core::TimeSpan(relTimeWindowStart),
	                               timeWindowLength);

	//SC_D.recordView->setBufferSize(timeWindowLength + 5*60); /*safety first! */
	SC_D.recordView->setTimeWindow(SC_D.timeWindow);
	SC_D.recordView->setTimeRange(SC_D.minTime, SC_D.maxTime);

	if ( origin->arrivalCount() > 0 ) {
		for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
			Arrival* arrival = origin->arrival(i);

			PickPtr pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
			if ( !pick ) {
				//std::cout << "pick not found" << std::endl;
				continue;
			}

			WaveformStreamID streamID = adjustWaveformStreamID(pick->waveformID());
			SensorLocation *loc = nullptr;

			Station *sta = Client::Inventory::Instance()->getStation(
				streamID.networkCode(), streamID.stationCode(), SC_D.origin->time());

			if ( sta )
				loc = findSensorLocation(sta, streamID.locationCode(), origin->time());

			Stream *cha = Client::Inventory::Instance()->getStream(pick.get());

			double dist;
			try { dist = arrival->distance(); }
			catch ( ... ) { dist = 0; }

			RecordViewItem* item = addStream(loc, pick->waveformID(), dist, pick->waveformID().stationCode().c_str(), true, false, cha);

			// A new item has been inserted
			if ( item != nullptr ) {
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

			addArrival(item->widget(), arrival, i);
		}
	}
	else
		loadNextStations();

	SC_D.timeWindowOfInterest.setStartTime(originTime + Core::TimeSpan(relTimeWindowStart));
	SC_D.timeWindowOfInterest.setEndTime(originTime + Core::TimeSpan(relTimeWindowStart + timeWindowLength));
	SC_D.loadedPicks = false;
	SC_D.picksInTime.clear();

	if ( CFG_LOAD_PICKS ) loadPicks();

	if ( SC_D.loadedPicks )
		SC_D.ui.actionShowUnassociatedPicks->setChecked(true);

	fillRawPicks();
	fillTheoreticalArrivals();
	SC_D.recordView->setAlignment(originTime);

	resetState();
	updateLayoutFromState();

	selectFirstVisibleItem(SC_D.recordView);

	setUpdatesEnabled(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PickerView::loadPicks() {
	if ( !SC_D.timeWindowOfInterest ) return -1;

	SEISCOMP_DEBUG("Loading picks in time window: %s ~ %s",
	               SC_D.timeWindowOfInterest.startTime().iso().c_str(),
	               SC_D.timeWindowOfInterest.endTime().iso().c_str());

	std::vector<Seiscomp::DataModel::PickPtr> savePicks = SC_D.picksInTime;

	if ( !SCApp->commandline().hasOption("offline") ) {
		if ( SC_D.reader ) {
			qApp->setOverrideCursor(Qt::WaitCursor);

			DatabaseIterator it = SC_D.reader->getPicks(SC_D.timeWindowOfInterest.startTime(),
			                                        SC_D.timeWindowOfInterest.endTime());
			for ( ; *it; ++it ) {
				Pick* pick = Pick::Cast(*it);
				if ( pick )
					SC_D.picksInTime.push_back(pick);
			}

			//std::cout << "read " << SC_D.picksInTime.size() << " picks in time" << std::endl;
			SC_D.loadedPicks = true;

			qApp->restoreOverrideCursor();
		}
	}
	else {
		qApp->setOverrideCursor(Qt::WaitCursor);

		EventParameters *ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
		if ( ep ) {
			for ( size_t i = 0; i < ep->pickCount(); ++i ) {
				Pick* pick = ep->pick(i);
				if ( pick && SC_D.timeWindowOfInterest.contains(pick->time().value()) )
					SC_D.picksInTime.push_back(pick);
			}
		}

		SC_D.loadedPicks = true;
		//std::cout << "read " << SC_D.picksInTime.size() << " picks in time" << std::endl;

		qApp->restoreOverrideCursor();
	}

	return (int)SC_D.picksInTime.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::setOrigin(Seiscomp::DataModel::Origin* o) {
	SC_D.origin = o;
	figureOutTravelTimeTable();

	// Remove picks and arrivals from all traces and update the ones
	// from the new origin
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* item = SC_D.recordView->itemAt(r);
		item->label()->setEnabled(true);
		item->widget()->clearMarker();
	}

	Core::Time originTime = SC_D.origin->time();
	if ( !originTime ) originTime = Core::Time::GMT();

	Core::Time minTime = originTime;
	Core::Time maxTime = originTime;

	// Add a buffer of one minute to the time span
	minTime -= SC_D.config.preOffset;
	maxTime += SC_D.config.postOffset;

	double relTimeWindowStart = minTime - originTime;
	double relTimeWindowEnd = maxTime - originTime;

	double timeWindowLength = relTimeWindowEnd - relTimeWindowStart;

	SC_D.minTime = relTimeWindowStart;
	SC_D.maxTime = relTimeWindowEnd;

	SEISCOMP_DEBUG("update time range to: [%.2f,%.2f]", SC_D.minTime, SC_D.maxTime);

	timeWindowLength = std::max(timeWindowLength, (double)SC_D.config.minimumTimeWindow);
	SC_D.timeWindow = Core::TimeWindow((Core::Time)originTime + Core::TimeSpan(relTimeWindowStart),
	                               timeWindowLength);

	SC_D.timeScale->setSelectionEnabled(false);

	if ( SC_D.origin ) {
		for ( size_t i = 0; i < SC_D.origin->arrivalCount(); ++i ) {
			Pick* pick = Pick::Find(SC_D.origin->arrival(i)->pickID());
			if ( pick ) {
				RecordViewItem* item = SC_D.recordView->item(adjustWaveformStreamID(pick->waveformID()));
				if ( !item ) {
					SensorLocation *loc = nullptr;

					Station *sta = Client::Inventory::Instance()->getStation(
						pick->waveformID().networkCode(),
						pick->waveformID().stationCode(),
						SC_D.origin->time());

					if ( sta )
						loc = findSensorLocation(sta, pick->waveformID().locationCode(), SC_D.origin->time());

					Stream *cha = Client::Inventory::Instance()->getStream(pick);

					double dist;
					try { dist = SC_D.origin->arrival(i)->distance(); }
					catch ( ... ) { dist = 0; }

					item = addStream(loc, pick->waveformID(), dist, pick->waveformID().stationCode().c_str(), true, false, cha);
					if ( item ) {
						SC_D.stations.insert((pick->waveformID().networkCode() + "." +
						                  pick->waveformID().stationCode()).c_str());
					}
				}

				if ( item )
					addArrival(item->widget(), SC_D.origin->arrival(i), i);
			}
		}
	}

	fillRawPicks();

	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem *item = SC_D.recordView->itemAt(r);
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());

		if ( SC_D.origin ) {
			double delta, az, baz;
			Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
			                  label->latitude, label->longitude, &delta, &az, &baz);

			label->orientationZRT.loadRotateZ(deg2rad(baz + 180.0));
		}
		else
			label->orientationZRT.identity();
	}


	if ( SC_D.comboRotation->currentIndex() == RT_ZRT )
		changeRotation(RT_ZRT);
	else if ( SC_D.comboRotation->currentIndex() == RT_ZNE )
		changeRotation(RT_ZNE);
	else if ( SC_D.comboRotation->currentIndex() == RT_ZH )
		changeRotation(RT_ZH);

	componentByState();
	updateOriginInformation();
	updateTheoreticalArrivals();
	alignByState();
	sortByState();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateOriginInformation() {
	QString title;

	if ( SC_D.origin ) {
		QString depth;
		try {
			depth = QString("%1").arg((int)SC_D.origin->depth());
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
const TravelTime* PickerView::findPhase(const TravelTimeList &ttt, const QString &phase, double delta) {
	TravelTimeList::const_iterator it;

	if (phase=="P" || phase=="P1")
		return firstArrivalP(&ttt);

	// First pass -> exact match
	for ( it = ttt.begin(); it != ttt.end(); ++it ) {
		if (delta>115) { // skip Pdiff et al.
			if (it->phase ==  "Pdiff") continue;
			if (it->phase == "pPdiff") continue;
			if (it->phase == "sPdiff") continue;
		}

		QString ph(it->phase.c_str());

		if ( phase == ph )
			return &(*it);

		if (phase=="P" && (it->phase == "Pn" || it->phase == "Pg" || it->phase == "Pb"))
			return &(*it);
	}

	if ( phase != "P" && phase != "S" )
		return nullptr;

	// Second pass -> find first phase that represents a
	// P or S phase
	for ( it = ttt.begin(); it != ttt.end(); ++it ) {
		if (delta>115) { // skip Pdiff et al.
			if (it->phase ==  "Pdiff") continue;
			if (it->phase == "pPdiff") continue;
			if (it->phase == "sPdiff") continue;
		}

		if ( phase[0] == getShortPhaseName(it->phase) )
			return &(*it);
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::addTheoreticalArrivals(RecordViewItem* item,
                                        const std::string& netCode,
                                        const std::string& staCode,
                                        const std::string& locCode) {
	if ( !SC_D.origin ) return false;

	// First clear all theoretical arrivals
	for ( int i = 0; i < item->widget()->markerCount(); ) {
		PickerMarker* m = static_cast<PickerMarker*>(item->widget()->marker(i));
		if ( m->type() == PickerMarker::Theoretical )
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

				PickerMarker* marker = new PickerMarker(
					item->widget(),
					(Core::Time)SC_D.origin->time() + Core::TimeSpan(tt->time),
					phase + THEORETICAL_POSTFIX,
					PickerMarker::Theoretical,
					false
				);

				marker->setVisible(SC_D.ui.actionShowTheoreticalArrivals->isChecked());

				// Set the description of the marker that is used as display text
				marker->setDescription(tt->phase.c_str());

				// Remember the phase
				currentPhases[tt->phase.c_str()] = marker;
			}

			foreach ( const QString &phase, SC_D.showPhases ) {
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

				PickerMarker* marker = new PickerMarker(
					item->widget(),
					(Core::Time)SC_D.origin->time() + Core::TimeSpan(tt->time),
					phase + THEORETICAL_POSTFIX,
					PickerMarker::Theoretical,
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

		for ( int i = 0; i < item->widget()->markerCount(); ++i ) {
			PickerMarker* m = static_cast<PickerMarker*>(item->widget()->marker(i));
			if ( m->text() == "P" && m->isArrival() ) {
				RecordMarker* m2 = item->widget()->marker("P" THEORETICAL_POSTFIX);
				if ( m2 ) {
					item->setValue(ITEM_RESIDUAL_INDEX, -fabs((double)(m->correctedTime() - m2->correctedTime())));
					break;
				}
			}
		}

		return true;
	}
	catch ( std::exception& excp ) {
		SEISCOMP_ERROR("%s", excp.what());
		//item->label()->setText("unknown", 0, Qt::darkRed);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::fillTheoreticalArrivals() {
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
bool PickerView::fillRawPicks() {
	bool pickAdded = false;
	for ( size_t i = 0; i < SC_D.picksInTime.size(); ++i ) {
		bool result = addRawPick(SC_D.picksInTime[i].get());
		pickAdded = result || pickAdded;
	}

	return pickAdded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addPick(Seiscomp::DataModel::Pick* pick) {
	if ( !SC_D.origin ) return;

	if ( (Core::Time)pick->time() > ((Core::Time)SC_D.origin->time() + SC_D.config.minimumTimeWindow) )
		return;

	SC_D.picksInTime.push_back(pick);
	addRawPick(pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setStationEnabled(const std::string& networkCode,
                                   const std::string& stationCode,
                                   bool state) {
	QList<RecordViewItem*> streams = SC_D.recordView->stationStreams(networkCode, stationCode);
	foreach ( RecordViewItem* item, streams ) {
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
		label->setConfigState(state);

		// Force state to false if item has no data yet and should be hidden
		item->forceInvisibilty(!isTracePicked(item->widget())
		                    && ((SC_D.config.hideStationsWithoutData && !label->hasGotData)
		                     || (SC_D.config.hideDisabledStations && !label->isEnabledByConfig)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::addRawPick(Seiscomp::DataModel::Pick *pick) {
	RecordViewItem* item = SC_D.recordView->item(adjustWaveformStreamID(pick->waveformID()));
	if ( !item )
		return false;

	RecordWidget* widget = item->widget();

	// Do we have a marker for this pick already?
	for ( int i = 0; i < widget->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(widget->marker(i));
		if ( m->pick() && m->pick()->publicID() == pick->publicID() )
			return false;
	}

	PickerMarker* marker = new PickerMarker(nullptr, pick->time(), PickerMarker::Pick, false);
	widget->insertMarker(0, marker);

	try {
		marker->setText(QString("%1" AUTOMATIC_POSTFIX).arg(pick->phaseHint().code().c_str()));

		if ( !pick->methodID().empty() ) {
			marker->setDescription(QString("%1<%2>")
			                       .arg(pick->phaseHint().code().c_str())
			                       .arg((char)toupper(pick->methodID()[0])));
		}
	}
	catch ( ... ) {}

	marker->setPick(pick);
	marker->update();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::queueStream(double dist, const DataModel::WaveformStreamID& streamID,
                             char component) {
	SC_D.allStreams.push_back(
		PickerViewPrivate::WaveformRequest(
			dist, Core::TimeWindow(), streamID, component
		)
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* PickerView::addStream(const DataModel::SensorLocation *sloc,
                                      const WaveformStreamID& streamID,
                                      double distance,
                                      const std::string& text,
                                      bool /*showDisabled*/,
                                      bool theoreticalArrivals, const Stream *base) {
	/*
	bool isEnabled = true;
	if ( !showDisabled ) {
		isEnabled = SCApp->isStationEnabled(streamID.networkCode(), streamID.stationCode());
		if ( !isEnabled )
			return nullptr;
	}
	*/
	bool isEnabled = SCApp->isStationEnabled(streamID.networkCode(), streamID.stationCode());

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

	RecordViewItem *item = addRawStream(sloc, streamID, distance, text, theoreticalArrivals, base);
	if ( item == nullptr ) return nullptr;

	item->setValue(ITEM_PRIORITY_INDEX, 0);

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
	label->setConfigState(isEnabled);
	label->hasGotData = false;

	item->forceInvisibilty(!label->isEnabledByConfig && SC_D.config.hideDisabledStations);

	if ( hasStrongMotion ) {
		// Try to find a corresponding StrongMotion stream and add
		// it to the view
		RecordViewItem *smItem = addRawStream(smsloc, smStreamID, distance, text, theoreticalArrivals);
		if ( smItem ) {
			label = static_cast<PickerRecordLabel*>(smItem->label());
			label->setLinkedItem(true);
			label->setConfigState(isEnabled);
			label->hasGotData = false;
			smItem->setValue(ITEM_PRIORITY_INDEX, 1);
			smItem->forceInvisibilty(!label->isEnabledByConfig && SC_D.config.hideDisabledStations);
			smItem->setVisible(false);

			// Start showing the expandable button when the first record arrives
			connect(smItem, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
			        static_cast<PickerRecordLabel*>(item->label()), SLOT(enableExpandable(const Seiscomp::Record*)));

			static_cast<PickerRecordLabel*>(item->label())->setControlledItem(smItem);
			//static_cast<PickerRecordLabel*>(item->label())->enabledExpandButton(smSC_D.item);

			smItem->label()->setBackgroundColor(QColor(192,192,255));
		}
	}

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickPolarity() {
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());

	if ( m == nullptr ) return;

	if ( !m->isPick() && !m->isArrival() ) return;
	if ( m->pick() && !m->isEnabled() ) return;

	// Create a new marker if the existing marker uses a pick already
	// to create a new pick
	if ( m->pick() ) {
		PickerMarker *old = m;
		m = new PickerMarker(old->parent(), *old);
		m->convertToManualPick();
		old->setType(PickerMarker::Pick);
		old->parent()->setCurrentMarker(m);
	}

	if ( sender() == SC_D.ui.actionSetPolarityPositive ) {
		m->setPolarity(PickPolarity(POSITIVE));
	}
	else if ( sender() == SC_D.ui.actionSetPolarityNegative ) {
		m->setPolarity(PickPolarity(NEGATIVE));
	}
	else if ( sender() == SC_D.ui.actionSetPolarityUndecidable ) {
		m->setPolarity(PickPolarity(UNDECIDABLE));
	}
	else if ( sender() == SC_D.ui.actionSetPolarityUnset ) {
		m->setPolarity(Core::None);
	}

	SC_D.currentRecord->update();
	SC_D.recordView->currentItem()->widget()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickOnset() {
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());

	if ( m == nullptr ) return;

	if ( !m->isPick() && !m->isArrival() ) return;
	if ( m->pick() && !m->isEnabled() ) return;

	// Create a new marker if the existing marker uses a pick already
	// to create a new pick
	if ( m->pick() ) {
		PickerMarker *old = m;
		m = new PickerMarker(old->parent(), *old);
		m->convertToManualPick();
		old->setType(PickerMarker::Pick);
		old->parent()->setCurrentMarker(m);
	}

	if ( sender() == SC_D.ui.actionSetPickOnsetEmergent ) {
		m->setPickOnset(PickOnset(EMERGENT));
	}
	else if ( sender() == SC_D.ui.actionSetPickOnsetImpulsive ) {
		m->setPickOnset(PickOnset(IMPULSIVE));
	}
	else if ( sender() == SC_D.ui.actionSetPickOnsetQuestionable ) {
		m->setPickOnset(PickOnset(QUESTIONABLE));
	}
	else if ( sender() == SC_D.ui.actionSetPickOnsetUnset ) {
		m->setPickOnset(Core::None);
	}

	SC_D.currentRecord->update();
	SC_D.recordView->currentItem()->widget()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickUncertainty() {
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
	if ( !SC_D.actionsUncertainty ) return;

	/*
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		foreach ( QAction *action, SC_D.actionsUncertainty->actions() ) {
			if ( sender() != action ) continue;
			bool ok;
			int idx = action->data().toInt(&ok);
			if ( !ok ) {
				cerr << "triggered uncertainty action with unexpected data: "
				     << action->data().toString().toStdString() << endl;
				return;
			}

			static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setCurrentUncertaintyIndex(idx);
		}
	}
	*/

	if ( m == nullptr ) return;

	if ( !m->isPick() && !m->isArrival() ) return;
	if ( m->pick() && !m->isEnabled() ) return;

	foreach ( QAction *action, SC_D.actionsUncertainty->actions() ) {
		if ( sender() != action ) continue;

		bool ok;
		int idx = action->data().toInt(&ok);
		if ( !ok ) {
			std::cerr << "triggered uncertainty action with unexpected data: "
			          << action->data().toString().toStdString() << std::endl;
			return;
		}

		if ( idx < -1 || idx >= SC_D.uncertainties.count() ) {
			std::cerr << "triggered uncertainty action out of range: "
			          << idx << " not in [0," << SC_D.uncertainties.count()-1
			          << "]" << std::endl;
			return;
		}

		// Create a new marker if the existing marker uses a pick already
		// to create a new pick
		if ( m->pick() ) {
			PickerMarker *old = m;
			m = new PickerMarker(old->parent(), *old);
			m->convertToManualPick();
			old->setType(PickerMarker::Pick);
			old->parent()->setCurrentMarker(m);
		}

		if ( idx == -1 )
			m->setUncertainty(-1,-1);
		else
			m->setUncertainty(SC_D.uncertainties[idx].first, SC_D.uncertainties[idx].second);

		updateUncertaintyHandles(m);

		// Store values since they are copied again after the context menu
		// is closed
		SC_D.tmpLowerUncertainty = m->lowerUncertainty();
		SC_D.tmpUpperUncertainty = m->upperUncertainty();

		SC_D.currentRecord->update();
		SC_D.recordView->currentItem()->widget()->update();
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::openContextMenu(const QPoint &p) {
	RecordLabel *label = static_cast<RecordLabel*>(sender());
	//std::cout << "Context menu request from " << waveformIDToStdString(item->streamID()) << std::endl;

	Client::Inventory* inv = Client::Inventory::Instance();
	if ( !inv ) {
		return;
	}

	QMenu menu(this);
	int entries = 0;

	QMenu *streams = menu.addMenu("Add stream");

	WaveformStreamID tmp(label->recordViewItem()->streamID());

	Station *station = inv->getStation(tmp.networkCode(), tmp.stationCode(), SC_D.origin->time());
	if ( !station ) {
		return;
	}

	std::set<std::string> codes;

	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= SC_D.origin->time() ) {
				continue;
			}
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > SC_D.origin->time() ) {
			continue;
		}

		try {
			loc->latitude(); loc->longitude();
		}
		catch ( ... ) {
			SEISCOMP_WARNING("SensorLocation %s.%s.%s has no valid coordinates",
			                 loc->station()->network()->code().c_str(),
			                 loc->station()->code().c_str(), loc->code().c_str());
			continue;
		}

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream* stream = loc->stream(j);

			std::string streamCode = stream->code().substr(0, stream->code().size()-1);
			std::string id = loc->code() + "." + streamCode;

			try {
				if ( stream->end() <= SC_D.origin->time() ) {
					continue;
				}
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > SC_D.origin->time() ) {
				continue;
			}

			if ( codes.find(id) != codes.end() ) {
				continue;
			}

			codes.insert(id);

			tmp.setLocationCode(loc->code());
			tmp.setChannelCode(streamCode + '?');

			if ( SC_D.recordView->item(tmp) ) {
				continue;
			}

			QAction *action = new QAction(id.c_str(), streams);
			action->setData(waveformIDToQString(tmp));
			streams->addAction(action);

			++entries;
		}
	}

	if ( !entries ) {
		return;
	}

	//menu.addAction(cutAct);
	//menu.addAction(copyAct);
	//menu.addAction(pasteAct);
	QAction *res = menu.exec(label->mapToGlobal(p));
	if ( !res ) {
		return;
	}

	QString data = res->data().toString();
	QStringList l = data.split('.');
	tmp.setNetworkCode(l[0].toStdString());
	tmp.setStationCode(l[1].toStdString());
	tmp.setLocationCode(l[2].toStdString());
	tmp.setChannelCode(l[3].toStdString());

	SensorLocation *loc = findSensorLocation(station, tmp.locationCode(), SC_D.origin->time());

	double delta, az, baz;
	if ( SC_D.origin ) {
		Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
		                  loc->latitude(), loc->longitude(), &delta, &az, &baz);
	}
	else {
		delta = 0;
	}

	addStream(loc, tmp, delta, tmp.stationCode().c_str(), false, true);

	fillRawPicks();

	sortByState();
	alignByState();
	componentByState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::openRecordContextMenu(const QPoint &p) {
	SC_D.currentRecord->setCurrentMarker(SC_D.currentRecord->hoveredMarker());
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());

	QMenu menu;

	QAction *defineUncertainties = nullptr;
	QAction *dropDirectivity = nullptr;
	QAction *deleteArrival = nullptr;
	QAction *deleteArrivalWithRemove = nullptr;
	QAction *removePick = nullptr;
	QAction *createArrival = nullptr;
	QAction *modifyOrigin = nullptr;
	QAction *artificialOrigin = nullptr;

	QList<QAction*> plugins;

	double lowerUncertainty = -1;
	double upperUncertainty = -1;

	bool markerMode = m && (m->isPick() || m->isArrival());

	if ( !markerMode && !SC_D.currentRecord->cursorText().isEmpty() ) {
		return;
	}

	if ( markerMode ) {
		// Save uncertainties to reset them again if changed
		// during preview
		SC_D.tmpLowerUncertainty = m->lowerUncertainty();
		SC_D.tmpUpperUncertainty = m->upperUncertainty();
	
		lowerUncertainty = SC_D.tmpLowerUncertainty;
		upperUncertainty = SC_D.tmpUpperUncertainty;

		if ( !m->pick() || m->isEnabled() ) {
			QMenu *menuPolarity = menu.addMenu(tr("Polarity"));
			QMenu *menuOnset = menu.addMenu(tr("Onset"));
			QMenu *menuUncertainty = menu.addMenu(tr("Uncertainty"));

			if ( SC_D.actionsUncertainty ) {
				connect(menuUncertainty, SIGNAL(hovered(QAction*)),
				        this, SLOT(previewUncertainty(QAction*)));

				foreach ( QAction *action, SC_D.actionsUncertainty->actions() )
					menuUncertainty->addAction(action);
				menuUncertainty->addSeparator();
			}

			defineUncertainties = menuUncertainty->addAction(tr("Define..."));

			menuPolarity->addAction(SC_D.ui.actionSetPolarityPositive);
			menuPolarity->addAction(SC_D.ui.actionSetPolarityNegative);
			menuPolarity->addAction(SC_D.ui.actionSetPolarityUndecidable);
			menuPolarity->addAction(SC_D.ui.actionSetPolarityUnset);

			menuOnset->addAction(SC_D.ui.actionSetPickOnsetEmergent);
			menuOnset->addAction(SC_D.ui.actionSetPickOnsetImpulsive);
			menuOnset->addAction(SC_D.ui.actionSetPickOnsetQuestionable);
			menuOnset->addAction(SC_D.ui.actionSetPickOnsetUnset);
		}

		bool needSeparator = !menu.isEmpty();

		if ( !SC_D.currentRecord->cursorText().isEmpty() &&
		     (m->isPick() || (m->isArrival() && m->text() != SC_D.currentRecord->cursorText())) ) {
			if ( needSeparator ) { menu.addSeparator(); needSeparator = false; }
			createArrival = menu.addAction(tr("Declare %1 arrival").arg(SC_D.currentRecord->cursorText()));
		}

		if ( m->backazimuth() && !m->pick() ) {
			dropDirectivity = menu.addAction(tr("Drop directivity information"));
		}

		if ( m->isArrival() ) {
			if ( needSeparator ) { menu.addSeparator(); needSeparator = false; }
			deleteArrival = menu.addAction(tr("Delete arrival"));
			if ( SC_D.loadedPicks )
				deleteArrivalWithRemove = menu.addAction(tr("Delete arrival and remove pick"));
		}
		else if ( m->isPick() ) {
			if ( needSeparator ) { menu.addSeparator(); needSeparator = false; }
			removePick = menu.addAction(tr("Remove pick"));
		}

		if ( !SC_D.markerPlugins.empty() ) {
			menu.addSeparator();
			foreach ( PickerMarkerActionPlugin *plugin, SC_D.markerPlugins ) {
				QAction *action = menu.addAction(plugin->title());
				action->setData(QVariant::fromValue((void*)plugin));
				plugins.append(action);
			}
		}

		menu.addSeparator();
	}

	modifyOrigin = menu.addAction(tr("Modify origin"));
	artificialOrigin = menu.addAction(tr("Create artificial origin"));

	QAction *res = menu.exec(SC_D.currentRecord->mapToGlobal(p));

	if ( !res ) {
		if ( markerMode ) {
			m->setUncertainty(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
			m->update();
			SC_D.currentRecord->update();
		}
		return;
	}

	if ( res == dropDirectivity ) {
		m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
		m->setBackazimuth(Core::None);
		m->setHorizontalSlowness(Core::None);
		m->update();
		return;
	}
	else if ( (res == deleteArrival) || (res == deleteArrivalWithRemove) ) {
		if ( SC_D.currentRecord->currentMarker() ) {
			m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
			if ( m->isArrival() ) {
				if ( m->isMovable() || !SC_D.loadedPicks ) {
					delete m;
					m = nullptr;
				}
				else
					m->setType(PickerMarker::Pick);
			}

			if ( m && res == deleteArrivalWithRemove ) delete m;

			SC_D.currentRecord->update();
			if ( SC_D.recordView->currentItem() ) SC_D.recordView->currentItem()->widget()->update();
		}

		return;
	}
	else if ( res == removePick ) {
		if ( SC_D.currentRecord->currentMarker() ) {
			m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
			/*
			// Remove the pick from the pick list to avoid loading of
			// the marker again if fillRawPicks is called
			if ( m->pick() ) {
				std::vector<DataModel::PickPtr>::iterator it;
				it = std::find(SC_D.picksInTime.begin(), SC_D.picksInTime.end(), m->pick());
				if ( it != SC_D.picksInTime.end() ) SC_D.picksInTime.erase(it);
			}
			*/
			delete m;
			SC_D.currentRecord->update();
		}
		return;
	}
	else if ( res == createArrival ) {
		if ( SC_D.currentRecord->currentMarker() ) {
			m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
			declareArrival(m, SC_D.currentRecord->cursorText(), true);
		}

		return;
	}
	else if ( res == modifyOrigin ) {
		if ( markerMode ) {
			m->setUncertainty(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
			m->update();
			SC_D.currentRecord->update();
		}

		double dep = SC_D.config.defaultDepth;
		try { dep = SC_D.origin->depth(); } catch ( ... ) {}
		OriginDialog dialog(
			static_cast<PickerRecordLabel*>(SC_D.recordView->currentItem()->label())->longitude,
			static_cast<PickerRecordLabel*>(SC_D.recordView->currentItem()->label())->latitude,
			dep,
			this
		);
		dialog.setTime(SC_D.currentRecord->unmapTime(p.x()));
		dialog.setWindowTitle("Modify origin");
		dialog.setSendButtonText("Apply");
		if ( dialog.exec() == QDialog::Accepted ) {
			OriginPtr tmpOrigin = Origin::Create();
			CreationInfo ci;
			ci.setAgencyID(SCApp->agencyID());
			ci.setAuthor(SCApp->author());
			ci.setCreationTime(Core::Time::GMT());
			//tmpOrigin->assign(SC_D.origin.get());
			tmpOrigin->setLatitude(dialog.latitude());
			tmpOrigin->setLongitude(dialog.longitude());
			tmpOrigin->setTime(Core::Time(dialog.getTime_t()));
			tmpOrigin->setDepth(RealQuantity(dialog.depth()));
			tmpOrigin->setDepthType(OriginDepthType(OPERATOR_ASSIGNED));
			tmpOrigin->setEvaluationMode(EvaluationMode(MANUAL));
			tmpOrigin->setCreationInfo(ci);
			for ( size_t i = 0; i < SC_D.origin->arrivalCount(); ++i ) {
				ArrivalPtr ar = new Arrival(*SC_D.origin->arrival(i));
				tmpOrigin->add(ar.get());
			}
			setOrigin(tmpOrigin.get());
			updateLayoutFromState();
		}

		return;
	}
	else if ( res == artificialOrigin ) {
		if ( markerMode ) {
			m->setUncertainty(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
			m->update();
			SC_D.currentRecord->update();
		}

		double dep = SC_D.config.defaultDepth;
		try { dep = SC_D.origin->depth(); } catch ( ... ) {}

		emit requestArtificialOrigin(
			static_cast<PickerRecordLabel*>(SC_D.recordView->currentItem()->label())->latitude,
			static_cast<PickerRecordLabel*>(SC_D.recordView->currentItem()->label())->longitude,
			dep,
			SC_D.currentRecord->unmapTime(p.x())
		);

		return;
	}

	if ( markerMode ) {
		// Reset uncertainties again
		m->setUncertainty(lowerUncertainty, upperUncertainty);

		// Fetch the current marker if changed in an action handler
		m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());

		if ( res == defineUncertainties ) {
			EditUncertainties dlg(this);

			dlg.setUncertainties(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
			connect(&dlg, SIGNAL(uncertaintiesChanged(double, double)),
			        this, SLOT(previewUncertainty(double, double)));

			int res = dlg.exec();

			m->setUncertainty(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
			m->update();

			if ( res == QDialog::Accepted ) {
				// Create a new pick if the current marker refers to an existing
				// pick already
				if ( m->pick() ) {
					PickerMarker *old = m;
					m = new PickerMarker(old->parent(), *old);
					m->convertToManualPick();
					old->setType(PickerMarker::Pick);
					old->parent()->setCurrentMarker(m);
				}
				m->setUncertainty(dlg.lowerUncertainty(), dlg.upperUncertainty());
			}

			updateUncertaintyHandles(m);

			SC_D.currentRecord->update();
			SC_D.recordView->currentItem()->widget()->update();
		}
		else {
			m->setUncertainty(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
			updateUncertaintyHandles(m);
			m->update();
			SC_D.currentRecord->update();
		}

		if ( plugins.contains(res) ) {
			PickerMarkerActionPlugin *plugin;
			plugin = static_cast<PickerMarkerActionPlugin*>(res->data().value<void*>());
			if ( plugin ) {
				if ( plugin->init(SC_D.currentRecord->streamID(), m->time()) ) {
					PickerRecordLabel *label = static_cast<PickerRecordLabel*>(SC_D.recordView->currentItem()->label());
					RecordSequence *seqZ = label->data.traces[0].raw;
					RecordSequence *seq1 = label->data.traces[1].raw;
					RecordSequence *seq2 = label->data.traces[2].raw;

					plugin->setRecords(seqZ, seq1, seq2);
					plugin->finalize();
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::currentMarkerChanged(Seiscomp::Gui::RecordMarker *m) {
	updateUncertaintyHandles(m);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateUncertaintyHandles(RecordMarker *marker) {
	if ( marker == nullptr ) {
		SC_D.timeScale->setSelectionEnabled(false);
		return;
	}

	PickerMarker *m = static_cast<PickerMarker*>(marker);
	bool canChangeUncertainty = true;

	if ( !m->isPick() && !m->isArrival() ) canChangeUncertainty = false;
	if ( m->pick() ) canChangeUncertainty = false;
	if ( !m->hasUncertainty() ) canChangeUncertainty = false;

	if ( canChangeUncertainty ) {
		SC_D.timeScale->setSelectionEnabled(true);
		SC_D.timeScale->setSelectionHandle(0, double(m->correctedTime()-SC_D.timeScale->alignment())-m->lowerUncertainty());
		SC_D.timeScale->setSelectionHandle(1, double(m->correctedTime()-SC_D.timeScale->alignment())+m->upperUncertainty());
	}
	else
		SC_D.timeScale->setSelectionEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::previewUncertainty(QAction *action) {
	bool ok;
	int idx = action->data().toInt(&ok);
	if ( !ok ) {
		// Reset
		previewUncertainty(SC_D.tmpLowerUncertainty, SC_D.tmpUpperUncertainty);
		return;
	}

	if ( idx == -1 )
		previewUncertainty(-1,-1);
	else if ( idx < 0 || idx >= SC_D.uncertainties.count() )
		return;
	else
		previewUncertainty(SC_D.uncertainties[idx].first, SC_D.uncertainties[idx].second);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::previewUncertainty(double lower, double upper) {
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());

	m->setUncertainty(lower, upper);
	updateUncertaintyHandles(m);
	SC_D.currentRecord->update();
	SC_D.recordView->currentItem()->widget()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::openConnectionInfo(const QPoint &p) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::destroyedSpectrumWidget(QObject *o) {
	QWidget *w = static_cast<QWidget*>(o);
	if ( w == SC_D.spectrumView ) {
		SC_D.spectrumView = nullptr;
		SC_D.defaultSpectrumWidgetSize = w->size();
		SC_D.spectrumWidgetGeometry = w->saveGeometry();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::ttInterfaceChanged(QString interface) {
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
void PickerView::ttTableChanged(QString tables) {
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
RecordViewItem* PickerView::addRawStream(const DataModel::SensorLocation *loc,
                                         const WaveformStreamID& sid,
                                         double distance,
                                         const std::string& text,
                                         bool theoreticalArrivals,
                                         const Stream *base) {
	WaveformStreamID streamID(sid);

	// Lookup station channel mapping
	QList<Config::ChannelMapItem> channelMapping = SC_D.config.channelMap.values((streamID.networkCode() + "." + streamID.stationCode()).c_str());
 	if ( channelMapping.isEmpty() )
		channelMapping = SC_D.config.channelMap.values((std::string("*.") + streamID.stationCode()).c_str());
	if ( channelMapping.isEmpty() )
		channelMapping = SC_D.config.channelMap.values((streamID.networkCode() + ".*").c_str());

	if ( !channelMapping.isEmpty() ) {
		QString channel = streamID.channelCode().substr(0,2).c_str();
		QString locChannel = (streamID.locationCode() + "." + streamID.channelCode().substr(0,2)).c_str();
		QListIterator<Config::ChannelMapItem> it(channelMapping);
		for ( it.toBack(); it.hasPrevious(); ) {
			const Config::ChannelMapItem &value = it.previous();
			if ( value.first == locChannel || value.first == channel ) {
				QStringList toks = value.second.split('.');
				if ( toks.size() == 1 )
					streamID.setChannelCode(toks[0].toStdString() + streamID.channelCode().substr(2));
				else if ( toks.size() == 2 ) {
					streamID.setLocationCode(toks[0].toStdString());
					streamID.setChannelCode(toks[1].toStdString() + streamID.channelCode().substr(2));
				}
				else
					SEISCOMP_WARNING("Invalid channel mapping target: %s", value.second.toStdString().c_str());

				break;
			}
		}
	}

	RecordViewItem *item = SC_D.recordView->addItem(adjustWaveformStreamID(streamID), text.c_str());
	if ( !item ) {
		return nullptr;
	}

	item->label()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(item->label(), SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(openContextMenu(const QPoint &)));

	if ( SC_D.currentRecord )
		item->widget()->setCursorText(SC_D.currentRecord->cursorText());

	item->label()->setText(text.c_str(), 0);
	QFont f(item->label()->font(0));
	f.setBold(true);
	item->label()->setFont(f, 0);

	QFontMetrics fm(f);
	item->label()->setWidth(fm.boundingRect("WWWW ").width(), 0);

	ThreeComponents tc;
	char comps[3] = {'Z', '1', '2'};

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
	label->data.setRecordWidget(item->widget());

	bool allComponents = true;
	label->gainUnit[0] = label->gainUnit[1] = label->gainUnit[2] = QString();

	if ( loc ) {
		getThreeComponents(tc, loc, streamID.channelCode().substr(0, streamID.channelCode().size()-1).c_str(), SC_D.origin->time());

		label->unit = UT_RAW;

		if ( tc.comps[ThreeComponents::Vertical] ) {
			comps[0] = *tc.comps[ThreeComponents::Vertical]->code().rbegin();
			label->gainUnit[0] = tc.comps[ThreeComponents::Vertical]->gainUnit().c_str();
			label->unit = fromGainUnit(tc.comps[ThreeComponents::Vertical]->gainUnit());
		}
		else {
			allComponents = false;
			if ( base )
				comps[0] = *base->code().rbegin();
			else
				comps[0] = COMP_NO_METADATA;
		}

		if ( tc.comps[ThreeComponents::FirstHorizontal] ) {
			comps[1] = *tc.comps[ThreeComponents::FirstHorizontal]->code().rbegin();
			label->gainUnit[1] = tc.comps[ThreeComponents::FirstHorizontal]->gainUnit().c_str();
			label->unit = fromGainUnit(tc.comps[ThreeComponents::FirstHorizontal]->gainUnit());
		}
		else {
			allComponents = false;
			comps[1] = COMP_NO_METADATA;
		}

		if ( tc.comps[ThreeComponents::SecondHorizontal] ) {
			comps[2] = *tc.comps[ThreeComponents::SecondHorizontal]->code().rbegin();
			label->gainUnit[2] = tc.comps[ThreeComponents::SecondHorizontal]->gainUnit().c_str();
			label->unit = fromGainUnit(tc.comps[ThreeComponents::SecondHorizontal]->gainUnit());
		}
		else {
			allComponents = false;
			comps[2] = COMP_NO_METADATA;
		}

		label->latitude = loc->latitude();
		label->longitude = loc->longitude();

		double delta, az, baz;
		Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
		                  label->latitude, label->longitude, &delta, &az, &baz);

		label->orientationZRT.loadRotateZ(deg2rad(baz + 180.0));
	}
	else {
		label->latitude = 999;
		label->longitude = 999;
		label->orientationZRT.identity();
		allComponents = false;
		comps[0] = COMP_NO_METADATA;
		comps[1] = COMP_NO_METADATA;
		comps[2] = COMP_NO_METADATA;
	}

	if ( !allComponents )
		SEISCOMP_WARNING("Unable to fetch all components of stream %s.%s.%s.%s",
		                 streamID.networkCode().c_str(), streamID.stationCode().c_str(),
		                 streamID.locationCode().c_str(), streamID.channelCode().substr(0,streamID.channelCode().size()-1).c_str());

	item->setData(QVariant(QString(text.c_str())));
	setupItem(comps, item);

	// Compute and set rotation matrix
	if ( allComponents ) {
		//cout << "[" << streamID.stationCode() << "]" << endl;
		try {
			Math::Vector3d n;
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

	applyFilter(item);
	applyRotation(item, SC_D.comboRotation->currentIndex());

	for ( int i = 0; i < 3; ++i ) {
		WaveformStreamID componentID = setWaveformIDComponent(streamID, comps[i]);
		label->data.traces[i].channelCode = componentID.channelCode();
		// Map waveformID to recordviewitem label
		SC_D.recordItemLabels[waveformIDToStdString(componentID)] = label;
	}

	if ( theoreticalArrivals )
		addTheoreticalArrivals(item, streamID.networkCode(), streamID.stationCode(), streamID.locationCode());

	for ( int i = 0; i < 3; ++i ) {
		if ( comps[i] == COMP_NO_METADATA ) continue;
		queueStream(distance, setWaveformIDComponent(streamID, comps[i]), Z12_COMPS[i]);
	}

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char PickerView::currentComponent() const {
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
void PickerView::setupItem(const char comps[3],
                           RecordViewItem* item) {
	connect(item->widget(), SIGNAL(cursorUpdated(RecordWidget*,int)),
	        this, SLOT(updateMainCursor(RecordWidget*,int)));

	connect(item, SIGNAL(componentChanged(RecordViewItem*, char)),
	        this, SLOT(updateItemLabel(RecordViewItem*, char)));

	connect(item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
	        this, SLOT(updateItemRecordState(const Seiscomp::Record*)));

	item->label()->setOrientation(Qt::Horizontal);
	item->label()->setToolTip("Timing quality: undefined");

	QPalette pal = item->widget()->palette();
	pal.setColor(QPalette::WindowText, QColor(128,128,128));
	//pal.setColor(QPalette::HighlightedText, QColor(128,128,128));
	item->widget()->setPalette(pal);

	item->widget()->setCustomBackgroundColor(SCScheme.colors.records.states.unrequested);

	item->widget()->setSlotCount(3);

	for ( int i = 0; i < 3; ++i ) {
		if ( comps[i] != COMP_NO_METADATA )
			item->insertComponent(comps[i], i);
		else
			item->widget()->setRecordID(i, "No metadata");
	}

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( inv ) {
		std::string channelCode = item->streamID().channelCode().substr(0,2);
		for ( int i = 0; i < 3; ++i ) {
			if ( comps[i] == COMP_NO_METADATA ) continue;
			Processing::Stream stream;
			try {
				stream.init(item->streamID().networkCode(),
				            item->streamID().stationCode(),
				            item->streamID().locationCode(),
				            channelCode + comps[i], SC_D.origin->time().value());
				if ( stream.gain > 0 )
					item->widget()->setRecordScale(i, 1E9 / stream.gain);
			}
			catch ( ... ) {}
		}
	}

	item->widget()->showScaledValues(SC_D.ui.actionShowTraceValuesInNmS->isChecked());
	updateRecordAxisLabel(item);

	// Default station distance is INFINITY to sort unknown stations
	// to the end of the view
	item->setValue(ITEM_DISTANCE_INDEX, std::numeric_limits<double>::infinity());
	// Default residual set to -INFINITY
	item->setValue(ITEM_RESIDUAL_INDEX, 0);
	// Default azimuth set to -INFINITY
	item->setValue(ITEM_AZIMUTH_INDEX, -std::numeric_limits<double>::infinity());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateMainCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 ) {
		if ( slot != SC_D.currentSlot ) {
			showComponent(comps[slot]);
		}
		if ( !SC_D.componentFollowsMouse && (slot != SC_D.currentRecord->currentRecords()) ) {
			updateSubCursor(w, slot);
		}
	}

	setCursorPos(w->cursorPos(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateSubCursor(RecordWidget* w, int s) {
	// Hide decorator on move
	if ( SC_D.currentRecord ) {
		auto d = static_cast<PickerTimeWindowDecorator*>(SC_D.currentRecord->decorator());
		if ( d ) {
			d->setVisible(false);
			SC_D.currentRecord->update();
		}
	}

	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 ) {
		if ( SC_D.componentFollowsMouse ) {
			if ( slot != SC_D.currentSlot ) {
				char comps[3] = {'Z', '1', '2'};
				showComponent(comps[slot]);
			}
		}
		else {
			QString text = SC_D.ui.labelCode->text();
			SC_D.currentRecord->setCurrentRecords(slot);

			int index = text.lastIndexOf(' ');
			if ( index >= 0 ) {
				if ( text.size() - index > 2 )
					text[text.size()-1] = SC_D.currentRecord->recordID(slot)[0];
				else
					text += SC_D.currentRecord->recordID(slot)[0];

				SC_D.ui.labelCode->setText(text);
			}
		}
	}

	if ( !SC_D.recordView->currentItem() ) return;

	SC_D.recordView->currentItem()->widget()->blockSignals(true);
	SC_D.recordView->currentItem()->widget()->setCursorPos(w->cursorPos());
	SC_D.recordView->currentItem()->widget()->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateRecordValue(Seiscomp::Core::Time t) {
	if ( !statusBar() ) return;

	const double *v = SC_D.currentRecord->value(t);

	if ( v == nullptr )
		statusBar()->clearMessage();
	else
		statusBar()->showMessage(QString("value = %1").arg(*v, 0, 'f', 2));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showTraceScaleToggled(bool e) {
	SC_D.currentRecord->showScaledValues(e);
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordViewItem* item = SC_D.recordView->itemAt(i);
		item->widget()->showScaledValues(e);
		updateRecordAxisLabel(item);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateItemLabel(RecordViewItem* item, char component) {
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

	int slot = item->mapComponentToSlot(component);

	if ( item == SC_D.recordView->currentItem() ) {
		QString text = SC_D.ui.labelCode->text();

		int index = text.lastIndexOf(' ');
		if ( index < 0 ) return;

		char comp = component;

		if ( slot >= 0 && slot < 3 ) {
			switch ( SC_D.comboRotation->currentIndex() ) {
				case RT_123:
					break;
				case RT_ZNE:
					comp = ZNE_COMPS[slot];
					break;
				case RT_ZRT:
					comp = ZRT_COMPS[slot];
					break;
				case RT_ZH:
					comp = ZH_COMPS[slot];
					break;
			}
		}

		if ( text.size() - index > 2 )
			text[text.size()-1] = comp;
		else
			text += comp;

		SC_D.ui.labelCode->setText(text);
	}

	updateTraceInfo(item, nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateItemRecordState(const Seiscomp::Record *rec) {
	// Reset acquisition related coloring since the first record
	// arrived already
	RecordViewItem *item = static_cast<RecordViewItem *>(sender());
	RecordWidget *widget = item->widget();
	int slot = item->mapComponentToSlot(*rec->channelCode().rbegin());
	widget->setRecordBackgroundColor(slot, SCScheme.colors.records.states.inProgress);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCursorPos(const Seiscomp::Core::Time& t, bool always) {
	SC_D.currentRecord->setCursorPos(t);

	if ( !always && SC_D.currentRecord->cursorText() == "" ) return;

	float offset = 0;

	if ( SC_D.centerSelection ) {
		float len = SC_D.recordView->currentItem()?
			SC_D.recordView->currentItem()->widget()->width()/SC_D.currentRecord->timeScale():
			SC_D.currentRecord->tmax() - SC_D.currentRecord->tmin();

		float pos = float(t - SC_D.currentRecord->alignment()) - len*SC_D.config.alignmentPosition;
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
void PickerView::setTimeRange(double tmin, double tmax) {
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
void PickerView::enableAutoScale() {
	SC_D.autoScaleZoomTrace = true;
	if ( SC_D.currentRecord ) {
		auto amplScale = SC_D.currentRecord->amplScale();
		SC_D.currentRecord->setNormalizationWindow(SC_D.currentRecord->visibleTimeWindow());
		SC_D.currentRecord->setAmplScale(amplScale);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::disableAutoScale() {
	SC_D.autoScaleZoomTrace = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setAlignment(Seiscomp::Core::Time t) {
	double offset = SC_D.currentRecord->alignment() - t;
	SC_D.currentRecord->setAlignment(t);

	// Because selection handle position are relative to the alignment
	// move them
	if ( SC_D.timeScale->isSelectionEnabled() ) {
		for ( int i = 0; i < SC_D.timeScale->selectionHandleCount(); ++i )
			SC_D.timeScale->setSelectionHandle(i, SC_D.timeScale->selectionHandlePos(i)+offset);
	}

	SC_D.timeScale->setAlignment(t);

	double tmin = SC_D.currentRecord->tmin()+offset;
	double tmax = SC_D.currentRecord->tmax()+offset;

	if ( SC_D.checkVisibility ) ensureVisibility(tmin, tmax);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::ensureVisibility(double &tmin, double &tmax) {
	if ( SC_D.recordView->currentItem() ) {
		RecordWidget* w = SC_D.recordView->currentItem()->widget();
		double leftOffset = tmin - w->tmin();
		double rightOffset = tmax - w->tmax();
		if ( leftOffset < 0 ) {
			tmin = w->tmin();
			tmax -= leftOffset;
		}
		else if ( rightOffset > 0 ) {
			double usedOffset = std::min(leftOffset, rightOffset);
			tmin -= usedOffset;
			tmax -= usedOffset;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin) {
	Core::Time left = time - Core::TimeSpan(pixelMargin/SC_D.currentRecord->timeScale());
	Core::Time right = time + Core::TimeSpan(pixelMargin/SC_D.currentRecord->timeScale());

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
void PickerView::moveTraces(double offset) {
	if ( fabs(offset) < 0.001 ) return;

	SC_D.recordView->move(offset);

	auto tmin = SC_D.recordView->timeRangeMin();
	auto tmax = SC_D.recordView->timeRangeMax();

	if ( tmin > SC_D.currentRecord->tmin() ) {
		offset = tmin - SC_D.currentRecord->tmin();
	}
	else if ( tmax < SC_D.currentRecord->tmax() ) {
		auto length = tmax - tmin;
		auto cr_length = SC_D.currentRecord->tmax() - SC_D.currentRecord->tmin();

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
void PickerView::move(double offset) {
	if ( fabs(offset) < 0.001 ) return;

	auto tmin = SC_D.currentRecord->tmin() + offset;
	auto tmax = SC_D.currentRecord->tmax() + offset;

	if ( tmin < SC_D.recordView->timeRangeMin() ) {
		offset = tmin - SC_D.recordView->timeRangeMin();
	}
	else if ( tmax > SC_D.recordView->timeRangeMax() ) {
		auto length = tmax - tmin;
		auto rv_length = SC_D.recordView->timeRangeMax() - SC_D.recordView->timeRangeMin();

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
void PickerView::itemSelected(RecordViewItem* item, RecordViewItem* lastItem) {
	double smin = 0;
	double smax = 0;

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
		SC_D.currentRecord->clearRecords();
		SC_D.currentRecord->setEnabled(false);
		SC_D.currentRecord->setMarkerSourceWidget(nullptr);
		static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setTraces(nullptr);
		return;
	}

	//SC_D.centerSelection = true;


	Core::Time cursorPos;
	RecordMarker* m = item->widget()->enabledMarker(item->widget()->cursorText());
	if ( m )
		cursorPos = m->correctedTime();
	else
		cursorPos = item->widget()->alignment() + relSelectedTime;

	//item->widget()->setCursorPos(cursorPos);
	//SC_D.currentRecord->setCursorPos(cursorPos);
	SC_D.currentRecord->setEnabled(item->widget()->isEnabled());

	connect(item->label(), SIGNAL(statusChanged(bool)),
	        this, SLOT(setCurrentRowEnabled(bool)));

	double amplScale = SC_D.currentRecord->amplScale();

	SC_D.currentRecord->setNormalizationWindow(item->widget()->normalizationWindow());
	SC_D.currentRecord->setAlignment(item->widget()->alignment());
	SC_D.timeScale->setAlignment(item->widget()->alignment());

	if ( smax - smin > 0 )
		setTimeRange(smin, smax);
	else
		setTimeRange(SC_D.recordView->timeRangeMin(),SC_D.recordView->timeRangeMax());

	//SC_D.currentRecord->setAmplScale(item->widget()->amplScale());
	SC_D.currentRecord->setAmplScale(amplScale);

	item->widget()->setShadowWidget(SC_D.currentRecord, false);
	SC_D.currentRecord->setMarkerSourceWidget(item->widget());

	if ( SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter(item);

	if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
		if ( SC_D.config.showAllComponents &&
		     SC_D.config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
			SC_D.currentRecord->setDrawMode(RecordWidget::InRows);
		else
			SC_D.currentRecord->setDrawMode(RecordWidget::Single);
	}
	else
		SC_D.currentRecord->setDrawMode(RecordWidget::Single);

	SC_D.ui.actionShowAllComponents->setEnabled(true);
	SC_D.ui.actionShowAllComponents->setChecked(SC_D.currentRecord->drawMode() == RecordWidget::InRows);

	/*
	SC_D.currentRecord->clearMarker();
	for ( int i = 0; i < item->widget()->markerCount(); ++i )
		new PickerMarker(SC_D.currentRecord, *static_cast<PickerMarker*>(item->widget()->marker(i)));
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
	int slot = item->mapComponentToSlot(component);

	if ( slot >= 0 && slot < 3 ) {
		switch ( SC_D.comboRotation->currentIndex() ) {
			case RT_123:
				break;
			case RT_ZNE:
				component = ZNE_COMPS[slot];
				break;
			case RT_ZRT:
				component = ZRT_COMPS[slot];
				break;
			case RT_ZH:
				component = ZH_COMPS[slot];
				break;
		}
	}

	for ( int i = 0; i < item->widget()->slotCount(); ++i ) {
		char code = SC_D.recordView->currentItem()->mapSlotToComponent(i);
		if ( code == '?' ) continue;

		switch ( SC_D.comboRotation->currentIndex() ) {
			case RT_123:
				SC_D.currentRecord->setRecordID(i, QString("%1").arg(code));
				break;
			case RT_ZNE:
				SC_D.currentRecord->setRecordID(i, QString("%1").arg(ZNE_COMPS[i]));
				break;
			case RT_ZRT:
				SC_D.currentRecord->setRecordID(i, QString("%1").arg(ZRT_COMPS[i]));
				break;
			case RT_ZH:
				SC_D.currentRecord->setRecordID(i, QString("%1").arg(ZH_COMPS[i]));
				break;
		}
	}

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

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setTraces(label->data.traces);

	currentMarkerChanged(SC_D.currentRecord->currentMarker());

#ifdef CENTER_SELECTION
	SC_D.centerSelection = true;
#endif
	//SC_D.currentRecord->enableFiltering(item->widget()->isFilteringEnabled());
	setCursorPos(cursorPos);
	SC_D.currentRecord->update();

	updateCurrentRowState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCurrentRowEnabled(bool enabled) {
	SC_D.currentRecord->setEnabled(enabled);
	updateCurrentRowState();

	RecordWidget* w = SC_D.recordView->currentItem()->widget();

	if ( w ) {
		for ( int i = 0; i < w->markerCount(); ++i ) {
			if ( w->marker(i)->id() >= 0 ) {
				emit arrivalChanged(w->marker(i)->id(), enabled?w->marker(i)->isEnabled():false);
				// To disable an arrival (trace) we have to keep this information
				// somewhere else not just in the picker
				emit arrivalEnableStateChanged(w->marker(i)->id(), enabled);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCurrentRowDisabled(bool disabled) {
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
void PickerView::setMarkerState(Seiscomp::Gui::RecordWidget* w, bool enabled) {
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
				emit arrivalChanged(arid, enabled);
				//emit arrivalEnableStateChanged(arid, enabled);
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
void PickerView::updateCurrentRowState() {
	//setMarkerState(SC_D.currentRecord, enabled);
	//SC_D.currentRecord->setEnabled(enabled);

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
void PickerView::updateTraceInfo(RecordViewItem* item,
                                 const Seiscomp::Record* rec) {
	float timingQuality = item->widget()->timingQuality(SC_D.currentSlot);
	if ( timingQuality >= 0 ) {
		if ( timingQuality > 100 ) timingQuality = 100;

		if ( timingQuality < 50 )
			static_cast<PickerRecordLabel*>(item->label())->setLabelColor(blend(SC_D.config.timingQualityMedium, SC_D.config.timingQualityLow, (int)(timingQuality*2)));
		else
			static_cast<PickerRecordLabel*>(item->label())->setLabelColor(blend(SC_D.config.timingQualityHigh, SC_D.config.timingQualityMedium, (int)((timingQuality-50)*2)));

		item->label()->setToolTip(QString("Timing quality: %1").arg((int)timingQuality));
	}
	else {
		static_cast<PickerRecordLabel*>(item->label())->removeLabelColor();
		item->label()->setToolTip("Timing quality: undefined");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::toggleFilter() {
	if ( SC_D.comboFilter->currentIndex() > 0 )
		SC_D.comboFilter->setCurrentIndex(0);
	else if ( SC_D.lastFilterIndex > 0 )
		SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::nextFilter() {
	// Filtering turned off
	int idx = SC_D.comboFilter->currentIndex();
	if ( idx == 0 ) return;

	++idx;
	if ( idx >= SC_D.comboFilter->count() )
		idx = 1;

	SC_D.comboFilter->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::previousFilter() {
	// Filtering turned off
	int idx = SC_D.comboFilter->currentIndex();
	if ( idx == 0 ) return;

	--idx;
	if ( idx < 1 )
		idx = SC_D.comboFilter->count()-1;

	SC_D.comboFilter->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addNewFilter(const QString& filter) {
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
void PickerView::scaleVisibleAmplitudes() {
	SC_D.recordView->scaleVisibleAmplitudes();

	SC_D.currentRecord->setNormalizationWindow(SC_D.currentRecord->visibleTimeWindow());
	SC_D.currentAmplScale = 1;
	SC_D.currentRecord->setAmplScale(0.0);
	//SC_D.currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*SC_D.currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::zoomSelectionHandleMoved(int handle, double pos, Qt::KeyboardModifiers mods) {
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
	if ( m == nullptr ) return;

	double trigger = double(m->correctedTime() - SC_D.timeScale->alignment());
	double value = pos - trigger;

	// Clip to a hundredth of a seconds
	if ( mods & Qt::ControlModifier ) {
		value *= 1E2;
		value = round(value);
		value *= 1E-2;
	}
	// Clip to a tenthousandth of a seconds
	else {
		value *= 1E4;
		value = round(value);
		value *= 1E-4;
	}

	switch ( handle ) {
		case 0:
			if ( value > 0 ) value = 0;

			if ( mods & Qt::ShiftModifier )
				m->setUncertainty(-value, -value);
			else
				m->setUncertainty(-value, m->upperUncertainty());

			m->setDrawUncertaintyValues(true);
			m->update();
			SC_D.currentRecord->update();

			SC_D.timeScale->setSelectionHandle(0, trigger-m->lowerUncertainty());
			SC_D.timeScale->setSelectionHandle(1, trigger+m->upperUncertainty());
			break;
		case 1:
			if ( value < 0 ) value = 0;

			if ( mods & Qt::ShiftModifier )
				m->setUncertainty(value, value);
			else
				m->setUncertainty(m->lowerUncertainty(), value);

			m->setDrawUncertaintyValues(true);
			m->update();
			SC_D.currentRecord->update();

			SC_D.timeScale->setSelectionHandle(0, trigger-m->lowerUncertainty());
			SC_D.timeScale->setSelectionHandle(1, trigger+m->upperUncertainty());
			break;
		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::zoomSelectionHandleMoveFinished() {
	PickerMarker *m = static_cast<PickerMarker*>(SC_D.currentRecord->currentMarker());
	if ( m == nullptr ) return;

	m->setDrawUncertaintyValues(false);
	m->update();
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeScale(double, double) {
	zoom(1.0);
	/*
	std::cout << "[changeScale]" << std::endl;
	std::cout << "new TimeScale(" << SC_D.currentRecord->timeScale() << ")" << std::endl;
	std::cout << "current TimeRange = " << SC_D.currentRecord->tmin() << ", " << SC_D.currentRecord->tmax() << ")" << std::endl;
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeTimeRange(double, double) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortAlphabetically() {
	SC_D.recordView->sortByTextAndValue(0, ITEM_PRIORITY_INDEX);

	SC_D.ui.actionSortAlphabetically->setChecked(true);
	SC_D.ui.actionSortByDistance->setChecked(false);
	SC_D.ui.actionSortByAzimuth->setChecked(false);
	SC_D.ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByDistance() {
	SC_D.recordView->sortByValue(ITEM_DISTANCE_INDEX, ITEM_PRIORITY_INDEX);

	SC_D.ui.actionSortAlphabetically->setChecked(false);
	SC_D.ui.actionSortByDistance->setChecked(true);
	SC_D.ui.actionSortByAzimuth->setChecked(false);
	SC_D.ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByAzimuth() {
	SC_D.recordView->sortByValue(ITEM_AZIMUTH_INDEX, ITEM_PRIORITY_INDEX);

	SC_D.ui.actionSortAlphabetically->setChecked(false);
	SC_D.ui.actionSortByDistance->setChecked(false);
	SC_D.ui.actionSortByAzimuth->setChecked(true);
	SC_D.ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByResidual() {
	SC_D.recordView->sortByValue(ITEM_RESIDUAL_INDEX);

	SC_D.ui.actionSortAlphabetically->setChecked(false);
	SC_D.ui.actionSortByDistance->setChecked(false);
	SC_D.ui.actionSortByAzimuth->setChecked(false);
	SC_D.ui.actionSortByResidual->setChecked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByPhase(const QString& phase) {
	SC_D.recordView->sortByMarkerTime(phase);

	SC_D.ui.actionSortAlphabetically->setChecked(false);
	SC_D.ui.actionSortByDistance->setChecked(false);
	SC_D.ui.actionSortByAzimuth->setChecked(false);
	SC_D.ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showAllComponents(bool showAll) {
	if ( SC_D.currentRecord ) {
		SC_D.currentRecord->setDrawMode(showAll ? RecordWidget::InRows : RecordWidget::Single);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showZComponent() {
	showComponent('Z');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showNComponent() {
	showComponent('1');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showEComponent() {
	showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnOriginTime() {
	SC_D.checkVisibility = false;
	SC_D.recordView->setAbsoluteTimeEnabled(true);
	SC_D.recordView->setTimeRange(SC_D.minTime, SC_D.maxTime);
	SC_D.checkVisibility = true;

	SC_D.recordView->setAlignment(SC_D.origin->time());

	SC_D.alignedOnOT = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnPArrivals() {
	alignOnPhase("P", false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnSArrivals() {
	alignOnPhase("S", false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::pickP(bool) {
	setCursorText("P");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::pickNone(bool) {
	if ( SC_D.recordView->currentItem() ) {
		// Only close widget if picking is already disabled
		if ( SC_D.recordView->currentItem()->widget()->cursorText().isEmpty() && SC_D.spectrumView )
			SC_D.spectrumView->close();
	}

	setCursorText("");

	if ( SC_D.recordView->currentItem() )
		SC_D.recordView->currentItem()->widget()->setCurrentMarker(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::pickS(bool) {
	setCursorText("S");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleAmplUp() {
	auto scale = SC_D.currentRecord->amplScale();
	//if ( scale >= 1 ) scale = SC_D.currentAmplScale;
	auto value = (scale == 0 ? 1.0 : scale) * SC_D.recordView->zoomFactor();
	if ( value > 1000 ) value = 1000;
	if ( /*value < 1*/true ) {
		SC_D.currentRecord->setAmplScale(value);
		SC_D.currentAmplScale = 1;
	}
	else {
		SC_D.currentRecord->setAmplScale(1);
		SC_D.currentAmplScale = value;
	}

	//SC_D.currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*SC_D.currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleAmplDown() {
	auto scale = SC_D.currentRecord->amplScale();
	//if ( scale >= 1 ) scale = SC_D.currentAmplScale;
	auto value = (scale == 0 ? 1.0 : scale) / SC_D.recordView->zoomFactor();
	//if ( value < 1 ) value = 1;
	if ( value < 0.001 ) value = 0.001;

	//SC_D.currentRecord->setAmplScale(value);
	if ( /*value < 1*/true ) {
		SC_D.currentRecord->setAmplScale(value);
		SC_D.currentAmplScale = 1;
	}
	else {
		SC_D.currentRecord->setAmplScale(1);
		SC_D.currentAmplScale = value;
	}

	//SC_D.currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*SC_D.currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleReset() {
	SC_D.currentRecord->setAmplScale(1.0);
	SC_D.currentAmplScale = 1.0;
	zoom(0.0);

	//SC_D.currentRecord->resize(SC_D.zoomTrace->width(), (int)(SC_D.zoomTrace->height()*SC_D.currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleTimeUp() {
	zoom(SC_D.recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleTimeDown() {
	zoom(1.0/SC_D.recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::zoom(float factor) {
	SC_D.zoom *= factor;
	if ( SC_D.zoom < 1.0 )
		SC_D.zoom = 1.0;

	if ( SC_D.zoom > 100 )
		SC_D.zoom = 100;

	double currentScale = SC_D.currentRecord->timeScale();
	double newScale = SC_D.recordView->timeScale()*SC_D.zoom;

	factor = newScale/currentScale;

	double tmin = SC_D.currentRecord->tmin();
	double tmax = SC_D.recordView->currentItem()?
		tmin + SC_D.recordView->currentItem()->widget()->width()/SC_D.currentRecord->timeScale():
		SC_D.currentRecord->tmax();
	double tcen = tmin + (tmax-tmin)*0.5;

	tmin = tcen - (tcen-tmin)/factor;
	tmax = tcen + (tmax-tcen)/factor;

	SC_D.currentRecord->setTimeScale(newScale);
	SC_D.timeScale->setScale(newScale);

	if ( SC_D.checkVisibility ) ensureVisibility(tmin, tmax);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::applyTimeRange(double rmin, double rmax) {
	double tmin = rmin;
	double tmax = rmax;

	double newScale = SC_D.currentRecord->canvasRect().width() / (tmax-tmin);
	if ( newScale < SC_D.recordView->timeScale() )
		newScale = SC_D.recordView->timeScale();

	if ( tmin < SC_D.recordView->currentItem()->widget()->tmin() )
		tmin = SC_D.recordView->currentItem()->widget()->tmin();

	SC_D.currentRecord->setTimeScale(newScale);
	SC_D.timeScale->setScale(newScale);

	// Calculate zoom
	SC_D.zoom = newScale / SC_D.recordView->timeScale();

	if ( SC_D.checkVisibility ) ensureVisibility(tmin, tmax);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scrollLeft() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp -= Core::TimeSpan((float)width()/(20*SC_D.currentRecord->timeScale()));
		setCursorPos(cp);
		return;
	}

	float offset = -(float)width()/(8*SC_D.currentRecord->timeScale());
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scrollFineLeft() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp -= Core::TimeSpan(1.0/SC_D.currentRecord->timeScale());
		setCursorPos(cp);
		return;
	}

	float offset = -1.0/SC_D.currentRecord->timeScale();
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scrollRight() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp += Core::TimeSpan((float)width()/(20*SC_D.currentRecord->timeScale()));
		setCursorPos(cp);
		return;
	}

	float offset = (float)width()/(8*SC_D.currentRecord->timeScale());
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scrollFineRight() {
	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();
		cp += Core::TimeSpan(1.0/SC_D.currentRecord->timeScale());
		setCursorPos(cp);
		return;
	}

	float offset = 1.0/SC_D.currentRecord->timeScale();
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::automaticRepick() {
	if ( SC_D.comboPicker == nullptr ) {
		statusBar()->showMessage("Automatic picking: no picker available", 2000);
		return;
	}

	if ( SC_D.recordView->currentItem() == nullptr ) {
		statusBar()->showMessage("Automatic picking: no active row", 2000);
		return;
	}

	if ( !SC_D.currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = SC_D.currentRecord->cursorPos();

		RecordSequence *seq =
			SC_D.currentRecord->isFilteringEnabled()
				?
				SC_D.currentRecord->filteredRecords(SC_D.currentSlot)
				:
				SC_D.currentRecord->records(SC_D.currentSlot);
		if ( seq ) {
			Processing::PickerPtr picker =
				Processing::PickerFactory::Create(SC_D.comboPicker->currentText().toStdString().c_str());

			if ( picker == nullptr ) {
				statusBar()->showMessage(QString("Automatic picking: unable to create picker '%1'").arg(SC_D.comboPicker->currentText()), 2000);
				return;
			}

			WaveformStreamID wid = SC_D.recordView->streamID(SC_D.recordView->currentItem()->row());
			KeyValues params;
			DataModel::ConfigModule *module = SCApp->configModule();
			if ( module != nullptr ) {
				for ( size_t i = 0; i < module->configStationCount(); ++i ) {
					DataModel::ConfigStation *station = module->configStation(i);
					if ( station->networkCode() != wid.networkCode() ||
					     station->stationCode() != wid.stationCode() ) continue;

					DataModel::Setup *configSetup = DataModel::findSetup(station, SCApp->name(), true);

					if ( configSetup ) {
						DataModel::ParameterSet* ps = nullptr;
						try {
							ps = DataModel::ParameterSet::Find(configSetup->parameterSetID());
						}
						catch ( Core::ValueException & ) {
							continue;
						}

						if ( !ps ) {
							SEISCOMP_ERROR("Cannot find parameter set %s",
							               configSetup->parameterSetID().c_str());
							continue;
						}

						params.init(ps);
					}
				}
			}

			if ( !picker->setup(Processing::Settings(SCApp->configModuleName(),
			                    wid.networkCode(), wid.stationCode(),
			                    wid.locationCode(), wid.channelCode(), &SCApp->configuration(),
			                    &params)) ) {
				statusBar()->showMessage("Automatic picking: unable to inialize picker");
				return;
			}

			if ( SC_D.config.repickerSignalStart ) {
				picker->setSignalStart(*SC_D.config.repickerSignalStart);
				SEISCOMP_DEBUG("Set repick start to %.2f", *SC_D.config.repickerSignalStart);
			}
			if ( SC_D.config.repickerSignalEnd ) {
				picker->setSignalEnd(*SC_D.config.repickerSignalEnd);
				SEISCOMP_DEBUG("Set repick end to %.2f", *SC_D.config.repickerSignalEnd);
			}

			picker->setTrigger(cp);
			picker->setPublishFunction(bind(&PickerView::emitPick, this, placeholders::_1, placeholders::_2));
			picker->computeTimeWindow();

			SEISCOMP_DEBUG("%s: ns=%f, ss=%f, se=%f", picker->methodID().c_str(),
			               picker->config().noiseBegin,
			               picker->config().signalBegin,
			               picker->config().signalEnd);
			SEISCOMP_DEBUG("%s: tw = %s ~ %s", picker->methodID().c_str(),
			               picker->timeWindow().startTime().iso().c_str(),
			               picker->timeWindow().endTime().iso().c_str());

			int count = picker->feedSequence(seq);
			statusBar()->showMessage(QString("Fed %1 of %2 records: state = %3(%4)")
			                         .arg(count)
			                         .arg(seq->size())
			                         .arg(picker->status().toString())
			                         .arg(picker->statusValue()), 2000);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::gotoNextMarker() {
	//if ( SC_D.currentRecord->cursorText().isEmpty() ) return;
	bool active = !SC_D.currentRecord->cursorText().isEmpty();

	RecordMarker *m = SC_D.currentRecord->nextMarker(SC_D.currentRecord->cursorPos());
	if ( m ) {
		bool oldCenter = SC_D.centerSelection;
		SC_D.centerSelection = active;
		setCursorPos(m->correctedTime());
		SC_D.currentRecord->setCurrentMarker(m);
		SC_D.centerSelection = oldCenter;

		ensureVisibility(m->correctedTime(), 5);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::gotoPreviousMarker() {
	//if ( SC_D.currentRecord->cursorText().isEmpty() ) return;
	bool active = !SC_D.currentRecord->cursorText().isEmpty();

	RecordMarker *m = SC_D.currentRecord->lastMarker(SC_D.currentRecord->cursorPos());
	if ( m ) {
		bool oldCenter = SC_D.centerSelection;
		SC_D.centerSelection = active;
		setCursorPos(m->correctedTime());
		SC_D.currentRecord->setCurrentMarker(m);
		SC_D.centerSelection = oldCenter;

		ensureVisibility(m->correctedTime(), 5);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::hasModifiedPicks() const {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = SC_D.recordView->itemAt(r);
		RecordWidget* widget = rvi->widget();
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker* marker = widget->marker(m);
			if ( marker->isModified() )
				return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::fetchManualPicks(std::vector<RecordMarker*>* markers) const {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = SC_D.recordView->itemAt(r);
		RecordWidget* widget = rvi->widget();

		if ( !widget->isEnabled() ) continue;

		// The number of markers for a given phase
		QMap<QString, bool> manualMarkers;
		bool hasManualPick = false;

		// Count the number of interesting markers for a particular phase
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			PickerMarker* marker = (PickerMarker*)widget->marker(m);
			//if ( (marker->isMovable() && (marker->isEnabled() || marker->pick() != nullptr)) )
			if ( marker->isArrival() && (marker->pick() == nullptr) )
				manualMarkers[marker->text()] = true;
		}

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			PickerMarker* marker = (PickerMarker*)widget->marker(m);

			// If the marker is not an arrival do nothing
			if ( !marker->isArrival() ) continue;

			bool hasManualMarkerForPhase = manualMarkers[marker->text()];

			PickPtr pick = marker->pick();

			// Picked marker and we've got an manual replacement: do nothing
			if ( hasManualMarkerForPhase && marker->pick() ) {
				SEISCOMP_DEBUG("   - ignore pick to be replaced");
				marker->setId(-1);
				continue;
			}

			/*
			// If the marker is not enabled do nothing and reset its bound arrival id
			if ( !marker->isEnabled() ) {
				// Preset markers are ignored when there is no manual arrival
				if ( hasManualMarkerForPhase && !marker->isMovable() )
					marker->setId(-1);
				else {
					if ( markers )
						markers->push_back(marker);
					SEISCOMP_DEBUG("   - reuse existing pick");
				}

				continue;
			}
			*/

			if ( pick ) {
				SEISCOMP_DEBUG("Checking existing pick, modified = %d", marker->isModified());
				if ( !marker->isModified() ) {
					if ( markers )
						markers->push_back(marker);
					SEISCOMP_DEBUG("   - reuse existing pick");
					continue;
				}
			}

			PickPtr p = findPick(widget, marker->correctedTime());
			// If the marker did not make any changes to the pick
			// attributes (polariy, uncertainty, ...) reuse it.
			if ( p && !marker->equalsPick(p.get()) ) p = nullptr;

			if ( !p ) {
				WaveformStreamID s = SC_D.recordView->streamID(r);
				p = Pick::Create();
				p->setWaveformID(WaveformStreamID(s.networkCode(), s.stationCode(),
				                                  s.locationCode(), s.channelCode().substr(0,2), ""));

#ifdef SET_PICKED_COMPONENT
				if ( marker->slot() >= 0 && marker->slot() < 3 ) {
					char comp;
					switch ( marker->rotation() ) {
						default:
						case RT_123:
							comp = rvi->mapSlotToComponent(marker->slot());
							break;
						case RT_ZNE:
							comp = ZNE_COMPS[marker->slot()];
							break;
						case RT_ZRT:
							comp = ZRT_COMPS[marker->slot()];
							break;
						case RT_ZH:
							comp = ZH_COMPS[marker->slot()];
							break;
					}

					p->waveformID().setChannelCode(p->waveformID().channelCode() + comp);
				}
				else if ( !marker->channelCode().empty() )
					p->waveformID().setChannelCode(marker->channelCode());
#endif

				p->setTime(marker->correctedTime());

				if ( marker->lowerUncertainty() >= 0 )
					p->time().setLowerUncertainty(marker->lowerUncertainty());
				if ( marker->upperUncertainty() >= 0 )
					p->time().setUpperUncertainty(marker->upperUncertainty());

				if ( !marker->filter().isEmpty() )
					p->setFilterID(marker->filter().toStdString());
				p->setPhaseHint(Phase((const char*)marker->text().toLatin1()));
				p->setEvaluationMode(EvaluationMode(MANUAL));
				p->setPolarity(marker->polarity());
				p->setBackazimuth(marker->backazimuth());
				p->setHorizontalSlowness(marker->horizontalSlowness());
				p->setOnset(marker->onset());
				CreationInfo ci;
				ci.setAgencyID(SCApp->agencyID());
				ci.setAuthor(SCApp->author());
				ci.setCreationTime(Core::Time::GMT());
				p->setCreationInfo(ci);

				SC_D.changedPicks.push_back(ObjectChangeList<DataModel::Pick>::value_type(p,true));
				SEISCOMP_DEBUG("   - created new pick");

				hasManualPick = true;
			}
			else {
				SEISCOMP_DEBUG("   - reuse active pick");
			}

			if ( markers ) markers->push_back(marker);
			marker->setPick(p.get());
		}

		// Remove automatic station picks if configured
		if ( hasManualPick && SC_D.config.removeAutomaticStationPicks && markers ) {
			for ( std::vector<RecordMarker*>::iterator it = markers->begin();
			      it != markers->end(); ) {
				try {
					if ( static_cast<PickerMarker*>(*it)->pick()->evaluationMode() == MANUAL ) {
						++it;
						continue;
					}
				}
				catch ( ... ) {}

				if ( (*it)->parent() == widget )
					it = markers->erase(it);
				else
					++it;
			}
		}
	}

	// Remove all automatic picks if configured
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
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::applyPicks() {
	for ( int r = 0; r < SC_D.recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = SC_D.recordView->itemAt(r);
		RecordWidget* widget = rvi->widget();

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker* marker = widget->marker(m);
			marker->apply();
		}
	}

	SC_D.changedPicks.clear();
	SC_D.recordView->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::getChangedPicks(ObjectChangeList<DataModel::Pick> &list) const {
	list = SC_D.changedPicks;
	SC_D.changedPicks.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setDefaultDisplay() {
	//alignByState();
	alignOnOriginTime();
	selectFirstVisibleItem(SC_D.recordView);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
bool PickerView::start() {
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
void PickerView::stop() {
	SC_D.recordView->stop();
	closeThreads();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::selectTrace(const std::string &net, const std::string &code) {
	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		if ( SC_D.recordView->itemAt(i)->streamID().networkCode() != net ) continue;
		if ( SC_D.recordView->itemAt(i)->streamID().stationCode() != code ) continue;
		SC_D.recordView->setCurrentItem(SC_D.recordView->itemAt(i));
		SC_D.recordView->ensureVisible(i);
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid) {
	RecordViewItem *item = SC_D.recordView->item(wid);
	if ( item ) {
		SC_D.recordView->setCurrentItem(item);
		SC_D.recordView->ensureVisible(item->row());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::closeThreads() {
	foreach ( RecordStreamThread* t, SC_D.acquisitionThreads) {
		disconnect(t, SIGNAL(handleError(const QString &)),
		           this, SLOT(handleAcquisitionError(const QString &)));

		t->stop(true);
		SEISCOMP_DEBUG("removed finished thread %d from list", t->ID());
		delete t;
	}

	SC_D.acquisitionThreads.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::handleAcquisitionError(const QString &msg) {
	QMessageBox::critical(this, tr("Acquistion error"), msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::acquisitionFinished() {
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
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());

			for ( int i = 0; i < 3; ++i ) {
				if ( label->data.traces[i].thread != t ) continue;
				if ( label->data.traces[i].raw && !label->data.traces[i].raw->empty() )
					widget->removeRecordBackgroundColor(i);
				else
					widget->setRecordBackgroundColor(i, SCScheme.colors.records.states.notAvailable);
				// Reset the thread
				label->data.traces[i].thread = nullptr;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::acquireStreams() {
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

	connect(t, SIGNAL(handleError(const QString &)),
	        this, SLOT(handleAcquisitionError(const QString &)));

	connect(t, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));

	connect(t, SIGNAL(finished()),
	        this, SLOT(acquisitionFinished()));


	t->setTimeWindow(SC_D.timeWindow);

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
		else
			t->addStream(it->streamID.networkCode(),
			             it->streamID.stationCode(),
			             it->streamID.locationCode(),
			             it->streamID.channelCode());

		RecordViewItem *item = SC_D.recordView->item(adjustWaveformStreamID(it->streamID));
		if ( item ) {
			int slot = item->mapComponentToSlot(*it->streamID.channelCode().rbegin());
			item->widget()->setRecordBackgroundColor(slot, SCScheme.colors.records.states.requested);
			// Store the acquisition thread as user data
			static_cast<PickerRecordLabel*>(item->label())->data.traces[slot].thread = t;
			item->widget()->setRecordUserData(slot, QVariant::fromValue((void*)t));
		}
	}

	SC_D.nextStreams.clear();

	SC_D.acquisitionThreads.push_back(t);
	t->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::receivedRecord(Seiscomp::Record *rec) {
	Seiscomp::RecordPtr tmp(rec);
	if ( !rec->data() ) return;

	std::string streamID = rec->streamID();
	auto it = SC_D.recordItemLabels.find(streamID);
	if ( it == SC_D.recordItemLabels.end() )
		return;

	PickerRecordLabel *label = it->second;
	int i;
	for ( i = 0; i < 3; ++i ) {
		if ( label->data.traces[i].channelCode == rec->channelCode() ) {
			if ( label->data.traces[i].raw == nullptr )
				label->data.traces[i].raw = new TimeWindowBuffer(SC_D.timeWindow);
			break;
		}
	}

	if ( i == 3 ) return;

	bool firstRecord = label->data.traces[i].raw->empty();

	if ( !label->data.traces[i].raw->feed(rec) ) return;

	if ( label->recordViewItem() == SC_D.recordView->currentItem() )
		static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->feedRaw(i, rec);

	// Check for out-of-order records
	if ( (label->data.traces[i].filter || label->data.enableTransformation) &&
	     label->data.traces[i].raw->back() != (const Record*)rec ) {
//		SEISCOMP_DEBUG("%s.%s.%s.%s: out of order record, reinitialize trace",
//		               rec->networkCode().c_str(),
//		               rec->stationCode().c_str(),
//		               rec->locationCode().c_str(),
//		               rec->channelCode().c_str());
		label->data.reset();
	}
	else
		label->data.transform(i, rec);

	RecordViewItem *item = label->recordViewItem();

	if ( firstRecord ) {
		item->widget()->setRecordBackgroundColor(i, SCScheme.colors.records.states.inProgress);
		label->hasGotData = true;

		if ( SC_D.config.hideStationsWithoutData ) {
			if ( !isTracePicked(item->widget()) )
				item->forceInvisibilty(!label->isEnabledByConfig && SC_D.config.hideDisabledStations);
		}

		// If this item is linked to another item, enable the expand button of
		// the controller
		if ( label->isLinkedItem() && label->_linkedItem != nullptr )
			static_cast<PickerRecordLabel*>(label->_linkedItem->label())->enabledExpandButton(item);
	}
	else {
		// Tell the widget to rebuild its traces
		//item->widget()->fed(i, rec);
		updateTraceInfo(item, rec);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::relocate() {
	std::vector<RecordMarker*> markers;
	fetchManualPicks(&markers);

	QMap<QString, PickerMarker*> pick2Marker;

	OriginPtr tmpOrigin = Origin::Create();
	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());
	tmpOrigin->assign(SC_D.origin.get());
	tmpOrigin->setCreationInfo(ci);

	double rms = 0.0;
	size_t rmsCount = 0;

	for ( size_t i = 0; i < markers.size(); ++i ) {
		RecordMarker *m = markers[i];
		PickerMarker *pm = static_cast<PickerMarker*>(m);
		PickPtr pick = ((PickerMarker*)markers[i])->pick();
		if ( !pick ) {
			SEISCOMP_ERROR("Pick not set in marker");
			continue;
		}

		SensorLocation* sloc = Client::Inventory::Instance()->getSensorLocation(pick.get());

		// Remove pick, when no station is configured for this pick
		if ( !sloc /*&& !m->isEnabled()*/ ) continue;

		double delta, az1, az2;
		Math::Geo::delazi(tmpOrigin->latitude(), tmpOrigin->longitude(),
		                  sloc->latitude(), sloc->longitude(), &delta, &az1, &az2);

		ArrivalPtr a = new Arrival();

		RecordWidget *w = m->parent();
		RecordMarker *tm = w->marker(m->text() + THEORETICAL_POSTFIX);
		if ( tm ) {
			a->setTimeResidual(double(pick->time().value() - tm->time()));
			if ( m->isEnabled() ) {
				rms += a->timeResidual() * a->timeResidual();
				++rmsCount;
			}
		}

		a->setDistance(delta);
		a->setAzimuth(az1);
		a->setPickID(pick->publicID());
		a->setWeight(m->isEnabled()/* && markers[i]->isMovable()*/ ? 1 : 0);
		a->setTimeUsed(m->isEnabled());
		a->setPhase(m->text().toStdString());
		tmpOrigin->add(a.get());
		pick2Marker[pick->publicID().c_str()] = static_cast<PickerMarker*>(markers[i]);

		if ( SC_D.origin ) {
			auto existingArrival = SC_D.origin->arrival(pick->publicID());
			if ( existingArrival ) {
				if ( m->isEnabled() ) {
					try {
						a->setBackazimuthUsed(existingArrival->backazimuthUsed());
					}
					catch ( ... ) {}
					try {
						a->setHorizontalSlownessUsed(existingArrival->horizontalSlownessUsed());
					}
					catch ( ... ) {}
				}
				else {
					if ( pm->backazimuth() ) {
						a->setBackazimuthUsed(false);
					}
					if ( pm->horizontalSlowness() ) {
						a->setHorizontalSlownessUsed(false);
					}
				}
			}
		}
	}

	OriginQuality q;
	try { q = tmpOrigin->quality(); } catch ( Core::ValueException & ) {}

	if ( rmsCount > 0 )
		q.setStandardError(sqrt(rms / rmsCount));
	else
		q.setStandardError(Core::None);

	tmpOrigin->setQuality(q);

	try {
		OriginPtr o = tmpOrigin;

		if ( o ) {
			SC_D.origin = o;

			for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
				PickerMarker* m = pick2Marker[o->arrival(i)->pickID().c_str()];
				if ( m ) {
					m->setId(i);
				}
			}

			emit originCreated(SC_D.origin.get());

			// Only clear the manual picks if the origin has been located
			// successfully. Otherwise the manual picks get lost after the
			// next successfully location
			SC_D.changedPicks.clear();

			setOrigin(SC_D.origin.get());
		}
		else {
			QMessageBox::critical(this, "Relocation error", "Relocation failed for some reason");
		}
	}
	catch ( Core::GeneralException& e ) {
		QMessageBox::critical(this, "Relocation error", e.what());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::modifyOrigin() {
	double dep = SC_D.config.defaultDepth;
	try { dep = SC_D.origin->depth(); } catch ( ... ) {}
	OriginDialog dialog(SC_D.origin->longitude().value(), SC_D.origin->latitude().value(), dep, this);
	dialog.setTime(SC_D.origin->time().value());
	dialog.setWindowTitle("Modify origin");
	dialog.setSendButtonText("Apply");
	if ( dialog.exec() == QDialog::Accepted ) {
		OriginPtr tmpOrigin = Origin::Create();
		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::GMT());
		//tmpOrigin->assign(SC_D.origin.get());
		tmpOrigin->setLatitude(dialog.latitude());
		tmpOrigin->setLongitude(dialog.longitude());
		tmpOrigin->setTime(Core::Time(dialog.getTime_t()));
		tmpOrigin->setDepth(RealQuantity(dialog.depth()));
		tmpOrigin->setDepthType(OriginDepthType(OPERATOR_ASSIGNED));
		tmpOrigin->setEvaluationMode(EvaluationMode(MANUAL));
		tmpOrigin->setCreationInfo(ci);
		for ( size_t i = 0; i < SC_D.origin->arrivalCount(); ++i ) {
			ArrivalPtr ar = new Arrival(*SC_D.origin->arrival(i));
			tmpOrigin->add(ar.get());
		}
		setOrigin(tmpOrigin.get());
		updateLayoutFromState();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::diffStreamState(Seiscomp::DataModel::Origin* oldOrigin,
                                 Seiscomp::DataModel::Origin* newOrigin) {
	// Do nothing for now
	return;

	QSet<QString> oldPicks, newPicks;

	for ( size_t i = 0; i < oldOrigin->arrivalCount(); ++i )
		oldPicks.insert(oldOrigin->arrival(i)->pickID().c_str());

	for ( size_t i = 0; i < newOrigin->arrivalCount(); ++i )
		newPicks.insert(newOrigin->arrival(i)->pickID().c_str());

	oldPicks -= newPicks;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addStations() {
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

		Stream *stream = nullptr;

		stream = findConfiguredStream(s, SC_D.origin->time());

		if ( stream == nullptr ) {
			// Preferred channel code is BH. If not available use either SH or skip.
			for ( size_t c = 0; c < SC_D.broadBandCodes.size(); ++c ) {
				stream = findStream(s, SC_D.broadBandCodes[c], SC_D.origin->time());
				if ( stream ) break;
			}
		}

		if ( stream == nullptr )
			stream = findStream(s, SC_D.origin->time(), Processing::WaveformProcessor::MeterPerSecond);
		if ( stream == nullptr )
			stream = findStream(s, SC_D.origin->time(), Processing::WaveformProcessor::MeterPerSecondSquared);
		if ( stream == nullptr )
			stream = findStream(s, SC_D.origin->time(), Processing::WaveformProcessor::Meter);

		if ( stream ) {
			WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");

			double delta, az1, az2;
			if ( SC_D.origin )
				Math::Geo::delazi(SC_D.origin->latitude(), SC_D.origin->longitude(),
				                  stream->sensorLocation()->latitude(),
				                  stream->sensorLocation()->longitude(), &delta, &az1, &az2);
			else
				delta = 0;

			RecordViewItem* item = addStream(stream->sensorLocation(), streamID,
			                                 delta, streamID.stationCode().c_str(),
			                                 false, true, stream);
			if ( item ) {
				SC_D.stations.insert(code);
				item->setVisible(!SC_D.ui.actionShowUsedStations->isChecked());
				if ( SC_D.config.hideStationsWithoutData )
					item->forceInvisibilty(true);
			}
		}
	}

	fillRawPicks();

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
void PickerView::searchStation() {
	SC_D.searchStation->selectAll();
	SC_D.searchStation->setVisible(true);
	SC_D.searchLabel->setVisible(true);

	//SC_D.searchStation->grabKeyboard();
	SC_D.searchStation->setFocus();
	SC_D.recordView->setFocusProxy(SC_D.searchStation);

	SC_D.ui.actionCreatePick->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::searchByText(const QString &text) {
	if ( text.isEmpty() ) return;

	QRegExp rx(text + "*");
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	while ( true ) {
		int row = SC_D.recordView->findByText(0, rx, SC_D.lastFoundRow+1);
		if ( row != -1 ) {
			SC_D.lastFoundRow = row;

			if ( !SC_D.recordView->itemAt(row)->isVisible() ) continue;

			SC_D.recordView->setCurrentItem(SC_D.recordView->itemAt(row));
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

		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::search(const QString &text) {
	SC_D.lastFoundRow = -1;

	searchByText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::nextSearch() {
	searchByText(SC_D.searchStation->text());
	if ( SC_D.lastFoundRow == -1 )
		searchByText(SC_D.searchStation->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::abortSearchStation() {
	SC_D.recordView->setFocusProxy(nullptr);
	SC_D.searchStation->releaseKeyboard();

	SC_D.searchStation->setVisible(false);
	SC_D.searchLabel->setVisible(false);

	SC_D.ui.actionCreatePick->setEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::emitPick(const Processing::Picker *picker,
                          const Processing::Picker::Result &res) {
	PickerTimeWindowDecorator *d;
	d = static_cast<PickerTimeWindowDecorator*>(SC_D.currentRecord->decorator());
	if ( !d ) {
		d = new PickerTimeWindowDecorator(this);
		SC_D.currentRecord->setDecorator(d);
	}

	// First set the cursor position otherwise the decorator would be
	// hidden immediately.
	setCursorPos(res.time);

	// Set up the decorator. It will be hidden on cursor move.
	d->setTimeWindowAndSNR(picker->signalWindow(), res.snr);
	d->setVisible(true);

	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateTheoreticalArrivals() {
	if ( SC_D.origin == nullptr ) return;

	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordViewItem* item = SC_D.recordView->itemAt(i);
		RecordWidget* widget = item->widget();

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			PickerMarker* marker = (PickerMarker*)widget->marker(m);
			if ( !marker->pick() ) {
				delete marker;
				--m;
			}
		}
	}

	fillTheoreticalArrivals();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::createPick() {
	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(SC_D.currentRecord, SC_D.currentRecord->cursorPos());

		SC_D.recordView->selectNextRow();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPick() {


	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(SC_D.currentRecord, SC_D.currentRecord->cursorPos());
	}
	else
		// Only show spectrum if picking is not enabled
		showSpectrum();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::confirmPick() {
	RecordViewItem *item = SC_D.recordView->currentItem();
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

		if ( item ) {
			Core::Time t = item->widget()->cursorPos();
			if ( (t < item->widget()->leftTime()) ||
			     (t > item->widget()->rightTime()) ) {
				double tmin = SC_D.recordView->timeRangeMin();
				double tmax = SC_D.recordView->timeRangeMax();

				double pos = t - SC_D.recordView->alignment();

				double offset = pos - (tmin+tmax)/2;
				SC_D.recordView->move(offset);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::resetPick() {
	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( !item ) return;

	PickerMarker *m = static_cast<PickerMarker*>(item->widget()->marker(item->widget()->cursorText(), true));
	if ( !m ) return;

	if ( m->isMoveCopyEnabled() )
		m->reset();
	else
		delete m;

	item->widget()->update();
	SC_D.currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::deletePick() {
	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( item ) {
		if ( item->widget()->currentMarker() ) {
			PickerMarker *m = static_cast<PickerMarker*>(item->widget()->currentMarker());
			if ( m->isArrival() ) {
				if ( m->isEnabled() ) {
					RecordMarker *old = item->widget()->marker(m->text());
					if ( old ) old->setEnabled(true);
				}

				if ( m->isMovable() || !SC_D.loadedPicks )
					delete m;
				else
					m->setType(PickerMarker::Pick);

				item->widget()->update();
				SC_D.currentRecord->update();
			}
		}
		else if ( !item->widget()->cursorText().isEmpty() )
			resetPick();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addFilter(const QString& name, const QString& filter) {
	if ( SC_D.comboFilter ) {
		if ( SC_D.comboFilter->findText(name) != -1 )
			return;

		SC_D.comboFilter->addItem(name, filter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::activateFilter(int index) {
	if ( !SC_D.comboFilter ) return;

	if ( index < 0 ) {
		SC_D.comboFilter->setCurrentIndex(0);
		return;
	}

	++index;

	if ( SC_D.comboFilter->count() > index ) {
		SC_D.comboFilter->setCurrentIndex(index);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specLogToggled(bool e) {
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setLogSpectrogram(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specSmoothToggled(bool e) {
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setSmoothSpectrogram(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specMinValue(double v) {
	SC_D.specOpts.minRange = v;
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setMinSpectrogramRange(SC_D.specOpts.minRange);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specMaxValue(double v) {
	SC_D.specOpts.maxRange = v;
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setMaxSpectrogramRange(SC_D.specOpts.maxRange);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specTimeWindow(double tw) {
	SC_D.specOpts.tw = tw;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specApply() {
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setSpectrogramTimeWindow(SC_D.specOpts.tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::limitFilterToZoomTrace(bool e) {
	changeFilter(SC_D.comboFilter->currentIndex(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showTheoreticalArrivals(bool v) {
	for ( int i = 0; i < SC_D.currentRecord->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(SC_D.currentRecord->marker(i));
		if ( m->type() == PickerMarker::Theoretical )
			m->setVisible(v);
	}

	// Since all markers are just proxies of the real traces we need
	// to update the zoom trace explicitly.
	SC_D.currentRecord->update();

	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordWidget *w = SC_D.recordView->itemAt(i)->widget();

		for ( int i = 0; i < w->markerCount(); ++i ) {
			PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
			if ( m->type() == PickerMarker::Theoretical )
				m->setVisible(v);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showUnassociatedPicks(bool v) {
	if ( v && !SC_D.loadedPicks ) {
		loadPicks();
		fillRawPicks();
	}

	for ( int i = 0; i < SC_D.currentRecord->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(SC_D.currentRecord->marker(i));
		if ( m->type() == PickerMarker::Pick )
			m->setVisible(v);
	}

	// Since all markers are just proxies of the real traces we need
	// to update the zoom trace explicitly.
	SC_D.currentRecord->update();

	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordWidget *w = SC_D.recordView->itemAt(i)->widget();

		for ( int i = 0; i < w->markerCount(); ++i ) {
			PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
			if ( m->type() == PickerMarker::Pick )
				m->setVisible(v);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showSpectrogram(bool v) {
	static_cast<ZoomRecordWidget*>(SC_D.currentRecord)->setShowSpectrogram(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showSpectrum() {
	RecordViewItem *item = SC_D.recordView->currentItem();
	if ( !item ) return;

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());

	if ( (SC_D.currentSlot < 0) || (SC_D.currentSlot > 2) ) {
		statusBar()->showMessage(tr("Error: invalid component selected"));
		return;
	}

	RecordSequence *seq = label->data.traces[SC_D.currentSlot].filter ? label->data.traces[SC_D.currentSlot].transformed : label->data.traces[SC_D.currentSlot].raw;
	if ( !seq ) {
		statusBar()->showMessage(tr("Error: cannot show spectrum, no data for current slot"));
		return;
	}

	Core::TimeWindow tw = SC_D.currentRecord->visibleTimeWindow();
	GenericRecordPtr trace = seq->continuousRecord<double>(&tw);
	if ( !trace ) {
		statusBar()->showMessage(tr("Error: failed to extract trace for spectrum"));
		return;
	}

	Processing::Stream tmp;
	tmp.init(trace->networkCode(), trace->stationCode(), trace->locationCode(), trace->channelCode(), trace->startTime());

	// Correct for gain if given
	if ( tmp.gain > 0.0 )
		tmp.applyGain(*static_cast<DoubleArray*>(trace->data()));

	// Remove mean
	*static_cast<DoubleArray*>(trace->data()) -= static_cast<DoubleArray*>(trace->data())->mean();

	if ( SC_D.spectrumView != nullptr ) {
		SC_D.spectrumView->setWindowTitle(tr("Spectrum of %1").arg(trace->streamID().c_str()));
		static_cast<SpectrumView*>(SC_D.spectrumView)->setData(trace.get(), tmp.sensor());
		return;
	}

	SpectrumView *spectrumView = new SpectrumView(this, Qt::Tool);
	SC_D.spectrumView = spectrumView;

	spectrumView->setAttribute(Qt::WA_DeleteOnClose);
	spectrumView->setWindowTitle(tr("Spectrum of %1").arg(trace->streamID().c_str()));

	connect(spectrumView, SIGNAL(destroyed(QObject*)), this, SLOT(destroyedSpectrumWidget(QObject*)));

	spectrumView->setData(trace.get(), tmp.sensor());

	if ( SC_D.spectrumWidgetGeometry.isEmpty() )
		spectrumView->resize(SC_D.defaultSpectrumWidgetSize);
	else
		spectrumView->restoreGeometry(SC_D.spectrumWidgetGeometry);
	spectrumView->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeFilter(int index) {
	changeFilter(index, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeRotation(int index) {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	SC_D.currentRotationMode = index;

	for ( int i = 0; i < SC_D.recordView->rowCount(); ++i ) {
		RecordViewItem* rvi = SC_D.recordView->itemAt(i);
		applyRotation(rvi, index);
		updateTraceInfo(rvi, nullptr);
	}

	// Change icons depending on the current rotation mode
	if ( index == RT_ZRT ) {
		SC_D.ui.actionShowNComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelR.png")));
		SC_D.ui.actionShowNComponent->setText(QString::fromUtf8("Radial"));
		SC_D.ui.actionShowNComponent->setToolTip(QString::fromUtf8("Show Radial Component (N)"));
		SC_D.ui.actionShowEComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelT.png")));
		SC_D.ui.actionShowEComponent->setText(QString::fromUtf8("Transversal"));
		SC_D.ui.actionShowEComponent->setToolTip(QString::fromUtf8("Show Transversal Component (E)"));
	}
	else {
		SC_D.ui.actionShowNComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelN.png")));
		SC_D.ui.actionShowNComponent->setText(QString::fromUtf8("North"));
		SC_D.ui.actionShowNComponent->setToolTip(QString::fromUtf8("Show North Component (N)"));
		SC_D.ui.actionShowEComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelE.png")));
		SC_D.ui.actionShowEComponent->setText(QString::fromUtf8("East"));
		SC_D.ui.actionShowEComponent->setToolTip(QString::fromUtf8("Show East Component (E)"));
	}


	if ( index == RT_ZNE || index == RT_ZRT || index == RT_ZH ) {
		bool tmp = SC_D.config.loadAllComponents;
		SC_D.config.loadAllComponents = true;

		// Fetch all components if not done already
		fetchComponent('?');

		SC_D.config.loadAllComponents = tmp;
	}

	if ( SC_D.recordView->currentItem() ) {
		updateItemLabel(SC_D.recordView->currentItem(), SC_D.recordView->currentItem()->currentComponent());

		for ( int i = 0; i < SC_D.currentRecord->slotCount(); ++i ) {
			char code = SC_D.recordView->currentItem()->mapSlotToComponent(i);
			if ( code == '?' ) continue;

			switch ( index ) {
				case RT_123:
					SC_D.currentRecord->setRecordID(i, QString("%1").arg(code));
					break;
				case RT_ZNE:
					SC_D.currentRecord->setRecordID(i, QString("%1").arg(ZNE_COMPS[i]));
					break;
				case RT_ZRT:
					SC_D.currentRecord->setRecordID(i, QString("%1").arg(ZRT_COMPS[i]));
					break;
				case RT_ZH:
					SC_D.currentRecord->setRecordID(i, QString("%1").arg(ZH_COMPS[i]));
					break;
			}
		}

		SC_D.currentRecord->update();
	}

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeUnit(int index) {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	SC_D.currentUnitMode = index;
	applyFilter();

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateRecordAxisLabel(RecordViewItem *item) {
	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());

	switch ( SC_D.currentUnitMode ) {
		case UT_DISP:
		case UT_VEL:
		case UT_ACC:
			if ( label->unit != UT_RAW ) {
				if ( item->widget()->areScaledValuesShown() ) {
					for ( int i = 0; i < 3; ++i )
						item->widget()->setRecordLabel(i, tr("n%1").arg(Units[SC_D.currentUnitMode-UT_ACC]));
				}
				else {
					for ( int i = 0; i < 3; ++i )
						item->widget()->setRecordLabel(i, tr("counts"));
				}
			}
			else {
				for ( int i = 0; i < 3; ++i )
					item->widget()->setRecordLabel(i, QString());
			}
			break;
		default:
			if ( item->widget()->areScaledValuesShown() ) {
				for ( int i = 0; i < 3; ++i ) {
					if ( label->gainUnit[i].isEmpty() )
						item->widget()->setRecordLabel(i, tr("-"));
					else
						item->widget()->setRecordLabel(i, tr("%1 * 1E9").arg(label->gainUnit[i]));
				}
			}
			else {
				for ( int i = 0; i < 3; ++i )
					item->widget()->setRecordLabel(i, tr("counts"));
			}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::applyFilter(RecordViewItem *item) {
	if ( item == nullptr ) {
		for ( int i = 0; i < SC_D.recordView->rowCount(); ++i )
			applyFilter(SC_D.recordView->itemAt(i));
	}
	else {
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
		int integrationSteps = 0;
		switch ( SC_D.currentUnitMode ) {
			case UT_DISP:
			case UT_VEL:
			case UT_ACC:
				if ( label->unit != UT_RAW ) {
					integrationSteps = SC_D.currentUnitMode - label->unit;
				}
				else {
					Math::Filtering::ConstFilter<double> constFilter(0);
					label->data.setFilter(&constFilter);
					return true;
				}
				break;
			case UT_RAW:
			default:
				break;
		}

		updateRecordAxisLabel(item);

		if ( integrationSteps == 0 )
			label->data.setFilter(SC_D.currentFilter);
		else {
			Math::Filtering::ChainFilter<double> chainFilter;

			if ( integrationSteps < 0  ) {
				// Derivation
				for ( int s = 0; s < -integrationSteps; ++s )
					chainFilter.add(new Math::Filtering::IIRDifferentiate<double>());
			}
			else {
				RecordWidget::Filter *preFilter = nullptr;

				for ( int s = 0; s < integrationSteps; ++s ) {
					if ( !SC_D.config.onlyApplyIntegrationFilterOnce || (s == 0) ) {
						if ( preFilter != nullptr )
							chainFilter.add(preFilter->clone());
						else if ( !SC_D.config.integrationFilter.isEmpty() ) {
							preFilter = Math::Filtering::InPlaceFilter<double>::Create(SC_D.config.integrationFilter.toStdString().c_str());
							if ( preFilter == nullptr ) {
								// ERROR
							}
							else
								chainFilter.add(preFilter->clone());
						}
					}

					chainFilter.add(new Math::Filtering::IIRIntegrate<double>());
				}

				if ( preFilter != nullptr )
					delete preFilter;
			}

			if ( SC_D.currentFilter )
				chainFilter.add(SC_D.currentFilter->clone());

			if ( chainFilter.filterCount() > 0 )
				label->data.setFilter(&chainFilter);
			else
				label->data.setFilter(nullptr);
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::applyRotation(RecordViewItem *item, int type) {
	switch ( type ) {
		case RT_123:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation.identity();
			label->data.setL2Horizontals(false);
			label->data.setTransformationEnabled(false);
			break;
		}
		case RT_ZNE:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation = label->orientationZNE;
			label->data.setL2Horizontals(false);
			label->data.setTransformationEnabled(true);
			break;
		}
		case RT_ZRT:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation.mult(label->orientationZRT, label->orientationZNE);
			label->data.setL2Horizontals(false);
			label->data.setTransformationEnabled(true);
			break;
		}
		case RT_ZH:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation.identity();
			label->data.setL2Horizontals(true);
			label->data.setTransformationEnabled(true);
			break;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeFilter(int index, bool) {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QString name = SC_D.comboFilter->itemText(index);
	QString filter = SC_D.comboFilter->itemData(index).toString();

	if ( name == NO_FILTER_STRING ) {
		if ( SC_D.currentFilter ) delete SC_D.currentFilter;
		SC_D.currentFilter = nullptr;
		SC_D.currentFilterID = QString();

		if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
			applyFilter();
		else
			applyFilter(SC_D.recordView->currentItem());

		QApplication::restoreOverrideCursor();
		return;
	}

	RecordWidget::Filter *newFilter = RecordWidget::Filter::Create(filter.toStdString());

	if ( !newFilter ) {
		QApplication::setOverrideCursor(Qt::ArrowCursor);
		QMessageBox::critical(this, "Invalid filter",
		                      QString("Unable to create filter: %1\nFilter: %2").arg(name).arg(filter));
		QApplication::restoreOverrideCursor();

		SC_D.comboFilter->blockSignals(true);
		SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
		SC_D.comboFilter->blockSignals(false);
	}
	else
		SC_D.currentFilterID = filter;

	if ( SC_D.currentFilter ) delete SC_D.currentFilter;
	SC_D.currentFilter = newFilter;
	if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter();
	else
		applyFilter(SC_D.recordView->currentItem());

	SC_D.lastFilterIndex = index;
	QApplication::restoreOverrideCursor();

	/*
	if ( index == SC_D.lastFilterIndex && !force ) {
		if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
			SC_D.recordView->enableFilter(SC_D.lastFilterIndex > 0);
		else
			SC_D.currentRecord->enableFiltering(SC_D.lastFilterIndex > 0);
		return;
	}

	QString name = SC_D.comboFilter->itemText(index);
	QString filter = SC_D.comboFilter->itemData(index).toString();

	if ( name == NOSC_D.FILTERSC_D.STRING ) {
		if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
			SC_D.recordView->enableFilter(false);
		else
			SC_D.currentRecord->enableFiltering(false);
		return;
	}


	bool filterApplied = false;
	// Here one should
	if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() ) {
		if ( SC_D.recordView->setFilterByName(filter) ) {
			SC_D.currentRecord->setFilter(SC_D.recordView->filter());
			SC_D.lastFilterIndex = index;
			filterApplied = true;
		}
	}
	else {
		RecordWidget::Filter *f =
			Util::createFilterByName<float>(filter.toStdString());

		if ( f != nullptr ) {
			SC_D.currentRecord->setFilter(f);
			SC_D.lastFilterIndex = index;
			filterApplied = true;
			delete f;
		}
	}


	if ( !filterApplied ) {
		QMessageBox::critical(this, "Invalid filter",
		                      QString("Unable to create filter: %1\nFilter: %2").arg(name).arg(filter));

		SC_D.comboFilter->blockSignals(true);
		SC_D.comboFilter->setCurrentIndex(SC_D.lastFilterIndex);
		SC_D.comboFilter->blockSignals(false);
	}

	//std::cout << "Current filter index: " << SC_D.lastFilterIndex << std::endl;
	if ( !SC_D.ui.actionLimitFilterToZoomTrace->isChecked() )
		SC_D.recordView->enableFilter(SC_D.lastFilterIndex > 0);
	else
		SC_D.currentRecord->enableFiltering(SC_D.lastFilterIndex > 0);
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setArrivalState(int arrivalId, bool state) {
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
bool PickerView::setArrivalState(RecordWidget* w, int arrivalId, bool state) {
	if ( !w->isEnabled() ) return false;

	// Find arrival and update state
	for ( int m = 0; m < w->markerCount(); ++m ) {
		PickerMarker *marker = (PickerMarker*)w->marker(m);
		if ( marker->id() == arrivalId && marker->isArrival() ) {
			marker->setEnabled(state);
			w->update();
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
