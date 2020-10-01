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



#ifndef SEISCOMP_GUI_WAVEFORMSETTINGS_H
#define SEISCOMP_GUI_WAVEFORMSETTINGS_H

#include <seiscomp/gui/datamodel/ui_pickersettings.h>
#include <seiscomp/gui/datamodel/pickerview.h>
#include <seiscomp/gui/datamodel/amplitudeview.h>
#include <seiscomp/gui/datamodel/originlocatorview.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API PickerSettings : public QDialog {
	Q_OBJECT

	public:
		PickerSettings(const OriginLocatorView::Config &c1,
		               const PickerView::Config &c2,
		               const AmplitudeView::Config &c3,
		               QWidget *parent = 0, Qt::WindowFlags f = 0);
		~PickerSettings();


	public:
		const Ui::PickerSettings &ui() const { return _ui; }

		OriginLocatorView::Config locatorConfig() const;
		PickerView::Config pickerConfig() const;
		AmplitudeView::Config amplitudeConfig() const;

		void setSaveEnabled(bool);

		bool saveSettings() const;

		int exec();


	signals:
		void saveRequested();


	private slots:
		void save();

		void adjustPreTime(int);
		void adjustPostTime(int);
		void adjustLength(int);

		void adjustPreSlider(const QTime&);
		void adjustPostSlider(const QTime&);
		void adjustLengthSlider(const QTime&);

		void adjustAmplitudePreTime(int);
		void adjustAmplitudePostTime(int);

		void adjustAmplitudePreSlider(const QTime&);
		void adjustAmplitudePostSlider(const QTime&);

		void addPickFilter();
		void removePickFilter();

		void movePickFilterUp();
		void movePickFilterDown();

		void addAmplitudeFilter();
		void removeAmplitudeFilter();

		void moveAmplitudeFilterUp();
		void moveAmplitudeFilterDown();


	private:
		Ui::PickerSettings                _ui;
		bool                              _saveSettings;
		QAbstractListModel               *_pickerFilterModel;
		QAbstractListModel               *_amplitudeFilterModel;
		mutable OriginLocatorView::Config _locatorConfig;
		mutable PickerView::Config        _pickerConfig;
		mutable AmplitudeView::Config     _amplitudeConfig;
};


}
}


#endif
