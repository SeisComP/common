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


#include "help.h"
#include "../icon.h"

#include <seiscomp/gui/core/compat.h>
#include <seiscomp/system/environment.h>

#include <QAbstractTextDocumentLayout>
#include <QActionGroup>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QMenu>
#include <QPainter>
#include <QTextEdit>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>


using namespace std;



namespace {


class IconBesideTextDelegate : public QAbstractItemDelegate {
	public:
		using QAbstractItemDelegate::QAbstractItemDelegate;

	public:
		void paint(QPainter *p,
		           const QStyleOptionViewItem &option,
		           const QModelIndex &index) const override {
			if ( !option.showDecorationSelected ) {
				const_cast<QStyleOptionViewItem*>(&option)->showDecorationSelected = true;
			}
			option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, option.widget);

			auto s = static_cast<const QListView*>(option.widget)->iconSize();
			auto spacing = static_cast<const QListView*>(option.widget)->spacing();
			auto pm = index.data(Qt::DecorationRole).value<QIcon>().pixmap(s);
			p->drawPixmap(
				option.rect.left(),
				option.rect.top() + (option.rect.height() - s.height()) / 2,
				pm
			);

			QString text = index.data().toString();

			p->setPen(
				option.palette.color(
					option.state & QStyle::State_Selected ?
						QPalette::HighlightedText : QPalette::Text
				)
			);
			auto tr = option.rect.adjusted(s.width() + spacing, 0, 0, 0);
			if ( QT_FM_WIDTH(option.fontMetrics, text) > tr.width() ) {
				text = option.fontMetrics.elidedText(text, Qt::ElideRight, tr.width());
			}

			p->drawText(tr, Qt::AlignLeft | Qt::AlignVCenter, text);
		}

		QSize sizeHint(const QStyleOptionViewItem &option,
		               const QModelIndex &index) const override {
			// 20 character, 1 character spacing and icon
			return QSize(
				option.fontMetrics.averageCharWidth() * 20 +
				static_cast<const QListView*>(option.widget)->spacing() +
				static_cast<const QListView*>(option.widget)->iconSize().width(),
				qMax(
					option.fontMetrics.height(),
					static_cast<const QListView*>(option.widget)->iconSize().height()
				)
			);
		}
};


}


#define TYPEROLE Qt::UserRole+1
#define PATHROLE Qt::UserRole+2
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HelpPanel::HelpPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	QAction *a;

	_name = "Docs";
	_icon = ::icon("menu_scconfig_docs");
	setHeadline("Changelog and Documentation");
	setDescription("View changelogs and documentations for installed packages and libraries");

	QToolBar *tools = new QToolBar;
	tools->setIconSize(QSize(24,24));
	tools->setAutoFillBackground(true);
	tools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	QActionGroup *viewMode = new QActionGroup(tools);
	viewMode->setExclusive(true);

	a = tools->addAction("Icons");
	a->setCheckable(true);
	viewMode->addAction(a);
	a->setIcon(::icon("view_icons"));
	a->setChecked(true);
	connect(a, &QAction::triggered, this, [this, a] {
		if ( a->isChecked() ) {
			_folderView->setViewMode(QListView::IconMode);
			_folderView->setIconSize(QSize(72, 72));
			_folderView->setSpacing(6);
		}
	});
	a = tools->addAction("List");
	a->setCheckable(true);
	viewMode->addAction(a);
	a->setIcon(::icon("view_list"));
	connect(a, &QAction::triggered, this, [this, a] {
		if ( a->isChecked() ) {
			_folderView->setViewMode(QListView::ListMode);
			_folderView->setIconSize(QSize(24, 24));
			_folderView->setSpacing(0);
		}
	});

	tools->addSeparator();

	a = tools->addAction("Refresh");
	a->setShortcut(QKeySequence(Qt::Key_F5));
	a->setIcon(::icon("refresh"));
	connect(a, &QAction::triggered, this, &HelpPanel::refresh);

	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(1);
	setLayout(l);

	l->addWidget(tools);

	_folderView = new QListView;
	_folderView->setFrameShape(QFrame::NoFrame);
	_folderView->setFlow(QListView::TopToBottom);
	_folderView->setResizeMode(QListView::Adjust);
	_folderView->setSelectionMode(QAbstractItemView::SingleSelection);

	_folderView->setViewMode(QListView::IconMode);
	_folderView->setMovement(QListView::Static);
	_folderView->setIconSize(QSize(72, 72));
	_folderView->setSpacing(6);
	_folderView->setItemDelegate(new IconBesideTextDelegate(_folderView));
	//_folderView->setGridSize(QSize(172,164 + fontMetrics().height()*2));
	_folderView->setUniformItemSizes(true);

	connect(_folderView, &QListView::activated, this, &HelpPanel::openIndex);

	_model = new QStandardItemModel;
	_folderView->setModel(_model);

	l->addWidget(_folderView);

	refresh();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HelpPanel::refresh() {
	static QIcon iconDoc = ::icon("docs_help");
	static QIcon iconChangelog = ::icon("docs_changelog");

	_model->clear();
	_model->setHeaderData(0, Qt::Horizontal, tr("Name"));

	auto env = Seiscomp::Environment::Instance();
	QDir docDir((env->shareDir() + "/doc").c_str());
	QFileInfoList entries = docDir.entryInfoList();

	foreach ( QFileInfo fileInfo, entries ) {
		if ( !fileInfo.isDir() ) continue;
		QString name = fileInfo.baseName();
		if ( name == "." || name == ".." ) continue;
		if ( name.isEmpty() ) continue;

		if ( QFile::exists(fileInfo.absoluteFilePath() + "/html/index.html") ) {
			QStandardItem *item = new QStandardItem;
			item->setText(name);
			item->setData(name.toLower(), Qt::UserRole);
			item->setIcon(iconDoc);
			item->setEditable(false);
			// Type HTML
			item->setData(1, TYPEROLE);
			item->setData(fileInfo.absoluteFilePath() + "/html/index.html", PATHROLE);
			_model->appendRow(item);
		}
		else if ( QFile::exists(fileInfo.absoluteFilePath() + "/index.html") ) {
			QStandardItem *item = new QStandardItem;
			item->setText(name);
			item->setData(name.toLower(), Qt::UserRole);
			item->setIcon(iconDoc);
			item->setEditable(false);
			// Type HTML
			item->setData(1, TYPEROLE);
			item->setData(fileInfo.absoluteFilePath() + "/index.html", PATHROLE);
			_model->appendRow(item);
		}

		if ( QFile::exists(fileInfo.absoluteFilePath() + "/CHANGELOG") ) {
			QStandardItem *item = new QStandardItem;
			item->setText(name);
			item->setData(name.toLower(), Qt::UserRole);
			item->setIcon(iconChangelog);
			item->setEditable(false);
			// Type changelog
			item->setData(2, TYPEROLE);
			item->setData(fileInfo.absoluteFilePath() + "/CHANGELOG", PATHROLE);
			_model->appendRow(item);
		}
	}

	_model->setSortRole(Qt::UserRole);
	_model->sort(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HelpPanel::openIndex(const QModelIndex &index) {
	int type = index.data(TYPEROLE).toInt();
	QString path = index.data(PATHROLE).toString();

	if ( type == 1 ) {
		QDesktopServices::openUrl(path);
	}
	else if ( type == 2 ) {
		QFile file(path);
		file.open(QFile::ReadOnly);
		QByteArray data = file.readAll();
		file.close();

		QDialog dlg;
		dlg.setWindowTitle(index.data().toString() + " - changelog");
		dlg.resize(QSize(dlg.fontMetrics().height()*60, dlg.fontMetrics().height()*25));

		QVBoxLayout *vl = new QVBoxLayout;

		QTextEdit *edit = new QTextEdit();
		edit->setWordWrapMode(QTextOption::NoWrap);
		edit->setReadOnly(true);
		edit->setText(data);

		vl->addWidget(edit);
		dlg.setLayout(vl);

		dlg.exec();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
