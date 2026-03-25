/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/


#include <seiscomp/gui/core/application.h>

#include "spectrogramsettings.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpectrogramSettings::SpectrogramSettings(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f) {
	ui.setupUi(this);
	connect(ui.btnApply, SIGNAL(clicked()), this, SIGNAL(apply()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramSettings::init(const System::Application *app, const std::string &prefix) {
	try {
		ui.cbSmoothing->setChecked(app->configGetBool(prefix + "smoothing"));
	}
	catch ( ... ) {}

	try {
		ui.cbLogScale->setChecked(app->configGetBool(prefix + "logScale"));
	}
	catch ( ... ) {}

	try {
		auto mode = app->configGetString(prefix + "normalization");
		if ( mode == "fixed" ) {
			ui.cbNormalization->setCurrentIndex(0);
		}
		else if ( mode == "frequency" ) {
			ui.cbNormalization->setCurrentIndex(1);
		}
		else if ( mode == "time" ) {
			ui.cbNormalization->setCurrentIndex(2);
		}
	}
	catch ( ... ) {}

	try {
		ui.cbShowAxis->setChecked(app->configGetBool(prefix + "axis"));
	}
	catch ( ... ) {}

	try {
		ui.spinMinAmp->setValue(app->configGetDouble(prefix + "minimumAmplitude"));
	}
	catch ( ... ) {}

	try {
		ui.spinMaxAmp->setValue(app->configGetDouble(prefix + "maximumAmplitude"));
	}
	catch ( ... ) {}

	try {
		ui.spinMinFrequency->setValue(app->configGetDouble(prefix + "minimumFrequency"));
	}
	catch ( ... ) {}

	try {
		ui.spinMaxFrequency->setValue(app->configGetDouble(prefix + "maximumFrequency"));
	}
	catch ( ... ) {}

	try {
		ui.spinTimeWindow->setValue(app->configGetDouble(prefix + "timeSpan"));
	}
	catch ( ... ) {}

	try {
		ui.spinOverlap->setValue(app->configGetDouble(prefix + "overlap") * 100);
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
