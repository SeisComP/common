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


#ifndef SEISCOMP_PROCESSING_WAVEFORMPIPE_H
#define SEISCOMP_PROCESSING_WAVEFORMPIPE_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/record.h>
#include <seiscomp/processing/waveformprocessor.h>

#include <boost/function.hpp>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(WaveformOperator);

//! WaveformPipe declares an interface to modify/manipulate/combine
//! records. It can be used to rotate 3 components or to combine
//! two horizontal components into a single component.
class SC_SYSTEM_CLIENT_API WaveformOperator : public Core::BaseObject {
	public:
		typedef boost::function<bool (const Record *)> StoreFunc;

		WaveformOperator();
		virtual ~WaveformOperator();


	public:
		//! Sets the storage function called when a new records is
		//! available.
		void setStoreFunc(const StoreFunc &func);


		//! Connects the output of op1 with input of op2. Calls setStoreFunc
		//! on op1.
		static void connect(WaveformOperator *op1, WaveformOperator *op2);


		//! Feeds a record. In case a status value larger that Terminated
		//! (which indicates an error) is returned, it is populated
		//! into the WaveformProcessor's status.
		virtual WaveformProcessor::Status feed(const Record *record) = 0;

		//! Resets the operator.
		virtual void reset() = 0;


	protected:
		bool store(const Record *rec) {
			if ( _func ) return _func(rec);
			return false;
		}


	private:
		StoreFunc     _func;
};


}
}


#endif
