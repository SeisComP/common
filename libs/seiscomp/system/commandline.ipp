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


template <typename T>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description, T* storage,
                            bool storageAsDefault) {
	options_description* o = findGroup(group);
	if ( o ) {
		if ( storageAsDefault && storage )
			(options_description_easy_init(o))(option, boost::program_options::value<T>(storage)->default_value(*storage), description);
		else
			(options_description_easy_init(o))(option, boost::program_options::value<T>(storage), description);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description,
                            std::vector<T>* storage) {
	options_description* o = findGroup(group);
	if ( o )
		// NOTE: Because of a bug in boost 1.33.1 the option multitoken() eats up
		//       all arguments until the end of the commandline. So multitoken works
		//       only for switches at the end of the commandline. To make it work
		//       multitoken is disabled. So one has to give the option multiple times.
		//       e.g.: 'test -i foo -i bar -j' instead of 'test -i foo bar -j'
		//(options_description_easy_init(o))(option, boost::program_options::value<std::vector<T> >(&storage)->multitoken(), description);
		(options_description_easy_init(o))(option, boost::program_options::value<std::vector<T> >(storage), description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, typename DT>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description, T* storage,
                            const DT& defaultValue) {
	options_description* o = findGroup(group);
	if ( o )
		(options_description_easy_init(o))(option, boost::program_options::value<T>(storage)->default_value(defaultValue), description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void CommandLine::addCustomOption(const char* group,
                                  const char* option,
                                  const char* description,
                                  T* customValidator) {
	options_description* o = findGroup(group);
	if ( o )
		(options_description_easy_init(o))(option, customValidator, description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline T CommandLine::option(const std::string& option) const {
	try {
		return boost::any_cast<T>(_variableMap[option].value());
	}
	catch ( ... ) {
		throw Core::TypeException("Invalid type for cast");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, int LEN>
inline T CommandLine::option(const char (&option)[LEN]) const {
	return this->option<T>(std::string(option));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
