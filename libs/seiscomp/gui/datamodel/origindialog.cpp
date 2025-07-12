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

#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/datamodel/origindialog.h>
#include <iostream>
#include <QTime>
#include <QDate>

#include <seiscomp/core/datetime.h>


namespace Seiscomp {
namespace Gui {


double OriginDialog::_defaultDepth = 10;


void OriginDialog::SetDefaultDepth(double depth) {
	_defaultDepth = depth;
}


double OriginDialog::DefaultDepth() {
	return _defaultDepth;
}


OriginDialog::OriginDialog(QWidget * parent, Qt::WindowFlags f) :
 QDialog(parent, f) {
	init(0.0, 0.0, _defaultDepth);
}


OriginDialog::OriginDialog(double lon, double lat,
                           QWidget * parent, Qt::WindowFlags f) :
 QDialog(parent, f) {
	init(lon, lat, _defaultDepth);
}


OriginDialog::OriginDialog(double lon, double lat, double dep,
                           QWidget* parent, Qt::WindowFlags f) :
 QDialog(parent, f) {
	init(lon, lat, dep);
}


OriginDialog::~OriginDialog() {}


time_t OriginDialog::getTime_t() const {
	Seiscomp::Core::Time t;
	t.set(_ui.dateTimeEdit->dateTime().date().year(),
	      _ui.dateTimeEdit->dateTime().date().month(),
	      _ui.dateTimeEdit->dateTime().date().day(),
	      _ui.dateTimeEdit->dateTime().time().hour(),
	      _ui.dateTimeEdit->dateTime().time().minute(),
	      _ui.dateTimeEdit->dateTime().time().second(),
	      0);

	if ( SCScheme.dateTime.useLocalTime )
		t = t.toUTC();

	return t.epochSeconds();
}


void OriginDialog::setTime(Core::Time t) {
	int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

	if ( SCScheme.dateTime.useLocalTime )
		t = t.toLocalTime();

	t.get(&y, &M, &d, &h, &m, &s);
	_ui.dateTimeEdit->setTime(QTime(h, m, s));
	_ui.dateTimeEdit->setDate(QDate(y, M, d));
}


double OriginDialog::latitude() const {
	return _ui.latLineEdit->text().toDouble();
}


void OriginDialog::setLatitude(double lat) {
	_ui.latLineEdit->setText(QString::number(lat));
}


double OriginDialog::longitude() const {
	return _ui.lonLineEdit->text().toDouble();
}


void OriginDialog::setLongitude(double lon) {
	_ui.lonLineEdit->setText(QString::number(lon));
}


double OriginDialog::depth() const {
	return _ui.depthLineEdit->text().toDouble();
}


void OriginDialog::setDepth(double dep) {
	_ui.depthLineEdit->setText(QString::number(dep));
}


void OriginDialog::enableAdvancedOptions(bool enable, bool checkable) {
	if ( enable ) {
		_ui.advancedGroupBox->show();
		_ui.advancedGroupBox->setCheckable(checkable);
	}
	else
		_ui.advancedGroupBox->hide();
}


bool OriginDialog::advanced() const {
	return _ui.advancedGroupBox->isChecked();
}


void OriginDialog::setAdvanced(bool checked) {
	_ui.advancedGroupBox->setChecked(checked);
}


int OriginDialog::phaseCount() const {
	return _ui.phaseCountLineEdit->text().toInt();
}


void OriginDialog::setPhaseCount(int count) {
	_ui.phaseCountLineEdit->setText(QString::number(count));
}


double OriginDialog::magValue() const {
	return _ui.magLineEdit->text().toDouble();
}


void OriginDialog::setMagValue(double mag) {
	_ui.magLineEdit->setText(QString::number(mag));
}


QString OriginDialog::magType() const {
	return _ui.magTypeComboBox->currentText().trimmed();
}


void OriginDialog::setMagType(const QString &type) {
	int index = _magTypes.indexOf(type);
	if ( index < 0 ) {
		index = 0;
		_ui.magTypeComboBox->addItem(type);
	}
	_ui.magTypeComboBox->setCurrentIndex(index);
}


void OriginDialog::setMagTypes(const QStringList &types) {
	QString type = magType();
	_magTypes = types;

	_ui.magTypeComboBox->clear();
	_ui.magTypeComboBox->addItems(_magTypes);
	if ( !type.isEmpty() ) {
		setMagType(type);
	}
}


void OriginDialog::setSendButtonText(const QString &text) {
	_ui.sendButton->setText(text);
}


void OriginDialog::loadSettings(const QString &groupName) {
	QSettings &s = SCApp->settings();
	s.beginGroup(groupName);
	setLongitude(s.value("longitude", longitude()).toDouble());
	setLatitude(s.value("latitude", latitude()).toDouble());
	setDepth(s.value("depth", depth()).toDouble());
	setAdvanced(s.value("advanced", advanced()).toBool());
	setPhaseCount(s.value("phaseCount", phaseCount()).toInt());
	setMagValue(s.value("magValue", magValue()).toDouble());
	setMagType(s.value("magType", magType()).toString());
	s.endGroup();
}


void OriginDialog::saveSettings(const QString &groupName) {
	QSettings &s = SCApp->settings();
	s.beginGroup(groupName);
	s.setValue("longitude", longitude());
	s.setValue("latitude", latitude());
	s.setValue("depth", depth());
	s.setValue("advanced", advanced());
	if ( advanced() ) {
		s.setValue("phaseCount", phaseCount());
		s.setValue("magValue", magValue());
		s.setValue("magType", magType());
	}
	s.endGroup();
}


void OriginDialog::onTextEdited(const QString &text) {
	auto lineEdit = qobject_cast<QLineEdit*>(sender());
	if ( !lineEdit || !lineEdit->validator() ) {
		return;
	}

	auto validator = qobject_cast<const QDoubleValidator*>(lineEdit->validator());
	if ( !validator ) {
		return;
	}

	bool ok;
	double value = text.toDouble(&ok);
	if ( !ok ) {
		return;
	}

	if ( value < validator->bottom() ) {
		lineEdit->setText(QString::number(validator->bottom()));
	}
	else if ( value > validator->top() ) {
		lineEdit->setText(QString::number(validator->top()));
	}
}


void OriginDialog::init(double lon, double lat, double dep) {
	_ui.setupUi(this);
	_ui.advancedGroupBox->hide();

	if ( SCScheme.dateTime.useLocalTime )
		_ui.dateTimeEdit->setDisplayFormat(_ui.dateTimeEdit->displayFormat() + " " + Core::Time::LocalTimeZone().c_str());
	else
		_ui.dateTimeEdit->setDisplayFormat(_ui.dateTimeEdit->displayFormat() + " UTC");

	if ( SCScheme.precision.originTime > 0 ) {
		// no evaluation of specific origin time precision because millisecond
		// spinner controls only work with 3 digits
		_ui.dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss.zzz");
	}

	QDoubleValidator *val = new QDoubleValidator(-90, 90, 10, _ui.latLineEdit);
	val->setNotation(QDoubleValidator::StandardNotation);
	_ui.latLineEdit->setValidator(val);

	val = new QDoubleValidator(-180, 180, 10, _ui.lonLineEdit);
	val->setNotation(QDoubleValidator::StandardNotation);
	_ui.lonLineEdit->setValidator(val);

	_ui.depthLineEdit->setValidator(new QDoubleValidator(_ui.depthLineEdit));
	_ui.phaseCountLineEdit->setValidator(new QIntValidator(0, INT_MAX, _ui.phaseCountLineEdit));
	_ui.magLineEdit->setValidator(new QDoubleValidator(_ui.magLineEdit));

	connect(_ui.latLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
	connect(_ui.lonLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));

	setTime(Core::Time::UTC());
	setLongitude(lon);
	setLatitude(lat);
	setDepth(dep);

	setAdvanced(false);
	setPhaseCount(10);
	setMagValue(5.0);
}


} // namesapce Gui
} // namespace Seiscomp
