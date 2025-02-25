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


#ifndef SEISCOMP_DATAMODEL_VERSION_H__
#define SEISCOMP_DATAMODEL_VERSION_H__


#define SEISCOMP_DATAMODEL_XMLNS_ROOT "http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/"
#define SEISCOMP_DATAMODEL_XMLNS SEISCOMP_DATAMODEL_XMLNS_ROOT "0.13"

namespace Seiscomp {
namespace DataModel {


struct Version {
	enum {
		Major = 0,
		Minor = 13,
		Revision = 2
	};
};


}
}


#endif
