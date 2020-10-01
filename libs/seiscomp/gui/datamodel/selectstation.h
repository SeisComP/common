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


#ifndef SEISCOMP_SELECTSTATION_H
#define SEISCOMP_SELECTSTATION_H


#include <QDialog>
#include <QAbstractTableModel>

#ifndef Q_MOC_RUN
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/station.h>
#endif
#include <seiscomp/gui/qt.h>

#include <seiscomp/gui/datamodel/ui_selectstation.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SelectStation : public QDialog {
	Q_OBJECT

	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		explicit SelectStation(Core::Time time, bool ignoreDisabledStations,
		                       QWidget* parent = 0, Qt::WindowFlags f = 0);
		explicit SelectStation(Core::Time time, bool ignoreDisabledStations,
		                       const QSet<QString> &blackList,
		                       QWidget* parent = 0, Qt::WindowFlags f = 0);
		~SelectStation();

		QList<DataModel::Station*> selectedStations() const;

		void setReferenceLocation(double lat, double lon);


	// ------------------------------------------------------------------
	// Private Interface
	// ------------------------------------------------------------------
	private slots:
		void listMatchingStations(const QString& substr);


	private:
		void init(Core::Time, bool ignoreDisabledStations,
		          const QSet<QString> *blackList);


	// ------------------------------------------------------------------
	// Private data members
	// ------------------------------------------------------------------
	private:
		Ui::SelectStation _ui;
};

} // namespace Gui
} // namespace Seiscomp

#endif
