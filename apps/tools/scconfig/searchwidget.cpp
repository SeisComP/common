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


#include "fancyview.h"
#include "gui.h"
#include "icon.h"
#include "searchwidget.h"

#include <seiscomp/system/model.h>

#include <QCheckBox>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>


using namespace Seiscomp::System;


namespace {


// The parameters of the global module are shown either as the "global" section
// of a binding or as the sections imported into a module configuration.
bool isGlobalSection(const QModelIndex &idx) {
	if ( idx.data(ConfigurationTreeItemModel::Type).toInt()
	     != ConfigurationTreeItemModel::TypeSection ) {
		return false;
	}

	auto *sec = reinterpret_cast<Section*>(
	                idx.data(ConfigurationTreeItemModel::Link).value<void*>());
	if ( !sec ) {
		return false;
	}

	return (sec->name == "global")
	    || (sec->description.compare(0, 27, "Import of global parameters") == 0);
}


// True if the shown module or binding is the global one, its parameters cannot
// be excluded from the search then.
bool showsGlobalModule(const QModelIndex &root) {
	switch ( root.data(ConfigurationTreeItemModel::Type).toInt() ) {
		case ConfigurationTreeItemModel::TypeModule: {
			auto *mod = reinterpret_cast<Module*>(
			                root.data(ConfigurationTreeItemModel::Link).value<void*>());
			return mod && (mod->definition->name == "global");
		}

		case ConfigurationTreeItemModel::TypeBinding: {
			auto *binding = reinterpret_cast<ModuleBinding*>(
			                    root.data(ConfigurationTreeItemModel::Link).value<void*>());
			auto *mod = binding ? static_cast<Module*>(binding->parent) : nullptr;
			return mod && (mod->definition->name == "global");
		}

		default:
			break;
	}

	return false;
}


void findMatch(QModelIndexList &hits, QModelIndex start, const QString &text,
               bool withNames, bool withValues, bool withHelp,
               bool excludeGlobal,
               Qt::CaseSensitivity cs, bool fullMatch) {
	const int rows = start.model()->rowCount(start);

	// First pass: only check parameters
	for ( int i = 0; i < rows; ++i ) {
		auto idx = start.model()->index(i, 0, start);
		auto type = idx.data(ConfigurationTreeItemModel::Type).toInt();
		if ( type == ConfigurationTreeItemModel::TypeStruct ) {
			continue;
		}

		if ( type != ConfigurationTreeItemModel::TypeParameter ) {
			continue;
		}

		bool hit = false;

		if ( !hit && withNames ) {
			hit = fullMatch ?
			          idx.data(Qt::DisplayRole).toString().compare(text, cs) == 0 :
			          idx.data(Qt::DisplayRole).toString().contains(text, cs);
		}

		if ( !hit && withValues ) {
			hit = fullMatch ?
			          idx.sibling(idx.row(), 2).data(Qt::DisplayRole).toString().compare(text, cs) == 0 :
			          idx.sibling(idx.row(), 2).data(Qt::DisplayRole).toString().contains(text, cs);
		}

		if ( !hit && withHelp ) {
			hit = fullMatch ?
			          idx.data(Qt::ToolTipRole).toString().compare(text, cs) == 0 :
			          idx.data(Qt::ToolTipRole).toString().contains(text, cs);
		}

		if ( hit ) {
			hits.append(idx);
		}
	}

	// Second pass: check non parameters
	for ( int i = 0; i < rows; ++i ) {
		auto idx = start.model()->index(i, 0, start);
		auto type = idx.data(ConfigurationTreeItemModel::Type).toInt();
		if ( type == ConfigurationTreeItemModel::TypeStruct ) {
			continue;
		}

		if ( type == ConfigurationTreeItemModel::TypeParameter ) {
			continue;
		}

		if ( excludeGlobal && isGlobalSection(idx) ) {
			continue;
		}

		bool hit = false;

		if ( !hit && withNames ) {
			hit = fullMatch ?
			          idx.data(Qt::DisplayRole).toString().compare(text, cs) == 0 :
			          idx.data(Qt::DisplayRole).toString().contains(text, cs);
		}

		if ( !hit && withHelp ) {
			hit = fullMatch ?
			          idx.data(Qt::ToolTipRole).toString().compare(text, cs) == 0 :
			          idx.data(Qt::ToolTipRole).toString().contains(text, cs);
		}

		if ( hit ) {
			hits.append(idx);
		}

		findMatch(hits, idx, text, withNames, withValues, withHelp, excludeGlobal,
		          cs, fullMatch);
	}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SearchWidget::SearchWidget(QAbstractItemView *view, QWidget *parent)
: QWidget(parent), _view(view) {
	setFocusPolicy(Qt::StrongFocus);

	auto searchLayout = new QHBoxLayout;
	setLayout(searchLayout);

	auto labelSearch = new QLabel;
	labelSearch->setText(tr("Search"));
	labelSearch->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred));

	_edit = new QLineEdit;
	_edit->installEventFilter(this);
	connect(_edit, &QLineEdit::textEdited, this, &SearchWidget::intermediateSearch);

	_labelCurrent = new QLabel;
	auto labelSep = new QLabel("/");
	_labelCount = new QLabel;

	auto searchPrev = new QPushButton;
	searchPrev->setIcon(icon("chevron_up"));
	connect(searchPrev, &QPushButton::clicked, this, &SearchWidget::prevSearch);

	auto searchNext = new QPushButton;
	searchNext->setIcon(icon("chevron_down"));
	connect(searchNext, &QPushButton::clicked, this, &SearchWidget::nextSearch);

	auto searchClose = new QPushButton;
	searchClose->setIcon(icon("close"));
	searchClose->setFlat(true);
	connect(searchClose, &QPushButton::clicked, this, &SearchWidget::closeRequested);

	auto withNames = new QCheckBox;
	withNames->setText(tr("&Names"));
	withNames->setChecked(_withNames);
	connect(withNames, &QCheckBox::toggled, this, [this](bool state) {
		_withNames = state;
		intermediateSearch(_edit->text());
	});

	auto withValues = new QCheckBox;
	withValues->setText(tr("&Values"));
	withValues->setChecked(_withValues);
	connect(withValues, &QCheckBox::toggled, this, [this](bool state) {
		_withValues = state;
		intermediateSearch(_edit->text());
	});

	auto withHelp = new QCheckBox;
	withHelp->setText(tr("&Description"));
	withHelp->setChecked(_withHelp);
	connect(withHelp, &QCheckBox::toggled, this, [this](bool state) {
		_withHelp = state;
		intermediateSearch(_edit->text());
	});

	_excludeGlobal = new QCheckBox;
	_excludeGlobal->setText(tr("Exclude &global"));
	_excludeGlobal->setChecked(_excludeGlobalValue);
	_excludeGlobal->setToolTip(tr("Skip the parameters inherited from the "
	                              "global module"));
	connect(_excludeGlobal, &QCheckBox::toggled, this, [this](bool state) {
		_excludeGlobalValue = state;
		intermediateSearch(_edit->text());
	});

	auto sep1 = new QFrame();
	sep1->setForegroundRole(QPalette::Mid);
	sep1->setFrameShape(QFrame::VLine);
	sep1->setFrameShadow(QFrame::Plain);

	searchLayout->addWidget(labelSearch);
	searchLayout->addWidget(_edit);
	searchLayout->addWidget(_labelCurrent);
	searchLayout->addWidget(labelSep);
	searchLayout->addWidget(_labelCount);
	searchLayout->addWidget(searchPrev);
	searchLayout->addWidget(searchNext);
	searchLayout->addWidget(sep1);
	searchLayout->addWidget(withNames);
	searchLayout->addWidget(withValues);
	searchLayout->addWidget(withHelp);
	searchLayout->addWidget(_excludeGlobal);

	searchLayout->addStretch();
	searchLayout->addWidget(searchClose);

	// The option depends on what is shown, this may change while the search
	// is open.
	if ( auto *fancyView = qobject_cast<FancyView*>(_view) ) {
		connect(fancyView, &FancyView::rootIndexChanged,
		        this, &SearchWidget::updateGlobalOption);
	}

	updateGlobalOption();

	setTabOrder(_edit, searchPrev);
	setTabOrder(searchPrev, searchNext);
	setTabOrder(searchNext, searchClose);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::reset() {
	_hits = {};
	_labelCount->setText(QString::number(_hits.size()));
	updateGlobalOption();
	showCurrent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::updateGlobalOption() {
	// Nothing can be excluded if the global parameters are the ones shown
	const bool available = !showsGlobalModule(_view->rootIndex());

	_excludeGlobal->setEnabled(available);

	if ( !available ) {
		// Leaves the option unchecked when it becomes available again
		_excludeGlobalValue = false;
		_excludeGlobal->setChecked(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::focusInEvent(QFocusEvent *e) {
	_edit->setFocus(e->reason());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SearchWidget::eventFilter(QObject *obj, QEvent *event) {
	if ( obj == _edit ) {
		if ( event->type() == QEvent::KeyPress ) {
			auto keyEvent = static_cast<QKeyEvent*>(event);
			if ( (keyEvent->key() == Qt::Key_Enter)
			  || (keyEvent->key() == Qt::Key_Return) ) {
				if ( keyEvent->modifiers() == Qt::ShiftModifier ) {
					prevSearch();
				}
				else {
					nextSearch();
				}
			}
		}
	}
	return QWidget::eventFilter(obj, event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::intermediateSearch(const QString &text) {
	match(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::match(QString text) {
	_hits = {};

	bool fullMatch = false;
	if ( (text.length() > 2) && text.startsWith('|') && text.endsWith('|') ) {
		fullMatch = true;
		text = text.mid(1, text.length() - 2);
	}

	Qt::CaseSensitivity cs = Qt::CaseInsensitive;
	if ( (text.length() > 2) && text.startsWith('"') && text.endsWith('"') ) {
		cs = Qt::CaseSensitive;
		text = text.mid(1, text.length() - 2);
	}

	if ( _view->rootIndex().model() ) {
		if ( text.isEmpty() ) {
			_currentSearchIndex = -1;
			_view->setCurrentIndex(_view->rootIndex());
			_view->scrollTo(_view->rootIndex());
		}
		else {
			findMatch(_hits, _view->rootIndex(), text,
			            _withNames, _withValues, _withHelp,
			            _excludeGlobalValue,
			            cs, fullMatch);
			_currentSearchIndex = _hits.isEmpty() ? -1 : 0;
		}
	}

	_labelCount->setText(QString::number(_hits.size()));
	showCurrent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::showCurrent() {
	if ( _hits.empty() ) {
		_view->setCurrentIndex(_view->rootIndex());
		_labelCurrent->setText("0");
		return;
	}

	_labelCurrent->setText(QString::number(_currentSearchIndex + 1));
	_view->setCurrentIndex(_hits[_currentSearchIndex]);
	_view->scrollTo(_hits[_currentSearchIndex]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::prevSearch() {
	if ( _hits.empty() ) {
		return;
	}

	--_currentSearchIndex;
	if ( _currentSearchIndex < 0 ) {
		_currentSearchIndex = _hits.size() - 1;
	}

	showCurrent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SearchWidget::nextSearch() {
	if ( _hits.empty() ) {
		return;
	}

	++_currentSearchIndex;
	if ( _currentSearchIndex >= _hits.size() ) {
		_currentSearchIndex = 0;
	}

	showCurrent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
