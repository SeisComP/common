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


#include <QApplication>
#include <QIconEngine>
#include <QSvgRenderer>
#include <QPainter>
#include <QPalette>
#include <QWidget>
#include <QFile>
#include <QPixmapCache>

#include "icon.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


class IconEngine : public QIconEngine {
	public:
		explicit IconEngine(const QString &name) {
			parseName(name);
		}

		explicit IconEngine(const QColor &cOnOff, const QString &name)
		: _color(cOnOff), _colorOff(cOnOff) {
			parseName(name);
		}

		explicit IconEngine(const QColor &cOn, const QColor &cOff, const QString &name)
		: _color(cOn), _colorOff(cOff) {
			parseName(name);
		}

	private:
		explicit IconEngine(const QColor &cOn, const QColor &cOff, const QString &nameOn, const QString &nameOff)
		: _color(cOn), _colorOff(cOff), _nameOn(nameOn), _nameOff(nameOff) {}

	private:
		void parseName(const QString &name) {
			int p = name.indexOf('|');
			if ( p < 0 ) {
				_nameOn = _nameOff = name;
			}
			else {
				_nameOn = name.left(p);
				_nameOff = name.right(name.length() - p - 1);
			}
		}

	public:
		QIconEngine *clone() const override {
			return new IconEngine(_color, _colorOff, _nameOn, _nameOff);
		}

		QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override {
			return Seiscomp::Gui::pixmap(_nameOn, size, 1.0).size();
		}

		QString key() const override {
			return "sc_svg_coloured";
		}

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		QString iconName() override {
#else
		QString iconName() const override {
#endif
			return _nameOn != _nameOff ? (_nameOn + '|' + _nameOff) : _nameOn;
		}

		void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
			QColor color = ( state == QIcon::On ) ? _color : _colorOff;
			if ( !color.isValid() ) {
				color = qApp->palette().color(QPalette::Normal, QPalette::Text);
			}

			QColor scaledColor;
			if ( mode == QIcon::Disabled ) {
				scaledColor = qApp->palette().color(QPalette::Disabled, QPalette::Text);
			}

			painter->drawPixmap(
				rect.left(), rect.top(),
				Seiscomp::Gui::pixmap(
					state == QIcon::On ? _nameOn : _nameOff,
					rect.size(), painter->device()->devicePixelRatioF(), color, scaledColor
				)
			);
		}

		QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
			QColor color = ( state == QIcon::On ) ? _color : _colorOff;
			if ( !color.isValid() ) {
				color = qApp->palette().color(QPalette::Normal, QPalette::Text);
			}

			QColor scaledColor;
			if ( mode == QIcon::Disabled ) {
				scaledColor = qApp->palette().color(QPalette::Disabled, QPalette::Text);
			}

			return Seiscomp::Gui::pixmap(
				state == QIcon::On ? _nameOn : _nameOff,
				size, 1.0, color, scaledColor
			);
		}

	private:
		QColor  _color;
		QColor  _colorOff;
		QString _nameOn;
		QString _nameOff;
};


QByteArray changeColor(const QString &path, const QColor &c) {
	QFile f(path);
	if ( !f.open(QIODevice::ReadOnly) ) {
		return {};
	}

	auto content = f.readAll();
	if ( c.isValid() ) {
		auto colorHex = c.name(QColor::HexRgb);
		content.replace("#000000", 7, qPrintable(colorHex), colorHex.size());
	}

	return content;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QIcon icon(QString name) {
	return QIcon(new IconEngine(name));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QIcon icon(QString name, const QColor &cOn) {
	return QIcon(new IconEngine(cOn, name));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QIcon icon(QString name, const QColor &cOn, const QColor &cOff) {
	return QIcon(new IconEngine(cOn, cOff, name));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

int iconSize(const QFontMetricsF &fm) {
	return static_cast<int>(fm.height());
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QString &name, const QSize &size, double dpr,
               const QColor &c, const QColor &scaledColor) {
	QString cacheId = c.isValid() ?
		QString("sc_svg_%1_%2x%3_%4_%5")
		.arg(name).arg(size.width()).arg(size.height())
		.arg(c.name(QColor::HexArgb)).arg(dpr)
	:
		QString("sc_svg_%1_%2x%3_%4")
		.arg(name).arg(size.width()).arg(size.height()).arg(dpr)
	;

	if ( scaledColor.isValid() ) {
		cacheId += QString("_%1").arg(scaledColor.name(QColor::HexArgb));
	}

	QPixmap pm;
#if QT_VERSION < QT_VERSION_CHECK(5,13,0)
	if ( QPixmapCache::find(cacheId, pm) ) {
#else
	if ( QPixmapCache::find(cacheId, &pm) ) {
#endif
		return pm;
	}

	auto svg = changeColor(":/sc/icons/" + name + ".svg", c);
	QSvgRenderer renderer;
	if ( !renderer.load(svg) ) {
		return {};
	}

	if ( !renderer.isValid() ) {
		return {};
	}

	auto actualSize = renderer.defaultSize();
	if ( !actualSize.isNull() ) {
		actualSize.scale(size, Qt::KeepAspectRatio);
	}

	if ( actualSize.isEmpty() ) {
		return {};
	}

	actualSize *= dpr;

	QImage img(actualSize, QImage::Format_ARGB32);
	img.fill(Qt::transparent);
	{
		QPainter p(&img);
		renderer.render(&p);
		p.end();
	}

	if ( scaledColor.isValid() ) {
#if 1
		int cGray = 0;
		if ( c.isValid() ) {
			cGray = (299 * c.red() + 587 * c.green() + 114 * c.blue()) / 1000;
		}

		int count = img.width() * img.height();
		QRgb *data = reinterpret_cast<QRgb*>(img.bits());
		for ( int i = 0; i < count; ++i, ++data ) {
			int gray = (299 * qRed(*data) + 587 * qGreen(*data) + 114 * qBlue(*data)) / 1000;
			int diff = 255 + gray - cGray;
			int red = (diff * scaledColor.red()) >> 8;
			int green = (diff * scaledColor.green()) >> 8;
			int blue = (diff * scaledColor.blue()) >> 8;
			if ( red < 0 ) { red = 0; } else if ( red > 255 ) { red = 255; }
			if ( green < 0 ) { green = 0; } else if ( green > 255 ) { green = 255; }
			if ( blue < 0 ) { blue = 0; } else if ( blue > 255 ) { blue = 255; }
			*data = qRgba(red, green, blue, qAlpha(*data));
		}
#else
		int cRed = 0, cGreen = 0, cBlue = 0;
		if ( c.isValid() ) {
			cRed = c.red();
			cGreen = c.green();
			cBlue = c.blue();
		}

		int count = img.width() * img.height();
		QRgb *data = reinterpret_cast<QRgb*>(img.bits());
		for ( int i = 0; i < count; ++i, ++data ) {
			int red = ((255 + qRed(*data) - cRed) * scaledColor.red()) >> 8;
			int green = ((255 + qGreen(*data) - cGreen) * scaledColor.green()) >> 8;
			int blue = ((255 + qBlue(*data) - cBlue) * scaledColor.blue()) >> 8;
			if ( red < 0 ) { red = 0; } else if ( red > 255 ) { red = 255; }
			if ( green < 0 ) { green = 0; } else if ( green > 255 ) { green = 255; }
			if ( blue < 0 ) { blue = 0; } else if ( blue > 255 ) { blue = 255; }
			*data = qRgba(red, green, blue, qAlpha(*data));
		}
#endif
	}

	pm = QPixmap::fromImage(img);
	pm.setDevicePixelRatio(dpr);

	QPixmapCache::insert(cacheId, pm);

	return pm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &color, double dpr) {
	auto e = iconSize(fm);
	return pixmap(name, QSize(e, e), dpr, color);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &color, double scale) {
	auto e = iconSize(QFontMetrics(parent->font()));
	return pixmap(
		name, QSize(e, e) * scale,
		parent->devicePixelRatioF(), color
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
