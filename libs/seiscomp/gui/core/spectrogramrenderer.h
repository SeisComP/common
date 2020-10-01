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


#ifndef SEISCOMP_GUI_CORE_SPECTROGRAMRENDERER_H
#define SEISCOMP_GUI_CORE_SPECTROGRAMRENDERER_H


#ifndef Q_MOC_RUN
#include <seiscomp/core/recordsequence.h>
#include <seiscomp/math/restitution/transferfunction.h>
#include <seiscomp/io/recordfilter/spectralizer.h>
#endif
#include <seiscomp/gui/core/lut.h>

#include <QPainter>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SpectrogramRenderer {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		SpectrogramRenderer();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the spectrogram options and calls reset().
		bool setOptions(const IO::Spectralizer::Options &opts);
		const IO::Spectralizer::Options &options() const { return _options; }

		//! Sets the scale of the raw stream data which is 1/gain to convert
		//! to sensor units
		void setScale(double scale);

		//! Sets the color gradient. Key range is normalized to [0,1].
		void setGradient(const Gradient &gradient);

		//! Sets the gradient range
		void setGradientRange(double lowerBound, double upperBound);

		double gradientLowerBound() const { return _gradient.lowerBound(); }
		double gradientUpperBound() const { return _gradient.upperBound(); }

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

		const OPT(double) &frequencyLowerBound() const { return _fmin; }
		const OPT(double) &frequencyUpperBound() const { return _fmax; }

		void setNormalizeAmplitudes(bool f);
		bool normalizeAmplitudes() const { return _normalize; }

		void setLogScale(bool f);
		bool logScale() const { return _logarithmic; }

		void setSmoothTransform(bool);
		bool smoothTransform() const { return _smoothTransform; }

		//! Sets the transfer function for deconvolution
		void setTransferFunction(Math::Restitution::FFT::TransferFunction *tf);

		bool isDirty() const { return _dirty; }

		//! Creates the spectrogram. This is usually done in render if the
		//! spectrogram is dirty but can called from outside.
		void renderSpectrogram();

		//! Renders the spectrogram with the given painter into the given rect.
		void render(QPainter &p, const QRect &rect, bool labelLeftAlign = true,
		            bool renderLabels = false);

		//! Renders the y axis. This call must precede a call to render otherwise
		//! the frequency range can by out of sync.
		void renderAxis(QPainter &p, const QRect &rect, bool leftAlign = true,
		                int paddingOuter = 6, int paddingInner = 0,
		                bool stretch = false);

		QPair<double,double> range() const;


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		struct SpecImage {
			QImage         data;
			Core::Time     startTime;
			Core::TimeSpan dt;
			double         minimumFrequency;
			double         maximumFrequency;
			int            width;
		};

		void setDirty();
		void addSpectrum(IO::Spectrum *);
		void fillRow(SpecImage &img, Seiscomp::ComplexDoubleArray *spec,
		             int column, int offset);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef QList<IO::SpectrumPtr> Spectra;

		typedef QList<SpecImage> SpecImageList;
		typedef StaticColorLUT<512> Gradient512;
		typedef Math::Restitution::FFT::TransferFunctionPtr TransferFunctionPtr;

		QImage::Format            _imageFormat;
		TransferFunctionPtr       _transferFunction;
		Core::TimeWindow          _timeWindow;
		Core::Time                _alignment;
		double                    _tmin, _tmax;
		double                    _scale;
		OPT(double)               _fmin, _fmax;
		double                    _ampMin, _ampMax;
		IO::Spectralizer::Options _options;
		IO::SpectralizerPtr       _spectralizer;
		Spectra                   _spectra;
		SpecImageList             _images;
		Gradient512               _gradient;
		bool                      _normalize;
		bool                      _logarithmic;
		bool                      _smoothTransform;
		bool                      _dirty;
		double                    _renderedFmin;
		double                    _renderedFmax;
};


inline QPair<double,double> SpectrogramRenderer::range() const {
	return QPair<double,double>(_renderedFmin, _renderedFmax);
}


}
}


#endif
