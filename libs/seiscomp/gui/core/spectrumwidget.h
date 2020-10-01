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


#ifndef SEISCOMP_GUI_CORE_SPECTRUMWIDGET_H
#define SEISCOMP_GUI_CORE_SPECTRUMWIDGET_H


#include <QWidget>
#include <seiscomp/gui/plot/datay.h>
#include <seiscomp/gui/plot/graph.h>

#ifndef Q_MOC_RUN
#include <seiscomp/math/fft.h>
#include <seiscomp/processing/response.h>
#endif


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SpectrumWidget : public QWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		SpectrumWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setMargin(int m) { _margin = qMax(0, m); }

		/**
		 * @brief Sets the spectrum values and the Nyquist frequency as the
		 *        frequency of the last spectrum value.
		 * @param freqNyquist The last spectrum value is measured for that frequency
		 * @param spec The spectrum values from 0Hz to [fNyquist]Hz
		 */
		void setSpectrum(double freqNyquist, const Math::ComplexArray &spec,
		                 Processing::Response *resp = nullptr,
		                 QString exportBasename = QString());

		//! Access the x-axis
		Axis &xAxis() { return _xAxis; }

		//! Access the y-axis
		Axis &yAxis() { return _yAxis; }


	// ----------------------------------------------------------------------
	//  Public slots
	// ----------------------------------------------------------------------
	public slots:
		void setAmplitudeSpectrum();
		void setPhaseSpectrum();
		void setPowerSpectrum();
		void setLogScaleX(bool logScale);
		void setLogScaleY(bool logScale);

		void setShowSpectrum(bool show);
		void setShowCorrected(bool show);
		void setShowResponse(bool show);

		/**
		 * @brief Exports all visible spectra into a simple ASCII file
		 */
		void exportSpectra();


	// ----------------------------------------------------------------------
	//  QWidget interface
	// ----------------------------------------------------------------------
	protected:
		void resizeEvent(QResizeEvent *e);
		void paintEvent(QPaintEvent *e);


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		void updateData();
		void draw(QPainter &, const Graph *);
		void updateRanges();
		void updateAxisLabels();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		enum Mode {
			Amplitude,
			Power,
			Phase
		};

		QString                 _exportBasename;
		double                  _freqNyquist;
		Math::ComplexArray      _spec;
		Processing::ResponsePtr _resp;
		Mode                    _mode;
		Graph                   _graphPowerSpectrum;
		Graph                   _graphResponseCorrectedPowerSpectrum;
		Graph                   _graphResponsePowerSpectrum;
		DataY                   _powerSpectrum;
		DataY                   _responseCorrectedPowerSpectrum;
		DataY                   _responsePowerSpectrum;
		int                     _margin;
		Axis                    _xAxis;
		Axis                    _yAxis;
		Axis                    _yAxis2;
};


}
}


#endif
