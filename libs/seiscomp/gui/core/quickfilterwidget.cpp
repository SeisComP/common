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


#include <seiscomp/gui/core/quickfilterwidget.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTimer>


namespace Seiscomp {
namespace Gui {


namespace {


enum FilterType {
	Bandpass,
	Highpass,
	Lowpass,
	Bandstop
};


const char *filterKeys[] = { "BW", "BW_HP", "BW_LP", "BW_BS" };


QDoubleSpinBox *createFrequencySpinBox(double value, const QString &prefix) {
	QDoubleSpinBox *spin = new QDoubleSpinBox;
	spin->setRange(0.001, 10000);
	spin->setDecimals(3);
	spin->setValue(value);
	spin->setPrefix(prefix);
	spin->setSuffix(" Hz");
	spin->setKeyboardTracking(false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	spin->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#else
	spin->setSingleStep(0.1);
#endif
	return spin;
}


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QuickFilterWidget::QuickFilterWidget(QWidget *parent)
: QWidget(parent) {
	_timer = new QTimer(this);
	_timer->setSingleShot(true);
	_timer->setInterval(250);
	connect(_timer, SIGNAL(timeout()), this, SLOT(emitFilter()));

	_type = new QComboBox;
	_type->addItem(tr("Bandpass"));
	_type->addItem(tr("Highpass"));
	_type->addItem(tr("Lowpass"));
	_type->addItem(tr("Bandstop"));
	_type->setToolTip(tr("Butterworth filter type"));

	_order = new QSpinBox;
	_order->setRange(1, 10);
	_order->setValue(3);
	_order->setPrefix(tr("Order "));
	_order->setKeyboardTracking(false);
	_order->setToolTip(tr("Filter order"));

	_fmin = createFrequencySpinBox(1.0, tr("Low "));
	_fmin->setToolTip(tr("Lower corner frequency"));
	_fmax = createFrequencySpinBox(8.0, tr("High "));
	_fmax->setToolTip(tr("Upper corner frequency"));

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(_type);
	layout->addWidget(_order);
	layout->addWidget(_fmin);
	layout->addWidget(_fmax);
	setLayout(layout);

	connect(_type, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged()));
	connect(_order, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
	connect(_fmin, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
	connect(_fmax, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));

	typeChanged();
	_timer->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString QuickFilterWidget::filter() const {
	int type = _type->currentIndex();
	QString order = QString::number(_order->value());
	QString fmin = QString::number(_fmin->value(), 'g', 10);
	QString fmax = QString::number(_fmax->value(), 'g', 10);

	switch ( type ) {
		case Highpass:
			return QString("%1(%2,%3)").arg(filterKeys[type], order, fmin);
		case Lowpass:
			return QString("%1(%2,%3)").arg(filterKeys[type], order, fmax);
		default:
			return QString("%1(%2,%3,%4)").arg(filterKeys[type], order, fmin, fmax);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QuickFilterWidget::setFilter(const QString &filter) {
	static const QRegularExpression re(
		"^\\s*(BW|BW_BP|BW_HP|BW_LP|BW_BS)\\s*\\(([^()]*)\\)\\s*$"
	);

	QRegularExpressionMatch match = re.match(filter);
	if ( !match.hasMatch() ) {
		return false;
	}

	QString key = match.captured(1);
	if ( key == "BW_BP" ) {
		key = "BW";
	}

	int type = -1;
	for ( int i = 0; i < 4; ++i ) {
		if ( key == filterKeys[i] ) {
			type = i;
		}
	}

	QStringList args = match.captured(2).split(',');
	int expected = (type == Highpass || type == Lowpass) ? 2 : 3;
	if ( args.size() != expected ) {
		return false;
	}

	QVector<double> values;
	for ( const QString &arg : args ) {
		bool ok;
		values.append(arg.trimmed().toDouble(&ok));
		if ( !ok ) {
			return false;
		}
	}

	blockSignals(true);
	_type->setCurrentIndex(type);
	_order->setValue(static_cast<int>(values[0]));
	if ( type == Highpass ) {
		_fmin->setValue(values[1]);
	}
	else if ( type == Lowpass ) {
		_fmax->setValue(values[1]);
	}
	else {
		_fmin->setValue(values[1]);
		_fmax->setValue(values[2]);
	}
	typeChanged();
	_timer->stop();
	blockSignals(false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QuickFilterWidget::isValid() const {
	int type = _type->currentIndex();
	if ( type == Highpass || type == Lowpass ) {
		return true;
	}

	return _fmin->value() < _fmax->value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuickFilterWidget::typeChanged() {
	int type = _type->currentIndex();
	_fmin->setVisible(type != Lowpass);
	_fmax->setVisible(type != Highpass);
	valueChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuickFilterWidget::valueChanged() {
	bool valid = isValid();
	QString tip = valid ? QString() : tr("The low corner must be below the high corner");
	QPalette pal = palette();
	if ( !valid ) {
		pal.setColor(QPalette::Text, Qt::red);
	}
	_fmin->setPalette(pal);
	_fmax->setPalette(pal);
	setToolTip(tip.isEmpty() ? filter() : tip);

	if ( valid ) {
		_timer->start();
	}
	else {
		_timer->stop();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuickFilterWidget::emitFilter() {
	if ( isValid() ) {
		emit filterChanged(filter());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
