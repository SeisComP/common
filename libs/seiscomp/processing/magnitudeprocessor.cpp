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


#define SEISCOMP_COMPONENT MagnitudeProcessor

#include <seiscomp/logging/log.h>
#include <seiscomp/processing/regions.h>
#include <seiscomp/processing/magnitudeprocessor.h>
#include <seiscomp/processing/magnitudes/utils.h>
#include <seiscomp/utils/units.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/system/environment.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/sensorlocation.h>

#include <cstring>
#include <map>
#include <mutex>


using namespace std;


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Processing::MagnitudeProcessor, SC_SYSTEM_CLIENT_API);


namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(MagnitudeProcessor, Core::BaseObject, "MagnitudeProcessor");


class MagnitudeProcessorAliasFactory : public Core::Generic::InterfaceFactoryInterface<MagnitudeProcessor> {
	public:
		MagnitudeProcessorAliasFactory(
			const std::string &service,
			const std::string &ampType,
			const Core::Generic::InterfaceFactoryInterface<MagnitudeProcessor> *source
		)
		: Core::Generic::InterfaceFactoryInterface<MagnitudeProcessor>(service.c_str())
		, _source(source)
		, _ampType(ampType) {}

		MagnitudeProcessor *create() const {
			auto proc = _source->create();
			if ( proc->type() != serviceName() ) {
				proc->_type = serviceName();
				if ( !_ampType.empty() ) {
					proc->_amplitudeType = _ampType;
				}
			}
			return proc;
		}

	private:
		const Core::Generic::InterfaceFactoryInterface<MagnitudeProcessor> *_source;
		string _ampType;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


using Regionalization = vector<MagnitudeProcessor::Locale>;


DEFINE_SMARTPOINTER(TypeSpecificRegionalization);
class TypeSpecificRegionalization : public Core::BaseObject {
	public:
		const Regions   *regions;
		Regionalization  regionalization;
};

using RegionalizationRegistry = map<string, TypeSpecificRegionalizationPtr>;
RegionalizationRegistry regionalizationRegistry;


class AliasFactories : public std::vector<MagnitudeProcessorAliasFactory*> {
	public:
		~AliasFactories() {
			for ( auto f : *this ) {
				delete f;
			}
		}

		bool createAlias(const std::string &aliasType,
		                 const std::string &sourceType,
		                 const std::string &sourceAmpType) {
			auto sourceFactory = MagnitudeProcessorFactory::Find(sourceType);
			if ( !sourceFactory ) {
				SEISCOMP_ERROR("alias: magnitude source factory '%s' does not exist",
				               sourceType.c_str());
				return false;
			}

			auto factory = MagnitudeProcessorFactory::Find(aliasType);
			if ( factory ) {
				SEISCOMP_ERROR("alias: magnitude alias type '%s' is already registered",
				               aliasType.c_str());
				return false;
			}

			push_back(
				new MagnitudeProcessorAliasFactory(
					aliasType, sourceAmpType, sourceFactory
				)
			);

			return true;
		}

		bool removeAlias(const std::string &aliasType) {
			auto factory = MagnitudeProcessorFactory::Find(aliasType);
			if ( !factory ) {
				SEISCOMP_ERROR("alias: magnitude alias type '%s' does not exist",
				               aliasType.c_str());
				return false;
			}

			delete factory;

			auto it = find(begin(), end(), factory);
			if ( it != end() ) {
				erase(it);
			}

			return true;
		}

		void clear() {
			for ( auto f : *this ) {
				delete f;
			}
			std::vector<MagnitudeProcessorAliasFactory*>::clear();
		}
};


map<string, OPT(TableXY<double>)> MwTables;
AliasFactories aliasFactories;
mutex globalParameterMutex;


template <typename T, typename CFG>
void readValue(T &value, const CFG *cfg, const std::string &var, const string &unit) {
	try {
		auto s = cfg->getString(var);
		try {
			value = Util::UnitConverter::parse<double>(s, unit);
		}
		catch ( exception &e ) {
			SEISCOMP_ERROR("%s: invalid value: %s", var, e.what());
			throw e;
		}
	}
	catch ( ... ) {}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MagnitudeProcessor::Correction::A *
MagnitudeProcessor::Correction::apply(double &val, const std::string &profile) const {
	auto it = profiles.find(profile);
	if ( it == profiles.end() ) return nullptr;
	val = it->second.first * val + it->second.second;
	return &it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::MagnitudeProcessor(const std::string& type)
: _type(type)
, _amplitudeType(type) {
	setCorrectionCoefficients(1.0, 0.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::~MagnitudeProcessor() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &MagnitudeProcessor::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor::typeMw() const {
	return "Mw(" + _type + ")";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor::amplitudeType() const {
	return _amplitudeType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::setup(const Settings &settings) {
	_minimumDistanceDeg = Core::None;
	_maximumDistanceDeg = Core::None;
	_minimumDepthKm = Core::None;
	_maximumDepthKm = Core::None;
	_minimumSNR = Core::None;
	_minimumPeriod = Core::None;
	_maximumPeriod = Core::None;

	setDefaults();

	if ( !Processor::setup(settings) ) {
		return false;
	}

	_networkCode = settings.networkCode;
	_stationCode = settings.stationCode;
	_locationCode = settings.locationCode;

	if ( !initRegionalization(settings) )
		return false;

	// Read station specific corrections
	// Read regionalized corrections
	map<string, double> linearCorrections;
	map<string, double> constantCorrections;

	_defaultCorrection = Correction::A(1.0, 0.0);

	{
		string strLinearCorrections;
		try {
			strLinearCorrections = settings.getString("magnitudes." + type() + ".multiplier");
		}
		catch ( ... ) {
			try {
				strLinearCorrections = settings.getDouble("mag." + type() + ".multiplier");
				SEISCOMP_WARNING("mag.%s.multiplier is deprecated", type().c_str());
				SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.multiplier",
				                 type().c_str());
			}
			catch ( ... ) {}
		}

		if ( !strLinearCorrections.empty() ) {
			vector<string> toks;
			Core::split(toks, strLinearCorrections.c_str(), ",", true);
			bool hasDefault = false;

			for ( const string &tok : toks ) {
				size_t p = tok.find(':');

				if ( p == string::npos ) {
					if ( hasDefault ) {
						SEISCOMP_ERROR("%s/%s.%s: Only one default linear correction allowed: %s",
						               _type.c_str(),
						               settings.networkCode.c_str(),
						               settings.stationCode.c_str(),
						               strLinearCorrections.c_str());
						return false;
					}

					if ( !Core::fromString(_defaultCorrection.first, tok) ) {
						SEISCOMP_ERROR("%s/%s.%s: Invalid linear correction value: %s",
						               _type.c_str(),
						               settings.networkCode.c_str(),
						               settings.stationCode.c_str(),
						               tok.c_str());
						return false;
					}

					hasDefault = true;
					continue;
				}

				double value;
				if ( !Core::fromString(value, tok.substr(p+1)) ) {
					SEISCOMP_ERROR("%s/%s.%s: Invalid linear correction: %s",
					               _type.c_str(),
					               settings.networkCode.c_str(),
					               settings.stationCode.c_str(),
					               tok.c_str());
					return false;
				}

				string name = tok.substr(0, p);
				Core::trim(name);
				if ( name.empty() ) {
					SEISCOMP_ERROR("%s/%s.%s: Empty linear correction profiles not allowed: %s",
					               _type.c_str(),
					               settings.networkCode.c_str(),
					               settings.stationCode.c_str(),
					               tok.c_str());
					return false;
				}

				linearCorrections[name] = value;
			}
		}
	}

	{
		string strConstantCorrections;
		try {
			strConstantCorrections = settings.getString("magnitudes." + type() + ".offset");
		}
		catch ( ... ) {
			try {
				strConstantCorrections = settings.getDouble("mag." + type() + ".offset");
				SEISCOMP_WARNING("mag.%s.offset is deprecated", type().c_str());
				SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.offset",
				                 type().c_str());
			}
			catch ( ... ) {}
		}

		if ( !strConstantCorrections.empty() ) {
			vector<string> toks;
			Core::split(toks, strConstantCorrections.c_str(), ",", true);
			bool hasDefault = false;

			for ( const string &tok : toks ) {
				size_t p = tok.find(':');

				if ( p == string::npos ) {
					if ( hasDefault ) {
						SEISCOMP_ERROR("%s/%s.%s: Only one default constant correction allowed: %s",
						               _type.c_str(),
						               settings.networkCode.c_str(),
						               settings.stationCode.c_str(),
						               strConstantCorrections.c_str());
						return false;
					}

					if ( !Core::fromString(_defaultCorrection.second, tok) ) {
						SEISCOMP_ERROR("%s/%s.%s: Invalid constant correction value: %s",
						               _type.c_str(),
						               settings.networkCode.c_str(),
						               settings.stationCode.c_str(),
						               tok.c_str());
						return false;
					}

					hasDefault = true;
					continue;
				}

				double value;
				if ( !Core::fromString(value, tok.substr(p+1)) ) {
					SEISCOMP_ERROR("%s/%s.%s: Invalid constant correction: %s",
					               _type.c_str(),
					               settings.networkCode.c_str(),
					               settings.stationCode.c_str(),
					               tok.c_str());
					return false;
				}

				string name = tok.substr(0, p);
				Core::trim(name);
				if ( name.empty() ) {
					SEISCOMP_ERROR("%s/%s.%s: Empty constant correction profiles not allowed: %s",
					               _type.c_str(),
					               settings.networkCode.c_str(),
					               settings.stationCode.c_str(),
					               tok.c_str());
					return false;
				}

				constantCorrections[name] = value;
			}
		}
	}

	{
		lock_guard<mutex> l(globalParameterMutex);

		auto it = regionalizationRegistry.find(type());
		if ( it != regionalizationRegistry.end() and it->second ) {
			// Setup region specific corrections
			for ( const Locale &cfg : it->second->regionalization ) {
				auto it1 = linearCorrections.find(cfg.name);
				auto it2 = constantCorrections.find(cfg.name);

				if ( it1 == linearCorrections.end() and it2 == constantCorrections.end() ) {
					continue;
				}

				Correction::A c(cfg.multiplier, cfg.offset);
				if ( it1 != linearCorrections.end() ) {
					c.first = it1->second;
					linearCorrections.erase(it1);
				}

				if ( it2 != constantCorrections.end() ) {
					c.second = it2->second;
					constantCorrections.erase(it2);
				}

				_corrections[cfg.name] = c;
			}
		}
	}

	try { readValue(_minimumDistanceDeg, &settings, "magnitudes." + type() + ".minDist", "deg"); }
	catch ( ... ) { return false; }
	try { readValue(_maximumDistanceDeg, &settings, "magnitudes." + type() + ".maxDist", "deg"); }
	catch ( ... ) { return false; }

	try { readValue(_minimumDepthKm, &settings, "magnitudes." + type() + ".minDepth", "km"); }
	catch ( ... ) { return false; }
	try { readValue(_maximumDepthKm, &settings, "magnitudes." + type() + ".maxDepth", "km"); }
	catch ( ... ) { return false; }

	try { _minimumSNR = settings.getDouble("magnitudes." + type() + ".minSNR"); }
	catch ( ... ) {}

	try { _minimumPeriod = settings.getDouble("magnitudes." + type() + ".minPeriod"); }
	catch ( ... ) {}
	try { _maximumPeriod = settings.getDouble("magnitudes." + type() + ".maxPeriod"); }
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::initLocale(Locale *, const Settings &, const string &) {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::readLocale(Locale *locale,
                                    const Settings &settings,
                                    const std::string &cfgPrefix) {
	locale->check = Locale::Source;
	locale->multiplier = 1.0;
	locale->offset = 0.0;

	const Config::Config *cfg = settings.localConfiguration;

	try { readValue(locale->minimumDistanceDeg, cfg, cfgPrefix + "minDist", "deg"); }
	catch ( ... ) { return false; }
	try { readValue(locale->maximumDistanceDeg, cfg, cfgPrefix + "maxDist", "deg"); }
	catch ( ... ) { return false; }
	try { readValue(locale->minimumDepthKm, cfg, cfgPrefix + "minDepth", "km"); }
	catch ( ... ) { return false; }
	try { readValue(locale->maximumDepthKm, cfg, cfgPrefix + "maxDepth", "km"); }
	catch ( ... ) { return false; }

	try { locale->multiplier = cfg->getDouble(cfgPrefix + "multiplier"); } catch ( ... ) {}
	try { locale->offset = cfg->getDouble(cfgPrefix + "offset"); } catch ( ... ) {}

	try {
		string check = cfg->getString(cfgPrefix + "check");
		if ( check == "source" ) {
			locale->check = Locale::Source;
		}
		else if ( check == "source-receiver" ) {
			locale->check = Locale::SourceReceiver;
		}
		else if ( check == "raypath" ) {
			locale->check = Locale::SourceReceiverPath;
		}
		else {
			SEISCOMP_ERROR("%scheck: invalid region check: %s",
			               cfgPrefix.c_str(), check.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	if ( !initLocale(locale, settings, cfgPrefix) ) {
		return false;
	}

	SEISCOMP_DEBUG("%s (locale)", _type.c_str());
	SEISCOMP_DEBUG("  + region: %s", locale->name.c_str());
	if ( locale->minimumDistanceDeg ) {
		SEISCOMP_DEBUG("  + minimum distance: %.3f", *locale->minimumDistanceDeg);
	}
	if ( locale->maximumDistanceDeg ) {
		SEISCOMP_DEBUG("  + maximum distance: %.3f", *locale->maximumDistanceDeg);
	}
	if ( locale->minimumDepthKm ) {
		SEISCOMP_DEBUG("  + minimum depth: %.3f", *locale->minimumDepthKm);
	}
	if ( locale->maximumDepthKm ) {
		SEISCOMP_DEBUG("  + maximum depth: %.3f", *locale->maximumDepthKm);
	}
	SEISCOMP_DEBUG("  + multiplier: %.3f", locale->multiplier);
	SEISCOMP_DEBUG("  + offset: %.3f", locale->offset);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::initRegionalization(const Settings &settings) {
	lock_guard<mutex> l(globalParameterMutex);

	TypeSpecificRegionalizationPtr regionalizedSettings;
	auto it = regionalizationRegistry.find(type());
	if ( it == regionalizationRegistry.end() ) {
		regionalizedSettings = new TypeSpecificRegionalization();
		regionalizationRegistry[type()] = nullptr;

		const Seiscomp::Config::Config *cfg = settings.localConfiguration;
		if ( cfg ) {
			try {
				if ( !cfg->getString("magnitudes." + type() + ".regions").empty() ) {
					SEISCOMP_WARNING("%s magnitude: ignoring obsolete "
					                 "configuration parameter: magnitudes.%s.regions",
					                 type().c_str(), type().c_str());
				}
			}
			catch ( ... ) {}

			try {
				string filename = cfg->getString("magnitudes." + type() + ".regionFile");
				filename = Seiscomp::Environment::Instance()->absolutePath(filename);
				regionalizedSettings->regions = Regions::load(filename);
				if ( !regionalizedSettings->regions ) {
					SEISCOMP_ERROR("Failed to read/parse %s regions file: %s",
					               type().c_str(), filename.c_str());
					return false;
				}

				for ( Geo::GeoFeature *feature : regionalizedSettings->regions->featureSet.features()) {
					if ( feature->name().empty() ) {
						continue;
					}

					if ( feature->name() == "world" ) {
						SEISCOMP_ERROR("Region name 'world' is not allowed as it is "
						               "reserved");
						return false;
					}

					string cfgPrefix = "magnitudes." + type() + ".region." + feature->name() + ".";
					try {
						if ( !cfg->getBool(cfgPrefix + "enable") ) {
							SEISCOMP_DEBUG("%s: - region %s (disabled)",
							               _type.c_str(), feature->name().c_str());
							continue;
						}
					}
					catch ( ... ) {
						SEISCOMP_DEBUG("%s: - region %s (disabled)",
						               _type.c_str(), feature->name().c_str());
						continue;
					}

					Locale config;
					config.name = feature->name();
					config.feature = feature;

					if ( !readLocale(&config, settings, cfgPrefix) ) {
						return false;
					}

					regionalizedSettings->regionalization.push_back(config);
				}
			}
			catch ( ... ) {}

			try {
				if ( cfg->getBool("magnitudes." + type() + ".region.world.enable") ) {
					string cfgPrefix = "magnitudes." + type() + ".region.world.";
					Locale config;
					config.name = "world";
					config.feature = nullptr;

					if ( !readLocale(&config, settings, cfgPrefix) )
						return false;

					regionalizedSettings->regionalization.push_back(config);
				}
			}
			catch ( ... ) {}
		}

		regionalizationRegistry[type()] = regionalizedSettings;
	}
	else {
		regionalizedSettings = it->second;
	}

	return regionalizedSettings ? true : false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status
MagnitudeProcessor::computeMagnitude(double amplitudeValue,
                                     const std::string &unit,
                                     double period, double snr,
                                     double delta, double depth,
                                     const DataModel::Origin *hypocenter,
                                     const DataModel::SensorLocation *receiver,
                                     const DataModel::Amplitude *amplitude,
                                     double &value) {
	const Locale *locale = nullptr;
	_treatAsValidMagnitude = false;

	if ( _minimumDepthKm && (depth < *_minimumDepthKm) ) {
		SEISCOMP_DEBUG("%s.%s: %s: depth out of range: %f < %f",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               depth, *_minimumDepthKm);
		return DepthOutOfRange;
	}

	if ( _maximumDepthKm && (depth > *_maximumDepthKm) ) {
		SEISCOMP_DEBUG("%s.%s: %s: depth out of range: %f > %f",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               depth, *_maximumDepthKm);
		return DepthOutOfRange;
	}

	// Check if regionalization is desired
	{
		lock_guard<mutex> l(globalParameterMutex);
		auto it = regionalizationRegistry.find(type());
		if ( it != regionalizationRegistry.end() ) {
			TypeSpecificRegionalization *tsr = it->second.get();
			if ( tsr and !tsr->regionalization.empty() ) {
				// There are region profiles so meta data are required
				if ( !hypocenter ) {
					return MetaDataRequired;
				}

				if ( !receiver ) {
					return MetaDataRequired;
				}

				double hypoLat, hypoLon;
				double recvLat, recvLon;

				try {
					// All attributes are optional and throw an exception if not set
					hypoLat = hypocenter->latitude().value();
					hypoLon = hypocenter->longitude().value();
				}
				catch ( ... ) {
					return MetaDataRequired;
				}

				try {
					// Both attributes are optional and throw an exception if not set
					recvLat = receiver->latitude();
					recvLon = receiver->longitude();
				}
				catch ( ... ) {
					return MetaDataRequired;
				}

				Status notFoundStatus = OK;

				for ( const Locale &profile : tsr->regionalization ) {
					if ( profile.feature ) {
						switch ( profile.check ) {
							case Locale::Source:
								if ( !profile.feature->contains(Geo::GeoCoordinate(hypoLat, hypoLon)) ) {
									notFoundStatus = EpicenterOutOfRegions;
									continue;
								}

								break;

							case Locale::SourceReceiver:
								if ( !profile.feature->contains(Geo::GeoCoordinate(hypoLat, hypoLon)) ) {
									notFoundStatus = EpicenterOutOfRegions;
									continue;
								}
								if ( !profile.feature->contains(Geo::GeoCoordinate(recvLat, recvLon)) ) {
									notFoundStatus = ReceiverOutOfRegions;
									continue;
								}
								break;

							case Locale::SourceReceiverPath:
								if ( !Regions::contains(profile.feature, hypoLat, hypoLon, recvLat, recvLon) ) {
									notFoundStatus = RayPathOutOfRegions;
									continue;
								}
								break;
						}
					}

					if ( profile.minimumDepthKm && depth < *profile.minimumDepthKm ) {
						notFoundStatus = DepthOutOfRange;
						continue;
					}
					if ( profile.maximumDepthKm && depth > *profile.maximumDepthKm ) {
						notFoundStatus = DepthOutOfRange;
						continue;
					}

					// Found region
					locale = &profile;

					if ( locale->minimumDistanceDeg && delta < *locale->minimumDistanceDeg ) {
						return DistanceOutOfRange;
					}
					if ( locale->maximumDistanceDeg && delta > *locale->maximumDistanceDeg ) {
						return DistanceOutOfRange;
					}

					break;
				}

				if ( !locale ) {
					return notFoundStatus;
				}
			}
		}
	}

	if ( locale ) {
		SEISCOMP_DEBUG("%s.%s: %s: locale = '%s'",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               locale->name.c_str());
	}

	auto r = computeMagnitude(amplitudeValue, unit, period, snr, delta, depth,
	                          hypocenter, receiver, amplitude, locale, value);

	if ( r != OK ) {
		return r;
	}

	/**
	 * ------------------------------------------------------------------------
	 * Station specific distance and depth checks
	 * ------------------------------------------------------------------------
	 *
	 * The distance checks will be performed after computing the magnitude so
	 * that a concrete implementation can still decide if it wants to keep the
	 * computed magnitude but with weight 0 and with passedQC flag set to
	 * false (_treatAsValidMagnitude = true).
	 * An example is to compute and keep all magnitudes at close distances
	 * but to not let them contribute to the network magnitude.
	 */

	if ( _minimumDistanceDeg && delta < *_minimumDistanceDeg ) {
		SEISCOMP_DEBUG("%s.%s: %s: station distance out of range: %f < %f",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               delta, *_minimumDistanceDeg);
		return DistanceOutOfRange;
	}

	if ( _maximumDistanceDeg && delta > *_maximumDistanceDeg ) {
		SEISCOMP_DEBUG("%s.%s: %s: station distance out of range: %f > %f",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               delta, *_maximumDistanceDeg);
		return DistanceOutOfRange;
	}

	if ( !locale ) {
		SEISCOMP_DEBUG("%s.%s: %s: effective correction (no locale) = %.2f:%.2f",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               _defaultCorrection.first, _defaultCorrection.second);
		value = _defaultCorrection.first * value + _defaultCorrection.second;
	}
	else {
		const Correction::A *corr = _corrections.apply(value, locale->name);
		if ( corr ) {
			SEISCOMP_DEBUG("%s.%s: %s: effective correction (regionalized binding) = %.2f:%.2f",
			               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
			               corr->first, corr->second);
		}
		else {
			SEISCOMP_DEBUG("%s.%s: %s: effective correction (region) = %.2f:%.2f",
			               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
			               locale->multiplier, locale->offset);
			value = locale->multiplier * value + locale->offset;
		}
	}

	/**
	 * ------------------------------------------------------------------------
	 * Quality checks
	 * ------------------------------------------------------------------------
	 *
	 * All quality checks will keep the computed magnitude but associated with
	 * weight 0 and with passedQC flag set to false. If amplitudes and
	 * magnitudes should be omitted completely then the corresponding amplitude
	 * should be configured to check those thresholds already.
	 */

	if ( _minimumSNR && snr < *_minimumSNR ) {
		r = SNROutOfRange;
		SEISCOMP_DEBUG("%s.%s: %s: SNR out of range: %f > %f: qc failed",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               snr, *_minimumSNR);
		_treatAsValidMagnitude = true;
	}

	if ( _minimumPeriod && period < *_minimumPeriod ) {
		r = PeriodOutOfRange;
		SEISCOMP_DEBUG("%s.%s: %s: period out of range: %f < %f: qc failed",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               period, *_minimumPeriod);
		_treatAsValidMagnitude = true;
	}
	else if ( _maximumPeriod && period > *_maximumPeriod ) {
		r = PeriodOutOfRange;
		SEISCOMP_DEBUG("%s.%s: %s: period out of range: %f > %f: qc failed",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               period, *_maximumPeriod);
		_treatAsValidMagnitude = true;
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor::estimateMw(
	const Config::Config *config,
	double magnitude,
	double &estimation,
	double &stdError)
{
	TableXY<double> *MwMapping{nullptr};

	{
		lock_guard<mutex> l(globalParameterMutex);

		auto it = MwTables.find(type());
		if ( it != MwTables.end() ) {
			if ( it->second ) {
				MwMapping = &*it->second;
			}
		}
		else if ( config ) {
			MwTables[type()] = Core::None;

			// Try to read it from configuration
			try {
				auto def = config->getStrings("magnitudes." + type() + ".MwMapping");
				TableXY<double> tmpMwMapping;

				if ( !tmpMwMapping.set(def) ) {
					SEISCOMP_ERROR("%s: Invalid Mw table: %s",
					               type().c_str(), Core::join(def, ", ").c_str());
				}
				else {
					SEISCOMP_DEBUG("%s: Mw table = %s", type().c_str(), Core::join(def, ", ").c_str());
					MwTables[type()] = tmpMwMapping;
					MwMapping = &*MwTables[type()];
				}
			}
			catch ( ... ) {}
		}
	}

	if ( !MwMapping ) {
		return MwEstimationNotSupported;
	}

	try {
		estimation = MwMapping->at(magnitude);
	}
	catch ( std::exception & ) {
		return AmplitudeOutOfRange;
	}

	stdError = -1;
	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor::setCorrectionCoefficients(double a, double b) {
	_defaultCorrection.first = a;
	_defaultCorrection.second = b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::convertAmplitude(double &amplitude,
                                          const std::string &amplitudeUnit,
                                          const std::string &desiredAmplitudeUnit) const {
	if ( amplitudeUnit.empty() or (amplitudeUnit == desiredAmplitudeUnit) ) {
		// No changes required
		return true;
	}

	const Util::UnitConversion *uc = Util::UnitConverter::get(amplitudeUnit);
	if ( !uc ) {
		// No conversion known, invalid amplitude unit
		return false;
	}

	// Convert to SI
	double amplitudeSI = uc->convert(amplitude);

	uc = Util::UnitConverter::get(desiredAmplitudeUnit);
	if ( !uc ) {
		SEISCOMP_ERROR("This must not happen: no converter for amplitude target unit '%s'",
		               desiredAmplitudeUnit.c_str());
		// This must not happen. The desired amplitude unit should always
		// have a mapping.
		return false;
	}

	double desiredAmplitude = uc->revert(amplitudeSI);

	SEISCOMP_DEBUG("Converted amplitude from %f %s to %f %s",
	               amplitude, amplitudeUnit.c_str(),
	               desiredAmplitude, desiredAmplitudeUnit.c_str());

	amplitude = desiredAmplitude;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::treatAsValidMagnitude() const {
	return _treatAsValidMagnitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor::finalizeMagnitude(DataModel::StationMagnitude *) const {
	// Nothing
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::CreateAlias(const std::string &aliasType,
                                     const std::string &sourceType,
                                     const string &sourceAmpType) {
	return aliasFactories.createAlias(aliasType, sourceType, sourceAmpType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::RemoveAlias(const std::string &aliasType) {
	return aliasFactories.removeAlias(aliasType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor::RemoveAllAliases() {
	aliasFactories.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
