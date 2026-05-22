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


#ifndef SEISCOMP_GUI_MAGNITUDEVIEW_PRIVATE_H
#define SEISCOMP_GUI_MAGNITUDEVIEW_PRIVATE_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the SeisComP API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <seiscomp/gui/datamodel/calculateamplitudes.h>
#include <seiscomp/gui/datamodel/ui_magnitudeview.h>
#include <seiscomp/gui/core/diagramwidget.h>
#include <seiscomp/gui/datamodel/magnitudemap.h>

#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/amplitude.h>

#include <seiscomp/datamodel/publicobjectcache.h>
#include <seiscomp/processing/magnitudeprocessor.h>


namespace Seiscomp::Gui {


class MagnitudeViewPrivate {
	private:
		MagnitudeViewPrivate(Seiscomp::DataModel::DatabaseQuery *q,
		                     Map::ImageTree *mapTree = nullptr)
		: reader(q), objCache(q, 500), maptree(mapTree) {}

	private:
		using AmplitudeEntry = CalculateAmplitudes::AmplitudeEntry;
		using PickAmplitudeMap = CalculateAmplitudes::PickAmplitudeMap;
		using AvailableTypes = Processing::MagnitudeProcessorFactory::ServiceNames;

		Seiscomp::DataModel::DatabaseQuery *reader;
		::Ui::MagnitudeView                 ui;

		Map::ImageTreePtr                   maptree;
		MagnitudeMap                       *map;

		DiagramWidget                      *stamagnitudes;
		QAbstractTableModel                *modelStationMagnitudes;
		QSortFilterProxyModel              *modelStationMagnitudesProxy;

		AmplitudeView::Config               amplitudeConfig;
		AmplitudeView                      *amplitudeView;

		QTabBar                            *tabMagnitudes;

		DataModel::OriginPtr                origin;
		DataModel::EventPtr                 event;
		DataModel::MagnitudePtr             netMag;

		double                              minStationMagnitude;
		double                              maxStationMagnitude;

		DataModel::PublicObjectRingBuffer   objCache;

		bool                                computeMagnitudesSilently;
		bool                                enableMagnitudeTypeSelection;
		OPT(std::string)                    defaultMagnitudeAggregation;

		PickAmplitudeMap                    amplitudes;
		std::string                         preferredMagnitudeID;
		std::vector<std::string>            magnitudeTypes;
		std::vector<std::string>            currentMagnitudeTypes;
		AvailableTypes                     *availableMagTypes;

	friend class MagnitudeView;
};


}


#endif
