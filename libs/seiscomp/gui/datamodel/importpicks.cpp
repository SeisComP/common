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

#include "importpicks_p.h"


#define SC_D (*_d_ptr)


namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImportPicksDialog::Selection ImportPicksDialog::_lastSelection = ImportPicksDialog::LatestOrigin;
int ImportPicksDialog::_lastCBSelection = ImportPicksDialog::CBUndefined;
QString ImportPicksDialog::_lastPhases;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImportPicksDialog::ImportPicksDialog(QWidget * parent, Qt::WindowFlags flags)
 : QDialog(parent, flags)
 , _d_ptr(new ImportPicksPrivate()) {
	SC_D.ui.setupUi(this);
	QFont f;
	f = SC_D.ui.radioLatestOrigin->font(); f.setBold(true); SC_D.ui.radioLatestOrigin->setFont(f);
	f = SC_D.ui.radioLatestAutomaticOrigin->font(); f.setBold(true); SC_D.ui.radioLatestAutomaticOrigin->setFont(f);
	f = SC_D.ui.radioMaxPhaseOrigin->font(); f.setBold(true); SC_D.ui.radioMaxPhaseOrigin->setFont(f);
	f = SC_D.ui.radioAllOrigins->font(); f.setBold(true); SC_D.ui.radioAllOrigins->setFont(f);

	switch ( _lastSelection ) {
		case LatestOrigin:
			SC_D.ui.radioLatestOrigin->setChecked(true);
			break;
		case LatestAutomaticOrigin:
			SC_D.ui.radioLatestAutomaticOrigin->setChecked(true);
			break;
		case MaxPhaseOrigin:
			SC_D.ui.radioMaxPhaseOrigin->setChecked(true);
			break;
		case AllOrigins:
			SC_D.ui.radioAllOrigins->setChecked(true);
			break;
	}

	if ( _lastCBSelection == CBUndefined ) {
		_lastCBSelection = currentCBSelection();
	}
	else {
		SC_D.ui.checkAllAgencies->setChecked(_lastCBSelection & CBImportAllPicks);
		SC_D.ui.checkAllPhases->setChecked(_lastCBSelection & CBImportAllPhases);
		SC_D.ui.checkPreferTargetPhases->setChecked(_lastCBSelection & CBPreferTargetPhases);
	}

	SC_D.ui.lineEditAcceptedPhases->setText(_lastPhases);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportPicksDialog::setDefaultAcceptedPhases(QString phases) {
	_lastPhases = phases;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportPicksDialog::setDefaultSelection(Selection sel) {
	_lastSelection = sel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportPicksDialog::setDefaultOptions(int options) {
	_lastCBSelection = options;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportPicksDialog::accept() {
	// Remember the last settings
	_lastPhases = SC_D.ui.lineEditAcceptedPhases->text();
	_lastCBSelection = currentCBSelection();
	_lastSelection = currentSelection();
	QDialog::accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImportPicksDialog::Selection ImportPicksDialog::currentSelection() const {
	if ( SC_D.ui.radioLatestOrigin->isChecked() ) {
		return LatestOrigin;
	}

	if ( SC_D.ui.radioLatestAutomaticOrigin->isChecked() ) {
		return LatestAutomaticOrigin;
	}

	if ( SC_D.ui.radioMaxPhaseOrigin->isChecked() ) {
		return MaxPhaseOrigin;
	}

	if ( SC_D.ui.radioAllOrigins->isChecked() ) {
		return AllOrigins;
	}

	return LatestOrigin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImportPicksDialog::importAllPicks() const {
	return SC_D.ui.checkAllAgencies->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImportPicksDialog::importAllPhases() const {
	return SC_D.ui.checkAllPhases->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ImportPicksDialog::preferTargetPhases() const {
	return SC_D.ui.checkPreferTargetPhases->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Util::StringFirewall ImportPicksDialog::allowedPhases() const {
	Util::StringFirewall phases;
	QStringList editPhases = SC_D.ui.lineEditAcceptedPhases->text().split(",");
	for ( auto &ph : editPhases ) {
		ph = ph.trimmed();
		if ( ph.isEmpty() ) {
			continue;
		}
		if ( ph[0] == '-' ) {
			phases.deny.insert(ph.mid(1).toStdString());
		}
		else if ( ph[1] == '+' ) {
			phases.allow.insert(ph.mid(1).toStdString());
		}
		else {
			phases.allow.insert(ph.toStdString());
		}
	}

	return phases;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ImportPicksDialog::currentCBSelection() const {
	int cbSelection = CBNone;
	if ( importAllPicks() ) {
		cbSelection |= CBImportAllPicks;
	}
	if ( importAllPhases() ) {
		cbSelection |= CBImportAllPhases;
	}
	if ( preferTargetPhases() ) {
		cbSelection |= CBPreferTargetPhases;
	}
	return cbSelection;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
