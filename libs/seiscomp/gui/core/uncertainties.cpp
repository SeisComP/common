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


#include <seiscomp/gui/core/uncertainties.h>


namespace Seiscomp {
namespace Gui {


EditUncertainties::EditUncertainties(QWidget * parent, Qt::WindowFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	connect(_ui.cbAsymmetric, SIGNAL(toggled(bool)),
	        _ui.labelUpperUncertainty, SLOT(setEnabled(bool)));
	connect(_ui.cbAsymmetric, SIGNAL(toggled(bool)),
	        _ui.spinUpperUncertainty, SLOT(setEnabled(bool)));
	connect(_ui.cbAsymmetric, SIGNAL(toggled(bool)),
	        this, SLOT(symmetryChanged(bool)));

	connect(_ui.spinLowerUncertainty, SIGNAL(valueChanged(double)),
	        this, SLOT(lowerChanged(double)));
	connect(_ui.spinUpperUncertainty, SIGNAL(valueChanged(double)),
	        this, SLOT(upperChanged(double)));
}


EditUncertainties::~EditUncertainties() {
}


void EditUncertainties::lowerChanged(double d) {
	if ( !_ui.cbAsymmetric->isChecked() ) {
		_ui.spinUpperUncertainty->blockSignals(true);
		_ui.spinUpperUncertainty->setValue(d);
		_ui.spinUpperUncertainty->blockSignals(false);
	}

	emit uncertaintiesChanged(lowerUncertainty(), upperUncertainty());
}


void EditUncertainties::upperChanged(double d) {
	emit uncertaintiesChanged(lowerUncertainty(), upperUncertainty());
}

void EditUncertainties::symmetryChanged(bool) {
	emit uncertaintiesChanged(lowerUncertainty(), upperUncertainty());
}


void EditUncertainties::setUncertainties(double lower, double upper) {
	_ui.spinLowerUncertainty->setValue(lower);
	_ui.spinUpperUncertainty->setValue(upper);

	_ui.cbAsymmetric->setChecked(
		_ui.spinLowerUncertainty->value() != _ui.spinUpperUncertainty->value()
	);
}


double EditUncertainties::lowerUncertainty() const {
	return _ui.spinLowerUncertainty->value();
}


double EditUncertainties::upperUncertainty() const {
	if ( !_ui.cbAsymmetric->isChecked() )
		return lowerUncertainty();

	return _ui.spinUpperUncertainty->value();
}


}
}
