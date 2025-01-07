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

#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/processing/operator/l2norm.h>
#include <seiscomp/processing/operator/ncomps.h>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Processing;


BOOST_AUTO_TEST_SUITE(seiscomp_processing_ncomps)


BOOST_AUTO_TEST_CASE(data) {
	using CodeL2Norm = Operator::CodeWrapper<double, 2, Operator::L2Norm>;
	using L2Norm = NCompsOperator<double, 2, CodeL2Norm>;

	auto op = new L2Norm(
		CodeL2Norm("BHN", "BHE", Operator::L2Norm<double,2>())
	);

	GenericRecordPtr nRec, eRec;
	Time startTime(2024, 12, 20, 0, 0, 0);

	DoubleArrayPtr nData = new DoubleArray(40);
	nData->fill(3);
	DoubleArrayPtr eData = new DoubleArray(40);
	eData->fill(4);

	const double fsamp = 20.0;
	nRec = new GenericRecord("XX", "ABCD", "", "BHN", startTime, fsamp);
	nRec->setData(nData.get());
	eRec = new GenericRecord("XX", "ABCD", "", "BHE", startTime, fsamp);
	eRec->setData(eData.get());

	RingBuffer seq(0);

	op->setStoreFunc([&seq](const Record *rec) -> bool {
		seq.feed(rec);
		return true;
	});

	op->feed(nRec.get());
	op->feed(eRec.get());

	BOOST_CHECK_EQUAL(seq.size(), 1);
	BOOST_CHECK_EQUAL(seq.timeWindow(), TimeWindow(startTime, startTime + TimeSpan(0, std::min(nData->size(), eData->size()) * 1000000 / fsamp)));

	const DoubleArray *l2data = DoubleArray::ConstCast(seq.front()->data());
	for ( int i = 0; i < l2data->size(); ++i ) {
		BOOST_CHECK_EQUAL((*l2data)[i], 5);
	}

	auto gap = Core::TimeSpan(10, 0);

	nRec = new GenericRecord("XX", "ABCD", "", "BHN", startTime + gap, fsamp);
	nRec->setData(nData.get());
	eRec = new GenericRecord("XX", "ABCD", "", "BHE", startTime + gap, fsamp);
	eRec->setData(eData.get());

	op->feed(nRec.get());
	op->feed(eRec.get());

	BOOST_CHECK_EQUAL(seq.size(), 2);
	BOOST_CHECK_EQUAL(seq.timeWindow(), TimeWindow(startTime, gap + TimeSpan(0, std::min(nData->size(), eData->size()) * 1000000 / fsamp)));

	l2data = DoubleArray::ConstCast((*(++seq.begin()))->data());
	for ( int i = 0; i < l2data->size(); ++i ) {
		BOOST_CHECK_EQUAL((*l2data)[i], 5);
	}
}


BOOST_AUTO_TEST_SUITE_END()
