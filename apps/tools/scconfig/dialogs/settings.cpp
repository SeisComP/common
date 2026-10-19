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

#include <QDialogButtonBox>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <limits>

#include <seiscomp/config/config.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/utils/files.h>

#include "settings.h"
#include "../fancyview.h"


using namespace Seiscomp;


SettingsDialog::SettingsDialog(QWidget *parent)
: QDialog(parent) {
	setWindowTitle(tr("scconfig configuration"));

	// All levels are opened with the largest integer, which reads -1
	const int expandLevel = FancyView::expandLevelDefault();

	_level = new QSpinBox;
	_level->setRange(-1, 9);
	_level->setValue(expandLevel == std::numeric_limits<int>::max() ?
	                 -1 : expandLevel);
	_level->setToolTip(expandLevelToolTip());

	auto *visibility = new QGroupBox(tr("Visibility"));
	auto *visibilityForm = new QFormLayout(visibility);
	visibilityForm->addRow(tr("Show level:"), _level);

	auto *note = new QLabel(tr("Set the visibility of shown module and binding "
	                           "parameters. Press Accept to apply and to save "
	                           "the decision in @SYSTEMCONFIGDIR@/scconfig.cfg.\n"
	                           "Temporarily set the show level in the "
	                           "Modules or Bindings panel."));
	note->setWordWrap(true);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::RestoreDefaults |
	                                     QDialogButtonBox::Save |
	                                     QDialogButtonBox::Cancel);
	buttons->button(QDialogButtonBox::Save)->setText(tr("Accept"));

	auto *defaults = buttons->button(QDialogButtonBox::RestoreDefaults);
	defaults->setText(tr("Default"));
	defaults->setToolTip(tr("Show the default values, accept to apply them"));
	connect(defaults, &QPushButton::clicked, this, [this] {
		_level->setValue(FancyView::DefaultExpandLevel);
	});
	connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(note);
	layout->addWidget(visibility);
	layout->addWidget(buttons);
}


QString SettingsDialog::expandLevelToolTip() {
	return tr("Nesting level up to which the parameters of a module or binding "
	          "show when loaded initially or after pressing Apply.\n"
	          "-1: Show the parameters on all levels\n"
	          "0: Show no parameter at all\n"
	          "1: Show all ungrouped parameters\n"
	          "2 - 9: Also show the parameters of the groups "
	          "up to that level");
}


void SettingsDialog::accept() {
	const std::string filename = Environment::Instance()->configFileLocation(
		"scconfig", Environment::CS_CONFIG_APP);

	Config::Config cfg;
	if ( Util::fileExists(filename) && !cfg.readConfig(filename) ) {
		QMessageBox::critical(this, tr("Save settings"),
		                      tr("Failed to read %1").arg(filename.c_str()));
		return;
	}

	if ( _level->value() == FancyView::DefaultExpandLevel ) {
		cfg.remove("expandSectionLevel");
	}
	else {
		cfg.setInt("expandSectionLevel", _level->value());
	}

	auto *symbols = cfg.symbolTable();
	if ( symbols->begin() == symbols->end() ) {
		if ( Util::fileExists(filename) && !QFile::remove(filename.c_str()) ) {
			QMessageBox::critical(this, tr("Save settings"),
			                      tr("Failed to remove %1").arg(filename.c_str()));
			return;
		}
	}
	else if ( !cfg.writeConfig(filename, false) ) {
		QMessageBox::critical(this, tr("Save settings"),
		                      tr("Failed to write %1").arg(filename.c_str()));
		return;
	}

	FancyView::setDefaultExpandLevel(_level->value());

	QDialog::accept();
}
