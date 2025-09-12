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
		IconEngine(const QString &name)
		: _name(name) {}

		IconEngine(const QColor &cOnOff, const QString &name)
		: _color(cOnOff), _colorOff(cOnOff), _name(name) {}

		IconEngine(const QColor &cOn, const QColor &cOff, const QString &name)
		: _color(cOn), _colorOff(cOff), _name(name) {}

	public:
		QIconEngine *clone() const override {
			return new IconEngine(_color, _colorOff, _name);
		}

		QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override {
			return Seiscomp::Gui::pixmap(_name, size).size();
		}

		QString key() const override {
			return "sc_svg_coloured";
		}

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		QString iconName() override {
#else
		QString iconName() const override {
#endif
			return _name;
		}

		void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
			auto pm = IconEngine::pixmap(rect.size(), mode, state);
			painter->drawPixmap(rect.left(), rect.top(), pm);
		}

		QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
			QColor color = ( state == QIcon::On ) ? _color : _colorOff;
			if ( !color.isValid() ) {
				if ( mode == QIcon::Disabled ) {
					color = qApp->palette().color(QPalette::Disabled, QPalette::Text);
				}
				else if ( mode == QIcon::Normal ) {
					color = qApp->palette().color(QPalette::Normal, QPalette::Text);
				}
				else if ( mode == QIcon::Active ) {
					color = qApp->palette().color(QPalette::Active, QPalette::Text);
				}
				else if ( mode == QIcon::Selected ) {
					color = qApp->palette().color(QPalette::Active, QPalette::Text);
				}
			}

			return Seiscomp::Gui::pixmap(_name, size, color);
		}

	private:
		QColor  _color;
		QColor  _colorOff;
		QString _name;
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
	// A good source for SVG icons is Google material at
	// https://fonts.google.com/icons?icon.set=Material+Symbols&icon.style=Rounded&selected=Material+Symbols+Rounded:check:FILL@1;wght@400;GRAD@0;opsz@24&icon.size=24&icon.color=%23000000
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

int iconSize(const QFontMetricsF &fm, double scale) {
	return static_cast<int>(fm.height() * scale);
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QString &name, const QSize &size, const QColor &c) {
	QString cacheId = QString("sc_svg_%1_%2x%3_%4")
	                  .arg(name).arg(size.width()).arg(size.height()).arg(c.name(QColor::HexArgb));
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

	QImage img(actualSize, QImage::Format_ARGB32_Premultiplied);
	img.fill(Qt::transparent);
	QPainter p(&img);
	renderer.render(&p);
	p.end();

	pm = QPixmap::fromImage(img);
	QPixmapCache::insert(cacheId, pm);

	return pm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QFontMetrics &fm, const QString &name, double scale) {
	auto e = iconSize(fm, scale);
	return pixmap(name, QSize(e, e));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QString &name, double scale) {
	return pixmap(QFontMetrics(parent->font()), name, scale);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &color, double scale) {
	auto e = iconSize(fm, scale);
	return pixmap(name, QSize(e, e), color);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &color, double scale) {
	return pixmap(QFontMetrics(parent->font()), name, color, scale);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QFontMetrics &fm, const QIcon &icon, double scale,
               QIcon::Mode mode, QIcon::State state) {
	return icon.pixmap(iconSize(fm, scale), mode, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QIcon &icon, double scale,
               QIcon::Mode mode, QIcon::State state) {
	return pixmap(QFontMetrics(parent->font()), icon, scale, mode, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
