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


#include "bindings.h"
#include "../fancyview.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDragEnterEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>

#include <seiscomp/gui/core/compat.h>


using namespace std;
using namespace Seiscomp::System;


enum StationRoles {
	Type = Qt::UserRole,
	Link = Qt::UserRole + 1,
	Net  = Qt::UserRole + 2,
	Sta  = Qt::UserRole + 3
};

enum Type {
	TypeUndefined = 0,
	TypeRoot,
	TypeNetwork,
	TypeStation,
	TypeModule,
	TypeProfile
};


template <typename T>
T *getLink(const QModelIndex &idx) {
	return static_cast<T*>(idx.data(Link).value<void*>());
}

class NameValidator : public QValidator {
	public:
		NameValidator(QWidget *parent)
		: QValidator(parent) {}

		State validate(QString &txt, int &/*unused*/) const override {
			txt = txt.toUpper();
			return Acceptable;
		}
};


class ProfileValidator : public QValidator {
	public:
		ProfileValidator(QWidget *parent)
		: QValidator(parent) {}

		State validate(QString &txt, int &pos) const override {
			auto cnt = txt.size();
			for ( auto i = 0; i < cnt; ++i ) {
				if ( txt[i].isDigit() ) {
					continue;
				}

				if ( txt[i].isLetterOrNumber() ) {
					continue;
				}

				if ( txt[i] == '_' ) {
					continue;
				}

				if ( txt[i] == '-' ) {
					continue;
				}

				pos = i;
				return Invalid;
			}

			return Acceptable;
		}
};


class NewNameDialog : public QDialog {
	public:
		NewNameDialog(const QModelIndex &root, bool forceUpper, QWidget *parent = nullptr)
			: QDialog(parent), _root(root) {
			auto *layout = new QVBoxLayout;
			setLayout(layout);

			_hint = new QLabel();
			QFont f = _hint->font();
			f.setItalic(true);
			_hint->setFont(f);
			layout->addWidget(_hint);

			auto *hlayout = new QHBoxLayout;
			auto *label = new QLabel("Name:");
			hlayout->addWidget(label);
			_name = new QLineEdit;
			if ( forceUpper ) {
				_name->setValidator(new NameValidator(this));
			}
			hlayout->addWidget(_name);
			layout->addLayout(hlayout);

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			auto *ok = new QPushButton("Ok");
			hlayout->addWidget(ok);
			auto *cancel = new QPushButton("Cancel");
			hlayout->addWidget(cancel);

			layout->addLayout(hlayout);

			connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}

		[[nodiscard]]
		QString name() const {
			return _name->text();
		}

		void setValidator(QValidator *validator) {
			_name->setValidator(validator);
		}

		void setHint(const QString &hint) {
			_hint->setText(hint);
		}

		void accept() override {
			int rows = _root.model()->rowCount(_root);

			if ( _name->text().isEmpty() ) {
				QMessageBox::critical(nullptr, "Empty name",
				                      "Empty names are not allowed. ");
				return;
			}

			for ( int i = 0; i < rows; ++i ) {
				if ( _root.model()->index(i, 0, _root).data().toString() == _name->text() ) {
					QMessageBox::critical(nullptr, "Duplicate name",
					                      "The name exists already and duplicate "
					                      "names are not allowed.");
					return;
				}
			}

			QDialog::accept();
		}

	private:
		QLabel      *_hint;
		QLineEdit   *_name;
		QModelIndex  _root;
};


class StationTreeView : public QTreeView {
	public:
		StationTreeView(BindingsPanel *panel) : _panel(panel) {
			setAcceptDrops(true);
		}

		[[nodiscard]]
		Qt::DropActions supportedDropActions() const {
			return Qt::LinkAction;
		}

		void dragEnterEvent(QDragEnterEvent *event) override {
			bool accepted = false;

			if ( event->mimeData() && event->mimeData()->hasText() ) {
				accepted = true;

				// Validate text content
				auto lines = event->mimeData()->text().split("\n", QT_SKIP_EMPTY_PARTS);
				foreach ( const QString &l, lines ) {
					// Do not accept unknown text
					if ( !l.startsWith("PROFILE ") ) {
						accepted = false;
						break;
					}
				}

				if ( accepted ) {
					_followSelection = selectionModel()->selectedRows().count() <= 1;
				}
			}

			event->setAccepted(accepted);
		}

		void dragMoveEvent(QDragMoveEvent *event) override {
			if ( rootIndex().data(Type).toInt() == TypeStation ) {
				event->accept();
				return;
			}

			QModelIndex idx = indexAt(QT_EVENT_POS(event));

			if ( idx.isValid() ) {
				if ( _followSelection ) {
					selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
					event->setAccepted(true);
				}
				else {
					event->setAccepted(selectionModel()->isSelected(idx));
				}
			}
			else {
				event->setAccepted(false);
			}
		}

		void dropEvent(QDropEvent *event) override {
			QStringList lines = event->mimeData()->text().split("\n", QT_SKIP_EMPTY_PARTS);
			foreach ( const QString &l, lines ) {
				if ( !l.startsWith("PROFILE ") ) {
					continue;
				}

				QString module;
				QString profile;
				QStringList toks = l.mid(8).split('/');
				if ( toks.size() != 2 ) {
					continue;
				}

				module = toks[0];
				profile = toks[1];

				foreach ( const QModelIndex &i, selectionModel()->selectedIndexes() ) {
					_panel->assignProfile(i, module, profile);
				}
			}
		}

	private:
		BindingsPanel  *_panel;
		bool            _followSelection;
};


class StationsFolderView : public QListView {
	public:
		StationsFolderView(BindingsPanel *panel) : _panel(panel) {
			setAcceptDrops(true);
		}

		[[nodiscard]]
		Qt::DropActions supportedDropActions() const {
			return Qt::LinkAction;
		}

		void dragEnterEvent(QDragEnterEvent *event) override {
			/*
			event->setAccepted(event->mimeData() &&
			                   event->mimeData()->hasText() &&
			                   (selectionModel()->hasSelection() ||
			                    rootIndex().data(Type).toInt() == TypeStation));
			*/
			bool accepted = false;

			if ( event->mimeData() && event->mimeData()->hasText() ) {
				accepted = true;

				// Validate text content
				auto lines = event->mimeData()->text().split("\n", QT_SKIP_EMPTY_PARTS);
				foreach ( const QString &l, lines ) {
					// Do not accept unknown text
					if ( !l.startsWith("PROFILE ") ) {
						accepted = false;
						break;
					}
				}

				if ( accepted ) {
					_selectedItems = selectionModel()->selectedIndexes();
				}
			}

			event->setAccepted(accepted);
		}

		void dragLeaveEvent(QDragLeaveEvent *event) override {
			_selectedItems.clear();
		}

		void dragMoveEvent(QDragMoveEvent *event) override {
			if ( rootIndex().data(Type).toInt() == TypeStation ) {
				event->accept();
				return;
			}

			QModelIndex idx = indexAt(QT_EVENT_POS(event));

			if ( idx.isValid() ) {
				selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
			}
			else {
				selectionModel()->clear();
				foreach ( const QModelIndex &i, _selectedItems ) {
					selectionModel()->select(i, QItemSelectionModel::Select);
				}
			}

			event->setAccepted(selectionModel()->hasSelection());
		}

		void dropEvent(QDropEvent * event) override {
			auto lines = event->mimeData()->text().split("\n", QT_SKIP_EMPTY_PARTS);
			foreach ( const QString &l, lines ) {
				if ( !l.startsWith("PROFILE ") ) {
					continue;
				}

				QString module;
				QString profile;
				QStringList toks = l.mid(8).split('/');
				if ( toks.size() != 2 ) {
					continue;
				}

				module = toks[0];
				profile = toks[1];

				if ( rootIndex().data(Type).toInt() == TypeStation ) {
					_panel->assignProfile(rootIndex(), module, profile);
				}
				else {
					foreach ( const QModelIndex &i, selectionModel()->selectedIndexes() ) {
						_panel->assignProfile(i, module, profile);
					}
				}
			}

			_selectedItems.clear();
		}


	private:
		BindingsPanel  *_panel;
		QModelIndexList _selectedItems;
};


class ProfilesModel : public QStandardItemModel {
	public:
		ProfilesModel(QObject *parent = nullptr) : QStandardItemModel(parent) {}

		[[nodiscard]]
		QMimeData *mimeData(const QModelIndexList &indexes) const override {
			QString text;
			foreach ( const QModelIndex &i, indexes ) {
				if ( i.data(Type).toInt() != TypeProfile ) {
					continue;
				}
				auto *b = getLink<ModuleBinding>(i);
				auto *mod = static_cast<Module*>(b->parent);

				if ( !text.isEmpty() ) {
					text += "\n";
				}
				text += QString("PROFILE %1/%2")
				        .arg(mod->definition->name.c_str(), b->name.c_str());
			}

			if ( text.isEmpty() ) {
				return nullptr;
			}

			auto *data = new QMimeData;
			data->setText(text);
			return data;
		}

		Qt::DropActions supportedDragActions() {
			return Qt::LinkAction;
		}
};


BindingsViewDelegate::BindingsViewDelegate(QObject *parent) : QItemDelegate(parent) {}

QWidget *BindingsViewDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
	if ( index.column() != 1 || index.data(Type).toInt() != TypeProfile ) {
		return nullptr;
	}

	auto *binding = getLink<ModuleBinding>(index.sibling(index.row(),0));
	if ( !binding ) {
		return nullptr;
	}

	auto *mod = static_cast<Module*>(binding->parent);
	if ( !mod ) {
		return nullptr;
	}

	auto *editor = new QComboBox(parent);
	editor->addItem("");

	Module::Profiles::iterator it;
	for ( it = mod->profiles.begin(); it != mod->profiles.end(); ++it ) {
		editor->addItem((*it)->name.c_str());
	}

	connect(editor, SIGNAL(currentIndexChanged(QString)),
	        this, SLOT(profileChanged(QString)));

	return editor;
}

void BindingsViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	QString value = index.model()->data(index, Qt::EditRole).toString();
	auto *cBox = static_cast<QComboBox*>(editor);
	cBox->setCurrentIndex(cBox->findText(value));
}

void BindingsViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const {
	auto *cBox = static_cast<QComboBox*>(editor);
	QString value = cBox->currentText();

	QModelIndex sibling = index.sibling(index.row(),0);
	auto *binding = getLink<ModuleBinding>(sibling);
	auto *mod = static_cast<Module*>(binding->parent);

	StationID id(sibling.data(Net).toString().toStdString(),
	             sibling.data(Sta).toString().toStdString());

	// Skip, if we changed nothing
	if ( binding && binding->name == value.toStdString() ) {
		return;
	}

	binding = mod->bind(id, value.toStdString());
	if ( !binding ) {
		setEditorData(cBox, index);
		QMessageBox::critical(nullptr, "Change profile",
		                            "Changing the profile failed.");
		return;
	}

	model->setData(sibling, QVariant::fromValue((void*)binding), Link);
	model->setData(index, value, Qt::EditRole);
}

QSize BindingsViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index ) const {
	QSize hint = QItemDelegate::sizeHint(option, index);

	if ( index.column() != 1 || index.data(Type).toInt() != TypeProfile ) {
		return hint;
	}

	return {hint.width(), hint.height()*150/100};
}

void BindingsViewDelegate::updateEditorGeometry(QWidget *editor,
                                                const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const {
	editor->setGeometry(option.rect);
	//editor->move(option.rect.topLeft());
}


void BindingsViewDelegate::profileChanged(const QString &text) {
	emit commitData(static_cast<QWidget*>(sender()));
}


BindingView::BindingView(QWidget *parent) : QWidget(parent), _model(nullptr) {
	_bindingModel = new ConfigurationTreeItemModel(this);
	_view = new FancyView(this);
	_view->setAutoFillBackground(true);

	_header = new QWidget;
	QPalette pal = _header->palette();
	pal.setColor(QPalette::Window, Qt::white);
	pal.setColor(QPalette::WindowText, Qt::gray);
	_header->setPalette(pal);

	_searchWidget = new QWidget;
	_searchWidget->setAutoFillBackground(true);
	auto *searchLayout = new QHBoxLayout;
	_searchWidget->setLayout(searchLayout);
	auto *labelSearch = new QLabel;
	labelSearch->setText("Search parameter:");
	labelSearch->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred));
	auto *search = new QLineEdit;
	connect(search, SIGNAL(textEdited(QString)), this, SLOT(search(QString)));
	connect(search, SIGNAL(returnPressed()), this, SLOT(search()));
	auto *searchClose = new QPushButton;
	searchClose->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
	searchClose->setFixedSize(18,18);

	connect(searchClose, SIGNAL(clicked()), this, SLOT(closeSearch()));

	searchLayout->setContentsMargins(8, 8, 8, 8);
	searchLayout->addWidget(labelSearch);
	searchLayout->addWidget(search);
	searchLayout->addWidget(searchClose);

	auto *activateSearch = new QAction(this);
	activateSearch->setShortcut(QKeySequence("Ctrl+f"));
	addAction(activateSearch);
	connect(activateSearch, SIGNAL(triggered()), _searchWidget, SLOT(show()));
	connect(activateSearch, SIGNAL(triggered()), search, SLOT(setFocus()));

	auto *closeSearch = new QAction(_searchWidget);
	closeSearch->setShortcut(QKeySequence("Esc"));
	closeSearch->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	_searchWidget->addAction(closeSearch);
	connect(closeSearch, SIGNAL(triggered()), this, SLOT(closeSearch()));

	_searchWidget->hide();

	QFont f = _header->font();
	f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	_header->setFont(f);

	_header->setAutoFillBackground(true);
	_header->hide();

	auto *hlayout = new QHBoxLayout;
	_header->setLayout(hlayout);

	_icon = new QLabel;
	hlayout->addWidget(_icon);
	hlayout->addStretch();

	_label = new QLabel;
	hlayout->addWidget(_label);

	auto *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(1);
	setLayout(l);
	l->addWidget(_header);
	l->addWidget(_searchWidget);
	l->addWidget(_view);
}


void BindingView::clear() {
	_bindingModel->clear();
	_view->setModel(nullptr);
	_label->setText("");
	_icon->setPixmap(QPixmap());
	_rootIndex = QModelIndex();
	_header->hide();
}


void BindingView::setModel(ConfigurationTreeItemModel *base,
                           QAbstractItemModel *model) {
	if ( model == _model ) {
		return;
	}

	if ( _model ) {
		_model->disconnect(this);
	}

	_bindingModel->disconnect();

	_model = model;

	if ( _model ) {
		connect(_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
		        this, SLOT(dataChanged(QModelIndex,QModelIndex)));
		connect(_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
		        this, SLOT(rowsRemoved(QModelIndex,int,int)));
	}

	if ( base ) {
		connect(_bindingModel, SIGNAL(modificationChanged(bool)),
		        base, SLOT(setModified(bool)));
	}
}


void BindingView::setRootIndex(const QModelIndex &index) {
	clear();

	//int type = index.data(Type).toInt();
	//if ( type != TypeModule ) return;

	_rootIndex = index;

	auto *b = getLink<ModuleBinding>(index);
	auto *mod = static_cast<Module*>(b->parent);
	_bindingModel->setModel(b);
	_view->setModel(_bindingModel);
	_view->setRootIndex(_bindingModel->index(0,0));

	auto icon = _rootIndex.data(Qt::DecorationRole).value<QIcon>();
	_icon->setPixmap(icon.pixmap(32,32));

	if ( b->name.empty() ) {
		_label->setText(QString("%1/%2.%3").arg(
		                    mod->definition->name.c_str(),
		                    index.data(Net).toString(),
		                    index.data(Sta).toString()));
	}
	else {
		_label->setText(QString("%1/%2")
		                .arg(mod->definition->name.c_str(), b->name.c_str()));
	}

	_header->show();
}


void BindingView::dataChanged(const QModelIndex &topLeft,
                              const QModelIndex &bottomRight) {
	if ( _rootIndex.parent() == topLeft.parent() &&
	     _rootIndex.row() == topLeft.row() &&
	     topLeft.data(Type).toInt() == TypeProfile ) {
		// Profile changed to my binding
		setRootIndex(topLeft.sibling(topLeft.row(), 0));
	}
}


void BindingView::rowsRemoved(const QModelIndex &parent, int start, int end) {
	if ( !_rootIndex.isValid() ) {
		return;
	}

	QModelIndex p = _rootIndex.parent();
	QModelIndex i = _rootIndex;

	while ( p.isValid() ) {
		if ( parent == p && start <= i.row() && i.row() <= end ) {
			clear();
			return;
		}

		i = p;
		p = p.parent();
	}
}


void BindingView::search(const QString &text) {
	QModelIndexList hits;

	QModelIndex idx = _view->rootIndex();
	auto *model = _view->model();

	if ( text.isEmpty() ) {
		_view->scrollTo(idx);
		_view->setCurrentIndex(idx);
		return;
	}

	int rows = model->rowCount(idx);

	for ( int i = 0; i < rows; ++i ) {
		hits = model->match(model->index(i, 0, idx), Qt::DisplayRole, text, 1,
		                    Qt::MatchStartsWith |
		                    Qt::MatchRecursive |
		                    Qt::MatchWrap);

		if ( hits.isEmpty() ) {
			continue;
		}

		_view->scrollTo(hits.first());
		_view->setCurrentIndex(hits.first());
		break;
	}
}


void BindingView::search() {
	search(static_cast<QLineEdit*>(sender())->text());
}


void BindingView::closeSearch() {
	_view->setCurrentIndex(_rootIndex);
	_searchWidget->hide();
}


BindingsPanel::BindingsPanel(QWidget *parent)
: ConfiguratorPanel(false, parent), _bindingsModel(nullptr), _profilesModel(nullptr) {
	_name = "Bindings";
	_icon = QIcon(":/res/icons/bindings.png");
	setHeadline("Bindings");
	setDescription("Configuration of module-station bindings and binding profiles.");

	_docIcon = QIcon(":/res/icons/document.png");
	_linkIcon = QIcon(":/res/icons/document_link.png");
	_docFolder = QIcon(":/res/icons/document_folder.png");

	auto *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);

	auto *splitter = new QSplitter;
	splitter->setHandleWidth(1);
	splitter->setChildrenCollapsible(false);
	l->addWidget(splitter);

	_stationsTreeView = new StationTreeView(this);
	_stationsTreeView->setFrameShape(QFrame::NoFrame);
	_stationsTreeView->setAutoFillBackground(true);
	static_cast<QTreeView*>(_stationsTreeView)->setRootIsDecorated(false);

	_stationsTreeView->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	_stationsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	auto *folderView = new QWidget(this);
	folderView->setAutoFillBackground(true);
	auto *fl = new QVBoxLayout;
	fl->setContentsMargins(0, 0, 0, 0);
	fl->setSpacing(0);
	folderView->setLayout(fl);

	_stationsFolderView = new StationsFolderView(this);
	_stationsFolderView->setFrameShape(QFrame::NoFrame);
	_stationsFolderView->setResizeMode(QListView::Adjust);
	_stationsFolderView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_stationsFolderView->setContextMenuPolicy(Qt::CustomContextMenu);

	switchToStationsIconView();

	connect(_stationsFolderView, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(folderViewContextMenu(QPoint)));

	auto *folderViewTools = new QToolBar;
	folderViewTools->setIconSize(QSize(16,16));
	folderViewTools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	_folderLevelUp = folderViewTools->addAction("Up");
	_folderLevelUp->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
	_folderLevelUp->setEnabled(false);
	connect(_folderLevelUp, SIGNAL(triggered(bool)), this, SLOT(folderLevelUp()));

	_deleteItem = folderViewTools->addAction("Delete");
	_deleteItem->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	connect(_deleteItem, SIGNAL(triggered(bool)), this, SLOT(deleteItem()));

	auto *a = folderViewTools->addAction("Icons");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToStationsIconView()));
	a = folderViewTools->addAction("List");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToStationsListView()));

	fl->addWidget(folderViewTools);
	fl->addWidget(_stationsFolderView);

	auto *container = new QWidget;
	auto *modulesFolderLayout = new QVBoxLayout;
	modulesFolderLayout->setContentsMargins(0, 0, 0, 0);
	modulesFolderLayout->setSpacing(0);
	container->setAutoFillBackground(true);
	container->setLayout(modulesFolderLayout);

	_bindingView = new BindingView;
	QSizePolicy sp = _bindingView->sizePolicy();
	sp.setHorizontalStretch(1);
	_bindingView->setSizePolicy(sp);

	folderViewTools = new QToolBar;
	folderViewTools->setIconSize(QSize(16,16));
	folderViewTools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	_addProfile = folderViewTools->addAction(tr("Add profile"));
	_addProfile->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
	_addProfile->setEnabled(false);
	connect(_addProfile, SIGNAL(triggered(bool)), this, SLOT(addProfile()));

	_deleteProfile = folderViewTools->addAction(tr("Delete"));
	_deleteProfile->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
	_deleteProfile->setEnabled(false);
	connect(_deleteProfile, SIGNAL(triggered(bool)), this, SLOT(deleteProfile()));

	a = folderViewTools->addAction("Icons");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToProfileIconView()));
	a = folderViewTools->addAction("List");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToProfileListView()));

	auto *splitter2 = new QSplitter(Qt::Vertical);
	splitter2->setHandleWidth(1);
	splitter2->addWidget(_stationsTreeView);
	splitter2->addWidget(folderView);

	auto *tree = new QTreeView;
	//tree->header()->hide();
	_modulesView = tree;
	_modulesView->setAutoFillBackground(true);
	_modulesView->setDragEnabled(true);
	_modulesView->setFrameShape(QFrame::NoFrame);
	_modulesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_modulesView->setContextMenuPolicy(Qt::CustomContextMenu);
	_modulesView->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(_modulesView, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(modulesViewContextMenu(QPoint)));

	_modulesFolderView = new QListView;
	_modulesFolderView->setResizeMode(QListView::Adjust);
	_modulesFolderView->setFrameShape(QFrame::NoFrame);
	_modulesFolderView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_modulesFolderView->setContextMenuPolicy(Qt::CustomContextMenu);
	_modulesFolderView->setEnabled(false);

	connect(_modulesFolderView, SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(modulesFolderViewContextMenu(QPoint)));

	switchToProfileIconView();

	modulesFolderLayout->addWidget(folderViewTools);
	modulesFolderLayout->addWidget(_modulesFolderView);

	auto *splitter3 = new QSplitter(Qt::Vertical);
	splitter3->setHandleWidth(1);
	splitter3->addWidget(_modulesView);
	splitter3->addWidget(container);

	splitter->addWidget(splitter2);
	splitter->addWidget(_bindingView);
	splitter->addWidget(splitter3);

	connect(_stationsTreeView, SIGNAL(clicked(QModelIndex)),
	        this, SLOT(bindingActivated(QModelIndex)));
	connect(_stationsTreeView, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(bindingDoubleClicked(QModelIndex)));

	connect(_stationsFolderView, SIGNAL(activated(QModelIndex)),
	        this, SLOT(changeFolder(QModelIndex)));
	connect(_stationsFolderView, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(bindingDoubleClicked(QModelIndex)));

	connect(_modulesView, SIGNAL(activated(QModelIndex)),
	        this, SLOT(profileActivated(QModelIndex)));

	connect(_modulesView, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(profileDoubleClicked(QModelIndex)));
	connect(_modulesFolderView, SIGNAL(doubleClicked(QModelIndex)),
	        this, SLOT(profileDoubleClicked(QModelIndex)));
}


void BindingsPanel::setModel(ConfigurationTreeItemModel *model) {
	ConfiguratorPanel::setModel(model);

	_bindingView->setModel(nullptr, nullptr);
	_stationsTreeView->setModel(nullptr);
	_stationsFolderView->setModel(nullptr);
	_modulesView->setModel(nullptr);
	_modulesFolderView->setModel(nullptr);

	delete _bindingsModel;
	delete _profilesModel;

	if ( !model ) {
		return;
	}

	// -------------------------------
	// Create bindings model
	// -------------------------------
	_bindingsModel = new QStandardItemModel(this);
	_bindingsModel->setColumnCount(2);
	_bindingsModel->setHeaderData(0, Qt::Horizontal, "Name");
	_bindingsModel->setHeaderData(1, Qt::Horizontal, "Profile");

	connect(_bindingsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	        this, SLOT(moduleBindingsChanged(QModelIndex,QModelIndex)));

	auto *root = _bindingsModel->invisibleRootItem();

	auto *base = model->model();

	using Bindings = QVector<ModuleBinding*>;
	using Stations = QMap<QString, Bindings>;
	using Networks = QMap<QString, Stations>;
	Networks networks;

	for ( const auto &[id, ptr] : base->stations ) {
		networks[id.networkCode.c_str()]
		        [id.stationCode.c_str()].clear();
	}

	for ( const auto &mod : base->modules ) {
		for ( const auto &[id, binding] : mod->bindings ) {
			// Build binding map
			networks[id.networkCode.c_str()]
			        [id.stationCode.c_str()].append(binding.get());
		}
	}

	Networks::iterator nit;
	Stations::iterator sit;
	Bindings::iterator bit;

	auto *rootItem = new QStandardItem("Networks");
	rootItem->setColumnCount(2);
	rootItem->setData(TypeRoot, Type);
	root->appendRow(rootItem);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();

	for ( nit = networks.begin(); nit != networks.end(); ++nit ) {
		const QString &networkCode = nit.key();
		auto *netItem = new QStandardItem(networkCode);
		netItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled/* | Qt::ItemIsDropEnabled*/);
		netItem->setData(TypeNetwork, Type);
		netItem->setData(_docFolder, Qt::DecorationRole);
		netItem->setColumnCount(2);
		rootItem->appendRow(netItem);

		for ( sit = nit.value().begin(); sit != nit.value().end(); ++sit ) {
			const QString &stationCode = sit.key();
			auto *staItem = new QStandardItem(stationCode);
			staItem->setData(TypeStation, Type);
			staItem->setData(_docFolder, Qt::DecorationRole);

			string rcFile = env->installDir();
			rcFile += "/var/lib/rc/station_";
			rcFile += networkCode.toStdString() + "_" + stationCode.toStdString();
			Seiscomp::Config::Config cfg;
			if ( cfg.readConfig(rcFile) ) {
				try {
					staItem->setData(cfg.getString("description").c_str(),
					                 Qt::ToolTipRole);
				}
				catch ( ... ) {}
			}

			staItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled/* | Qt::ItemIsDropEnabled*/);
			netItem->appendRow(staItem);

			for ( bit = sit.value().begin(); bit != sit.value().end(); ++bit ) {
				auto *binding = *bit;

				auto *mod = static_cast<Module*>(binding->parent);
				auto *bindItem = new QStandardItem(mod->definition->name.c_str());
				bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled/* | Qt::ItemIsDropEnabled*/);
				bindItem->setData(networkCode, Net);
				bindItem->setData(stationCode, Sta);
				bindItem->setData(TypeModule, Type);
				bindItem->setData(QVariant::fromValue((void*)binding), Link);

				if ( binding->name.empty() ) {
					bindItem->setData(_docIcon, Qt::DecorationRole);
				}
				else {
					bindItem->setData(_linkIcon, Qt::DecorationRole);
				}

				auto *profileItem = new QStandardItem(binding->name.c_str());
				profileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable/* | Qt::ItemIsDropEnabled*/);
				profileItem->setData(TypeProfile, Type);
				//profileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

				staItem->appendRow(QList<QStandardItem*>() << bindItem << profileItem);
			}
		}
	}

	_bindingView->setModel(model, _bindingsModel);

	_stationsTreeView->setModel(_bindingsModel);
	_stationsTreeView->setRootIndex(root->index());
	_stationsTreeView->setItemDelegate(new BindingsViewDelegate(_stationsTreeView));
	static_cast<QTreeView*>(_stationsTreeView)->expand(rootItem->index());

	_stationsFolderView->setModel(_bindingsModel);
	_stationsFolderView->setRootIndex(rootItem->index());


	// -------------------------------
	// Create profiles model
	// -------------------------------
	_profilesModel = new ProfilesModel(this);
	_profilesModel->setColumnCount(1);
	_profilesModel->setHeaderData(0, Qt::Horizontal, "Name");

	root = _profilesModel->invisibleRootItem();
	rootItem = new QStandardItem("/");
	rootItem->setData(TypeRoot, Type);
	root->appendRow(rootItem);

	for ( const auto &mod : base->modules ) {
		if ( !mod->supportsBindings() ) {
			continue;
		}

		auto *modItem = new QStandardItem(mod->definition->name.c_str());
		modItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		modItem->setData(TypeModule, Type);
		modItem->setData(QVariant::fromValue(static_cast<void*>(mod.get())), Link);
		modItem->setData(_docFolder, Qt::DecorationRole);
		rootItem->appendRow(modItem);

		for ( const auto &profile : mod->profiles ) {
			auto *profItem = new QStandardItem(profile->name.c_str());
			profItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
			profItem->setData(TypeProfile, Type);
			profItem->setData(QVariant::fromValue(static_cast<void*>(profile.get())), Link);
			profItem->setData(_docIcon, Qt::DecorationRole);
			modItem->appendRow(profItem);
		}
	}

	_modulesFolderView->setModel(nullptr);
	_modulesView->setModel(_profilesModel);
	_modulesView->setRootIndex(rootItem->index());
	connect(_modulesView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	        this, SLOT(moduleTreeCurrentChanged(QModelIndex,QModelIndex)));

	changeFolder(_stationsFolderView->rootIndex());
}


void BindingsPanel::moduleTreeCurrentChanged(const QModelIndex &curr, const QModelIndex &prev) {
	switch ( curr.data(Type).toInt() ) {
		case TypeModule:
			_addProfile->setEnabled(true);
			_deleteProfile->setEnabled(true);
			_modulesFolderView->setModel(_profilesModel);
			_modulesFolderView->setEnabled(true);
			if ( _modulesFolderView->rootIndex() != curr ) {
				_modulesFolderView->setRootIndex(curr);
				_modulesFolderView->selectionModel()->clear();
			}
			break;
		case TypeProfile:
			_addProfile->setEnabled(true);
			_deleteProfile->setEnabled(true);
			_modulesFolderView->setModel(_profilesModel);
			_modulesFolderView->setEnabled(true);
			if ( _modulesFolderView->rootIndex() != curr.parent() ) {
				_modulesFolderView->setRootIndex(curr.parent());
				_modulesFolderView->selectionModel()->clear();
			}
			break;
		default:
			_addProfile->setEnabled(false);
			_deleteProfile->setEnabled(false);
			_modulesFolderView->setEnabled(false);
			_modulesFolderView->setModel(nullptr);
	}
}


void BindingsPanel::moduleBindingsChanged(const QModelIndex &topLeft,
                                          const QModelIndex &bottomRight) {
	// Only react on profile changes
	int type = topLeft.data(Type).toInt();

	if ( type == TypeProfile ) {
		// Update icon
		if ( topLeft.data().toString().isEmpty() ) {
			_bindingsModel->setData(topLeft.sibling(topLeft.row(),0), _docIcon, Qt::DecorationRole);
		}
		else {
			_bindingsModel->setData(topLeft.sibling(topLeft.row(),0), _linkIcon, Qt::DecorationRole);
		}
		_model->setModified();
		return;
	}
}


void BindingsPanel::bindingActivated(const QModelIndex &idx) {
	changeFolder(idx);
}


void BindingsPanel::bindingDoubleClicked(const QModelIndex &idx) {
	int type = idx.data(Type).toInt();

	if ( type != TypeModule ) {
		return;
	}

	if ( !idx.sibling(idx.row(), 1).data().toString().isEmpty() ) {
		if ( QMessageBox::question(this, tr("Warning"),
		                           tr("The binding you are about to open is a profile and "
		                              "can potentially affect many stations. "
		                              "Do you want to continue?"),
		                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No ) {
			return;
		}
	}

	_bindingView->setModel(_model, _bindingsModel);
	_bindingView->setRootIndex(idx.sibling(idx.row(),0));
	_stationsTreeView->setCurrentIndex(idx.sibling(idx.row(),0));
	_stationsFolderView->setCurrentIndex(idx.sibling(idx.row(),0));

	if ( _stationsFolderView->rootIndex() != idx.parent() ) {
		_stationsFolderView->setRootIndex(idx.parent());
		_folderLevelUp->setEnabled(idx.parent().isValid());
	}
}


void BindingsPanel::profileActivated(const QModelIndex &idx) {
	/*
	int type = idx.data(Type).toInt();
	if ( type != TypeProfile ) return;

	_bindingView->setModel(_profilesModel);
	_bindingView->setRootIndex(idx);
	*/
}


void BindingsPanel::profileDoubleClicked(const QModelIndex &idx) {
	/*
	int type = idx.data(Type).toInt();
	if ( type != TypeProfile ) return;

	ModuleBinding *profile = static_cast<ModuleBinding*>(idx.data(Link).value<void*>());

	QItemSelectionModel *selectionModel = _stationsTreeView->selectionModel();
	if ( selectionModel ) {
		selectionModel->clear();
		selectBindings(selectionModel, profile);
	}
	*/

	int type = idx.data(Type).toInt();
	if ( type != TypeProfile ) {
		return;
	}

	_modulesView->setCurrentIndex(idx.sibling(idx.row(),0));
	_modulesFolderView->setCurrentIndex(idx.sibling(idx.row(),0));

	_bindingView->setModel(_model, _profilesModel);
	_bindingView->setRootIndex(idx);
}


void BindingsPanel::selectBindings(QItemSelectionModel *selectionModel,
                                   void *link, const QModelIndex &parent) {
	int rows = _bindingsModel->rowCount(parent);
	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = _bindingsModel->index(i, 0, parent);
		if ( idx.data(Type).toInt() == TypeModule ) {
			if ( link == idx.data(Link).value<void*>() ) {
				selectionModel->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			}
		}

		selectBindings(selectionModel, link, idx);
	}
}


void BindingsPanel::changeFolder(const QModelIndex &idx_) {
	if ( QApplication::keyboardModifiers() != Qt::NoModifier ) {
		return;
	}

	QModelIndex idx = idx_.sibling(idx_.row(), 0);

	int type = idx.data(Type).toInt();

	if ( type >= TypeModule ) {
		return;
	}

	_stationsTreeView->setCurrentIndex(idx_);
	_stationsFolderView->setRootIndex(idx);
	_stationsFolderView->selectionModel()->clear();

	_folderLevelUp->setEnabled(idx.parent().isValid());
}


void BindingsPanel::collectModuleBindings(ModuleBindingMap &map,
                                          const QModelIndex &idx) {
	int type = idx.data(Type).toInt();
	if ( type == TypeNetwork || type == TypeStation ) {
		auto *model = _stationsFolderView->model();
		int rowCount = model->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i ) {
			collectModuleBindings(map, model->index(i, 0, idx));
		}
		return;
	}

	if ( type == TypeModule ) {
		auto *b = getLink<ModuleBinding>(idx);
		auto *mod = (Module*)b->parent;
		map[QString(mod->definition->name.c_str())].insert(QString(b->name.c_str()));
		return;
	}
}


void BindingsPanel::clearModuleBindings(const QModelIndex &idx) {
	int type = idx.data(Type).toInt();
	auto *model = _stationsFolderView->model();
	if ( type == TypeRoot || type == TypeNetwork ) {
		int rowCount = model->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i ) {
			clearModuleBindings(model->index(i, 0, idx));
		}
		return;
	}

	if ( type == TypeStation ) {
		// Collect all childs (bindings)
		QList<QPersistentModelIndex> indexes;
		int rowCount = model->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i ) {
			indexes.append(model->index(i, 0, idx));
		}

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) {
				continue;
			}

			StationID id(qPrintable(i.data(Net).toString()),
			             qPrintable(i.data(Sta).toString()));

			auto *b = getLink<ModuleBinding>(i);
			auto *mod = (Module*)b->parent;

			if ( mod->removeStation(id) ) {
				_stationsFolderView->model()->removeRow(i.row(), idx);
			}
		}

		_model->setModified();

		return;
	}
}


void BindingsPanel::removeModuleBindings(const QModelIndex &idx,
                                         const QString &modName,
                                         const QString *name) {
	int type = idx.data(Type).toInt();
	auto *model = _stationsFolderView->model();
	if ( type == TypeRoot || type == TypeNetwork ) {
		int rowCount = model->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i ) {
			removeModuleBindings(model->index(i, 0, idx), modName, name);
		}
		return;
	}

	if ( type == TypeStation ) {
		// Collect all childs (bindings)
		QList<QPersistentModelIndex> indexes;
		auto *model = _stationsFolderView->model();
		int rowCount = model->rowCount(idx);
		for ( int i = 0; i < rowCount; ++i ) {
			indexes.append(model->index(i, 0, idx));
		}

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) {
				continue;
			}

			StationID id(qPrintable(i.data(Net).toString()),
			             qPrintable(i.data(Sta).toString()));

			auto *b = getLink<ModuleBinding>(i);
			auto *mod = static_cast<Module*>(b->parent);

			if ( modName != mod->definition->name.c_str() ||
			     (name && *name != b->name.c_str()) ) {
				continue;
			}

			if ( mod->removeStation(id) ) {
				_stationsFolderView->model()->removeRow(i.row(), idx);
			}
		}

		_model->setModified();

		return;
	}
}


void BindingsPanel::folderViewContextMenu(const QPoint &p) {
	QModelIndex idx = _stationsFolderView->rootIndex();
	QModelIndex hoveredIdx = _stationsFolderView->indexAt(p);
	QAction *a;

	if ( !idx.isValid() ) {
		return;
	}

	if ( hoveredIdx.isValid() ) {
		hoveredIdx = hoveredIdx.sibling(hoveredIdx.row(), 0);
		int type = hoveredIdx.data(Type).toInt();

		if ( type == TypeNetwork || type == TypeStation ) {
			QMenu menu;
			auto *clearAction = menu.addAction("Clear all module bindings");
			auto *deleteAction = menu.addAction("Delete");
			QMenu *removeBinding = nullptr;

			auto *sel = _stationsFolderView->selectionModel();

			ModuleBindingMap bindingMap;
			foreach ( const QModelIndex &idx, sel->selectedIndexes() ) {
				collectModuleBindings(bindingMap, idx);
			}

			if ( !bindingMap.empty() ) {
				removeBinding = menu.addMenu("Remove module binding");
				ModuleBindingMap::iterator it;
				for ( it = bindingMap.begin(); it != bindingMap.end(); ++it ) {
					auto *sub = removeBinding->addMenu(it.key());
					sub->setObjectName(it.key());
					foreach ( const QString &name, it.value() ) {
						QAction *action;
						if ( name.isEmpty() ) {
							action = sub->addAction("station");
						}
						else {
							action = sub->addAction(QString("profile_%1").arg(name));
						}

						action->setData(name);
					}

					if ( it.value().count() > 1 ) {
						sub->addSeparator();
						sub->addAction("Any");
					}
				}
			}

			a = menu.exec(_stationsFolderView->mapToGlobal(p));
			if ( a == clearAction ) {
				if ( sel->selectedIndexes().count() > 0 ) {
					if ( QMessageBox::question(nullptr, "Clear bindings",
					       QString("Do you really want to remove all module\n"
					               "bindings of %1 selected %2?")
					       .arg(sel->selectedIndexes().count())
					       .arg(type == TypeNetwork?"network(s)":"station(s)"),
					       QMessageBox::Yes | QMessageBox::No
					     ) != QMessageBox::Yes ) {
						return;
					}
				}

				foreach ( const QModelIndex &idx, sel->selectedIndexes() ) {
					clearModuleBindings(idx);
				}
			}
			else if ( a == deleteAction ) {
				deleteItem();
			}
			else if ( a && removeBinding && a->parent()
			       && (a->parent()->parent() == removeBinding) ) {
				QString bindingName = a->data().toString();
				QString moduleName = a->parent()->objectName();

				QString *bindingNameFilter = nullptr;
				if ( a->data().isValid() ) {
					bindingNameFilter = &bindingName;
				}

				foreach ( const QModelIndex &idx, sel->selectedIndexes() ) {
					removeModuleBindings(idx, moduleName, bindingNameFilter);
				}
			}
		}
		else if ( type == TypeModule ) {
			QItemSelectionModel *sel = _stationsFolderView->selectionModel();
			QMenu menu;
			QMenu *menuProfile = nullptr;
			Module *mod = nullptr;

			if ( sel->isSelected(hoveredIdx) &&
			     sel->selectedIndexes().count() == 1 ) {
				menuProfile = menu.addMenu("Change profile");
				auto *b = getLink<ModuleBinding>(hoveredIdx);
				mod = (Module*)b->parent;
				a = menuProfile->addAction("None");
				a->setEnabled(!b->name.empty());
				a->setData(QVariant::fromValue(nullptr));
				for ( const auto &profile : mod->profiles ) {
					a = menuProfile->addAction(profile->name.c_str());
					a->setData(QVariant::fromValue(static_cast<void*>(profile.get())));
					QFont f = a->font();
					f.setBold(true);
					a->setFont(f);
					if ( profile == b ) {
						a->setEnabled(false);
					}
				}
			}

			QAction *deleteAction = menu.addAction("Delete");

			a = menu.exec(_stationsFolderView->mapToGlobal(p));
			if ( !a ) {
				return;
			}

			if ( a == deleteAction ) {
				deleteItem();
			}
			else if ( a->parent() == menuProfile ) {
				StationID id(qPrintable(hoveredIdx.data(Net).toString()),
				             qPrintable(hoveredIdx.data(Sta).toString()));

				auto *b = reinterpret_cast<ModuleBinding*>(a->data().value<void*>());
				if ( b ) {
					if ( !mod->bind(id, b) ) {
						b = nullptr;
					}
				}
				else {
					b = mod->bind(id, "");
				}

				if ( !b ) {
					QMessageBox::critical(nullptr, "Change profile",
					                            "Changing the profile failed.");
					return;
				}

				QAbstractItemModel *m = _stationsFolderView->model();
				m->setData(hoveredIdx, QVariant::fromValue((void*)b), Link);
				m->setData(hoveredIdx.sibling(hoveredIdx.row(), 1), b->name.c_str(), Qt::EditRole);
			}
		}

		return;
	}

	int type = idx.data(Type).toInt();

	switch ( type ) {
		case TypeRoot:
		{
		QMenu menu;
		auto *addNetwork = menu.addAction("Add network");
		a = menu.exec(_stationsFolderView->mapToGlobal(p));
		if ( !a ) {
			return;
		}

		if ( a == addNetwork ) {
			NewNameDialog dlg(idx, true, this);
			dlg.setWindowTitle("New network name");
			if ( dlg.exec() != QDialog::Accepted ) {
				return;
			}

			auto *netItem = new QStandardItem(dlg.name());
			netItem->setData(TypeNetwork, Type);
			netItem->setData(_docFolder, Qt::DecorationRole);
			netItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			auto *rootItem = _bindingsModel->itemFromIndex(idx);
			rootItem->appendRow(netItem);
			_stationsFolderView->selectionModel()->setCurrentIndex(
				netItem->index(), QItemSelectionModel::ClearAndSelect
			);
		}
		}
		break;

		// Root is network, allow to modify stations
		case TypeNetwork:
		{
		QMenu menu;
		QAction *addStation = menu.addAction("Add station");
		a = menu.exec(_stationsFolderView->mapToGlobal(p));
		if ( !a ) {
			return;
		}

		if ( a == addStation ) {
			NewNameDialog dlg(idx, true, this);
			dlg.setWindowTitle("New station name");
			if ( dlg.exec() != QDialog::Accepted ) {
				return;
			}

			StationID id(qPrintable(idx.data().toString()),
			             qPrintable(dlg.name()));

			if ( !_model->model()->addStation(id) ) {
				QMessageBox::critical(this, "Add station",
				                      "Adding the station failed.");
				return;
			}

			auto *staItem = new QStandardItem(dlg.name());
			staItem->setData(TypeStation, Type);
			staItem->setData(_docFolder, Qt::DecorationRole);
			staItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QStandardItem *netItem = _bindingsModel->itemFromIndex(idx);
			netItem->appendRow(staItem);
			_stationsFolderView->selectionModel()->setCurrentIndex(
				staItem->index(), QItemSelectionModel::ClearAndSelect
			);
		}
		}
		break;

		// Root is station, allow to modify bindings
		case TypeStation:
		{
		QMenu menu;
		auto *bindingMenu = menu.addMenu("Add binding");

		StationID id(qPrintable(idx.parent().data().toString()),
		             qPrintable(idx.data().toString()));
		auto *model = _model->model();

		for ( const auto &mod : model->modules ) {
			if ( mod->supportsBindings() && mod->getBinding(id) == nullptr ) {
				a = bindingMenu->addAction(mod->definition->name.c_str());
				a->setData(QVariant::fromValue(static_cast<void*>(mod.get())));
			}
		}

		if ( bindingMenu->isEmpty() ) {
			bindingMenu->setEnabled(false);
		}

		a = menu.exec(_stationsFolderView->mapToGlobal(p));
		if ( a == nullptr ) {
			return;
		}

		if ( a->parent() == bindingMenu ) {
			auto *mod = reinterpret_cast<Module*>(a->data().value<void*>());
			ModuleBinding *binding = mod->bind(id, "");
			if ( binding == nullptr ) {
				QMessageBox::critical(nullptr, "Add binding",
				                      "Creation of binding failed.");
				return;
			}

			auto *bindItem = new QStandardItem(mod->definition->name.c_str());
			bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			bindItem->setData(id.networkCode.c_str(), Net);
			bindItem->setData(id.stationCode.c_str(), Sta);
			bindItem->setData(TypeModule, Type);
			bindItem->setData(QVariant::fromValue((void*)binding), Link);

			if ( binding->name.empty() ) {
				bindItem->setData(_docIcon, Qt::DecorationRole);
			}
			else {
				bindItem->setData(_linkIcon, Qt::DecorationRole);
			}

			auto *profileItem = new QStandardItem(binding->name.c_str());
			profileItem->setData(TypeProfile, Type);

			auto *item = _bindingsModel->itemFromIndex(idx);
			item->appendRow(QList<QStandardItem*>() << bindItem << profileItem);
			_stationsFolderView->selectionModel()->setCurrentIndex(
				bindItem->index(), QItemSelectionModel::ClearAndSelect
			);

			_bindingView->setModel(_model, _bindingsModel);
			_bindingView->setRootIndex(bindItem->index());
		}
		}
		break;
	}
}


void BindingsPanel::modulesViewContextMenu(const QPoint &p) {
	QModelIndex idx = _modulesView->rootIndex();
	QModelIndex hoveredIdx = _modulesView->indexAt(p);

	if ( !idx.isValid() ) {
		return;
	}

	if ( hoveredIdx.data(Type).toInt() != TypeModule ) {
		return;
	}

	QMenu menu;
	QAction *addAction = menu.addAction(tr("Add %1 profile").arg(hoveredIdx.data().toString()));
	QAction *res = menu.exec(_modulesView->mapToGlobal(p));

	if ( res == addAction ) {
		addProfile();
	}
}


void BindingsPanel::modulesFolderViewContextMenu(const QPoint &p) {
	QModelIndex idx = _modulesFolderView->rootIndex();
	//QModelIndex hoveredIdx = _modulesFolderView->indexAt(p);

	if ( !idx.isValid() ) {
		return;
	}

	QMenu menu;

	auto *addAction = menu.addAction(tr("Add profile"));
	QAction *delAction = nullptr;

	if ( !_modulesFolderView->selectionModel()->selectedIndexes().empty() ) {
		delAction = menu.addAction(tr("Delete selected profiles"));
	}

	QAction *res = menu.exec(_modulesFolderView->mapToGlobal(p));
	if ( res == addAction ) {
		addProfile();
	}
	else if ( res == delAction ) {
		deleteProfile();
	}
}


void BindingsPanel::folderLevelUp() {
	if ( _stationsFolderView->rootIndex().parent().isValid() ) {
		changeFolder(_stationsFolderView->rootIndex().parent());
	}
}


void BindingsPanel::deleteItem() {
	QModelIndex idx = _stationsFolderView->rootIndex();
	int type = idx.data(Type).toInt();

	if ( type == TypeRoot ) {
		QItemSelectionModel *sel = _stationsFolderView->selectionModel();
		QModelIndexList selection = sel->selectedIndexes();
		if ( selection.count() > 0 ) {
			if ( QMessageBox::question(nullptr, "Delete",
			       QString("Do you really want to delete %1 network(s)?")
			       .arg(selection.count()),
			       QMessageBox::Yes | QMessageBox::No
			     ) != QMessageBox::Yes ) {
				return;
			}
		}

		QList<QPersistentModelIndex> indexes;
		foreach ( const QModelIndex &i, selection )
			if ( i.column() == 0 ) {
				indexes.append(i);
			}

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) {
				continue;
			}

			while ( i.model()->rowCount(i) > 0 ) {
				deleteStation(i.model()->index(0, 0, i));
			}

			_stationsFolderView->model()->removeRow(i.row(), idx);
		}
	}
	else if ( type == TypeNetwork ) {
		QItemSelectionModel *sel = _stationsFolderView->selectionModel();
		QModelIndexList selection = sel->selectedIndexes();
		if ( selection.count() > 0 ) {
			if ( QMessageBox::question(nullptr, "Delete",
			       QString("Do you really want to delete %1 station(s)?")
			       .arg(selection.count()),
			       QMessageBox::Yes | QMessageBox::No
			     ) != QMessageBox::Yes ) {
				return;
			}
		}

		QList<QPersistentModelIndex> indexes;
		foreach ( const QModelIndex &i, selection )
			if ( i.column() == 0 ) {
				indexes.append(i);
			}

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) {
				continue;
			}
			deleteStation(i);
		}
	}
	if ( type == TypeStation ) {
		QItemSelectionModel *sel = _stationsFolderView->selectionModel();
		QModelIndexList selection = sel->selectedIndexes();
		if ( selection.count() > 0 ) {
			if ( QMessageBox::question(nullptr, "Delete",
			       QString("Do you really want to delete %1 binding(s)?")
			       .arg(selection.count()),
			       QMessageBox::Yes | QMessageBox::No
			     ) != QMessageBox::Yes ) {
				return;
			}
		}

		QList<QPersistentModelIndex> indexes;
		foreach ( const QModelIndex &i, selection )
			if ( i.column() == 0 ) {
				indexes.append(i);
			}

		foreach ( const QPersistentModelIndex &i, indexes ) {
			if ( !i.isValid() ) {
				continue;
			}

			StationID id(qPrintable(i.data(Net).toString()),
			             qPrintable(i.data(Sta).toString()));

			auto *b = getLink<ModuleBinding>(i);
			auto *mod = (Module*)b->parent;

			if ( mod->removeStation(id) ) {
				_stationsFolderView->model()->removeRow(i.row(), idx);
				_model->setModified();
			}
		}
	}
}


void BindingsPanel::deleteStation(const QModelIndex &idx) {
	StationID id(qPrintable(idx.parent().data().toString()),
	             qPrintable(idx.data().toString()));

	_model->model()->removeStation(id);
	_bindingsModel->removeRow(idx.row(), idx.parent());
	_model->setModified();
}


void BindingsPanel::deleteProfile(const QModelIndex &idx) {
	auto *binding = getLink<ModuleBinding>(idx);
	auto *mod = (Module*)binding->parent;
	mod->removeProfile(binding);
	syncProfileRemoval(_bindingsModel, binding);
	_model->setModified();
}


void BindingsPanel::syncProfileRemoval(QAbstractItemModel *m, void *link, const QModelIndex &parent) {
	int rows = m->rowCount(parent);
	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = m->index(i, 0, parent);
		syncProfileRemoval(m, link, idx);

		if ( idx.data(Type).toInt() == TypeModule ) {
			if ( link == idx.data(Link).value<void*>() ) {
				m->removeRow(idx.row(), parent);
				--i; --rows;
			}
		}
	}
}


bool BindingsPanel::assignProfile(const QModelIndex &idx, const QString &module,
                                  const QString &profile) {
	int type = idx.data(Type).toInt();
	switch ( type ) {
		case TypeRoot:
		{
			int rows = _bindingsModel->rowCount(idx);
			for ( int i = 0; i < rows; ++i ) {
				assignProfile(_bindingsModel->index(i, 0, idx), module, profile);
			}
			return true;
		}
		case TypeNetwork:
		{
			int rows = _bindingsModel->rowCount(idx);
			for ( int i = 0; i < rows; ++i ) {
				assignProfile(_bindingsModel->index(i, 0, idx), module, profile);
			}
			_model->setModified();
			return true;
		}
		case TypeStation:
		{
			// Get module
			auto *mod = _model->model()->module(qPrintable(module));
			if ( mod == nullptr ) {
				return false;
			}

			auto *prof = mod->getProfile(qPrintable(profile));
			if ( prof == nullptr ) {
				return false;
			}

			StationID id(qPrintable(idx.parent().data().toString()),
			             qPrintable(idx.data().toString()));

			int rows = _bindingsModel->rowCount(idx);
			for ( int i = 0; i < rows; ++i ) {
				auto child = _bindingsModel->index(i, 0, idx);
				if ( mod->definition->name != qPrintable(child.data().toString()) ) {
					continue;
				}

				if ( mod->bind(id, prof) ) {
					// Update an available binding
					_bindingsModel->setData(child, QVariant::fromValue((void*)prof), Link);
					_bindingsModel->setData(child.sibling(child.row(), 1), prof->name.c_str(), Qt::EditRole);
				}
				else {
					cerr << "ERROR: could not assign binding" << endl;
				}

				_model->setModified();
				return true;
			}

			// Nothing updated so far: create a new binding
			if ( !mod->bind(id, prof) ) {
				cerr << "ERROR: binding failed" << endl;
				return false;
			}

			auto *bindItem = new QStandardItem(mod->definition->name.c_str());
			bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			bindItem->setData(id.networkCode.c_str(), Net);
			bindItem->setData(id.stationCode.c_str(), Sta);
			bindItem->setData(TypeModule, Type);
			bindItem->setData(QVariant::fromValue((void*)prof), Link);

			if ( prof->name.empty() ) {
				bindItem->setData(_docIcon, Qt::DecorationRole);
			}
			else {
				bindItem->setData(_linkIcon, Qt::DecorationRole);
			}

			auto *profileItem = new QStandardItem(prof->name.c_str());
			profileItem->setData(TypeProfile, Type);

			auto *item = _bindingsModel->itemFromIndex(idx);
			item->appendRow(QList<QStandardItem*>() << bindItem << profileItem);

			_model->setModified();
			return true;
		}

		default:
			break;
	}

	return false;
}


void BindingsPanel::switchToStationsIconView() {
	_stationsFolderView->setViewMode(QListView::IconMode);
	_stationsFolderView->setGridSize(QSize(64,64));
	_stationsFolderView->setAcceptDrops(true);
}


void BindingsPanel::switchToStationsListView() {
	_stationsFolderView->setViewMode(QListView::ListMode);
	_stationsFolderView->setGridSize(QSize());
	_stationsFolderView->setSpacing(0);
	_stationsFolderView->setAcceptDrops(true);
}


void BindingsPanel::switchToProfileIconView() {
	_modulesFolderView->setViewMode(QListView::IconMode);
	_modulesFolderView->setGridSize(QSize(80,64));
	_modulesFolderView->setDragEnabled(true);
}


void BindingsPanel::switchToProfileListView() {
	_modulesFolderView->setViewMode(QListView::ListMode);
	_modulesFolderView->setGridSize(QSize());
	_modulesFolderView->setSpacing(0);
	_modulesFolderView->setDragEnabled(true);
}


void BindingsPanel::addProfile() {
	if ( _modulesFolderView->rootIndex().data(Type) != TypeModule ) {
		return;
	}

	NewNameDialog dlg(_modulesFolderView->rootIndex(), false, this);
	dlg.setWindowTitle(QString("New %1 profile").arg(_modulesFolderView->rootIndex().data().toString()));
	dlg.setHint("Only alphanumeric characters, underscore and dash are supported.");
	dlg.setValidator(new ProfileValidator(&dlg));
	if ( dlg.exec() != QDialog::Accepted ) {
		return;
	}

	auto *mod = getLink<Module>(_modulesFolderView->rootIndex());

	ModuleBinding *profile = mod->createProfile(dlg.name().toStdString());

	if ( profile == nullptr ) {
		QMessageBox::critical(this, "Add profile",
		                      "Adding the profile failed.");
		return;
	}

	auto *profItem = new QStandardItem(profile->name.c_str());
	profItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
	profItem->setData(TypeProfile, Type);
	profItem->setData(QVariant::fromValue((void*)profile), Link);
	profItem->setData(_docIcon, Qt::DecorationRole);

	auto *item = _profilesModel->itemFromIndex(_modulesFolderView->rootIndex());
	item->appendRow(profItem);
}


void BindingsPanel::deleteProfile() {
	if ( _modulesFolderView->rootIndex().data(Type) != TypeModule ) {
		return;
	}

	QItemSelectionModel *sel = _modulesFolderView->selectionModel();
	QModelIndexList selection = sel->selectedIndexes();
	if ( selection.isEmpty() ) {
		return;
	}

	if ( QMessageBox::question(nullptr, "Delete",
	           QString("Do you really want to delete %1 profile(s)?\n"
	                   "Each active binding that is using one of the "
	                   "selected profiles will be removed as well.")
	           .arg(selection.count()),
	           QMessageBox::Yes | QMessageBox::No
	     ) != QMessageBox::Yes ) {
		return;
	}

	QList<QPersistentModelIndex> indexes;
	foreach ( const QModelIndex &i, selection )
		if ( i.column() == 0 ) {
			indexes.append(i);
		}

	foreach ( const QPersistentModelIndex &i, indexes ) {
		if ( !i.isValid() ) {
			continue;
		}

		deleteProfile(i);
		_modulesFolderView->model()->removeRow(i.row(), i.parent());
	}

	_model->setModified();
}
