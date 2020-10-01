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


#ifndef SEISCOMP_CORE_EXCEPTIONS_H
#define SEISCOMP_CORE_EXCEPTIONS_H

#include <seiscomp/core.h>

#include <string>
#include <exception>
#include <typeinfo>

namespace Seiscomp {
namespace Core {


class SC_SYSTEM_CORE_API GeneralException : public std::exception {
	public:
		GeneralException();
		GeneralException( const std::string& str);

		virtual ~GeneralException() throw();
		
		virtual const char* what( void ) const throw();

	private:
		std::string _descr;
};


class SC_SYSTEM_CORE_API MemoryException : public GeneralException {
	public:
		MemoryException();
		MemoryException(std::string what);
};


class SC_SYSTEM_CORE_API StreamException : public GeneralException {
	public:
		StreamException();
		StreamException(std::string what);
};


class SC_SYSTEM_CORE_API EndOfStreamException : public StreamException {
	public:
		EndOfStreamException();
		EndOfStreamException(std::string what);
};


class SC_SYSTEM_CORE_API TypeConversionException : public GeneralException {
	public:
		TypeConversionException();
		TypeConversionException(const std::string& str);
};


class SC_SYSTEM_CORE_API OverflowException : public GeneralException {
	public:
		OverflowException();
		OverflowException(const std::string& str);
};


class SC_SYSTEM_CORE_API UnderflowException : public GeneralException {
	public:
		UnderflowException();
		UnderflowException(const std::string& str);
};


class SC_SYSTEM_CORE_API ValueException : public GeneralException {
	public:
		ValueException();
		ValueException(const std::string& str);
};


class SC_SYSTEM_CORE_API TypeException : public GeneralException {
	public:
		TypeException();
		TypeException(const std::string& str);
};


class SC_SYSTEM_CORE_API ClassNotFound : public GeneralException {
	public:
		ClassNotFound();
		ClassNotFound(const std::string& str);
};


class SC_SYSTEM_CORE_API DuplicateClassname : public GeneralException {
	public:
		DuplicateClassname();
		DuplicateClassname(const std::string& str);
};


}
}

#endif
