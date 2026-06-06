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


#ifndef SEISCOMP_GUI_WAVEFORMAUDIO_H
#define SEISCOMP_GUI_WAVEFORMAUDIO_H


#include <QObject>
#include <QProcess>
#include <vector>

#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API WaveformAudio : public QObject {
	Q_OBJECT

	public:
		WaveformAudio(QObject *parent = nullptr);
		~WaveformAudio();

	public:
		void setWaveformData(const std::vector<double> &data,
		                     double originalSampleRate,
		                     float speedFactor = 160.0f);

		bool isPlaying() const;
		bool isEnabled() const;
		void setEnabled(bool enabled);

		float speedFactor() const;
		void setSpeedFactor(float factor);

		int audioDurationMs() const;
		double dataDurationSec() const;

	public slots:
		void play();
		void stop();

	signals:
		void playbackStarted();
		void playbackFinished();
		void playbackError(const QString &error);

	private:
		bool generateWavFile(const QString &filePath);
		bool writeWavHeader(QIODevice &device, int dataSize);

	private slots:
		void onProcessFinished(int exitCode, QProcess::ExitStatus status);
		void onProcessErrorOccurred(QProcess::ProcessError error);

	private:
		std::vector<double>  _waveformData;
		std::vector<float>   _audioSamples;
		double               _originalSampleRate{0.0};
		float                _speedFactor{160.0f};
		int                  _audioDurationMs{0};
		double               _dataDurationSec{0.0};
		bool                 _enabled{false};
		bool                 _playing{false};
		QProcess            *_process{nullptr};

		static constexpr int AUDIO_OUTPUT_RATE = 44100;
};


}
}


#endif
