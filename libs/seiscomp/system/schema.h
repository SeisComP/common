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


#ifndef SEISCOMP_CONFIGURATION_SCHEMA_H
#define SEISCOMP_CONFIGURATION_SCHEMA_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core.h>

#include <string>
#include <iostream>


namespace Seiscomp {
namespace System {


class SchemaVisitor;

DEFINE_SMARTPOINTER(SchemaParameter);
class SC_SYSTEM_CORE_API SchemaParameter : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaParameter);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaParameter() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string              name;
		std::string              type;
		std::string              unit;
		std::string              range;
		std::vector<std::string> values;
		std::vector<std::string> options;
		std::string              defaultValue;
		std::string              description;
		OPT(bool)                readOnly;
};


DEFINE_SMARTPOINTER(SchemaGroup);
DEFINE_SMARTPOINTER(SchemaStructure);

DEFINE_SMARTPOINTER(SchemaStructExtent);
DEFINE_SMARTPOINTER(SchemaParameters);
class SC_SYSTEM_CORE_API SchemaParameters : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaParameter);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaParameters() {}


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		size_t parameterCount() const;
		SchemaParameter *parameter(size_t i);
		bool add(SchemaParameter *param);

		size_t groupCount() const;
		SchemaGroup *group(size_t i);
		bool add(SchemaGroup *group);

		size_t structureCount() const;
		SchemaStructure *structure(size_t i);
		bool add(SchemaStructure *structure);

		bool isEmpty() const;

		void accept(SchemaVisitor *) const;


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	private:
		std::vector<SchemaParameterPtr> _parameters;
		std::vector<SchemaGroupPtr>     _groups;
		std::vector<SchemaStructurePtr> _structs;

	public:
		std::vector<SchemaStructExtentPtr> structExtents;
};

struct SchemaStructExtent : SchemaParameters {
	DECLARE_SC_CLASS(SchemaStructExtent);

	void serialize(Archive& ar) override;
	std::string type;
	std::string matchName;
};



class SC_SYSTEM_CORE_API SchemaGroup : public SchemaParameters {
	DECLARE_SC_CLASS(SchemaGroup);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaGroup() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::string description;
};


class SC_SYSTEM_CORE_API SchemaStructure : public SchemaParameters {
	DECLARE_SC_CLASS(SchemaStructure);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaStructure() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string type;
		std::string title;
		std::string link;
		std::string description;
};


DEFINE_SMARTPOINTER(SchemaSetupInput);
DEFINE_SMARTPOINTER(SchemaSetupInputOption);
class SC_SYSTEM_CORE_API SchemaSetupInputOption : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaSetupInputOption);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetupInputOption() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string value;
		std::string description;
		std::vector<SchemaSetupInputPtr> inputs;
};


class SC_SYSTEM_CORE_API SchemaSetupInput : public SchemaParameter {
	DECLARE_SC_CLASS(SchemaSetupInput);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetupInput() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string                            text;
		std::string                            echo;
		std::vector<SchemaSetupInputOptionPtr> options;
};


DEFINE_SMARTPOINTER(SchemaSetupGroup);
class SC_SYSTEM_CORE_API SchemaSetupGroup : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaSetupGroup);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetupGroup() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::vector<SchemaSetupInputPtr> inputs;
};



DEFINE_SMARTPOINTER(SchemaSetup);
class SC_SYSTEM_CORE_API SchemaSetup : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaSetup);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetup() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::vector<SchemaSetupGroupPtr> groups;
};


DEFINE_SMARTPOINTER(SchemaModule);
class SC_SYSTEM_CORE_API SchemaModule : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaModule);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaModule() : aliasedModule(nullptr) {}

		bool isStandalone() const {
			return standalone && *standalone;
		}

		void accept(SchemaVisitor *) const;


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		SchemaModule       *aliasedModule;
		std::string         name;
		std::string         category;
		std::string         import;
		std::string         description;
		OPT(bool)           standalone;
		OPT(bool)           inheritGlobalBinding;
		SchemaParametersPtr parameters;
		SchemaSetupPtr      setup;
};


DEFINE_SMARTPOINTER(SchemaPlugin);
class SC_SYSTEM_CORE_API SchemaPlugin : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaPlugin);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaPlugin() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string               name;
		std::vector<std::string>  extends;
		std::string               description;
		SchemaParametersPtr       parameters;
		SchemaSetupPtr            setup;
};


DEFINE_SMARTPOINTER(SchemaBinding);
class SC_SYSTEM_CORE_API SchemaBinding : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaBinding);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaBinding() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string         name;
		std::string         module;
		std::string         category;
		std::string         description;
		SchemaParametersPtr parameters;
};


DEFINE_SMARTPOINTER(SchemaDefinitions);
class SC_SYSTEM_CORE_API SchemaDefinitions : public Core::BaseObject {
	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef std::vector<SchemaPlugin*>  PluginList;
		typedef std::vector<SchemaBinding*> BindingList;


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaDefinitions() {}


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		SchemaModule *createAlias(const char *existingModule, const char *newModule);
		bool removeAlias(const char *existingModule);

		size_t moduleCount() const;
		SchemaModule *module(size_t i);
		SchemaModule *module(const char *name);
		SchemaModule *module(const std::string &name);
		bool add(SchemaModule *module);

		size_t pluginCount() const;
		SchemaPlugin *plugin(size_t i);
		SchemaPlugin *plugin(const char *name);
		SchemaPlugin *plugin(const std::string &name);
		bool add(SchemaPlugin *plugin);

		size_t bindingCount() const;
		SchemaBinding *binding(size_t i);
		SchemaBinding *binding(const char *name);
		SchemaBinding *binding(const std::string &name);
		bool add(SchemaBinding *binding);

		//! Returns all plugins for a certain module
		//! The plugin pointers are managed by the Definition instance
		//! and must not be deleted.
		PluginList pluginsForModule(const char *name) const;
		PluginList pluginsForModule(const std::string &name) const;

		//! Returns all bindings for a certain module
		//! The binding pointers are managed by the Definition instance
		//! and must not be deleted.
		BindingList bindingsForModule(const char *name) const;
		BindingList bindingsForModule(const std::string &name) const;


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar) override;


	// ------------------------------------------------------------------
	//  Read methods
	// ------------------------------------------------------------------
	public:
		bool load(const char *path);
		bool reload();


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	private:
		std::vector<SchemaModulePtr>  _modules;
		std::vector<SchemaPluginPtr>  _plugins;
		std::vector<SchemaBindingPtr> _bindings;
		std::string                   _path;
};


class SC_SYSTEM_CORE_API SchemaVisitor {
	protected:
		SchemaVisitor() {}
		virtual ~SchemaVisitor() {}

	protected:
		virtual bool visit(const SchemaModule*) = 0;
		virtual bool visit(const SchemaGroup*) = 0;
		virtual bool visit(const SchemaStructure*) = 0;
		virtual void visit(const SchemaParameter*) = 0;
		virtual void finished() = 0;

	friend class SchemaModule;
	friend class SchemaParameters;
	friend class SchemaGroup;
	friend class SchemaStructure;
};


}
}


#endif
