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


#ifndef SEISCOMP_GUI_ORIGINDIALOG_H
#define SEISCOMP_GUI_ORIGINDIALOG_H


#include <ctime>

#include <QDialog>
#include <QDateTime>
#include <QString>

#include <seiscomp/gui/datamodel/ui_origindialog.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/gui/qt.h>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API OriginDialog : public QDialog
{

	Q_OBJECT

public:
	static void SetDefaultDepth(double depth);
	static double DefaultDepth();

	OriginDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
	OriginDialog(double lon, double lat,
	             QWidget * parent = 0, Qt::WindowFlags f = 0);

	OriginDialog(double lon, double lat, double dep,
	             QWidget * parent = 0, Qt::WindowFlags f = 0);

	~OriginDialog();

	time_t getTime_t() const;
	void setTime(Core::Time t);

	double longitude() const;
	void setLongitude(double lon);

	double latitude() const;
	void setLatitude(double lat);

	double depth() const;
	void setDepth(double dep);

	// enable advanced group box
	void enableAdvancedOptions(bool enable = true, bool checkable = true);

	bool advanced() const;
	void setAdvanced(bool checked);

	int phaseCount() const;
	void setPhaseCount(int count);

	double magValue() const;
	void setMagValue(double mag);

	QString magType() const;
	void setMagType(const QString &type);
	void setMagTypes(const QStringList &types);

	void setSendButtonText(const QString &text);

	void loadSettings(const QString &groupName = "OriginDialog");
	void saveSettings(const QString &groupName = "OriginDialog");

private:
	void init(double lon, double lat, double dep);

private:
	static double _defaultDepth;

	Ui::OriginDialog _ui;
	QStringList      _magTypes;
};


} // namespace Gui
} // namespace Seiscomp

#endif
