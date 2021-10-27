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


#ifndef SEISCOMP_GEO_FEATURESET_H
#define SEISCOMP_GEO_FEATURESET_H


#include <seiscomp/core.h>
#include <seiscomp/geo/feature.h>

#include <vector>
#include <boost/filesystem/path.hpp>


namespace Seiscomp {
namespace Geo {


class GeoFeatureSet;

class SC_SYSTEM_CORE_API GeoFeatureSetObserver {
	public:
		GeoFeatureSetObserver();
		virtual ~GeoFeatureSetObserver();

	public:
		//! Signals that the feature set has been updated.
		virtual void geoFeatureSetUpdated() = 0;

	private:
		GeoFeatureSet *_observedSet;

	friend class GeoFeatureSet;
};


class SC_SYSTEM_CORE_API GeoFeatureSet : public Core::BaseObject {
	public:
		/** Default constructor */
		GeoFeatureSet();
		/** Destructor */
		virtual ~GeoFeatureSet();

		/** Copy operator, intentionally left undefined */
		GeoFeatureSet & operator=(const GeoFeatureSet &);


	public:
		bool registerObserver(GeoFeatureSetObserver*);
		bool unregisterObserver(GeoFeatureSetObserver*);


	public:
		/**
		 * Removes and destructs all elements from the _features and
		 * _categories vectors
		 */
		void clear();

		/**
		 * @brief Loads the default geofeature dataset
		 */
		void load();

		/**
		 * Initializes the _feature vector with all BNA-files of the specified
		 * directory. The directory is searched recursively. The name of any
		 * subdirectory is used as a category.
		 * The feature set is cleared prior to reading the files.
		 *
		 * @deprecated This method is deprecated and will be removed in future
		 * releases. Consider readDir() instead.
		 */
		size_t readBNADir(const std::string &dirPath) __attribute__((deprecated));

		/**
		 * Reads one BNA-file
		 * @deprecated This method is deprecated and will be removed in future
		 * releases. Consider readFile() or Seiscomp::Geo::readBNA() instead.
		 */
		bool readBNAFile(const std::string& filename, const Category* category) __attribute__((deprecated));

		/**
		 * Initializes the _feature vector with all files (*.bna, *.geojson) of
		 * the specified directory. The directory is searched recursively.
		 * The name of any subdirectory is used as a category.
		 *
		 * All files found are added to the current feature set. It is not
		 * cleared automatically.
		 */
		size_t readDir(const std::string &dirPath);

		/**
		 * @brief Reads a geo feature vector file of any supported format.
		 * This method might throw an exception to signal an error.
		 *
 		 * @param filename The filename to be read. The file extension is being
 		 *        used to determine the file format.
		 * @param category An optional category.
		 * @return Number of features read. If a negative number is returned
		 *         then this file was ignored because its file extension is
		 *         unknown.
		 * @since SeisComP ABI version 14.3.0
		 */
		ssize_t readFile(const std::string& filename, const Category* category);

		bool addFeature(GeoFeature *feature);

		/** Returns reference to GeoFeature vector */
		const std::vector<GeoFeature*> &features() const { return _features; };

		/** Returns reference to Category vector */
		const std::vector<Category*> &categories() const { return _categories; };


	private:
		/** Copy constructor, private -> non copyable */
		GeoFeatureSet(const GeoFeatureSet &);

		/** Reads a BNADir recursively, used by readBNADir() */
		size_t readBNADirRecursive(const boost::filesystem::path &directory,
		                           Category *category);

		/** Reads a GeoJSONDir recursively, used by readGeoJSONDir() */
		size_t readDirRecursive(const boost::filesystem::path &directory,
		                        Category *category);

		/** Prints the number of polygons read */
		const std::string initStatus(const std::string &directory,
		                             unsigned int fileCount) const;

		/** Compares two GeoFeatures by their rank */
		static bool compareByRank(const GeoFeature* gf1, const GeoFeature* gf2);

		/**
		 * Creates and inserts a new Category object into the Category vector.
		 * The name of the new category is set to the name parameter, prefixed
		 * (if available) by the name of the parent category.
		 */
		Category* addNewCategory(const std::string name,
		                         const Category* parent = nullptr);


	private:
		/** Vector of GeoFeatures */
		std::vector<GeoFeature*> _features;

		/** Vector of Categories */
		std::vector<Category*> _categories;

		typedef std::vector<GeoFeatureSetObserver*> ObserverList;
		ObserverList _observers;
};


class SC_SYSTEM_CORE_API GeoFeatureSetSingleton {
	public:
		/** Returns the singleton instance of this class */
		static GeoFeatureSet &getInstance();

	private:
		/** Default constructor */
		GeoFeatureSetSingleton();

	private:
		GeoFeatureSet _geoFeatureSet;
};


} // of ns Geo
} // of ns Seiscomp


#endif // SEISCOMP_GEO_FEATURESET_H__
