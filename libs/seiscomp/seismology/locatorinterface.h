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



#ifndef SEISCOMP_SEISMOLOGY_LOCATORINTERFACE_H
#define SEISCOMP_SEISMOLOGY_LOCATORINTERFACE_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/config/config.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/core.h>


#define SC3_LOCATOR_INTERFACE_VERSION 2

/******************************************************************************
 API Changelog
 ******************************************************************************
 2
   - First defined version
   - Replaced WeightedPick with PickItem which allows not to only use a binary
     weight but flags to enable/disable the time, backazimuth and/or slowness
 */

namespace Seiscomp {
namespace Seismology {


class PickNotFoundException;
class LocatorException;
class StationNotFoundException;


DEFINE_SMARTPOINTER(SensorLocationDelegate);

class SC_SYSTEM_CORE_API SensorLocationDelegate : public Core::BaseObject {
	public:
		SensorLocationDelegate();

	public:
		virtual DataModel::SensorLocation* getSensorLocation(DataModel::Pick *pick) const = 0;
};


DEFINE_SMARTPOINTER(LocatorInterface);


/**
 * @brief The LocatorInterface class defines an abstract interface to
 *        various locators such as LOCSAT, Hypo71 or NonLinLoc.
 *
 * An instance of an implementation of this class must be reentrant, meaning
 * that two instances are not interfering with each other when either used
 * from within different threads or interleaved. The requirement is illustrated
 * in the following code:
 *
 * @code
 * // Run two instances in one thread
 * Locator l1, l2;
 * l1.init(cfg);
 * l2.init(cfg);
 * l1.setProfile("profile1");
 * l2.setProfile("profile2");
 * org1 = l1.locate(org1);
 * org2 = l2.locate(org2);
 @ @endcode
 *
 * This code must produce the exactly same output as
 *
 * @code
 * Locator l1, l2;
 * l1.init(cfg);
 * l1.setProfile("profile1");
 * org1 = l1.locate(org1);
 *
 * l2.init(cfg);
 * l2.setProfile("profile2");
 * org2 = l2.locate(org2);
 * @endcode
 *
 * Involving threads looks like this
 *
 * @code
 * void locate() {
 *     Locator l;
 *     l.init(cfg);
 *     l.setProfile("someProfile");
 *     org = l.locate(org);
 * }
 *
 * thread1 = thread(locate);
 * thread2 = thread(locate);
 * thread1.join();
 * thread2.join();
 * @endcode
 *
 * Note that this does not require the code to be thread-safe. Calling
 * locate() on the same instance from within two different threads is not
 * required to work.
 */
class SC_SYSTEM_CORE_API LocatorInterface : public Core::BaseObject {
	public:
		MAKEENUM(
			Flags,
			EVALUES(
				F_NONE        = 0x00,
				F_BACKAZIMUTH = 0x01,
				F_SLOWNESS    = 0x02,
				F_TIME        = 0x04,
				F_ALL         = F_BACKAZIMUTH | F_SLOWNESS | F_TIME
			),
			ENAMES(
				"None",
				"Backazimuth",
				"Horizontal slowness",
				"Time",
				"All"
			)
		);

		enum Capability {
			NoCapability          = 0x0000,
			InitialLocation       = 0x0001,
			FixedDepth            = 0x0002,
			DistanceCutOff        = 0x0004,
			IgnoreInitialLocation = 0x0008,
			CapQuantity
		};

		enum MessageType {
			Log,
			Warning
		};

		struct PickItem {
			PickItem(DataModel::Pick *pick = nullptr, int f = F_ALL)
			: pick(pick), flags(f) {}
			PickItem(DataModel::PickPtr pick, int f = F_ALL)
			: pick(pick), flags(f) {}

			DataModel::PickPtr    pick;
			int                   flags;
		};

		typedef std::vector<PickItem> PickList;
		typedef std::vector<std::string> IDList;
		typedef std::map<std::string, std::string> ParameterMap;


	public:
		LocatorInterface();
		virtual ~LocatorInterface();


	public:
		static LocatorInterface *Create(const char *algo);

		//! Returns the name of the locator, e.g. LocSAT or NonLinLoc
		const std::string &name() const;

		//! Sets a delegate which returns a sensor location of
		//! a pick. If no sensor location delegate is set the
		//! default query will be used instead.
		void setSensorLocationDelegate(SensorLocationDelegate *delegate);

		//! Initialize the configuration
		virtual bool init(const Config::Config &config) = 0;

		//! Returns supported parameters to be changed. The default
		//! implementation returns an empty list.
		virtual IDList parameters() const;

		//! Returns the value of a parameter. The default implementation
		//! returns always an empty string.
		virtual std::string parameter(const std::string &name) const;

		//! Sets the value of a parameter. The default implementation
		//! returns always false.
		virtual bool setParameter(const std::string &name,
		                          const std::string &value);

		//! Returns supported locator profiles (velocity models, ...)
		virtual IDList profiles() const = 0;

		//! specify the Earth model to be used, e.g. "iasp91"
		virtual void setProfile(const std::string &name) = 0;

		//! Returns the implementations capabilities
		virtual int capabilities() const = 0;

		//! the following all return nullptr if (re)location failed
		virtual DataModel::Origin *locate(PickList& pickList) = 0;
		virtual DataModel::Origin *locate(PickList& pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  const Core::Time &initTime) = 0;
		virtual DataModel::Origin *relocate(const DataModel::Origin *origin) = 0;

		//! Returns a string (optional) valid for the last
		//! (re)locate call. Supported are log and warning
		//! messages. Errors are reported by throwing an exception.
		virtual std::string lastMessage(MessageType) const;

		//! queries for a set capability
		bool supports(Capability) const;

		//! Fixes the depth in km:
		void setFixedDepth(double depth, bool use=true);
		void useFixedDepth(bool use=true);
		double fixedDepth() const { return _fixedDepth; }
		bool usingFixedDepth() const { return _usingFixedDepth; }
		void releaseDepth();

		//! Sets the distance cut-off in km
		void setDistanceCutOff(double distance);
		void releaseDistanceCutOff();

		bool isInitialLocationIgnored() const { return _ignoreInitialLocation; }
		void setIgnoreInitialLocation(bool f) { _ignoreInitialLocation = f; }


	public:
		//! Finds the pick referenced by an arrival
		//! The default implementation looks up the global
		//! objects pool with publicID = arrival->pickID()
		virtual DataModel::Pick *getPick(DataModel::Arrival *arrival) const;

		//! Find the station referenced by a pick
		//! The default implementation looks up the global inventory
		//! and tries to find the station there if no sensor location
		//! delegate is set. Otherwise it queries for the station there.
		virtual DataModel::SensorLocation* getSensorLocation(DataModel::Pick *pick) const;


	protected:
		std::string               _name;
		SensorLocationDelegatePtr _sensorLocationDelegate;
		bool                      _usingFixedDepth;
		double                    _fixedDepth;
		bool                      _enableDistanceCutOff;
		double                    _distanceCutOff;
		bool                      _ignoreInitialLocation;
};


class SC_SYSTEM_CORE_API PickNotFoundException : public Core::GeneralException {
	public:
		PickNotFoundException();
		PickNotFoundException(const std::string& str);
};

class SC_SYSTEM_CORE_API LocatorException : public Core::GeneralException {
	public:
		LocatorException();
		LocatorException(const std::string& str);
};

class SC_SYSTEM_CORE_API StationNotFoundException : public Core::GeneralException {
	public:
		StationNotFoundException();
		StationNotFoundException(const std::string& str);
};


/**
 * @brief Extracts arrival information to create the usage flags. In particular
 *        Arrival.timeUsed, Arrival.horizontalSlownessUsed and
 *        Arrival.backazimuthUsed are evaluated. An unset value means 'used'.
 *        Furthermore the final flags are update according to the arrival
 *        weight. So a weight of 0 will unset all flags.
 * @param arrival The arrival to extract the flags from
 * @return Flags
 */
int arrivalToFlags(const DataModel::Arrival *arrival);

/**
 * @brief Applies locator flags to an arrival.
 * @param arrival The arrival to be updated
 * @param flags The usage flags
 */
void flagsToArrival(DataModel::Arrival *arrival, int flags);


DEFINE_INTERFACE_FACTORY(LocatorInterface);


} // of namespace Seismology
} // of namespace Seiscomp


#define REGISTER_LOCATOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Seismology::LocatorInterface, Class> __##Class##InterfaceFactory__(Service)


#endif
