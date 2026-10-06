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


#include <seiscomp/gui/datamodel/spectraldiagnosticsview.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/core/scheme.h>
#include <seiscomp/gui/plot/axis.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <cmath>
#include <fstream>
#include <functional>


using namespace Seiscomp::Processing;


namespace Seiscomp {
namespace Gui {


namespace {


QColor componentColor(char comp) {
	switch ( comp ) {
		case 'N': case '1': case 'R': return QColor(214, 39, 40);
		case 'E': case '2': case 'T': return QColor(44, 160, 44);
		default: return QColor(31, 119, 180);
	}
}


QString formatValue(double v) {
	if ( v != 0 && (std::fabs(v) < 1E-2 || std::fabs(v) >= 1E5) ) {
		return QString::number(v, 'e', 3);
	}
	return QString::number(v, 'g', 4);
}


QString roleName(SpectralCurve::Role role) {
	switch ( role ) {
		case SpectralCurve::Signal: return "signal";
		case SpectralCurve::Noise: return "noise";
		case SpectralCurve::Model: return "model";
		case SpectralCurve::Correction: return "correction";
		case SpectralCurve::Response: return "response";
		default: return "other";
	}
}


bool isPlottable(double f, double v) {
	return f > 0 && v > 0 && std::isfinite(v);
}


//! log10 of the curve value at f, interpolated in log-log, or NaN
double logValueAt(const SpectralCurve &c, double f) {
	size_t n = std::min(c.freq.size(), c.value.size());
	for ( size_t i = 1; i < n; ++i ) {
		if ( c.freq[i] < f ) continue;
		if ( c.freq[i-1] > f ) break;
		if ( !isPlottable(c.freq[i-1], c.value[i-1]) || !isPlottable(c.freq[i], c.value[i]) ) {
			break;
		}
		double t = (log10(f) - log10(c.freq[i-1])) / (log10(c.freq[i]) - log10(c.freq[i-1]));
		return log10(c.value[i-1]) + t * (log10(c.value[i]) - log10(c.value[i-1]));
	}
	return std::nan("");
}


}


/**
 * Log-log plot of the curves. Corrections and responses are usually
 * dimensionless or in other units than the spectra and go on the right axis.
 * The first band can be edited by dragging its edges, or a new one by
 * dragging across the plot.
 */
class SpectralDiagnosticsPlot : public QWidget {
	public:
		SpectralDiagnosticsPlot(QWidget *parent = nullptr)
		: QWidget(parent) {
			setBackgroundRole(QPalette::Base);
			setAutoFillBackground(true);
			setMinimumSize(320, 240);
			setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			setMouseTracking(true);

			QColor axisColor = palette().color(QPalette::Text);
			QColor base = palette().color(QPalette::Base);
			for ( Axis *axis : { &_xAxis, &_yAxis, &_yAxis2 } ) {
				axis->setPen(axisColor);
				axis->setGridPen(blend(base, axisColor, 192 * 100 / 256));
				axis->setSubGridPen(blend(base, axisColor, 224 * 100 / 256));
				axis->setLogScale(true);
			}

			_xAxis.setPosition(Axis::Bottom);
			_yAxis.setPosition(Axis::Left);
			_yAxis2.setPosition(Axis::Right);
			_xAxis.setLabel(tr("Frequency in Hz"));
			_yAxis2.setGrid(false);
		}

	public:
		void setDiagnostics(const SpectralDiagnostics *diag) {
			_diag = diag;
			_dragEdge = NoEdge;
			update();
		}

		void setOverlay(const QVector<SpectralDiagnosticsView::OverlayCurve> *overlay) {
			_overlay = overlay;
			_hoverOverlay = -1;
			update();
		}

		void setBandEditable(bool editable) {
			_bandEditable = editable;
			_dragEdge = NoEdge;
			unsetCursor();
			update();
		}

		//! Called with a description of the value under the mouse cursor
		std::function<void (const QString &)> cursorChanged;
		//! Called when the user dragged a band
		std::function<void (double, double)> bandEdited;

		void setShowNoise(bool f) { _showNoise = f; update(); }
		void setShowCorrections(bool f) { _showCorrections = f; update(); }
		void setShowOther(bool f) { _showOther = f; update(); }

	protected:
		void paintEvent(QPaintEvent *) override {
			QPainter p(this);
			_plotRect = QRect();

			if ( !_diag || _diag->curves.empty() ) {
				p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
				p.drawText(rect(), Qt::AlignCenter, tr("No spectra available"));
				return;
			}

			Range xRange, yRange, y2Range;
			std::string yUnit, y2Unit;
			bool hasY2 = false;

			for ( const auto &c : _diag->curves ) {
				if ( !isVisible(c) ) {
					continue;
				}
				bool right = onRightAxis(c);
				Range &yr = right ? y2Range : yRange;
				if ( right ) {
					hasY2 = true;
					if ( y2Unit.empty() ) y2Unit = c.unit;
				}
				else if ( yUnit.empty() ) {
					yUnit = c.unit;
				}

				for ( size_t i = 0; i < c.freq.size() && i < c.value.size(); ++i ) {
					if ( !isPlottable(c.freq[i], c.value[i]) ) continue;
					extend(xRange, c.freq[i]);
					extend(yr, c.value[i]);
				}
			}

			if ( _overlay ) {
				for ( const auto &o : *_overlay ) {
					for ( size_t i = 0; i < o.curve.freq.size() && i < o.curve.value.size(); ++i ) {
						if ( !isPlottable(o.curve.freq[i], o.curve.value[i]) ) continue;
						extend(xRange, o.curve.freq[i]);
						extend(yRange, o.curve.value[i]);
					}
				}
			}

			if ( !xRange.isValid() || !yRange.isValid() ) {
				p.drawText(rect(), Qt::AlignCenter, tr("No positive values to plot"));
				return;
			}

			// Some headroom for the legend and to not stick to the frame
			yRange.lower /= 2;
			yRange.upper *= 4;
			_xAxis.setRange(xRange);
			_yAxis.setRange(yRange);
			_yAxis.setLabel(yUnit.empty() ? tr("Amplitude") : QString(yUnit.c_str()));
			_yAxis2.setVisible(hasY2);
			if ( hasY2 ) {
				y2Range.lower /= 2;
				y2Range.upper *= 2;
				_yAxis2.setRange(y2Range);
				_yAxis2.setLabel(y2Unit.empty() ? tr("Correction") : QString(y2Unit.c_str()));
			}

			const int margin = 9;
			int axisHeight = _xAxis.sizeHint(p);

			QRect yAxisRect(margin, margin, 0, height() - axisHeight - margin * 2);
			_yAxis.updateLayout(p, yAxisRect);

			QRect yAxis2Rect(width() - 1 - margin, margin, 0, height() - axisHeight - margin * 2);
			if ( hasY2 ) {
				_yAxis2.updateLayout(p, yAxis2Rect);
			}

			QRect xAxisRect(yAxisRect.right(), height() - 1 - margin,
			                yAxis2Rect.left() - yAxisRect.right() + 1, 0);
			_xAxis.updateLayout(p, xAxisRect);

			QRect plotRect(xAxisRect.left(), yAxisRect.top(),
			               xAxisRect.width(), yAxisRect.height());

			// Keep the mapping for the mouse interaction
			_plotRect = plotRect;
			_xRange = xRange;
			_yRange = yRange;
			_xPix = Range(_xAxis.unproject(xRange.lower), _xAxis.unproject(xRange.upper));
			_yPix = Range(_yAxis.unproject(yRange.lower), _yAxis.unproject(yRange.upper));
			_yUnit = yUnit.c_str();

			_xAxis.drawGrid(p, plotRect, true, false);
			_yAxis.drawGrid(p, plotRect, true, false);
			_xAxis.drawGrid(p, plotRect, false, true);
			_yAxis.drawGrid(p, plotRect, false, true);

			_xAxis.draw(p, xAxisRect);
			_yAxis.draw(p, yAxisRect);
			if ( hasY2 ) {
				_yAxis2.draw(p, yAxis2Rect);
			}

			p.save();
			p.setClipRect(plotRect);
			p.translate(plotRect.left(), plotRect.bottom());

			// Bands
			QColor bandColor = palette().color(QPalette::Highlight);
			bandColor.setAlpha(28);
			QColor edgeColor = palette().color(QPalette::Highlight);
			for ( const auto &band : currentBands() ) {
				if ( band.first <= 0 || band.second <= band.first ) continue;
				double x0 = _xAxis.unproject(band.first);
				double x1 = _xAxis.unproject(band.second);
				p.fillRect(QRectF(x0, -plotRect.height(), x1 - x0, plotRect.height()), bandColor);
				if ( _bandEditable ) {
					p.setPen(QPen(edgeColor, 2));
					p.drawLine(QPointF(x0, 0), QPointF(x0, -plotRect.height()));
					p.drawLine(QPointF(x1, 0), QPointF(x1, -plotRect.height()));
				}
			}

			p.setRenderHint(QPainter::Antialiasing, true);

			// Other stations
			if ( _overlay ) {
				QColor c = palette().color(QPalette::Text);
				for ( int i = 0; i < _overlay->size(); ++i ) {
					c.setAlpha(i == _hoverOverlay ? 200 : 45);
					p.setPen(QPen(c, i == _hoverOverlay ? 2 : 1));
					p.drawPolyline(polygon((*_overlay)[i].curve, _yAxis));
				}
			}

			// Curves
			for ( const auto &c : _diag->curves ) {
				if ( !isVisible(c) ) continue;
				p.setPen(pen(c));
				p.drawPolyline(polygon(c, onRightAxis(c) ? _yAxis2 : _yAxis));
			}

			// Frequency markers
			p.setPen(QPen(palette().color(QPalette::Text), 1, Qt::DashDotLine));
			for ( const auto &param : _diag->parameters ) {
				if ( !param.frequencyMarker || param.value <= 0 ) continue;
				double x = _xAxis.unproject(param.value);
				p.drawLine(QPointF(x, 0), QPointF(x, -plotRect.height()));
				p.drawText(QPointF(x + 3, -plotRect.height() + p.fontMetrics().height()),
				           QString("%1=%2").arg(param.id.c_str()).arg(formatValue(param.value)));
			}

			// Cursor line
			if ( _mouseX >= plotRect.left() && _mouseX <= plotRect.right() ) {
				QColor c = palette().color(QPalette::Text);
				c.setAlpha(80);
				p.setPen(QPen(c, 1));
				double x = _mouseX - plotRect.left();
				p.drawLine(QPointF(x, 0), QPointF(x, -plotRect.height()));
			}
			p.restore();

			drawLegend(p, plotRect);
		}

		void mousePressEvent(QMouseEvent *e) override {
			if ( !_bandEditable || e->button() != Qt::LeftButton
			  || !_plotRect.contains(e->pos()) ) {
				return;
			}

			_dragEdge = edgeAt(e->pos().x());
			auto bands = currentBands();
			if ( _dragEdge == NoEdge ) {
				// Start a new band
				double f = frequencyAt(e->pos().x());
				_dragBand = { f, f };
				_dragEdge = NewBand;
				_dragAnchor = f;
			}
			else {
				_dragBand = bands.front();
			}
		}

		void mouseMoveEvent(QMouseEvent *e) override {
			if ( !_plotRect.isValid() || _xPix.length() == 0 || _yPix.length() == 0 ) {
				return;
			}

			if ( _dragEdge != NoEdge ) {
				double f = frequencyAt(qBound(_plotRect.left(), e->pos().x(), _plotRect.right()));
				switch ( _dragEdge ) {
					case LowerEdge: _dragBand.first = f; break;
					case UpperEdge: _dragBand.second = f; break;
					case NewBand: _dragBand = { std::min(f, _dragAnchor), std::max(f, _dragAnchor) }; break;
					default: break;
				}
			}
			else if ( _bandEditable ) {
				if ( edgeAt(e->pos().x()) != NoEdge && _plotRect.contains(e->pos()) ) {
					setCursor(Qt::SizeHorCursor);
				}
				else {
					unsetCursor();
				}
			}

			if ( !_plotRect.contains(e->pos()) ) {
				clearCursor();
				return;
			}

			double f = frequencyAt(e->pos().x());
			double v = fromPixel(_plotRect.bottom() - e->pos().y(), _yPix, _yRange);
			_mouseX = e->pos().x();

			QString text = QString("f = %1 Hz (T = %2 s), %3 %4")
			               .arg(formatValue(f), formatValue(1.0 / f), formatValue(v), _yUnit);

			// The other station closest to the cursor
			int hover = -1;
			if ( _overlay ) {
				double best = 0.15; // max. distance in decades
				for ( int i = 0; i < _overlay->size(); ++i ) {
					double lv = logValueAt((*_overlay)[i].curve, f);
					if ( std::isnan(lv) ) continue;
					double dist = std::fabs(lv - log10(v));
					if ( dist < best ) {
						best = dist;
						hover = i;
					}
				}
			}

			if ( hover >= 0 ) {
				text += QString("  —  %1").arg((*_overlay)[hover].station);
			}

			_hoverOverlay = hover;
			if ( _dragEdge != NoEdge ) {
				text = QString("band %1 – %2 Hz").arg(formatValue(_dragBand.first),
				                                      formatValue(_dragBand.second));
			}

			update();
			if ( cursorChanged ) {
				cursorChanged(text);
			}
		}

		void mouseReleaseEvent(QMouseEvent *e) override {
			if ( _dragEdge == NoEdge || e->button() != Qt::LeftButton ) {
				return;
			}

			_dragEdge = NoEdge;
			auto band = std::make_pair(std::min(_dragBand.first, _dragBand.second),
			                           std::max(_dragBand.first, _dragBand.second));
			update();

			// Ignore clicks and bands narrower than ~5%
			if ( band.first > 0 && band.second > band.first * 1.05 && bandEdited ) {
				bandEdited(band.first, band.second);
			}
		}

		void leaveEvent(QEvent *) override {
			clearCursor();
		}

	private:
		enum Edge { NoEdge, LowerEdge, UpperEdge, NewBand };

		std::vector<std::pair<double,double>> currentBands() const {
			if ( _dragEdge != NoEdge ) {
				return { _dragBand };
			}
			return _diag ? _diag->bands : std::vector<std::pair<double,double>>();
		}

		Edge edgeAt(int x) const {
			if ( !_diag || _diag->bands.empty() ) return NoEdge;
			const auto &band = _diag->bands.front();
			double x0 = _plotRect.left() + _xAxis.unproject(band.first);
			double x1 = _plotRect.left() + _xAxis.unproject(band.second);
			if ( std::fabs(x - x0) <= 5 ) return LowerEdge;
			if ( std::fabs(x - x1) <= 5 ) return UpperEdge;
			return NoEdge;
		}

		double frequencyAt(int x) const {
			return fromPixel(x - _plotRect.left(), _xPix, _xRange);
		}

		//! Inverse of the log axis mapping between pixel and value ranges
		static double fromPixel(double pix, const Range &pixRange, const Range &valueRange) {
			double t = (pix - pixRange.lower) / (pixRange.upper - pixRange.lower);
			double l0 = log10(valueRange.lower), l1 = log10(valueRange.upper);
			return pow(10.0, l0 + t * (l1 - l0));
		}

		QPolygonF polygon(const SpectralCurve &c, const Axis &yAxis) const {
			QPolygonF poly;
			for ( size_t i = 0; i < c.freq.size() && i < c.value.size(); ++i ) {
				if ( !isPlottable(c.freq[i], c.value[i]) ) continue;
				poly.append(QPointF(_xAxis.unproject(c.freq[i]), -yAxis.unproject(c.value[i])));
			}
			return poly;
		}

		void clearCursor() {
			if ( _mouseX >= 0 || _hoverOverlay >= 0 ) {
				_mouseX = -1;
				_hoverOverlay = -1;
				update();
			}
			if ( cursorChanged ) {
				cursorChanged(QString());
			}
		}

		static void extend(Range &r, double v) {
			if ( !r.isValid() ) {
				r = Range(v, v);
			}
			else {
				r.extend(Range(v, v));
			}
		}

		static bool onRightAxis(const SpectralCurve &c) {
			return c.role == SpectralCurve::Correction
			    || c.role == SpectralCurve::Response;
		}

		bool isVisible(const SpectralCurve &c) const {
			switch ( c.role ) {
				case SpectralCurve::Noise: return _showNoise;
				case SpectralCurve::Correction:
				case SpectralCurve::Response: return _showCorrections;
				case SpectralCurve::Other: return _showOther;
				default: return true;
			}
		}

		QPen pen(const SpectralCurve &c) const {
			QColor col = componentColor(c.component);
			switch ( c.role ) {
				case SpectralCurve::Signal:
					return QPen(col, 2);
				case SpectralCurve::Noise:
					col.setAlpha(110);
					return QPen(col, 1);
				case SpectralCurve::Model:
					return QPen(palette().color(QPalette::Text), 1.5, Qt::DashLine);
				case SpectralCurve::Correction:
				case SpectralCurve::Response:
					return QPen(QColor(128, 128, 128), 1, Qt::DotLine);
				default:
					return QPen(col, 1, Qt::DotLine);
			}
		}

		void drawLegend(QPainter &p, const QRect &plotRect) {
			QFontMetrics fm = p.fontMetrics();
			int lineHeight = fm.height();
			int lineWidth = 24;
			int textWidth = 0;
			int rows = 0;
			bool overlay = _overlay && !_overlay->isEmpty();
			QString overlayText = overlay ? tr("%1 other stations").arg(_overlay->size()) : QString();

			for ( const auto &c : _diag->curves ) {
				if ( !isVisible(c) ) continue;
				textWidth = qMax(textWidth, QT_FM_WIDTH(fm, legendText(c)));
				++rows;
			}
			if ( overlay ) {
				textWidth = qMax(textWidth, QT_FM_WIDTH(fm, overlayText));
				++rows;
			}

			if ( !rows ) return;

			QRect r(plotRect.right() - textWidth - lineWidth - 18, plotRect.top() + 6,
			        textWidth + lineWidth + 12, rows * lineHeight + 6);
			QColor bg = palette().color(QPalette::Base);
			bg.setAlpha(220);
			p.fillRect(r, bg);

			int y = r.top() + 3;
			auto row = [&](const QPen &pen, const QString &text) {
				p.setPen(pen);
				p.drawLine(r.left() + 4, y + lineHeight / 2, r.left() + 4 + lineWidth, y + lineHeight / 2);
				p.setPen(palette().color(QPalette::Text));
				p.drawText(QRect(r.left() + 8 + lineWidth, y, textWidth, lineHeight),
				           Qt::AlignLeft | Qt::AlignVCenter, text);
				y += lineHeight;
			};

			for ( const auto &c : _diag->curves ) {
				if ( !isVisible(c) ) continue;
				row(pen(c), legendText(c));
			}
			if ( overlay ) {
				QColor c = palette().color(QPalette::Text);
				c.setAlpha(90);
				row(QPen(c, 1), overlayText);
			}
		}

		static QString legendText(const SpectralCurve &c) {
			return c.label.empty() ? roleName(c.role) : QString(c.label.c_str());
		}

	private:
		const SpectralDiagnostics                          *_diag{nullptr};
		const QVector<SpectralDiagnosticsView::OverlayCurve> *_overlay{nullptr};
		Axis                       _xAxis;
		Axis                       _yAxis;
		Axis                       _yAxis2;
		bool                       _showNoise{true};
		bool                       _showCorrections{false};
		bool                       _showOther{true};
		bool                       _bandEditable{false};

		QRect                      _plotRect;
		Range                      _xRange, _yRange;
		Range                      _xPix, _yPix;
		QString                    _yUnit;
		int                        _mouseX{-1};
		int                        _hoverOverlay{-1};

		Edge                       _dragEdge{NoEdge};
		std::pair<double,double>   _dragBand;
		double                     _dragAnchor{0};
};


SpectralDiagnosticsView::SpectralDiagnosticsView(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f) {
	_header = new QLabel;
	_header->setTextFormat(Qt::RichText);
	_header->setWordWrap(true);

	_btnPrevious = new QToolButton;
	_btnPrevious->setArrowType(Qt::LeftArrow);
	_btnPrevious->setToolTip(tr("Previous station (Left, Up, Page Up)"));
	_btnNext = new QToolButton;
	_btnNext->setArrowType(Qt::RightArrow);
	_btnNext->setToolTip(tr("Next station (Right, Down, Page Down)"));
	_position = new QLabel;
	_position->setMinimumWidth(QT_FM_WIDTH(_position->fontMetrics(), "000 / 000"));
	_position->setAlignment(Qt::AlignCenter);
	connect(_btnPrevious, &QToolButton::clicked, this, &SpectralDiagnosticsView::previousRequested);
	connect(_btnNext, &QToolButton::clicked, this, &SpectralDiagnosticsView::nextRequested);

	_order = new QComboBox;
	_order->addItem(tr("List order"), ListOrder);
	_order->addItem(tr("Deviation from median"), MagnitudeDeviation);
	_order->addItem(tr("Worst residual first"), FitResidual);
	_order->addItem(tr("Lowest SNR first"), LowestSNR);
	_order->setToolTip(tr("The order in which previous/next step through the stations"));
	connect(_order, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &SpectralDiagnosticsView::orderChanged);

	_useStation = new QCheckBox(tr("Use station"));
	_useStation->setToolTip(tr("Use the amplitude of this station for the magnitude (U)"));
	_useStation->setEnabled(false);
	connect(_useStation, &QCheckBox::clicked, this, &SpectralDiagnosticsView::stationUseChanged);

	for ( const char *key : { "Left", "Up", "PgUp" } ) {
		auto *sc = new QShortcut(QKeySequence(key), this);
		connect(sc, &QShortcut::activated, this, &SpectralDiagnosticsView::previousRequested);
	}
	for ( const char *key : { "Right", "Down", "PgDown" } ) {
		auto *sc = new QShortcut(QKeySequence(key), this);
		connect(sc, &QShortcut::activated, this, &SpectralDiagnosticsView::nextRequested);
	}
	auto *useShortcut = new QShortcut(QKeySequence("U"), this);
	connect(useShortcut, &QShortcut::activated, this, [this]() {
		if ( _useStation->isEnabled() ) {
			_useStation->click();
		}
	});
	auto *closeShortcut = new QShortcut(QKeySequence("Esc"), this);
	connect(closeShortcut, &QShortcut::activated, this, &QWidget::close);

	auto *top = new QHBoxLayout;
	top->addWidget(_btnPrevious);
	top->addWidget(_position);
	top->addWidget(_btnNext);
	top->addWidget(_order);
	top->addSpacing(8);
	top->addWidget(_header, 1);
	top->addWidget(_useStation);

	_plot = new SpectralDiagnosticsPlot;
	_cursorInfo = new QLabel;
	_cursorInfo->setMinimumWidth(QT_FM_WIDTH(_cursorInfo->fontMetrics(),
	                                         "f = 0.0000 Hz (T = 0.000 s), 0.000e+00 nm*s  —  XX.XXXXX"));
	_plot->cursorChanged = [this](const QString &text) { _cursorInfo->setText(text); };
	_plot->bandEdited = [this](double fmin, double fmax) { emit spectralBandChanged(fmin, fmax); };

	_table = new QTableWidget(0, 3);
	_table->setHorizontalHeaderLabels({ tr("Parameter"), tr("Value"), tr("Unit") });
	_table->verticalHeader()->setVisible(false);
	_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setStretchLastSection(true);
	_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_table->setSelectionMode(QAbstractItemView::NoSelection);

	auto *splitter = new QSplitter(Qt::Horizontal);
	splitter->addWidget(_plot);
	splitter->addWidget(_table);
	splitter->setStretchFactor(0, 4);
	splitter->setStretchFactor(1, 1);
	_table->setMinimumWidth(240);
	splitter->setSizes({ 660, 260 });

	_showNoise = new QCheckBox(tr("Noise"));
	_showNoise->setChecked(true);
	_showCorrections = new QCheckBox(tr("Corrections"));
	_showCorrections->setChecked(false);
	_showOther = new QCheckBox(tr("Other curves"));
	_showOther->setChecked(true);
	_showOverlay = new QCheckBox(tr("Overlay stations"));
	_showOverlay->setToolTip(tr("Show the signal spectra of all other stations, "
	                            "scaled to the hypocentral distance of this station"));
	_showOverlay->setChecked(false);
	connect(_showNoise, &QCheckBox::toggled, this, [this](bool f) { _plot->setShowNoise(f); });
	connect(_showCorrections, &QCheckBox::toggled, this, [this](bool f) { _plot->setShowCorrections(f); });
	connect(_showOther, &QCheckBox::toggled, this, [this](bool f) { _plot->setShowOther(f); });
	connect(_showOverlay, &QCheckBox::toggled, this, [this](bool f) {
		if ( !f ) setOverlay({});
		emit overlayToggled(f);
	});

	_btnResetBand = new QPushButton(tr("Auto band"));
	_btnResetBand->setToolTip(tr("Drag the band edges in the plot (or across the plot) to refit "
	                             "in another band; this restores the automatic band"));
	_btnResetBand->setVisible(false);
	connect(_btnResetBand, &QPushButton::clicked, this, &SpectralDiagnosticsView::spectralBandReset);

	auto *exportButton = new QPushButton(tr("Export"));
	connect(exportButton, &QPushButton::clicked, this, &SpectralDiagnosticsView::exportCurves);
	auto *closeButton = new QPushButton(tr("Close"));
	connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

	auto *buttons = new QHBoxLayout;
	buttons->addWidget(_cursorInfo, 1);
	buttons->addSpacing(12);
	buttons->addWidget(_showNoise);
	buttons->addWidget(_showCorrections);
	buttons->addWidget(_showOther);
	buttons->addWidget(_showOverlay);
	buttons->addSpacing(12);
	buttons->addWidget(_btnResetBand);
	buttons->addWidget(exportButton);
	buttons->addWidget(closeButton);

	auto *layout = new QVBoxLayout(this);
	layout->addLayout(top);
	layout->addWidget(splitter, 1);
	layout->addLayout(buttons);

	setNavigation(0, 0);
	resize(1080, 580);
}


void SpectralDiagnosticsView::setDiagnostics(const QString &title,
                                             const QString &source,
                                             const SpectralDiagnostics &diag) {
	_diag = diag;
	_title = title;
	_source = source;
	_message.clear();
	updateHeader();
	_plot->setDiagnostics(&_diag);
	updateTable();
}


void SpectralDiagnosticsView::clear(const QString &title, const QString &message) {
	_diag.clear();
	_title = title;
	_source.clear();
	_message = message;
	updateHeader();
	_plot->setDiagnostics(nullptr);
	updateTable();
}


void SpectralDiagnosticsView::setNavigation(int index, int count) {
	_btnPrevious->setEnabled(count > 0 && index > 1);
	_btnNext->setEnabled(count > 0 && index < count);
	_position->setText(count > 0 ? QString("%1 / %2").arg(index).arg(count) : QString());
}


void SpectralDiagnosticsView::setStationInfo(const QString &info) {
	_stationInfo = info;
	updateHeader();
}


void SpectralDiagnosticsView::setStationUsed(bool available, bool used) {
	_useStation->setEnabled(available);
	_useStation->setChecked(available && used);
}


void SpectralDiagnosticsView::setOverlay(const QVector<OverlayCurve> &curves) {
	_overlay = curves;
	_plot->setOverlay(_overlay.isEmpty() ? nullptr : &_overlay);
}


bool SpectralDiagnosticsView::overlayEnabled() const {
	return _showOverlay->isChecked();
}


SpectralDiagnosticsView::Order SpectralDiagnosticsView::order() const {
	return static_cast<Order>(_order->currentData().toInt());
}


void SpectralDiagnosticsView::setBandEditable(bool editable) {
	_plot->setBandEditable(editable);
	_btnResetBand->setVisible(editable);
}


void SpectralDiagnosticsView::showEvent(QShowEvent *e) {
	QWidget::showEvent(e);
	emit visibilityChanged(true);
}


void SpectralDiagnosticsView::hideEvent(QHideEvent *e) {
	QWidget::hideEvent(e);
	emit visibilityChanged(false);
}


void SpectralDiagnosticsView::updateHeader() {
	QString text = QString("<b>%1</b>").arg(_title.toHtmlEscaped());
	if ( !_stationInfo.isEmpty() ) {
		text += QString(" &nbsp; %1").arg(_stationInfo.toHtmlEscaped());
	}

	text += "<br/>";

	if ( !_message.isEmpty() ) {
		text += QString("<i>%1</i>").arg(_message.toHtmlEscaped());
	}
	else {
		QString status = _diag.status.empty() ? QString("ok") : QString(_diag.status.c_str());
		// Combined measurements may report one status per component,
		// e.g. "N: ok, E: ok"
		bool ok = true;
		for ( const QString &part : status.split(',') ) {
			if ( !part.trimmed().endsWith("ok") ) {
				ok = false;
			}
		}
		text += QString("%1 &nbsp; <span style='color:%2'>%3</span>")
		        .arg(_source.toHtmlEscaped(), ok ? "green" : "#b00020",
		             status.toHtmlEscaped());
	}

	_header->setText(text);
	setWindowTitle(tr("Spectrum of %1").arg(_title));
}


void SpectralDiagnosticsView::updateTable() {
	_table->setRowCount(0);
	for ( const auto &param : _diag.parameters ) {
		int row = _table->rowCount();
		_table->insertRow(row);

		QString value = formatValue(param.value);
		if ( param.lowerUncertainty || param.upperUncertainty ) {
			if ( param.lowerUncertainty && param.upperUncertainty
			  && *param.lowerUncertainty == *param.upperUncertainty ) {
				value += QString(" ± %1").arg(formatValue(*param.lowerUncertainty));
			}
			else {
				value += QString(" -%1/+%2")
				         .arg(param.lowerUncertainty ? formatValue(*param.lowerUncertainty) : "?")
				         .arg(param.upperUncertainty ? formatValue(*param.upperUncertainty) : "?");
			}
		}

		_table->setItem(row, 0, new QTableWidgetItem(param.id.c_str()));
		auto *item = new QTableWidgetItem(value);
		item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_table->setItem(row, 1, item);
		_table->setItem(row, 2, new QTableWidgetItem(param.unit.c_str()));
	}

	for ( const auto &band : _diag.bands ) {
		int row = _table->rowCount();
		_table->insertRow(row);
		_table->setItem(row, 0, new QTableWidgetItem(tr("band")));
		auto *item = new QTableWidgetItem(QString("%1 – %2")
		                                  .arg(formatValue(band.first), formatValue(band.second)));
		item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_table->setItem(row, 1, item);
		_table->setItem(row, 2, new QTableWidgetItem("Hz"));
	}
}


void SpectralDiagnosticsView::exportCurves() {
	if ( _diag.curves.empty() ) return;

	QString name = _title;
	name.replace(' ', '_');
	QString fn = QFileDialog::getSaveFileName(this, tr("Export spectra"),
	                                          QString("%1-spectra.txt").arg(name),
	                                          tr("Text files (*.txt)"));
	if ( fn.isEmpty() ) return;

	std::ofstream os(fn.toStdString());
	if ( !os ) {
		QMessageBox::critical(this, tr("Export"), tr("Could not open %1").arg(fn));
		return;
	}

	os << "# " << _title.toStdString() << "\n";
	if ( !_diag.status.empty() ) os << "# status: " << _diag.status << "\n";
	for ( const auto &param : _diag.parameters ) {
		os << "# " << param.id << " = " << param.value << " " << param.unit << "\n";
	}
	for ( const auto &band : _diag.bands ) {
		os << "# band = " << band.first << " " << band.second << " Hz\n";
	}
	for ( const auto &c : _diag.curves ) {
		os << "\n# curve: " << c.label << " (" << roleName(c.role).toStdString()
		   << ", " << (c.component ? c.component : '-') << ", " << c.unit << ")\n";
		for ( size_t i = 0; i < c.freq.size() && i < c.value.size(); ++i ) {
			os << c.freq[i] << " " << c.value[i] << "\n";
		}
	}
}


}
}
