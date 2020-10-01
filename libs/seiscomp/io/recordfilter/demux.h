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


#ifndef SEISCOMP_IO_RECORDFILTER_DEMUX_H
#define SEISCOMP_IO_RECORDFILTER_DEMUX_H

#include <seiscomp/io/recordfilter.h>
#include <map>


namespace Seiscomp {
namespace IO {


/**
 * \brief Record demuxer that demultiplexes different channels and applies
 * \brief a given record filter to each of them.
 */
class SC_SYSTEM_CORE_API RecordDemuxFilter : public RecordFilterInterface {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructs a record demuxer with an optional record filter
		//! applied to each channel.
		//! Note: the ownership goes to the record demuxer
		RecordDemuxFilter(RecordFilterInterface *recordFilter = nullptr);
		virtual ~RecordDemuxFilter();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Note: the ownership goes to the record demuxer
		void setFilter(RecordFilterInterface *recordFilter);


	// ------------------------------------------------------------------
	//  RecordFilter interface
	// ------------------------------------------------------------------
	public:
		virtual Record *feed(const Record *rec);
		virtual Record *flush();
		virtual void reset();
		virtual RecordFilterInterface *clone() const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		typedef std::map<std::string, RecordFilterInterfacePtr> FilterMap;
		RecordFilterInterfacePtr _template;
		FilterMap                _streams;
};


}
}

#endif
