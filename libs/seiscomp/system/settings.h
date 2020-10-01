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


#ifndef SEISCOMP_SYSTEM_SETTINGS_H
#define SEISCOMP_SYSTEM_SETTINGS_H


#include <seiscomp/core/strings.h>
#include <seiscomp/config/config.h>

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>

#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>


namespace Seiscomp {
namespace System {

namespace Generic {

namespace Detail {


class No { };
class Yes { No no[2]; };

template <typename Type, Type Ptr>
struct MemberHelperClass;

template <typename T, typename Type>
Yes MemberHelper_accept(MemberHelperClass<Type, &T::accept> *);
template<typename T,typename Type>
No MemberHelper_accept(...);

template<typename T, typename PARAM>
struct HasMemberAccept {
	enum {
		value = sizeof(MemberHelper_accept<T, void (T::*)(PARAM &)>(0)) == sizeof(Yes)
	};
};


template <typename T>
struct IsClassType {
	enum {
		value = boost::is_same<int,T>::value ?
		0
		:
		(
			boost::is_same<unsigned int,T>::value ?
			0
			:
			(
				boost::is_same<double,T>::value ?
				0
				:
				(
					boost::is_same<bool,T>::value ?
					0
					:
					1
				)
			)
		)
	};
};


template <typename C, typename T>
struct MustMatch {
	static T get(const C &value) {
		throw std::runtime_error("key attribute must be a string");
	}
};

template <typename C>
struct MustMatch<C,C> {
	static C get(const C &value) {
		return value;
	}
};


template <typename T>
T getConfig(const Config::Config *cfg, const std::string &symbol, bool asPath);


}


template <template <typename> class VisitedItem, class Proc>
class SettingsVisitor {
	public:
		typedef SettingsVisitor<VisitedItem, Proc> SelfType;

		SettingsVisitor() : _success(true) {}


	// ----------------------------------------------------------------------
	//  Public interface for visited classes
	// ----------------------------------------------------------------------
	public:
		//! Bind a value
		template <typename T>
		SettingsVisitor &operator&(VisitedItem<T> visitedItem);

		//! Bind a value
		template <typename T>
		SettingsVisitor &operator&(VisitedItem< std::vector<T> > visitedItem);


	// ----------------------------------------------------------------------
	//  Public state management
	// ----------------------------------------------------------------------
	public:
		bool success() const;
		const std::string lastError() const;

		//! Resets the success flag and the error message
		void reset();

		//! Returns the processor
		Proc &proc();


	public:
		operator bool() const;


	// ----------------------------------------------------------------------
	//  Interface to be used in processor
	// ----------------------------------------------------------------------
	public:
		template <typename T>
		void push(const VisitedItem<T> &visitedItem);
		void pop();
		void setError(const std::string &msg);


	// ----------------------------------------------------------------------
	//  Private methods and helpers
	// ----------------------------------------------------------------------
	private:
		template <typename T, int IS_CLASS>
		struct VisitHelper {};

		// Non-class types never have member functions. This includes vectors
		// of non-class types
		template <typename T>
		struct VisitHelper<T, 0> {
			static SettingsVisitor &process(SettingsVisitor &visitor, VisitedItem<T> &visitedItem) {
				visitor.handleSingle(visitedItem);
				return visitor;
			}
		};

		// Class type
		template <typename T>
		struct VisitHelper<T, 1> {
			static SettingsVisitor &process(SettingsVisitor &visitor, VisitedItem<T> &visitedItem) {
				return VisitCompositeHelper<T, Detail::HasMemberAccept<T, SelfType>::value>::dispatch(visitor, visitedItem);
			}
		};

		// Vector with class type
		template <typename T>
		struct VisitHelper<std::vector<T>, 1> {
			static SettingsVisitor &process(SettingsVisitor &visitor, VisitedItem< std::vector<T> > &visitedItem) {
				return VisitCompositeHelper<std::vector<T>, Detail::HasMemberAccept<T, SelfType>::value>::dispatch(visitor, visitedItem);
			}
		};

		template <typename T, int HAS_VISIT>
		struct VisitCompositeHelper {};

		// No bind method
		template <typename T>
		struct VisitCompositeHelper<T,0> {
			static SettingsVisitor &dispatch(SettingsVisitor &visitor, VisitedItem<T> &visitedItem) {
				visitor.handleSingle(visitedItem);
				return visitor;
			}
		};

		// Bind method => descent
		template <typename T>
		struct VisitCompositeHelper<T,1> {
			static SettingsVisitor &dispatch(SettingsVisitor &visitor, VisitedItem<T> &visitedItem) {
				visitor.push(visitedItem);
				visitedItem.value.accept(visitor);
				visitor.pop();
				return visitor;
			}
		};

		// Vector of elements without bind method
		template <typename T>
		struct VisitCompositeHelper< std::vector<T> ,0> {
			static SettingsVisitor &dispatch(SettingsVisitor &visitor, VisitedItem< std::vector<T> > &visitedItem) {
				visitor.handleSingle(visitedItem);
				return visitor;
			}
		};

		// Vector with elements with bind method
		template <typename T>
		struct VisitCompositeHelper< std::vector<T> ,1> {
			static SettingsVisitor &dispatch(SettingsVisitor &visitor, VisitedItem< std::vector<T> > &visitedItem) {
				visitor.handleMultiple(visitedItem);
				return visitor;
			}
		};


	// ----------------------------------------------------------------------
	//  Callbacks which delegate calls to the processor
	// ----------------------------------------------------------------------
	private:
		template <typename T>
		void handleSingle(VisitedItem<T> &visitedItem);

		template <typename T>
		void handleMultiple(VisitedItem< std::vector<T> > &visitedItem);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	public:
		std::string indent; //!< Indentation for each tree level in two-space steps
		std::string configPrefix;

	protected:
		std::deque<std::string> _prefixStack;
		bool                    _success;
		std::string             _errorMessage;

	private:
		Proc _proc;
};


}


template <typename T>
struct ConfigOptionBinding {
	enum Flags {
		NoFlags         = 0x00,
		IsKey           = 0x01,
		InterpretAsPath = 0x02
	};

	ConfigOptionBinding(T &value, int flags, const char *configFileRelativeSymbol)
	: value(value)
	, flags(flags)
	, configFileRelativeSymbol(configFileRelativeSymbol) {}

	bool isKey() { return flags & IsKey; }

	T          &value;
	int         flags;
	const char *configFileRelativeSymbol;
};


class ConfigOptionLinker {
	public:
		ConfigOptionLinker()
		: _stage(None) {}


	public:
		void get(const Config::Config *cfg) {
			setStage(GetCfg);
			_external.cfg = cfg;
		}

		void get(const Config::Config &cfg) {
			get(&cfg);
		}

		void dump(std::ostream &os) {
			setStage(Print);
			_external.os = &os;
		}


	public:
		// A single non-array option
		template <typename T, typename V>
		void visitSingle(V &visitor, ConfigOptionBinding<T> &visitedItem) {
			switch ( _stage ) {
				case GetCfg:
					if ( !visitedItem.isKey() && !visitedItem.configFileRelativeSymbol )
						return;
					if ( !CfgLinkHelper<T, IsNativelySupported<T>::value>::process(*this, visitedItem, visitor.configPrefix) )
						visitor.setError("Invalid configuration value for " + visitor.configPrefix + visitedItem.configFileRelativeSymbol);
					break;
					break;
				case Print:
					if ( visitedItem.configFileRelativeSymbol )
						*_external.os << visitor.configPrefix << visitedItem.configFileRelativeSymbol;
					else if ( visitedItem.isKey() )
						*_external.os << "*KEY*";
					else
						return;
					*_external.os << ": ";
					PrintHelper<T, IsNativelySupported<T>::value>::process(*_external.os, visitedItem.value);
					*_external.os << std::endl;
					break;
				default:
					break;
			}
		}

		// A single array option
		template <typename T, typename V>
		void visitSingle(V &visitor, ConfigOptionBinding< std::vector<T> > &visitedItem) {
			switch ( _stage ) {
				case GetCfg:
					if ( !visitedItem.configFileRelativeSymbol )
						return;
					if ( !CfgLinkHelper<std::vector<T>, IsNativelySupported<T>::value>::process(*this, visitedItem, visitor.configPrefix) )
						visitor.setError("Invalid configuration value for " + visitor.configPrefix + visitedItem.configFileRelativeSymbol);
					break;
				case Print:
					if ( visitedItem.configFileRelativeSymbol )
						*_external.os << visitor.configPrefix << visitedItem.configFileRelativeSymbol;
					else if ( visitedItem.isKey() )
						*_external.os << "*KEY*";
					else
						return;
					*_external.os << ": ";
					PrintHelper<std::vector<T>, IsNativelySupported<T>::value>::process(*_external.os, visitedItem.value);
					*_external.os << std::endl;
					break;
				default:
					break;
			}
		}

		// An array option consisting of composites
		template <typename T, typename V>
		void visitMultiple(V &visitor, ConfigOptionBinding< std::vector<T> > &visitedItem) {
			switch ( _stage ) {
				case GetCfg:
					try {
						std::vector<std::string> items;
						items = Generic::Detail::getConfig< std::vector<std::string> >(_external.cfg, visitor.configPrefix + visitedItem.configFileRelativeSymbol, false);
						std::string oldKey = _key;
						visitor.push(visitedItem);
						for ( size_t i = 0; i < items.size(); ++i ) {
							T value;
							ConfigOptionBinding<T> item(value, false, items[i].c_str());
							std::string oldKey = _key;
							visitor.push(item);
							_key = items[i];
							item.value.accept(visitor);
							if ( visitor.success() )
								visitedItem.value.push_back(value);
							visitor.pop();
							_key = oldKey;
						}
						visitor.pop();
						_key = oldKey;
					}
					catch ( ... ) {}
					break;
				case Print:
					if ( visitedItem.configFileRelativeSymbol ) {
						*_external.os << visitor.configPrefix << visitedItem.configFileRelativeSymbol;
						*_external.os << ": ";
						if ( visitedItem.value.empty() )
							*_external.os << "[]" << std::endl;
						else {
							*_external.os << "[" << visitedItem.value.size() << "]" << std::endl;
							std::string oldKey = _key;
							visitor.push(visitedItem);
							for ( size_t i = 0; i < visitedItem.value.size(); ++i ) {
								*_external.os << "{" << std::endl;
								visitedItem.value[i].accept(visitor);
								*_external.os << "}" << std::endl;
							}
							visitor.pop();
							_key = oldKey;
						}
					}
					break;
				default:
					break;
			}
		}


	// Helpers
	private:
		template <typename T>
		T key() const {
			return Generic::Detail::MustMatch<std::string,T>::get(_key);
		}


		template <typename T>
		struct IsNativelySupported {
			enum {
				value = Generic::Detail::IsClassType<T>::value  ?
				(
					boost::is_same<std::string,T>::value ?
					1
					:
					0
				)
				:
				1
			};
		};


		template <typename T, int IS_SUPPORTED>
		struct CfgLinkHelper {};

		template <typename T>
		struct CfgLinkHelper<T,0> {
			template <typename P>
			static bool process(P &proc,
			                    ConfigOptionBinding<T> &visitedItem,
			                    const std::string &prefix) {
				try {
					std::string tmp;
					if ( visitedItem.isKey() )
						tmp = proc.template key<std::string>();
					else
						tmp = Generic::Detail::getConfig<std::string>(proc._external.cfg, prefix + visitedItem.configFileRelativeSymbol, visitedItem.flags & ConfigOptionBinding< std::vector<T> >::InterpretAsPath);
					return fromString(visitedItem.value, tmp);
				}
				catch ( ... ) {}

				return true;
			}
		};

		template <typename T>
		struct CfgLinkHelper<std::vector<T>,0> {
			template <typename P>
			static bool process(P &proc,
			                    ConfigOptionBinding< std::vector<T> > &visitedItem,
			                    const std::string &prefix) {
				try {
					std::vector<std::string> tmp;
					if ( visitedItem.isKey() )
						tmp = proc.template key<std::vector<std::string> >();
					else
						tmp = Generic::Detail::getConfig<std::vector<std::string> >(proc._external.cfg, prefix + visitedItem.configFileRelativeSymbol, visitedItem.flags & ConfigOptionBinding< std::vector<T> >::InterpretAsPath);
					visitedItem.value.resize(tmp.size());
					for ( size_t i = 0; i < tmp.size(); ++i ) {
						if ( !fromString(visitedItem.value[i], tmp[i]) )
							return false;
					}
				}
				catch ( ... ) {}

				return true;
			}
		};

		template <typename T>
		struct CfgLinkHelper<T,1> {
			template <typename P>
			static bool process(P &proc,
			                    ConfigOptionBinding<T> &visitedItem,
			                    const std::string &prefix) {
				if ( visitedItem.isKey() )
					visitedItem.value = proc.template key<T>();
				else {
					try {
						visitedItem.value = Generic::Detail::getConfig<T>(proc._external.cfg, prefix + visitedItem.configFileRelativeSymbol, visitedItem.flags & ConfigOptionBinding< std::vector<T> >::InterpretAsPath);
					}
					catch ( ... ) {}
				}
				return true;
			}
		};

		template <typename T, int IS_SUPPORTED>
		struct PrintHelper {};

		template <typename T>
		struct PrintHelper<T,0> {
			static void process(std::ostream &os, const T &value) {
				os << toString(value);
			}
		};

		template <typename T>
		struct PrintHelper<std::vector<T>,0> {
			static void process(std::ostream &os, const std::vector<T> &value) {
				if ( value.empty() ) {
					os << "[]";
					return;
				}

				for ( size_t i = 0; i < value.size(); ++i ) {
					if ( i ) os << ", ";
					PrintHelper<T,0>::process(os, value[i]);
				}
			}
		};

		template <typename T>
		struct PrintHelper<T,1> {
			static void process(std::ostream &os, const T &value) {
				os << value;
			}
		};

		template <typename T>
		struct PrintHelper<std::vector<T>,1> {
			static void process(std::ostream &os, const std::vector<T> &value) {
				if ( value.empty() )
					os << "[]";
				else
					os << Core::toString(value);
			}
		};


	private:
		enum Stage {
			None,
			GetCfg,
			Print
		};

		void setStage(Stage s) {
			_stage = s;
		}

		Stage       _stage;
		std::string _key; //!< The current array item key value

		// Output structures depending on the stage
		union {
			std::ostream         *os;
			const Config::Config *cfg;
		}           _external;
};


struct ConfigSettingsLinker : Generic::SettingsVisitor<ConfigOptionBinding, ConfigOptionLinker> {
	template <typename T>
	static ConfigOptionBinding<T> key(T &boundValue) {
		return ConfigOptionBinding<T>(boundValue, ConfigOptionBinding<T>::IsKey, nullptr);
	}

	template <typename T>
	static ConfigOptionBinding<T> cfg(T &boundValue, const char *name) {
		return ConfigOptionBinding<T>(boundValue, 0, name);
	}

	template <typename T>
	static ConfigOptionBinding<T> cfgAsPath(T &boundValue, const char *name) {
		return ConfigOptionBinding<T>(boundValue, ConfigOptionBinding<T>::InterpretAsPath, name);
	}
};



}
}


#include <seiscomp/system/settings.ipp>


#endif
