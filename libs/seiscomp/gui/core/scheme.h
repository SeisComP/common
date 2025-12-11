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



#ifndef SEISCOMP_GUI_SCHEME_H
#define SEISCOMP_GUI_SCHEME_H


#include <QColor>
#include <QPen>
#include <QBrush>
#include <QPoint>
#include <QFont>
#include <seiscomp/gui/qt.h>
#include <seiscomp/gui/core/gradient.h>
#include <seiscomp/gui/core/recordwidget.h>


class QTabWidget;


namespace Seiscomp {
namespace Gui {


class SC_GUI_API Scheme {
	public:
		Scheme();

		void applyTabPosition(QTabWidget *tab);

		struct Colors {
			Colors();

			struct Splash {
				QColor version{0, 104, 158};
				QColor message{123, 123, 123};
			};

			struct Picks {
				QColor manual{Qt::green};
				QColor automatic{Qt::red};
				QColor undefined{Qt::gray};
				QColor disabled{Qt::gray};
			};

			struct Arrivals {
				Arrivals();
				QColor   manual{0, 160, 0};
				QColor   automatic{160, 0, 0};
				QColor   theoretical{0, 0, 160};
				QColor   undefined{160, 0, 0};
				QColor   disabled{Qt::gray};
				QPen     uncertainties{{Qt::gray}, 0.8};
				QPen     defaultUncertainties{{{128,128,128,64}}, 0.8};
				Gradient residuals;
			};

			struct Magnitudes {
				Magnitudes();
				QColor set{0, 160, 0};
				QColor unset{Qt::transparent};
				QColor disabled{Qt::gray};
				Gradient residuals;
			};

			struct RecordStates {
				QColor unrequested{0,0,0,128};
				QColor requested{255,255,0,128};
				QColor inProgress{0,255,0,16};
				QColor notAvailable{255,0,0,128};
			};

			struct BrushPen {
				QPen pen;
				QBrush brush;
			};

			struct RecordBorders {
				RecordBorders();
				BrushPen standard;
				BrushPen signatureValid;
				BrushPen signatureInvalid;
			};

			struct Records {
				QColor alignment{Qt::red};
				QColor background;
				QColor alternateBackground;
				QColor foreground{128, 128, 128};
				QColor alternateForeground{128, 128, 128};
				QColor spectrogram{Qt::black};
				QPen offset{{192,192,255}};
				QPen gridPen{{{0,0,0,32}}, 1, Qt::DashLine};
				QPen subGridPen{{{0,0,0,0}}, 1, Qt::DotLine};
				QBrush gaps{{255, 255, 0, 64}};
				QBrush overlaps{{255, 0, 255, 64}};
				RecordStates states;
				RecordBorders borders;
			};

			struct Stations {
				QColor text{Qt::white};
				QColor associated{130, 173, 88};
				QColor selected{77, 77, 184};
				QColor triggering{Qt::red};
				QColor triggered0{0, 128, 255};
				QColor triggered1{0, 0, 255};
				QColor triggered2{0, 0, 128};
				QColor disabled{102, 102, 102, 100};
				QColor idle{102, 102, 102, 128};
			};

			struct QC {
				QColor delay0{0, 255, 255};
				QColor delay1{0, 255, 0};
				QColor delay2{255, 253, 0};
				QColor delay3{255, 102, 51};
				QColor delay4{255, 0, 0};
				QColor delay5{204, 204, 204};
				QColor delay6{153, 153, 153};
				QColor delay7{102, 102, 102};
				QColor qcWarning{Qt::yellow};
				QColor qcError{Qt::red};
				QColor qcOk{Qt::green};
				QColor qcNotSet{0, 0, 0};
			};

			struct ConfigGradient {
				Gradient gradient;
				bool     discrete;
			};

			struct OriginSymbol {
				OriginSymbol();
				bool           classic{false};
				ConfigGradient depth;
			};

			struct OriginStatus {
				QColor automatic{Qt::red};
				QColor manual{Qt::darkGreen};
			};

			struct GroundMotion {
				QColor gmNotSet{0, 0, 0};
				QColor gm0{0, 0, 255};
				QColor gm1{0, 0, 255};
				QColor gm2{0, 167, 255};
				QColor gm3{0, 238, 255};
				QColor gm4{0, 255,  0};
				QColor gm5{255, 255, 0};
				QColor gm6{255, 210, 0};
				QColor gm7{255, 160, 0};
				QColor gm8{255, 0, 0};
				QColor gm9{160, 0, 60};
			};

			struct RecordView {
				QColor selectedTraceZoom{192, 192, 255, 192};
			};

			struct Map {
				QColor lines{255, 255, 255, 64};
				QColor outlines{255, 255, 255};
				QPen   directivity{{255 ,160, 0}};
				QPen   grid{Qt::white, 1, Qt::DotLine};
				QColor stationAnnotations{Qt::red};
				QColor cityLabels{Qt::black};
				QColor cityOutlines{Qt::black};
				QColor cityCapital{255, 160, 122};
				QColor cityNormal{Qt::white};
				QColor cityHalo{Qt::white};

				struct {
					QPen   normalText{{192,192,192}};
					QPen   normalBorder{{160,160,164}};
					QBrush normalBackground{{32,32,32,192}};

					QPen   highlightedText{{0,0,0}};
					QPen   highlightedBorder{{160,160,164}};
					QBrush highlightedBackground{{255,255,255,192}};

					int    textSize{9};
				}      annotations;
			};

			struct Legend {
				QColor background{255, 255, 255, 224};
				QColor border{160, 160, 160};
				QColor text{64, 64, 64};
				QColor headerText{0, 0, 0};
			};

			public:
				QColor                    background;
				Splash                    splash;
				Records                   records;
				Gradient                  spectrogram;
				Picks                     picks;
				Arrivals                  arrivals;
				Magnitudes                magnitudes;
				Stations                  stations;
				QC                        qc;
				OriginSymbol              originSymbol;
				OriginStatus              originStatus;
				GroundMotion              gm;
				RecordView                recordView;
				Map                       map;
				Legend                    legend;
				QMap<std::string, QColor> agencies;
		};

		struct Fonts {
			void setBase(const QFont& f);

			QFont base;
			QFont smaller;
			QFont normal;
			QFont large;
			QFont highlight;
			QFont heading1;
			QFont heading2;
			QFont heading3;

			QFont cityLabels;
			QFont splashVersion;
			QFont splashMessage;
		};

		struct Splash {
			struct Pos {
				QPoint pos;
				int    align;
			};

			Pos version{{390, 145}, Qt::AlignRight | Qt::AlignTop};
			Pos message{{30, 371}, Qt::AlignLeft | Qt::AlignBottom};
		};

		struct Map {
			enum class StationSymbol {
				Triangle,
				Diamond,
				Box
			};

			int           stationSize{12};
			StationSymbol stationSymbol{StationSymbol::Triangle};
			int           originSymbolMinSize{9};
			double        originSymbolMinMag{1.2};
			double        originSymbolScaleMag{4.9};
			bool          vectorLayerAntiAlias{true};
			bool          bilinearFilter{true};
			bool          showGrid{true};
			bool          showLayers{true};
			bool          showCities{true};
			bool          showLegends{false};
			int           cityPopulationWeight{150};
			int           cityHaloWidth{0};
			bool          toBGR{false};
			int           polygonRoughness{3};
			std::string   projection;
			int           maxZoom{24};
		};

		struct Marker {
			int lineWidth{1};
		};

		struct RecordBorders {
			Gui::RecordWidget::RecordBorderDrawMode drawMode{Gui::RecordWidget::Box};
		};

		struct Records {
			int           lineWidth{1};
			bool          antiAliasing{true};
			bool          optimize{true};
			bool          showEngineeringValues{true};
			RecordBorders recordBorders;
		};

		struct Precision {
			int depth{0};
			int distance{1};
			int location{2};
			int magnitude{1};
			int originTime{0};
			int pickTime{1};
			int traceValues{1};
			int rms{1};
			int uncertainties{0};
		};

		struct Unit {
			bool distanceInKM{false};
		};

		struct DateTime {
			bool useLocalTime{false};
		};

	public:
		bool      showMenu{true};
		bool      showStatusBar{true};
		int       tabPosition{-1};
		bool      distanceHypocentral{false};

		Splash    splash;
		Colors    colors;
		Marker    marker;
		Records   records;
		Map       map;
		Precision precision;
		Unit      unit;
		DateTime  dateTime;

		Fonts     fonts;

		QFont font(int relativeFontSize, bool bold = false, bool italic = false);

		void fetch();

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC_GUI_API QColor blend(const QColor& c1, const QColor& c2, int percentOfC1);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SC_GUI_API QColor blend(const QColor& c1, const QColor& c2);


}
}


#endif

