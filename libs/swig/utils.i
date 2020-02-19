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

%module(package="seiscomp") utils

%{
#include "seiscomp/utils/files.h"
#include "seiscomp/utils/timer.h"
#include "seiscomp/utils/units.h"
%}

%include stl.i
%include "seiscomp/core.h"
%include "seiscomp/utils/files.h"
%include "seiscomp/utils/timer.h"
%include "seiscomp/utils/units.h"

%newobject Seiscomp::Util::stringToStreambuf;
%newobject Seiscomp::Util::file2ostream;
%newobject Seiscomp::Util::file2istream;
