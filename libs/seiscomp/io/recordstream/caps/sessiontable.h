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


#ifndef SEISCOMP_IO_CAPS_SESSIONTABLE_H
#define SEISCOMP_IO_CAPS_SESSIONTABLE_H


#include "packet.h"

#include <seiscomp/core/optional.h>

#include <functional>
#include <map>
#include <string>


namespace Seiscomp {
namespace IO {
namespace CAPS {


struct SessionTableItem {
	std::string  streamID;
	std::string  net;
	std::string  sta;
	std::string  loc;
	std::string  cha;
	int          samplingFrequency{0};
	int          samplingFrequencyDivider{0};
	double       fSamplingFrequency{0.0};
	PacketType   type;
	DataType     dataType{DT_Unknown};
	int          dataSize{0};
	UOM          uom;
	OPT(Time)    startTime;
	OPT(Time)    endTime;
	void        *userData{nullptr};

	bool splitStreamID();
};


class SessionTable : public std::map<int, SessionTableItem> {
	public:
		enum Status {Success, Error, EOD};

		typedef std::function<void (SessionTableItem*)> CallbackFunc;

	public:
		//! Default constructor
		SessionTable();
		virtual ~SessionTable() {}

	public:
		//! Resets state
		void reset();

		SessionTableItem* getItem(int id) {
			SessionTable::iterator it = find(id);
			if ( it == end() ) return NULL;

			return &it->second;
		}

		Status handleResponse(const char *src_data, int data_len);

		void setItemAddedFunc(const CallbackFunc &func) { _itemAddedFunc = func; }
		void setItemAboutToBeRemovedFunc(const CallbackFunc &func) {
			_itemAboutToBeRemovedFunc = func;
		}

	private:
		enum ResponseState {
			Unspecific,
			Requests
		};

		typedef std::map<std::string, int> StreamIDLookupTable;

	private:
		void registerItem(int id, SessionTableItem &item);
		void removeStream(const std::string & streamID);

	private:
		ResponseState        _state;
		StreamIDLookupTable  _streamIDLookup;
		CallbackFunc         _itemAddedFunc;
		CallbackFunc         _itemAboutToBeRemovedFunc;
};


}
}
}


#endif
