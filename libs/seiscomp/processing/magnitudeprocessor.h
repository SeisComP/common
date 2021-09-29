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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core/enumeration.h>
#include <seiscomp/processing/processor.h>
#include <seiscomp/client.h>


namespace Seiscomp {

namespace DataModel {

class Amplitude;
class StationMagnitude;
class Origin;
class SensorLocation;

}

namespace Geo {

class GeoFeature;

}

namespace Processing {


DEFINE_SMARTPOINTER(MagnitudeProcessor);

class SC_SYSTEM_CLIENT_API MagnitudeProcessor : public Processor {
	DECLARE_SC_CLASS(MagnitudeProcessor)

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		MAKEENUM(
			Status,
			EVALUES(
				//! No error
				OK,
				//! Given amplitude is out of range
				AmplitudeOutOfRange,
				//! Given depth is out of range to continue processing
				DepthOutOfRange,
				//! Given distance is out of range to continue processing
				DistanceOutOfRange,
				//! Given period is out of range to continue processing
				PeriodOutOfRange,
				//! Given amplitude SNR is out of range to continue processing
				SNROutOfRange,
				//! Either the origin or the sensor location hasn't been set
				//! in call to computeMagnitude
				MetaDataRequired,
				//! The epicentre is out of supported regions
				EpicenterOutOfRegions,
				//! The receiver is out of supported regions
				ReceiverOutOfRegions,
				//! The entire raypath does not lie entirely in the supported
				//! regions
				RayPathOutOfRegions,
				//! The unit of the input amplitude was not understood
				InvalidAmplitudeUnit,
				//! The amplitude object was missing
				MissingAmplitudeObject,
				//! The estimation of the Mw magnitude is not supported
				MwEstimationNotSupported,
				//! The configuration is not complete
				IncompleteConfiguration,
				//! Unspecified error
				Error
			),
			ENAMES(
				"OK",
				"amplitude out of range",
				"depth out of range",
				"distance out of range",
				"period out of range",
				"signal-to-noise ratio out of range",
				"meta data required",
				"epicenter out of regions",
				"receiver out of regions",
				"ray path out of regions",
				"invalid amplitude unit",
				"missing amplitude object",
				"Mw estimation not supported",
				"configuration incomplete",
				"error"
			)
		);

		struct Locale {
			enum Check {
				Source,
				SourceReceiver,
				SourceReceiverPath
			};

			OPT(double)            minimumDistance;
			OPT(double)            maximumDistance;
			OPT(double)            minimumDepth;
			OPT(double)            maximumDepth;
			double                 multiplier;
			double                 offset;

			std::string            name;
			const Geo::GeoFeature *feature;
			Check                  check;
			Core::BaseObjectPtr    extra;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		MagnitudeProcessor();
		MagnitudeProcessor(const std::string& type);

		//! D'tor
		~MagnitudeProcessor();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the type of magnitude to be calculated
		const std::string &type() const;

		//! Returns the type of the Mw estimation
		//! The default implementation returns "Mw($type)"
		virtual std::string typeMw() const;

		/**
		 * Returns the amplitude type used as input for this magnitude
		 * processor.
		 * The default implementation returns type()
		 * @return 
		 */
		virtual std::string amplitudeType() const;

		virtual bool setup(const Settings &settings) override;

		/**
		 * @brief Computes the magnitude from an amplitude. The method signature
		 *        has changed with API version >= 11. Prior to that version,
		 *        hypocenter, receiver and amplitude were not present.
		 * @param amplitudeValue The amplitude value without unit. The unit is
		                         implicitly defined by the requested amplitude
		 *                       type.
		 * @param unit The unit of the amplitude.
		 * @param period The measured period of the amplitude in seconds.
		 * @param snr The measured SNR of the amplitude.
		 * @param delta The distance from the epicenter in degrees.
		 * @param depth The depth of the hypocenter in kilometers.
		 * @param hypocenter The optional origin which describes the hypocenter.
		 * @param receiver The sensor location meta-data of the receiver.
		 * @param amplitude The optional amplitude object from which the values
		 *                  were extracted.
		 * @param value The return value, the magnitude.
		 * @return The status of the computation.
		 */
		Status computeMagnitude(double amplitudeValue, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *amplitude,
		                        double &value);

		/**
		 * @brief When computeMagnitude return an error the computed magnitude
		 *        value might nevertheless contain a meaningful value. E.g. if
		 *        the distance is out of range according to the defined rules,
		 *        the computation for a lower distance might still result in
		 *        valid values. This function indicates whether the returned
		 *        value should be treated as valid magnitude or not, even if an
		 *        error was returned. This function's return value must not be
		 *        used when computeMagnitude returned OK. A valid return value
		 *        is only provided in case the computation failed. To make it
		 *        clear: this function can only be called after
		 *        computeMagnitude and only if computeMagnitude(...) != OK.
		 * @return Whether the computed magnitude is a valid value or has to be
		 *         ignored. The default implementation returns false.
		 */
		virtual bool treatAsValidMagnitude() const;

		/**
		 * @brief Estimates the Mw magnitude from a given magnitude. The
		 *        default implementation returns MwEstimationNotSupported.
		 * @param magnitude Input magnitude value.
		 * @param estimation Resulting Mw estimation.
		 * @param stdError Resulting standard error.
		 * @return The status of the computation.
		 */
		virtual Status estimateMw(double magnitude, double &estimation,
		                          double &stdError);

		void setCorrectionCoefficients(double a, double b);

		/**
		 * @brief Allows to finalize a magnitude object as created by
		 *        client code.
		 *
		 * This method will usually be called right before the magnitude will
		 * be stored or sent and inside the emit handler. It allows processors
		 * to set specific attributes or to add comments.
		 * The default implementation does nothing.
		 * @param magnitude The magnitude to be finalized
		 */
		virtual void finalizeMagnitude(DataModel::StationMagnitude *magnitude) const;


	protected:
		/**
		 * @brief Computes the magnitude from an amplitude. The method signature
		 *        has changed with API version >= 11. Prior to that version,
		 *        hypocenter, receiver and amplitude were not present.
		 * @param amplitudeValue The amplitude value without unit. The unit is
		                         implicitly defined by the requested amplitude
		 *                       type.
		 * @param unit The unit of the amplitude.
		 * @param period The measured period of the amplitude in seconds.
		 * @param snr The measured SNR of the amplitude.
		 * @param delta The distance from the epicenter in degrees.
		 * @param depth The depth of the hypocenter in kilometers.
		 * @param hypocenter The optional origin which describes the hypocenter.
		 * @param receiver The sensor location meta-data of the receiver.
		 * @param amplitude The optional amplitude object from which the values
		 *                  were extracted.
		 * @param value The return value, the magnitude.
		 * @return The status of the computation.
		 */
		virtual Status computeMagnitude(double amplitudeValue, const std::string &unit,
		                                double period, double snr,
		                                double delta, double depth,
		                                const DataModel::Origin *hypocenter,
		                                const DataModel::SensorLocation *receiver,
		                                const DataModel::Amplitude *amplitude,
		                                const Locale *locale,
		                                double &value) = 0;

		virtual bool initLocale(Locale *locale,
		                        const Settings &settings,
		                        const std::string &configPrefix);

		bool initRegionalization(const Settings &settings);

		/**
		 * @brief Converts an amplitude value in an input unit to a value in
		 *        an output unit, e.g. mm/s -> nm/s.
		 * @param amplitude The input value which will be changed
		 * @param amplitudeUnit The unit associated with the input amplitude value
		 * @param desiredAmplitudeUnit The desired amplitude unit of the output
		 *                             value.
		 * @return Success or not
		 */
		bool convertAmplitude(double &amplitude,
		                      const std::string &amplitudeUnit,
		                      const std::string &desiredAmplitudeUnit) const;

	private:
		bool readLocale(Locale *locale,
		                const Settings &settings,
		                const std::string &configPrefix);

	protected:
		struct Correction {
			typedef std::pair<double, double> A;
			typedef std::map<std::string, A> Profiles;

			Profiles::mapped_type &operator[](const Profiles::key_type &key) {
				return profiles[key];
			}

			const A *apply(double &val, const std::string &profile) const;

			Profiles profiles;
		};

		std::string   _type;
		std::string   _networkCode;
		std::string   _stationCode;
		Correction    _corrections;
		Correction::A _defaultCorrection;
};


DEFINE_INTERFACE_FACTORY(MagnitudeProcessor);


}
}


#define REGISTER_MAGNITUDEPROCESSOR_VAR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::MagnitudeProcessor, Class> __##Class##InterfaceFactory__(Service)

#define REGISTER_MAGNITUDEPROCESSOR(Class, Service) \
static REGISTER_MAGNITUDEPROCESSOR_VAR(Class, Service)


#endif
