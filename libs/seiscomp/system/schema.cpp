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


#define SEISCOMP_COMPONENT DESC

#include <seiscomp/logging/log.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <boost/version.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
#include <fstream>

#include <seiscomp/system/schema.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/system.h>


namespace fs = boost::filesystem;
static fs::directory_iterator fsDirEnd;


using namespace std;


namespace Seiscomp {
namespace System {


IMPLEMENT_SC_CLASS(SchemaParameter, "Configuration::Parameter");
IMPLEMENT_SC_CLASS(SchemaParameters, "Configuration::Parameters");
IMPLEMENT_SC_CLASS(SchemaGroup, "Configuration::Group");
IMPLEMENT_SC_CLASS(SchemaStructure, "Configuration::Struct");
IMPLEMENT_SC_CLASS(SchemaStructExtent, "Configuration::Struct::Extent");
IMPLEMENT_SC_CLASS(SchemaModule, "Configuration::Module");
IMPLEMENT_SC_CLASS(SchemaPlugin, "Configuration::Plugin");
IMPLEMENT_SC_CLASS(SchemaBinding, "Configuration::Binding");
IMPLEMENT_SC_CLASS(SchemaSetupInputOption, "Configuration::SetupInputOption");
IMPLEMENT_SC_CLASS(SchemaSetupInput, "Configuration::SetupInput");
IMPLEMENT_SC_CLASS(SchemaSetupGroup, "Configuration::SetupGroup");
IMPLEMENT_SC_CLASS(SchemaSetup, "Configuration::Setup");


#if BOOST_VERSION <= 103301
#define FS_STR(it) it->string()
#else
#define FS_STR(it) it->path().string()
#endif



namespace {


bool moduleSort(const SchemaModulePtr& x, const SchemaModulePtr& y) {
	return x->name < y->name;
}


void convertDoc(std::string &text) {
	static const std::string whitespace = "\t\n\v\f\r ";

	Core::trim(text);
	size_t pos = 0;
	while ( (pos = text.find_first_of(whitespace, pos)) != std::string::npos ) {
		size_t next = text.find_first_not_of(whitespace, pos);
		int nl = 0;
		if ( next - pos > 1 ) {
			size_t c = next-pos-1;
			for ( size_t i = 0; i < c; ++i ) {
				if ( text[pos+i] == '\n' ) ++nl;
			}

			text.erase(pos+1, c);
		}
		text[pos] = nl < 2?' ':'\n';
		++pos;
	}
}


}


void SchemaParameter::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("name", name, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT("type", type);
	ar & NAMED_OBJECT("unit", unit);
	ar & NAMED_OBJECT("default", defaultValue);
	ar & NAMED_OBJECT("read-only", readOnly);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("values", values);
	ar & NAMED_OBJECT("range", range);
	ar & NAMED_OBJECT("options", options);
	if ( ar.isReading() ) convertDoc(description);
}


void SchemaGroup::serialize(Archive& ar) {
	SchemaParameters::serialize(ar);
	ar & NAMED_OBJECT_HINT("name", name, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);
	if ( ar.isReading() ) convertDoc(description);
}


void SchemaStructure::serialize(Archive& ar) {
	SchemaParameters::serialize(ar);
	ar & NAMED_OBJECT("link", link);
	ar & NAMED_OBJECT_HINT("type", type, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT("title", title);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);
	if ( ar.isReading() ) convertDoc(description);
}


void SchemaParameters::serialize(Archive& ar) {
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("parameter",
		Seiscomp::Core::Generic::containerMember(
			_parameters,
			[this](const SchemaParameterPtr &schemaParameter) {
				return add(schemaParameter.get());
			}
		),
		Archive::STATIC_TYPE
	);

	ar & NAMED_OBJECT_HINT("group",
		Seiscomp::Core::Generic::containerMember(
			_groups,
			[this](const SchemaGroupPtr &schemaGroup) {
				return add(schemaGroup.get());
			}
		),
		Archive::STATIC_TYPE
	);

	ar & NAMED_OBJECT_HINT("struct",
		Seiscomp::Core::Generic::containerMember(
			_structs,
			[this](const SchemaStructurePtr &schemaStructure) {
				return add(schemaStructure.get());
			}
		),
		Archive::STATIC_TYPE
	);

	ar & NAMED_OBJECT_HINT("extend-struct",
		Seiscomp::Core::Generic::containerMember(
			structExtents,
			[this](const SchemaStructExtentPtr &extension) {
				return structExtents.push_back(extension);
			}
		),
		Archive::STATIC_TYPE
	);
}


size_t SchemaParameters::parameterCount() const {
	return _parameters.size();
}

SchemaParameter *SchemaParameters::parameter(size_t i) {
	return _parameters[i].get();
}

bool SchemaParameters::add(SchemaParameter *param) {
	for ( size_t i = 0; i < _parameters.size(); ++i ) {
		if ( _parameters[i]->name == param->name )
			return false;
	}

	_parameters.push_back(param);
	return true;
}


size_t SchemaParameters::groupCount() const {
	return _groups.size();
}

SchemaGroup *SchemaParameters::group(size_t i) {
	return _groups[i].get();
}

bool SchemaParameters::add(SchemaGroup *group) {
	for ( size_t i = 0; i < _groups.size(); ++i ) {
		if ( _groups[i]->name == group->name )
			return false;
	}

	_groups.push_back(group);
	return true;
}


size_t SchemaParameters::structureCount() const {
	return _structs.size();
}


SchemaStructure *SchemaParameters::structure(size_t i) {
	return _structs[i].get();
}


bool SchemaParameters::add(SchemaStructure *structure) {
	for ( size_t i = 0; i < _structs.size(); ++i ) {
		if ( _structs[i]->type == structure->type )
			return false;
	}

	_structs.push_back(structure);
	return true;
}


bool SchemaParameters::isEmpty() const {
	return _parameters.empty() && _groups.empty() && _structs.empty();
}


void SchemaParameters::accept(SchemaVisitor *v) const {
	for ( size_t i = 0; i < _parameters.size(); ++i )
		v->visit(_parameters[i].get());

	for ( size_t i = 0; i < _groups.size(); ++i ) {
		if ( v->visit(_groups[i].get()) )
			_groups[i]->accept(v);
	}

	for ( size_t i = 0; i < _structs.size(); ++i ) {
		if ( v->visit(_structs[i].get()) )
			_structs[i]->accept(v);
	}

	v->finished();
}


void SchemaSetupInputOption::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("value", value, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("input", inputs, Archive::STATIC_TYPE);
	if ( ar.isReading() ) convertDoc(description);
}


void SchemaSetupInput::serialize(Archive& ar) {
	SchemaParameter::serialize(ar);

	ar & NAMED_OBJECT_HINT("text", text, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT("echo", echo);
	ar & NAMED_OBJECT_HINT("option", options, Archive::STATIC_TYPE);
	if ( ar.isReading() ) convertDoc(text);
}


void SchemaSetupGroup::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("name", name, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("input", inputs, Archive::STATIC_TYPE);
}


void SchemaSetup::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("group", groups, Archive::STATIC_TYPE);
}


void SchemaModule::accept(SchemaVisitor *v) const {
	if ( v->visit(this) )
		parameters->accept(v);
}


void SchemaModule::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("name", name, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT("category", category);
	ar & NAMED_OBJECT("import", import);
	ar & NAMED_OBJECT("standalone", standalone);
	ar & NAMED_OBJECT("inherit-global-bindings", inheritGlobalBinding);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);

	ar & NAMED_OBJECT_HINT("configuration", parameters, Archive::STATIC_TYPE | Archive::XML_ELEMENT);

	ar & NAMED_OBJECT_HINT("setup", setup, Archive::STATIC_TYPE | Archive::XML_ELEMENT);

	if ( ar.isReading() )
		convertDoc(description);
}


void SchemaStructExtent::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("type", type, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT("match-name", matchName);
	SchemaParameters::serialize(ar);
}


void SchemaPlugin::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("name", name, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("extends", extends, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("configuration", parameters, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("setup", setup, Archive::STATIC_TYPE | Archive::XML_ELEMENT);

	if ( ar.isReading() ) convertDoc(description);
}


void SchemaBinding::serialize(Archive& ar) {
	ar & NAMED_OBJECT_HINT("name", name, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("module", module, Archive::XML_MANDATORY);
	ar & NAMED_OBJECT("category", category);
	ar & NAMED_OBJECT_HINT("description", description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("configuration", parameters, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.isReading() ) convertDoc(description);
}


SchemaModule *SchemaDefinitions::createAlias(const char *existingModule, const char *newModule) {
	// Does the source module exist?
	SchemaModule *mod = module(existingModule);
	if ( mod == nullptr ) return nullptr;

	// Is the new module name still unused?
	SchemaModulePtr newMod = module(newModule);
	if ( newMod != nullptr ) return nullptr;

	// Copy the existing module
	newMod = new SchemaModule(*mod);
	newMod->aliasedModule = mod;
	newMod->name = newModule;

	// And add it
	if ( !add(newMod.get()) ) return nullptr;

	// Now copy the bindings, plugins
	PluginList  plugins  = pluginsForModule(existingModule);
	BindingList bindings = bindingsForModule(existingModule);

	PluginList::iterator it1;
	for ( it1 = plugins.begin(); it1 != plugins.end(); ++it1 ) {
		SchemaPlugin *p = *it1;
		// Add the new name to the list of modules that it extents
		p->extends.push_back(newModule);
	}

	BindingList::iterator it2;
	for ( it2 = bindings.begin(); it2 != bindings.end(); ++it2 ) {
		// Copy the existing binding
		SchemaBindingPtr newBinding = new SchemaBinding(**it2);
		newBinding->module = newModule;
		add(newBinding.get());
	}

	return newMod.get();
}


bool SchemaDefinitions::removeAlias(const char *existingModule) {
	// Does the alias exist?
	for ( size_t i = 0; i < _modules.size(); ++i ) {
		if ( _modules[i]->name != existingModule ) continue;

		// This is not an alias
		if ( _modules[i]->aliasedModule == nullptr )
			break;

		_modules.erase(_modules.begin() + i);
		return true;
	}

	return false;
}




size_t SchemaDefinitions::moduleCount() const {
	return _modules.size();
}

SchemaModule *SchemaDefinitions::module(size_t i) {
	return _modules[i].get();
}


SchemaModule *SchemaDefinitions::module(const char *name) {
	for ( size_t i = 0; i < _modules.size(); ++i ) {
		if ( _modules[i]->name == name )
			return _modules[i].get();
	}

	return nullptr;
}


SchemaModule *SchemaDefinitions::module(const std::string &name) {
	return module(name.c_str());
}


bool SchemaDefinitions::add(SchemaModule *module) {
	for ( size_t i = 0; i < _modules.size(); ++i ) {
		if ( _modules[i]->name == module->name )
			return false;
	}

	_modules.push_back(module);
	return true;
}

size_t SchemaDefinitions::pluginCount() const {
	return _plugins.size();
}


SchemaPlugin *SchemaDefinitions::plugin(size_t i) {
	return _plugins[i].get();
}


SchemaPlugin *SchemaDefinitions::plugin(const char *name) {
	for ( size_t i = 0; i < _plugins.size(); ++i ) {
		if ( _plugins[i]->name == name )
			return _plugins[i].get();
	}

	return nullptr;
}


SchemaPlugin *SchemaDefinitions::plugin(const std::string &name) {
	return plugin(name.c_str());
}


bool SchemaDefinitions::add(SchemaPlugin *plugin) {
	for ( size_t i = 0; i < _plugins.size(); ++i ) {
		if ( _plugins[i]->name == plugin->name )
			return false;
	}

	_plugins.push_back(plugin);
	return true;
}


size_t SchemaDefinitions::bindingCount() const {
	return _bindings.size();
}


SchemaBinding *SchemaDefinitions::binding(size_t i) {
	return _bindings[i].get();
}


SchemaBinding *SchemaDefinitions::binding(const char *name) {
	for ( size_t i = 0; i < _bindings.size(); ++i ) {
		if ( _bindings[i]->name == name )
			return _bindings[i].get();
	}

	return nullptr;
}


SchemaBinding *SchemaDefinitions::binding(const std::string &name) {
	return binding(name.c_str());
}


bool SchemaDefinitions::add(SchemaBinding *binding) {
	for ( size_t i = 0; i < _bindings.size(); ++i ) {
		if ( _bindings[i]->name == binding->name &&
		     _bindings[i]->module == binding->module ) {
			return false;
		}
	}

	_bindings.push_back(binding);

	return true;
}


std::vector<SchemaPlugin*> SchemaDefinitions::pluginsForModule(const char *name) const {
	std::vector<SchemaPlugin*> plugins;

	for ( size_t i = 0; i < _plugins.size(); ++i ) {
		for ( size_t j = 0; j < _plugins[i]->extends.size(); ++j ) {
			if ( _plugins[i]->extends[j] == name ) {
				plugins.push_back(_plugins[i].get());
			}
		}
	}

	std::sort(plugins.begin(), plugins.end(), [](SchemaPlugin *a, SchemaPlugin *b) {
		if ( !a->parameters ) {
			return b->parameters != nullptr;
		}
		if ( !b->parameters ) {
			return false;
		}
		return a->parameters->structExtents.size() < b->parameters->structExtents.size();
	});

	return plugins;
}


std::vector<SchemaPlugin*> SchemaDefinitions::pluginsForModule(const std::string &name) const {
	return pluginsForModule(name.c_str());
}


bool SchemaDefinitions::load(const char *path) {
	_path = path;
	return reload();
}


bool SchemaDefinitions::reload() {
	IO::XMLArchive ar;

	_modules.clear();
	_plugins.clear();
	_bindings.clear();

	try {
		SC_FS_DECLARE_PATH(directory, _path);

		// Search for all XML files and parse them
		for ( fs::directory_iterator it(directory); it != fsDirEnd; ++it ) {
			if ( fs::is_directory(*it) ) continue;
			string filename = SC_FS_IT_STR(it);
			if ( fs::path(filename).extension() != ".xml" ) continue;

			SEISCOMP_DEBUG("Loading %s", filename.c_str());
			if ( !ar.open(filename.c_str()) ) {
				SEISCOMP_ERROR("Failed to load %s", filename.c_str());
				continue;
			}

			serialize(ar);
			ar.close();
		}

		// Read the aliases file and create the aliases
		fs::path aliases = _path / fs::path("aliases");
		ifstream ifs(aliases.string().c_str());
		if ( ifs.is_open() ) {
			string line;
			while ( getline(ifs, line) ) {
				Core::trim(line);
				// Skip empty lines
				if ( line.empty() ) continue;
				// Skip comments
				if ( line[0] == '#' ) continue;

				size_t pos = line.find('=');
				if ( pos == string::npos ) continue;

				string newMod = line.substr(0, pos);
				string oldMod = line.substr(pos+1);

				Core::trim(newMod); Core::trim(oldMod);

				createAlias(oldMod.c_str(), newMod.c_str());
			}
		}
	}
	catch ( std::exception &exc ) {
		SEISCOMP_ERROR("%s", exc.what());
		return false;
	}

	// Sort modules by name
	std::sort(_modules.begin(), _modules.end(), moduleSort);

	return true;
}


std::vector<SchemaBinding*> SchemaDefinitions::bindingsForModule(const std::string &name) const {
	return bindingsForModule(name.c_str());
}


std::vector<SchemaBinding*> SchemaDefinitions::bindingsForModule(const char *name) const {
	std::vector<SchemaBinding*> bindings;

	for ( size_t i = 0; i < _bindings.size(); ++i ) {
		if ( _bindings[i]->module == name ) {
			bindings.push_back(_bindings[i].get());
		}
	}

	std::sort(bindings.begin(), bindings.end(), [](SchemaBinding *a, SchemaBinding *b) {
		if ( !a->parameters ) {
			return b->parameters != nullptr;
		}
		if ( !b->parameters ) {
			return false;
		}
		return a->parameters->structExtents.size() < b->parameters->structExtents.size();
	});

	return bindings;
}


void SchemaDefinitions::serialize(Archive& ar) {
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("module",
		Seiscomp::Core::Generic::containerMember(
			_modules,
			[this](const SchemaModulePtr &schemaModule) {
				return add(schemaModule.get());
			}
		),
		Archive::STATIC_TYPE
	);

	ar & NAMED_OBJECT_HINT("plugin",
		Seiscomp::Core::Generic::containerMember(
			_plugins,
			[this](const SchemaPluginPtr &schemaPlugin) {
				return add(schemaPlugin.get());
			}
		),
		Archive::STATIC_TYPE
	);

	ar & NAMED_OBJECT_HINT("binding",
		Seiscomp::Core::Generic::containerMember(
			_bindings,
			[this](const SchemaBindingPtr &schemaBinding) {
				return add(schemaBinding.get());
			}
		),
		Archive::STATIC_TYPE
	);
}


}
}
