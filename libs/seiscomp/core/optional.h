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


#ifndef SEISCOMP_CORE_OPTIONAL_H
#define SEISCOMP_CORE_OPTIONAL_H

#include <seiscomp/core.h>

#include <exception>
#include <boost/optional.hpp>
#include <boost/none.hpp>

namespace Seiscomp {
namespace Core {

/** \brief Redefines boost::optional<T>
  * Optional values can be set or unset.
  * \code
  *   void print(const Optional<int>::Impl& v) {
  *     if ( !v )
  *       cout << "value of v is not set" << endl;
  *     else
  *       cout << *v << endl;
  *   }
  *
  *   Optional<int>::Impl a = 5;
  *   print(a);  // output: "5"
  *   a = None;
  *   print(a);  // output: "value of v is not set"
  * \endcode
  */
template <typename T>
struct Optional {
	typedef ::boost::optional<T> Impl;
};

/** Defines None */
SC_SYSTEM_CORE_API extern ::boost::none_t const None;

template <typename T>
T value(const boost::optional<T>&);

class SC_SYSTEM_CORE_API ValueError : public std::exception {
	public:
		ValueError() throw();
		~ValueError() throw();

	public:
		const char* what() const throw();
};

/** Macro to use optional values easily */
#define OPT(T) Seiscomp::Core::Optional<T>::Impl
/** Macro to use optional values as const reference */
#define OPT_CR(T) const Seiscomp::Core::Optional<T>::Impl&

#include <seiscomp/core/optional.inl>

}
}

#endif
