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



#define SEISCOMP_COMPONENT Gui::ImportPicks

#include "importpicks.h"

namespace Seiscomp {

namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImportPicksDialog::Selection ImportPicksDialog::_lastSelection = ImportPicksDialog::LatestOrigin;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImportPicksDialog::ImportPicksDialog(QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags) {
	_ui.setupUi(this);
	QFont f;
	f = _ui.radioLatestOrigin->font(); f.setBold(true); _ui.radioLatestOrigin->setFont(f);
	f = _ui.radioLatestAutomaticOrigin->font(); f.setBold(true); _ui.radioLatestAutomaticOrigin->setFont(f);
	f = _ui.radioMaxPhaseOrigin->font(); f.setBold(true); _ui.radioMaxPhaseOrigin->setFont(f);
	f = _ui.radioAllOrigins->font(); f.setBold(true); _ui.radioAllOrigins->setFont(f);

	switch ( _lastSelection ) {
		case LatestOrigin:
			_ui.radioLatestOrigin->setChecked(true);
			break;
		case LatestAutomaticOrigin:
			_ui.radioLatestAutomaticOrigin->setChecked(true);
			break;
		case MaxPhaseOrigin:
			_ui.radioMaxPhaseOrigin->setChecked(true);
			break;
		case AllOrigins:
			_ui.radioAllOrigins->setChecked(true);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImportPicksDialog::Selection ImportPicksDialog::currentSelection() const {
	if ( _ui.radioLatestOrigin->isChecked() )
		return _lastSelection = LatestOrigin;

	if ( _ui.radioLatestAutomaticOrigin->isChecked() )
		return _lastSelection = LatestAutomaticOrigin;

	if ( _ui.radioMaxPhaseOrigin->isChecked() )
		return _lastSelection = MaxPhaseOrigin;

	if ( _ui.radioAllOrigins->isChecked() )
		return _lastSelection = AllOrigins;

	return _lastSelection = LatestOrigin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImportPicksDialog::importAllPicks() const {
	return _ui.checkAllAgencies->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImportPicksDialog::importAllPhases() const {
	return _ui.checkAllPhases->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImportPicksDialog::preferTargetPhases() const {
	return _ui.checkPreferTargetPhases->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
