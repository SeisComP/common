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


#include <seiscomp/geo/featureset.h>

#define SEISCOMP_COMPONENT Geo
#define CONFIG_BASE "geo.feature"

#include <seiscomp/system/environment.h>
#include <seiscomp/core/system.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/geo/formats/bna.h>
#include <seiscomp/geo/formats/fep.h>
#include <seiscomp/geo/formats/geojson.h>

#include <boost/version.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>


namespace fs = boost::filesystem;
using namespace std;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Geo {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet &GeoFeatureSetSingleton::getInstance() {
	static GeoFeatureSetSingleton instance;
	return instance._geoFeatureSet;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSetSingleton::GeoFeatureSetSingleton() {
	_geoFeatureSet.load();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSetObserver::GeoFeatureSetObserver() : _observedSet(nullptr) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSetObserver::~GeoFeatureSetObserver() {
	if ( _observedSet ) {
		_observedSet->unregisterObserver(this);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet::GeoFeatureSet(const GeoFeatureSet &/*unused*/) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoFeatureSet::~GeoFeatureSet() {
	clear();

	for ( auto *observer : _observers ) {
		observer->_observedSet = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::registerObserver(GeoFeatureSetObserver *observer) {
	auto it = find(_observers.begin(), _observers.end(), observer);
	if ( it != _observers.end() ) {
		return false;
	}

	observer->_observedSet = this;
	_observers.push_back(observer);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::unregisterObserver(GeoFeatureSetObserver *observer) {
	auto it = find(_observers.begin(), _observers.end(), observer);
	if ( it == _observers.end() ) {
		return false;
	}

	observer->_observedSet = nullptr;
	_observers.erase(it);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::operator==(const GeoFeatureSet &other) const {
	return std::tie(_features, _categories, _observers) ==
	       std::tie(other._features, other._categories, other._observers);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::operator!=(const GeoFeatureSet &other) const {
	return !(*this == other);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureSet::clear() {
	// Delete all GeoFeatures
	for ( const auto *feature : _features ) {
		delete feature;
	}
	_features.clear();

	// Delete all Categories
	for ( const auto *category : _categories ) {
		delete category;
	}
	_categories.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoFeatureSet::load() {
	clear();

	Environment* env = Environment::Instance();
	if ( !readDir(env->configDir() + "/spatial/vector") ) {
		if ( !readDir(env->shareDir() + "/spatial/vector") ) {
			// For backward compatibility
			if ( !readDir(env->configDir() + "/bna") ) {
				if ( readDir(env->shareDir() + "/bna") ) {
					SEISCOMP_WARNING("The spatial vector data directory %s/bna is deprecated, "
					                 "please move your files to %s/spatial/vector",
					                 env->shareDir().c_str(),
					                 env->shareDir().c_str());
				}
			}
			else {
				SEISCOMP_WARNING("The spatial vector data directory %s/bna is deprecated, "
				                 "please move your files to %s/spatial/vector",
				                 env->configDir().c_str(),
				                 env->configDir().c_str());
			}
		}
	}

	for ( auto *observer : _observers ) {
		observer->geoFeatureSetUpdated();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t GeoFeatureSet::readBNADir(const string& dirPath) {
	// Clear the current GeoFeatureSet
	clear();

	// Get the config base directory
	fs::path directory;
	try {
		directory = SC_FS_PATH(dirPath);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Invalid path '%s'", dirPath.c_str());
		return 0;
	}

	// Read the BNA directory recursively
	Core::Time start = Core::Time::UTC();
	size_t fileCount = readBNADirRecursive(directory, addNewCategory(""));
	SEISCOMP_INFO("%s in %fs", initStatus(dirPath, fileCount).c_str(),
	              (Core::Time::UTC()-start).length());

	// Sort the features according to their rank
 	sort(_features.begin(), _features.end(), compareByRank);

	return fileCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t GeoFeatureSet::readDir(const std::string &dirPath) {
	// Clear the current GeoFeatureSet
	clear();

	// Get the config base directory
	fs::path directory;
	try {
		directory = SC_FS_PATH(dirPath);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Invalid path '%s'", dirPath.c_str());
		return 0;
	}

	// Read the directory recursively
	Core::Time start = Core::Time::UTC();
	size_t fileCount = readDirRecursive(directory, addNewCategory(""));
	SEISCOMP_INFO("%s in %fs", initStatus(dirPath, fileCount).c_str(),
	              (Core::Time::UTC()-start).length());

	// Sort the features according to their rank
 	sort(_features.begin(), _features.end(), compareByRank);

	return fileCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t GeoFeatureSet::readBNADirRecursive(const fs::path &directory,
                                          Category *category) {
	// store directory path the data was read from
	category->dataDir = directory.string();

	size_t fileCount = 0;

	try {
		fs::directory_iterator end_itd;
		fs::directory_iterator itd(directory);

		string fileName;
		for ( ; itd != end_itd; ++itd ) {
			fileName = SC_FS_IT_LEAF(itd);

			// If the current directory entry is a directory, then
			// descend into this directory
			if ( fs::is_directory(*itd) ) {
				fileCount += readBNADirRecursive(
					*itd,
					addNewCategory(fileName, category)
				);
			}
			else if ( fileName.length() < 4 ||
			          fileName.substr(fileName.length() - 4) != ".bna") {
				continue;
			}
			else if ( readBNA(*this, SC_FS_IT_STR(itd), category) > 0 ) {
				++fileCount;
			}
			else {
				SEISCOMP_ERROR("Error reading file: %s", SC_FS_IT_STR(itd).c_str());
			}
		}
	}
	catch ( const exception & ) {}

	return fileCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t GeoFeatureSet::readDirRecursive(const boost::filesystem::path &directory,
                                       Category *category) {
	// store directory path the data was read from
	category->dataDir = directory.string();

	size_t fileCount = 0;

	try {
		fs::directory_iterator end_itd;
		fs::directory_iterator itd(directory);

		string fileName;
		for ( ; itd != end_itd; ++itd ) {
			fileName = SC_FS_IT_LEAF(itd);

			// If the current directory entry is a directory, then
			// descend into this directory
			if ( fs::is_directory(*itd) ) {
				fileCount += readDirRecursive(
					*itd,
					addNewCategory(fileName, category)
				);
				continue;
			}

			try {
				if ( readFile(SC_FS_IT_STR(itd), category) > 0 ) {
					++fileCount;
				}
			}
			catch ( std::exception &e ) {
				SEISCOMP_ERROR("Error reading file '%s': %s",
				               SC_FS_IT_STR(itd).c_str(), e.what());
			}
		}
	}
	catch ( const exception & ) {}

	return fileCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Category* GeoFeatureSet::addNewCategory(string name, const Category *parent) {
	auto *category = new Category(_categories.size(),
		!parent || parent->name.empty() ? name :
		parent->name + "." + name, parent);
	category->localName = std::move(name);
	_categories.push_back(category);
	return category;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string GeoFeatureSet::initStatus(const string &directory,
                                 unsigned int fileCount) const {
	size_t vertexCount = 0;

	Features::const_iterator itf;

	for ( itf = _features.begin(); itf != _features.end(); ++itf ) {
		vertexCount += (*itf)->vertices().size();
	}

	ostringstream buffer;
	buffer << "Read " << _features.size()
	       << " features(s) with a total number of "
	       << vertexCount << " vertices from " << fileCount
	       << " file(s) found under " << directory;

	return buffer.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::readBNAFile(const string &filename,
                                const Category *category) {
	return readBNA(*this, filename, category) > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ssize_t GeoFeatureSet::readFile(const std::string &filename,
                                const Category *category) {
	if ( filename.length() >= 4 &&
	     filename.substr(filename.length() - 4) == ".bna") {
		return static_cast<ssize_t>(readBNA(*this, filename, category));
	}

	if ( filename.length() >= 8 &&
	     filename.substr(filename.length() - 8) == ".geojson") {
		return static_cast<ssize_t>(readGeoJSON(*this, filename, category));
	}

	if ( filename.length() >= 4 &&
	     filename.substr(filename.length() - 4) == ".fep") {
		return static_cast<ssize_t>(readFEP(*this, filename, category));
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::addFeature(GeoFeature *feature) {
	_features.push_back(feature);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool GeoFeatureSet::compareByRank(const GeoFeature* gf1, const GeoFeature* gf2) {
  	return gf1->rank() < gf2->rank();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream& operator<<(std::ostream& os, const GeoFeatureSet &gfs) {
	writeGeoJSON(os, gfs.features());
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Seiscomp::Geo
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
