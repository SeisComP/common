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


#ifndef SEISCOMP_GUI_CORE_SPECTROGRAMWIDGET_H
#define SEISCOMP_GUI_CORE_SPECTROGRAMWIDGET_H


#include <QWidget>
#include <seiscomp/gui/core/spectrogramrenderer.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SpectrogramWidget : public QWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef IO::Spectralizer::Options SpectrumOptions;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		SpectrogramWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the spectrogram options and calls reset().
		bool setSpectrumOptions(const SpectrumOptions &opts);

		const SpectrumOptions &spectrumOptions() const;

		//! Sets the scale of the raw stream data which is 1/gain
		void setScale(double scale);

		//! Sets the gradient range
		void setGradientRange(double lowerBound, double upperBound);

		double gradientLowerBound() const;
		double gradientUpperBound() const;

		//! Resets the spectrogram and deletes all data
		void reset();

		//! Feeds a record for processing. Records must be timely ordered
		//! otherwise gaps are produced.
		bool feed(const Record *rec);
		bool feedSequence(const RecordSequence *seq);

		//! Resets the view and feeds the sequence
		void setRecords(const RecordSequence *seq);

		void setAlignment(const Core::Time &align);
		void setTimeRange(double tmin, double tmax);

		//! Sets the current time window of the data
		void setTimeWindow(const Core::TimeWindow &tw);

		//! Sets the frequency range to be displayed. A value lower or equal to
		//! zero refers to the global minimum or the global maximum
		//! respectively.
		void setFrequencyRange(OPT(double) fmin, OPT(double) fmax);

		const OPT(double) &frequencyLowerBound() const;
		const OPT(double) &frequencyUpperBound() const;

		void setNormalizeAmplitudes(bool f);
		bool normalizeAmplitudes() const;

		void setLogScale(bool f);
		bool logScale() const;

		void setSmoothTransform(bool st);
		bool smoothTransform() const;

		//! Sets the transfer function for deconvolution
		void setTransferFunction(Math::Restitution::FFT::TransferFunction *tf);


	// ----------------------------------------------------------------------
	//  Public slots
	// ----------------------------------------------------------------------
	public slots:
		void updateSpectrogram();


	// ----------------------------------------------------------------------
	//  Protected Qt Interface
	// ----------------------------------------------------------------------
	protected:
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		SpectrogramRenderer _renderer;
};


}
}


#endif
