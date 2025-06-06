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


#ifndef SEISCOMP_GUI_MAP_LEGEND_H
#define SEISCOMP_GUI_MAP_LEGEND_H


#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#endif

#include <seiscomp/gui/qt.h>

#include <QFont>
#include <QSize>
#include <QString>
#include <QObject>
#include <QPoint>


class QPainter;


namespace Seiscomp {
namespace Gui {
namespace Map {


class Canvas;
class Layer;


class SC_GUI_API Legend : public QObject {
	Q_OBJECT

	public:
		Legend(QObject *parent = nullptr);
		Legend(const QString&, QObject *parent = nullptr);
		virtual ~Legend() {}

		Legend &operator =(const Legend &other);


	public:
		virtual void bringToFront();

		virtual Legend* clone() const { return nullptr; }

		virtual void draw(const QRect&, QPainter&) = 0;

		Qt::Alignment alignment() const { return _alignment; }
		void setArea(Qt::Alignment alignment) { _alignment = alignment; }

		void setEnabled(bool);
		bool isEnabled() const;

		const QFont& font() const { return _font; }
		void setFont(const QFont &font) { _font = font; }

		Layer *layer() const { return _layer; }
		void setLayer(Layer* layer) { _layer = layer; }

		int margin() const { return _margin; }
		void setMargin(int margin) { _margin = margin; }

		int spacing() const { return _spacing; }
		void setSpacing(int spacing) { _spacing = spacing; }

		const QString& title() const { return _title; }
		void setTitle(const QString &title) { _title = title; }

		const QFont& titleFont() const { return _titleFont; }
		void setTitleFont(const QFont &font) { _titleFont = font; }

		bool isVisible() const { return _visible; }

		const QSize &size() const { return _size; }

		virtual void contextResizeEvent(const QSize &size) {}


	signals:
		void bringToFrontRequested(Seiscomp::Gui::Map::Legend*);
		void enabled(Seiscomp::Gui::Map::Legend*, bool);
		void visibilityChanged(bool);


	private:
		void setVisible(bool visible) {
			_visible = visible;
			emit visibilityChanged(visible);
		}


	protected:
		int            _margin{9};
		int            _spacing{4};
		QFont          _font;
		QFont          _titleFont;
		QSize          _size;
		Layer         *_layer{nullptr};
		QString        _title;
		Qt::Alignment  _alignment{Qt::AlignLeft | Qt::AlignTop};
		bool           _enabled{true};
		QPoint         _pos;
		bool           _visible{true};


	friend class Canvas;
};


} // namespace Map
} // namespce Gui
} // namespace Seiscomp


#endif
