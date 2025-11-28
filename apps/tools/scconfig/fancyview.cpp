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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

#include <QScrollBar>
#include <QResizeEvent>

#include <seiscomp/system/environment.h>
#include <seiscomp/config/config.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/gui/core/flowlayout.h>

#include "fancyview.h"
#include "gui.h"
#include "icon.h"


using namespace std;

using namespace Seiscomp;
using namespace Seiscomp::System;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Q_DECLARE_METATYPE(FancyViewItem)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


QColor CategoryTextColor(255, 255, 255);
QColor CategoryBgColor(38, 80, 128);
QColor AlertColor(0xfa, 0x9f, 0x8c);
QColor AlertFrameColor(0xb2, 0x6f, 0x65);
QColor AlertTextColor(0, 0, 0);


int layoutPadding() {
	return QApplication::fontMetrics().ascent() / 2;
}


QSize qSmartMinSize(const QSize &sizeHint, const QSize &minSizeHint,
                    const QSize &minSize, const QSize &maxSize,
                    const QSizePolicy &sizePolicy) {
	QSize s(0, 0);

	if (sizePolicy.horizontalPolicy() != QSizePolicy::Ignored) {
		if (sizePolicy.horizontalPolicy() & QSizePolicy::ShrinkFlag)
			s.setWidth(minSizeHint.width());
		else
			s.setWidth(qMax(sizeHint.width(), minSizeHint.width()));
	}

	if (sizePolicy.verticalPolicy() != QSizePolicy::Ignored) {
		if (sizePolicy.verticalPolicy() & QSizePolicy::ShrinkFlag) {
			s.setHeight(minSizeHint.height());
		} else {
			s.setHeight(qMax(sizeHint.height(), minSizeHint.height()));
		}
	}

	s = s.boundedTo(maxSize);
	if (minSize.width() > 0)
		s.setWidth(minSize.width());
	if (minSize.height() > 0)
		s.setHeight(minSize.height());

	return s.expandedTo(QSize(0,0));
}


QSize qSmartMinSize(const QWidget *w) {
	return qSmartMinSize(w->sizeHint(), w->minimumSizeHint(),
	                     w->minimumSize(), w->maximumSize(),
	                     w->sizePolicy());
}


string maxSize(const string &text, size_t maxWidth) {
	if ( text.size() <= maxWidth ) return text;
	size_t pos = text.find_last_of(' ', maxWidth-4);

	if ( pos != string::npos )
		return text.substr(0, pos) + " ...";

	return text.substr(0, maxWidth-4) + " ...";
}


string string2Block(const string &input, size_t lineWidth) {
	string txt = input;
	size_t s = 0;
	size_t to = s + lineWidth;

	while ( to < txt.length() ) {
		// find linebreaks and comment each new line
		size_t p = txt.find_first_of('\n', s);
		if ( p != string::npos && (p - s) < lineWidth) {
			s = p + 1;
		}
		else {
			// insert line break if possible at last space else inside word
			// without hyphenation
			p = txt.find_last_of(' ', to-1);
			if ( p == string::npos || p < s || (p -s) > lineWidth) {
				txt.insert(to, "\n");
				s = to + 1;
			}
			else {
				txt[p] = '\n';
				s = p+1;
			}
		}

		to = s + lineWidth;
	}

	// comment line breaks in last line
	while ( s < txt.length() ) {
		size_t p = txt.find_first_of('\n', s);
		if ( p == string::npos ) break;
		s = p+1;
	}

	return txt;
}


QString encodeHTML(const QString &input) {
	QString rich;
	const int len = input.length();
	rich.reserve(int(len * 1.1));

	for ( int i = 0; i < len; ++i ) {
		if ( input.at(i) == QLatin1Char('<') ) {
			rich += QLatin1String("&lt;");
		}
		else if ( input.at(i) == QLatin1Char('>') ) {
			rich += QLatin1String("&gt;");
		}
		else if ( input.at(i) == QLatin1Char('&') ) {
			rich += QLatin1String("&amp;");
		}
		else if ( input.at(i) == QLatin1Char('"') ) {
			rich += QLatin1String("&quot;");
		}
		else if ( input.at(i) == QLatin1Char('\n') ) {
			rich += QLatin1String("<br/>");
		}
		else if ( input.at(i) == QLatin1Char(' ') ) {
			rich += QLatin1String("&nbsp;");
		}
		else {
			rich += input.at(i);
		}
	}

	rich.squeeze();
	return rich;
}


class NewStructDialog : public QDialog {
	public:
		NewStructDialog(const Container *c, QWidget *parent = 0)
			: QDialog(parent), _container(c) {
			QVBoxLayout *layout = new QVBoxLayout;
			setLayout(layout);

			QHBoxLayout *hlayout = new QHBoxLayout;
			QLabel *label = new QLabel("Name:");
			hlayout->addWidget(label);
			_name = new QLineEdit;
			QRegularExpression rx("[A-Za-z0-9_\\(){}-]+");
			_name->setValidator(new QRegularExpressionValidator(rx,0));
			hlayout->addWidget(_name);
			layout->addLayout(hlayout);

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			QPushButton *ok = new QPushButton("Ok");
			hlayout->addWidget(ok);
			QPushButton *cancel = new QPushButton("Cancel");
			hlayout->addWidget(cancel);

			layout->addLayout(hlayout);

			connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}

		QString name() const {
			return _name->text();
		}

		void accept() {
			if ( _name->text().isEmpty() ) {
				QMessageBox::critical(NULL, "Empty name",
				                      "Empty names are not allowed. ");
				return;
			}

			if ( _container->hasStructure(qPrintable(_name->text())) ) {
				QMessageBox::critical(NULL, "Duplicate name",
				                      "The name exists already and duplicate "
				                      "names are not allowed.");
				return;
			}

			QDialog::accept();
		}

	private:
		const Container *_container;
		QLineEdit       *_name;
};


class NewCatBindingDialog : public QDialog {
	public:
		NewCatBindingDialog(const BindingCategory *c,
		                    const std::string &type,
		                    QWidget *parent = 0)
		    : QDialog(parent), _cat(c), _type(type) {
			setWindowTitle(tr("New %1 binding name").arg(c->name.data()));
			QVBoxLayout *layout = new QVBoxLayout;
			setLayout(layout);

			QHBoxLayout *hlayout = new QHBoxLayout;
			QLabel *label = new QLabel("Name:");
			hlayout->addWidget(label);
			_name = new QLineEdit;
			QRegularExpression rx("[A-Za-z0-9_\\(){}-]+");
			_name->setValidator(new QRegularExpressionValidator(rx,0));
			hlayout->addWidget(_name);
			layout->addLayout(hlayout);

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			QPushButton *ok = new QPushButton("Ok");
			hlayout->addWidget(ok);
			QPushButton *cancel = new QPushButton("Cancel");
			hlayout->addWidget(cancel);

			layout->addLayout(hlayout);

			connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}

		std::string name() const {
			return _name->text().toStdString();
		}

		void accept() {
			std::string alias;
			if ( _name->text().isEmpty() )
				alias = _type;
			else
				alias = _name->text().toStdString();

			if ( _cat->hasBinding(alias.c_str()) ) {
				QMessageBox::critical(NULL, "Duplicate alias",
				                      "The alias exists already and duplicate "
				                      "aliases are not allowed.");
				return;
			}

			QDialog::accept();
		}

	private:
		const BindingCategory *_cat;
		std::string            _type;
		QLineEdit             *_name;
};


class BlockWidget : public QWidget {
	public:
		BlockWidget(QWidget *parent = 0) : QWidget(parent) {
			_hasCustomBackground = false;
			setContentsMargins(layoutPadding() * 3, 0, 0, 0);
		}

		void setBackgroundColor(QColor bg) {
			_hasCustomBackground = true;
			_bg = bg;
		}

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);

			if ( _hasCustomBackground ) {
				p.fillRect(rect(), _bg);
			}

			p.setPen(palette().color(QPalette::Mid));

			if ( p.device()->devicePixelRatioF() > 1.0 ) {
				p.setRenderHint(QPainter::Antialiasing, true);
				p.drawLine(layoutPadding() + 0.5, rect().top(), layoutPadding() + 0.5, rect().bottom());
			}
			else {
				p.drawLine(layoutPadding(), rect().top(), layoutPadding(), rect().bottom());
			}
		}

	private:
		bool   _hasCustomBackground;
		QColor _bg;
};


class ViewItemWidget : public QWidget {
	public:
		ViewItemWidget(QWidget *parent = 0) : QWidget(parent) {
			_isSelected = false;
		}

		void setSelected(bool s) {
			_isSelected = s;
			update();
		}

		bool isSelected() const { return _isSelected; }

	protected:
		void paintEvent(QPaintEvent *e) {
			QPainter p(this);
			if ( _isSelected ) {
				QLinearGradient grad(QPoint(0, 0), QPoint(28, 28));
				QColor c0 = palette().color(QPalette::Highlight);
				c0.setAlpha(32);
				QColor c1 = palette().color(QPalette::Base);
				c1.setAlpha(32);
				grad.setColorAt(0, c0);
				grad.setColorAt(0.66, c0);
				grad.setColorAt(0.67, c1);
				grad.setColorAt(1, c1);
				grad.setSpread(QGradient::RepeatSpread);
				c0.setAlpha(64);
				p.setPen(c0);
				p.setBrush(grad);
				if ( p.device()->devicePixelRatioF() > 1.0 ) {
					p.setRenderHint(QPainter::Antialiasing, true);
					p.drawRoundedRect(rect().adjusted(1, 0, 0, 0), 8, 8);
				}
				else {
					p.drawRoundedRect(rect(), 8, 8);
				}
			}
		}

	private:
		bool _isSelected;
};


class IconLabel : public QLabel {
	public:
		IconLabel(QIcon icon, QWidget *parent = nullptr)
		: QLabel(parent) {
			setPixmap(icon.pixmap(QFontMetrics(font()).ascent()));
		}
};


class HRuler : public QWidget {
	public:
		HRuler(qreal width = 1, QWidget *parent = nullptr)
		: QWidget(parent), _width(width) {
			setForegroundRole(QPalette::Highlight);
			setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
			setFixedHeight(_width * 2);
		}

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);
			QPoint p1, p2;
			if ( p.device()->devicePixelRatioF() > 1.0 ) {
				p.setRenderHint(QPainter::Antialiasing, true);
			}
			auto rect = contentsRect();
			p1 = QPoint(rect.x(), rect.y() + rect.height() / 2);
			p2 = QPoint(rect.x() + rect.width(), p1.y());
			p.setPen(QPen(palette().brush(foregroundRole()), _width));
			p.drawLine(p1, p2);
		}

	private:
		qreal _width;
};


class BlockHandle : public QToolButton {
	public:
		BlockHandle(QWidget *parent = 0) : QToolButton(parent) {
			setFixedWidth(layoutPadding() * 2);
			setFixedHeight(layoutPadding() * 2);
			setForegroundRole(QPalette::Mid);
		}

	protected:
		void paintEvent(QPaintEvent *) {
			static auto expand = ::icon("chevron_right", palette().color(foregroundRole())).pixmap(layoutPadding() * 2);
			static auto collapse = ::icon("chevron_down", palette().color(foregroundRole())).pixmap(layoutPadding() * 2);

			QPainter p(this);

			if ( isChecked() ) {
				p.drawPixmap(0, 0, collapse);
			}
			else {
				p.drawPixmap(0, 0, expand);
			}
		}
};


class Header : public QWidget {
	public:
		Header(QColor bg, QWidget *parent = 0) : QWidget(parent), _bg(bg) {}

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);
			p.fillRect(rect(), _bg);
		}

	private:
		QColor _bg;
};


class HeaderLabel : public QLabel {
	public:
		HeaderLabel() {
			QPalette pal = palette();
			pal.setColor(QPalette::Text, blend(pal.color(QPalette::Text), pal.color(QPalette::Highlight), 50));
			setPalette(pal);
		}
};


class IconButton : public QAbstractButton {
	public:
		IconButton(const QIcon &icon)
		: QAbstractButton() {
			setIcon(icon);
		}


	protected:
		void paintEvent(QPaintEvent *) {
			auto pixmap = icon().pixmap(
				size(),
				isEnabled() ? QIcon::Normal : QIcon::Disabled,
				isChecked() ? QIcon::On : QIcon::Off
			);

			QPainter p(this);
			p.drawPixmap(0, 0, pixmap);
		}
};


class BaseTextLabel : public QWidget {
	public:
		BaseTextLabel(QWidget *parent = 0) : QWidget(parent) {
			QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
			sp.setHeightForWidth(true);
			setSizePolicy(sp);
		}

		void setText(const QString &text) {
			_text = text;
		}

		const QString &text() const {
			return _text;
		}

		int heightForWidth(int w) const {
			auto m = contentsMargins();
			int prefHeight =
				fontMetrics().boundingRect(
					0, 0, w - m.left() - m.right(), QWIDGETSIZE_MAX,
					Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, _text
				).height() + m.top() + m.bottom();
			return prefHeight;
		}

		void paintEvent(QPaintEvent *e) {
			QPainter p(this);

			if ( autoFillBackground() ) {
				p.fillRect(e->rect(), palette().color(QPalette::Window));
			}

			p.drawText(contentsRect(), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, _text);
		}


	private:
		QString _text;
};


class DescLabel : public BaseTextLabel {
	public:
		DescLabel(QWidget *parent = 0) : BaseTextLabel(parent) {
			QPalette pal = palette();
			pal.setColor(QPalette::Text, blend(pal.color(QPalette::Text), pal.color(QPalette::Highlight), 50));
			setPalette(pal);
		}
};


class HelpLabel : public BaseTextLabel {
	public:
		HelpLabel(QWidget *parent = 0) : BaseTextLabel(parent) {
			setForegroundRole(QPalette::Dark);
		}
};


class AlertLabel : public BaseTextLabel {
	public:
		AlertLabel(QWidget *parent = 0) : BaseTextLabel(parent) {
			QPalette pal = palette();
			pal.setColor(QPalette::Text, AlertColor);
			setPalette(pal);
		}
};


class StringEdit : public QLineEdit, public FancyViewItemEdit {
	public:
		StringEdit(QWidget *parent = 0) : QLineEdit(parent) {}

		QWidget *widget() { return this; }

		void setValue(const QString &value) {
			setText(value);
		}

		QString value() const {
			return text();
		}

	/*
	protected:
		void focusInEvent(QFocusEvent *e) {
			QPalette pal = palette();
			pal.setColor(QPalette::Base, QColor(255,255,224));
			setPalette(pal);
			QLineEdit::focusInEvent(e);
		}

		void focusOutEvent(QFocusEvent *e) {
			QPalette pal;
			setPalette(pal);
			QLineEdit::focusOutEvent(e);
		}
	*/
};


class BoolEdit : public QCheckBox, public FancyViewItemEdit {
	public:
		BoolEdit(QWidget *parent = 0) : QCheckBox(parent) {}

		QWidget *widget() { return this; }

		void setValue(const QString &value) {
			setChecked(value.compare("true", Qt::CaseInsensitive) == 0);
		}

		QString value() const {
			return isChecked()?"true":"false";
		}
};


class ComboEdit : public QComboBox, public FancyViewItemEdit {
	public:
		ComboEdit(QWidget *parent = 0) : QComboBox(parent) {
			setEditable(true);
			completer()->setCaseSensitivity(Qt::CaseSensitive);
		}

		QWidget *widget() { return this; }

		void setValue(const QString &value) {
			setCurrentText(value);
		}

		QString value() const {
			return currentText();
		}

	/*
	protected:
		void focusInEvent(QFocusEvent *e) {
			QPalette pal = palette();
			pal.setColor(QPalette::Base, QColor(255,255,224));
			setPalette(pal);
			QLineEdit::focusInEvent(e);
		}

		void focusOutEvent(QFocusEvent *e) {
			QPalette pal;
			setPalette(pal);
			QLineEdit::focusOutEvent(e);
		}
	*/
};


class EvalHintWidget : public QLabel {
	public:
		EvalHintWidget(QWidget *w) : QLabel(w) {
			setFrameStyle(QFrame::NoFrame);
		}

	protected:
		void paintEvent(QPaintEvent *e) {
			QPainter p(this);
			p.setPen(QPen(palette().color(QPalette::WindowText), 3, Qt::DotLine));
			p.setBrush(palette().color(QPalette::Window));
			p.drawRect(rect().adjusted(0,0,-1,-1));
			QLabel::paintEvent(e);
		}
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FancyViewItem::FancyViewItem(const QModelIndex &idx, QWidget *c)
: index(idx), container(c) {
	if ( container ) {
		// Link the container widget with its FancyViewItem
		container->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(*this));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyViewItem::updated() {
	bool isInitial = index.sibling(index.row(), 3).data(Qt::DisplayRole).toBool() ==
	                 index.sibling(index.row(), 3).data(ConfigurationTreeItemModel::Initial).toBool();

	if ( isInitial && !index.sibling(index.row(), 3).data(Qt::DisplayRole).toBool() ) {
		isInitial &= index.sibling(index.row(), 2).data(Qt::DisplayRole).toString() ==
		             index.sibling(index.row(), 2).data(ConfigurationTreeItemModel::Initial).toString();
	}

	/*
	std::cerr << qPrintable(index.sibling(index.row(), 0).data().toString())
	          << " "
	          << index.sibling(index.row(), 3).data(ConfigurationTreeItemModel::Initial).toBool()
	          << ":"
	          << index.sibling(index.row(), 3).data(Qt::DisplayRole).toBool()
	          << " "
	          << "\"" << qPrintable(index.sibling(index.row(), 2).data(Qt::DisplayRole).toString()) << "\" "
	          << "\"" << qPrintable(index.sibling(index.row(), 2).data(ConfigurationTreeItemModel::Initial).toString()) << "\""
	          << std::endl;
	*/

	reset->setVisible(!isInitial);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FancyView::FancyView(QWidget *parent) : QAbstractItemView(parent) {
	_iconEdit = icon("param_edit|param_edit_off");
	_iconReset = icon("refresh", palette().color(QPalette::Highlight));
	setFrameShape(QFrame::NoFrame);
	setFocusPolicy(Qt::StrongFocus);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect FancyView::visualRect(const QModelIndex &index) const {
	return QRect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::scrollTo(const QModelIndex &index, ScrollHint hint) {
	auto it = _viewItems.find(index);
	if ( it == _viewItems.end() ) {
		return;
	}

	QWidget *w = it.value().container;
	if ( !w->isVisible() ) {
		auto parent = index.parent();
		while ( parent.isValid() ) {
			it = _viewItems.find(parent);
			if ( it != _viewItems.end() ) {
				auto toggle = it.value().toggle;
				if ( toggle && !toggle->isChecked() ) {
					toggle->setChecked(true);
				}
			}

			parent = parent.parent();
		}
	}

	QPoint p = _rootWidget->mapFromGlobal(w->mapToGlobal(QPoint(0, 0)));
	horizontalScrollBar()->setValue(p.x());
	verticalScrollBar()->setValue(p.y());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QModelIndex FancyView::indexAt(const QPoint &point) const {
	return QModelIndex();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::setModel(QAbstractItemModel *model) {
	QAbstractItemView::setModel(model);

	if ( _rootWidget ) {
		delete _rootWidget;
		_rootWidget = nullptr;
	}

	horizontalScrollBar()->setRange(0, 0);
	verticalScrollBar()->setRange(0, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::setRootIndex(const QModelIndex &index) {
	QAbstractItemView::setRootIndex(index);
	if ( _rootWidget ) {
		delete _rootWidget;
		_rootWidget = nullptr;
	}

	_currentItem = nullptr;
	_viewItems = ViewItems();

	if ( !index.isValid() ) {
		return;
	}

	_rootWidget = new QWidget(viewport());
	//_rootWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	QVBoxLayout *l = new QVBoxLayout;
	l->setSpacing(layoutPadding());
	setMargin(l, layoutPadding() * 2);
	_rootWidget->setLayout(l);

	int rows = model()->rowCount(index);

	if ( !_btnSearch ) {
		_btnSearch = new QPushButton(icon("search"), {}, this);
		_btnSearch->setVisible(true);
		connect(_btnSearch, &QPushButton::clicked, this, &FancyView::searchRequested);
	}

	/*
	if ( index.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeModule ) {
		Module *mod = reinterpret_cast<Module*>(index.data(ConfigurationTreeItemModel::Link).value<void*>());
		QLabel *about = new QLabel(_rootWidget);
		about->setWordWrap(true);
		about->setText(mod->definition->description.c_str());
		about->setMargin(8);
		l->addWidget(about);
	}
	*/

	{
		auto helpLayout = new QHBoxLayout;
		setMargin(helpLayout, 0);
		helpLayout->addWidget(new QLabel("Parameter options:"));
		auto tmpLayout = new QHBoxLayout;
		tmpLayout->setSpacing(0);
		tmpLayout->addWidget(new IconLabel(icon("param_edit")));
		tmpLayout->addWidget(new QLabel("|"));
		tmpLayout->addWidget(new IconLabel(icon("param_edit_off")));
		helpLayout->addLayout(tmpLayout);
		helpLayout->addWidget(new QLabel("Set/Remove value"));
		helpLayout->addSpacing(QFontMetrics(font()).averageCharWidth());
		helpLayout->addWidget(new IconLabel(icon("refresh", palette().color(QPalette::Highlight))));
		helpLayout->addWidget(new QLabel("Reset to intial state"));
		helpLayout->addStretch();
		// helpLayout->addWidget(_btnSearch);
		l->addLayout(helpLayout);
	}

	if ( index.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeModule ) {
		Module *mod = reinterpret_cast<Module*>(index.data(ConfigurationTreeItemModel::Link).value<void*>());
		QLabel *info = new QLabel(_rootWidget);
		info->setWordWrap(true);
		info->setForegroundRole(QPalette::Highlight);
		if ( mod->supportsBindings() ) {
			info->setText(tr(
				"This module considers module configuration parameters. "
				"It also requires bindings which may overwrite module "
				"configuration parameters."
			));
		}
		else {
			info->setText(tr(
				"This module only considers module configuration parameters. "
				"It does not provide a bindings configuration."
			));
		}
		l->addWidget(info);
	}

	QString secName;
	int type = index.data(ConfigurationTreeItemModel::Type).toInt();
	if ( type == ConfigurationTreeItemModel::TypeModule ||
	     type == ConfigurationTreeItemModel::TypeBinding ) {
		secName = index.data().toString();
	}

	for ( int i = 0; i < rows; ++i ) {
		auto idx = model()->index(i, 0, index);
		QWidget *w = createWidgetFromIndex(idx, secName);
		if ( w ) {
			l->addWidget(w);
		}
	}

	l->addStretch();

	_rootWidget->installEventFilter(this);
	_rootWidget->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::setConfigStage(Seiscomp::Environment::ConfigStage cs) {
	_configStage = cs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QWidget *FancyView::createWidgetFromIndex(const QModelIndex &idx,
                                          const QString &rootSecName) {
	ViewItemWidget *w = new ViewItemWidget;
	QBoxLayout *l = new QVBoxLayout;
	l->setSpacing(layoutPadding());
	setMargin(l, 1);
	w->setLayout(l);

	w->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	int rows = model()->rowCount(idx);
	int type = idx.data(ConfigurationTreeItemModel::Type).toInt();

	switch ( type ) {
		case ConfigurationTreeItemModel::TypeCategoryBinding:
		{
			Binding *binding = reinterpret_cast<Binding*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( binding ) {
				FancyViewItem item(idx, w);
				add(l, item, binding, true);
				l->setContentsMargins(0, 0, 0, 0);

				w->setProperty("viewBinding", QVariant::fromValue((void*)binding));

				bool firstParameter = true;
				QLayout *paramLayout = nullptr;

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new Seiscomp::Gui::FlowLayout(0, layoutPadding() * 2, layoutPadding() * 2);
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);

					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					auto desc = new AlertLabel;
					desc->setText("This section does not contain a parameter to configure...");
					l->addWidget(desc);
				}

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeCategory:
		{
			BindingCategory *cat = reinterpret_cast<BindingCategory*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if (cat ) {
				FancyViewItem item(idx, w);
				add(l, item, cat, false);
				l->setContentsMargins(0, 0, 0, 0);

				auto alert = new AlertLabel;
				alert->setText(QString("No binding for \"%1\" selected.").arg(cat->name.c_str()));
				l->addWidget(alert);
				alert->setVisible(cat->bindings.empty());

				w->setProperty("statusLabel", QVariant::fromValue((void*)alert));

				for ( int r = 0; r < rows; ++r ) {
					auto secIdx = model()->index(r, 0, idx);
					int type = secIdx.data(ConfigurationTreeItemModel::Type).toInt();
					if ( type != ConfigurationTreeItemModel::TypeCategoryBinding )
						continue;

					QWidget *bw = createWidgetFromIndex(secIdx, rootSecName);
					bw->setProperty("statusLabel", QVariant::fromValue((void*)alert));
					l->addWidget(bw);
				}

				size_t catBindingCount = cat->bindingTypes.size();
				QComboBox *comboBox = NULL;

				comboBox = new QComboBox;
				comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
				//comboBox->addItem("- None -");

				for ( size_t i = 0; i < catBindingCount; ++i ) {
					Binding *b = cat->bindingTypes[i].get();
					if ( b->definition->description.empty() ) {
						comboBox->addItem(b->definition->name.c_str(),
						                  model()->index(i, 0, item.index).data());
					}
					else {
						comboBox->addItem(
							QString("%1 - %2")
							.arg(b->definition->name.c_str())
							.arg(maxSize(b->definition->description, 40).c_str()),
							model()->index(i, 0, item.index).data()
						);
						comboBox->setItemData(comboBox->count()-1, string2Block(b->definition->description, 100).c_str(), Qt::ToolTipRole);
					}
					comboBox->setItemData(comboBox->count()-1, b->definition->name.c_str());
				}

				comboBox->model()->sort(0);
				comboBox->setCurrentIndex(0);

				QToolButton *addButton = new QToolButton;
				addButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
				addButton->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
				addButton->setProperty("comboBox", QVariant::fromValue((void*)comboBox));
				addButton->setIcon(::icon("add"));
				addButton->setToolTip(QString("Add a new '%1' instance").arg(cat->name.c_str()));
				addButton->setEnabled(catBindingCount > 0);

				QHBoxLayout *hlayout = new QHBoxLayout;
				hlayout->addWidget(addButton);
				hlayout->addWidget(comboBox);
				hlayout->addStretch();
				connect(comboBox, SIGNAL(currentIndexChanged(int)),
				        this, SLOT(bindingCategoryChanged(int)));

				connect(addButton, SIGNAL(clicked()), this, SLOT(addCategoryBinding()));

				if ( comboBox ) {
					comboBox->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
					comboBox->setProperty("cat.layout", QVariant::fromValue((void*)l));
				}

				l->addLayout(hlayout);

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeSection:
		{
			Section *sec = reinterpret_cast<Section*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( sec ) {
				FancyViewItem item(idx, w);
				add(l, item, sec, idx.data().toString() != rootSecName);
				l->setContentsMargins(0, 0, 0, 0);

				bool firstParameter = true;
				QLayout *paramLayout = NULL;

				if ( !sec->description.empty() ) {
					StatusLabel *desc = new StatusLabel;
					desc->setWordWrap(true);
					desc->setInfoText(sec->description.c_str());
					l->addWidget(desc);
				}

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new Seiscomp::Gui::FlowLayout(0, layoutPadding() * 2, layoutPadding() * 2);
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);

					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					auto alert = new AlertLabel;
					alert->setText("This section does not contain a parameter to configure...");
					l->addWidget(alert);
				}

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeGroup:
		{
			Group *group = reinterpret_cast<Group*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( group ) {
				FancyViewItem item(idx, w);
				add(l, item, group);
				l->setContentsMargins(0, 0, 0, 0);

				bool firstParameter = true;
				QLayout *paramLayout = nullptr;

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new Seiscomp::Gui::FlowLayout(0, layoutPadding() * 2, layoutPadding() * 2);
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);
					if ( item.isValid() ) {
						_viewItems[child] = item;
					}
				}

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter ) {
						continue;
					}
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw ) {
						l->addWidget(cw);
					}
				}

				if ( rows == 0 ) {
					auto alert = new AlertLabel;
					alert->setText("This group does not contain a parameter to configure...");
					l->addWidget(alert);
				}

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeParameter:
		{
			FancyViewItem item = add(l, idx);
			l->setContentsMargins(0, 0, 0, 0);
			if ( item.isValid() ) {
				_viewItems[idx] = item;
			}

			break;
		}
		case ConfigurationTreeItemModel::TypeStruct:
		{
			Structure *struc = reinterpret_cast<Structure*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( struc ) {
				FancyViewItem item(idx, w);
				add(l, item, struc);
				l->setContentsMargins(0, 0, 0, 0);

				if ( struc->name.empty() ) break;

				bool firstParameter = true;
				QLayout *paramLayout = NULL;

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new Seiscomp::Gui::FlowLayout(0, layoutPadding() * 2, layoutPadding() * 2);
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);
					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					auto child = model()->index(i, 0, idx);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					auto alert = new AlertLabel;
					alert->setText("This group does not contain a parameter to configure...");
					l->addWidget(alert);
				}
			}
			break;
		}
		default:
			break;
	}

	return w;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item,
                    Seiscomp::System::BindingCategory *cat, bool collapsed) {
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing(layoutPadding());
	hlayout->setContentsMargins(0, 0, 0, 0);

	item.toggle = new BlockHandle;
	item.toggle->setCheckable(true);
	item.toggle->setChecked(true);

	Header *header = new Header(CategoryBgColor);
	header->setLayout(hlayout);

	hlayout->addWidget(item.toggle);

	QLabel *catName = new QLabel;
	catName->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = catName->font();
	f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	//f.setItalic(true);
	catName->setFont(f);
	catName->setText(item.index.data().toString());
	QPalette pal = catName->palette();
	pal.setColor(QPalette::Text, CategoryTextColor);
	catName->setPalette(pal);

	item.label = catName;

	hlayout->addWidget(catName);

	HRuler *hline = new HRuler(1);
	hlayout->addWidget(hline);

	//layout->addLayout(hlayout);
	layout->addWidget(header);

	BlockWidget *catWidget = new BlockWidget;
	catWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	layout->addWidget(catWidget);

	layout = new QVBoxLayout;
	layout->setSpacing(layoutPadding());
	catWidget->setLayout(layout);

	if ( collapsed ) {
		item.toggle->setChecked(false);
		catWidget->setVisible(false);
	}

	connect(item.toggle, &BlockHandle::toggled, catWidget, &QWidget::setVisible);

	/*
	size_t catBindingCount = cat->bindingTypes.size();
	QComboBox *comboBox = NULL;

	comboBox = new QComboBox;
	comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	comboBox->addItem("- None -");

	for ( size_t i = 0; i < catBindingCount; ++i ) {
		Binding *b = cat->bindingTypes[i].get();
		//comboBox->addItem(b->definition->description.c_str(), model()->index(i, 0, item.index).data());
		comboBox->addItem(b->definition->name.c_str(), model()->index(i, 0, item.index).data());
	}

	catWidget->setBackgroundColor(blend(pal.color(QPalette::Base), CategoryBgColor, 90));
	comboBox->setCurrentIndex(0);
	hlayout->addWidget(comboBox);
	connect(comboBox, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(bindingCategoryChanged(int)));

	QToolButton *addButton = new QToolButton;
	addButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	addButton->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
	addButton->setIcon(QIcon(":/res/icons/add.png"));
	addButton->setToolTip(QString("Add a new '%1' instance").arg(cat->name.c_str()));

	hlayout->addWidget(addButton);

	if ( comboBox ) {
		comboBox->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
		comboBox->setProperty("cat.layout", QVariant::fromValue((void*)layout));
	}
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item,
                    Seiscomp::System::Binding *binding, bool collapsed) {
	item.toggle = new BlockHandle;
	item.toggle->setCheckable(true);
	item.toggle->setChecked(true);

	QToolButton *removeButton = new QToolButton;
	removeButton->setIcon(::icon("delete_forever"));
	removeButton->setToolTip(QString("Remove binding '%1'").arg(binding->name.c_str()));
	removeButton->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeCategoryBinding()));

	QLabel *sectionName = new HeaderLabel;
	sectionName->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = sectionName->font();
	//f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	//f.setItalic(true);
	sectionName->setFont(f);

	QString label = item.index.data().toString();
	if ( label != binding->name.c_str() ) {
		sectionName->setText(item.index.data().toString() + " : " + binding->name.c_str());
	}
	else {
		sectionName->setText(item.index.data().toString());
	}

	item.label = sectionName;

	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->setSpacing(layoutPadding());
	hlayout->addWidget(item.toggle);
	hlayout->addWidget(sectionName);
	hlayout->addWidget(new HRuler);

	hlayout->addWidget(removeButton);

	layout->addLayout(hlayout);

	BlockWidget *sectionWidget = new BlockWidget;
	auto pal = sectionWidget->palette();
	sectionWidget->setBackgroundColor(blend(pal.color(QPalette::Base), CategoryBgColor, 90));
	sectionWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	layout->addWidget(sectionWidget);

	layout = new QVBoxLayout;
	layout->setSpacing(layoutPadding());
	sectionWidget->setLayout(layout);

	if ( collapsed ) {
		item.toggle->setChecked(false);
		sectionWidget->setVisible(false);
	}

	connect(item.toggle, &BlockHandle::toggled, sectionWidget, &QWidget::setVisible);

	return true;
}


bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item,
                    Seiscomp::System::Section *sec, bool collapsed) {
	item.toggle = new BlockHandle;
	item.toggle->setCheckable(true);
	item.toggle->setChecked(true);

	QLabel *sectionName = new HeaderLabel;
	sectionName->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = sectionName->font();
	f.setPointSize(f.pointSize() * 150 / 100);
	f.setBold(true);
	//f.setItalic(true);
	sectionName->setFont(f);
	sectionName->setText(item.index.data().toString());

	item.label = sectionName;

	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing(layoutPadding());
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(item.toggle);
	hlayout->addWidget(sectionName);
	hlayout->addWidget(new HRuler(2));

	layout->addLayout(hlayout);

	QWidget *sectionWidget = new BlockWidget;
	sectionWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	layout->addWidget(sectionWidget);

	layout = new QVBoxLayout;
	layout->setSpacing(layoutPadding());
	sectionWidget->setLayout(layout);

	if ( collapsed ) {
		item.toggle->setChecked(false);
		sectionWidget->setVisible(false);
	}

	connect(item.toggle, &BlockHandle::toggled, sectionWidget, &QWidget::setVisible);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item, Group *group) {
	// layout->addSpacing(16);

	// Build header
	item.toggle = new BlockHandle;
	item.toggle->setCheckable(true);
	item.toggle->setChecked(true);

	QLabel *name = new HeaderLabel;
	name->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = name->font();
	//f.setPointSize(f.pointSize()*125/100);
	f.setBold(true);
	name->setFont(f);
	name->setText(item.index.data().toString());//  group->definition->name.c_str());

	item.label = name;

	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing(layoutPadding());
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(item.toggle);
	hlayout->addWidget(name);
	hlayout->addWidget(new HRuler);

	layout->addLayout(hlayout);

	// Build group widget
	QWidget *groupWidget = new BlockWidget;
	//groupWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	//groupWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QVBoxLayout *groupLayout = new QVBoxLayout;
	groupLayout->setSpacing(layoutPadding());
	groupWidget->setLayout(groupLayout);

	if ( !group->definition->description.empty() ) {
		auto desc = new DescLabel;
		desc->setText(group->definition->description.c_str());
		groupLayout->addWidget(desc);
		item.description = desc;
	}

	layout->addWidget(groupWidget);
	layout = groupLayout;

	connect(item.toggle, &BlockHandle::toggled, groupWidget, &QWidget::setVisible);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item, Structure *struc) {
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing(layoutPadding());
	hlayout->setContentsMargins(0, 0, 0, 0);

	QLabel *type = new HeaderLabel;
	type->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

	QToolButton *modify = new QToolButton;
	modify->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	modify->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

	// Just the definition?
	if ( struc->name.empty() ) {
		if ( !struc->definition->title.empty() ) {
			type->setText(struc->definition->title.c_str());
			type->setToolTip(struc->definition->type.c_str());
		}
		else {
			type->setText(struc->definition->type.c_str());
		}
		modify->setIcon(::icon("add"));
		modify->setToolTip(QString("Create a new '%1' instance").arg(struc->definition->type.c_str()));
		//add->setFlat(true);
		//add->setIconSize(QSize(20,20));
		//add->setFixedSize(QSize(22,22));

		connect(modify, SIGNAL(clicked()), this, SLOT(addStruct()));

		hlayout->addWidget(modify);
		hlayout->addWidget(type);
		hlayout->addStretch();
	}
	else {
		// Build header
		item.toggle = new BlockHandle;
		item.toggle->setCheckable(true);
		hlayout->addWidget(item.toggle);

		type->setText(struc->name.c_str());
		modify->setIcon(::icon("delete_forever"));
		modify->setToolTip(QString("Delete structure '%1'").arg(type->text()));

		connect(modify, SIGNAL(clicked()), this, SLOT(removeStruct()));

		QFont f = type->font();
		f.setBold(true);
		f.setItalic(true);
		type->setFont(f);

		item.toggle->setChecked(true);

		hlayout->addWidget(type);
		hlayout->addWidget(new HRuler);
		hlayout->addWidget(modify);
	}

	layout->addLayout(hlayout);

	item.label = type;

	if ( !struc->name.empty() ) {
		// Build group widget
		QWidget *groupWidget = new BlockWidget;

		QVBoxLayout *groupLayout = new QVBoxLayout;
		groupLayout->setSpacing(layoutPadding());
		groupWidget->setLayout(groupLayout);

		if ( !struc->definition->description.empty() ) {
			auto desc = new DescLabel;
			desc->setText(struc->definition->description.c_str());
			groupLayout->addWidget(desc);
			item.description = desc;
		}

		layout->addWidget(groupWidget);
		layout = groupLayout;

		groupWidget->setVisible(item.toggle->isChecked());

		connect(item.toggle, &QAbstractButton::toggled, groupWidget, &QWidget::setVisible);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FancyViewItem FancyView::add(QLayout *layout, const QModelIndex &idx) {
	Parameter *param = reinterpret_cast<Parameter*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
	if ( !param ) {
		return FancyViewItem();
	}

	ViewItemWidget *paramWidget = new ViewItemWidget;
	QVBoxLayout *paramLayout = new QVBoxLayout;
	setMargin(paramLayout, 1);
	paramLayout->setSpacing(0);
	paramWidget->setLayout(paramLayout);

	paramWidget->setObjectName(param->definition->name.c_str());

	// Value not set but imported?
	bool isOverridden = param->symbol.stage > _configStage;
	bool isDefined = param->symbol.stage > Environment::CS_UNDEFINED;

	FancyViewItemEdit *inputWidget = nullptr;
	QWidget *textWidget;
	QHBoxLayout *nameLayout = new QHBoxLayout;

	QString paramLabel = idx.data().toString();
	if ( !param->definition->unit.empty() ) {
		paramLabel += QString(" [%1]").arg(param->definition->unit.c_str());
	}

	if ( param->definition->type == "boolean" ) {
		paramLayout->addStretch();

		BoolEdit *checkBox = new BoolEdit;
		QFont f = checkBox->font();
		f.setBold(true);
		checkBox->setFont(f);
		checkBox->setText(paramLabel);
		checkBox->setValue(idx.sibling(idx.row(),2).data().toString());
		inputWidget = checkBox;
		textWidget = checkBox;

		connect(checkBox, &BoolEdit::toggled, this, &FancyView::optionToggled);

		nameLayout->addWidget(checkBox);
		paramLayout->addLayout(nameLayout);

		if ( isOverridden ) {
			QPalette pal = checkBox->palette();
			QColor oldButton = pal.color(QPalette::Disabled, QPalette::Base);
			pal.setColor(QPalette::Base, AlertColor);
			pal.setColor(QPalette::Button, AlertColor);
			pal.setColor(QPalette::Window, AlertColor);
			pal.setColor(QPalette::Highlight, AlertColor);
			pal.setColor(QPalette::ButtonText, AlertTextColor);
			pal.setColor(QPalette::Disabled, QPalette::Base, blend(AlertColor, oldButton, 50));
			pal.setColor(QPalette::Disabled, QPalette::Button, blend(AlertColor, oldButton, 50));
			pal.setColor(QPalette::Disabled, QPalette::Window, blend(AlertColor, oldButton, 50));
			pal.setColor(QPalette::Disabled, QPalette::Highlight, blend(AlertColor, oldButton, 50));
			checkBox->setPalette(pal);
		}
	}
	else {
		QLabel *name = new QLabel;
		QFont f = name->font();
		f.setBold(true);
		name->setFont(f);
		name->setText(paramLabel);

		textWidget = name;

		if ( param->definition->values.empty()
		  || (param->definition->type.compare(0, 5, "list:") == 0)
		  || (param->definition->type == "file") ) {
			// No predefined values or the type is a list of some type
			StringEdit *edit = new StringEdit;
			edit->setValue(idx.sibling(idx.row(),2).data().toString());
			connect(edit, SIGNAL(editingFinished()), this, SLOT(optionTextEdited()));
			connect(edit, SIGNAL(textEdited(QString)), this, SLOT(optionTextChanged(QString)));
			inputWidget = edit;
		}
		else {
			ComboEdit *combo = new ComboEdit;
			for ( const auto &value : param->definition->values ) {
				combo->addItem(value.c_str());
			}
			combo->setValue(idx.sibling(idx.row(),2).data().toString());
			connect(combo, SIGNAL(currentTextChanged(QString)), this, SLOT(optionTextChanged(QString)));
			connect(combo, SIGNAL(currentTextChanged(QString)), this, SLOT(optionTextEdited()));
			inputWidget = combo;
		}

		if ( isOverridden ) {
			QPalette pal = inputWidget->widget()->palette();

			QColor oldBase = pal.color(QPalette::Disabled, QPalette::Base);
			QColor oldWindow = pal.color(QPalette::Disabled, QPalette::Window);
			QColor oldText = pal.color(QPalette::Disabled, QPalette::Text);

			pal.setColor(QPalette::Base, AlertColor);
			pal.setColor(QPalette::Text, AlertTextColor);
			pal.setColor(QPalette::Window, AlertFrameColor);
			pal.setColor(QPalette::Disabled, QPalette::Base, blend(AlertColor, oldBase, 50));
			pal.setColor(QPalette::Disabled, QPalette::Window, blend(AlertFrameColor, oldWindow, 50));
			pal.setColor(QPalette::Disabled, QPalette::Text, blend(AlertTextColor, oldText, 50));
			inputWidget->widget()->setPalette(pal);
		}

		nameLayout->addWidget(name);
		paramLayout->addLayout(nameLayout);
		paramLayout->addWidget(inputWidget->widget());
	}

	nameLayout->addStretch();
	auto btnReset = new IconButton(_iconReset);
	nameLayout->addWidget(btnReset);
	auto btnEdit = new IconButton(_iconEdit);
	nameLayout->addWidget(btnEdit);

	updateToolTip(inputWidget->widget(), param);

	FancyViewItem item(idx, paramWidget);
	item.reset = btnReset;
	item.editControl = btnEdit;
	item.label = textWidget;
	item.input = inputWidget;

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
	if ( !descText.empty() ) {
		auto help = new HelpLabel;
		help->setText(maxSize(descText, 60).c_str());
		QString content(string2Block(descText, 80).c_str());
		content = encodeHTML(content);

		QString toolTip = QString("<p style='white-space:pre'>%1</p>").arg(content);
		help->setToolTip(toolTip);

		paramLayout->addWidget(help);
		item.description = help;

		if ( item.input->widget() ) {
			item.input->widget()->setWhatsThis(descText.c_str());
		}
	}
	else {
		item.description = nullptr;
	}

	if ( (idx.sibling(idx.row(), 2).flags() & Qt::ItemIsEnabled) == 0 ) {
		item.label->setEnabled(false);
		item.input->widget()->setEnabled(false);
		if ( item.description ) {
			item.description->setEnabled(false);
		}
	}

	if ( btnEdit ) {
		btnEdit->setCheckable(true);
		btnEdit->setFixedSize(16, 16);
		btnEdit->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

		if ( idx.sibling(idx.row(), 3).data().toBool() ) {
			btnEdit->setChecked(true);
			btnEdit->setToolTip(isDefined?
			                   "This parameter is locked. Its is already "
			                   "defined in an earlier or later configuration stage.\n"
			                   "If you want to redefine it you can press "
			                   "the button to unlock it."
			                   :
			                   "This parameter is currently not set and the "
			                   "default value is displayed.\n"
			                   "To redefine the parameter, press the button "
			                   "to unlock it.");
		}
		else {
			btnEdit->setChecked(false);
			btnEdit->setToolTip("This parameter is present in the application "
			                   "configuration. To remove the parameter\nin "
			                   "order to use the applications default, press "
			                   "the button and lock it.");
		}

		connect(btnEdit, &QAbstractButton::toggled, this, &FancyView::editChanged);
	}

	if ( btnReset ) {
		btnReset->setVisible(false);
		btnReset->setCheckable(false);
		btnReset->setFixedSize(16, 16);
		btnReset->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
		connect(btnReset, &QAbstractButton::clicked, this, &FancyView::resetValue);
	}

	paramLayout->addStretch();

	layout->addWidget(paramWidget);

	// Link the view item with the input widget
	inputWidget->widget()->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
	item.updated();

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::showSearchButton(bool v) {
	if ( _btnSearch ) {
		_btnSearch->setVisible(v);
		_btnSearch->setToolTip(tr("Search - Ctrl + F"));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                            const QVector<int> &roles) {
	auto it = _viewItems.find(topLeft.sibling(topLeft.row(),0));
	if ( it == _viewItems.end() ) {
		return;
	}

	if ( _blockPopulate != it.value().input->widget() ) {
		auto &item = it.value();
		bool isEnabled = model()->flags(topLeft.sibling(topLeft.row(), 2)) & Qt::ItemIsEnabled;
		item.label->setEnabled(isEnabled);
		item.input->widget()->setEnabled(isEnabled);
		if ( item.description ) {
			item.description->setEnabled(isEnabled);
		}

		// Change values
		if ( topLeft.column() == 2 ) {
			item.input->setValue(topLeft.data().toString());
		}

		if ( (topLeft.column() == 2) || (topLeft.column() == 3) ) {
			item.updated();
		}
	}

	QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::rowsInserted(const QModelIndex &parent, int start, int end) {
	QAbstractItemView::rowsInserted(parent, start, end);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) {
	QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::eventFilter(QObject *o, QEvent *e) {
	if ( o == _rootWidget && e->type() == QEvent::LayoutRequest ) {
		updateContentGeometry();
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QModelIndex FancyView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                  Qt::KeyboardModifiers modifiers) {
	return QModelIndex();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FancyView::horizontalOffset() const {
	return horizontalScrollBar()->value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FancyView::verticalOffset() const {
	return verticalScrollBar()->value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::isIndexHidden(const QModelIndex &index) const {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::setSelection(const QRect&, QItemSelectionModel::SelectionFlags command) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::mousePressEvent(QMouseEvent *event) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::mouseMoveEvent(QMouseEvent *event) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::mouseReleaseEvent(QMouseEvent *event) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::editChanged(bool state) {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) {
		return;
	}

	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(), 0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);

	//if ( item.input ) item.input->setDisabled(state);
	//if ( item.label ) item.label->setDisabled(state);

	model()->setData(item.index.sibling(item.index.row(), 3), state);

	updateToolTip(item.input->widget(), param);
	item.updated();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::resetValue() {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) {
		return;
	}

	// Restore initial check state
	item.editControl->setChecked(model()->data(item.index.sibling(item.index.row(), 3), ConfigurationTreeItemModel::Initial).toBool());
	model()->setData(
		item.index.sibling(item.index.row(), 2),
		model()->data(item.index.sibling(item.index.row(), 2), ConfigurationTreeItemModel::Initial).toString()
	);

	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(),0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);
	updateToolTip(item.input->widget(), param);
	item.updated();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::evaluateValue(const std::string& valueTest,
                              const Seiscomp::System::Parameter *param,
                              QString &eval, bool verbose=false) {
	auto evalSize = eval.size();

	// test values types
	// types tested: int, uint, boolean, float, double, time, boolean, file,
	// directory, host and port, gradient
	const auto& type = param->definition->type;
	string symbolURIString = param->symbol.uri.empty() ? "" : param->symbol.uri + ": ";

	// boolean
	if ( type == "boolean" ) {
		if ( valueTest != "true" && valueTest != "false" ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '"
				     << valueTest << "' must be true or false " << endl;
			}
			eval += "<b>Value must be true or false:</b> ";
		}
	}
	// double/float
	else if ( (type == "double") || (type == "list:double") ||
	          (type == "float") || (type == "list:float") ) {
		double value;
		if ( !valueTest.empty() && !Core::fromString(value, valueTest) ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '"
				     << valueTest << "' must be double"<< endl;
			}
			eval += "<b>Value must be double:</b> ";
		}
	}
	// integer
	else if ( (type == "int") || (type == "list:int") ) {
		int value;
		if ( !valueTest.empty() && !Core::fromString(value, valueTest) ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '" << valueTest
				     << "' must be integer"<< endl;
			}
			eval += "<b>Value must be integer:</b> ";
		}
	}
	// unsigned integer
	else if ( (type == "uint") || (type == "list:uint") ) {
		int value;
		if ( !valueTest.empty() && (
		         !Core::fromString(value, valueTest) || value < 0 ) ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '" << valueTest
				     << "' must be unsigned integer"<< endl;
			}
			eval += "<b>Value must be unsigned integer:</b> ";
		}
	}
	// time
	else if ( (type == "time") || (type == "list:time") ) {
		Core::Time value;
		if ( !valueTest.empty() && !Core::fromString(value, valueTest) ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '"
				     << valueTest << "' must be time"<< endl;
			}
			eval += "<b>Value must be time:</b> ";
		}
	}
	else if ( (type == "file") || (type == "list:file") ) {
		if ( valueTest.empty() ) {
			return eval.size() > evalSize;
		}
		auto value = Seiscomp::Environment::Instance()->absolutePath(valueTest);
		// file must not exist as directory
		QFile dir(value.c_str());
		QFileInfo fileInfo(dir);
		if ( fileInfo.exists() && fileInfo.isDir() ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '" << valueTest
				     << "' file is actually an existing directory" << endl;
			}
			eval += "<b>File is actually a directory:</b> ";
			return eval.size() > evalSize;
		}

		for ( auto &item : param->definition->options ) {
			if ( item == "read" ) {
				// files must exist if tagged as read
				QFile file(value.c_str());
				QFileInfo fileInfo(file);
				// File not found is actually not an error
				if ( !fileInfo.isReadable() ) {
					if ( verbose && !symbolURIString.empty() ) {
						cerr << symbolURIString << param->variableName << " = '"
						     << valueTest << "' readable file must exist"<< endl;
					}
					eval += "<b>Readable file must exist:</b> ";
					break;
				}
			}
			else if ( item == "write" ) {
				QFile file(value.c_str());
				QFileInfo fileInfo(file);
				// File exists and is writable is actually not an error
				if ( fileInfo.isWritable() ) {
					continue;
				}
				// Check if the parent directory exists
				QDir checkDir(QFileInfo(value.c_str()).absolutePath());
				if ( !checkDir.exists() ) {
					if ( verbose && !symbolURIString.empty() ) {
						cerr << symbolURIString << param->variableName << " = '" << valueTest
						     << "' parent directory must exist" << endl;
					}
					eval += "<b>Parent directory must exist:</b> ";
					break;
				}
			}
			else if ( item == "execute" ) {
				// files must be executable if tagged as execute
				QFile file(value.c_str());
				QFileInfo fileInfo(file);
				if ( !valueTest.empty() && !fileInfo.isExecutable() ) {
					if ( verbose && !symbolURIString.empty() ) {
						cerr << symbolURIString << param->variableName << " = '"
						     << valueTest << "' executable file must exist"<< endl;
					}
					eval += "<b>Executable file must exist:</b> ";
					break;
				}
			}
			else {
				continue;
			}
		}
	}
	else if ( (type == "directory") || (type == "list:directory") ) {
		if ( valueTest.empty() ) {
			return eval.size() > evalSize;
		}
		auto value = Seiscomp::Environment::Instance()->absolutePath(valueTest);
		// directory must not exist as file
		QFile dir(value.c_str());
		QFileInfo fileInfo(dir);
		if ( fileInfo.exists() && fileInfo.isFile() ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '" << valueTest
				     << "' directory is actually an existing file" << endl;
			}
			eval += "<b>Directory is actually a file:</b> ";
		}

		// check options
		for ( auto &item : param->definition->options ) {
			if ( item == "read") {
				// directoryies must exist if tagged as read
				QDir dir(value.c_str());
				// Directory not found is actually not an error
				if ( !dir.exists() ) {
					if ( verbose && !symbolURIString.empty() ) {
						cerr << symbolURIString << param->variableName << " = '"
						     << valueTest << "' directory must exist"<< endl;
					}
					eval += "<b>Directory must exist:</b> ";
					break;
				}
			}
			else if ( item == "write" ) {
				// directory must exist or parent directory must be writable
				// and it must not be a file if tagged as read
				// Check if the parent directory exists and is writable
				QString parentDir = QFileInfo(dir).absolutePath();
				QDir parentDirObj(parentDir);
				if ( !parentDirObj.exists() ) {
					if ( verbose && !symbolURIString.empty() ) {
						cerr << symbolURIString << param->variableName << " = '" << valueTest
						     << "' parent directory must exist" << endl;
					}
					eval += "<b>Parent directory must exist:</b> ";
					break;
				}
			}
			else {
				continue;
			}
		}
	}
	// host and port [ip][:port]
	else if ( type == "host-with-port" ) {
		vector<string> toks;
		Seiscomp::Core::split(toks, valueTest, ":", false);
		if ( toks.size() > 2 ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '"
				     << valueTest << "' only one colon allowed"<< endl;
			}
			eval += "<b>Only one colon allowed:</b> ";
		}
		else if ( toks.size() == 2 ) {
			int port;
			if ( !Seiscomp::Core::fromString(port, toks[1]) ||
			     port < 1 || port > 65535 ) {
				if ( verbose && !symbolURIString.empty() ) {
					cerr << symbolURIString << param->variableName << " = '"
					     << valueTest
					     << "' port not a valid integer in range [1, 65535]"
					     << endl;
				}
				eval += "<b>Port not a valid integer in range [1, 65535]:</b> ";
			}
		}
	}
	// gradient
	else if ( type == "gradient" ) {
		// value must contain a colon
		if ( valueTest.find(':') == std::string::npos ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '"
				     << valueTest
				     << "' gradient value must contain ':' " << endl;
			}
			eval += "<b>Gradient value must contain ':'<b>: ";
		}
	}

	// test if values are in range
	if ( !param->definition->range.empty() ) {
		double value;
		vector<string> toks;
		Core::split(toks, param->definition->range.c_str(), ":");
		double rangeMin = std::numeric_limits<double>::min();
		double rangeMax = std::numeric_limits<double>::max();
		if ( toks.size() == 2 ) {
			if ( !Core::fromString(rangeMin, Core::trim(toks[0])) ) {
				if ( verbose ) {
					cerr << "Undescribed range minimum of parameter " << param->definition->name
						 << " : " << param->definition->range << " - assuming "
						 << rangeMin << endl;
				}
			}
			if ( !Core::fromString(rangeMax, Core::trim(toks[1])) ) {
				if ( verbose ) {
					cerr << "Undescribed range maximum of parameter " << param->definition->name
						 << " : " << param->definition->range << " - assuming "
						 << rangeMax << endl;
				}
			}

			if ( Core::fromString(value, valueTest) && (
			     value < rangeMin || value > rangeMax ) ) {
				if ( verbose && !symbolURIString.empty() ) {
					cerr << symbolURIString << param->variableName << " = '"
					     << valueTest << "' is not in range: '"
					     << param->definition->range << "'" << endl;
				}
				eval += "<b>Out of range value:</b> ";
			}
		}
		else {
			if ( verbose ) {
				cerr << "Undescribed range of parameter " << param->definition->name
					 << " : " << param->definition->range << endl;
			}
		}
	}

	return eval.size() > evalSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::updateToolTip(QWidget *w, Seiscomp::System::Parameter *param) {
	bool isOverridden = param->symbol.stage > _configStage;
	vector<string> values;
	QString eval;
	string errmsg;
	if ( Config::Config::Eval(param->symbol.content, values, true, NULL, &errmsg) ) {
		// value must not be a list, strings may contain commas and are not tested
		if ( (param->definition->type.size() < 5)
		     || ((param->definition->type != "string")
		         && (param->definition->type != "gradient")
		         && (param->definition->type.substr(0, 5) != "list:")) ) {
			string valueTest = param->symbol.content;
			// value to test contains a comma which is not supported
			string symbolURIString = param->symbol.uri.empty() ? "" : param->symbol.uri + ": ";
			if ( valueTest.find(',') != std::string::npos ) {
					cerr << symbolURIString << param->variableName << " = '"
					     << valueTest << "' is not described as list " << endl;
				eval += "<b>Value is not described as list</b>";
			}
		}

		for ( const auto& value : values ) {
			if ( !eval.isEmpty() ) {
				eval += "<br/>";
			}
			FancyView::evaluateValue(value, param, eval, true);
			eval += encodeHTML(value.c_str());
		}
	}
	else {
		eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");
	}

	QString toolTip = QString("<b>Location</b><br/>%1<br/><br/>"
	                          "<b>Evaluated</b><br/>%2")
	                  .arg(param->symbol.uri.c_str(), eval);

	if ( isOverridden ) {
		toolTip += QString("<br/><br/><b>WARNING</b><br/><i>This value is overridden in a "
		                   "later stage which supersedes the current stage. "
		                   "Whatever is entered here will not be active in the "
		                   "final configuration. The superseded value is used instead.</i>");
	}

	w->setToolTip(toolTip);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::optionTextEdited() {
	if ( _optionEditHint ) {
		_optionEditHint->hide();
	}

	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) {
		return;
	}

	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(),0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);

	if ( item.input->value() != item.index.sibling(item.index.row(), 2).data() ) {
		model()->setData(item.index.sibling(item.index.row(), 2), item.input->value());
		item.updated();
		updateToolTip(item.input->widget(), param);
	}

	//setFocus(Qt::ActiveWindowFocusReason);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::optionTextChanged(const QString &txt) {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) {
		return;
	}

	if ( !_optionEditHint ) {
		_optionEditHint = new EvalHintWidget(this);
		_optionEditHint->setContentsMargins(6, 6, 6, 6);
	}

	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(),0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);
	vector<string> values;
	QString eval;
	string errmsg;
	QPalette pal = _optionEditHint->palette();
	pal.setColor(QPalette::Window, QColor(255,255,255,192));

	bool issueFound = false;
	if ( Config::Config::Eval(item.input->value().toStdString(), values, true, NULL, &errmsg) ) {
		// value must not be a list, strings may contain commas and are not tested
		if ( (param->definition->type.size() < 5)
		     || (param->definition->type != "string"
		         && (param->definition->type != "gradient")
		         && param->definition->type.substr(0, 5) != "list:") ) {
			string valueTest = item.input->value().toStdString();
			// value to test contains a comma which is not supported
			if ( valueTest.find(',') != std::string::npos ) {
				eval += "<b>Value is not described as list</b>";
				issueFound = true;
			}
		}

		for ( const auto& value : values ) {
			if ( !eval.isEmpty() ) {
				eval += "<hr/>";
			}

			if ( FancyView::evaluateValue(value, param, eval) ) {
				issueFound = true;
			}
			eval += encodeHTML(value.c_str());
		}

		// paint text in orange if issues are found
		pal.setColor(QPalette::WindowText,
		             issueFound ? QColor(255,127,0) : QColor(32,128,32));

		_optionEditHint->setText(QString("<b>Evaluation</b> (%1 item%2)<br/><br/>%3")
		                         .arg(values.size()).arg(values.size() == 1 ? "" : "s", eval));
	}
	else {
		pal.setColor(QPalette::WindowText, QColor(128,32,32));
		eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");

		_optionEditHint->setText(QString("<b>Error</b><br/><br/>%1").arg(eval));
	}

	_optionEditHint->setPalette(pal);
	QSize size = _optionEditHint->sizeHint();
	_optionEditHint->resize(size);
	// Find best position

	QPoint tl = item.input->widget()->mapToGlobal(QPoint(0,0));
	QPoint br = item.input->widget()->mapToGlobal(QPoint(item.input->widget()->width(),
	                                                     item.input->widget()->height()));

	tl -= QPoint(-6,-6);
	br += QPoint(+6,+6);

	tl = mapFromGlobal(tl);
	br = mapFromGlobal(br);

	int x,y;

	if ( tl.x() + size.width() <= width() ) {
		x = tl.x();
	}
	else {
		x = width()-size.width();
		if ( x < 0 ) {
			x = 0;
		}
	}

	if ( br.y() + size.height() <= height() || (height()-br.y()) >= tl.y() ) {
		y = br.y();
	}
	else {
		y = tl.y() - size.height();
	}

	_optionEditHint->move(x,y);
	_optionEditHint->show();

	_blockPopulate = w;
	model()->setData(item.index.sibling(item.index.row(), 2), item.input->value());
	item.updated();
	_blockPopulate = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::optionToggled(bool opt) {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) {
		return;
	}

	model()->setData(item.index.sibling(item.index.row(), 2), item.input->value());
	item.updated();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::bindingCategoryChanged(int idx) {
	/*
	QComboBox *comboBox = static_cast<QComboBox*>(sender());

	FancyViewItem item = comboBox->property("viewItem").value<FancyViewItem>();
	BindingCategory *cat = reinterpret_cast<BindingCategory*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());

	QLayout *l = reinterpret_cast<QLayout*>(comboBox->property("cat.layout").value<void*>());
	QString name;

	if ( idx > 0 )
		name = comboBox->itemData(idx).toString();

	cat->activeBinding = NULL;

	// Update visibility state
	for ( int i = 0; i < l->count(); ++i ) {
		QWidget *child = l->itemAt(i)->widget();
		Binding *b = reinterpret_cast<Binding*>(child->property("viewBinding").value<void*>());
		if ( (b && name == b->definition->name.c_str()) || (!b && idx == 0) ) {
			child->setVisible(true);
			cat->activeBinding = b;
		}
		else
			child->setVisible(false);
	}

	// Update link to trigger model.dataChanged signal
	model()->setData(item.index, QVariant::fromValue((void*)cat), ConfigurationTreeItemModel::Link);
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::addCategoryBinding() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	QComboBox *cb = (QComboBox*)w->property("comboBox").value<void*>();
	QString type = cb->itemData(cb->currentIndex()).toString();
	if ( type.isEmpty() ) {
		QMessageBox::critical(NULL, "Internal error",
		                      "The type must not be empty.");
		return;
	}

	BindingCategory *cat = reinterpret_cast<BindingCategory*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Binding *typeBinding = cat->binding(type.toStdString());
	if ( typeBinding == NULL ) {
		QMessageBox::critical(NULL, "Internal error",
		                      "The selected type is not available.");
		return;
	}

	NewCatBindingDialog dlg(cat, type.toStdString(), this);
	if ( dlg.exec() != QDialog::Accepted ) return;

	Binding *nb = cat->instantiate(typeBinding, dlg.name().c_str());
	if ( nb == NULL ) {
		QMessageBox::critical(NULL, "Internal error",
		                      "Adding binding failed.");
		return;
	}

	const char *alias = cat->alias(nb);

	// Propagate the new entry to the model and create new widgets
	int row = item.index.model()->rowCount(item.index);
	model()->insertRow(row, item.index);
	auto ni = model()->index(row, 0, item.index);
	model()->setData(ni, alias);
	model()->setData(ni, item.index.data(ConfigurationTreeItemModel::Level), ConfigurationTreeItemModel::Level);
	model()->setData(ni, QVariant::fromValue((void*)nb), ConfigurationTreeItemModel::Link);
	model()->setData(ni, ConfigurationTreeItemModel::TypeCategoryBinding, ConfigurationTreeItemModel::Type);

	QWidget *status = (QWidget*)item.container->property("statusLabel").value<void*>();
	if ( status ) status->setVisible(cat->bindings.empty());

	QBoxLayout *l = (QBoxLayout*)w->parentWidget()->layout();
	l->insertWidget(row+1, createWidgetFromIndex(ni, ""));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::removeCategoryBinding() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Binding *b = reinterpret_cast<Binding*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	BindingCategory *c = reinterpret_cast<BindingCategory*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	if ( !c->removeInstance(b) ) {
		cerr << "ERROR: failed to remove binding from category, registered "
		        "bindings: " << c->bindings.size() << endl;
		return;
	}

	QWidget *status = (QWidget*)item.container->property("statusLabel").value<void*>();
	if ( status ) status->setVisible(c->bindings.empty());

	ViewItems::iterator it = _viewItems.find(item.index);
	if ( it != _viewItems.end() )
		_viewItems.erase(it);
	else
		std::cerr << "ERROR: view item does not exist for index" << std::endl;

	model()->removeRow(item.index.row(), item.index.parent());

	if ( item.container ) {
		delete item.container;
	}
}


void FancyView::addStruct() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Structure *s = reinterpret_cast<Structure*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Container *c = reinterpret_cast<Container*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	NewStructDialog dlg(c);
	if ( dlg.exec() != QDialog::Accepted ) {
		return;
	}

	Structure *ns = c->instantiate(s, qPrintable(dlg.name()));
	if ( ns ) {
		// Propagate the new entry to the model and create new widgets
		int row = item.index.row();//model()->rowCount(item.index.parent());
		model()->insertRow(row, item.index.parent());
		auto ni = model()->index(row, 0, item.index.parent());
		model()->setData(ni, ns->name.c_str());
		model()->setData(ni, item.index.data(ConfigurationTreeItemModel::Level), ConfigurationTreeItemModel::Level);
		model()->setData(ni, QVariant::fromValue((void*)ns), ConfigurationTreeItemModel::Link);
		model()->setData(ni, ConfigurationTreeItemModel::TypeStruct, ConfigurationTreeItemModel::Type);

		QBoxLayout *l = (QBoxLayout*)item.container->parentWidget()->layout();
		l->insertWidget(row, createWidgetFromIndex(ni, ""));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::removeStruct() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Structure *s = reinterpret_cast<Structure*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Container *c = reinterpret_cast<Container*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	if ( !c->remove(s) ) {
		cerr << "ERROR: failed to remove structure from container, registered "
		        "structures: " << c->structures.size() << endl;
		return;
	}

	ViewItems::iterator it = _viewItems.find(item.index);
	if ( it != _viewItems.end() ) {
		_viewItems.erase(it);
	}
	else {
		cerr << "ERROR: view item does not exist for index" << endl;
	}

	model()->removeRow(item.index.row(), item.index.parent());

	if ( item.container ) {
		delete item.container;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::updateContentGeometry() {
	if ( !_rootWidget ) {
		return;
	}

	{
		int top = 0, right = 0;
		_btnSearch->resize(_btnSearch->sizeHint());
		if ( _rootWidget->layout() ) {
			top = _rootWidget->layout()->contentsMargins().top();
			right = _rootWidget->layout()->contentsMargins().right();
		}
		_btnSearch->move(
			width() - _btnSearch->width() -
			(verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0) - right,
			top
		);
	}

	QSize p = viewport()->size();
	QSize min = qSmartMinSize(_rootWidget);

	if ((_rootWidget->layout() ? _rootWidget->layout()->hasHeightForWidth() : _rootWidget->sizePolicy().hasHeightForWidth())) {
		QSize p_hfw = p.expandedTo(min);
		int h = _rootWidget->heightForWidth(p_hfw.width());
		min = QSize(p_hfw.width(), qMax(p_hfw.height(), h));
	}

	min = p.expandedTo(min);

	_rootWidget->setGeometry(-horizontalScrollBar()->value(),
	                         -verticalScrollBar()->value(),
	                         min.width(), min.height());

	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setRange(0, qMax(0, _rootWidget->width() - viewport()->width()));
	verticalScrollBar()->setSingleStep(20);
	verticalScrollBar()->setPageStep(viewport()->height());
	verticalScrollBar()->setRange(0, qMax(0, _rootWidget->height() - viewport()->height()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::resizeEvent(QResizeEvent *event) {
	QAbstractItemView::resizeEvent(event);
	updateContentGeometry();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::scrollContentsBy(int dx, int dy) {
	viewport()->scroll(dx, dy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRegion FancyView::visualRegionForSelection(const QItemSelection &selection) const {
	return QRegion();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::currentChanged(const QModelIndex &curr, const QModelIndex &) {
	if ( _currentItem ) {
		static_cast<ViewItemWidget*>(_currentItem)->setSelected(false);
		_currentItem = NULL;
	}

	auto it = _viewItems.find(curr);
	if ( it == _viewItems.end() ) {
		return;
	}

	_currentItem = it.value().container;
	static_cast<ViewItemWidget*>(_currentItem)->setSelected(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::keyboardSearch(const QString &search) {
	if ( _currentItem ) {
		static_cast<ViewItemWidget*>(_currentItem)->setSelected(false);
		_currentItem = NULL;
	}

	// TODO: implement keyboard search
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
