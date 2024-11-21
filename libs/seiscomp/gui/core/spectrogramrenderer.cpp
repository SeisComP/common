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


#include <seiscomp/gui/core/spectrogramrenderer.h>
#include <seiscomp/gui/core/application.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpectrogramRenderer::SpectrogramRenderer() {
	_tmin = _tmax = 0;
	_scale = 1.0;
	_ampMin = -15;
	_ampMax = -5;
	_imageFormat = QImage::Format_RGB32;

	if ( SCApp ) {
		setGradient(SCApp->scheme().colors.spectrogram);
	}

	_normalizationMode = NormalizationMode::Fixed;
	_logarithmic = false;
	_smoothTransform = true;
	_dirty = true;

	_renderedFmin = _renderedFmax = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setGradient(const Gradient &gradient) {
	Gradient::const_iterator it;
	bool hasAlphaChannel = false;

	for ( it = gradient.begin(); it != gradient.end(); ++it ) {
		if ( it.value().first.alpha() < 255 ) {
			hasAlphaChannel = true;
			break;
		}
	}

	_imageFormat = hasAlphaChannel ? QImage::Format_ARGB32 : QImage::Format_RGB32;

	_gradient.generateFrom(gradient);
	_gradient.setRange(_ampMin, _ampMax);

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpectrogramRenderer::setOptions(const IO::Spectralizer::Options &opts) {
	if ( !_spectralizer )
		_spectralizer = new IO::Spectralizer;

	// Reset data
	reset();

	_options = opts;
	return _spectralizer->setOptions(_options);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setScale(double scale) {
	_scale = scale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setGradientRange(double lowerBound, double upperBound) {
	_ampMin = lowerBound;
	_ampMax = upperBound;

	_gradient.setRange(lowerBound, upperBound);

	if ( !_spectralizer ) return;

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::reset() {
	if ( !_spectralizer ) return;

	_spectra.clear();
	_images.clear();

	_spectralizer = new IO::Spectralizer;
	_spectralizer->setOptions(_options);

	_renderedFmin = _renderedFmax = -1;

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpectrogramRenderer::feed(const Record *rec) {
	if ( !_spectralizer ) {
		return false;
	}

	// Record clipped
	if ( _timeWindow.startTime().valid() && rec->endTime() <= _timeWindow.startTime() ) {
		return false;
	}

	// Record clipped
	if ( _timeWindow.endTime().valid() && rec->startTime() >= _timeWindow.endTime() ) {
		return false;
	}

	if ( _spectralizer->push(rec) ) {
		IO::SpectrumPtr spec;

		while ( (spec = _spectralizer->pop()) ) {
			if ( !spec->isValid() ) {
				continue;
			}

			// Deconvolution
			if ( _transferFunction ) {
				Seiscomp::ComplexDoubleArray *data = spec->data();
				double df = spec->maximumFrequency() / (data->size()-1);
				_transferFunction->deconvolve(data->size()-1, data->typedData()+1, df, df);
			}

			_spectra.push_back(new PowerSpectrum(*spec, _scale));
			if ( !_dirty ) {
				addSpectrum(_spectra.back().get());
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpectrogramRenderer::feedSequence(const RecordSequence *seq) {
	if ( seq == nullptr ) return true;

	RecordSequence::const_iterator it;
	bool result = false;
	for ( it = seq->begin(); it != seq->end(); ++it )
		if ( feed(it->get()) ) result = true;

	setDirty();

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setRecords(const RecordSequence *seq) {
	reset();
	feedSequence(seq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setAlignment(const Core::Time &align) {
	_alignment = align;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setTimeRange(double tmin, double tmax) {
	_tmin = tmin;
	_tmax = tmax;

	if ( _tmin > _tmax )
		std::swap(_tmin, _tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setTimeWindow(const Core::TimeWindow& tw) {
	_timeWindow = tw;

	bool needUpdate = false;

	// Trim front
	if ( _timeWindow.startTime().valid() && !_spectra.empty() ) {
		auto it = _spectra.begin();

		while ( it != _spectra.end() ) {
			auto spec = it->get();
			if ( spec->endTime <= _timeWindow.startTime() ) {
				it = _spectra.erase(it);
				needUpdate = true;
			}
			else {
				break;
			}
		}
	}

	// Trim back
	if ( _timeWindow.endTime().valid() && !_spectra.empty() ) {
		auto it = _spectra.end();

		while ( it != _spectra.begin() ) {
			--it;
			auto spec = it->get();
			if ( spec->startTime >= _timeWindow.endTime() ) {
				it = _spectra.erase(it);
				needUpdate = true;
			}
			else {
				break;
			}
		}
	}

	if ( needUpdate ) {
		setDirty();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setFrequencyRange(OPT(double) fmin, OPT(double) fmax) {
	_fmin = fmin;
	_fmax = fmax;

	if ( normalizeAmplitudes() ) {
		setDirty();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setNormalizeAmplitudes(bool f) {
	setNormalizationMode(f ? NormalizationMode::Frequency : NormalizationMode::Fixed);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setNormalizationMode(NormalizationMode mode) {
	if ( _normalizationMode == mode ) {
		return;
	}

	_normalizationMode = mode;
	_normalizationAmpRange[0] = _normalizationAmpRange[1] = -1;

	if ( !_spectralizer ) {
		return;
	}

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setLogScale(bool f) {
	if ( _logarithmic == f ) {
		return;
	}

	_logarithmic = f;

	if ( !_spectralizer ) {
		return;
	}

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setSmoothTransform(bool st) {
	_smoothTransform = st;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setTransferFunction(Math::Restitution::FFT::TransferFunction *tf) {
	_transferFunction = tf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setDirty() {
	_dirty = true;
	_images.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::renderSpectrogram() {
	_images.clear();

	for ( auto &spec : _spectra ) {
		addSpectrum(spec.get());
	}

	_dirty = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::addSpectrum(const PowerSpectrum *spec) {
	if ( _normalizationMode == NormalizationMode::Time ) {
		_dirty = true;
		return;
	}

#define PRE_ALLOC_WIDTH 100
	auto data = spec->data.get();
	int offset = 0;

	double minFreq = spec->minimumFrequency();
	int nSamples = data->size();

	if ( _logarithmic ) {
		++offset;
		--nSamples;
		minFreq = spec->maximumFrequency() / nSamples;
	}

	if ( _images.empty() ) {
		SpecImage img;
		img.minimumFrequency = minFreq;
		img.maximumFrequency = spec->maximumFrequency();
		img.startTime = spec->center();
		img.dt = spec->dt;

		img.data = QImage(PRE_ALLOC_WIDTH, data->size(), _imageFormat);
		img.width = 1;

		fillRow(img, data, 0, offset);

		_images.append(img);
	}
	else {
		SpecImage &img = _images.back();
		Core::Time newTime = spec->center();

		// Do more checks on gaps and so on

		double dt = (double)img.dt;
		Core::Time currentEndTime = img.startTime + Core::TimeSpan(img.width*dt);

		bool needNewImage = (fabs((double)(newTime - currentEndTime)) > dt*0.5)
		                 || (img.data.height() != nSamples)
		                 || (img.minimumFrequency != minFreq)
		                 || (img.maximumFrequency != spec->maximumFrequency())
		                 || (img.dt != spec->dt);

		// Gap, overlap or different meta data -> start new image
		if ( needNewImage ) {
			if ( img.width < img.data.width() ) {
				// Trim image width
				img.data = img.data.copy(0,0,img.width,img.data.height());
			}

			SpecImage newImg;
			newImg.minimumFrequency = minFreq;
			newImg.maximumFrequency = spec->maximumFrequency();
			newImg.startTime = spec->center();
			newImg.dt = spec->dt;

			newImg.data = QImage(PRE_ALLOC_WIDTH, data->size(), _imageFormat);
			newImg.width = 1;
			fillRow(newImg, data, 0, offset);

			_images.append(newImg);
		}
		else {
			if ( img.width >= img.data.width() ) {
				// Extent image by one column
				img.data = img.data.copy(0,0,img.width+PRE_ALLOC_WIDTH,img.data.height());
			}

			// Fill colors for column
			fillRow(img, data, img.width, offset);
			++img.width;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::fillRow(SpecImage &img, DoubleArray *spec,
                                  int column, int offset) {
	QRgb *rgb = (QRgb*)img.data.bits();
	int ofs = img.data.width();
	int n = spec->size();
	double maxFreq = img.maximumFrequency;
	double norm = _scale * 0.5 / img.maximumFrequency;

	// Goto nth column
	rgb += column;

	if ( _normalizationMode != NormalizationMode::Fixed ) {
		double amin = -1, amax = -1;
		double fmin = 0.0, fmax = maxFreq;
		if ( _fmin ) {
			fmin = *_fmin;
		}
		if ( _fmax ) {
			fmax = *_fmax;
		}

		// Compute min/max amplitude
		double f = maxFreq;
		double df = maxFreq / (spec->size()-1);

		for ( int i = n; i > offset; --i, f -= df ) {
			if ( f < fmin || f > fmax ) {
				continue;
			}

			double ps = (*spec)[i-1] * norm * norm;
			if ( ps > 0 ) {
				if ( amin < 0 || amin > ps ) {
					amin = ps;
				}
				if ( amax < 0 || amax < ps ) {
					amax = ps;
				}
			}
		}

		double ascale = 1.0;

		if ( amin > 0 && amax > 0 ) {
			amin = log10(amin);
			amax = log10(amax);

			double arange = amax - amin;
			if ( arange > 0 ) {
				ascale = 1.0 / arange;
			}
		}
		else {
			amin = 0;
			ascale = 0;
		}

		if ( _logarithmic ) {
			double logTo = log10(n);
			double logFrom = 0;
			double logRange = logTo - logFrom;

			for ( int i = n; i > offset; --i ) {
				double li = pow(10.0, (i - 1) * logRange / (n - 1) + logFrom) - 1;
				int i0 = static_cast<int>(li);
				double t = li-i0;
				double ps;

				if ( i0 >= n-1 ) {
					ps = (*spec)[n-1] * norm * norm;
				}
				else if ( t == 0 ) {
					ps = (*spec)[i0] * norm * norm;
				}
				else {
					ps = (*spec)[i0] * norm * norm;
					ps = ps * (1-t) + ((*spec)[i0+1] * norm * norm) * t;
				}

				double amp = ps > 0 ? log10(ps) : _gradient.lowerBound();
				amp = (amp - amin) * ascale;

				*rgb = _gradient.valueAtNormalizedIndex(amp);
				rgb += ofs;
			}
		}
		else {
			for ( int i = n; i > offset; --i ) {
				double ps = (*spec)[i-1] * norm * norm;
				double amp = ps > 0 ? log10(ps) : _gradient.lowerBound();
				amp = (amp - amin) * ascale;

				*rgb = _gradient.valueAtNormalizedIndex(amp);
				rgb += ofs;
			}
		}
	}
	else {
		// Go from highest to lowest frequency
		if ( _logarithmic ) {
			double logTo = log10(n);
			double logFrom = 0;
			double logRange = logTo - logFrom;

			for ( int i = n; i > offset; --i ) {
				double li = pow(10.0, (i-1)*logRange/(n-1) + logFrom) - 1;
				int i0 = (int)li;
				double t = li-i0;
				double ps;

				if ( i0 >= n-1 ) {
					ps = (*spec)[n-1] * norm * norm;
				}
				else if ( t == 0 ) {
					ps = (*spec)[i0] * norm * norm;
				}
				else {
					ps = (*spec)[i0] * norm * norm;
					ps = ps * (1-t) + ((*spec)[i0+1] * norm * norm) * t;
				}

				double amp = ps > 0 ? log10(ps) : _gradient.lowerBound();

				*rgb = _gradient.valueAt(amp);
				rgb += ofs;
			}
		}
		else {
			for ( int i = n; i > offset; --i ) {
				double ps = (*spec)[i-1] * norm * norm;
				double amp = ps > 0 ? log10(ps) : _gradient.lowerBound();

				*rgb = _gradient.valueAt(amp);
				rgb += ofs;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::render(QPainter &p, const QRect &rect,
                                 bool labelLeftAlign, bool renderLabels) {
	SpecImageList::iterator it;
	double fmin = -1, lfmin = 0;
	double fmax = -1, lfmax = 0;

	double frange;

	_renderedFmin = fmin;
	_renderedFmax = fmax;

	int w = rect.width();
	int h = rect.height();

	if ( (h <= 0) || (w <= 0) ) {
		return;
	}

	Core::Time t0 = _alignment + Core::TimeSpan(_tmin);
	Core::Time t1 = _alignment + Core::TimeSpan(_tmax);

	if ( _normalizationMode == NormalizationMode::Time ) {
		double minAmp = -1;
		double maxAmp = -1;
		bool first = true;

		for ( auto &spec : _spectra ) {
			if ( spec->center() + spec->dt <= t0 ) {
				continue;
			}

			if ( t1 <= spec->center() ) {
				continue;
			}

			if ( first ) {
				minAmp = spec->minimumAmplitude;
				maxAmp = spec->maximumAmplitude;
				first = false;
			}
			else {
				if ( spec->minimumAmplitude < minAmp ) {
					minAmp = spec->minimumAmplitude;
				}

				if ( spec->maximumAmplitude > maxAmp ) {
					maxAmp = spec->maximumAmplitude;
				}
			}

			minAmp = floor(minAmp);
			maxAmp = ceil(maxAmp);
		}

		if ( _dirty || (minAmp != _normalizationAmpRange[0]) || (maxAmp != _normalizationAmpRange[1]) ) {
			_normalizationAmpRange[0] = minAmp;
			_normalizationAmpRange[1] = maxAmp;

			minAmp = _ampMin;
			maxAmp = _ampMax;

			_normalizationMode = NormalizationMode::Fixed;
			setGradientRange(_normalizationAmpRange[0], _normalizationAmpRange[1]);
			renderSpectrogram();

			_normalizationMode = NormalizationMode::Time;
			_ampMin = minAmp;
			_ampMax = maxAmp;

			_gradient.setRange(_ampMin, _ampMax);
			_dirty = false;
		}
	}

	if ( _dirty ) {
		renderSpectrogram();
	}

	if ( _images.empty() ) {
		return;
	}

	if ( !_fmax ) {
		bool first = true;
		for ( it = _images.begin(); it != _images.end(); ++ it ) {
			if ( first ) {
				fmax = it->maximumFrequency;
				first = false;
			}
			else if ( it->maximumFrequency > fmax )
				fmax = it->maximumFrequency;
		}
	}
	else
		fmax = *_fmax;

	if ( !_fmin ) {
		bool first = true;
		for ( it = _images.begin(); it != _images.end(); ++ it ) {
			if ( first ) {
				fmin = it->minimumFrequency;
				first = false;
			}
			else if ( it->minimumFrequency < fmin )
				fmin = it->minimumFrequency;
		}
	}
	else
		fmin = *_fmin;

	if ( fmin > fmax ) std::swap(fmin, fmax);

	if ( _logarithmic ) {
		lfmin = log10(fmin);
		lfmax = log10(fmax);
		frange = lfmax - lfmin;
	}
	else
		frange = fmax - fmin;

	_renderedFmin = fmin;
	_renderedFmax = fmax;

	double xlen = static_cast<double>(t1 - t0);

	// Nothing to draw
	if ( xlen <= 0 ) return;

	double dx = 1.0/xlen;

	p.save();
	if ( _smoothTransform )
		p.setRenderHint(QPainter::SmoothPixmapTransform);

	// Draw images
	for ( it = _images.begin(); it != _images.end(); ++ it ) {
		SpecImage &img = *it;

		// Clip by min frequency
		if ( img.minimumFrequency >= fmax ) continue;

		// Clip by max frequency
		if ( img.maximumFrequency <= fmin ) continue;

		Core::Time startTime = img.startTime - Core::TimeSpan((double)img.dt*0.5);
		Core::Time endTime   = startTime + Core::TimeSpan((double)img.dt * img.width);

		// Clip by start time
		if ( startTime >= t1 ) continue;

		// Clip by end time
		if ( endTime <= t0 ) continue;

		double x0 = static_cast<double>(startTime - t0);
		double x1 = static_cast<double>(endTime - t0);

		// Convert start and end time into widget coordinates
		int ix0 = (int)(x0*dx*w);
		int ix1 = (int)(x1*dx*w);

		// Source lines to plot
		int sy0 = 0;
		int sy1 = img.data.height();

		int ty0 = 0;
		int ty1 = h;

		if ( _logarithmic ) {
			if ( img.minimumFrequency > fmin  ) {
				// Map screen to image
				ty0 = (log10(img.minimumFrequency) - lfmin) / frange * h;
			}
			else /* fmin >= img.minimumFrequency */ {
				// Map image to screen
				sy0 = (lfmin - log10(img.minimumFrequency)) / (log10(img.maximumFrequency) - log10(img.minimumFrequency)) * img.data.height();
			}

			if ( img.maximumFrequency < fmax  ) {
				// Map screen to image
				ty1 = (log10(img.maximumFrequency) - lfmin) / frange * h;
			}
			else /* fmax <= img.maximumFrequency */ {
				// Map image to screen
				sy1 = (lfmax - log10(img.minimumFrequency)) / (log10(img.maximumFrequency) - log10(img.minimumFrequency)) * img.data.height();
			}
		}
		else {
			if ( img.minimumFrequency > fmin )
				ty0 = (img.minimumFrequency - fmin) / frange * h;
			else /* fmin >= img.minimumFrequency */
				sy0 = (fmin - img.minimumFrequency) / (img.maximumFrequency - img.minimumFrequency) * img.data.height();

			if ( img.maximumFrequency < fmax )
				ty1 = (img.maximumFrequency - fmin) / frange * h;
			else /* fmax <= img.maximumFrequency */
				sy1 = (fmax - img.minimumFrequency) / (img.maximumFrequency - img.minimumFrequency) * img.data.height();
		}

		QRect sourceRect(0,img.data.height()-sy1,img.width,sy1-sy0),
		      targetRect(rect.left()+ix0,rect.top()+h-ty1,ix1-ix0,ty1-ty0);

		p.drawImage(targetRect, img.data, sourceRect);
	}

	if ( renderLabels && (fmin >= 0) && (fmax >= 0) ) {
		QFont font = p.font();
		QFontInfo fi(font);
		int fontSize = std::min(h/2, fi.pixelSize());

		font.setPixelSize(fontSize);

		p.setFont(font);

		QString minFreq = QString("%1Hz").arg(fmin);
		QString maxFreq = QString("%1Hz").arg(fmax);

		QRect minR = p.fontMetrics().boundingRect(minFreq);
		QRect maxR = p.fontMetrics().boundingRect(maxFreq);

		minR.adjust(-2,-2,2,2);
		maxR.adjust(-2,-2,2,2);

		if ( labelLeftAlign )
			minR.moveBottomLeft(rect.bottomLeft()-QPoint(0,1));
		else
			minR.moveBottomRight(rect.bottomRight()-QPoint(0,1));

		p.drawRect(minR);
		p.drawText(minR, Qt::AlignCenter, minFreq);

		if ( labelLeftAlign )
			maxR.moveTopLeft(rect.topLeft());
		else
			maxR.moveTopRight(rect.topRight());

		p.drawRect(maxR);
		p.drawText(maxR, Qt::AlignCenter, maxFreq);
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::renderAxis(QPainter &p, const QRect &rect,
                                     bool leftAlign, int paddingOuter,
                                     int paddingInner, bool stretch) {
	if ( (_renderedFmin < 0) || (_renderedFmax < 0) ) return;

	int w = rect.width();
	int h = rect.height();

	if ( (h <= 0) || (w <= 0) ) return;

	QFont font = p.font();
	QFontInfo fi(font);
	int fontSize = std::min(h/2, fi.pixelSize());

	font.setPixelSize(fontSize);

	p.setFont(font);

	double frange = _renderedFmax - _renderedFmin;

	int fontHeight = p.fontMetrics().height();
	int tickLength = fontHeight/2+1;
	int tickSpacing = tickLength*3/2-tickLength;

	QString minFreq = QString("%1").arg(_renderedFmin);
	QString maxFreq = QString("%1").arg(_renderedFmax);

	QRect minR = p.fontMetrics().boundingRect(minFreq);
	QRect maxR = p.fontMetrics().boundingRect(maxFreq);

	QString axisLabel = "f [1/T] in Hz";
	QRect axisLabelR = p.fontMetrics().boundingRect(axisLabel);

	if ( axisLabelR.width() >= h-4 ) {
		axisLabel = "Hz";
		axisLabelR = p.fontMetrics().boundingRect(axisLabel);
	}

	minR.adjust(0,-1,0,1);
	maxR.adjust(0,-1,0,1);

	int wAxis = std::max(minR.width(), maxR.width()) + tickLength + tickSpacing + paddingInner;
	if ( stretch )
		wAxis = rect.width() - axisLabelR.height() - 2;

	QRect axis(0,0,wAxis,h), area;

	if ( leftAlign ) {
		axis.moveTopLeft(rect.topLeft());
		axis.translate(axisLabelR.height()+2,0);
		area = axis.adjusted(-axisLabelR.height()-2,0,0,0);
	}
	else {
		axis.moveTopRight(rect.topRight());
		axis.translate(-axisLabelR.height()-2,0);
		area = axis.adjusted(0,0,axisLabelR.height()+2,0);
	}

	p.fillRect(area, p.brush());

	if ( leftAlign ) {
		axis.adjust(0,0,-paddingInner,0);
		p.drawLine(axis.topRight(), axis.bottomRight());
	}
	else {
		axis.adjust(paddingInner,0,0,0);
		p.drawLine(axis.topLeft(), axis.bottomLeft());
	}

	p.save();
	p.translate(area.right()-2, area.center().y()+axisLabelR.width()/2);
	p.rotate(-90);
	p.drawText(axisLabelR, Qt::AlignHCenter | Qt::AlignTop, axisLabel);
	p.restore();

	if ( leftAlign ) {
		p.drawText(axis.adjusted(0,0,-tickLength-tickSpacing,0), Qt::AlignRight | Qt::AlignTop, maxFreq);
		p.drawText(axis.adjusted(0,0,-tickLength-tickSpacing,0), Qt::AlignRight | Qt::AlignBottom, minFreq);
	}
	else {
		p.drawText(axis.adjusted(tickLength+tickSpacing,0,0,0), Qt::AlignLeft | Qt::AlignTop, maxFreq);
		p.drawText(axis.adjusted(tickLength+tickSpacing,0,0,0), Qt::AlignLeft | Qt::AlignBottom, minFreq);
	}

	// Reduce axis rect to free area
	QRect labels = axis.adjusted(0,maxR.height()-2,0,-minR.height()-2);
	if ( leftAlign )
		labels.adjust(0,0,-tickLength-tickSpacing,0);
	else
		labels.adjust(tickLength+tickSpacing,0,0,0);

	int steps = 10;
	double vSpacing;

	if ( _logarithmic ) {
		vSpacing = double(log10(axis.height())) / steps;
	}
	else {
		vSpacing = double(axis.height()) / steps;
		if ( vSpacing < 4 ) {
			steps = 5;
			vSpacing = double(axis.height()) / steps;
		}
	}

	double fSpacing = frange / steps;

	int xPos = leftAlign ? axis.right()-tickLength : axis.left();
	int sxPos = leftAlign ? axis.right()-tickLength/2 : axis.left();
	int subSteps = 10;
	while ( vSpacing*2 < fontHeight*subSteps )
		subSteps /= 2;

	p.save();
	p.setBrush(Qt::NoBrush);

	int alignmentFlags = (leftAlign ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter;

	double f = _renderedFmax;
	double yPos = 0;

	for ( int i = 0; i <= steps; ++i, yPos += vSpacing, f -= fSpacing ) {
		int y = axis.top() + (int)(_logarithmic ? pow(10.0, yPos) : yPos);
		if ( y > axis.bottom() ) y = axis.bottom();
		p.drawLine(xPos, y, xPos+tickLength, y);

		for ( int j = 1; j < subSteps; ++j ) {
			int sy = y + j*vSpacing/subSteps;
			p.drawLine(sxPos, sy, sxPos+tickLength/2, sy);
		}

		// Try to render label
		QString freq = QString("%1").arg(f);
		QRect freqR = p.fontMetrics().boundingRect(freq);
		freqR.adjust(0,1,0,1);

		if ( leftAlign )
			freqR.moveRight(labels.right());
		else
			freqR.moveLeft(labels.left());

		freqR.moveTop(y-freqR.height()/2);

		if ( labels.contains(freqR) ) {
			p.drawText(freqR, alignmentFlags, freq);
			// Shrink the labels area
			labels.setTop(freqR.bottom());
		}
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
