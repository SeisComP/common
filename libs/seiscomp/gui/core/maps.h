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

#ifndef SEISCOMP_GUI_MAPS_H
#define SEISCOMP_GUI_MAPS_H


#include <seiscomp/gui/qt.h>
#include <QString>


namespace Seiscomp {
namespace Gui {


/**
 * ----------------------------------------------------------------------------
 * TileStore version history
 * ----------------------------------------------------------------------------
 * 1
 *   - Initial version
 * 2
 *   - Allow TileStore::load to return null images
 * 3
 *   - Do not inherit from MapTree and remove MapTreeNode completely
 *   - Add maxLevel query
 *   - Replace TextureID with Tile
 *   - All parameters with type MapTreeNode are replaced by parameters
 *     with type Tile
 * 4 - Add two new methods to notify about asynchronous tile loading
 *     success and cancellation
 */
#define TILESTORE_VERSION 4


struct SC_GUI_API MapsDesc {
	QString location;
	QString type;
	bool    isMercatorProjected;
	size_t  cacheSize;
};



}
}


#endif
