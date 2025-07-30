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


#define SEISCOMP_COMPONENT Gui::MapWidget

#include <seiscomp/geo/featureset.h>
#include <seiscomp/geo/formats/geojson.h>
#include <seiscomp/gui/map/mapwidget.h>
#include <seiscomp/gui/map/projection.h>
#include <seiscomp/gui/map/texturecache.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/geo.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>
#include <QWidget>

#include <cmath>

#ifdef WIN32
#undef min
#undef max
#endif


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


const char *cmStrMeasure = "Measurements";
const char *cmStrMeasureClipboard = "Copy to Clipboard";
const char *cmStrMeasureSaveGeoFeature = "Save as GeoJSON/BNA File";
const char *cmStrProjection = "Projection";
const char *cmStrFilter = "Filter";
const char *cmStrNearest = "Nearest";
const char *cmStrBilinear = "Bilinear";
const char *cmStrScreenshot = "Save image";


inline QString lat2String(double lat, int precision) {
	return QString("%1%2")
	       .arg(fabs(lat), precision + 3, 'f', precision)
	       .arg(lat < 0 ? " S" : (lat > 0 ? " N" : ""));
}

inline QString lon2String(double lon, int precision) {
	lon = fmod(lon, 360.0);

	if ( lon < 0 ) {
		lon += 360.0;
	}

	if ( lon > 180.0 ) {
		lon -= 360.0;
	}

	return QString("%1%2")
	        .arg(fabs(lon), precision + 4, 'f', precision)
	        .arg(lon < 0 ? " W" : (lon > 0 ? " E" : ""));
}


inline double hav(double x) {
	return (1.0 - cos(x)) / 2.0;
}

// TODO: fix poly area for sourth pole
double polyArea(const QVector<QPointF> &coords) {
	double lat1;
	double lon1;
	double lat2;
	double lon2;
	double e;
	double sum = 0.0;

	for ( int i = 0 ; i < coords.size() ; ++i ) {
		lat1 = coords[i].y() * M_PI / 180.0;
		lon1 = coords[i].x() * M_PI / 180.0;
		lat2 = coords[(i + 1) % coords.size()].y() * M_PI / 180.0;
		lon2 = coords[(i + 1) % coords.size()].x() * M_PI / 180.0;

		if ( lon1 == lon2 ) {
			continue;
		}

		double h = hav(lat2 - lat1) + cos(lat1) * cos(lat2) * hav(lon2 - lon1);

		double a = 2 * asin(sqrt(h));
		double b = M_PI / 2.0 - lat2;
		double c = M_PI / 2.0 - lat1;
		double s = 0.5 * (a + b + c);
		double t = tan(s / 2.0 ) * tan((s - a) / 2.0) *
		           tan((s - b) / 2.0) * tan((s - c) / 2);

		e = fabs(4.0 * atan(sqrt(fabs(t))));
		if ( lon2 < lon1 ) {
			e = -e;
		}
		if ( fabs(lon2 - lon1) > M_PI ) {
			e = -e;
		}
		sum += e;
	}

	return fabs(sum) * 6378.137 * 6378.137;
}


} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SaveGeoFeatureDialog::SaveGeoFeatureDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f) {
	this->setWindowTitle(cmStrMeasureSaveGeoFeature);
	auto *layout = new QGridLayout();
	int row = 0;

	// polygon name
	layout->addWidget(new QLabel("Name"), row, 0);
	name = new QLineEdit("", this);
	name->setToolTip("Name of the polygon, may be plotted on map");
	layout->addWidget(name, row++, 1);

	// rank
	layout->addWidget(new QLabel("Rank"), row, 0);
	rank = new QSpinBox(this);
	rank->setMinimum(1);
	rank->setMaximum(1000);
	rank->setValue(1);
	rank->setToolTip("Zoom level the polygon will become visible");
	layout->addWidget(rank, row++, 1);

	// is closed polygon
	closedPolygon = new QCheckBox("Closed Polygon");
	closedPolygon->setToolTip("Defines if your line stroke should be saved as "
	                          "closed polygon or an open polyline");
	closedPolygon->setChecked(true);
	layout->addWidget(closedPolygon, row++, 1);

	// file name
	layout->addWidget(new QLabel("File Name"), row, 0);
	filename = new QLineEdit(QString("%1/spatial/vector/custom.geojson")
	                         .arg(Seiscomp::Environment::Instance()->shareDir().c_str()));

	std::ostringstream toolTip;
	std::ostringstream toolTipPath;
	toolTip << "Path and file name. ";
	QDir dir1((Environment::Instance()->shareDir() + "/bna").c_str());
	QDir dir2((Environment::Instance()->configDir() + "/bna").c_str());
	if ( dir1.exists() || dir2.exists() ) {
		toolTipPath << "Writing to 'spatial/' is default. "
		               "GeoJSON files will be ignored in:\n";
		if ( dir1.exists() ) {
			toolTipPath << Environment::Instance()->shareDir().c_str() << "/bna";
		}
		if ( dir1.exists() && dir2.exists() ) {
			toolTipPath << "\n";
		}
		if ( dir2.exists() ) {
			toolTipPath << Environment::Instance()->configDir().c_str() << "/bna";
		}
		toolTip << toolTipPath.str();
	}
	filename->setToolTip(toolTip.str().c_str());
	layout->addWidget(filename, row++, 1);
	if ( dir1.exists() || dir2.exists() ) {
		layout->addWidget(new QLabel(toolTipPath.str().c_str()), row++, 1);
	}

	// is closed polygon
	fileAppend = new QCheckBox("Append to File");
	fileAppend->setToolTip("Defines if the new geo feature should be appended "
	                       "to an existing file or if the file should be "
	                       "overridden");
	fileAppend->setChecked(true);
	layout->addWidget(fileAppend, row++, 1);

	// OK + Cancel buttons
	auto *buttonBox =
	        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(buttonBox, row++, 0, 1, 0);
	this->setLayout(layout);
	this->setMinimumSize(450,0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget::MapWidget(QWidget *parent, Qt::WindowFlags f)
 : QWidget(parent, f), _canvas(nullptr) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget::MapWidget(const MapsDesc &meta, QWidget *parent, Qt::WindowFlags f)
 : QWidget(parent, f), _canvas(meta) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget::MapWidget(Map::ImageTree *mapTree, QWidget *parent, Qt::WindowFlags f)
 : QWidget(parent, f), _canvas(mapTree) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::init() {
	_canvas.setBackgroundColor(palette().color(QPalette::Window));
	connect(&_canvas, SIGNAL(bufferUpdated()), this, SLOT(bufferUpdated()));
	connect(&_canvas, SIGNAL(projectionChanged(Seiscomp::Gui::Map::Projection*)),
	        this, SLOT(projectionChanged(Seiscomp::Gui::Map::Projection*)));
	connect(&_canvas, SIGNAL(customLayer(QPainter*)),
	        this, SLOT(drawCustomLayer(QPainter*)));
	connect(&_canvas, SIGNAL(updateRequested()), this, SLOT(update()));

	_isDragging = false;
	_isZooming = false;
	_isMeasuring = false;
	_filterMap = SCScheme.map.bilinearFilter;
	_canvas.setBilinearFilter(_filterMap);

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	//setAttribute(Qt::WA_PaintOnScreen);

	try { _zoomSensitivity = SCApp->configGetDouble("map.zoom.sensitivity"); }
	catch ( ... ) { _zoomSensitivity = 0.5; }

	_zoomControls = new QWidget(this);
	auto *zoomIn = new QToolButton;
	auto *zoomOut = new QToolButton;
	auto *zoomLayout = new QVBoxLayout;
	_zoomControls->setLayout(zoomLayout);
	zoomLayout->addWidget(zoomIn);
	zoomLayout->addWidget(zoomOut);

	zoomIn->setIcon(QIcon(":/map/icons/zoomin.png"));
	zoomOut->setIcon(QIcon(":/map/icons/zoomout.png"));

	_zoomControls->move(0,0);
	_zoomControls->hide();

	connect(zoomIn, SIGNAL(pressed()), this, SLOT(zoomIn()));
	connect(zoomOut, SIGNAL(pressed()), this, SLOT(zoomOut()));

	_measureSaveDialog = nullptr;
	_forceGrayScale = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::projectionChanged(Seiscomp::Gui::Map::Projection *p) {
	if ( p ) {
		setAttribute(Qt::WA_OpaquePaintEvent, p->isRectangular());
	}
	else {
		setAttribute(Qt::WA_OpaquePaintEvent, false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::zoomIn() {
	_canvas.setZoomLevel(_canvas.zoomLevel() * pow(2.0, _zoomSensitivity));
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::zoomOut() {
	_canvas.setZoomLevel(_canvas.zoomLevel() / pow(2.0, _zoomSensitivity));
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setDrawGrid(bool e) {
	_canvas.setDrawGrid(e);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setDrawLayers(bool e) {
	_canvas.setDrawLayers(e);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setDrawCities(bool e) {
	_canvas.setDrawCities(e);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setDrawLegends(bool e) {
	_canvas.setDrawLegends(e);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::setGrayScale(bool f) {
	if ( _forceGrayScale == f ) {
		return;
	}

	_forceGrayScale = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MapWidget::isGrayScale() const {
	return _forceGrayScale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MapWidget::saveScreenshot() {
	QString filename = QFileDialog::getSaveFileName(
		this,
		tr("Save image file"),
		QString(),
		tr("Images (*.png *.jpg *.bmp *.ppm *.xbm *.xpm)")
	);

	if ( filename.isEmpty() ) {
		return false;
	}

	QImage image(size(), QImage::Format_ARGB32);
	image.fill(0);

	QPainter p(&image);
	_canvas.draw(p);

	if ( !image.save(filename) ) {
		QMessageBox::warning(this, tr("Error"),
		                     tr("Failed to save image to %1").arg(filename));
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::draw(QPainter &painter) {
	_canvas.setPreviewMode(_isDragging);
	_canvas.setGrayScale(!isEnabled() || _forceGrayScale);
	_canvas.draw(painter);

	if ( _isZooming ) {
		if ( _firstDraggingPosition != _lastDraggingPosition ) {
			painter.setRenderHint(QPainter::Antialiasing, false);
			painter.setPen(Qt::white);
			painter.setBrush(QColor(255,255,255,128));
			painter.drawRect(QRect(_firstDraggingPosition, _lastDraggingPosition));
		}
	}

	if ( _isMeasuring ) {
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setPen(QPen(Qt::red, 2));

		// draw geo line
		QPoint p;
		double dist = 0.0;
		_canvas.projection()->project(p, _measurePoints[0]);
		painter.drawEllipse(QPointF(p), 1.3, 1.3);
		for ( int i = 1; i < _measurePoints.size(); ++i ) {
			_canvas.projection()->project(p, _measurePoints[i]);
			painter.drawEllipse(QPointF(p), 1.3, 1.3);
			dist += _canvas.drawLine(painter, _measurePoints[i-1], _measurePoints[i]);
		}

		QString aziArea;
		if ( _measurePoints.size() > 2 ) {
			painter.save();
			QPen pen = QPen(Qt::red, 1, Qt::DashLine);
			QVector<qreal> dashes;
			dashes << 3 << 7;
			pen.setDashPattern(dashes);
			painter.setPen(pen);
			_canvas.drawLine(painter, _measurePoints.last(), _measurePoints.first());
			painter.restore();
			aziArea = QString("Area    : %1 km²").arg(polyArea(_measurePoints));
		}
		else {
			double tmp;
			double azi1;
			double azi2;
			Math::Geo::delazi(_measurePoints.first().y(), _measurePoints.first().x(),
			                  _measurePoints.last().y(), _measurePoints.last().x(),
			                  &tmp, &azi1, &azi2);
			aziArea = QString("AZ / BAZ: %1 ° / %2 °")
			                  .arg(azi1, 0, 'f', 1)
			                  .arg(azi2, 0, 'f', 1);
		}

		int precision = 0;
		if ( _canvas.projection()->zoom() > 0 ) {
			precision = (int) log10(_canvas.projection()->zoom());
		}
		++precision;

		QString distStr = QString("Distance: %1 km / %2 °")
		                  .arg(Math::Geo::deg2km(dist), 0, 'f', precision)
		                  .arg(dist, 0, 'f', precision + 2);

		QFont f = painter.font();
		QFont mf = f;
		mf.setFamily("Monospace");
		mf.setStyleHint(QFont::TypeWriter);

		QFontMetrics mfm(mf);
		QFontMetrics fm(f);
		int h = mfm.height() * 4 + fm.height();
		int padding = QT_FM_WIDTH(fm, " ");
		QRect r = QRect(0, rect().height() - h, QT_FM_WIDTH(mfm, distStr) + 2*padding, h);
		painter.setPen(QPen(Qt::black));
		painter.fillRect(r, QBrush(QColor(255, 255, 255, 140)));

		r.setLeft(padding);
		painter.setFont(mf);
		_measureText = QString("Start   : %1 / %2\n"
		                       "End     : %3 / %4\n"
		                       "%5\n"
		                       "%6")
		               .arg(lat2String(_measurePoints.first().y(), precision),
		                    lon2String(_measurePoints.first().x(), precision),
		                    lat2String(_measurePoints.last().y(), precision),
		                    lon2String(_measurePoints.last().x(), precision),
		                    distStr, aziArea);

		painter.drawText(r, Qt::AlignLeft, _measureText);

		r.setTop(rect().height() - fm.height());
		r.setRight(r.width()-padding);
		painter.setFont(f);
		painter.drawText(r, Qt::AlignRight, "(right click to copy/save)");
		painter.restore();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::paintEvent(QPaintEvent* e) {
	QPainter painter(this);
	draw(painter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::resizeEvent(QResizeEvent* event) {
	_canvas.setSize(event->size().width(), event->size().height());
	_zoomControls->resize(_zoomControls->sizeHint());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::updateContextMenu(QMenu *menu) {
	QAction *action;

	_contextProjectionMenu = nullptr;
	_contextFilterMenu = nullptr;

	menu->addAction(cmStrScreenshot);

	// Copy Measurements
	if ( !_measureText.isEmpty() ) {
		QMenu *measurementsMenu = menu->addMenu(cmStrMeasure);
		measurementsMenu->addAction(cmStrMeasureClipboard);
		measurementsMenu->addAction(cmStrMeasureSaveGeoFeature);
	}

	// Projection and Filters
	auto *services = Map::ProjectionFactory::Services();
	if ( services ) {
		if ( services->size() > 1 ) {
			_contextProjectionMenu = menu->addMenu(cmStrProjection);
		}
		_contextFilterMenu = menu->addMenu(cmStrFilter);

		for ( const auto &service : *services ) {
			if ( _contextProjectionMenu ) {
				action = _contextProjectionMenu->addAction(service.c_str());
				if ( service == _canvas.projectionName() ) {
					action->setEnabled(false);
				}
			}
		}
		delete services;

		action = _contextFilterMenu->addAction(cmStrNearest);
		action->setEnabled(_filterMap);
		action = _contextFilterMenu->addAction(cmStrBilinear);
		action->setEnabled(!_filterMap);
	}

	_canvas.menu(menu);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::executeContextMenuAction(QAction *action) {
	if ( action == nullptr ) {
		_contextProjectionMenu = nullptr;
		_contextFilterMenu = nullptr;
		return;
	}

	QString actionText = action->text();
	if ( actionText[0] == '&' ) {
		actionText.remove(0, 1);
	}

	if ( actionText == cmStrScreenshot ) {
		saveScreenshot();
	}
	else if ( _contextProjectionMenu && action->parent() == _contextProjectionMenu ) {
		_canvas.setProjectionByName(actionText.toStdString().c_str());
	}
	else if ( _contextFilterMenu && action->parent() == _contextFilterMenu ) {
		_filterMap = actionText == cmStrBilinear;
		_canvas.setBilinearFilter(_filterMap);
	}
	else if ( actionText == cmStrMeasureClipboard ) {
		QString text = _measureText;
		text.append("\n\nlat lon");
		for ( const auto &p : std::as_const(_measurePoints) ) {
			text.append(QString("\n%1 %2").arg(p.y()).arg(p.x()));
		}
		QApplication::clipboard()->setText(text);
	}
	else if ( actionText == cmStrMeasureSaveGeoFeature ) {
		if ( !_measureSaveDialog ) {
			_measureSaveDialog = new SaveGeoFeatureDialog(this);
		}

		while ( _measureSaveDialog->exec() ) {
			bool append = _measureSaveDialog->fileAppend->isChecked();
			QFileInfo fileInfo(_measureSaveDialog->filename->text());
			if ( fileInfo.isDir() ) {
				QMessageBox::warning(this, "Invalid file name",
				                     "The specified file is a directory");
				continue;
			}

			if ( fileInfo.suffix() != "bna" && fileInfo.suffix() != "geojson" ) {
				QMessageBox::warning(this, "Invalid format",
				                     QString("Unsupported file format: %s")
				                            .arg(fileInfo.suffix()));
				continue;
			}

			QDir dir = fileInfo.absoluteDir();
			if ( !dir.exists() && !dir.mkpath(".") ) {
				QMessageBox::warning(this, "Error creating path",
				                     QString("Could not create file path: %1")
				                            .arg(dir.absolutePath()));
				continue;
			}

			if ( !append && fileInfo.isFile() && QMessageBox::question(
			         this, "File exists", "The specified file already exists. "
			         "Do you want to override it?",
			         QMessageBox::Yes|QMessageBox::No) == QMessageBox::No ) {
				continue;
			}

			bool closedPolygon = _measureSaveDialog->closedPolygon->isChecked();
			if ( fileInfo.suffix() == "bna" ) {
				QFile file(fileInfo.absoluteFilePath());
				if ( !file.open(QIODevice::WriteOnly |
				                (append ? QIODevice::Append : QIODevice::Truncate)) ) {
					QMessageBox::warning(this, "Could not open file",
					                     QString("Could not open file for writing: %1")
					                            .arg(fileInfo.absoluteFilePath()));
					continue;
				}

				QTextStream stream(&file);
				QString header = QString(R"("%1","rank %2",%3)")
				    .arg(_measureSaveDialog->name->text())
				    .arg(_measureSaveDialog->rank->value())
				    .arg(closedPolygon?_measurePoints.size():-_measurePoints.size());
				stream << header << "\n";
				for ( const auto &p : std::as_const(_measurePoints) ) {
					stream << p.x() << ","<< p.y() << "\n";
				}
				file.close();
			}
			else if ( fileInfo.suffix() == "geojson" ) {
				auto path = fileInfo.absoluteFilePath().toStdString();
				Geo::GeoFeatureSet gfs;
				if ( append && fileInfo.isFile() ) {
					try {
						Geo::readGeoJSON(gfs, path);
					}
					catch ( Core::StreamException &e ) {
						QMessageBox::warning(this, "Could not open file",
						                     QString(e.what()));
						continue;
					}
				}

				auto *gf = new Geo::GeoFeature();
				for ( const auto &p : std::as_const(_measurePoints) ) {
					gf->addVertex(p.y(), p.x());
				}

				gf->setClosedPolygon(closedPolygon && gf->vertices().size() > 2);
				gf->setName(_measureSaveDialog->name->text().toStdString());
				gf->setRank(_measureSaveDialog->rank->value());

				if ( gfs.features().empty()  ) {
					if ( !writeGeoJSON(path, *gf, 2) ) {
						QMessageBox::warning(this, "Could not write feature", "");
					}
					delete gf;
					gf = nullptr;
				}
				else {
					gfs.addFeature(gf); // gfs takes ownership
					auto gfWritten = writeGeoJSON(path, gfs.features(), 2);
					if ( gfWritten != gfs.features().size() ) {
						auto msg = QString("%1 instead of %2 features have been "
						                   "written to: %3")
						           .arg(QString::number(gfWritten),
						                QString::number(gfs.features().size()),
						                path.c_str());
						QMessageBox::warning(this,
						                     "Wrote invalid number of features",
						                     msg);
					}
				}
			}

			break;
		}
	}

	_contextProjectionMenu = nullptr;
	_contextFilterMenu = nullptr;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::contextMenuEvent(QContextMenuEvent *event) {
	if ( _canvas.filterContextMenuEvent(event, this) ) {
		return;
	}

	QMenu menu(this);

	updateContextMenu(&menu);

	// Evaluation action performed
	executeContextMenuAction(menu.exec(event->globalPos()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mousePressEvent(QMouseEvent* event) {
	if ( event->button() == Qt::LeftButton ) {
		_firstDraggingPosition = event->pos();
		_lastDraggingPosition = _firstDraggingPosition;
		_firstDrag = true;

		if ( event->modifiers() == Qt::ShiftModifier ) {
			_isZooming = true;
			setToolTip(QString());
			update();
		}
		else if ( event->modifiers() == Qt::ControlModifier ) {
			QPointF p;
			_canvas.projection()->unproject(p, _lastDraggingPosition);
			if ( !_isMeasuring ) {
				_isMeasuring = true;
				_measurePoints.push_back(p);
			}
			_measurePoints.push_back(p);
			unsetCursor();
			setToolTip(QString());
			update();
			return;
		}

		if ( event->modifiers() == Qt::NoModifier ) {
			if ( _isMeasuring ) {
				_isMeasuring = false;
				_measurePoints.clear();
				_measureText.clear();
			}

			_isDragging = true;
		}
	}

	_canvas.filterMousePressEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mouseReleaseEvent(QMouseEvent* event) {
	if ( event->button() == Qt::LeftButton ) {
		if ( _isDragging ) {
			_isDragging = false;
			update();
		}
		else if ( _isZooming ) {
			_isZooming = false;
			if ( _firstDraggingPosition != _lastDraggingPosition ) {
				QPointF p0;
				QPointF p1;
				_canvas.projection()->unproject(p0, _firstDraggingPosition);
				_canvas.projection()->unproject(p1, _lastDraggingPosition);
				canvas().displayRect(QRectF(p0, p1).normalized());
				update();
			}
		}
	}

	_canvas.filterMouseReleaseEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mouseMoveEvent(QMouseEvent* event) {
	if ( _isDragging ) {
		if ( _firstDrag ) {
			_firstDrag = false;
		}

		QPoint delta = event->pos() - _lastDraggingPosition;
		_lastDraggingPosition = event->pos();

		_canvas.translate(delta);

		update();
	}
	else if ( _isZooming ) {
		_lastDraggingPosition = event->pos();
		update();
	}
	else if ( _isMeasuring ) {
		_canvas.projection()->unproject(_measurePoints.last(), event->pos());
		_canvas.filterMouseMoveEvent(event);
		update();
	}
	else {
		if ( !_canvas.filterMouseMoveEvent(event) ) {
			_zoomControls->setVisible(_zoomControls->geometry().contains(event->pos()));
		}

		bool hasLayerCursor = false;
		bool hasLayerToolTip = false;

		if ( _canvas.hoverLayer() ) {
			Map::Layer *l = _canvas.hoverLayer();
			if ( l ) {
				if ( l->hasCursorShape() ) {
					setCursor(l->cursorShape());
					hasLayerCursor = true;
				}

				if ( !l->toolTip().isEmpty() ) {
					setToolTip(l->toolTip());
					hasLayerToolTip = true;
				}
			}
		}

		if ( !hasLayerCursor ) {
			unsetCursor();
		}

		if ( !hasLayerToolTip ) {
			setToolTip(QString());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
	if ( event->button() == Qt::LeftButton &&
	     !_canvas.filterMouseDoubleClickEvent(event) ) {
		_canvas.centerMap(event->pos());
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::keyPressEvent(QKeyEvent* e) {
	if ( _canvas.filterKeyPressEvent(e) ) {
		bool hasLayerCursor = false;

		if ( _canvas.hoverLayer() ) {
			Map::Layer *l = _canvas.hoverLayer();
			if ( l && l->hasCursorShape() ) {
				setCursor(l->cursorShape());
				hasLayerCursor = true;
			}
		}

		if ( !hasLayerCursor ) {
			unsetCursor();
		}

		e->accept();
		return;
	}

	e->accept();

	int key = e->key();

	switch ( key ) {
		case Qt::Key_Plus:
		case Qt::Key_I:
			if ( e->modifiers() == Qt::NoModifier ) {
				zoomIn();
			}
			break;
		case Qt::Key_Minus:
		case Qt::Key_O:
			if ( e->modifiers() == Qt::NoModifier ) {
				zoomOut();
			}
			break;
		case Qt::Key_Left:
			_canvas.translate(QPointF(-1.0,0.0));
			update();
			break;
		case Qt::Key_Right:
			_canvas.translate(QPointF(1.0,0.0));
			update();
			break;
		case Qt::Key_Up:
			_canvas.translate(QPointF(0.0,1.0));
			update();
			break;
		case Qt::Key_Down:
			_canvas.translate(QPointF(0.0,-1.0));
			update();
			break;
		case Qt::Key_G:
			_canvas.setDrawGrid(!_canvas.isDrawGridEnabled());
			break;
		case Qt::Key_C:
			_canvas.setDrawCities(!_canvas.isDrawCitiesEnabled());
			break;
		default:
			e->ignore();
			emit keyPressed(e);
			break;
	};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::keyReleaseEvent(QKeyEvent *e) {
	if ( _canvas.filterKeyReleaseEvent(e) ) {
		bool hasLayerCursor = false;

		if ( _canvas.hoverLayer() ) {
			Map::Layer *l = _canvas.hoverLayer();
			if ( l && l->hasCursorShape() ) {
				setCursor(l->cursorShape());
				hasLayerCursor = true;
			}
		}

		if ( !hasLayerCursor ) {
			unsetCursor();
		}

		e->accept();
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::wheelEvent(QWheelEvent *e) {
	double zoomDelta = static_cast<double>(QT_WE_DELTA(e)) * (1.0/120.0);
	if ( _canvas.setZoomLevel(_canvas.zoomLevel() * pow(2.0, zoomDelta*_zoomSensitivity)) ) {
		update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::leaveEvent(QEvent *e) {
	QWidget::leaveEvent(e);
	_zoomControls->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MapWidget::heightForWidth(int w) const {
	return w;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MapWidget::bufferUpdated() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Seiscomp::Gui
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
