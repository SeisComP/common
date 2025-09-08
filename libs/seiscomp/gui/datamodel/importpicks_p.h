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


#ifndef SEISCOMP_GUI_IMPORTPICKS_PRIVATE_H
#define SEISCOMP_GUI_IMPORTPICKS_PRIVATE_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the SeisComP API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "importpicks.h"

#include <seiscomp/gui/datamodel/ui_importpicks.h>


namespace Seiscomp::Gui {


class ImportPicksPrivate {
	private:
		ImportPicksPrivate() {}

		::Ui::ImportPicks ui;

	friend class ImportPicksDialog;
};


} // namespace Seiscomp::Gui


#endif
