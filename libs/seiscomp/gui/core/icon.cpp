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
#include <QPainter>
#include <QPalette>
#include <QWidget>

#include "icon.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


class IconEngine : public QIconEngine {
	public:
		IconEngine(const QIcon &i)
		: _icon(i) {}

		IconEngine(const QColor &cOnOff, const QIcon &i)
		: _color(cOnOff), _colorOff(cOnOff), _icon(i) {}

		IconEngine(const QColor &cOn, const QColor &cOff, const QIcon &i)
		: _color(cOn), _colorOff(cOff), _icon(i) {}

	public:
		QIconEngine *clone() const override {
			return new IconEngine(_color, _colorOff, _icon);
		}

		QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
			return _icon.actualSize(size, mode, state);
		}

		QString key() const override {
			return "sc_svg_coloured";
		}

		QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal,
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		                            QIcon::State state = QIcon::Off) override {
#else
		                            QIcon::State state = QIcon::Off) const override {
#endif
			return _icon.availableSizes(mode, state);
		}

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		QString iconName() override {
#else
		QString iconName() const override {
#endif

			return _icon.name();
		}

		void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
			auto pm = pixmap(rect.size(), mode, state);
			painter->drawPixmap(rect.left(), rect.top(), pm);
		}

		QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
			QPixmap pm(size);
			pm.fill(Qt::transparent);

			QPainter p(&pm);

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

			QRect r(QPoint(0, 0), size);
			_icon.paint(&p, r, Qt::AlignCenter, mode, state);
			p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
			p.fillRect(r, color);

			return pm;
		}

	private:
		QColor _color;
		QColor _colorOff;
		QIcon  _icon;
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QIcon icon(QString name) {
	// A good source for SVG icons is Google material at
	// https://fonts.google.com/icons?icon.set=Material+Symbols&icon.style=Rounded&selected=Material+Symbols+Rounded:check:FILL@1;wght@400;GRAD@0;opsz@24&icon.size=24&icon.color=%231f1f1f
	QIcon i(":/sc/icons/" + name + ".svg");
	auto engine = new IconEngine(i);
	return QIcon(engine);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QIcon icon(QString name, const QColor &cOn) {
	QIcon i(":/sc/icons/" + name + ".svg");
	auto engine = new IconEngine(cOn, i);
	return QIcon(engine);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QIcon icon(QString name, const QColor &cOn, const QColor &cOff) {
	QIcon i(":/sc/icons/" + name + ".svg");
	auto engine = new IconEngine(cOn, cOff, i);
	return QIcon(engine);
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
QPixmap pixmap(const QFontMetrics &fm, const QString &name, double scale) {
	return icon(name).pixmap(iconSize(fm, scale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QString &name, double scale) {
	return pixmap(QFontMetrics(parent->font()), name, scale);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &cOnOff, double scale) {
	return icon(name, cOnOff).pixmap(iconSize(fm, scale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &cOnOff, double scale) {
	return pixmap(QFontMetrics(parent->font()), name, cOnOff, scale);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &cOn, const QColor &cOff, double scale) {
	return icon(name, cOn, cOff).pixmap(iconSize(fm, scale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &cOn, const QColor &cOff, double scale) {
	return pixmap(QFontMetrics(parent->font()), name, cOn, cOff, scale);
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
