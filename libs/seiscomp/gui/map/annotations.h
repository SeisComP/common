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


#ifndef SEISCOMP_GUI_MAP_ANNOTATIONS_H
#define SEISCOMP_GUI_MAP_ANNOTATIONS_H


#include <QColor>
#include <QFont>
#include <QPen>
#include <QPoint>
#include <QString>
#include <QVariant>


namespace Seiscomp {
namespace Gui {
namespace Map {


class Annotations;
class AnnotationItem;


struct AnnotationStyle : public QObject {
	AnnotationStyle(QObject *parent = nullptr);
	virtual ~AnnotationStyle() {}

	virtual void draw(QPainter &painter, const AnnotationItem &item);

	virtual QRect itemRect(QPainter &painter, const AnnotationItem &item,
	                       const QPoint &pos) const;

	struct Palette {
		QPen   textPen;
		QPen   borderPen;
		QBrush brush;
	};

	// One for normal (0) and one for highlighted (1)
	Palette palette[2];
	QFont   font;
};


/**
 * @brief The AnnotationItem struct holds properties like text, text color etc.
 *        which might be used when rendering an item on a map.
*/
class AnnotationItem {
	private:
		AnnotationItem(Annotations *parent) : pool(parent) {}
		AnnotationItem(Annotations *parent, const QString &text)
		: pool(parent), text(text) {}

	public:
		~AnnotationItem();

	public:
		/**
		 * @brief Updates the label rect of the item based on the given position
		 *        and style
		 * @param pos The position of the label rect
		 */
		void updateLabelRect(QPainter &painter, const QPoint &pos) {
			if ( style ) {
				labelRect = style->itemRect(painter, *this, pos);
			}
			else {
				labelRect = QRect();
			}
		}

	public:
		Annotations     *pool{nullptr};
		QString          text;
		AnnotationStyle *style{nullptr};
		bool             visible{true};
		bool             highlighted{false};
		QRect            labelRect;
		QVariant         data;

	friend class Annotations;
};


class Annotations : public QObject {
	public:
		typedef QList<AnnotationItem*> Items;
		typedef Items::const_iterator const_iterator;

	public:
		Annotations(QObject *parent = nullptr);
		~Annotations();

		const_iterator begin() const {
			return _items.begin();
		}

		const_iterator end() const {
			return _items.end();
		}

		/**
		 * @brief Clears the annotation pool
		 * This call calls delete for each AnnotationItem.
		 */
		void clear();

		int size() { return _items.size(); }

		/**
		 * @brief Adds a new object to the annotation pool
		 * @param text The annotation text
		 * @return Link to an annotation item which is owned by this instance
		 */
		AnnotationItem *add(const QString &text);

		/**
		 * @brief Removes annotation item from annotation pool and deletes it
		 * @param item The annotation item to remove
		 * @return true if removed, false otherwise
		 */
		bool remove(const AnnotationItem *item);

		/**
		 * @brief Removes annotation item from annotation pool but does not delete
		 *        it. The pool member in the item will be set to nullptr.
		 * @param item The annotation item to be taken
		 * @return true if taken, false otherwise
		 */
		bool take(const AnnotationItem *item);

		/**
		 * @brief Sets the default annotation style used while
		 * creating annotation items
		 * @param style The annotation style to set
		 */
		void setDefaultStyle(AnnotationStyle *style) {
			_style = style;
		}


	private:
		Items            _items;
		AnnotationStyle  _defaultStyle;
		AnnotationStyle *_style{nullptr};
};


} // ns Map
} // ns Gui
} // ns Seiscomp


#endif
