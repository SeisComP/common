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


#ifndef SEISCOMP_GUI_IMPORTPICKS_H
#define SEISCOMP_GUI_IMPORTPICKS_H


#include <seiscomp/utils/stringfirewall.h>
#include <seiscomp/gui/qt.h>

#include <QDialog>
#include <set>
#include <string>


namespace Seiscomp::Gui {


class ImportPicksPrivate;


class SC_GUI_API ImportPicksDialog : public QDialog {
	Q_OBJECT

	public:
		enum Selection {
			LatestOrigin,
			LatestAutomaticOrigin,
			MaxPhaseOrigin,
			AllOrigins
		};

		enum CBSelection {
			CBUndefined = -1,
			CBNone = 0,
			CBImportAllPicks = 1 << 0,
			CBImportAllPhases = 1 << 1,
			CBPreferTargetPhases = 1 << 2,
		};


	public:
		ImportPicksDialog(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());


	public:
		static void setDefaultAcceptedPhases(QString phases);
		static void setDefaultSelection(Selection sel);
		static void setDefaultOptions(int options);

		void accept() override;

		Selection currentSelection() const;
		bool importAllPicks() const;
		bool importAllPhases() const;
		bool preferTargetPhases() const;
		Util::StringFirewall allowedPhases() const;


	private:
		int currentCBSelection() const;

	private:
		ImportPicksPrivate *_d_ptr;
		static Selection _lastSelection;
		static int _lastCBSelection;
		static QString _lastPhases;
};


}


#endif
