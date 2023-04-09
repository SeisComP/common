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

#ifndef SEISCOMP_SEISMOLOGY_LOCATOR_STDLOC_H
#define SEISCOMP_SEISMOLOGY_LOCATOR_STDLOC_H

#include <seiscomp/core/plugin.h>
#include <seiscomp/seismology/locatorinterface.h>


namespace {


class StdLoc : public Seiscomp::Seismology::LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		StdLoc() = default;

		//! D'tor
		~StdLoc() = default;

	// ----------------------------------------------------------------------
	//  Locator interface implementation
	// ----------------------------------------------------------------------
	public:
	//! Initializes the locator.
	virtual bool init(const Seiscomp::Config::Config &config) override;

	//! Returns supported parameters to be changed.
	virtual IDList parameters() const override { return _allowedParameters; }

	//! Returns the value of a parameter.
	virtual std::string parameter(const std::string &name) const override;

	//! Sets the value of a parameter.
	virtual bool setParameter(const std::string &name,
							const std::string &value) override;

	//! List available profiles
	virtual IDList profiles() const override;

	//! specify the profile to be used
	virtual void setProfile(const std::string &name) override;

	//! Returns the implementations capabilities
	virtual int capabilities() const override {
	return InitialLocation | IgnoreInitialLocation;
	}

	virtual Seiscomp::DataModel::Origin *locate(PickList &pickList) override;
	virtual Seiscomp::DataModel::Origin *
	locate(PickList &pickList, double initLat, double initLon, double initDepth,
		 const Seiscomp::Core::Time &initTime) override;
	virtual Seiscomp::DataModel::Origin *
	relocate(const Seiscomp::DataModel::Origin *origin) override;

	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:

		// Covariance Matrix (X axis as W-E and Y axis as S-N)
		// Since it is a diagonal matrix the duplicated values
		// are commented
		struct CovMtrx {
			bool valid;
			double   sxx,  sxy,  sxz,  sxt;
			double /*syx,*/syy,  syz,  syt;
			double /*szx,  szy,*/szz,  szt;
			double /*stx,  sty,  stz,*/stt;
		};

		struct Cell {
			bool valid;
			double x, y, z;
			struct {
				double depth, lat, lon;
				Seiscomp::Core::Time time;
				double error;
			} org;
		};

		bool loadTTT();

		void computeAdditionlPickInfo(const PickList &pickList,
		                              std::vector<double> &weights,
		                              std::vector<double> &sensorLat,
		                              std::vector<double> &sensorLon,
		                              std::vector<double> &sensorElev) const;

		void locateGridSearch(const PickList &pickList,
		                      const std::vector<double> &weights,
		                      const std::vector<double> &sensorLat,
		                      const std::vector<double> &sensorLon,
		                      const std::vector<double> &sensorElev, double &newLat,
		                      double &newLon, double &newDepth,
		                      Seiscomp::Core::Time &newTime,
		                      std::vector<double> &travelTimes,
		                      CovMtrx& covm, bool computeCovMtrx) const;

		void locateLeastSquares(const PickList &pickList,
		                        const std::vector<double> &weights,
		                        const std::vector<double> &sensorLat,
		                        const std::vector<double> &sensorLon,
		                        const std::vector<double> &sensorElev, double initLat,
		                        double initLon, double initDepth,
		                        Seiscomp::Core::Time initTime, double &newLat,
		                        double &newLon, double &newDepth,
		                        Seiscomp::Core::Time &newTime,
		                        std::vector<double> &travelTimes,
		                        CovMtrx& covm, bool computeCovMtrx) const;

		void computeCovarianceMatrix(const std::vector<Cell>& cells,
		                             const Cell& bestCell,
		                             CovMtrx& covmOut) const;
		void computeCovarianceMatrix(const System& eq, CovMtrx& covmOut) const;

		Seiscomp::DataModel::Origin *
		createOrigin(const PickList &pickList,
		             const std::vector<double> &weights,
		             const std::vector<double> &sensorLat,
		             const std::vector<double> &sensorLon,
		             const std::vector<double> &sensorElev,
		             double originLat, double originLon, double originDepth,
		             const Seiscomp::Core::Time &originTime,
		             const std::vector<double> &travelTimes,
		             const CovMtrx& covm) const;

		struct Profile {
			std::string name;

			enum class Method {
				LeastSquares,
				GridSearch,
				GridAndLsqr,
			} method;

			std::string tttType;
			std::string tttModel;
			bool PSTableOnly;
			bool usePickUncertainties;
			std::vector<double> pickUncertaintyClasses;
			bool enableConfidenceEllipsoid;
			double confLevel;

			struct {
				double autoLatLon;
				double originLat;
				double originLon;
				double originDepth; // km
				double xExtent;     // km
				double yExtent;     // km
				double zExtent;     // km
				double cellXExtent; // km
				double cellYExtent; // km
				double cellZExtent; // km
				std::string errorType;
				double maxRms;
			} gridSearch;

			struct {
				int iterations;
				double dampingFactor;
				std::string solverType;
			} leastSquare;
		};

		Profile _currentProfile;
		std::map<std::string, Profile> _profiles;

		Seiscomp::TravelTimeTableInterfacePtr _ttt;
		std::string _tttType;  // currently loaded _ttt
		std::string _tttModel; // currently loaded _ttt

		static const IDList _allowedParameters;
};


} // namespace


#endif
