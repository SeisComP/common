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


#define SEISCOMP_TEST_MODULE SeisComP


#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/processing/amplitudes/MLv.h>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Processing;


BOOST_AUTO_TEST_SUITE(seiscomp_processing_amplitudes)


BOOST_AUTO_TEST_CASE(signalTime) {
	AmplitudeProcessor_MLv proc;
	proc.setMargin(TimeSpan(0, 0));
	proc.setTrigger(Time(2023, 9, 14, 0, 0, 0));
	proc.setNoiseStart(0);
	proc.setNoiseEnd(0);
	proc.setSignalStart("0");
	proc.setSignalEnd(150);
	proc.computeTimeWindow();
	BOOST_CHECK_EQUAL(proc.status(), WaveformProcessor::WaitingForData);
	BOOST_CHECK_EQUAL(proc.timeWindow().startTime().iso(), Time(2023, 9, 14, 0, 0, 0).iso());
	BOOST_CHECK_EQUAL(proc.timeWindow().endTime().iso(), Time(2023, 9, 14, 0, 2, 30).iso());

	proc.reset();
	proc.setTrigger(Time(2023, 9, 14, 0, 0, 0));
	proc.setNoiseStart(0);
	proc.setNoiseEnd(0);
	proc.setSignalStart("0");
	proc.setSignalEnd("R / 3");
	proc.computeTimeWindow();
	BOOST_CHECK_EQUAL(proc.status(), WaveformProcessor::MissingHypocenter);

	proc.reset();
	proc.setTrigger(Time(2023, 9, 14, 0, 0, 0));
	proc.setNoiseStart(0);
	proc.setNoiseEnd(0);
	proc.setSignalStart("0");
	proc.setSignalEnd("min(R / 3, 150)");
	proc.computeTimeWindow();
	BOOST_CHECK_EQUAL(proc.status(), WaveformProcessor::WaitingForData);
	BOOST_CHECK_EQUAL(proc.timeWindow().startTime().iso(), Time(2023, 9, 14, 0, 0, 0).iso());
	BOOST_CHECK_EQUAL(proc.timeWindow().endTime().iso(), Time(2023, 9, 14, 0, 2, 30).iso());

	OriginPtr org = Origin::Create();
	org->setTime(Time(2023, 9, 13, 23, 50, 0));
	org->setLatitude(0);
	org->setLongitude(0);

	PickPtr pick1 = Pick::Create();
	pick1->setTime(Time(2023, 9, 14, 0, 0, 1));

	ArrivalPtr arr1 = new Arrival();
	arr1->setPickID(pick1->publicID());
	arr1->setPhase(Phase("P1"));

	org->add(arr1.get());

	SensorLocationPtr sloc = SensorLocation::Create();
	sloc->setLatitude(0.0);
	sloc->setLongitude(1.0);

	proc.reset();
	proc.setTrigger(Time(2023, 9, 14, 0, 0, 0));
	proc.setNoiseStart(0);
	proc.setNoiseEnd(0);
	proc.setSignalStart("0");
	proc.setSignalEnd("min(R / 3, 150)");
	proc.setEnvironment(org.get(), sloc.get(), nullptr);
	proc.computeTimeWindow();
	BOOST_CHECK_EQUAL(proc.status(), WaveformProcessor::WaitingForData);
	BOOST_CHECK_EQUAL(proc.timeWindow().startTime().iso(), Time(2023, 9, 14, 0, 0, 0).iso());
	BOOST_CHECK_EQUAL(proc.timeWindow().endTime().iso(), Time(2023, 9, 14, 0, 0, 37, 65027).iso());

	proc.reset();
	proc.setTrigger(Time(2023, 9, 14, 0, 0, 0));
	proc.setNoiseStart(0);
	proc.setNoiseEnd(0);
	proc.setSignalStart("0");
	proc.setSignalEnd("arr(P1)");
	proc.setEnvironment(org.get(), sloc.get(), nullptr);
	proc.computeTimeWindow();
	BOOST_CHECK_EQUAL(proc.status(), WaveformProcessor::ArrivalNotFound);

	proc.reset();
	proc.setTrigger(Time(2023, 9, 14, 0, 0, 0));
	proc.setNoiseStart(0);
	proc.setNoiseEnd(0);
	proc.setSignalStart("0");
	proc.setSignalEnd("arr(P1, true)");
	proc.setEnvironment(org.get(), sloc.get(), nullptr);
	proc.computeTimeWindow();
	BOOST_CHECK_EQUAL(proc.status(), WaveformProcessor::WaitingForData);
	BOOST_CHECK_EQUAL(proc.timeWindow().startTime().iso(), Time(2023, 9, 14, 0, 0, 0).iso());
	BOOST_CHECK_EQUAL(proc.timeWindow().endTime().iso(), Time(2023, 9, 14, 0, 0, 1, 0).iso());
}


BOOST_AUTO_TEST_SUITE_END()
