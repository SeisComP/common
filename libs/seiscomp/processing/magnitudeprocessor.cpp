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
#include <seiscomp/utils/units.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/system/environment.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/sensorlocation.h>

#include <cstring>
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
mutex regionalizationRegistryMutex;


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


AliasFactories aliasFactories;


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
	if ( !Processor::setup(settings) )
		return false;

	_networkCode = settings.networkCode;
	_stationCode = settings.stationCode;

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
						SEISCOMP_ERROR("%s.%s: Only one default linear correction allowed: %s",
						               settings.networkCode.c_str(),
						               settings.stationCode.c_str(),
						               strLinearCorrections.c_str());
						return false;
					}

					if ( !Core::fromString(_defaultCorrection.first, tok) ) {
						SEISCOMP_ERROR("%s.%s: Invalid linear correction value: %s",
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
					SEISCOMP_ERROR("%s.%s: Invalid linear correction: %s",
					               settings.networkCode.c_str(),
					               settings.stationCode.c_str(),
					               tok.c_str());
					return false;
				}

				string name = tok.substr(0, p);
				Core::trim(name);
				if ( name.empty() ) {
					SEISCOMP_ERROR("%s.%s: Empty linear correction profiles not allowed: %s",
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
						SEISCOMP_ERROR("%s.%s: Only one default constant correction allowed: %s",
						               settings.networkCode.c_str(),
						               settings.stationCode.c_str(),
						               strConstantCorrections.c_str());
						return false;
					}

					if ( !Core::fromString(_defaultCorrection.second, tok) ) {
						SEISCOMP_ERROR("%s.%s: Invalid constant correction value: %s",
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
					SEISCOMP_ERROR("%s.%s: Invalid constant correction: %s",
					               settings.networkCode.c_str(),
					               settings.stationCode.c_str(),
					               tok.c_str());
					return false;
				}

				string name = tok.substr(0, p);
				Core::trim(name);
				if ( name.empty() ) {
					SEISCOMP_ERROR("%s.%s: Empty constant correction profiles not allowed: %s",
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
		lock_guard<mutex> l(regionalizationRegistryMutex);

		TypeSpecificRegionalizationPtr regionalizedSettings;
		auto it = regionalizationRegistry.find(type());
		if ( it != regionalizationRegistry.end() and it->second ) {
			// Setup region specific corrections
			for ( const Locale &cfg : it->second->regionalization ) {
				auto it1 = linearCorrections.find(cfg.name);
				auto it2 = constantCorrections.find(cfg.name);

				if ( it1 == linearCorrections.end() and it2 == constantCorrections.end() )
					continue;

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

	try { locale->minimumDistance  = cfg->getDouble(cfgPrefix + "minDist"); } catch ( ... ) {}
	try { locale->maximumDistance  = cfg->getDouble(cfgPrefix + "maxDist"); } catch ( ... ) {}
	try { locale->minimumDepth = cfg->getDouble(cfgPrefix + "minDepth"); } catch ( ... ) {}
	try { locale->maximumDepth = cfg->getDouble(cfgPrefix + "maxDepth"); } catch ( ... ) {}
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

	if ( !initLocale(locale, settings, cfgPrefix) )
		return false;

	SEISCOMP_DEBUG("%s: + region %s", _type.c_str(), locale->name.c_str());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor::initRegionalization(const Settings &settings) {
	lock_guard<mutex> l(regionalizationRegistryMutex);

	TypeSpecificRegionalizationPtr regionalizedSettings;
	auto it = regionalizationRegistry.find(type());
	if ( it == regionalizationRegistry.end() ) {
		regionalizedSettings = new TypeSpecificRegionalization();
		regionalizationRegistry[type()] = nullptr;

		const Seiscomp::Config::Config *cfg = settings.localConfiguration;
		if ( cfg ) {
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
					if ( feature->name().empty() ) continue;
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

					if ( !readLocale(&config, settings, cfgPrefix) )
						return false;

					regionalizedSettings->regionalization.push_back(config);
				}

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
			catch ( ... ) {}
		}

		regionalizationRegistry[type()] = regionalizedSettings;
	}
	else
		regionalizedSettings = it->second;

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

	// Check if regionalization is desired
	{
		lock_guard<mutex> l(regionalizationRegistryMutex);
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

					// Found region
					locale = &profile;

					if ( locale->minimumDistance and delta < *locale->minimumDistance )
						return DistanceOutOfRange;
					if ( locale->maximumDistance and delta > *locale->maximumDistance )
						return DistanceOutOfRange;

					if ( locale->minimumDepth and depth < *locale->minimumDepth )
						return DepthOutOfRange;
					if ( locale->maximumDepth and depth > *locale->maximumDepth )
						return DepthOutOfRange;

					break;
				}

				if ( !locale ) {
					return notFoundStatus;
				}
			}
		}
	}

	if ( locale )
		SEISCOMP_DEBUG("%s.%s: %s: locale = '%s'",
		               _networkCode.c_str(), _stationCode.c_str(), _type.c_str(),
		               locale->name.c_str());

	auto r = computeMagnitude(amplitudeValue, unit, period, snr, delta, depth,
	                          hypocenter, receiver, amplitude, locale, value);

	if ( r != OK )
		return r;

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

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor::estimateMw(
	double ,
	double &,
	double &)
{
	return MwEstimationNotSupported;
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
	if ( uc == nullptr ) {
		// No conversion known, invalid amplitude unit
		return false;
	}

	// Convert to SI
	double amplitudeSI = uc->convert(amplitude);

	uc = Util::UnitConverter::get(desiredAmplitudeUnit);
	if ( uc == nullptr ) {
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
	return false;
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
