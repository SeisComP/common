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



#ifndef SEISCOMP_PROCESSING_OPERATOR_NCOMPS_H
#define SEISCOMP_PROCESSING_OPERATOR_NCOMPS_H


#include <seiscomp/processing/waveformoperator.h>
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/recordsequence.h>


namespace Seiscomp {
namespace Processing {


/*
template <typename T, int N>
class Proc {
	// Process N traces in place of length n
	void operator()(const Record *, T *data[N], int n, const Core::Time &stime, double sfreq) const;

	// Publish a processed component
	bool publish(int c) const;

	// Returns the component index of a given channel code
	int compIndex(const std::string &code) const;
};
*/


template <typename T, int N, class PROC, int BSIZE=-1>
class NCompsOperator : public WaveformOperator {
	public:
		NCompsOperator(const PROC &proc) : _proc(proc), _processing(false) {}

		WaveformProcessor::Status feed(const Record *record);
		void reset();

	protected:
		WaveformProcessor::Status process(int comp, const Record *rec);

	protected:
		struct State {
			State() : buffer(BSIZE) {}
			RingBuffer  buffer;
			Core::Time  endTime;
		};

		// Stores the N channel codes and the according record buffer
		State _states[N];
		PROC  _proc;
		bool  _processing;
};


namespace Operator {


template <typename T, int N, template <typename,int> class PROC>
class CodeWrapper {};


template <typename T, template <typename,int> class PROC>
class CodeWrapper<T,2,PROC> {
	public:
		CodeWrapper(const std::string &code1, const std::string &code2,
		            const PROC<T,2> &proc) : _proc(proc) {}

		void operator()(const Record *rec, T *data[2], int n, const Core::Time &stime, double sfreq) const { _proc(rec, data, n, stime, sfreq); }
		bool publish(int c) const { return _proc.publish(c); }

		int compIndex(const std::string &code) const { return -1; }

		const std::string &translateChannelCode(int c, const std::string &code) { return code; }

	private:
		PROC<T,2>   _proc;
		std::string _code1;
		std::string _code2;
};


template <typename T, template <typename,int> class PROC>
class CodeWrapper<T,3,PROC> {
	public:
		CodeWrapper(const std::string &code1, const std::string &code2,
		            const std::string &code3, const PROC<T,3> &proc)
		: _proc(proc), _code1(code1), _code2(code2), _code3(code3) {}

		void operator()(const Record *rec, T *data[3], int n, const Core::Time &stime, double sfreq) const { _proc(rec, data, n, stime, sfreq); }
		bool publish(int c) const { return _proc.publish(c); }

		int compIndex(const std::string &code) const {
			if ( code == _code1 ) return 0;
			else if ( code == _code2 ) return 1;
			else if ( code == _code3 ) return 2;
			return -1;
		}

		const std::string &translateChannelCode(int c, const std::string &code) { return code; }


	private:
		PROC<T,3>   _proc;
		std::string _code1;
		std::string _code2;
		std::string _code3;
};


template <typename T, int N, template <typename,int> class PROC>
class StreamConfigWrapper {
	public:
		StreamConfigWrapper(Stream configs[N], const PROC<T,N> &proc)
		: _proc(proc), _configs(configs) {}

		void operator()(const Record *rec, T *data[N], int n, const Core::Time &stime, double sfreq) const {
			// Sensitivity correction before applying the operator
			for ( int c = 0; c < N; ++c ) {
				if ( _configs[c].gain == 0.0 ) continue;
				double scale = 1.0 / _configs[c].gain;
				T *trace = data[c];

				for ( int i = 0; i < n; ++i, ++trace )
					*trace *= scale;
			}

			// Call real operator
			_proc(rec, data, n, stime, sfreq);
		}

		bool publish(int c) const { return _proc.publish(c); }

		int compIndex(const std::string &code) const {
			for ( int i = 0; i < N; ++i )
				if ( code == _configs[i].code() ) return i;
			return -1;
		}

		const std::string &translateChannelCode(int c, const std::string &code) { return code; }


	private:
		PROC<T,N>     _proc;
		const Stream *_configs;
};


template <typename T, int N>
class NoOpWrapper {
	public:
		NoOpWrapper(Stream configs[N]) : _configs(configs) { }

		// Process N traces in place of length n
		void operator()(const Record *, T *data[N], int n, const Core::Time &stime, double sfreq) const {}

		// publishs a processed component
		bool publish(int c) const { return c < N; }

		// Returns the component index of a given channel code
		int compIndex(const std::string &code) const {
			for ( int i = 0; i < N; ++i )
				if ( code == _configs[i].code() ) return i;
			return -1;
		}

	private:
			const Stream *_configs;
};


template <typename T, int N, class PROC>
class FilterWrapper {
	public:
		FilterWrapper(Math::Filtering::InPlaceFilter<T> *filter,
									const PROC &proc)
		: _proc(proc), _baseFilter(filter) {
			for ( int i = 0; i < N; ++i ) _filter[i] = nullptr;
		}

		~FilterWrapper() {
			for ( int i = 0; i < N; ++i )
				if ( _filter[i] ) delete _filter[i];
		}

		void operator()(const Record *rec, T *data[N], int n, const Core::Time &stime, double sfreq) const {
			if ( _baseFilter ) {
				for ( int i = 0; i < N; ++i ) {
					if ( _filter[i] == nullptr ) {
						_filter[i] = _baseFilter->clone();
						_filter[i]->setSamplingFrequency(sfreq);
					}

					_filter[i]->apply(n, data[i]);
				}
			}

			// Call real operator
			_proc(rec, data, n, stime, sfreq);
		}

		bool publish(int c) const { return _proc.publish(c); }

		int compIndex(const std::string &code) const {
			return _proc.compIndex(code);
		}

		const std::string &translateChannelCode(int, const std::string &code) {
			return code;
		}


	private:
		PROC _proc;
		Math::Filtering::InPlaceFilter<T> *_baseFilter;
		mutable Math::Filtering::InPlaceFilter<T> *_filter[N];
};


}

}
}


#include <seiscomp/processing/operator/ncomps.ipp>


#endif
