/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                                                 #
* #                                                                              #
* #    This program is free software; you can redistribute it and/or modify      #
* #    it under the terms of the GNU General Public License as published by      #
* #    the Free Software Foundation; either version 2 of the License, or         #
* #    (at your option) any later version.                                       #
* #                                                                              #
* #    This program is distributed in the hope that it will be useful,           #
* #    but WITHOUT ANY WARRANTY; without even the implied warranty of            #
* #    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
* #    GNU General Public License for more details.                              #
* #                                                                              #
* #    You should have received a copy of the GNU General Public License         #
* #    along with this program; if not, write to the                             #
* #    Free Software Foundation, Inc.,                                           #
* #    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 #
* ################################################################################ */


#define SEISCOMP_COMPONENT MLcan

#include <seiscomp/logging/log.h>
#include <seiscomp/processing/amplitudes/MLcan.h>

#include <functional>
#include <cstdio>


using namespace std;


namespace Seiscomp {
namespace Processing {


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeValue average(
	const AmplitudeProcessor::AmplitudeValue &v0,
	const AmplitudeProcessor::AmplitudeValue &v1)
{
	AmplitudeProcessor::AmplitudeValue v;
	// Average both values
	v.value = (v0.value + v1.value) * 0.5;

	// Compute lower and upper uncertainty
	double v0l = v0.value;
	double v0u = v0.value;
	double v1l = v1.value;
	double v1u = v1.value;

	if ( v0.lowerUncertainty ) v0l -= *v0.lowerUncertainty;
	if ( v0.upperUncertainty ) v0u += *v0.upperUncertainty;
	if ( v1.lowerUncertainty ) v1l -= *v1.lowerUncertainty;
	if ( v1.upperUncertainty ) v1u += *v1.upperUncertainty;

	double l = 0, u = 0;

	l = max(l, v.value - v0l);
	l = max(l, v.value - v1l);

	u = max(u, v0u - v.value);
	u = max(u, v1u - v.value);

	v.lowerUncertainty = l;
	v.upperUncertainty = u;

	return v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeTime average(
	const AmplitudeProcessor::AmplitudeTime &t0,
	const AmplitudeProcessor::AmplitudeTime &t1)
{
	AmplitudeProcessor::AmplitudeTime t;
	t.reference = Core::Time((double(t0.reference) + double(t1.reference)) * 0.5);

	// Compute lower and upper uncertainty
	Core::Time t0b = t0.reference + Core::TimeSpan(t0.begin);
	Core::Time t0e = t0.reference + Core::TimeSpan(t0.end);
	Core::Time t1b = t1.reference + Core::TimeSpan(t1.begin);
	Core::Time t1e = t1.reference + Core::TimeSpan(t1.end);

	Core::Time minTime = t.reference;
	Core::Time maxTime = t.reference;

	minTime = min(minTime, t0b);
	minTime = min(minTime, t0e);
	minTime = min(minTime, t1b);
	minTime = min(minTime, t1e);

	maxTime = max(maxTime, t0b);
	maxTime = max(maxTime, t0e);
	maxTime = max(maxTime, t1b);
	maxTime = max(maxTime, t1e);

	t.begin = (double)(minTime - t.reference);
	t.end = (double)(maxTime - t.reference);

	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::AmplitudeValue gmean(
	const AmplitudeProcessor::AmplitudeValue &v0,
	const AmplitudeProcessor::AmplitudeValue &v1)
{
	AmplitudeProcessor::AmplitudeValue v;
	// Average both values
	v.value = sqrt(v0.value * v1.value);

	// Compute lower and upper uncertainty
	double v0l = v0.value;
	double v0u = v0.value;
	double v1l = v1.value;
	double v1u = v1.value;

	if ( v0.lowerUncertainty ) v0l -= *v0.lowerUncertainty;
	if ( v0.upperUncertainty ) v0u += *v0.upperUncertainty;
	if ( v1.lowerUncertainty ) v1l -= *v1.lowerUncertainty;
	if ( v1.upperUncertainty ) v1u += *v1.upperUncertainty;

	double l = 0, u = 0;

	l = max(l, v.value - v0l);
	l = max(l, v.value - v1l);

	u = max(u, v0u - v.value);
	u = max(u, v1u - v.value);

	v.lowerUncertainty = l;
	v.upperUncertainty = u;

	return v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_MLcan2h, "MLcan");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_MLcan::AmplitudeProcessor_MLcan()
: AbstractAmplitudeProcessor_ML("MLcan") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_MLcan2h::AmplitudeProcessor_MLcan2h()
: Processing::AmplitudeProcessor("MLcan") {
	setSignalEnd("min(R / 4 , 150)");
	setMinSNR(0);
	// Maximum distance is 8 degrees
	setMaxDist(8);
	// Maximum depth is 80 km
	setMaxDepth(80);

	setUsedComponent(Horizontal);

	_combiner = TakeAverage;

	_ampN.setUsedComponent(FirstHorizontal);
	_ampE.setUsedComponent(SecondHorizontal);

	_ampE.setPublishFunction(bind(&AmplitudeProcessor_MLcan2h::newAmplitude, this, placeholders::_1, placeholders::_2));
	_ampN.setPublishFunction(bind(&AmplitudeProcessor_MLcan2h::newAmplitude, this, placeholders::_1, placeholders::_2));

	// Propagate configuration to single processors
	_ampN.setConfig(config());
	_ampE.setConfig(config());
}



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AmplitudeProcessor *AmplitudeProcessor_MLcan2h::componentProcessor(Component comp) const {
	switch ( comp ) {
		case FirstHorizontalComponent:
			return &_ampN;
		case SecondHorizontalComponent:
			return &_ampE;
		default:
			break;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DoubleArray *AmplitudeProcessor_MLcan2h::processedData(Component comp) const {
	switch ( comp ) {
		case FirstHorizontalComponent:
			return _ampN.processedData(comp);
		case SecondHorizontalComponent:
			return _ampE.processedData(comp);
		default:
			break;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::reprocess(OPT(double) searchBegin, OPT(double) searchEnd) {
	setStatus(WaitingForData, 0);
	_ampN.setConfig(config());
	_ampE.setConfig(config());

	_results[0] = _results[1] = Core::None;

	_ampN.reprocess(searchBegin, searchEnd);
	_ampE.reprocess(searchBegin, searchEnd);

	if ( !isFinished() ) {
		if ( _ampN.status() > Finished )
			setStatus(_ampN.status(), _ampN.statusValue());
		else
			setStatus(_ampE.status(), _ampE.statusValue());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AmplitudeProcessor_MLcan2h::capabilities() const {
	return _ampN.capabilities() | Combiner;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::IDList
AmplitudeProcessor_MLcan2h::capabilityParameters(Capability cap) const {
	if ( cap == Combiner ) {
		IDList params;
		params.push_back("Average");
		params.push_back("Max");
		params.push_back("Min");
		params.push_back("Geometric mean");
		return params;
	}

	return _ampN.capabilityParameters(cap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_MLcan2h::setParameter(Capability cap, const string &value) {
	if ( cap == Combiner ) {
		if ( value == "Min" ) {
			_combiner = TakeMin;
			return true;
		}
		else if ( value == "Max" ) {
			_combiner = TakeMax;
			return true;
		}
		else if ( value == "Average" ) {
			_combiner = TakeAverage;
			return true;
		}
		else if ( value == "Geometric mean" ) {
			_combiner = TakeGeometricMean;
			return true;
		}

		return false;
	}

	_ampN.setParameter(cap, value);
	return _ampE.setParameter(cap, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string AmplitudeProcessor_MLcan2h::parameter(Capability cap) const {
	if ( cap == Combiner ) {
		switch ( _combiner ) {
			case TakeMin:
				return "Min";
			case TakeMax:
				return "Max";
			case TakeAverage:
				return "Average";
			case TakeGeometricMean:
				return "Geometric mean";
			default:
				break;
		}
	}

	return _ampN.parameter(cap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_MLcan2h::setup(const Settings &settings) {
	// Copy the stream configurations (gain, orientation, responses, ...) to
	// the horizontal processors
	_ampN.streamConfig(FirstHorizontalComponent) = streamConfig(FirstHorizontalComponent);
	_ampE.streamConfig(SecondHorizontalComponent) = streamConfig(SecondHorizontalComponent);

	_combiner = TakeAverage;

	try {
		string s = settings.getString("amplitudes." + _type + ".combiner");
		if ( s == "average" )
			_combiner = TakeAverage;
		else if ( s == "max" )
			_combiner = TakeMax;
		else if ( s == "min" )
			_combiner = TakeMin;
		else if ( s == "geometric_mean" )
			_combiner = TakeGeometricMean;
		else {
			SEISCOMP_ERROR("%s: invalid combiner type for station %s.%s: %s",
			               _type.c_str(),
			               settings.networkCode.c_str(), settings.stationCode.c_str(),
			               s.c_str());
			return false;
		}
	}
	catch ( ... ) {}

	if ( !AmplitudeProcessor::setup(settings) ) return false;

	// Setup each component
	if ( !_ampN.setup(settings) || !_ampE.setup(settings) ) return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::setTrigger(const Core::Time& trigger) {
	AmplitudeProcessor::setTrigger(trigger);
	_ampE.setTrigger(trigger);
	_ampN.setTrigger(trigger);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::computeTimeWindow() {
	// Copy configuration to each component
	_ampN.setConfig(config());
	_ampE.setConfig(config());

	_ampN.computeTimeWindow();
	_ampE.computeTimeWindow();

	// computeTimeWindow evaluates the signal times. This copies back the
	// evaluated times.
	setConfig(_ampE.config());

	setTimeWindow(_ampE.timeWindow() | _ampN.timeWindow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::setEnvironment(const DataModel::Origin *hypocenter,
                                             const DataModel::SensorLocation *receiver,
                                             const DataModel::Pick *pick) {
	_ampN.setEnvironment(hypocenter, receiver, pick);
	_ampE.setEnvironment(hypocenter, receiver, pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::reset() {
	AmplitudeProcessor::reset();

	_results[0] = _results[1] = Core::None;

	_ampE.reset();
	_ampN.reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::close() const {
	// TODO: Check for best available amplitude here
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_MLcan2h::feed(const Record *record) {
	// Both processors finished already?
	if ( _ampE.isFinished() && _ampN.isFinished() ) return false;

	// Did an error occur?
	if ( status() > WaveformProcessor::Finished ) return false;

	if ( record->channelCode() == _streamConfig[FirstHorizontalComponent].code() ) {
		if ( !_ampN.isFinished() ) {
			_ampN.feed(record);
			if ( _ampN.status() == InProgress )
				setStatus(WaveformProcessor::InProgress, _ampN.statusValue());
			else if ( _ampN.isFinished() && _ampE.isFinished() ) {
				if ( !isFinished() ) {
					if ( _ampE.status() == Finished )
						setStatus(_ampN.status(), _ampN.statusValue());
					else
						setStatus(_ampE.status(), _ampE.statusValue());
				}
			}
		}
	}
	else if ( record->channelCode() == _streamConfig[SecondHorizontalComponent].code() ) {
		if ( !_ampE.isFinished() ) {
			_ampE.feed(record);
			if ( _ampE.status() == InProgress )
				setStatus(WaveformProcessor::InProgress, _ampE.statusValue());
			else if ( _ampE.isFinished() && _ampN.isFinished() ) {
				if ( !isFinished() ) {
					if ( _ampN.status() == Finished )
						setStatus(_ampE.status(), _ampE.statusValue());
					else
						setStatus(_ampN.status(), _ampN.statusValue());
				}
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_MLcan2h::computeAmplitude(const DoubleArray &data,
                                               size_t i1, size_t i2,
                                               size_t si1, size_t si2,
                                               double offset,
                                               AmplitudeIndex *dt,
                                               AmplitudeValue *amplitude,
                                               double *period, double *snr) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_MLcan2h::newAmplitude(const AmplitudeProcessor *proc,
                                           const AmplitudeProcessor::Result &res) {
	if ( isFinished() ) return;

	int idx = 0;

	if ( proc == &_ampE ) {
		idx = 0;
	}
	else if ( proc == &_ampN ) {
		idx = 1;
	}

	_results[idx] = ComponentResult();
	_results[idx]->value = res.amplitude;
	_results[idx]->time = res.time;
	_results[idx]->snr = res.snr;
	_results[idx]->period = res.period;

	if ( _results[0] && _results[1] ) {
		setStatus(Finished, 100.);
		Result newRes;
		newRes.record = res.record;

		switch ( _combiner ) {
			case TakeAverage:
				newRes.amplitude = average(_results[0]->value, _results[1]->value);
				newRes.time = average(_results[0]->time, _results[1]->time);
				newRes.snr = (_results[0]->snr + _results[1]->snr) * 0.5;
				newRes.period = (_results[0]->period + _results[1]->period) * 0.5;
				newRes.component = Horizontal;
				break;
			case TakeGeometricMean:
				newRes.amplitude = gmean(_results[0]->value, _results[1]->value);
				newRes.time = average(_results[0]->time, _results[1]->time);
				newRes.snr = (_results[0]->snr + _results[1]->snr) * 0.5;
				newRes.period = (_results[0]->period + _results[1]->period) * 0.5;
				newRes.component = Horizontal;
				break;
			case TakeMin:
				if ( _results[0]->value.value <= _results[1]->value.value ) {
					newRes.amplitude = _results[0]->value;
					newRes.time = _results[0]->time;
					newRes.snr = _results[0]->snr;
					newRes.period = _results[0]->period;
					newRes.component = _ampE.usedComponent();
				}
				else {
					newRes.amplitude = _results[1]->value;
					newRes.time = _results[1]->time;
					newRes.snr = _results[1]->snr;
					newRes.period = _results[1]->period;
					newRes.component = _ampN.usedComponent();
				}
				break;
			case TakeMax:
				if ( _results[0]->value.value >= _results[1]->value.value ) {
					newRes.amplitude =  _results[0]->value;
					newRes.time = _results[0]->time;
					newRes.snr = _results[0]->snr;
					newRes.period = _results[0]->period;
					newRes.component = _ampE.usedComponent();
				}
				else {
					newRes.amplitude =  _results[1]->value;
					newRes.time = _results[1]->time;
					newRes.snr = _results[1]->snr;
					newRes.period = _results[1]->period;
					newRes.component = _ampN.usedComponent();
				}
				break;
		};

		emitAmplitude(newRes);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
