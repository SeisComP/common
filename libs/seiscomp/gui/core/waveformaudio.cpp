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


#include "waveformaudio.h"

#include <QFile>
#include <QTemporaryFile>
#include <cmath>
#include <cstring>
#include <algorithm>


namespace Seiscomp {
namespace Gui {


WaveformAudio::WaveformAudio(QObject *parent)
: QObject(parent) {
}

WaveformAudio::~WaveformAudio() {
	stop();
}

void WaveformAudio::setWaveformData(const std::vector<double> &data,
                                    double originalSampleRate,
                                    float speedFactor) {
	stop();

	_waveformData = data;
	_originalSampleRate = originalSampleRate;
	_speedFactor = speedFactor;
	_audioSamples.clear();
	_audioDurationMs = 0;
	_dataDurationSec = 0.0;

	if ( _waveformData.empty() || _originalSampleRate <= 0.0 ) {
		return;
	}

	_dataDurationSec = static_cast<double>(_waveformData.size()) / _originalSampleRate;

	double mean = 0.0;
	for ( double val : _waveformData ) {
		mean += val;
	}
	mean /= static_cast<double>(_waveformData.size());

	double maxVal = 0.0;
	for ( double val : _waveformData ) {
		double absVal = std::abs(val - mean);
		if ( absVal > maxVal ) maxVal = absVal;
	}

	if ( maxVal <= 0.0 ) {
		return;
	}

	int dataSize = static_cast<int>(_waveformData.size());
	double effectiveRate = _originalSampleRate * static_cast<double>(_speedFactor);
	double sampleRatio = effectiveRate / static_cast<double>(AUDIO_OUTPUT_RATE);

	int totalAudioSamples = static_cast<int>(
		static_cast<double>(dataSize) / sampleRatio
	);

	_audioSamples.reserve(totalAudioSamples);

	double currentPos = 0.0;
	for ( int i = 0; i < totalAudioSamples; ++i ) {
		if ( currentPos >= static_cast<double>(dataSize - 1) ) {
			_audioSamples.push_back(0.0f);
			currentPos += sampleRatio;
			continue;
		}

		int idx0 = static_cast<int>(currentPos);
		int idx1 = idx0 + 1;
		if ( idx1 >= dataSize ) idx1 = dataSize - 1;

		double frac = currentPos - static_cast<double>(idx0);
		double sample = (_waveformData[idx0] - mean) * (1.0 - frac)
		              + (_waveformData[idx1] - mean) * frac;

		sample /= maxVal;

		if ( sample > 1.0 ) sample = 1.0;
		if ( sample < -1.0 ) sample = -1.0;

		_audioSamples.push_back(static_cast<float>(sample));

		currentPos += sampleRatio;
	}

	_audioDurationMs = static_cast<int>(
		(static_cast<double>(_audioSamples.size()) /
		 static_cast<double>(AUDIO_OUTPUT_RATE)) * 1000.0
	);
}

bool WaveformAudio::isPlaying() const {
	return _playing;
}

bool WaveformAudio::isEnabled() const {
	return _enabled;
}

void WaveformAudio::setEnabled(bool enabled) {
	if ( _enabled != enabled ) {
		_enabled = enabled;
		if ( !_enabled ) {
			stop();
		}
	}
}

float WaveformAudio::speedFactor() const {
	return _speedFactor;
}

void WaveformAudio::setSpeedFactor(float factor) {
	if ( factor > 0.0f ) {
		_speedFactor = factor;
	}
}

int WaveformAudio::audioDurationMs() const {
	return _audioDurationMs;
}

double WaveformAudio::dataDurationSec() const {
	return _dataDurationSec;
}

void WaveformAudio::play() {
	if ( !_enabled || _audioSamples.empty() || _playing ) {
		return;
	}

	QTemporaryFile *tmpFile = new QTemporaryFile(
		QStringLiteral("/tmp/seiscomp_audio_XXXXXX.wav"), this
	);
	tmpFile->setAutoRemove(true);

	if ( !tmpFile->open() ) {
		delete tmpFile;
		emit playbackError(QStringLiteral("Cannot create temporary file for audio"));
		return;
	}

	QString filePath = tmpFile->fileName();
	tmpFile->close();

	if ( !generateWavFile(filePath) ) {
		emit playbackError(QStringLiteral("Cannot write audio data to file"));
		return;
	}

	_process = new QProcess(this);
	connect(_process,
	        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
	        this, &WaveformAudio::onProcessFinished);
	connect(_process, &QProcess::errorOccurred,
	        this, &WaveformAudio::onProcessErrorOccurred);

	QStringList args;
	args << filePath;

	QString program = QStringLiteral("aplay");
	_process->start(program, args);

	if ( _process->waitForStarted(3000) ) {
		_playing = true;
		emit playbackStarted();
	}
	else {
		delete _process;
		_process = nullptr;

		program = QStringLiteral("paplay");
		_process = new QProcess(this);
		connect(_process,
		        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		        this, &WaveformAudio::onProcessFinished);
		connect(_process, &QProcess::errorOccurred,
		        this, &WaveformAudio::onProcessErrorOccurred);

		_process->start(program, args);

		if ( _process->waitForStarted(3000) ) {
			_playing = true;
			emit playbackStarted();
		}
		else {
			delete _process;
			_process = nullptr;
			emit playbackError(QStringLiteral("Cannot start audio player (aplay or paplay)"));
		}
	}
}

void WaveformAudio::stop() {
	if ( _process ) {
		_process->kill();
		_process->waitForFinished(1000);
		delete _process;
		_process = nullptr;
	}
	_playing = false;
}

bool WaveformAudio::generateWavFile(const QString &filePath) {
	QFile file(filePath);
	if ( !file.open(QIODevice::WriteOnly) ) {
		return false;
	}

	int numSamples = static_cast<int>(_audioSamples.size());
	int dataSize = numSamples * 2;

	if ( !writeWavHeader(file, dataSize) ) {
		return false;
	}

	QByteArray buffer;
	buffer.reserve(dataSize);
	for ( int i = 0; i < numSamples; ++i ) {
		float sample = _audioSamples[i];
		if ( sample > 1.0f ) sample = 1.0f;
		if ( sample < -1.0f ) sample = -1.0f;
		int16_t pcm = static_cast<int16_t>(sample * 32767.0f);
		buffer.append(static_cast<char>(pcm & 0xFF));
		buffer.append(static_cast<char>((pcm >> 8) & 0xFF));
	}

	file.write(buffer);
	file.close();
	return true;
}

bool WaveformAudio::writeWavHeader(QIODevice &device, int dataSize) {
	struct WavHeader {
		char     riffId[4];
		uint32_t fileSize;
		char     waveId[4];
		char     fmtId[4];
		uint32_t fmtSize;
		uint16_t audioFormat;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
		char     dataId[4];
		uint32_t dataSize;
	} header;

	std::memcpy(header.riffId, "RIFF", 4);
	header.fileSize = 36 + dataSize;
	std::memcpy(header.waveId, "WAVE", 4);
	std::memcpy(header.fmtId, "fmt ", 4);
	header.fmtSize = 16;
	header.audioFormat = 1;
	header.numChannels = 1;
	header.sampleRate = AUDIO_OUTPUT_RATE;
	header.byteRate = AUDIO_OUTPUT_RATE * 2;
	header.blockAlign = 2;
	header.bitsPerSample = 16;
	std::memcpy(header.dataId, "data", 4);
	header.dataSize = dataSize;

	device.write(reinterpret_cast<const char*>(&header), sizeof(header));
	return true;
}

void WaveformAudio::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
	Q_UNUSED(exitCode)
	Q_UNUSED(status)

	if ( _process ) {
		delete _process;
		_process = nullptr;
	}

	_playing = false;
	emit playbackFinished();
}

void WaveformAudio::onProcessErrorOccurred(QProcess::ProcessError error) {
	if ( _process && error == QProcess::FailedToStart ) {
		QString err = _process->errorString();
		delete _process;
		_process = nullptr;
		_playing = false;
		emit playbackError(err);
	}
}


}
}
