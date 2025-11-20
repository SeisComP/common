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


#define SEISCOMP_COMPONENT Configurator

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QIconEngine>
#include <QFile>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QWidget>

#include <QtSvg/QSvgRenderer>

#include <seiscomp/version.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/core/system.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/system/environment.h>

#include "about.h"
#include "avatar.h"
#include "editor.h"
#include "icon.h"
#include "gui.h"
#include "wizard.h"

#include "panels/bindings.h"
#include "panels/information.h"
#include "panels/inventory.h"
#include "panels/modules.h"
#include "panels/system.h"
#include "panels/help.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::System;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


const auto globalGridColor = QPalette::Mid;
const auto globalListWidth = 100;
const auto globalIconSize = QSize(24, 24);


enum Columns {
	NAME,
	TYPE,
	VALUE,
	LOCKED
};


class SvgIconEngine : public QIconEngine {
	public:
		explicit SvgIconEngine(const QString &svg) {
			_content = svg.toUtf8();
		}

	public:
		QIconEngine *clone() const override {
			return new SvgIconEngine(_content);
		}

		QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override {
			return QSize(48, 48);
		}

		QString key() const override {
			return "scconfig_svg_content";
		}

		void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
			painter->drawPixmap(
				rect.left(), rect.top(),
				pixmap(rect.size(), mode, state)
			);
		}

		QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
			QSvgRenderer renderer;
			if ( !renderer.load(_content) ) {
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

			QImage img(actualSize, QImage::Format_ARGB32);
			img.fill(Qt::transparent);
			{
				QPainter p(&img);
				renderer.render(&p);
				p.end();
			}

			/*
			if ( mode == QIcon::Disabled ) {
				int count = img.width() * img.height();
				QRgb *data = reinterpret_cast<QRgb*>(img.bits());
				for ( int i = 0; i < count; ++i, ++data ) {
					int gray = (299 * qRed(*data) + 587 * qGreen(*data) + 114 * qBlue(*data)) / 1000;
					*data = qRgba(gray, gray, gray, qAlpha(*data));
				}
			}
			*/

			auto pm = QPixmap::fromImage(img);
			pm.setDevicePixelRatio(qApp->devicePixelRatio());

			return pm;
		}

	private:
		QByteArray _content;
};


QIcon svg(const QString &document) {
	return QIcon(new SvgIconEngine(document));
}


class ModuleListDelegate : public QItemDelegate {
	public:
		ModuleListDelegate(QObject *parent) : QItemDelegate(parent) {}

	public:
		void paint(QPainter *p,
		           const QStyleOptionViewItem &option,
		           const QModelIndex &index) const override {
			if ( option.state & QStyle::State_Selected ) {
				p->fillRect(option.rect, option.palette.color(QPalette::Highlight));
				p->setPen(option.palette.color(QPalette::HighlightedText));
			}
			else if ( option.state & QStyle::State_MouseOver ) {
				auto c = blend(option.palette.color(QPalette::Highlight), option.palette.color(QPalette::Window), 50);
				p->fillRect(option.rect, c);
				p->setPen(option.palette.color(QPalette::HighlightedText));
			}
			else {
				p->setPen(option.palette.color(QPalette::Text));
			}

			int em = option.fontMetrics.height() / 2;
			auto rect = option.rect.adjusted(0, em / 2, 0, -em / 2);

			QIcon ico = index.data(Qt::DecorationRole).value<QIcon>();
			if ( !ico.isNull() ) {
				auto pm = ico.pixmap(static_cast<const QListWidget*>(option.widget)->iconSize());
				auto layoutSize = pm.size() / pm.devicePixelRatioF();
				p->drawPixmap(
					rect.left() + (rect.width() - layoutSize.width()) / 2,
					rect.top() + (rect.height() - option.fontMetrics.height() - layoutSize.height()) / 2,
					pm
				);
			}

			p->drawText(rect,
			            Qt::AlignHCenter | Qt::AlignBottom,
			            index.data(Qt::DisplayRole).toString());
		}

		QSize sizeHint(const QStyleOptionViewItem &option,
		               const QModelIndex &index) const override {
			return QSize(
				globalListWidth,
				static_cast<const QListWidget*>(option.widget)->iconSize().height() +
				option.fontMetrics.height() * 3 / 2
			);
		}
};


class ModuleListPainter : public QObject {
	public:
		ModuleListPainter(QObject *parent = NULL) : QObject(parent) {}

	protected:
		bool eventFilter(QObject *obj, QEvent *event) {
			if ( event->type() == QEvent::Paint ) {
				QWidget *w = static_cast<QWidget*>(obj);
				QPainter p(w);
				p.fillRect(w->rect(), w->palette().color(w->parentWidget()->backgroundRole()));
				return false;
			}

			return QObject::eventFilter(obj, event);
		}
};


class StageSelectionDialog : public QDialog {
	public:
		enum Mode {
			User,
			System
		};

		StageSelectionDialog(QWidget *parent) : QDialog(parent) {
			Environment *env = Environment::Instance();

			QVBoxLayout *layout = new QVBoxLayout;
			QHBoxLayout *hlayout;
			QLabel *label;

			label = new QLabel;
			QFont f = label->font();
			f.setBold(true);
			f.setPointSize(f.pointSize()*150/100);
			label->setFont(f);
			label->setText(tr("Select configuration mode"));
			label->setAlignment(Qt::AlignCenter);
			layout->addWidget(label);

			layout->addSpacing(fontMetrics().ascent());

			// Create dialog here
			_systemMode = new QPushButton;
			_systemMode->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
			_systemMode->setIcon(::icon("menu_scconfig_system"));
			_systemMode->setIconSize(QSize(70, 70));

			label = new QLabel;
			label->setWordWrap(true);
			label->setAlignment(Qt::AlignCenter);
			label->setText(QString(tr("Manage system configuration in <i>%1</i>.")).arg(env->appConfigDir().c_str()));

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			hlayout->addWidget(_systemMode);
			hlayout->addStretch();

			layout->addLayout(hlayout);
			layout->addWidget(label);

			QFrame *frame = new QFrame;
			frame->setFrameShape(QFrame::HLine);

			layout->addWidget(frame);

			_userMode = new QPushButton;
			_userMode->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
			_userMode->setIcon(svg(multiavatar().data()));
			_userMode->setIconSize(QSize(70, 70));

			label = new QLabel;
			label->setWordWrap(true);
			label->setAlignment(Qt::AlignCenter);
			label->setText(QString(tr("Manage user configuration in <i>%1</i>.")).arg(env->configDir().c_str()));
			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			hlayout->addWidget(_userMode);
			hlayout->addStretch();

			layout->addLayout(hlayout);
			layout->addWidget(label);

			layout->addStretch();

			setLayout(layout);

			connect(_userMode, &QPushButton::clicked, this, &QDialog::accept);
			connect(_systemMode, &QPushButton::clicked, this, &QDialog::accept);
		}

		void accept() {
			QObject *w = sender();

			if ( w == _userMode ) {
				_mode = User;
			}
			else if ( w == _systemMode ) {
				_mode = System;
			}

			QDialog::accept();
		}

		Mode mode() { return _mode; }


	private:
		Mode         _mode;
		QPushButton *_userMode;
		QPushButton *_systemMode;
};


class HeadlineLabel : public QLabel {
	public:
		using QLabel::QLabel;

	public:
		void setText(const QString &text, const QString &subText) {
			QLabel::setText(text);
			_subText = subText;
			update();
		}

	protected:
		void paintEvent(QPaintEvent *) override {
			QPainter p(this);
			p.setPen(palette().color(foregroundRole()));
			p.drawText(rect().adjusted(20, 0, 0, 0),
			           Qt::AlignLeft | Qt::AlignVCenter, text());
			if ( !_subText.isEmpty() ) {
				auto tw = QT_FM_WIDTH(fontMetrics(), text());
				auto f = p.font();
				f.setBold(false);
				p.setFont(f);
				p.drawText(rect().adjusted(20 + tw, 0, 0, 0),
				           Qt::AlignLeft | Qt::AlignVCenter, QString(" / %1").arg(_subText));
			}
		}

	private:
		QString _subText;
};


class InfoLabel : public QLabel {
	public:
		using QLabel::QLabel;

	protected:
		void paintEvent(QPaintEvent *) override {
			QPainter p(this);
			p.setPen(palette().color(foregroundRole()));

			auto tw = QT_FM_WIDTH(fontMetrics(), text());
			QRect tr = rect().adjusted(20, 0, -20, 0);

			if ( tw > tr.width() ) {
				p.drawText(
					tr, Qt::AlignLeft | Qt::AlignVCenter,
					fontMetrics().elidedText(text(), Qt::ElideRight, tr.width())
				);
			}
			else {
				p.drawText(tr, Qt::AlignLeft | Qt::AlignVCenter, text());
			}
		}
};


class MouseTrackLabel : public QWidget {
	public:
		MouseTrackLabel(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		: QWidget(parent, f) {
			setMouseTracking(true);
		}

		void setIcon(const QIcon &icon, const QSize &size) {
			_icon = icon;
			_iconSize = size;
			_annotationIcon = {};
			updatePixmap();
			updateAnnotationPixmap();
		}

		void setAnnotationIcon(const QIcon &icon) {
			_annotationIcon = icon;
			updateAnnotationPixmap();
		}

	protected:
		void paintEvent(QPaintEvent *) override {
			QPainter p(this);
			QSize layoutSize = _pixmap.size() / _pixmap.devicePixelRatioF();
			int xofs = (width() - layoutSize.width()) / 2;
			int yofs = (height() - layoutSize.height()) / 2;
			p.drawPixmap(xofs, yofs, _pixmap);

			if ( !_annotationPixmap.isNull() ) {
				layoutSize = _annotationPixmap.size() / _annotationPixmap.devicePixelRatioF();
				xofs = width() - layoutSize.width();
				yofs = 0;
				p.drawPixmap(xofs, yofs, _annotationPixmap);
			}
		}

	private:
		void updatePixmap() {
			_pixmap = _icon.pixmap(_iconSize);
			update();
		}

		void updateAnnotationPixmap() {
			if ( !_annotationIcon.isNull() ) {
				_annotationPixmap = _annotationIcon.pixmap(size() / 2);
			}
			else {
				_annotationPixmap = {};
			}
			update();
		}

	private:
		QIcon   _icon;
		QSize   _iconSize;
		QPixmap _pixmap;
		QIcon   _annotationIcon;
		QPixmap _annotationPixmap;
};


typedef QList<QStandardItem*> QStandardItemList;


QString paramValue(const Parameter *param, Environment::ConfigStage targetStage) {
	QString value;

	// If the current stage is defined, display its value
	if ( param->symbols[targetStage] &&
	     param->symbols[targetStage]->symbol.stage != Environment::CS_UNDEFINED ) {
		value = param->symbols[targetStage]->symbol.content.c_str();
	}
	else {
		// Otherwise use the resolved value
		value = param->symbol.content.c_str();
	}

	/*
	QString value;
	for ( size_t i = 0; i < param->symbol.values.size(); ++i ) {
		if ( i > 0 ) value += ", ";
		value += param->symbol.values[i].c_str();
	}
	*/

	return value;
}


void addParameter(QStandardItem *item, Parameter *param, int level,
                  Environment::ConfigStage targetStage) {
	QStandardItem *name = new QStandardItem(param->definition->name.c_str());
	QStandardItem *type = new QStandardItem(param->definition->type.c_str());

	QString valueText = paramValue(param, targetStage);

	QStandardItem *value = new QStandardItem();
	value->setData(valueText, Qt::DisplayRole);
	QStandardItem *locked = new QStandardItem();
	QStandardItem *values = new QStandardItem(Core::toString(param->definition->values).c_str());
	QStandardItem *range = new QStandardItem(param->definition->range.c_str());
	QStandardItem *options = new QStandardItem(Core::toString(param->definition->options).c_str());

	name->setData(level, ConfigurationTreeItemModel::Level);
	name->setData(QVariant::fromValue((void*)param), ConfigurationTreeItemModel::Link);
	name->setData(ConfigurationTreeItemModel::TypeParameter, ConfigurationTreeItemModel::Type);

	if ( param->initial[targetStage] &&
	     param->initial[targetStage]->symbol.stage != Environment::CS_UNDEFINED ) {
		value->setData(QString(param->initial[targetStage]->symbol.content.data()), ConfigurationTreeItemModel::Initial);
		locked->setData(false, ConfigurationTreeItemModel::Initial);
	}
	else {
		locked->setData(true, ConfigurationTreeItemModel::Initial);
	}

	if ( param->symbols[targetStage] &&
	     param->symbols[targetStage]->symbol.stage != Environment::CS_UNDEFINED ) {
		locked->setData(false, Qt::DisplayRole);
	}
	else {
		locked->setData(true, Qt::DisplayRole);
	}

	name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	type->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	options->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	value->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable);
	locked->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

	std::string descText;
	if ( !param->definition->description.empty() ) {
		descText = param->definition->description;
	}

	if ( !param->definition->type.empty() ) {
		if ( !descText.empty() ) {
			descText = descText + "\n";
		}
		descText = descText + "Type: " + param->definition->type;
	}
	if ( !param->definition->values.empty() ) {
		if ( !descText.empty() ) {
			descText = descText + "\n";
		}
		descText = descText + "Supported values: " + Core::toString(param->definition->values);
	}
	if ( !param->definition->range.empty() ) {
		if ( !descText.empty() ) {
			descText = descText + "\n";
		}
		descText = descText + "Range: " + param->definition->range;
	}
	if ( !param->definition->options.empty() ) {
		if ( !descText.empty() ) {
			descText = descText + "\n";
		}
		descText = descText + "Options: " + Core::toString(param->definition->options);
	}
	name->setToolTip(descText.c_str());
	type->setToolTip(name->toolTip());
	value->setToolTip(name->toolTip());
	locked->setToolTip(param->symbol.uri.c_str());

	item->appendRow(QStandardItemList() << name << type << value << locked << values << range << options);

	//for ( size_t i = 0; i < param->childs.size(); ++i )
	//	addParameters(name, param->childs[i].get(), level+1);
}


void addGroup(QStandardItem *item, Group *group, int level,
              Environment::ConfigStage targetStage);
void addStructure(QStandardItem *item, const Structure *struc, int level,
                  Environment::ConfigStage targetStage);

void loadStructure(QStandardItem *item, const Structure *struc, int level,
                   Environment::ConfigStage targetStage) {
	for ( size_t i = 0; i < struc->groups.size(); ++i ) {
		addGroup(item, struc->groups[i].get(), level+1, targetStage);
	}

	for ( size_t i = 0; i < struc->parameters.size(); ++i ) {
		addParameter(item, struc->parameters[i].get(), level+1, targetStage);
	}

	for ( size_t i = 0; i < struc->structures.size(); ++i ) {
		addStructure(item, struc->structures[i].get(), level+1, targetStage);
	}

	for ( size_t i = 0; i < struc->structureTypes.size(); ++i ) {
		addStructure(item, struc->structureTypes[i].get(), level+1, targetStage);
	}
}


void addStructure(QStandardItem *item, const Structure *struc, int level,
                  Environment::ConfigStage targetStage) {
	QStandardItem *child = new QStandardItem(struc->definition->type.c_str());
	child->setData(level, ConfigurationTreeItemModel::Level);
	child->setData(QVariant::fromValue((void*)struc), ConfigurationTreeItemModel::Link);
	child->setData(ConfigurationTreeItemModel::TypeStruct, ConfigurationTreeItemModel::Type);
	child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->appendRow(QStandardItemList() << child);
	loadStructure(child, struc, level, targetStage);
}


void addGroup(QStandardItem *item, Group *group, int level,
              Environment::ConfigStage targetStage) {
	QStandardItem *name = new QStandardItem(group->definition->name.c_str());

	name->setData(level, ConfigurationTreeItemModel::Level);
	name->setData(QVariant::fromValue((void*)group), ConfigurationTreeItemModel::Link);
	name->setData(ConfigurationTreeItemModel::TypeGroup, ConfigurationTreeItemModel::Type);

	name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	name->setToolTip(group->definition->description.c_str());

	item->appendRow(QStandardItemList() << name);

	for ( size_t i = 0; i < group->groups.size(); ++i ) {
		addGroup(name, group->groups[i].get(), level+1, targetStage);
	}

	for ( size_t i = 0; i < group->parameters.size(); ++i ) {
		addParameter(name, group->parameters[i].get(), level+1, targetStage);
	}

	for ( size_t i = 0; i < group->structures.size(); ++i ) {
		addStructure(name, group->structures[i].get(), level+1, targetStage);
	}

	for ( size_t i = 0; i < group->structureTypes.size(); ++i ) {
		addStructure(name, group->structureTypes[i].get(), level+1, targetStage);
	}
}


QString firstLine(const QString &input) {
	return input.mid(0, input.indexOf('\n'));
}


QString multiline(const QString &input, int lineWidth) {
	if ( input.length() <= lineWidth ) return input;

	QString txt = input;
	int s = 0;
	int to = s + lineWidth;

	while ( to < input.length() ) {
		int p = input.indexOf('\n', s);
		if ( p >= 0 && p <= to ) {
			s = p+1;
			to = s + lineWidth;
			continue;
		}

		p = input.lastIndexOf(' ', to);
		if ( p <= s ) {
			txt.insert(lineWidth, '\n');
			s = lineWidth+1;
		}
		else {
			txt[p] = '\n';
			s = p+1;
		}

		to = s + lineWidth;
	}

	return txt;
}


class StaticDialog : public QDialog {
	public:
		StaticDialog() {}

		void accept() {
			setResult(QDialog::Accepted);
			emit finished(result());
		}

		void reject() {
			setResult(QDialog::Rejected);
			emit finished(result());
		}
};


struct QtConfigDelegate : System::ConfigDelegate {
	QtConfigDelegate(QSettings *sets)
	: dialog(NULL), settings(sets), hasErrors(false)
	, needsReload(false) {}

	~QtConfigDelegate() {
		if ( dialog != NULL ) {
			settings->beginGroup("CodeView");
			settings->setValue("geometry", dialog->saveGeometry());
			settings->endGroup();
			delete dialog;
		}
	}

	void log(Config::LogLevel level, const char *filename, int line, const char *msg) {
		if ( level != Config::ERROR ) return;
		errors.append(QPair<int,QString>(line, QString(msg).trimmed()));
	}

	void finishedReading(const char *filename) {
		if ( dialog == NULL ) return;

		if ( lastErrorFile == filename ) {
			handleReadError(filename);
			lastErrorFile = QString();
		}
	}

	bool handleReadError(const char *filename) {
		lastErrorFile = filename;

		if ( dialog == NULL ) {
			dialog = new StaticDialog;
			dialog->setWindowModality(Qt::ApplicationModal);

			QVBoxLayout *l = new QVBoxLayout;
			l->setSpacing(0);
			setMargin(l, 0);

			headerLabel = new StatusLabel;
			headerLabel->setWordWrap(true);
			l->addWidget(headerLabel);

			fileWidget = new ConfigFileWidget;
			l->addWidget(fileWidget);

			dialog->resize(800,600);
			settings->beginGroup("CodeView");
			dialog->restoreGeometry(settings->value("geometry").toByteArray());
			settings->endGroup();

			QHBoxLayout *buttonLayout = new QHBoxLayout;
			buttonLayout->addStretch();
			setMargin(buttonLayout, 4);

			okButton = new QPushButton;
			cancelButton = new QPushButton;

			QObject::connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
			QObject::connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

			buttonLayout->addWidget(okButton);
			buttonLayout->addWidget(cancelButton);

			l->addLayout(buttonLayout);

			dialog->setLayout(l);
		}

		dialog->setWindowTitle(QString("CodeView %1").arg(filename));

		if ( errors.empty() ) {
			okButton->setText(QObject::tr("OK"));
			okButton->show();
			cancelButton->hide();
			headerLabel->setSuccessText(QString("%1 parsed successfully").arg(filename));
		}
		else {
			okButton->setText(QObject::tr("Reload"));
			okButton->show();
			cancelButton->setText(QObject::tr("Ignore"));
			cancelButton->show();
			headerLabel->setErrorText(QString("%1 failed to parse.\n"
			                                  "Please correct the errors "
			                                  "below and press 'Reload' to continue with a "
			                                  "correct configuration file.\n"
			                                  "'Ignore' will ignore the errors which "
			                                  "can lead to undesired behaviour.")
			                          .arg(filename));
		}

		if ( !fileWidget->loadFile(filename) )
			return false;

		fileWidget->setErrors(errors, true);

		errors.clear();

		QEventLoop eventLoop;

		if ( !dialog->isVisible() ) {
			dialog->show();
		}

		QObject::connect(dialog, &QDialog::finished, &eventLoop, &QEventLoop::quit);
		eventLoop.exec();

		int res = dialog->result();

		if ( res == QDialog::Rejected ) {
			hasErrors = true;
			return false;
		}

		fileWidget->saveFile(filename);
		return true;
	}

	void caseSensitivityConflict(const CSConflict &csc) {
		SEISCOMP_WARNING("%s: possible cs conflict: %s should be %s",
		                 csc.symbol->uri.c_str(), csc.symbol->name.c_str(),
		                 csc.parameter->variableName.c_str());
		conflicts.append(csc);
	}

	void showConflicts() {
		// No conflicts?
		if ( conflicts.empty() ) return;

		QDialog dlg;
		QVBoxLayout *l = new QVBoxLayout;
		l->setSpacing(0);
		setMargin(l, 0);
		dlg.setLayout(l);

		StatusLabel *headerLabel = new StatusLabel;
		headerLabel->setWordWrap(true);
		l->addWidget(headerLabel);

		headerLabel->setWarningText(headerLabel->tr("Found possible conflicts due to "
		                            "case sensitivity of parameter names.\n"
		                            "Files which are disabled below are not under control "
		                            "of scconfig and cannot be changed. This needs to be fixed by "
		                            "either recreating aliases or updating the default configuration files."));

		ConfigConflictWidget *w = new ConfigConflictWidget;
		w->setConflicts(conflicts);
		l->addWidget(w);

		QHBoxLayout *buttonLayout = new QHBoxLayout;
		setMargin(buttonLayout, 4);

		QPushButton *fix = new QPushButton;
		buttonLayout->addWidget(fix);
		fix->setText(fix->tr("Fix parameter(s)"));
		fix->setToolTip(fix->tr("Fixes all selected conflicts by correcting the parameter names."));

		buttonLayout->addStretch();

		QPushButton *close = new QPushButton;
		buttonLayout->addWidget(close);
		close->setText(close->tr("Close"));

		QObject::connect(fix, &QPushButton::clicked, w, &ConfigConflictWidget::fixConflicts);
		QObject::connect(close, &QPushButton::clicked, &dlg, &QDialog::accept);

		l->addLayout(buttonLayout);

		dlg.resize(800,600);
		settings->beginGroup("Conflicts");
		dlg.restoreGeometry(settings->value("geometry").toByteArray());
		settings->endGroup();

		dlg.exec();

		settings->beginGroup("Conflicts");
		settings->setValue("geometry", dlg.saveGeometry());
		settings->endGroup();
	}

	void finishedWriting(const char *filename, const ChangeList &changes) {
		// Do nothing currently. Might be used to track unchanged or changed
		// files if required.
	}

	void hasWriteError(const char *filename) {
		fileWriteErrors.append(filename);
		hasErrors = true;
	}

	bool handleWriteTimeMismatch(const char *filename, const ChangeList &changes) {
		QDialog dlg;
		QVBoxLayout *l = new QVBoxLayout;
		l->setSpacing(0);
		setMargin(l, 0);
		dlg.setLayout(l);

		StatusLabel *headerLabel = new StatusLabel;
		headerLabel->setWordWrap(true);
		l->addWidget(headerLabel);

		headerLabel->setInfoText(headerLabel->tr("%1 has changed on disk. The local changes do not reflect the changes made to the "
		                                         "file outside of scconfig. Below are the changes listed that would be applied to the "
		                                         "current file on disk. You can decide whether to apply the changes or to keep the file.\n\n"
		                                         "If you keep the file the configuration will be reloaded afterwards.").arg(filename));

		ConfigChangesWidget *w = new ConfigChangesWidget;
		w->setChanges(changes);
		l->addWidget(w);

		QHBoxLayout *buttonLayout = new QHBoxLayout;
		setMargin(buttonLayout, 4);

		QPushButton *btnReplace = new QPushButton;
		btnReplace->setText(btnReplace->tr("Apply"));
		btnReplace->setToolTip(btnReplace->tr("Apply the local changes to %1").arg(filename));

		QPushButton *btnKeep = new QPushButton;
		btnKeep->setText(btnKeep->tr("Keep"));

		buttonLayout->addWidget(btnReplace);
		buttonLayout->addStretch();
		buttonLayout->addWidget(btnKeep);

		QObject::connect(btnKeep, &QPushButton::clicked, &dlg, &QDialog::accept);
		QObject::connect(btnReplace, &QPushButton::clicked, &dlg, &QDialog::reject);

		l->addLayout(buttonLayout);

		dlg.resize(600, 400);
		settings->beginGroup("Changes");
		dlg.restoreGeometry(settings->value("geometry").toByteArray());
		settings->endGroup();

		bool keep = dlg.exec() == QDialog::Accepted;

		settings->beginGroup("Changes");
		settings->setValue("geometry", dlg.saveGeometry());
		settings->endGroup();

		if ( keep ) {
			needsReload = true;
		}

		return keep;
	}

	QDialog *dialog;
	QSettings *settings;
	StatusLabel *headerLabel;
	ConfigFileWidget *fileWidget;
	QPushButton *okButton;
	QPushButton *cancelButton;
	QString lastErrorFile;
	QList<ConfigFileWidget::Error> errors;
	QStringList fileWriteErrors;
	bool hasErrors;
	bool needsReload;
	QList<CSConflict> conflicts;
};


}


QColor blend(const QColor& c1, const QColor& c2, int percentOfC1) {
	return QColor((c1.red()*percentOfC1 + c2.red()*(100-percentOfC1)) / 100,
	              (c1.green()*percentOfC1 + c2.green()*(100-percentOfC1)) / 100,
	              (c1.blue()*percentOfC1 + c2.blue()*(100-percentOfC1)) / 100);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StatusLabel::StatusLabel(QWidget *parent) : QLabel(parent) {
	setMargin(QFontMetrics(font()).ascent() / 2);
	setInfoText("");
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	setForegroundRole(QPalette::WindowText);
	setBackgroundRole(QPalette::Window);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::setIcon(const QString &name) {
	int em = QFontMetrics(font()).height();
	_icon = icon(name, palette().color(foregroundRole())).pixmap(em, em);
	setMinimumHeight(_icon.height() + margin() * 2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::setColors(QColor color, QColor background) {
	QPalette pal = palette();
	pal.setColor(foregroundRole(), color);
	pal.setColor(backgroundRole(), background);
	setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::setInfoText(const QString &t) {
	setColors(infoForeground, infoBackground);
	setIcon("hint");
	QLabel::setText(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::setSuccessText(const QString &t) {
	setColors(successForeground, successBackground);
	setIcon("check");
	QLabel::setText(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::setWarningText(const QString &t) {
	setColors(warningForeground, warningBackground);
	setIcon("warning");
	QLabel::setText(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::setErrorText(const QString &t) {
	setColors(errorForeground, errorBackground);
	setIcon("warning");
	QLabel::setText(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StatusLabel::paintEvent(QPaintEvent *) {
	QPainter p(this);
	p.fillRect(rect(), palette().color(backgroundRole()));
	p.setPen(palette().color(foregroundRole()));

	QSize iconLayoutSize = _icon.size() / _icon.devicePixelRatioF();
	p.drawPixmap(margin(), (height() - iconLayoutSize.height()) / 2, _icon);

	if ( wordWrap() ) {
		p.drawText(
			rect().adjusted(
				margin() + iconLayoutSize.width() + margin(), margin(),
				-margin(), -margin()
			),
			alignment() | Qt::TextWordWrap, text()
		);
	}
	else {
		p.drawText(
			rect().adjusted(
				margin() + iconLayoutSize.width() + margin(), margin(),
				-margin(), -margin()
			),
			alignment(), text()
		);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfigurationTreeItemModel::ConfigurationTreeItemModel(
	QObject *parent,
	System::Model *tree,
	Environment::ConfigStage stage
) : QStandardItemModel(parent), _modified(false)
{
	if ( tree ) {
		setModel(tree, stage);
	}
	_configStage = Environment::CS_CONFIG_APP;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfigurationTreeItemModel::setModel(System::Model *tree,
                                          Environment::ConfigStage stage) {
	_model = tree;
	_configStage = stage;

	clear();
	setColumnCount(4);
	setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Value" << "Locked");

	QStandardItem *root = invisibleRootItem();

	for ( size_t i = 0; i < tree->modules.size(); ++i ) {
		Module *mod = tree->modules[i].get();
		QStandardItem *modItem = new QStandardItem(mod->definition->name.c_str());
		modItem->setData(0, Level);
		modItem->setData(QVariant::fromValue((void*)mod), Link);
		modItem->setData(TypeModule, Type);
		modItem->setColumnCount(columnCount());
		modItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		root->appendRow(modItem);

		for ( size_t j = 0; j < mod->sections.size(); ++j ) {
			Section *sec = mod->sections[j].get();
			QStandardItem *secItem = new QStandardItem(sec->name.c_str());
			secItem->setData(1, Level);
			secItem->setData(QVariant::fromValue((void*)sec), Link);
			secItem->setData(TypeSection, Type);
			secItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			modItem->appendRow(secItem);

			for ( size_t g = 0; g < sec->groups.size(); ++g ) {
				addGroup(secItem, sec->groups[g].get(), 2, _configStage);
			}

			for ( size_t p = 0; p < sec->parameters.size(); ++p ) {
				addParameter(secItem, sec->parameters[p].get(), 2, _configStage);
			}

			for ( size_t s = 0; s < sec->structureTypes.size(); ++s ) {
				addStructure(secItem, sec->structureTypes[s].get(), 2, _configStage);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfigurationTreeItemModel::setModel(System::ModuleBinding *b) {
	_model = nullptr;
	// The stage for the binding is always config app
	_configStage = Environment::CS_CONFIG_APP;

	clear();
	setColumnCount(4);
	setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Value" << "Locked");

	QStandardItem *root = invisibleRootItem();

	Module *mod = static_cast<Module*>(b->parent);

	QStandardItem *bindItem = new QStandardItem(mod->definition->name.c_str());
	bindItem->setData(0, Level);
	bindItem->setData(QVariant::fromValue((void*)b), Link);
	bindItem->setData(TypeBinding, Type);
	bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	root->appendRow(bindItem);

	QStandardItem *secItem;

	for ( size_t s = 0; s < b->sections.size(); ++s ) {
		Section *sec = b->sections[s].get();

		secItem = new QStandardItem(sec->name.c_str());
		secItem->setData(1, Level);
		secItem->setData(QVariant::fromValue((void*)sec), Link);
		secItem->setData(TypeSection, Type);
		secItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		bindItem->appendRow(secItem);

		for ( size_t g = 0; g < sec->groups.size(); ++g ) {
			addGroup(secItem, sec->groups[g].get(), 2, _configStage);
		}

		for ( size_t i = 0; i < sec->structures.size(); ++i ) {
			addStructure(secItem, sec->structures[i].get(), 2, _configStage);
		}

		for ( size_t i = 0; i < sec->structureTypes.size(); ++i ) {
			addStructure(secItem, sec->structureTypes[i].get(), 2, _configStage);
		}

		for ( size_t p = 0; p < sec->parameters.size(); ++p ) {
			addParameter(secItem, sec->parameters[p].get(), 2, _configStage);
		}
	}

	for ( size_t c = 0; c < b->categories.size(); ++c ) {
		BindingCategory *cat = b->categories[c].get();
		QStandardItem *catItem = new QStandardItem(cat->name.c_str());
		catItem->setData(1, Level);
		catItem->setData(QVariant::fromValue((void*)cat), Link);
		catItem->setData(TypeCategory, Type);
		catItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		catItem->setColumnCount(columnCount());
		bindItem->appendRow(catItem);

		for ( size_t s = 0; s < cat->bindings.size(); ++s ) {
			Binding *catBinding = cat->bindings[s].binding.get();

			secItem = new QStandardItem(cat->bindings[s].alias.c_str());
			secItem->setData(2, Level);
			secItem->setData(QVariant::fromValue((void*)catBinding), Link);
			secItem->setData(TypeCategoryBinding, Type);
			secItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			catItem->appendRow(secItem);

			for ( size_t i = 0; i < catBinding->sections.size(); ++i ) {
				Section *sec = catBinding->sections[i].get();

				for ( size_t g = 0; g < sec->groups.size(); ++g ) {
					addGroup(secItem, sec->groups[g].get(), 3, _configStage);
				}

				for ( size_t p = 0; p < sec->parameters.size(); ++p ) {
					addParameter(secItem, sec->parameters[p].get(), 3, _configStage);
				}

				for ( size_t i = 0; i < sec->structures.size(); ++i ) {
					addStructure(secItem, sec->structures[i].get(), 3, _configStage);
				}

				for ( size_t i = 0; i < sec->structureTypes.size(); ++i ) {
					addStructure(secItem, sec->structureTypes[i].get(), 3, _configStage);
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfigurationTreeItemModel::setModified(bool m) {
	bool changed = _modified != m;
	_modified = m;
	if ( changed ) {
		emit modificationChanged(_modified);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfigurationTreeItemModel::saved() {
	int rows = invisibleRootItem()->rowCount();
	for ( int i = 0; i < rows; ++i ) {
		reset(index(i, 0));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfigurationTreeItemModel::reset(QModelIndex idx) {
	auto type = idx.sibling(idx.row(), NAME).data(ConfigurationTreeItemModel::Type).toInt();

	if ( type == ConfigurationTreeItemModel::TypeParameter ) {
		auto param = reinterpret_cast<Parameter*>(
			idx.sibling(idx.row(), NAME).data(ConfigurationTreeItemModel::Link).value<void*>()
		);

		// Reset parameter
		auto value = idx.sibling(idx.row(), VALUE);
		auto locked = idx.sibling(idx.row(), LOCKED);

		if ( param->initial[_configStage] &&
		     param->initial[_configStage]->symbol.stage != Environment::CS_UNDEFINED ) {
			setData(value, QString(param->initial[_configStage]->symbol.content.data()), ConfigurationTreeItemModel::Initial);
			setData(locked, false, ConfigurationTreeItemModel::Initial);
		}
		else {
			setData(value, QVariant(), ConfigurationTreeItemModel::Initial);
			setData(locked, true, ConfigurationTreeItemModel::Initial);
		}

		if ( param->symbols[_configStage] &&
			 param->symbols[_configStage]->symbol.stage != Environment::CS_UNDEFINED ) {
			setData(locked, false, Qt::DisplayRole);
		}
		else {
			setData(locked, true, Qt::DisplayRole);
		}

		return;
	}

	const int rows = idx.model()->rowCount(idx);
	for ( int i = 0; i < rows; ++i ) {
		reset(index(i, NAME, idx));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Environment::ConfigStage ConfigurationTreeItemModel::configStage() const {
	return _configStage;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConfigurationTreeItemModel::setData(const QModelIndex &idx, const QVariant &v, int role) {
	int type = idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Type).toInt();

	// Only parameters or none types are allowed to be changed for now
	if ( type != TypeParameter && type != TypeNone ) {
		return false;
	}

	if ( type == TypeParameter ) {
		if ( role != Qt::EditRole ) {
			return QStandardItemModel::setData(idx, v, role);
		}

		Parameter *param = reinterpret_cast<Parameter*>(
			idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Link).value<void*>()
		);

		switch ( idx.column() ) {
			case VALUE:
				param->symbols[_configStage]->symbol.content = v.toString().toStdString();
				// Reset URI to show that this value has not yet been written
				// to the configuration file
				param->symbols[_configStage]->symbol.uri = "";
				param->updateFinalValue();
				updateDerivedParameters(QModelIndex(), param, param->symbols[_configStage].get());
				setModified();
				return QStandardItemModel::setData(idx, paramValue(param, _configStage), role);
			case LOCKED:
				if ( v.toBool() ) {
					param->symbols[_configStage]->symbol.stage = Environment::CS_UNDEFINED;
				}
				else {
					param->symbols[_configStage]->symbol.content = param->symbol.content;
					param->symbols[_configStage]->symbol.stage = _configStage;
				}

				param->updateFinalValue();
				updateDerivedParameters(QModelIndex(), param, param->symbols[_configStage].get());
				QStandardItemModel::setData(idx.sibling(idx.row(),VALUE), paramValue(param, _configStage));
				break;
			default:
				return false;
		}
	}
	else if ( type == TypeNone ) {
		if ( role == Type && v.toInt() == TypeStruct ) {
			// Load the struct tree here
			QStandardItem *structItem = itemFromIndex(idx);
			Structure *struc = reinterpret_cast<Structure*>(idx.data(Link).value<void*>());

			QStandardItem *modItem = structItem->parent();
			while ( modItem &&
			        modItem->data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeModule )
				modItem = modItem->parent();

			if ( modItem ) {
				Module *mod = (Module*)modItem->data(ConfigurationTreeItemModel::Link).value<void*>();
				_model->update(mod, struc);
			}

			loadStructure(structItem, struc, idx.data(Level).toInt(), _configStage);
		}
		else if ( role == Type && v.toInt() == TypeCategoryBinding ) {
			// Load the binding tree here
			QStandardItem *bindingItem = itemFromIndex(idx);
			Binding *binding = reinterpret_cast<Binding*>(idx.data(Link).value<void*>());
			BindingCategory *cat = reinterpret_cast<BindingCategory*>(binding->parent);
			Module *mod = (Module*)cat->parent->parent;
			mod->model->updateBinding((ModuleBinding*)cat->parent, binding);

			for ( size_t s = 0; s < binding->sections.size(); ++ s ) {
				Section *sec = binding->sections[s].get();

				for ( size_t g = 0; g < sec->groups.size(); ++g )
					addGroup(bindingItem, sec->groups[g].get(), idx.data(Level).toInt(), _configStage);

				for ( size_t p = 0; p < sec->parameters.size(); ++p )
					addParameter(bindingItem, sec->parameters[p].get(), idx.data(Level).toInt(), _configStage);

				for ( size_t i = 0; i < sec->structures.size(); ++i )
					addStructure(bindingItem, sec->structures[i].get(), idx.data(Level).toInt(), _configStage);

				for ( size_t i = 0; i < sec->structureTypes.size(); ++i )
					addStructure(bindingItem, sec->structureTypes[i].get(), idx.data(Level).toInt(), _configStage);
			}
		}
	}

	setModified();
	return QStandardItemModel::setData(idx, v, role);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::ItemFlags ConfigurationTreeItemModel::flags(const QModelIndex &idx) const {
	int type = idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Type).toInt();
	if ( type != ConfigurationTreeItemModel::TypeParameter)
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	Qt::ItemFlags flags = Qt::ItemIsSelectable;

	switch ( idx.column() ) {
		case NAME:
		case TYPE:
			flags |= Qt::ItemIsEnabled;
			break;
		case VALUE:
			flags |= Qt::ItemIsEditable;
			if ( !idx.sibling(idx.row(),LOCKED).data().toBool() ) flags |= Qt::ItemIsEnabled;
			break;
		case LOCKED:
			flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
			break;
		default:
			break;
	}

	return flags;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfigurationTreeItemModel::updateDerivedParameters(
	const QModelIndex &idx,
	System::Parameter *super,
	System::SymbolMapItem *item
)
{
	if ( idx.isValid() ) {
		int type = idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Type).toInt();

		if ( type == ConfigurationTreeItemModel::TypeParameter ) {
			Parameter *param = reinterpret_cast<Parameter*>(
				idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Link).value<void*>()
			);

			if ( param != super ) {
				for ( int i = 0; i < Environment::CS_QUANTITY; ++i ) {
					if ( param->symbols[i] == item) {
						if ( i == _configStage ) {
							bool locked = item->symbol.stage == Environment::CS_UNDEFINED;
							if ( idx.sibling(idx.row(),LOCKED).data().toBool() != locked )
								QStandardItemModel::setData(idx.sibling(idx.row(),LOCKED), locked);
						}

						param->updateFinalValue();
						QStandardItemModel::setData(idx.sibling(idx.row(),VALUE), paramValue(param, _configStage));

						break;
					}
				}
			}

			/*
			if ( param->inherts(super) ) {
				param->updateFinalValue();
				QStandardItemModel::setData(idx.sibling(idx.row(),VALUE), paramValue(param));
			}
			*/
		}
	}

	int rows = rowCount(idx);
	for ( int i = 0; i < rows; ++i ) {
		updateDerivedParameters(index(i,NAME,idx), super, item);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfiguratorPanel::setHeadline(QString head, QString sub) {
	_headline = head;
	_subHeadline = sub;
	emit headlineChanged(_headline, _subHeadline);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfiguratorPanel::setModel(ConfigurationTreeItemModel *model) {
	_model = model;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfiguratorPanel::saved() {
	//
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConfiguratorPanel::setDescription(QString desc) {
	_description = desc;
	emit descriptionChanged(_description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Configurator::Configurator(Environment::ConfigStage stage, QWidget *parent)
: QMainWindow(parent), _settings("gempa", "scconfig") {
	setObjectName("Configurator");
	applyTheme();

	_configurationStage = stage;

	_proxy = nullptr;
	_model = new ConfigurationTreeItemModel(this);

	setBackgroundRole(globalGridColor);
	setAutoFillBackground(true);

	_toolBar = addToolBar("Main buttons");
	_toolBar->setObjectName("ToolBar");
	_toolBar->setMovable(false);
	_toolBar->setFloatable(false);
	_toolBar->setAllowedAreas(Qt::TopToolBarArea);
	_toolBar->setIconSize(globalIconSize);
	_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	addToolBar(_toolBar);

	auto a = _toolBar->addAction(tr("Save"));
	a->setIcon(::icon("save"));
	a->setShortcut(QKeySequence("Ctrl+S"));
	a->setToolTip(tr("Save module and bindings configuration - %1").arg(a->shortcut().toString()));
	connect(a, &QAction::triggered, this, &Configurator::save);

	a = _toolBar->addAction(tr("Reset all"));
	a->setIcon(::icon("refresh"));
	a->setShortcut(QKeySequence("Ctrl+R"));
	a->setToolTip(tr("Reset/reload all module and binding configurations - %1").arg(a->shortcut().toString()));
	connect(a, &QAction::triggered, this, &Configurator::resetAll);

	a = _toolBar->addAction(tr("Help"));
	a->setIcon(::icon("menu_scconfig_docs"));
	a->setShortcut(QKeySequence::HelpContents);
	a->setToolTip(tr("Documentation for scconfig - %1").arg(a->shortcut().toString()));
	connect(a, &QAction::triggered, this, &Configurator::showHelp);

	a = _toolBar->addAction(tr("&About"));
	a->setIcon(::icon("info"));
	a->setToolTip(tr("About scconfig"));
	connect(a, &QAction::triggered, this, &Configurator::showAbout);

	auto spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	_toolBar->addWidget(spacer);

	a = _toolBar->addAction(tr("Wizard"));
	a->setIcon(::icon("wizard_hat"));
	a->setShortcut(QKeySequence("Ctrl+N"));
	a->setToolTip(tr("Run configuration wizard - %1").arg(a->shortcut().toString()));
	connect(a, &QAction::triggered, this, &Configurator::wizard);

	a = _toolBar->addAction(tr("Switch mode"));
	a->setIcon(::icon("mode_switch"));
	a->setToolTip(tr("Switch module configuration between system and user mode"));
	connect(a, &QAction::triggered, this, &Configurator::switchMode);

	a = _toolBar->addAction(tr("Quit"));
	a->setIcon(::icon("quit"));
	a->setShortcut(QKeySequence("Ctrl+Q"));
	a->setToolTip(tr("Close scconfig - %1").arg(a->shortcut().toString()));
	connect(a, &QAction::triggered, this, &Configurator::close);

	setWindowIcon(QIcon(":/scconfig/icons/seiscomp.svg"));

	auto centralLayout = new QGridLayout;
	centralLayout->setSpacing(1);
	setMargin(centralLayout, 0);

	auto centralWidget = new QWidget;
	centralWidget->setBackgroundRole(QPalette::Window);
	centralWidget->setLayout(centralLayout);

	setCentralWidget(centralWidget);

	QFont h1 = font();
	h1.setPointSize(h1.pointSize() * 2);
	h1.setBold(true);

	_modeLabel = new MouseTrackLabel;
	_modeLabel->setAutoFillBackground(true);
	centralLayout->addWidget(_modeLabel, 0, 0);

	auto infoWidget = new QWidget;
	infoWidget->setAutoFillBackground(true);
	infoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(infoWidget, &QWidget::customContextMenuRequested, this, [this, infoWidget](const QPoint &p) {
		auto menu = createPopupMenu();
		menu->exec(infoWidget->mapToGlobal(p));
		delete menu;
	});

	_headline = new HeadlineLabel;
	_headline->setForegroundRole(QPalette::Dark);
	_headline->setFont(h1);
	_headline->setMargin(fontMetrics().ascent());

	_description = new InfoLabel;
	_description->setForegroundRole(QPalette::Dark);
	_description->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));

	QVBoxLayout *vl = new QVBoxLayout;
	vl->addWidget(_headline);
	vl->addWidget(_description);
	vl->setContentsMargins(0, 0, 0, fontMetrics().ascent());
	vl->setSpacing(0);
	infoWidget->setLayout(vl);
	infoWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	centralLayout->addWidget(infoWidget, 0, 1);

	_listWidget = new QListWidget;
	_listWidget->setForegroundRole(QPalette::Dark);
	_listWidget->setFixedWidth(globalListWidth);
	_listWidget->setFrameShape(QFrame::NoFrame);
	_listWidget->setIconSize(QSize(48, 48));
	_listWidget->viewport()->installEventFilter(new ModuleListPainter(this));
	_listWidget->setItemDelegate(new ModuleListDelegate(this));
	_listWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

	centralLayout->addWidget(_listWidget, 1, 0);

	QList<ConfiguratorPanel*> panels;
	panels.append(new SystemPanel);
	panels.append(new ModulesPanel);
	panels.append(new BindingsPanel);
	panels.append(new InventoryPanel);
	panels.append(new InformationPanel);
	panels.append(new HelpPanel);

	_statusLabel = new StatusLabel;
	_statusLabel->hide();
	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->setSpacing(0);
	vlayout->addWidget(_statusLabel);

	foreach ( auto *panel, panels) {
		auto action = new QAction(panel->title());
		auto row = _listWidget->count();
		action->setShortcut(QKeySequence(Qt::Key_1 + row));
		connect(action, &QAction::triggered, _listWidget, [this, row] {
			_listWidget->setCurrentRow(row);
		});
		addAction(action);

		Panel p;
		p.second = panel;
		p.first = new QListWidgetItem(panel->title(), _listWidget);
		vlayout->addWidget(panel);
		panel->hide();
		p.first->setData(Qt::DecorationRole, panel->icon());
		p.first->setData(Qt::UserRole, QVariant::fromValue((void*)panel));
		_panels.append(p);

		connect(panel, &ConfiguratorPanel::reloadRequested, this, &Configurator::resetAll);

		auto requiredWidth = QT_FM_WIDTH(_listWidget->fontMetrics(), panel->title());
		if ( requiredWidth > _listWidget->width() ) {
			_listWidget->setFixedWidth(requiredWidth + _listWidget->fontMetrics().averageCharWidth());
		}
	}

	centralLayout->addLayout(vlayout, 1, 1);

	connect(_listWidget, &QListWidget::currentItemChanged, this, &Configurator::sectionChanged);

	QAction *whatsThis = QWhatsThis::createAction(this);
	whatsThis->setShortcut(QKeySequence::WhatsThis);
	addAction(whatsThis);

	_firstShow = true;

	connect(&_statusTimer, &QTimer::timeout, this, &Configurator::statusTimer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Configurator::~Configurator() {
	_settings.beginGroup(objectName());
	_settings.setValue("geometry", saveGeometry());
	_settings.setValue("state", saveState());
	_settings.endGroup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::updateModeLabel() {
	const auto version = Core::Version(SEISCOMP_VERSION).toString();
	switch ( _configurationStage ) {
		case Environment::CS_USER_APP:
			setWindowTitle(tr("SeisComP %1 - user configuration [ %2 ]")
			               .arg(version.c_str(), Environment::Instance()->configDir().c_str()));
			static_cast<MouseTrackLabel*>(_modeLabel)->setIcon(
				svg(multiavatar().data()), QSize(72, 72)
			);
			static_cast<MouseTrackLabel*>(_modeLabel)->setAnnotationIcon(
				QIcon(":/scconfig/icons/mode_user_attention.svg")
			);
			break;
		case Environment::CS_CONFIG_APP:
			setWindowTitle(tr("SeisComP %1 - system configuration [ %2 ]")
			               .arg(version.c_str(), Environment::Instance()->appConfigDir().c_str()));
			{
				QIcon icon = ::icon("mode_system");
				static_cast<MouseTrackLabel*>(_modeLabel)->setIcon(icon, QSize(72, 72));
			}
			break;
		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Configurator::setModel(System::Model *model) {
	QtConfigDelegate cd(&_settings);
	model->readConfig(Environment::CS_USER_APP, &cd);
	cd.showConflicts();

	if ( _configurationStage == Environment::CS_UNDEFINED ) {
		StageSelectionDialog dlg(this);
		if ( dlg.exec() == QDialog::Accepted ) {
			switch ( dlg.mode() ) {
				case StageSelectionDialog::User:
					_configurationStage = Environment::CS_USER_APP;
					break;
				case StageSelectionDialog::System:
					_configurationStage = Environment::CS_CONFIG_APP;
					break;
			}
		}
		else {
			return false;
		}
	}

	if ( cd.hasErrors ) {
		showWarningMessage("Configuration loaded with errors");
	}

	updateModeLabel();

	_model->setModel(model, _configurationStage);

	foreach ( Panel p, _panels ) {
		p.second->setModel(_model);
	}

	_listWidget->setCurrentRow(0);

	//_treeView->hideColumn(1);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::applyTheme() {
	auto p = qApp->palette();
	p.setColor(QPalette::Dark, blend(p.color(QPalette::Base), p.color(QPalette::Text), 25));
	p.setColor(QPalette::Mid, blend(p.color(QPalette::Base), p.color(QPalette::Text), 75));
	qApp->setPalette(p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::changeEvent(QEvent *event) {
	if ( event->type() == QEvent::ThemeChange ) {
		applyTheme();
	}
	QMainWindow::changeEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::showEvent(QShowEvent *event) {
	if ( _firstShow ) {
		_settings.beginGroup(objectName());
		restoreGeometry(_settings.value("geometry").toByteArray());
		restoreState(_settings.value("state").toByteArray());
		_settings.endGroup();

		_firstShow = false;

		// Check lock file
		Environment *env = Environment::Instance();

		QString statDir = QDir::toNativeSeparators(
			(env->installDir() + "/var/run").c_str());

		QString statFile = QDir::toNativeSeparators(
			statDir + "/seiscomp.init");

		if ( QFile::exists(statFile) ) return;

		// Create state file
		QDir dir(statDir);
		dir.mkpath(".");
		QFile f(statFile);
		f.open(QIODevice::WriteOnly | QIODevice::Truncate);
		f.close();

		if ( QMessageBox::question(this, tr("First start"),
		                           tr("This seems to be the first start of the SeisComP configurator.\n"
		                              "Do you want to run the initial setup?\n"
		                              "Hint: You can say no here and start the wizard at any "
		                              "time with Ctrl+N."),
		                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No )
			return;

		wizard();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::closeEvent(QCloseEvent *event) {
	if ( _model->isModified() ) {
		auto r = QMessageBox::question(
			this, "Configuration changed",
			"The configuration is modified. Do you want to save it?",
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
		);
		if ( r == QMessageBox::Yes ) {
			save();
		}
		else if ( r == QMessageBox::Cancel ) {
			event->ignore();
			return;
		}
	}

	QMainWindow::closeEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::panelHeadlineChanged(const QString &text, const QString &subText) {
	static_cast<HeadlineLabel*>(_headline)->setText(text, subText);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::panelDescriptionChanged(const QString &text) {
	_description->setText(firstLine(text));
	_description->setToolTip(multiline(text, 80));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::switchToSystemMode() {
	if ( _configurationStage == Environment::CS_CONFIG_APP ) {
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	_configurationStage = Environment::CS_CONFIG_APP;
	_model->setModel(_model->model(), _configurationStage);

	foreach ( Panel p, _panels ) {
		p.second->setModel(_model);
	}

	updateModeLabel();

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::switchToUserMode() {
	if ( _configurationStage == Environment::CS_USER_APP ) {
		return;
	}

	auto r = QMessageBox::question(
		this, tr("Switch to user mode"),
		tr(
			"You are switching configuration from system to user mode!\n"
			"All module parameters will be written to configuration files in ~/.seiscomp "
			"overriding configurations made in system mode. They can later only be "
			"changed or removed in user mode or by editing the files in ~/.seiscomp.\n"
			"\n"
			"Would you like to switch to user mode?"
		)
	);

	if ( r != QMessageBox::Yes ) {
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	_configurationStage = Environment::CS_USER_APP;
	_model->setModel(_model->model(), _configurationStage);

	foreach ( Panel p, _panels ) {
		p.second->setModel(_model);
	}

	updateModeLabel();

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::showStatusMessage(const QString &msg) {
	_statusTimer.stop();
	_statusLabel->setSuccessText(msg);
	_statusLabel->show();
	_statusTimer.setSingleShot(true);
	_statusTimer.start(3000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::showWarningMessage(const QString &msg) {
	_statusTimer.stop();
	_statusLabel->setWarningText(msg);
	_statusLabel->show();
	_statusTimer.setSingleShot(true);
	_statusTimer.start(3000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::switchMode() {
	if ( _configurationStage == Environment::CS_CONFIG_APP ) {
		switchToUserMode();
	}
	else {
		switchToSystemMode();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::statusTimer() {
	if ( _statusTimer.isSingleShot() ) {
		_statusLabel->setInfoText("");
		_statusLabel->hide();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::wizard() {
	WizardModel wizard;

	SchemaDefinitions *schema = _model->model()->schema;
	for ( size_t i = 0; i < schema->moduleCount(); ++i ) {
		SchemaModule *mod = schema->module(i);
		SchemaSetup *setup = mod->setup.get();
		if ( setup != NULL ) {
			for ( size_t g = 0; g < setup->groups.size(); ++g )
				wizard[mod->name].push_back(setup->groups[g].get());
		}

		SchemaDefinitions::PluginList plugins;
		plugins = schema->pluginsForModule(mod->name);

		for ( size_t p = 0; p < plugins.size(); ++p ) {
			SchemaPlugin *plugin = plugins[p];
			setup = plugin->setup.get();
			if ( setup != NULL ) {
				for ( size_t g = 0; g < setup->groups.size(); ++g )
					wizard[mod->name].push_back(setup->groups[g].get());
			}
		}
	}

	if ( wizard.empty() ) {
		return;
	}

	WizardWidget w(&wizard);
	w.exec();

	if ( w.ranSetup() ) {
		QApplication::setOverrideCursor(Qt::WaitCursor);

		QtConfigDelegate cd(&_settings);
		_model->model()->readConfig(Environment::CS_USER_APP, &cd);
		_model->setModel(_model->model(), _configurationStage);

		foreach ( Panel p, _panels ) {
			p.second->setModel(_model);
		}

		QApplication::restoreOverrideCursor();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::resetAll() {
	bool errors = false;

	if ( _model->model() ) {
		_model->model()->schema->reload();
		_model->model()->recreate();

		QtConfigDelegate cd(&_settings);
		_model->model()->readConfig(Environment::CS_USER_APP, &cd);
		_model->setModel(_model->model(), _configurationStage);
		_model->setModified(false);
		errors = cd.hasErrors;
		cd.showConflicts();
	}

	foreach ( Panel p, _panels ) {
		p.second->setModel(_model);
	}

	if ( errors ) {
		showWarningMessage("Configuration reset got errors");
	}
	else {
		showStatusMessage("Configuration reset successfully");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::showAbout() {
	auto *w = new AboutWidget(nullptr);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setWindowModality(Qt::ApplicationModal);
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::showHelp() {
	QString indexFile = QString("%1/doc/seiscomp/html/apps/scconfig.html")
	                    .arg(Environment::Instance()->shareDir().c_str());

	if ( !QFile::exists(indexFile) ) {
		QMessageBox::information(nullptr, QString("scconfig help"),
		                         tr("Help for scconfig is not available."));
		return;
	}

	QDesktopServices::openUrl(QString("file://%1").arg(indexFile));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::save() {
	QtConfigDelegate cd(&_settings);
	_model->model()->writeConfig(true, _configurationStage, &cd);
	_model->setModified(false);
	_model->saved();
	foreach ( Panel p, _panels ) {
		p.second->saved();
	}

	if ( cd.hasErrors ) {
		showWarningMessage(QString("Failed to write configuration for %1 files\n%2")
		                   .arg(cd.fileWriteErrors.count()).arg(cd.fileWriteErrors.join("\n")));
	}
	else {
		showStatusMessage("Configuration saved");
	}

	if ( cd.needsReload ) {
		resetAll();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Configurator::sectionChanged(QListWidgetItem *curr, QListWidgetItem *prev) {
	ConfiguratorPanel *pw = prev?(ConfiguratorPanel*)prev->data(Qt::UserRole).value<void*>():nullptr;
	ConfiguratorPanel *cw = curr?(ConfiguratorPanel*)curr->data(Qt::UserRole).value<void*>():nullptr;

	// Do we switch to a panel that depends on the configuration on disk?
	// We should then ask the user to flush internal modifications.
	if ( cw && cw->isExternalConfigurationUsed() && _model->isModified() ) {
		if ( QMessageBox::question(
		       this, "Configuration changed",
		       "The configuration has changed and the current panel depends "
		       "on it. You must save the configuration to ensure the correct "
		       "behaviour.\nSave?",
		       QMessageBox::Yes, QMessageBox::No
		     ) == QMessageBox::Yes )
			save();
	}

	if ( pw ) {
		pw->hide();
		pw->disconnect(this);
	}

	if ( cw ) {
		cw->show();
		connect(cw, &ConfiguratorPanel::headlineChanged, this, &Configurator::panelHeadlineChanged);
		connect(cw, &ConfiguratorPanel::descriptionChanged, this, &Configurator::panelDescriptionChanged);
		connect(cw, &ConfiguratorPanel::reloadRequested, this, &Configurator::resetAll);
		cw->activated();
	}

	if ( curr ) {
		panelHeadlineChanged(cw->headline(), cw->subHeadline());
		panelDescriptionChanged(cw->description());
	}
	else {
		static_cast<HeadlineLabel*>(_headline)->setText({}, {});
		_description->setText("");
		_description->setToolTip("");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
