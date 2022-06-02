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


#ifndef SEISCOMP_GUI_MAPWIDGET_H
#define SEISCOMP_GUI_MAPWIDGET_H


#include <seiscomp/gui/map/canvas.h>

#ifndef Q_MOC_RUN
#include <seiscomp/math/coord.h>
#endif

#include <QColor>
#include <QDialog>
#include <QPen>
#include <QtGui>

class QLineEdit;
class QCheckBox;
class QSpinBox;


namespace Seiscomp {
namespace Gui {

class SaveBNADialog : public QDialog {
	Q_OBJECT

	public:
		SaveBNADialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

	public:
		QLineEdit *name;
		QCheckBox *closedPolygon;
		QCheckBox *fileAppend;
		QSpinBox  *rank;
		QLineEdit *filename;
};


class SC_GUI_API MapWidget : public QWidget {
	Q_OBJECT

	public:
		MapWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
		MapWidget(const MapsDesc &meta, QWidget *parent = 0, Qt::WindowFlags f = 0);
		MapWidget(Map::ImageTree *mapTree, QWidget *parent = 0, Qt::WindowFlags f = 0);
		virtual ~MapWidget();

		Map::Canvas &canvas() { return _canvas; }
		const Map::Canvas &canvas() const { return _canvas; }

		int heightForWidth(int w) const;

		bool isGrayScale() const;
		bool isMeasuring() const;
		bool isDragging() const;

		bool saveScreenshot();

		virtual void draw(QPainter&);


	public slots:
		/**
		 * @brief Sets map rendering in grayscale mode even if the widget is
		 *        enabled.
		 * @param f The enable flag
		 */
		void setGrayScale(bool f);

		void setDrawGrid(bool);
		void setDrawLayers(bool);
		void setDrawCities(bool);
		void setDrawLegends(bool);


	protected slots:
		virtual void bufferUpdated();
		virtual void drawCustomLayer(QPainter *p) {}


	private slots:
		void projectionChanged(Seiscomp::Gui::Map::Projection*);
		void zoomIn();
		void zoomOut();


	signals:
		void keyPressed(QKeyEvent*);


	protected:
		void paintEvent(QPaintEvent*);
		void resizeEvent(QResizeEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
		void wheelEvent(QWheelEvent*);
		void contextMenuEvent(QContextMenuEvent*);
		void leaveEvent(QEvent*);

		void updateContextMenu(QMenu *menu);
		void executeContextMenuAction(QAction *action);


	private:
		void init();


	private:
		Map::Canvas _canvas;

		std::string _currentProjection;

		bool     _firstDrag;
		bool     _isDragging;
		bool     _isMeasuring;
		bool     _filterMap;
		bool     _forceGrayScale;

		QVector<QPointF> _measurePoints;
		QString          _measureText;
		SaveBNADialog   *_measureBNADialog;
		QPoint   _lastDraggingPosition;

		QMenu   *_contextProjectionMenu;
		QMenu   *_contextFilterMenu;

		double   _zoomSensitivity;

		QWidget *_zoomControls;
};


inline bool MapWidget::isMeasuring() const {
	return _isMeasuring;
}

inline bool MapWidget::isDragging() const {
	return _isDragging;
}


}
}

#endif
