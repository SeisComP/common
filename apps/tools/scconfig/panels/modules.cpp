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


#include "modules.h"
#include "../fancyview.h"
#include "../icon.h"
#include "../searchwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QAction>

#include <seiscomp/gui/core/compat.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::System;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


/*
void dumpTree(QModelIndex start, int indent = 0) {
	int rows = start.model()->rowCount(start);
	for ( int i = 0; i < rows; ++i ) {
		auto idx = start.model()->index(i, 0, start);
		auto type = idx.data(ConfigurationTreeItemModel::Type).toInt();
		if ( type == ConfigurationTreeItemModel::TypeStruct ) {
			continue;
		}

		for ( int i = 0; i < indent; ++i ) {
			std::cerr << " ";
		}
		std::cerr << "+ " << idx.data(Qt::DisplayRole).toString().toStdString() << std::endl;
		dumpTree(idx, indent + 2);
	}
}
*/


QTreeWidgetItem *createPath(QTreeWidget *tree, const QString &path, bool expand) {
	QTreeWidgetItem *parent = nullptr;
	auto dirs = path.split('/', QT_SKIP_EMPTY_PARTS);

	foreach ( const QString &dir, dirs ) {
		QTreeWidgetItem *node = nullptr;
		if ( !parent ) {
			for ( int i = 0; i < tree->topLevelItemCount(); ++i ) {
				if ( tree->topLevelItem(i)->text(0) == dir ) {
					node = tree->topLevelItem(i);
					break;
				}
			}

			if ( !node ) {
				node = new QTreeWidgetItem(tree, QStringList() << dir, 0);
			}
		}
		else {
			for ( int i = 0; i < parent->childCount(); ++i ) {
				if ( parent->child(i)->text(0) == dir ) {
					node = parent->child(i);
					break;
				}
			}

			if ( !node ) {
				node = new QTreeWidgetItem(parent, QStringList() << dir, 0);
			}
		}

		if ( expand ) {
			tree->expandItem(node);
		}

		QFont boldFont = tree->font();
		boldFont.setBold(true);
		node->setData(0, Qt::FontRole, boldFont);
		node->setData(0, Qt::DecorationRole, ::icon("folder"));
		node->setFlags(node->flags() & ~Qt::ItemIsSelectable);
		node->setToolTip(0, "");
		parent = node;
	}

	return parent;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ModulesPanel::ModulesPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	_name = "Modules";
	_icon = ::icon("menu_scconfig_modules");
	setDescription("Configuration of module parameters");
	setHeadline("Configuration");

	QWidget *configurationPanel = this;
	configurationPanel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));
	//configurationPanel->setAutoFillBackground(true);
	QHBoxLayout *configurationLayout = new QHBoxLayout;
	configurationLayout->setContentsMargins(0, 0, 0, 0);
	configurationLayout->setSpacing(1);
	configurationPanel->setLayout(configurationLayout);

	QSplitter *configPanelSplitter = new QSplitter;
	configPanelSplitter->setHandleWidth(1);
	configPanelSplitter->setChildrenCollapsible(false);
	configurationLayout->addWidget(configPanelSplitter);

	_moduleTree = new QTreeWidget;
	_moduleTree->setFrameShape(QFrame::NoFrame);
	_moduleTree->setAutoFillBackground(true);
	configPanelSplitter->addWidget(_moduleTree);

	connect(_moduleTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	        this, SLOT(moduleSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	/*
	connect(_moduleTree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
	        this, SLOT(moduleSelected(QTreeWidgetItem*,int)));
	*/

	/*
	QTreeView *treeView = new QTreeView;
	treeView->setFrameShape(QFrame::NoFrame);
	treeView->setAutoFillBackground(true);
	treeView->setAlternatingRowColors(true);

	treeView->setItemDelegate(new ConfigurationTreeItemItemDelegate(this));
	treeView->setSortingEnabled(true);
	treeView->sortByColumn(0, Qt::AscendingOrder);

	_moduleView = treeView;
	*/
	//FancyWidget *fancyView = new FancyWidget;
	_moduleView = new FancyView;

	QWidget *settingsPanel = new QWidget;
	QVBoxLayout *settingsLayout = new QVBoxLayout;
	settingsLayout->setSpacing(1);
	settingsLayout->setContentsMargins(0, 0, 0, 0);
	settingsPanel->setLayout(settingsLayout);

	QSizePolicy sp = settingsPanel->sizePolicy();
	sp.setHorizontalStretch(1);
	settingsPanel->setSizePolicy(sp);

	QVBoxLayout *moduleViewLayout = new QVBoxLayout;

	_searchWidget = new SearchWidget(_moduleView);
	_searchWidget->setAutoFillBackground(true);
	connect(_searchWidget, &SearchWidget::closeRequested, this, &ModulesPanel::closeSearch);

	QAction *activateSearch = new QAction(this);
	activateSearch->setShortcut(QKeySequence("Ctrl+f"));
	addAction(activateSearch);
	connect(activateSearch, &QAction::triggered, this, &ModulesPanel::openSearch);

	connect(static_cast<FancyView*>(_moduleView), &FancyView::searchRequested, this, &ModulesPanel::openSearch);

	QAction *closeSearch = new QAction(this);
	closeSearch->setShortcut(QKeySequence("Esc"));
	_searchWidget->addAction(closeSearch);
	connect(closeSearch, &QAction::triggered, this, &ModulesPanel::closeSearch);

	_searchWidget->hide();
	moduleViewLayout->addWidget(_searchWidget);
	moduleViewLayout->addWidget(_moduleView);

	settingsLayout->addLayout(moduleViewLayout);
	configPanelSplitter->addWidget(settingsPanel);

	_modified = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::setModel(ConfigurationTreeItemModel *model) {
	if ( model == _model ) {
		return;
	}

	if ( _model ) {
		_model->disconnect(this);
	}

	ConfiguratorPanel::setModel(model);
	applyModel();

	connect(_model, &QAbstractItemModel::modelReset, this, &ModulesPanel::applyModel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::applyModel() {
	((FancyView*)_moduleView)->setConfigStage(_model->configStage());
	_moduleView->setModel(_model);

	while ( _moduleTree->topLevelItemCount() > 0 ) {
		delete _moduleTree->takeTopLevelItem(0);
	}

	_moduleTree->setColumnCount(1);
	_moduleTree->header()->hide();

	QFont boldFont = _moduleTree->font();
	boldFont.setBold(true);
	QFont italicFont = _moduleTree->font();
	italicFont.setItalic(true);

	System::Model *base = _model->model();

	QTreeWidgetItem *firstModule = nullptr;
	QTreeWidgetItem *emptyItem = nullptr;

	for ( size_t i = 0; i < base->modules.size(); ++i ) {
		if ( !base->modules[i]->hasConfiguration() ) {
			continue;
		}
		if ( !base->modules[i]->definition->category.empty() ) {
			continue;
		}

		if ( !emptyItem ) {
			emptyItem = new QTreeWidgetItem(_moduleTree, QStringList() << "<empty>", 0);
			emptyItem->setToolTip(0, "");
			emptyItem->setData(0, Qt::FontRole, boldFont);
			emptyItem->setData(0, Qt::DecorationRole, ::icon("folder"));
			emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
		}

		QTreeWidgetItem *mitem = new QTreeWidgetItem(emptyItem, QStringList() << base->modules[i]->definition->name.c_str(), 1);
		if ( base->modules[i]->definition->aliasedModule ) {
			mitem->setData(0, Qt::DecorationRole, ::icon("file_link"));
			mitem->setData(0, Qt::FontRole, italicFont);
			mitem->setToolTip(0, tr("This is an alias for %1").arg(base->modules[i]->definition->aliasedModule->name.c_str()));
		}
		else {
			mitem->setData(0, Qt::DecorationRole, ::icon("file"));
			mitem->setToolTip(0, "");
		}

		if ( !firstModule ) {
			firstModule = mitem;
		}
	}

	if ( emptyItem ) {
		_moduleTree->expandItem(emptyItem);
	}

	for ( auto it = base->categories.begin(); it != base->categories.end(); ++it ) {
		QTreeWidgetItem *citem = createPath(_moduleTree, it->c_str(), true);

		for ( size_t i = 0; i < base->modules.size(); ++i ) {
			if ( !base->modules[i]->hasConfiguration() ) {
				continue;
			}
			if ( base->modules[i]->definition->category != *it ) {
				continue;
			}
			QTreeWidgetItem *mitem = new QTreeWidgetItem(citem, QStringList() << base->modules[i]->definition->name.c_str(), 1);
			if ( base->modules[i]->definition->aliasedModule ) {
				mitem->setData(0, Qt::DecorationRole, ::icon("file_link"));
				mitem->setData(0, Qt::FontRole, italicFont);
				mitem->setToolTip(0, tr("This is an alias for %1").arg(base->modules[i]->definition->aliasedModule->name.c_str()));
			}
			else {
				mitem->setData(0, Qt::DecorationRole, ::icon("file"));
				mitem->setToolTip(0, "");
			}

			if ( !firstModule ) {
				firstModule = mitem;
			}
		}
	}

	if ( firstModule ) {
		_moduleTree->setCurrentItem(firstModule);
		moduleSelected(firstModule, 0);
	}

	_modified = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::moduleSelectionChanged(QTreeWidgetItem *curr,QTreeWidgetItem*) {
	moduleSelected(curr, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::moduleSelected(QTreeWidgetItem *item, int) {
	if ( item->type() != 1 ) {
		return;
	}

	int rows = _model->rowCount();
	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = _model->index(i,0);
		if ( idx.data(ConfigurationTreeItemModel::Type).toInt() !=
		     ConfigurationTreeItemModel::TypeModule ) {
			continue;
		}

		if ( idx.data().toString() == item->text(0) ) {
			moduleChanged(idx);
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::moduleChanged(const QModelIndex &index) {
	if ( index.data(ConfigurationTreeItemModel::Type).toInt() !=
	     ConfigurationTreeItemModel::TypeModule ) {
		return;
	}
	Module *mod = reinterpret_cast<Module*>(index.data(ConfigurationTreeItemModel::Link).value<void*>());

	//static_cast<FancyWidget*>(_moduleView)->setModel(mod);
	_moduleView->setRootIndex(index);

	setHeadline("Configuration", index.data(Qt::DisplayRole).toString());

	if ( mod ) {
		setDescription(mod->definition->description.c_str());
	}
	else {
		setDescription("...");
	}

	/*
	if ( _listWidget->currentItem() == _configItem ) {
		_headline->setText(_configItem->data(Qt::UserRole+1).toString());
		_description->setText(firstLine(_configItem->data(Qt::UserRole+2).toString()));
		_description->setToolTip(multiline(_configItem->data(Qt::UserRole+2).toString(), 80));
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::openSearch() {
	_searchWidget->reset();
	_searchWidget->show();
	_searchWidget->setFocus(Qt::ActiveWindowFocusReason);
	static_cast<FancyView*>(_moduleView)->showSearchButton(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ModulesPanel::closeSearch() {
	_moduleView->setCurrentIndex(_moduleView->rootIndex());
	_searchWidget->reset();
	_searchWidget->hide();
	static_cast<FancyView*>(_moduleView)->showSearchButton(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
