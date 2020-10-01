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


#ifndef SEISCOMP_DATAMODEL_SCDM_TRUNK_EXCHANGE_H
#define SEISCOMP_DATAMODEL_SCDM_TRUNK_EXCHANGE_H


#include <seiscomp/io/importer.h>
#include <seiscomp/io/exporter.h>


namespace Seiscomp {
namespace DataModel {


class ImporterTrunk : public IO::Importer {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ImporterTrunk();


	// ------------------------------------------------------------------
	//  Importer interface
	// ------------------------------------------------------------------
	protected:
		Core::BaseObject *get(std::streambuf* buf);
};


class ExporterTrunk : public IO::Exporter {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterTrunk();


	// ------------------------------------------------------------------
	//  Exporter interface
	// ------------------------------------------------------------------
	protected:
		bool put(std::streambuf* buf, Core::BaseObject *);
		bool put(std::streambuf* buf, const IO::ExportObjectList &objects);
};


}
}


#endif
