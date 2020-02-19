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


#include <seiscomp/core/exceptions.h>

namespace Seiscomp {
namespace Core {


GeneralException::GeneralException() {}
GeneralException::GeneralException( const std::string& str) : std::exception() {
	_descr = str;
}

GeneralException::~GeneralException() throw() {}

const char* GeneralException::what( void ) const throw() {
	return _descr.c_str();
}


MemoryException::MemoryException() : GeneralException("memory allocation error") {}
MemoryException::MemoryException(std::string what) : GeneralException(what) {}


StreamException::StreamException() : GeneralException("stream error") {}
StreamException::StreamException(std::string what) : GeneralException(what) {}


EndOfStreamException::EndOfStreamException() : StreamException("end of stream") {}
EndOfStreamException::EndOfStreamException(std::string what) : StreamException(what) {}


TypeConversionException::TypeConversionException() : GeneralException("type conversion error") {}
TypeConversionException::TypeConversionException(const std::string& str ) : GeneralException(str) {}


OverflowException::OverflowException() : GeneralException("overflow") {}
OverflowException::OverflowException(const std::string& str ) : GeneralException(str) {}


UnderflowException::UnderflowException() : GeneralException("underflow") {}
UnderflowException::UnderflowException(const std::string& str ) : GeneralException(str) {}


ValueException::ValueException() : GeneralException("value error") {}
ValueException::ValueException(const std::string& str ) : GeneralException(str) {}


TypeException::TypeException() : GeneralException("type error") {}
TypeException::TypeException(const std::string& str ) : GeneralException(str) {}


ClassNotFound::ClassNotFound() : GeneralException("the requested classname has not been registered") {}
ClassNotFound::ClassNotFound(const std::string& str ) : GeneralException(str) {}


DuplicateClassname::DuplicateClassname() : GeneralException("duplicate classname") {}
DuplicateClassname::DuplicateClassname(const std::string& str ) : GeneralException(str) {}


}
}
