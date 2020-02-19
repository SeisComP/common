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


#include <seiscomp/gui/core/optionaldoublespinbox.h>

#include <QToolButton>
#include <QVBoxLayout>
#include <QStyle>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OptionalDoubleSpinBox::OptionalDoubleSpinBox(QWidget *parent)
: QDoubleSpinBox(parent) {
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	int horizontalSpacing = style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
	if ( horizontalSpacing < 0 )
		horizontalSpacing = 6;

	int fontSize = fontInfo().pixelSize();

	_spacing = frameWidth + horizontalSpacing;

	setSpecialValueText(tr("Unset"));
	setStyleSheet(QString("QDoubleSpinBox { padding-left: %1px }")
	              .arg(fontSize+_spacing + fontMetrics().boundingRect(' ').width()));

	_resetButton = new QToolButton(this);
	_resetButton->setObjectName("reset");
	QFont f = _resetButton->font();
	f.setBold(true);
	_resetButton->setFont(f);
	_resetButton->setPopupMode(QToolButton::InstantPopup);

	connect(this, SIGNAL(valueChanged(double)), this, SLOT(changedContent()));
	connect(_resetButton, SIGNAL(clicked()), this, SLOT(resetContent()));

	QString styleSheet = QString("QToolButton#reset { border: none; padding: 0; margin: 0; padding-bottom: 0.5px; margin-left: %1px; min-height: 0 }"
	                             "QToolButton#reset::menu-indicator { image: url(:/blank); }")
	                     .arg(_spacing);
	_resetButton->setStyleSheet(styleSheet);

	QVBoxLayout *vl = new QVBoxLayout;
	vl->setMargin(0);
	vl->addStretch();
	vl->addWidget(_resetButton);
	vl->addStretch();

	setLayout(vl);

	changedContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OptionalDoubleSpinBox::isValid() const {
	return value() != minimum();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OptionalDoubleSpinBox::resetContent() {
	setValue(minimum());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OptionalDoubleSpinBox::changedContent() {
	if ( value() == minimum() ) {
		_resetButton->setEnabled(false);
		_resetButton->setText("X");
		_resetButton->setToolTip(tr("Value is unset"));
	}
	else {
		_resetButton->setEnabled(true);
		_resetButton->setText("X");
		_resetButton->setToolTip(tr("Unset value"));
	}

	QRect iconRect = _resetButton->fontMetrics().boundingRect(_resetButton->text());
	_resetButton->setFixedWidth(iconRect.width() + _spacing);
	_resetButton->setFixedHeight(iconRect.height() + 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
