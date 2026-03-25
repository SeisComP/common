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


#include <seiscomp/unittest/unittests.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/io/recordfilter/resample.h>

#include <iostream>


using namespace Seiscomp;
#define EPSILON 1E-13


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE(seiscomp_io_recordfilter_resample)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(Resample1) {
	IO::RecordResampler<double> resampler(1.0);
	GenericRecord rec("XX", "ABCD", "", "TH1", Core::Time(2025, 7, 1), 2.0);
	DoubleArrayPtr data = new DoubleArray(1000);
	for ( int i = 0; i < data->size(); ++i ) {
		data->set(i, i);
	}
	rec.setData(data.get());

	auto out = resampler.feed(&rec);
	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(out->startTime().iso(), "2025-07-01T00:00:11.000001Z");
	BOOST_CHECK_EQUAL(out->sampleCount(), 479);
	{
		auto data = static_cast<const DoubleArray*>(out->data());
		BOOST_CHECK_CLOSE(data->mean(), 494.83260259014986, EPSILON);
		BOOST_CHECK_CLOSE(data->median(), 494.8326025901498, EPSILON);
		BOOST_CHECK_CLOSE(data->min(), 21.772634513966601, EPSILON);
		BOOST_CHECK_CLOSE(data->max(), 967.89257066633331, EPSILON);
	}
	delete out;
	out = resampler.feed(nullptr);
	BOOST_REQUIRE(!out);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(Resample2) {
	IO::RecordResampler<double> resampler(1.0);
	GenericRecord rec("XX", "ABCD", "", "TH1", Core::Time(2025, 7, 1), 2.0);
	DoubleArrayPtr data = new DoubleArray(500);
	for ( int i = 0; i < data->size(); ++i ) {
		data->set(i, i);
	}
	rec.setData(data.get());

	auto out = resampler.feed(&rec);
	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(out->startTime().iso(), "2025-07-01T00:00:11.000001Z");
	BOOST_CHECK_EQUAL(out->sampleCount(), 229);
	{
		auto data = static_cast<const DoubleArray*>(out->data());
		BOOST_CHECK_CLOSE(data->mean(), 247.41630129507496, EPSILON);
		BOOST_CHECK_CLOSE(data->median(), 247.41630129507496, EPSILON);
		BOOST_CHECK_CLOSE(data->min(), 21.772634513966601, EPSILON);
		BOOST_CHECK_CLOSE(data->max(), 473.05996807618345, EPSILON);
	}
	delete out;

	for ( int i = 0; i < data->size(); ++i ) {
		data->set(i, 500 + i);
	}
	rec.setStartTime(Core::Time(2025, 7, 1, 0, 4, 10));

	out = resampler.feed(&rec);
	BOOST_REQUIRE(out);
	BOOST_CHECK_EQUAL(out->startTime().iso(), "2025-07-01T00:04:00.0000Z");
	BOOST_CHECK_EQUAL(out->sampleCount(), 250);
	{
		auto data = static_cast<const DoubleArray*>(out->data());
		BOOST_CHECK_CLOSE(data->mean(), 721.46593457643837, EPSILON);
		BOOST_CHECK_CLOSE(data->median(), 722.45559978161873, EPSILON);
		BOOST_CHECK_CLOSE(data->min(), 475.03929848654377, EPSILON);
		BOOST_CHECK_CLOSE(data->max(), 967.89257066633331, EPSILON);
	}
	delete out;
	out = resampler.feed(nullptr);
	BOOST_REQUIRE(!out);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
