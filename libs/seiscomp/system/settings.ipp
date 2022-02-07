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


namespace Seiscomp {
namespace System {

namespace Generic {


template <template <typename> class VisitedItem, class Proc>
template <typename T>
inline SettingsVisitor<VisitedItem, Proc> &SettingsVisitor<VisitedItem, Proc>
::operator&(VisitedItem<T> visitedItem) {
	return VisitHelper<T, Detail::IsClassType<T>::value>::process(*this, visitedItem);
}

template <template <typename> class VisitedItem, class Proc>
template <typename T>
inline SettingsVisitor<VisitedItem, Proc> &SettingsVisitor<VisitedItem, Proc>
::operator&(VisitedItem< std::vector<T> > visitedItem) {
	return VisitHelper<std::vector<T>, Detail::IsClassType<T>::value>::process(*this, visitedItem);
}

template <template <typename> class VisitedItem, class Proc>
inline bool SettingsVisitor<VisitedItem, Proc>
::success() const {
	return _success;
}

template <template <typename> class VisitedItem, class Proc>
inline const std::string SettingsVisitor<VisitedItem, Proc>
::lastError() const {
	return _errorMessage;
}

template <template <typename> class VisitedItem, class Proc>
inline void SettingsVisitor<VisitedItem, Proc>::reset() {
	_success = true;
	_errorMessage = std::string();
	configPrefix = "";
	indent = "";
}


template <template <typename> class VisitedItem, class Proc>
inline Proc &SettingsVisitor<VisitedItem, Proc>
::proc() {
	return _proc;
}


template <template <typename> class VisitedItem, class Proc>
inline SettingsVisitor<VisitedItem, Proc>
::operator bool() const {
	return success();
}

template <template <typename> class VisitedItem, class Proc>
template <typename T>
inline void SettingsVisitor<VisitedItem, Proc>::push(const VisitedItem<T> &visitedItem) {
	_prefixStack.push_back(configPrefix);
	indent += "  ";
	if ( visitedItem.configFileRelativeSymbol && *visitedItem.configFileRelativeSymbol ) {
		if ( !configPrefix.empty() ) {
			configPrefix += '.';
		}
		configPrefix += visitedItem.configFileRelativeSymbol;
	}
}

template <template <typename> class VisitedItem, class Proc>
inline void SettingsVisitor<VisitedItem, Proc>::pop() {
	configPrefix = _prefixStack.back();
	_prefixStack.pop_back();
	indent.erase(indent.size()-2);
}

template <template <typename> class VisitedItem, class Proc>
inline void SettingsVisitor<VisitedItem, Proc>::setError(const std::string &msg) {
	_success = false;
	_errorMessage = msg;
}

template <template <typename> class VisitedItem, class Proc>
template <typename T>
inline void SettingsVisitor<VisitedItem, Proc>::handleSingle(VisitedItem<T> &visitedItem) {
	_proc.visitSingle(*this, visitedItem);
}

template <template <typename> class VisitedItem, class Proc>
template <typename T>
inline void SettingsVisitor<VisitedItem, Proc>::handleMultiple(VisitedItem< std::vector<T> > &visitedItem) {
	_proc.visitMultiple(*this, visitedItem);
}


}

}
}
