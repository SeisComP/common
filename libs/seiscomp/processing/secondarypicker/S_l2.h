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


#ifndef SEISCOMP_PROCESSING_PICKER_S_L2_H
#define SEISCOMP_PROCESSING_PICKER_S_L2_H


#include <seiscomp/processing/secondarypicker.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API SL2Picker : public SecondaryPicker {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		struct L2Config {
			double      threshold;
			double      minSNR;
			double      margin;
			double      timeCorr;
			std::string filter;
			std::string detecFilter;
		};

		struct State {
			State();
			bool        aicValid;
			double      aicStart;
			double      aicEnd;
			Core::Time  detection;
			Core::Time  pick;
			double      snr;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SL2Picker();

		//! D'tor
		~SL2Picker();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setup(const Settings &settings) override;
		void setSaveIntermediate(bool);

		const std::string &methodID() const override;
		const std::string &filterID() const override;

		bool setL2Config(const L2Config &l2config);

		//! Returns the current configuration
		const L2Config &l2Config() const { return _l2Config; }

		const State &state() const { return _state; }
		const Result &result() const { return _result; }

		//! Returns detection data from noiseBegin if setSaveIntermediate
		//! has been enabled before processing started.
		const DoubleArray &processedData() const { return _detectionTrace; }

	protected:
		bool applyConfig();
		void fill(size_t n, double *samples) override;
		void process(const Record *rec, const DoubleArray &filteredData) override;

	private:
		bool        _initialized;
		L2Config    _l2Config;
		State       _state;
		Result      _result;
		Filter     *_compFilter;
		bool        _saveIntermediate;
		DoubleArray _detectionTrace;
};


}
}


#endif
