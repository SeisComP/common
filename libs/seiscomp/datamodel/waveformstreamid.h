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


#ifndef SEISCOMP_DATAMODEL_WAVEFORMSTREAMID_H
#define SEISCOMP_DATAMODEL_WAVEFORMSTREAMID_H


#include <string>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(WaveformStreamID);


/**
 * \brief Reference to a stream description in an inventory. This is
 * \brief mostly
 * \brief equivalent to the combination of networkCode, stationCode,
 * \brief locationCode
 * \brief and channelCode. However, additional information, e. g.,
 * \brief sampling rate,
 * \brief can be referenced by the resourceURI. It is recommended to
 * \brief use
 * \brief resourceURI as a flexible, abstract, and unique stream ID
 * \brief that allows to
 * \brief describe different processing levels, or resampled/filtered
 * \brief products of
 * \brief the same initial stream, without violating the intrinsic
 * \brief meaning of the
 * \brief legacy identifiers (network, station, channel, and location
 * \brief codes).
 * \brief However, for operation in the context of legacy systems,
 * \brief the classical
 * \brief identifier components are supported.
 */
class SC_SYSTEM_CORE_API WaveformStreamID : public Core::BaseObject {
	DECLARE_SC_CLASS(WaveformStreamID)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		WaveformStreamID();

		//! Copy constructor
		WaveformStreamID(const WaveformStreamID &other);

		//! Custom constructor
		WaveformStreamID(const std::string& resourceURI);
		WaveformStreamID(const std::string& networkCode,
		                 const std::string& stationCode,
		                 const std::string& locationCode,
		                 const std::string& channelCode,
		                 const std::string& resourceURI);

		//! Destructor
		~WaveformStreamID() override;


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator std::string&();
		operator const std::string&() const;

		//! Copies the metadata of other to this
		WaveformStreamID &operator=(const WaveformStreamID &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const WaveformStreamID &other) const;
		bool operator!=(const WaveformStreamID &other) const;

		//! Wrapper that calls operator==
		bool equal(const WaveformStreamID &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Network code. String with a maximum length of 8 characters.
		void setNetworkCode(const std::string& networkCode);
		const std::string& networkCode() const;

		//! Station code. String with a maximum length of 8 characters.
		void setStationCode(const std::string& stationCode);
		const std::string& stationCode() const;

		//! Location code. String with a maximum length of 8 characters.
		void setLocationCode(const std::string& locationCode);
		const std::string& locationCode() const;

		//! Channel code. String with a maximum length of 8 characters.
		void setChannelCode(const std::string& channelCode);
		const std::string& channelCode() const;

		//! Optional resource identifier for the waveform stream.
		//! QuakeML adopts
		//! in many places resource descriptors with a well-defined
		//! syntax for
		//! unambiguous resource identification. Resource identifiers
		//! are designed
		//! to be backward compatible with existing descriptors. In
		//! SeisComP this
		//! identifier is not used at all.
		void setResourceURI(const std::string& resourceURI);
		const std::string& resourceURI() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _networkCode;
		std::string _stationCode;
		std::string _locationCode;
		std::string _channelCode;
		std::string _resourceURI;
};


}
}


#endif
