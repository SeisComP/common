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


#ifndef SEISCOMP_DATAMODEL_SCDM_IMS10_EXCHANGE_H
#define SEISCOMP_DATAMODEL_SCDM_IMS10_EXCHANGE_H


#include <seiscomp/io/exporter.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/stationmagnitudecontribution.h>


namespace Seiscomp {
namespace DataModel {


class ExporterIMS10 : public IO::Exporter {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterIMS10();

	// ------------------------------------------------------------------
	//  Exporter interface
	// ------------------------------------------------------------------
	protected:
		bool put(std::streambuf* buf, Core::BaseObject *);
	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		std::vector<Seiscomp::DataModel::ArrivalPtr> _arrivalList;
		std::vector<Seiscomp::DataModel::PickPtr> _pickList;
		std::vector<Seiscomp::DataModel::AmplitudePtr> _amplitudeList;
		std::vector<Seiscomp::DataModel::StationMagnitudePtr> _stationMagnitudeList;
};


}
}


#endif
