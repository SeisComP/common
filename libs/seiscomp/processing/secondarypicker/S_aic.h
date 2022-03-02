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


#ifndef SEISCOMP_PROCESSING_PICKER_S_AIC_H
#define SEISCOMP_PROCESSING_PICKER_S_AIC_H


#include <seiscomp/processing/secondarypicker.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API SAICPicker : public SecondaryPicker {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		struct AICConfig {
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
		SAICPicker(const std::string& methodID, StreamComponent c);

		//! D'tor
		~SAICPicker();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		void setSaveIntermediate(bool);

		const std::string &methodID() const override;
		const std::string &filterID() const override;

		bool setAicConfig(const AICConfig &config);

		//! Returns the current configuration
		const AICConfig &aicConfig() const { return _aicConfig; }

		const State &state() const { return _state; }
		const Result &result() const { return _result; }

		//! Returns detection data from noiseBegin if setSaveIntermediate
		//! has been enabled before processing started.
		const DoubleArray &processedData() const { return _detectionTrace; }

	protected:
		virtual WaveformOperator* createFilterOperator(Filter* compFilter) = 0;
		bool applyConfig();
		void fill(size_t n, double *samples) override;
		void process(const Record *rec, const DoubleArray &filteredData) override;

	private:
		bool        _initialized;
		AICConfig   _aicConfig;
		State       _state;
		Result      _result;
		Filter     *_compFilter;
		bool        _saveIntermediate;
		DoubleArray _detectionTrace;
		const std::string _methodID;
};


}
}


#endif
