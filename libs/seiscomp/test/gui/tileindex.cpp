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

#include <seiscomp/gui/map/imagetree.h>
#include <random>

namespace bu = boost::unit_test;
using namespace std;


BOOST_AUTO_TEST_SUITE(seiscomp_gui_tileindex)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(index) {
	Seiscomp::Gui::Map::TileIndex index;
	index = Seiscomp::Gui::Map::TileIndex(1, 2, 3);
	BOOST_CHECK_EQUAL(index.level(), 1);
	BOOST_CHECK_EQUAL(index.row(), 2);
	BOOST_CHECK_EQUAL(index.column(), 3);

	int maxLevel = 1 << Seiscomp::Gui::Map::TileIndex::LevelBits;
	int maxRow = 1 << Seiscomp::Gui::Map::TileIndex::RowBits;
	int maxColumn = 1 << Seiscomp::Gui::Map::TileIndex::ColumnBits;

	for ( int l = 0; l < maxLevel; ++l ) {
		for ( int r = 0; r < 100000; ++r ) {
			int row = int64_t(rand()) * maxRow / RAND_MAX;
			int column = int64_t(rand()) * maxColumn / RAND_MAX;
			index = Seiscomp::Gui::Map::TileIndex(l, row, column);
			BOOST_CHECK_EQUAL(index.level(), l);
			BOOST_CHECK_EQUAL(index.row(), row);
			BOOST_CHECK_EQUAL(index.column(), column);
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
