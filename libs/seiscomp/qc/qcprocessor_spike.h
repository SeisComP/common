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


#ifndef SEISCOMP_PROCESSING_QCPROCESSORSPIKE_H
#define SEISCOMP_PROCESSING_QCPROCESSORSPIKE_H


#include <seiscomp/qc/qcprocessor.h>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(QcProcessorSpike);

class SC_SYSTEM_CLIENT_API QcProcessorSpike : public QcProcessor {
	DECLARE_SC_CLASS(QcProcessorSpike);

	public:
		QcProcessorSpike();
		~QcProcessorSpike();

		typedef std::map<Core::Time, double> Spikes;
		Spikes getSpikes();

		bool feed(const Record *record);
		void _setFilter(double fsamp);

		bool setState(const Record* record, const DoubleArray& data);

	private:
		bool _initFilter;
};


}
}

#endif
