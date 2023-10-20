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
		enum class NormalizationMode {
			Fixed,
			Frequency,
			Time
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
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
		void setNormalizationMode(NormalizationMode mode);
		bool normalizeAmplitudes() const { return _normalizationMode != NormalizationMode::Fixed; }

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

		DEFINE_SMARTPOINTER(PowerSpectrum);
		struct PowerSpectrum : public Core::BaseObject {
			PowerSpectrum(const IO::Spectrum &spectrum, double scale)
			: startTime(spectrum.startTime()), endTime(spectrum.endTime())
			, dt(spectrum.dt()), frequency(spectrum.maximumFrequency()) {
				centerTime = startTime + Core::TimeSpan(static_cast<double>(length()) * 0.5);
				auto d = spectrum.data();
				minimumAmplitude = maximumAmplitude = -1;
				if ( d ) {
					data = new DoubleArray(d->size());
					for ( int i = 0; i < d->size(); ++i ) {
						(*data)[i] = (*d)[i].real() * (*d)[i].real() + (*d)[i].imag() * (*d)[i].imag();
						if ( minimumAmplitude < 0 || minimumAmplitude > (*data)[i] ) {
							minimumAmplitude = (*data)[i];
						}
						if ( maximumAmplitude < (*data)[i] ) {
							maximumAmplitude = (*data)[i];
						}
					}

					if ( minimumAmplitude > 0 ) {
						double norm = scale * 0.5 / frequency;
						minimumAmplitude = log10(minimumAmplitude * norm * norm);
						maximumAmplitude = log10(maximumAmplitude * norm * norm);
					}
				}
			}

			bool isValid() const { return data && data->size() > 0; }

			Core::TimeSpan length() const { return endTime - startTime; }
			const Core::Time &center() const { return centerTime; }

			double minimumFrequency() const { return 0; }
			double maximumFrequency() const { return frequency; }

			Core::Time     startTime;
			Core::Time     endTime;
			Core::Time     centerTime;
			Core::TimeSpan dt;
			double         frequency;
			DoubleArrayPtr data;
			double         minimumAmplitude;
			double         maximumAmplitude;
		};

		void setDirty();
		void addSpectrum(const PowerSpectrum *);
		void fillRow(SpecImage &img, DoubleArray *spec,
		             int column, int offset);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef QList<PowerSpectrumPtr> PowerSpectra;

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
		PowerSpectra              _spectra;
		SpecImageList             _images;
		Gradient512               _gradient;
		NormalizationMode         _normalizationMode;
		double                    _normalizationAmpRange[2];
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
