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


#ifndef SEISCOMP_GUI_ALG_MAPTREE_H
#define SEISCOMP_GUI_ALG_MAPTREE_H


#include <string>
#include <QString>
#include <QImage>
#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {
namespace Alg {


class MapTree;


DEFINE_SMARTPOINTER(MapTreeNode);
class SC_GUI_API MapTreeNode : public Seiscomp::Core::BaseObject {
	public:
		MapTreeNode();
		~MapTreeNode();

		void setChildren(int index, MapTreeNode*);
		void resetChildren(int index);

		//! Loads the next child. This function does not load the complete
		//! subtree.
		void loadChildren(MapTree*, int index);

		MapTreeNode* children(int index) const { return _child[index].node.get(); }
		bool initialized(int index) const { return _child[index].initialized; }

		int childrenCount() const;
		int depth() const;

		int level() const { return _level; }
		int row() const { return _row; }
		int column() const { return _column; }

		MapTreeNode *parent() const { return _parent; }


	protected:
		static MapTreeNode* Create(MapTree* root, MapTreeNode *parent,
		                           int level, int column, int row,
		                           int subLevels);

	protected:
		struct ChildItem {
			ChildItem() : initialized(false) {}
			MapTreeNodePtr node;
			bool           initialized;
		};

		ChildItem    _child[4];
		MapTreeNode *_parent;

		int _level;
		int _row;
		int _column;
};


DEFINE_SMARTPOINTER(MapTree);
class SC_GUI_API MapTree : public MapTreeNode {
	protected:
		MapTree();

	public:
		virtual ~MapTree() {}

	public:
		const QSize &tileSize() const { return _tilesize; }
		const QSize &mapSize() const { return _mapsize; }


	protected:
		//! Creates and validates an fileID
		virtual bool validate(int level, int column, int row) const = 0;

	protected:
		QSize       _tilesize;
		QSize       _mapsize;

	friend class MapTreeNode;
};


}
}
}


#endif
