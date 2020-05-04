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


#ifndef SEISCOMP_PROCESSING_ARAICPICKER_H
#define SEISCOMP_PROCESSING_ARAICPICKER_H


#include <seiscomp/processing/picker.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API ARAICPicker : public Picker {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		ARAICPicker();
		ARAICPicker(const Core::Time& trigger);

		//! D'tor
		~ARAICPicker();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setup(const Settings &settings) override;

		const std::string &methodID() const override;
		const std::string &filterID() const override;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		bool calculatePick(int n, const double *data,
		                   int signalStartIdx, int signalEndIdx,
		                   int &triggerIdx, int &lowerUncertainty,
		                   int &upperUncertainty, double &snr,
		                   OPT(Polarity) &polarity) override;

	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		std::string _filter;
		bool _dumpTraces;
};


}
}

#endif
