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



#ifndef SEISCOMP_GUI_RECORDPOLYLINE_H
#define SEISCOMP_GUI_RECORDPOLYLINE_H


#include <QPen>
#include <QPolygon>
#include <QPainter>
#include <QVector>

#ifndef Q_MOC_RUN
#include <seiscomp/core/record.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/recordsequence.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


DEFINE_SMARTPOINTER(AbstractRecordPolyline);
class SC_GUI_API AbstractRecordPolyline : public Seiscomp::Core::BaseObject {
	public:
		virtual void draw(QPainter &) = 0;
		virtual void drawGaps(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) = 0;
		virtual void draw(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) = 0;
		virtual bool isEmpty() const = 0;

		qreal baseline() const;

	protected:
		qreal _baseline;
};


DEFINE_SMARTPOINTER(RecordPolyline);
class SC_GUI_API RecordPolyline : public AbstractRecordPolyline,
                                  public QVector<QPolygon> {
	public:
		RecordPolyline();


	public:
		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(Record const *, double pixelPerSecond,
		            double amplMin, double amplMax, double amplOffset,
		            int height, float *timingQuality = nullptr,
		            bool optimization = true);

		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(RecordSequence const *, double pixelPerSecond,
		            double amplMin, double amplMax, double amplOffset,
		            int height, float *timingQuality = nullptr,
		            QVector<QPair<int,int> >* gaps = nullptr,
		            bool optimization = true);

		void create(RecordSequence const *,
		            const Core::Time &start,
		            const Core::Time &end,
		            double pixelPerSecond,
		            double amplMin, double amplMax, double amplOffset,
		            int height, float *timingQuality = nullptr,
		            QVector<QPair<int,int> >* gaps = nullptr,
		            bool optimization = true);

		void createStepFunction(RecordSequence const *, double pixelPerSecond,
		                        double amplMin, double amplMax, double amplOffset,
		                        int height, float multiplier = 1.0);

		void createSteps(RecordSequence const *, double pixelPerSecond,
		                 double amplMin, double amplMax, double amplOffset,
		                 int height, QVector<QPair<int,int> >* gaps = nullptr);

		void createSteps(RecordSequence const *,
		                 const Core::Time &start,
		                 const Core::Time &end,
		                 double pixelPerSecond,
		                 double amplMin, double amplMax, double amplOffset,
		                 int height, QVector<QPair<int,int> >* gaps = nullptr);

		// Returns the number of points
		int points() const;

	public:
		void draw(QPainter&) override;
		void drawGaps(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) override;
		void draw(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) override;
		bool isEmpty() const override { return QVector<QPolygon>::isEmpty(); }


	private:
		template <typename T>
		void pushRecord(QPolygon *&poly, const T *data, int count,
		                bool merge,
		                double yscl, double amplOffset,
		                bool optimization,
		                int x0, double sampleWidth,
		                int &collapsedSamples,
		                int &y_min, int &y_max,
		                int &x_out, int &y_out,
		                int &x_pos, int &y_pos);
};


DEFINE_SMARTPOINTER(RecordPolylineF);
class SC_GUI_API RecordPolylineF : public AbstractRecordPolyline,
                                   public QVector<QPolygonF> {
	public:
		RecordPolylineF();


	public:
		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(Record const *, double pixelPerSecond,
		            double amplMin, double amplMax, double amplOffset,
		            int height, float *timingQuality = nullptr,
		            bool optimization = true);

		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(RecordSequence const *, double pixelPerSecond,
		            double amplMin, double amplMax, double amplOffset,
		            int height, float *timingQuality = nullptr,
		            QVector<QPair<qreal,qreal> >* gaps = nullptr,
		            bool optimization = true);

		void create(RecordSequence const *,
		            const Core::Time &start,
		            const Core::Time &end,
		            double pixelPerSecond,
		            double amplMin, double amplMax, double amplOffset,
		            int height, float *timingQuality = nullptr,
		            QVector<QPair<qreal,qreal> >* gaps = nullptr,
		            bool optimization = true);

		// Returns the number of points
		int points() const;

	public:
		void draw(QPainter &) override;
		void drawGaps(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) override;
		void draw(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) override;
		bool isEmpty() const override { return QVector<QPolygonF>::isEmpty(); }


	private:
		template <typename T>
		void pushRecord(QPolygonF *&poly, const T *data, int count,
		                bool merge,
		                double yscl, double amplOffset,
		                bool optimization,
		                qreal x0, double sampleWidth,
		                int &collapsedSamples,
		                qreal &y_min, qreal &y_max,
		                qreal &x_out, qreal &y_out,
		                qreal &x_pos, qreal &y_pos);
};


inline qreal AbstractRecordPolyline::baseline() const {
	return _baseline;
}



}
}


# endif
