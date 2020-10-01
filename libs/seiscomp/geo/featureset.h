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
		 */
		size_t readBNADir(const std::string& dirPath);

		/** Reads one BNA-file */
		bool readBNAFile(const std::string& filename, const Category* category);

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

		/** Prints the number of polygons read */
		const std::string initStatus(const std::string &directory,
		                             unsigned int fileCount) const;

		/** Reads the BNA-header */
		bool readBNAHeader(std::string &segment, unsigned int &rank,
		                   GeoFeature::Attributes &attributes,
		                   unsigned int &points, bool &isClosed, std::string &error,
		                   const std::string &line) const;

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
