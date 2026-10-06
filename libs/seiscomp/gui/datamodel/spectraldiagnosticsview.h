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


#ifndef SEISCOMP_GUI_SPECTRALDIAGNOSTICSVIEW_H
#define SEISCOMP_GUI_SPECTRALDIAGNOSTICSVIEW_H


#include <QVector>
#include <QWidget>
#include <seiscomp/gui/qt.h>
#ifndef Q_MOC_RUN
#include <seiscomp/processing/spectraldiagnostics.h>
#endif


class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QToolButton;


namespace Seiscomp {
namespace Gui {


class SpectralDiagnosticsPlot;


/**
 * @brief Shows the spectral diagnostics of an amplitude measurement: the
 *        curves grouped by their role on log-log axes, the used frequency
 *        bands, frequency markers and a table of the parameters.
 *
 * The view does not know about stations. Navigation, the station state and
 * band changes are requested through signals and handled by the owner,
 * e.g. the amplitude review window.
 */
class SC_GUI_API SpectralDiagnosticsView : public QWidget {
	Q_OBJECT

	public:
		//! A curve of another station shown for comparison
		struct OverlayCurve {
			QString                   station;
			Processing::SpectralCurve curve;
		};

		//! The order in which previous/next step through the stations
		enum Order {
			ListOrder,
			MagnitudeDeviation,
			FitResidual,
			LowestSNR
		};

	public:
		SpectralDiagnosticsView(QWidget *parent = nullptr,
		                        Qt::WindowFlags f = Qt::WindowFlags());

	public:
		/**
		 * @brief Sets the diagnostics to display.
		 * @param title The title, e.g. the stream ID
		 * @param source Where the diagnostics come from, e.g. the processor
		 *               or "signal/noise windows"
		 * @param diag The diagnostics
		 */
		void setDiagnostics(const QString &title, const QString &source,
		                    const Processing::SpectralDiagnostics &diag);

		//! Clears the view and shows a message instead
		void clear(const QString &title, const QString &message);

		/**
		 * @brief Sets the position of the shown station in the list of
		 *        stations, e.g. 3 of 35. Navigation is disabled if count
		 *        is 0.
		 */
		void setNavigation(int index, int count);

		//! Sets additional station information, e.g. distance and magnitude
		void setStationInfo(const QString &info);

		/**
		 * @brief Sets the state of the "Use station" toggle.
		 * @param available Whether the station can be enabled/disabled
		 * @param used The current state
		 */
		void setStationUsed(bool available, bool used);

		//! Sets the curves of the other stations shown with "Overlay"
		void setOverlay(const QVector<OverlayCurve> &curves);
		bool overlayEnabled() const;

		Order order() const;

		//! Allows dragging the band edges, see spectralBandChanged
		void setBandEditable(bool editable);

	signals:
		void previousRequested();
		void nextRequested();
		//! Emitted when the user toggles the "Use station" checkbox
		void stationUseChanged(bool used);
		void overlayToggled(bool enabled);
		void orderChanged();
		//! Emitted when the user dragged a new band
		void spectralBandChanged(double fmin, double fmax);
		//! Emitted when the user requested the automatic band
		void spectralBandReset();
		//! Emitted when the view is shown or hidden (closed)
		void visibilityChanged(bool visible);

	protected:
		void showEvent(QShowEvent *e) override;
		void hideEvent(QHideEvent *e) override;

	private slots:
		void exportCurves();

	private:
		void updateHeader();
		void updateTable();

	private:
		Processing::SpectralDiagnostics  _diag;
		QVector<OverlayCurve>            _overlay;
		QString                          _title;
		QString                          _source;
		QString                          _message;
		QString                          _stationInfo;
		QLabel                          *_header;
		QLabel                          *_cursorInfo;
		QToolButton                     *_btnPrevious;
		QToolButton                     *_btnNext;
		QLabel                          *_position;
		QComboBox                       *_order;
		QCheckBox                       *_useStation;
		SpectralDiagnosticsPlot         *_plot;
		QTableWidget                    *_table;
		QCheckBox                       *_showNoise;
		QCheckBox                       *_showCorrections;
		QCheckBox                       *_showOther;
		QCheckBox                       *_showOverlay;
		QPushButton                     *_btnResetBand;
};


}
}


#endif
