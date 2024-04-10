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


#ifndef SEISCOMP_IO_RECORDFILTER_PIPE_H
#define SEISCOMP_IO_RECORDFILTER_PIPE_H


#include <seiscomp/io/recordfilter.h>
#include <vector>


namespace Seiscomp {
namespace IO {


/**
 * \brief Filter chain that routes records through a chain of filters.
 */
class SC_SYSTEM_CORE_API PipeFilter : public RecordFilterInterface {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Connects two filters
		PipeFilter(RecordFilterInterface *filter1 = nullptr,
		           RecordFilterInterface *filter2 = nullptr);


	// ------------------------------------------------------------------
	//  RecordFilter interface
	// ------------------------------------------------------------------
	public:
		Record *feed(const Record *rec) override;
		Record *flush() override;
		void reset() override;
		RecordFilterInterface *clone() const override;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		RecordFilterInterfacePtr _first;
		RecordFilterInterfacePtr _second;
};


}
}


#endif
