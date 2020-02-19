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


#include <seiscomp/core/rtti.h>


namespace Seiscomp {
namespace Core {


RTTI::RTTI(const char* classname, const RTTI* parent)
	: _classname(classname), _parent(parent) {
}


const RTTI* RTTI::parent() const {
	return _parent;
}


const char* RTTI::className() const {
	return _classname.c_str();
}


bool RTTI::operator==(const RTTI& other) const {
	return this == &other;
}


bool RTTI::operator!=(const RTTI& other) const {
	return this != &other;
}


bool RTTI::before(const RTTI& other) const {
	const RTTI* parent = other.parent();
	while ( parent != NULL && parent != this )
		parent = parent->parent();

	return parent != NULL;
}


bool RTTI::isTypeOf(const RTTI& other) const {
	return (*this == other) || other.before(*this);
}


}
}
