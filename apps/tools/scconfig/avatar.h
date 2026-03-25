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
 * ----------------------------------------------------------------------- *
 *                                                                         *
 * This is a port of the Multiavatar code to C++:                          *
 *                                                                         *
 * URL: https://multiavatar.com                                            *
 * GitHub: https://github.com/multiavatar/Multiavatar                      *
 * Copyright: Gie Katon (2020-2021) (https://giekaton.com)                 *
 ***************************************************************************/


#ifndef SEISCOMP_CONFIGURATION_AVATAR_H
#define SEISCOMP_CONFIGURATION_AVATAR_H


#include <string>


std::string multiavatar(std::string something = {}, bool withoutEnv = false);


#endif
