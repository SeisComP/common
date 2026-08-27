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
#include <QAction>
#include <QToolButton>
#include <QTimer>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>
#include <QStandardItemModel>
#include <QCompleter>
#include <QClipboard>
#include <QCalendarWidget>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QStringList>

#include <QScrollArea>
#include <QScrollBar>
#include <QResizeEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <fstream>
#include <functional>
#include <set>

#include <seiscomp/system/environment.h>
#include <seiscomp/config/config.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/gui/core/compat.h>
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


/**
 * @brief Marks all parameters of a container as removed for a given stage.
 *
 * Module configurations are merged with the file on disk. A parameter which
 * is no longer part of the model is not touched by the writer unless it is
 * registered as unknown parameter without symbol. Otherwise the explicitly
 * configured parameters of a deleted structure would survive in the file and
 * recreate the structure at the next startup.
 */
void markRemoved(Module *mod, const Container *cont, int stage) {
	for ( const auto &param : cont->parameters ) {
		auto *unknown = mod->findParameter(param->variableName);
		if ( !unknown ) {
			unknown = new Parameter(nullptr, param->variableName);
			mod->unknowns.push_back(unknown);
		}

		unknown->symbols[stage] = nullptr;
	}

	for ( const auto &group : cont->groups ) {
		markRemoved(mod, group.get(), stage);
	}

	for ( const auto &structure : cont->structures ) {
		markRemoved(mod, structure.get(), stage);
	}
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
				QMessageBox::critical(nullptr, "Empty name",
				                      "Empty names are not allowed. ");
				return;
			}

			if ( _container->hasStructure(qPrintable(_name->text())) ) {
				QMessageBox::critical(nullptr, "Duplicate name",
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
				QMessageBox::critical(nullptr, "Duplicate alias",
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
			_cachedWidth = -1;
			updateGeometry();
		}

		const QString &text() const {
			return _text;
		}

		int heightForWidth(int w) const {
			// Wrapping the text is expensive and the layout asks for the
			// same width over and over again.
			if ( w == _cachedWidth ) {
				return _cachedHeight;
			}

			auto m = contentsMargins();
			int prefHeight =
				fontMetrics().boundingRect(
					0, 0, w - m.left() - m.right(), QWIDGETSIZE_MAX,
					Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, _text
				).height() + m.top() + m.bottom();

			_cachedWidth = w;
			_cachedHeight = prefHeight;

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
		QString     _text;
		mutable int _cachedWidth{-1};
		mutable int _cachedHeight{0};
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

		// Assigns a value programmatically and notifies listeners as if the
		// user had entered it manually. textEdited() runs the evaluation of
		// the new value, editingFinished() commits it.
		void setEditedValue(const QString &value) {
			setText(value);
			emit textEdited(value);
			emit editingFinished();
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

			// Building the completer for every parameter is expensive and
			// only pays off when the field is actually typed in.
			setCompleter(nullptr);
		}

	protected:
		void focusInEvent(QFocusEvent *event) override {
			if ( !completer() && count() ) {
				auto *c = new QCompleter(model(), this);
				c->setCaseSensitivity(Qt::CaseSensitive);
				setCompleter(c);
			}

			QComboBox::focusInEvent(event);
		}

	public:
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


// Input field of a list parameter. It looks and behaves like the combo box of
// a parameter with predefined values, but its drop-down does not open a list
// of items. Instead it opens the window in which the single values are
// selected and ordered.
class ListEdit : public ComboEdit {
	public:
		using ComboEdit::ComboEdit;

		void setPopupHandler(const std::function<void ()> &handler) {
			_popupHandler = handler;
		}

		void showPopup() override {
			if ( _popupHandler ) {
				_popupHandler();
			}
		}

	private:
		std::function<void ()> _popupHandler;
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


// Rewrites an absolute path using SeisComP's path variables (e.g.
// @SYSTEMCONFIGDIR@, @DATADIR@) so the stored value stays portable across
// installations. The variable whose expanded directory is the longest matching
// path-boundary prefix is used, e.g. @KEYDIR@ in favour of @SYSTEMCONFIGDIR@.
// If the path is not located below any known directory it is returned
// unchanged. The result resolves back to the original path via
// Environment::absolutePath().
QString seiscompVariablePath(const QString &path) {
	Environment *env = Environment::Instance();

	struct Mapping {
		const char  *variable;
		std::string  directory;
	};

	// The order is irrelevant: the longest matching directory always wins,
	// which resolves the nesting (e.g. @DATADIR@ below @ROOTDIR@).
	const Mapping mappings[] = {
		{ "@KEYDIR@",           env->appConfigDir() + "/key" },
		{ "@SYSTEMCONFIGDIR@",  env->appConfigDir() },
		{ "@DEFAULTCONFIGDIR@", env->globalConfigDir() },
		{ "@CONFIGDIR@",        env->configDir() },
		{ "@DATADIR@",          env->shareDir() },
		{ "@LOGDIR@",           env->logDir() },
		{ "@ROOTDIR@",          env->installDir() },
		{ "@HOMEDIR@",          env->homeDir() }
	};

	const QString target = QDir::cleanPath(path);

	QString bestValue;
	int bestLength = -1;

	for ( const auto &mapping : mappings ) {
		if ( mapping.directory.empty() ) {
			continue;
		}

		const QString base = QDir::cleanPath(QString::fromStdString(mapping.directory));
		// Skip empty bases and any candidate that cannot beat the current best.
		if ( base.isEmpty() || (base.length() <= bestLength) ) {
			continue;
		}

		if ( target == base ) {
			bestValue = mapping.variable;
			bestLength = base.length();
		}
		else if ( target.startsWith(base + '/') ) {
			// mid() keeps the leading separator, yielding "@VAR@/sub/file".
			bestValue = mapping.variable + target.mid(base.length());
			bestLength = base.length();
		}
	}

	// Path is outside any known SeisComP directory: keep it absolute.
	return bestValue.isEmpty() ? target : bestValue;
}


// Checks whether a module holds parameters which are not maintained by
// scconfig: parameters named "module.trunk.*" and parameters read from a file
// pulled in with "include". The latter are recognized by their origin, they
// come from a file which is not one of the configuration files of the module.
// Standalone modules neither read the global configuration nor consider
// "module.trunk.*" parameters, so both are ignored for them. Modules which do
// not inherit the global bindings ignore the "module.trunk.global.*"
// parameters as well.
class ExternalParameterFinder : public ModelVisitor {
	public:
		ExternalParameterFinder(const Module *mod)
		: _standalone(mod->definition->isStandalone())
		  // The global module is configured by the global parameters itself,
		  // it does not inherit them from another module.
		, _inheritGlobalBinding(mod->definition->name == "global"
		                        || (mod->definition->inheritGlobalBinding
		                            && *mod->definition->inheritGlobalBinding)) {
			for ( int stage = Environment::CS_FIRST;
			      stage <= Environment::CS_LAST; ++stage ) {
				const std::string uri =
				        Environment::Instance()->configFileLocation(
				            mod->definition->name, stage);

				_configFiles.insert(uri);

				if ( (stage == Environment::CS_DEFAULT_GLOBAL)
				     || (stage == Environment::CS_CONFIG_GLOBAL)
				     || (stage == Environment::CS_USER_GLOBAL) ) {
					_globalFiles.insert(uri);
				}
			}
		}

		bool trunk() const { return _trunk; }
		bool included() const { return _included; }

		//! Adds the file a binding is read from to the files of the module
		void addConfigFile(const std::string &uri) {
			_configFiles.insert(uri);
		}

		/**
		 * @brief Checks a binding file for an include statement.
		 *
		 * The file is read directly because a parameter which is not part of
		 * the binding description never reaches the binding itself and the
		 * symbols of the model are keyed by the file which is read, not by
		 * the file a value originates from.
		 */
		void checkBindingFile(const std::string &configFile) {
			addConfigFile(configFile);

			if ( _included || configFile.empty() ) {
				return;
			}

			std::ifstream file(configFile);
			if ( !file.is_open() ) {
				return;
			}

			std::string line;
			while ( std::getline(file, line) ) {
				const std::string statement = Core::trim(line);

				// "include" must be followed by the file to read
				if ( (statement.compare(0, 8, "include ") == 0)
				     || (statement.compare(0, 8, "include\t") == 0) ) {
					_included = true;
					return;
				}
			}
		}

	protected:
		bool visit(Module*) override { return !complete(); }
		bool visit(Section*) override { return !complete(); }
		bool visit(Group*) override { return !complete(); }
		bool visit(Structure*) override { return !complete(); }

		void visit(Parameter *param, bool) override {
			const std::string &uri = param->symbol.uri;

			// A standalone module is not configured by the global files
			if ( _standalone && _globalFiles.count(uri) ) {
				return;
			}

			// "module.trunk.*" parameters do not apply to standalone modules.
			// Without the global bindings the module does not consider the
			// "module.trunk.global.*" parameters either.
			if ( !_standalone
			     && (param->variableName.compare(0, 13, "module.trunk.") == 0)
			     && (_inheritGlobalBinding
			         || (param->variableName.compare(0, 20, "module.trunk.global.") != 0)) ) {
				_trunk = true;
			}

			if ( !uri.empty() && !_configFiles.count(uri) ) {
				_included = true;
			}
		}

	private:
		bool complete() const { return (_trunk || _standalone) && _included; }

		bool                  _standalone{false};
		bool                  _inheritGlobalBinding{false};
		std::set<std::string> _configFiles;
		std::set<std::string> _globalFiles;
		bool                  _trunk{false};
		bool                  _included{false};
};


// Adds a note about the parameters of a module which cannot be adjusted with
// scconfig. If a binding is given its own parameters are considered as well,
// its file may pull in an include of its own. Does nothing if there are no
// such parameters.
void addExternalParameterHint(QBoxLayout *layout, const Module *mod,
                              const ModuleBinding *binding = nullptr) {
	if ( !mod ) {
		return;
	}

	ExternalParameterFinder externalParameters(mod);
	mod->accept(&externalParameters);

	if ( binding ) {
		// Only the file of the binding itself, the include of a global
		// binding is reported when that binding is shown.
		externalParameters.checkBindingFile(binding->configFile);
		binding->accept(&externalParameters);
	}

	QString hintText;
	if ( externalParameters.trunk() && externalParameters.included() ) {
		hintText = QObject::tr("Some parameters start with \"module.trunk.\" or "
		                       "are read from include file. These parameters can "
		                       "only be adjusted outside of scconfig.");
	}
	else if ( externalParameters.trunk() ) {
		hintText = QObject::tr("Some parameters start with \"module.trunk.\". "
		                       "These parameter can only be adjusted outside of "
		                       "scconfig.");
	}
	else if ( externalParameters.included() ) {
		hintText = QObject::tr("Some parameters are read from include file. "
		                       "These parameter can only be adjusted outside of "
		                       "scconfig.");
	}

	if ( hintText.isEmpty() ) {
		return;
	}

	auto *hint = new QLabel;
	hint->setWordWrap(true);

	// The color is set on the text itself, a palette color would be overridden
	// by the style sheet of the theme.
	hint->setText(QString("<span style=\"color:%1;\">%2</span>")
	              .arg(QColor(255, 127, 0).name(), hintText));
	layout->addWidget(hint);
}


// Builds the tooltip of a widget when it is about to be shown. Evaluating the
// value of every parameter while the panel is created is expensive, in
// particular for file and directory parameters which are looked up on disk.
// The issues of a value are reported on the console with the first evaluation
// only, further ones would repeat the same messages on every hover.
class LazyToolTip : public QObject {
	public:
		LazyToolTip(QWidget *watched, const std::function<void (bool)> &build)
		: QObject(watched), _build(build) {
			watched->installEventFilter(this);
		}

	protected:
		bool eventFilter(QObject *watched, QEvent *event) override {
			if ( event->type() == QEvent::ToolTip ) {
				_build(_verbose);
				_verbose = false;
			}

			return QObject::eventFilter(watched, event);
		}

	private:
		std::function<void (bool)> _build;
		bool                       _verbose{true};
};


// A QLabel that invokes a callback when clicked while enabled. Used to make a
// parameter name open the value editor.
class ClickableLabel : public QLabel {
	public:
		using QLabel::QLabel;

		std::function<void()> onClick;

	protected:
		void mouseReleaseEvent(QMouseEvent *event) override {
			if ( isEnabled() && onClick
			     && (event->button() == Qt::LeftButton)
			     && rect().contains(event->pos()) ) {
				onClick();
			}
			QLabel::mouseReleaseEvent(event);
		}
};


// Adds the button opening a selection dialog to the right hand side inside the
// input field, in the same place as the drop-down arrow of a combo box. The
// button is only active while the field is active for input. Returns the action
// to connect to, null if the input widget holds no line edit.
QAction *makeButtonEditor(FancyViewItemEdit *edit, const QString &iconName,
                          const QString &tooltip) {
	auto *field = qobject_cast<QLineEdit*>(edit->widget());
	if ( !field ) {
		// The input field of a combo box is one of its children
		field = edit->widget()->findChild<QLineEdit*>();
	}

	if ( !field ) {
		return nullptr;
	}

	// Owned by 'field', so it is destroyed together with it.
	auto *action = field->addAction(icon(iconName), QLineEdit::TrailingPosition);
	action->setToolTip(tooltip);

	return action;
}


// Builds the file dialog name filter from the parameter's "fileTypeFilter="
// options.
// Each such option provides one file-type filter in Qt's native format, e.g.
// options="read, fileTypeFilter=XML (*.xml);;Config (*.cfg)" or
// options="read, fileTypeFilter=XML (*.xml), fileTypeFilter=Config (*.cfg)"
// Append or prepend ";;All (*.*)" or "All (*.*);;" so that any file can be
// selected.
QString fileTypeFilter(const Parameter *param) {
	QStringList filters;

	for ( const auto &option : param->definition->options ) {
		const QString token = QString::fromStdString(option).trimmed();
		if ( token.startsWith("fileTypeFilter=") ) {
			const QString filter = token.mid(15).trimmed();
			if ( !filter.isEmpty() ) {
				filters << filter;
			}
		}
	}

	return filters.join(";;");
}


// Removes the double quotes protecting a value
QString unquoteValue(const QString &value) {
	const QString text = value.trimmed();

	if ( (text.size() > 1) && text.startsWith('"') && text.endsWith('"') ) {
		return text.mid(1, text.size() - 2);
	}

	return text;
}


// Protects a value with double quotes if it contains the list separator or
// leading or trailing blanks
QString quoteValue(const QString &value) {
	if ( value.contains(',') || (value != value.trimmed()) ) {
		return '"' + value + '"';
	}

	return value;
}


// Splits a configuration value into its items. Commas within double quotes are
// part of the value and do not separate items.
QStringList splitValues(const QString &value) {
	QStringList values;
	QString current;
	bool quoted = false;

	for ( const QChar c : value ) {
		if ( c == '"' ) {
			quoted = !quoted;
		}
		else if ( (c == ',') && !quoted ) {
			values << unquoteValue(current);
			current.clear();
			continue;
		}

		current += c;
	}

	values << unquoteValue(current);

	return values;
}


// Removes duplicate items from a configuration value, keeping the first
// occurrence of each
QString removeDuplicateValues(const QString &value) {
	QStringList values;

	for ( const auto &item : splitValues(value) ) {
		if ( !item.isEmpty() && !values.contains(item) ) {
			values << item;
		}
	}

	QStringList protectedValues;
	for ( const auto &item : values ) {
		protectedValues << quoteValue(item);
	}

	return protectedValues.join(',');
}


// Creates the option removing duplicates from a list of values
QCheckBox *makeUniqueOption() {
	auto *unique = new QCheckBox(QObject::tr("Remove duplicates"));
	unique->setChecked(true);
	unique->setToolTip(QObject::tr("Keep only the first occurrence of a value "
	                               "when the changes are applied"));
	return unique;
}


// True if the parameter holds a list of values
bool isListType(const Parameter *param) {
	return param->definition->type.compare(0, 5, "list:") == 0;
}


// The type of a parameter without the "list:" prefix
std::string baseType(const Parameter *param) {
	return isListType(param) ? param->definition->type.substr(5)
	                         : param->definition->type;
}


// Assigns a value selected in a dialog to the input widget. For list
// parameters the value is appended to the values configured so far.
void commitValue(FancyViewItemEdit *edit, const QString &value, bool append) {
	QString text = value;

	if ( append ) {
		const QString current = edit->value().trimmed();
		if ( !current.isEmpty() ) {
			text = current + "," + value;
		}
	}

	// StringEdit needs an explicit editingFinished() to commit the value, the
	// combo box commits through its own change signal.
	if ( auto *stringEdit = dynamic_cast<StringEdit*>(edit) ) {
		stringEdit->setEditedValue(text);
	}
	else {
		edit->setValue(text);
	}
}


// Parses a color given as a W3C color keyword, as hexadecimal digits with an
// optional leading '#' (RGB, RGBA, RRGGBB or RRGGBBAA) or in the functional
// notation rgb(r,g,b) or rgba(r,g,b,a).
bool parseColor(const QString &value, QColor &color) {
	QString text = value.trimmed();
	if ( text.isEmpty() ) {
		return false;
	}

	// Color keywords and the notations known to Qt
	QColor named(text);
	if ( named.isValid() ) {
		color = named;
		return true;
	}

	if ( text.startsWith("rgb(") || text.startsWith("rgba(") ) {
		if ( !text.endsWith(')') ) {
			return false;
		}

		const int start = text.indexOf('(') + 1;
		const QStringList tokens = text.mid(start, text.size() - start - 1).split(',');

		if ( (tokens.size() < 3) || (tokens.size() > 4) ) {
			return false;
		}

		int rgba[4] = { 0, 0, 0, 255 };
		for ( int i = 0; i < tokens.size(); ++i ) {
			bool ok = false;
			rgba[i] = tokens[i].trimmed().toInt(&ok);
			if ( !ok || (rgba[i] < 0) || (rgba[i] > 255) ) {
				return false;
			}
		}

		color = QColor(rgba[0], rgba[1], rgba[2], rgba[3]);
		return true;
	}

	// Hexadecimal digits without the leading '#' expected by Qt
	if ( text.startsWith('#') ) {
		text = text.mid(1);
	}

	switch ( text.size() ) {
		case 3:
		case 6:
			named = QColor("#" + text);
			break;

		case 4:
			// Qt expects the alpha channel first
			named = QColor("#" + text.right(1) + text.left(3));
			break;

		case 8:
			named = QColor("#" + text.right(2) + text.left(6));
			break;

		default:
			return false;
	}

	if ( !named.isValid() ) {
		return false;
	}

	color = named;
	return true;
}


// The notations a color can be written in
enum ColorFormat {
	ColorHex = 0,
	ColorKeyword,
	ColorFunction
};


// The notation a value is written in, hexadecimal if it cannot be told
ColorFormat colorFormatOf(const QString &value) {
	const QString text = value.trimmed();

	if ( text.startsWith("rgb(") || text.startsWith("rgba(") ) {
		return ColorFunction;
	}

	// A keyword contains no digits and no leading '#'
	if ( !text.isEmpty() && !text.startsWith('#') && text[0].isLetter()
	     && QColor(text).isValid() ) {
		return ColorKeyword;
	}

	return ColorHex;
}


// The W3C color keyword of a color, empty if the color has no name. The alpha
// channel cannot be expressed by a keyword.
QString colorKeyword(const QColor &color) {
	if ( color.alpha() != 255 ) {
		return {};
	}

	const QString name = color.name(QColor::HexRgb);
	for ( const auto &keyword : QColor::colorNames() ) {
		if ( QColor(keyword).name(QColor::HexRgb) == name ) {
			return keyword;
		}
	}

	return {};
}


// Prints a color in the functional notation, the alpha channel is only added
// if the color is not opaque.
QString colorFunction(const QColor &color) {
	if ( color.alpha() != 255 ) {
		return QString("rgba(%1,%2,%3,%4)")
		       .arg(color.red()).arg(color.green())
		       .arg(color.blue()).arg(color.alpha());
	}

	return QString("rgb(%1,%2,%3)")
	       .arg(color.red()).arg(color.green()).arg(color.blue());
}


// Prints a color as hexadecimal digits, the alpha channel is only added if the
// color is not opaque.
QString formatColor(const QColor &color) {
	QString text = QString("%1%2%3")
	               .arg(color.red(), 2, 16, QLatin1Char('0'))
	               .arg(color.green(), 2, 16, QLatin1Char('0'))
	               .arg(color.blue(), 2, 16, QLatin1Char('0'));

	if ( color.alpha() != 255 ) {
		text += QString("%1").arg(color.alpha(), 2, 16, QLatin1Char('0'));
	}

	return text.toUpper();
}


// A square filled with the given color, shown inside the input field
QIcon colorSwatch(const QColor &color) {
	QPixmap pixmap(QApplication::fontMetrics().ascent(),
	               QApplication::fontMetrics().ascent());
	pixmap.fill(color.isValid() ? color : QColor(Qt::transparent));

	QPainter p(&pixmap);
	p.setPen(QApplication::palette().color(QPalette::Mid));
	p.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);
	p.end();

	// The color in use is also shown while the parameter is locked, so the
	// same pixmap is used for the disabled state instead of the faded one
	// Qt would generate.
	QIcon swatch;
	swatch.addPixmap(pixmap, QIcon::Normal);
	swatch.addPixmap(pixmap, QIcon::Disabled);
	swatch.addPixmap(pixmap, QIcon::Active);
	swatch.addPixmap(pixmap, QIcon::Selected);

	return swatch;
}


// The colors of the scheme section of the global module. They are offered as
// basic colors so that a configuration stays consistent with the appearance of
// the GUI applications. The list is sorted by shades of grey first and by hue
// afterwards. Alpha channels are not part of it, the transparency is chosen in
// the dialog.
const char *SchemeColors[] = {
	"#000000", "#202020", "#666666", "#808080", "#999999",
	"#A0A0A4", "#C0C0C0", "#CCCCCC", "#FFFFFF",
	"#A00000", "#FF0000", "#FF6633", "#FFA000", "#FFFD00", "#FFFF00",
	"#82AD58", "#00A000", "#00FF00", "#00FFFF",
	"#02589E", "#C0C0FF", "#0000A0", "#FF00FF"
};


// Keeps the button picking a color from the screen marked as long as the
// current color is the one which was picked. Qt offers no signal for the end
// of the picking, it stops with the mouse button being released.
class ScreenPickWatcher : public QObject {
	public:
		ScreenPickWatcher(QColorDialog *chooser, QPushButton *button)
		: QObject(button), _button(button) {
			chooser->installEventFilter(this);

			connect(button, &QPushButton::clicked, this, [this]() {
				_picking = true;
				_button->setChecked(true);
			});

			// Any other way of choosing a color releases the button
			connect(chooser, &QColorDialog::currentColorChanged, this, [this]() {
				if ( !_picking ) {
					_button->setChecked(false);
				}
			});
		}

	protected:
		bool eventFilter(QObject *watched, QEvent *event) override {
			if ( _picking && (event->type() == QEvent::MouseButtonRelease) ) {
				// The color under the cursor was taken over
				_picking = false;
			}

			return QObject::eventFilter(watched, event);
		}

	private:
		QPushButton *_button;
		bool         _picking{false};
};


// Inserts a widget into the box layout holding 'sibling', right in front of
// it. Returns whether a layout containing the sibling was found.
bool insertBefore(QLayout *layout, QWidget *sibling, QWidget *widget) {
	if ( !layout ) {
		return false;
	}

	for ( int i = 0; i < layout->count(); ++i ) {
		auto *item = layout->itemAt(i);

		if ( item->widget() == sibling ) {
			auto *box = qobject_cast<QBoxLayout*>(layout);
			if ( !box ) {
				return false;
			}

			box->insertWidget(i, widget);
			return true;
		}

		if ( insertBefore(item->layout(), sibling, widget) ) {
			return true;
		}
	}

	return false;
}


// A drop-down offering the W3C color keywords, each shown with a swatch of
// the color it stands for.
QComboBox *makeKeywordSelector(QColorDialog *chooser) {
	auto *keywords = new QComboBox;
	keywords->setToolTip(QObject::tr("Select a color by its keyword"));

	// The first entry is a placeholder, selecting it does not change the
	// color.
	keywords->addItem(QObject::tr("Color keyword..."));

	for ( const auto &name : QColor::colorNames() ) {
		keywords->addItem(colorSwatch(QColor(name)), name);
	}

	QObject::connect(keywords, QOverload<int>::of(&QComboBox::activated),
	                 chooser, [chooser, keywords](int index) {
		if ( index > 0 ) {
			chooser->setCurrentColor(QColor(keywords->itemText(index)));
		}
	});

	return keywords;
}


// Creates a composite editor for "color" parameters: the given input widget
// plus a swatch inside it which opens the color selection dialog.
QWidget *makeColorEditor(StringEdit *edit, const Parameter *param) {
	auto *select = makeButtonEditor(edit, "add", QObject::tr("Select a color"));
	if ( !select ) {
		return edit;
	}

	// The swatch shows the color which is configured
	auto showColor = [edit, select]() {
		QColor color;
		parseColor(edit->value(), color);
		select->setIcon(colorSwatch(color));
	};

	QObject::connect(edit, &QLineEdit::textChanged, edit,
	                 [showColor](const QString &) { showColor(); });
	showColor();

	const QString title = QObject::tr("Select the color for %1")
	                      .arg(QString::fromStdString(param->variableName));

	QObject::connect(select, &QAction::triggered, edit, [edit, title]() {
		QColor current;
		if ( !parseColor(edit->value(), current) ) {
			current = Qt::white;
		}

		QDialog dialog(edit);
		dialog.setWindowTitle(title);

		auto *layout = new QVBoxLayout(&dialog);

		// The dialog is embedded so that the notation to store can be
		// chosen along with the color.
		auto *chooser = new QColorDialog(current);
		chooser->setWindowFlags(Qt::Widget);
		chooser->setOptions(QColorDialog::ShowAlphaChannel |
		                    QColorDialog::NoButtons |
		                    QColorDialog::DontUseNativeDialog);

		// Offer the colors of the scheme as basic colors. They are a global
		// setting of the dialog, so they are set before it is shown.
		for ( size_t i = 0; i < sizeof(SchemeColors) / sizeof(*SchemeColors); ++i ) {
			QColorDialog::setStandardColor(i, QColor(SchemeColors[i]));
		}

		// Offer the color keywords above the button picking a color from the
		// screen, which is the only push button left after NoButtons.
		auto *keywords = makeKeywordSelector(chooser);

		// The children exist once the layout of the dialog was created
		chooser->ensurePolished();
		auto *screenPicker = chooser->findChild<QPushButton*>();

		if ( screenPicker ) {
			// The button is marked while the color comes from the screen, it
			// must not stay the highlighted default button afterwards.
			screenPicker->setAutoDefault(false);
			screenPicker->setCheckable(true);

			// Owned by the button, so it is destroyed together with it.
			new ScreenPickWatcher(chooser, screenPicker);
		}

		if ( !screenPicker
		     || !insertBefore(chooser->layout(), screenPicker, keywords) ) {
			// The dialog does not look as expected, offer the keywords below
			// it rather than not at all.
			layout->addWidget(keywords);
		}

		layout->addWidget(chooser);

		auto *format = new QComboBox;
		format->addItem(QObject::tr("Hexadecimal"), ColorHex);
		format->addItem(QObject::tr("Color keyword"), ColorKeyword);
		format->addItem(QObject::tr("rgb() or rgba()"), ColorFunction);
		format->setCurrentIndex(colorFormatOf(edit->value()));

		// A keyword is only available for a color which has one
		auto updateFormats = [chooser, format]() {
			const bool named = !colorKeyword(chooser->currentColor()).isEmpty();

			auto *model = qobject_cast<QStandardItemModel*>(format->model());
			if ( auto *item = model ? model->item(ColorKeyword) : nullptr ) {
				item->setEnabled(named);
			}

			if ( !named && (format->currentIndex() == ColorKeyword) ) {
				format->setCurrentIndex(ColorHex);
			}
		};

		QObject::connect(chooser, &QColorDialog::currentColorChanged,
		                 format, [updateFormats]() { updateFormats(); });
		updateFormats();

		auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
		                                     QDialogButtonBox::Cancel);
		QObject::connect(buttons, &QDialogButtonBox::accepted,
		                 &dialog, &QDialog::accept);
		QObject::connect(buttons, &QDialogButtonBox::rejected,
		                 &dialog, &QDialog::reject);

		// The notation to store shares the line with the buttons
		auto *bottom = new QHBoxLayout;
		bottom->setContentsMargins(0, 0, 0, 0);
		bottom->addWidget(new QLabel(QObject::tr("Store as")));
		bottom->addWidget(format);
		bottom->addStretch();
		bottom->addWidget(buttons);
		layout->addLayout(bottom);

		// Confirming the dialog is the default action
		if ( auto *okButton = buttons->button(QDialogButtonBox::Ok) ) {
			okButton->setAutoDefault(true);
			okButton->setDefault(true);
		}

		if ( dialog.exec() != QDialog::Accepted ) {
			// Dialog cancelled: keep the current value untouched.
			return;
		}

		const QColor color = chooser->currentColor();
		if ( !color.isValid() ) {
			return;
		}

		QString value;
		switch ( format->currentIndex() ) {
			case ColorKeyword:
				value = colorKeyword(color);
				break;

			case ColorFunction:
				value = colorFunction(color);
				break;

			default:
				break;
		}

		if ( value.isEmpty() ) {
			value = formatColor(color);
		}

		// A color was actively chosen, so unlock the parameter (mark it as
		// edited) in case it was still showing its default value.
		FancyViewItem item = edit->property("viewItem").value<FancyViewItem>();
		if ( item.isValid() && item.editControl && item.editControl->isChecked() ) {
			item.editControl->setChecked(false);
		}

		commitValue(edit, value, false);
	});

	return edit;
}


// Creates a composite editor for "file" and "directory" parameters: the given
// line edit plus a button which opens a file or directory selection dialog. The
// selected path is written back into the line edit and committed through its
// regular editing-finished path.
// Opens the file or directory selection dialog for a parameter. 'current' is
// the path shown initially. Returns whether a path was selected and stores it
// in 'value' using SeisComP's path variables when possible.
bool selectPath(QWidget *parent, const Parameter *param, const QString &current,
                QString &value) {
	const bool isDirectory = (baseType(param) == "directory");

	// For files, determine whether the parameter requires an existing file
	// (read/execute) or may point to a not yet existing file (write or
	// unspecified). Directories are always selected from existing ones.
	bool mustExist = false;
	bool isWrite = false;
	for ( const auto &option : param->definition->options ) {
		if ( (option == "read") || (option == "execute") ) {
			mustExist = true;
		}
		else if ( option == "write" ) {
			isWrite = true;
		}
	}

	// 'edit' is passed as the connection context so the lambda is removed
	// automatically when the line edit is destroyed. The dialog header names the
	// parameter the file or directory is selected for. File parameters offer the
	// file-type filters from the "filter=" options plus an "All (*.*)" entry.
	const QString title = (isDirectory ? QObject::tr("Select directory for %1")
	                                    : QObject::tr("Select file for %1"))
	                      .arg(QString::fromStdString(param->variableName));
	const QString nameFilter = isDirectory ? QString() : fileTypeFilter(param);

	// Resolve the current value to an absolute path used as the dialog's start
	// location. Empty or relative values are resolved against the SeisComP
	// installation directory, matching value validation.
	QString startPath;
	if ( !current.trimmed().isEmpty() ) {
		startPath = Environment::Instance()->absolutePath(
		                current.trimmed().toStdString()).c_str();
	}
	else {
		startPath = Environment::Instance()->installDir().c_str();
	}

	QString selection;
	if ( isDirectory ) {
		selection = QFileDialog::getExistingDirectory(parent, title, startPath);
	}
	else if ( isWrite && !mustExist ) {
		// Output file: allow selecting a not yet existing file without an
		// overwrite confirmation. The non-native dialog is used so the file
		// type selector shows the full filter string including the pattern
		// (e.g. "Text (*.xml)") instead of only the description text.
		selection = QFileDialog::getSaveFileName(
		                parent, title, startPath, nameFilter, nullptr,
		                QFileDialog::DontConfirmOverwrite | QFileDialog::DontUseNativeDialog);
	}
	else {
		selection = QFileDialog::getOpenFileName(
		                parent, title, startPath, nameFilter, nullptr,
		                QFileDialog::DontUseNativeDialog);
	}

	if ( selection.isEmpty() ) {
		// Dialog cancelled: keep the current value untouched.
		return false;
	}

	// Store the path using SeisComP's path variables when possible so the
	// configuration remains portable across installations.
	value = seiscompVariablePath(selection);

	return true;
}


// Creates a composite editor for "file" and "directory" parameters: the given
// input widget plus a button which opens the selection dialog.
QWidget *makePathEditor(StringEdit *edit, const Parameter *param) {
	const bool isDirectory = (baseType(param) == "directory");

	auto *browse = makeButtonEditor(
	                   edit, "folder",
	                   isDirectory ? QObject::tr("Select a directory")
	                               : QObject::tr("Select a file"));

	if ( !browse ) {
		return edit;
	}

	// 'edit' is passed as the connection context so the lambda is removed
	// automatically when the line edit is destroyed.
	QObject::connect(browse, &QAction::triggered, edit, [edit, param]() {
		QString value;
		if ( !selectPath(edit, param, edit->value(), value) ) {
			return;
		}

		// A path was actively chosen, so unlock the parameter (mark it as
		// edited) in case it was still showing its default value.
		FancyViewItem item = edit->property("viewItem").value<FancyViewItem>();
		if ( item.isValid() && item.editControl && item.editControl->isChecked() ) {
			item.editControl->setChecked(false);
		}

		commitValue(edit, value, false);
	});

	return edit;
}


bool selectTime(QWidget *parent, const Parameter *param, const QString &current,
                QString &value);


// Editor for list parameters. Below the input field with the comma separated
// value it shows one row per value: the configured ones first in their
// configured order, followed by the predefined values which are not
// configured. Values are activated with their check box, renamed unless they
// are predefined and reordered by dragging their handle. Every change
// repopulates the input field.
class ListEditor : public QWidget {
	public:
		ListEditor(FancyViewItemEdit *edit, const Parameter *param, QWidget *parent = nullptr)
		: QWidget(parent), _edit(edit), _param(param) {
			for ( const auto &value : param->definition->values ) {
				// The schema may deliver the values as separate entries or as
				// one comma separated string. Quoted values are unprotected.
				_values << splitValues(value.c_str());
			}

			_values.removeAll(QString());

			_rows = new QWidget;
			_rowLayout = new QVBoxLayout(_rows);
			_rowLayout->setContentsMargins(0, 0, 0, 0);
			_rowLayout->setSpacing(layoutPadding() / 2);

			_scroll = new QScrollArea;
			_scroll->setFrameShape(QFrame::NoFrame);
			_scroll->setWidgetResizable(true);
			_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			_scroll->setWidget(_rows);

			_add = new QToolButton;
			_add->setIcon(icon("add"));
			_add->setAutoRaise(true);
			_add->setToolTip(tr("Add a value"));

			auto *layout = new QVBoxLayout(this);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(layoutPadding() / 2);
			layout->addWidget(_scroll);
			layout->addWidget(_add, 0, Qt::AlignLeft);

			QObject::connect(_add, &QToolButton::clicked, this, [this]() {
				appendRow(QString(), true, false);
				updateHeight();

				// Let the new value be typed right away
				auto *row = _rowLayout->itemAt(_rowLayout->count() - 1)->widget();
				auto *text = row->findChild<QLineEdit*>();
				if ( text ) {
					text->setFocus(Qt::OtherFocusReason);
				}
			});

			// Values typed into the input field are mirrored into the rows.
			// A combo box is edited through its internal line edit.
			auto *field = qobject_cast<QLineEdit*>(_edit->widget());
			if ( !field ) {
				field = _edit->widget()->findChild<QLineEdit*>();
			}

			if ( field ) {
				QObject::connect(field, &QLineEdit::textChanged, this, [this]() {
					if ( !_updating ) {
						showValue();
					}
				});
			}

			showValue();
		}

	protected:
		//! Reorders the rows while a handle is dragged
		bool eventFilter(QObject *watched, QEvent *event) override {
			auto *handle = qobject_cast<QWidget*>(watched);
			if ( !handle ) {
				return QWidget::eventFilter(watched, event);
			}

			switch ( event->type() ) {
				case QEvent::MouseButtonPress:
					_dragRow = rowIndex(handle->parentWidget());
					return true;

				case QEvent::MouseMove: {
					if ( _dragRow < 0 ) {
						break;
					}

					// The rows may have been rebuilt since the drag started
					if ( _dragRow >= _rowLayout->count() ) {
						_dragRow = -1;
						break;
					}

					int target = rowAt(_rows->mapFromGlobal(QCursor::pos()).y());
					if ( (target >= 0) && (target != _dragRow) ) {
						auto *entry = _rowLayout->takeAt(_dragRow);
						if ( entry ) {
							_rowLayout->insertWidget(target, entry->widget());
							delete entry;
							_dragRow = target;
						}
					}

					return true;
				}

				case QEvent::MouseButtonRelease:
					if ( _dragRow >= 0 ) {
						_dragRow = -1;
						commit();
						return true;
					}
					break;

				default:
					break;
			}

			return QWidget::eventFilter(watched, event);
		}

	private:
		//! Rebuilds the rows from the current content of the input field
		void showValue() {
			// The rows are replaced, any running drag refers to gone widgets
			_dragRow = -1;

			while ( _rowLayout->count() ) {
				auto *entry = _rowLayout->takeAt(0);
				delete entry->widget();
				delete entry;
			}

			QStringList configured;
			for ( const auto &value : splitValues(_edit->value()) ) {
				if ( !value.isEmpty() ) {
					configured << value;
					appendRow(value, true, _values.contains(value));
				}
			}

			// The predefined values stay available, the ones which are not
			// configured are appended as inactive rows.
			for ( const auto &value : _values ) {
				if ( !configured.contains(value) ) {
					appendRow(value, false, true);
				}
			}

			updateHeight();
		}

		void appendRow(const QString &value, bool active, bool predefined) {
			auto *row = new QWidget;
			auto *rowLayout = new QHBoxLayout(row);
			rowLayout->setContentsMargins(0, 0, 0, 0);
			rowLayout->setSpacing(layoutPadding() / 2);

			auto *handle = new QLabel("::");
			handle->setToolTip(tr("Drag to change the order"));
			handle->setCursor(Qt::SizeVerCursor);
			handle->installEventFilter(this);

			// A predefined value is taken in and out of the configuration
			// with its check box, it is never removed from the list. An
			// additional value is removed with its button instead.
			auto *check = new QCheckBox;
			check->setChecked(active);
			check->setToolTip(tr("Add this value to the configuration"));
			check->setVisible(predefined);

			// Predefined values must stay available, only additional values
			// can be renamed or removed.
			auto *text = new QLineEdit(value);
			text->setReadOnly(predefined);
			text->setFrame(!predefined);

			auto *remove = new QToolButton;
			remove->setIcon(icon("delete"));
			remove->setAutoRaise(true);
			remove->setToolTip(tr("Remove this value"));
			remove->setVisible(!predefined);

			rowLayout->addWidget(check);
			rowLayout->addWidget(remove);
			rowLayout->addWidget(text, 1);

			// The single values are selected with the same dialog as the
			// value of a parameter which is not a list.
			const std::string type = baseType(_param);
			const bool isPath = (type == "file") || (type == "directory");

			if ( (type == "time") || isPath ) {
				// Turns the row into a value of its own, the predefined value
				// it may have shown is offered again at the end of the list.
				auto assign = [this, text, check, remove](const QString &value) {
					text->setReadOnly(false);
					text->setFrame(true);
					check->setVisible(false);
					remove->setVisible(true);
					text->setText(value);

					ensurePredefined();
					commit();
				};

				auto *select = new QToolButton;
				select->setIcon(icon(isPath ? "folder" : "calendar"));
				select->setAutoRaise(true);
				select->setToolTip(isPath ?
				                   (type == "directory" ?
				                    tr("Select a directory") : tr("Select a file")) :
				                   tr("Select date and time"));
				rowLayout->addWidget(select);

				QObject::connect(select, &QToolButton::clicked, this,
				                 [this, text, isPath, assign]() {
					QString value;

					if ( isPath ) {
						if ( selectPath(this, _param, text->text(), value) ) {
							assign(value);
						}
					}
					else if ( selectTime(this, _param, text->text(), value) ) {
						assign(value);
					}
				});
			}

			rowLayout->addWidget(handle);

			QObject::connect(check, &QCheckBox::toggled, this, [this]() {
				commit();
			});
			QObject::connect(text, &QLineEdit::textEdited, this, [this]() {
				commit();
			});
			QObject::connect(text, &QLineEdit::editingFinished, this, [this]() {
				// A renamed row may have carried a predefined value
				ensurePredefined();
			});
			QObject::connect(remove, &QToolButton::clicked, this, [this, row]() {
				int index = rowIndex(row);
				if ( index >= 0 ) {
					auto *entry = _rowLayout->takeAt(index);
					delete entry->widget();
					delete entry;
					ensurePredefined();
					updateHeight();
					commit();
				}
			});

			_rowLayout->addWidget(row);
		}

		//! Predefined values stay available. Those which are not shown as a
		//! row anymore are appended as inactive rows at the end of the list.
		void ensurePredefined() {
			QStringList shown;

			for ( int i = 0; i < _rowLayout->count(); ++i ) {
				auto *text = _rowLayout->itemAt(i)->widget()->findChild<QLineEdit*>();
				if ( text ) {
					shown << text->text().trimmed();
				}
			}

			for ( const auto &value : _values ) {
				if ( !shown.contains(value) ) {
					appendRow(value, false, true);
				}
			}

			updateHeight();
		}

		//! Writes the activated values back into the input field
		void commit() {
			QStringList values;

			for ( int i = 0; i < _rowLayout->count(); ++i ) {
				auto *row = _rowLayout->itemAt(i)->widget();
				auto *check = row->findChild<QCheckBox*>();
				auto *text = row->findChild<QLineEdit*>();
				if ( !check || !text ) {
					continue;
				}

				// Additional values have no check box, they are configured
				// as long as they are in the list
				if ( !check->isHidden() && !check->isChecked() ) {
					continue;
				}

				const QString value = text->text();
				if ( !value.trimmed().isEmpty() ) {
					values << quoteValue(value);
				}
			}

			_updating = true;
			commitValue(_edit, values.join(','), false);
			_updating = false;
		}

		int rowIndex(const QWidget *row) const {
			for ( int i = 0; i < _rowLayout->count(); ++i ) {
				if ( _rowLayout->itemAt(i)->widget() == row ) {
					return i;
				}
			}

			return -1;
		}

		int rowAt(int y) const {
			for ( int i = 0; i < _rowLayout->count(); ++i ) {
				const QRect geometry = _rowLayout->itemAt(i)->widget()->geometry();
				if ( y < geometry.bottom() ) {
					return i;
				}
			}

			return _rowLayout->count() - 1;
		}

		//! Shows at most 10 rows, the remaining ones are scrolled to
		void updateHeight() {
			_rows->adjustSize();

			const int rows = _rowLayout->count();
			if ( !rows ) {
				_scroll->setFixedHeight(0);
				return;
			}

			const int rowHeight = _rowLayout->itemAt(0)->widget()->sizeHint().height();

			// Room for one more row than currently shown, so adding a value
			// does not resize the list and the window stays large enough.
			const int visible = qMin(rows + 1, 10);

			_scroll->setFixedHeight(visible * rowHeight +
			                        (visible - 1) * _rowLayout->spacing() +
			                        layoutPadding());

			// Let the window grow with the list rather than squeezing the
			// widgets below it. It is never shrunk.
			if ( auto *win = window() ) {
				if ( win->layout() ) {
					// The size hint must account for the new height
					win->layout()->activate();
				}

				win->resize(win->size().expandedTo(win->sizeHint()));
			}
		}

		FancyViewItemEdit *_edit{nullptr};
		const Parameter   *_param{nullptr};
		QStringList  _values;
		QScrollArea *_scroll{nullptr};
		QToolButton *_add{nullptr};
		QWidget     *_rows{nullptr};
		QVBoxLayout *_rowLayout{nullptr};
		int          _dragRow{-1};
		bool         _updating{false};
};


// Splits the range of a time parameter into its begin and end value. Time
// values contain ':' themselves, so the range separator is '..' rather than
// the ':' used for numeric ranges. Either side may be empty for an open range.
void splitTimeRange(const QString &range, QString &begin, QString &end) {
	const QStringList tokens = range.split("..");
	if ( tokens.size() != 2 ) {
		return;
	}

	begin = tokens[0].trimmed();
	end = tokens[1].trimmed();
}


// A parsed time value, invalid if the string could not be interpreted.
struct ParsedTime {
	bool       valid{false};
	Core::Time time;
};


ParsedTime parseTime(const QString &value) {
	ParsedTime parsed;
	parsed.valid = !value.isEmpty()
	            && parsed.time.fromString(value.trimmed().toStdString());
	return parsed;
}


// Prints a time at microsecond precision.
QString formatTime(const Core::Time &time) {
	return time.toString("%FT%T.%6f").c_str();
}


// Prints a time with the trailing zeros of the fractional seconds removed,
// keeping at least one decimal place.
QString formatTimeCompact(const Core::Time &time) {
	QString text = formatTime(time);

	int last = text.size() - 1;
	while ( (last > 0) && (text[last] == '0') && (text[last - 1] != '.') ) {
		--last;
	}

	return text.left(last + 1);
}


// Prints a time at microsecond precision. The fractional seconds are omitted
// if they are zero.
QString formatTimeValue(const Core::Time &time) {
	return time.microseconds() ? formatTime(time)
	                           : time.toString("%FT%T").c_str();
}


// The date of a parsed time, invalid if there is none.
QDate timeDate(const ParsedTime &parsed) {
	if ( !parsed.valid ) {
		return {};
	}

	return QDateTime::fromString(formatTime(parsed.time).left(19),
	                             "yyyy-MM-ddTHH:mm:ss").date();
}


// Line edit for ISO times which steps the value under the cursor with the
// mouse wheel and the up and down keys. All fields up to the seconds are
// stepped, the fractional seconds are typed.
class TimeEdit : public QLineEdit {
	public:
		TimeEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

		//! Restricts the values which can be reached by stepping
		void setLimits(const ParsedTime &min, const ParsedTime &max) {
			_min = min;
			_max = max;
		}

	protected:
		void keyPressEvent(QKeyEvent *event) override {
			if ( event->key() == Qt::Key_Up ) {
				step(cursorPosition(), 1);
				return;
			}

			if ( event->key() == Qt::Key_Down ) {
				step(cursorPosition(), -1);
				return;
			}

			QLineEdit::keyPressEvent(event);
		}

		void wheelEvent(QWheelEvent *event) override {
			const int delta = QT_WE_DELTA(event);
			if ( delta ) {
				// Step the field the mouse points at, not the one the text
				// cursor happens to be in.
				step(cursorPositionAt(QT_WE_POS(event)), delta > 0 ? 1 : -1);
			}

			event->accept();
		}

	private:
		void step(int pos, int amount) {
			const QString value = text().trimmed();

			// The date and time part is of fixed length, everything beyond
			// it is the fractional seconds which are kept as they are.
			QDateTime dateTime = QDateTime::fromString(value.left(19),
			                                           "yyyy-MM-ddTHH:mm:ss");
			if ( !dateTime.isValid() ) {
				return;
			}

			if ( pos <= 4 ) {
				dateTime = dateTime.addYears(amount);
			}
			else if ( pos <= 7 ) {
				dateTime = dateTime.addMonths(amount);
			}
			else if ( pos <= 10 ) {
				dateTime = dateTime.addDays(amount);
			}
			else if ( pos <= 13 ) {
				dateTime = dateTime.addSecs(amount * 3600);
			}
			else if ( pos <= 16 ) {
				dateTime = dateTime.addSecs(amount * 60);
			}
			else if ( pos <= 19 ) {
				dateTime = dateTime.addSecs(amount);
			}
			else {
				// Fractional seconds are not stepped
				return;
			}

			QString fraction = value.mid(19);
			if ( !fraction.startsWith('.') ) {
				fraction = ".0";
			}

			QString stepped = dateTime.toString("yyyy-MM-ddTHH:mm:ss") + fraction;

			// Do not step out of the range of the parameter
			const ParsedTime parsed = parseTime(stepped);
			if ( parsed.valid ) {
				if ( _min.valid && (parsed.time < _min.time) ) {
					stepped = formatTimeCompact(_min.time);
				}
				else if ( _max.valid && (parsed.time > _max.time) ) {
					stepped = formatTimeCompact(_max.time);
				}
			}

			setText(stepped);
			setCursorPosition(pos);
		}

		ParsedTime _min;
		ParsedTime _max;
};


// The limits of the value range as text lines, ready to be shown above an
// editor. Times are printed at microsecond precision, other types are shown as
// configured. Either limit may be missing for an open range.
QStringList formatRangeLines(const Parameter *param) {
	const QString range = QString::fromStdString(param->definition->range);
	QString begin, end;

	if ( baseType(param) == "time" ) {
		splitTimeRange(range, begin, end);

		const ParsedTime min = parseTime(begin);
		const ParsedTime max = parseTime(end);
		begin = min.valid ? formatTime(min.time) : QString();
		end = max.valid ? formatTime(max.time) : QString();
	}
	else {
		// The limits are separated by the commonly used '..' or by ':'
		int separator = range.indexOf("..");
		int width = 2;

		if ( separator < 0 ) {
			separator = range.indexOf(':');
			width = 1;
		}

		if ( separator >= 0 ) {
			begin = range.left(separator).trimmed();
			end = range.mid(separator + width).trimmed();
		}
	}

	QStringList lines;

	if ( !begin.isEmpty() ) {
		lines << (begin + " -");
	}

	if ( !end.isEmpty() ) {
		lines << end;
	}

	return lines;
}


// Creates a composite editor for "time" parameters: the given line edit plus a
// button which opens a date and time selection dialog. The selected time is
// written back into the line edit as %FT%T.%6f and committed through its
// regular editing-finished path.
// Opens the date and time selection window for a parameter. 'current' is the
// time shown initially, the default value of the parameter is used if it
// cannot be parsed. Returns whether a time was selected and stores it in
// 'value' at full precision.
bool selectTime(QWidget *parent, const Parameter *param, const QString &current,
                QString &value) {
	const QString title = QObject::tr("Select date and time for %1")
	                      .arg(QString::fromStdString(param->variableName));

	const QString defaultValue = QString::fromStdString(param->definition->defaultValue);

	// The allowed range, if any, is shown above the input field and limits
	// the selection.
	QString rangeBegin, rangeEnd;
	splitTimeRange(QString::fromStdString(param->definition->range),
	               rangeBegin, rangeEnd);

	const ParsedTime rangeMin = parseTime(rangeBegin);
	const ParsedTime rangeMax = parseTime(rangeEnd);

	const QStringList rangeLines = formatRangeLines(param);

	{
		QDialog dialog(parent);
		dialog.setWindowTitle(title);

		auto *layout = new QVBoxLayout(&dialog);

		auto *timeEdit = new TimeEdit;
		timeEdit->setMinimumWidth(220);
		timeEdit->setLimits(rangeMin, rangeMax);

		// Picks the date, the time of day is kept.
		auto *calendarButton = new QToolButton;
		calendarButton->setIcon(::icon("calendar"));
		calendarButton->setToolTip(QObject::tr("Select date"));
		QObject::connect(calendarButton, &QToolButton::clicked, &dialog,
		                 [timeEdit, calendarButton, rangeMin, rangeMax]() {
			auto *popup = new QDialog(timeEdit, Qt::Popup);
			popup->setAttribute(Qt::WA_DeleteOnClose);

			auto *popupLayout = new QVBoxLayout(popup);
			setMargin(popupLayout, 0);

			auto *calendar = new QCalendarWidget;

			// Offer only the days covered by the range of the parameter
			const QDate minDate = timeDate(rangeMin);
			if ( minDate.isValid() ) {
				calendar->setMinimumDate(minDate);
			}

			const QDate maxDate = timeDate(rangeMax);
			if ( maxDate.isValid() ) {
				calendar->setMaximumDate(maxDate);
			}

			const QDateTime current = QDateTime::fromString(
				timeEdit->text().trimmed().left(19), "yyyy-MM-ddTHH:mm:ss");
			if ( current.isValid() ) {
				calendar->setSelectedDate(current.date());
			}

			QObject::connect(calendar, &QCalendarWidget::clicked, popup,
			                 [timeEdit, popup](const QDate &date) {
				const QString value = timeEdit->text().trimmed();
				timeEdit->setText(date.toString("yyyy-MM-dd") +
				                  (value.size() > 10 ? value.mid(10)
				                                     : QString("T00:00:00.0")));
				popup->close();
			});

			popupLayout->addWidget(calendar);
			popup->move(calendarButton->mapToGlobal(
				QPoint(0, calendarButton->height())));
			popup->show();
		});

		// Fills in the current time truncated to full seconds.
		auto *now = new QPushButton(QObject::tr("Now (UTC)"));
		now->setToolTip(QObject::tr("Use current UTC time"));
		QObject::connect(now, &QPushButton::clicked, &dialog, [timeEdit]() {
			timeEdit->setText(
				QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddTHH:mm:ss") + ".0");
		});

		auto *timeLayout = new QHBoxLayout;
		timeLayout->setContentsMargins(0, 0, 0, 0);
		timeLayout->addWidget(timeEdit);
		timeLayout->addWidget(calendarButton);
		timeLayout->addWidget(now);

		auto *form = new QFormLayout;

		// One row per range line so that "Range:" is aligned with the
		// begin time and the end time gets an empty label.
		for ( int i = 0; i < rangeLines.size(); ++i ) {
			form->addRow(i ? QString() : QObject::tr("Range:"),
			             new QLabel(rangeLines[i]));
		}

		form->addRow(QObject::tr("Date and time:"), timeLayout);
		layout->addLayout(form);

		auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
		                                     QDialogButtonBox::Cancel |
		                                     QDialogButtonBox::Reset);
		QObject::connect(buttons, &QDialogButtonBox::accepted,
		                 &dialog, &QDialog::accept);
		QObject::connect(buttons, &QDialogButtonBox::rejected,
		                 &dialog, &QDialog::reject);

		// Shows the given time, falling back to midnight of the current day
		// if it cannot be parsed. Trailing zeros of the fractional seconds
		// are dropped, one decimal place is always shown.
		auto showValue = [timeEdit](const QString &value) {
			const ParsedTime parsed = parseTime(value);
			if ( parsed.valid ) {
				timeEdit->setText(formatTimeCompact(parsed.time));
			}
			else {
				timeEdit->setText(
					QDate::currentDate().toString("yyyy-MM-dd") + "T00:00:00.0");
			}
		};

		// The selection starts at the given time, falling back to the
		// default value of the parameter if there is none.
		const QString savedValue = current.trimmed();
		const QString initialValue = savedValue.isEmpty() ? defaultValue : savedValue;

		auto *reset = buttons->button(QDialogButtonBox::Reset);
		// Some styles decorate standard buttons with an icon
		reset->setIcon(QIcon());
		reset->setToolTip(QObject::tr("Restore the configured value"));
		QObject::connect(reset, &QPushButton::clicked, &dialog,
		                 [showValue, initialValue]() {
			showValue(initialValue);
		});

		layout->addWidget(buttons);

		// The value must be a time within the range of the parameter,
		// otherwise it cannot be accepted.
		auto *okButton = buttons->button(QDialogButtonBox::Ok);
		auto validate = [timeEdit, okButton, rangeMin, rangeMax]() {
			const ParsedTime parsed = parseTime(timeEdit->text());
			const bool inRange = parsed.valid
			                  && (!rangeMin.valid || (parsed.time >= rangeMin.time))
			                  && (!rangeMax.valid || (parsed.time <= rangeMax.time));
			okButton->setEnabled(inRange);
		};

		QObject::connect(timeEdit, &QLineEdit::textChanged, okButton,
		                 [validate](const QString &) { validate(); });

		showValue(initialValue);
		validate();

		if ( dialog.exec() != QDialog::Accepted ) {
			// Dialog cancelled: keep the current value untouched.
			return false;
		}

		// Written at full precision, whole seconds without a fraction
		value = formatTimeValue(parseTime(timeEdit->text()).time);
	}

	return true;
}


// Creates a composite editor for "time" parameters: the given input widget
// plus a button which opens the date and time selection window.
QWidget *makeTimeEditor(FancyViewItemEdit *edit, const Parameter *param) {
	auto *select = makeButtonEditor(edit, "calendar",
	                                QObject::tr("Select date and time"));

	if ( !select ) {
		return edit->widget();
	}

	const bool append = isListType(param);

	// 'edit' is passed as the connection context so the lambda is removed
	// automatically when the input widget is destroyed.
	QObject::connect(select, &QAction::triggered, edit->widget(),
	                 [edit, param, append]() {
		QString value;
		if ( !selectTime(edit->widget(), param, edit->value(), value) ) {
			return;
		}

		// A time was actively chosen, so unlock the parameter (mark it as
		// edited) in case it was still showing its default value.
		FancyViewItem item = edit->widget()->property("viewItem").value<FancyViewItem>();
		if ( item.isValid() && item.editControl && item.editControl->isChecked() ) {
			item.editControl->setChecked(false);
		}

		commitValue(edit, value, append);
	});

	return edit->widget();
}


// Opens the window in which the single values of a list parameter are
// activated, edited and ordered. The editor writes every change directly into
// the input field, so the window restores the value it started with when the
// changes are rejected.
void showListEditor(FancyViewItemEdit *edit, const Parameter *param) {
	QDialog dialog(edit->widget());
	dialog.setWindowTitle(QString::fromStdString(param->variableName));

	auto *layout = new QVBoxLayout(&dialog);

	auto *hint = new QLabel(
		QObject::tr("Add, select and order a list of values. Press '+' for adding "
		            "new values. Checking adds the value to configuration. "
		            "Unchecking or clicking on the trashcan removes it."));
	hint->setWordWrap(true);
	layout->addWidget(hint);

	// One row per range line so that "Range:" is aligned with the begin
	// value and the end value gets an empty label.
	const QStringList rangeLines = formatRangeLines(param);
	if ( !rangeLines.isEmpty() ) {
		auto *form = new QFormLayout;
		for ( int i = 0; i < rangeLines.size(); ++i ) {
			form->addRow(i ? QString() : QObject::tr("Range:"),
			             new QLabel(rangeLines[i]));
		}
		layout->addLayout(form);
	}

	layout->addWidget(new ListEditor(edit, param));

	// Keep the type apart from the add button of the list above it
	layout->addSpacing(layoutPadding());

	auto *type = new QLabel(QString("<b>%1:</b> %2")
	                        .arg(QObject::tr("Type"),
	                             QString::fromStdString(param->definition->type)));
	type->setTextFormat(Qt::RichText);
	// Allow selecting the text with the mouse to copy it.
	type->setTextInteractionFlags(Qt::TextSelectableByMouse |
	                              Qt::TextSelectableByKeyboard);
	layout->addWidget(type);

	auto *unique = makeUniqueOption();
	layout->addWidget(unique);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
	                                     QDialogButtonBox::Cancel |
	                                     QDialogButtonBox::Reset);
	QObject::connect(buttons, &QDialogButtonBox::accepted,
	                 &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected,
	                 &dialog, &QDialog::reject);

	const QString initialValue = edit->value();

	auto *reset = buttons->button(QDialogButtonBox::Reset);
	// Some styles decorate standard buttons with an icon
	reset->setIcon(QIcon());
	reset->setToolTip(QObject::tr("Restore the configured values"));
	QObject::connect(reset, &QPushButton::clicked, &dialog, [edit, initialValue]() {
		commitValue(edit, initialValue, false);
	});

	layout->addWidget(buttons);

	if ( dialog.exec() != QDialog::Accepted ) {
		// Dialog cancelled: drop the changes made in the editor.
		commitValue(edit, initialValue, false);
	}
	else if ( unique->isChecked() ) {
		commitValue(edit, removeDuplicateValues(edit->value()), false);
	}
}




// Opens a single-line editor dialog for the value of the given input widget.
// The dialog title shows the full dotted parameter name and, below the editor
// field, the parameter description together with its type, default value,
// allowed values and options. On accept the parameter is unlocked (if it was
// still showing its default value) and the new value is committed through the
// input widget's regular change path.
void openValueEditor(FancyViewItemEdit *input, const Parameter *param) {
	QDialog dialog(input->widget());
	dialog.setWindowTitle(QString::fromStdString(param->variableName));

	auto *layout = new QVBoxLayout(&dialog);
	auto *editor = new QLineEdit(input->value());
	editor->setMinimumWidth(400);
	layout->addWidget(editor);

	// Copies the parameter as it would be written to the configuration file,
	// using the value currently entered in the editor. It is disabled while the
	// entered value does not evaluate.
	auto *copyButton = new QPushButton(QObject::tr("Copy parameter"));
	const QString name = QString::fromStdString(param->variableName);
	QObject::connect(copyButton, &QPushButton::clicked, &dialog, [editor, name]() {
		QApplication::clipboard()->setText(name + " = " + editor->text());
	});

	// Live evaluation of the entered value, mirroring the popup shown while
	// editing the value directly in the regular parameter field, including its
	// colors: green when fine, orange when issues are found, red on errors.
	auto *evaluation = new QLabel;
	evaluation->setTextFormat(Qt::RichText);
	evaluation->setWordWrap(true);
	evaluation->setAutoFillBackground(true);
	evaluation->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	// The evaluation grows with the value list but is limited to 30 lines;
	// longer lists can be scrolled.
	auto *evalScroll = new QScrollArea;
	evalScroll->setWidgetResizable(true);
	evalScroll->setFrameShape(QFrame::NoFrame);
	evalScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	evalScroll->setWidget(evaluation);
	layout->addWidget(evalScroll);

	auto updateEvaluation = [&dialog, evaluation, evalScroll, copyButton, param](const QString &text) {
		std::vector<std::string> values;
		std::string errmsg;
		QString eval;

		QPalette pal = evaluation->palette();
		pal.setColor(QPalette::Window, QColor(255, 255, 255, 192));

		bool issueFound = false;
		if ( Config::Config::Eval(text.toStdString(), values, true, nullptr, &errmsg) ) {
			// value must not be a list, strings may contain commas and are not tested
			if ( (param->definition->type.size() < 5)
			     || (param->definition->type != "string"
			         && param->definition->type != "gradient"
			         && param->definition->type.substr(0, 5) != "list:") ) {
				// value to test contains a comma which is not supported
				if ( text.toStdString().find(',') != std::string::npos ) {
					eval += "<b>Value is not described as list</b>";
					issueFound = true;
				}
			}

			for ( const auto &value : values ) {
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
			             issueFound ? QColor(255, 127, 0) : QColor(32, 128, 32));
			evaluation->setText(QString("<b>Evaluation</b> (%1 item%2)<br/><br/>%3")
			                    .arg(values.size())
			                    .arg(values.size() == 1 ? "" : "s", eval));
			copyButton->setEnabled(true);
		}
		else {
			pal.setColor(QPalette::WindowText, QColor(128, 32, 32));
			evaluation->setText(QString("<b>Error</b><br/><br/><i>%1</i>")
			                    .arg(QString::fromStdString(errmsg).replace('\n', "<br/>")));
			copyButton->setEnabled(false);
		}

		evaluation->setPalette(pal);

		// Limit the evaluation to at most 30 lines. Shorter content shrinks the
		// area, longer value lists are capped and can be scrolled. The dialog
		// grows with the area until the limit is reached.
		int maxHeight = evaluation->fontMetrics().lineSpacing() * 30;
		int width = evalScroll->viewport()->width();
		if ( width <= 0 ) {
			width = 400;
		}
		int contentHeight = evaluation->heightForWidth(width);
		if ( contentHeight <= 0 ) {
			contentHeight = evaluation->sizeHint().height();
		}
		evalScroll->setFixedHeight(qMin(contentHeight, maxHeight));

		dialog.layout()->activate();
		QSize hint = dialog.sizeHint();
		dialog.resize(qMax(dialog.width(), hint.width()), hint.height());
	};

	QObject::connect(editor, &QLineEdit::textChanged, evaluation, updateEvaluation);

	const SchemaParameter *def = param->definition;

	QStringList fields;
	auto addField = [&fields](const QString &label, const QString &value) {
		if ( !value.isEmpty() ) {
			fields << ("<b>" + label + ":</b> " + value.toHtmlEscaped());
		}
	};
	auto joinList = [](const std::vector<std::string> &values) {
		QStringList list;
		for ( const auto &value : values ) {
			list << QString::fromStdString(value);
		}
		return list.join(", ");
	};

	addField(QObject::tr("Type"), QString::fromStdString(def->type));
	addField(QObject::tr("Default"), QString::fromStdString(def->defaultValue));
	addField(QObject::tr("Values"), joinList(def->values));
	addField(QObject::tr("Options"), joinList(def->options));

	QString info;
	if ( !def->description.empty() ) {
		info = QString::fromStdString(def->description).toHtmlEscaped().replace('\n', "<br/>");
	}
	if ( !fields.isEmpty() ) {
		if ( !info.isEmpty() ) {
			info += "<br/><br/>";
		}
		info += fields.join("<br/>");
	}

	if ( !info.isEmpty() ) {
		auto *details = new QLabel(info);
		details->setTextFormat(Qt::RichText);
		details->setWordWrap(true);
		// Allow selecting the text with the mouse to copy it.
		details->setTextInteractionFlags(Qt::TextSelectableByMouse |
		                                 Qt::TextSelectableByKeyboard);
		layout->addWidget(details);
	}

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
	                                     QDialogButtonBox::Cancel);

	// Copy button on the left, Ok/Cancel on the right.
	// Lists may hold the same value more than once
	QCheckBox *unique = nullptr;
	if ( isListType(param) ) {
		unique = makeUniqueOption();
		layout->addWidget(unique);
	}

	auto *bottom = new QHBoxLayout;
	bottom->addWidget(copyButton);
	bottom->addStretch();
	bottom->addWidget(buttons);
	layout->addLayout(bottom);

	QObject::connect(buttons, &QDialogButtonBox::accepted,
	                 &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected,
	                 &dialog, &QDialog::reject);

	// Evaluate the initial value once the whole dialog has been laid out, so the
	// initial size already accounts for all widgets.
	updateEvaluation(editor->text());

	if ( dialog.exec() != QDialog::Accepted ) {
		return;
	}

	FancyViewItem item = input->widget()->property("viewItem").value<FancyViewItem>();
	if ( item.isValid() && item.editControl && item.editControl->isChecked() ) {
		item.editControl->setChecked(false);
	}

	const QString value = unique && unique->isChecked() ?
	                      removeDuplicateValues(editor->text()) : editor->text();

	commitValue(input, value, false);
}


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
	// The target may live in a section which was not created yet
	realizePath(index);

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

	// Expanding a section or creating its content changes the geometry of
	// the widgets. Without settling the layout first the position read below
	// and the range of the scroll bars are the ones from before.
	if ( _rootWidget->layout() ) {
		_rootWidget->layout()->activate();
	}

	updateContentGeometry();

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




namespace {


// Returns the full name a tree item stands for, empty if it has none
QString itemName(const QModelIndex &idx) {
	auto *link = idx.data(ConfigurationTreeItemModel::Link).value<void*>();
	if ( !link ) {
		return {};
	}

	switch ( idx.data(ConfigurationTreeItemModel::Type).toInt() ) {
		case ConfigurationTreeItemModel::TypeParameter:
			return reinterpret_cast<Parameter*>(link)->variableName.c_str();

		case ConfigurationTreeItemModel::TypeSection:
			return reinterpret_cast<Section*>(link)->name.c_str();

		case ConfigurationTreeItemModel::TypeGroup:
		case ConfigurationTreeItemModel::TypeStruct:
			return reinterpret_cast<Container*>(link)->path.c_str();

		default:
			break;
	}

	return {};
}


// Depth first search for the item of a parameter or a section
QModelIndex findByName(const QAbstractItemModel *model, const QModelIndex &parent,
                       const QString &name) {
	for ( int row = 0; row < model->rowCount(parent); ++row ) {
		auto idx = model->index(row, 0, parent);

		if ( !itemName(idx).compare(name, Qt::CaseInsensitive) ) {
			return idx;
		}

		auto hit = findByName(model, idx, name);
		if ( hit.isValid() ) {
			return hit;
		}
	}

	return {};
}


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FancyView::showParameter(const QString &name) {
	if ( !model() || name.isEmpty() ) {
		return false;
	}

	auto idx = findByName(model(), rootIndex(), name.trimmed());
	if ( !idx.isValid() ) {
		return false;
	}

	// Creates the section holding the item if it was deferred
	setCurrentIndex(idx);
	scrollTo(idx);

	// While the view is being set up the geometry is not final yet, so the
	// position is taken again once the pending layout was processed.
	QPersistentModelIndex target(idx);
	QTimer::singleShot(0, this, [this, target]() {
		if ( target.isValid() ) {
			scrollTo(target);
		}
	});

	return true;
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
	_pendingSections.clear();

	if ( !index.isValid() ) {
		emit rootIndexChanged();
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

	int type = index.data(ConfigurationTreeItemModel::Type).toInt();

	if ( type == ConfigurationTreeItemModel::TypeModule ) {
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

		addExternalParameterHint(l, mod);
	}
	else if ( type == ConfigurationTreeItemModel::TypeBinding ) {
		// The module configuration of the module the binding belongs to may
		// override binding parameters, so the same hint applies here. The
		// model of a binding view holds no system model, the module is
		// reached through the binding itself.
		auto *binding = reinterpret_cast<ModuleBinding*>(
		                    index.data(ConfigurationTreeItemModel::Link).value<void*>());

		if ( binding ) {
			addExternalParameterHint(
				l, static_cast<Module*>(binding->parent), binding);
		}
	}

	QString secName;
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
	_rootWidget->setUpdatesEnabled(false);
	_rootWidget->show();
	_rootWidget->setUpdatesEnabled(true);

	emit rootIndexChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::setConfigStage(Seiscomp::Environment::ConfigStage cs) {
	_configStage = cs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::populateChildren(QBoxLayout *l, const QModelIndex &idx,
                                 const QString &rootSecName,
                                 const QString &emptyText) {
	int rows = model()->rowCount(idx);

	bool firstParameter = true;
	QLayout *paramLayout = nullptr;

	for ( int i = 0; i < rows; ++i ) {
		auto child = model()->index(i, 0, idx);
		if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter ) {
			continue;
		}

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
		alert->setText(emptyText);
		l->addWidget(alert);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::realizeSection(const QModelIndex &idx) {
	auto it = _pendingSections.find(idx);
	if ( it == _pendingSections.end() ) {
		return;
	}

	// Take the builder out first, creating the children may realize further
	// sections of its own.
	auto build = it.value();
	_pendingSections.erase(it);
	build();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::realizePath(const QModelIndex &idx) {
	// The outermost section must be created first, its children may be
	// deferred themselves.
	QList<QModelIndex> path;
	for ( auto parent = idx; parent.isValid(); parent = parent.parent() ) {
		path.prepend(parent);
	}

	for ( const auto &entry : path ) {
		realizeSection(entry);
	}
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

				populateChildren(l, idx, rootSecName,
				                 tr("This section does not contain a parameter to configure..."));

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
				QComboBox *comboBox = nullptr;

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
				const bool collapsed = idx.data().toString() != rootSecName;

				FancyViewItem item(idx, w);
				add(l, item, sec, collapsed);
				l->setContentsMargins(0, 0, 0, 0);

				if ( !sec->description.empty() ) {
					StatusLabel *desc = new StatusLabel;
					desc->setWordWrap(true);
					desc->setInfoText(sec->description.c_str());
					l->addWidget(desc);
				}

				const QString emptyText =
					tr("This section does not contain a parameter to configure...");

				// The parameters of a collapsed section are not visible, so
				// they are created when the section is opened the first time.
				if ( collapsed && item.toggle ) {
					QPersistentModelIndex pidx(idx);
					_pendingSections[pidx] =
						[this, l, pidx, rootSecName, emptyText]() {
							populateChildren(l, pidx, rootSecName, emptyText);
						};

					connect(item.toggle, &QAbstractButton::toggled, this,
					        [this, pidx](bool checked) {
						if ( checked ) {
							realizeSection(pidx);
						}
					});
				}
				else {
					populateChildren(l, idx, rootSecName, emptyText);
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

				populateChildren(l, idx, rootSecName,
				                 tr("This group does not contain a parameter to configure..."));

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

				populateChildren(l, idx, rootSecName,
				                 tr("This group does not contain a parameter to configure..."));
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
	QComboBox *comboBox = nullptr;

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
		checkBox->setValue(idx.sibling(idx.row(),2).data().toString());
		inputWidget = checkBox;

		connect(checkBox, &BoolEdit::toggled, this, &FancyView::optionToggled);

		// The checkbox toggles the value; a separate clickable name opens the
		// value editor, consistent with the other parameter types.
		ClickableLabel *name = new ClickableLabel;
		QFont f = name->font();
		f.setBold(true);
		name->setFont(f);
		name->setText(paramLabel);
		name->setCursor(Qt::PointingHandCursor);
		name->setToolTip(tr("Click to edit the value"));
		name->onClick = [checkBox, param]() { openValueEditor(checkBox, param); };
		textWidget = name;

		nameLayout->addWidget(checkBox);
		nameLayout->addWidget(name);
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
		ClickableLabel *name = new ClickableLabel;
		QFont f = name->font();
		f.setBold(true);
		name->setFont(f);
		name->setText(paramLabel);

		textWidget = name;

		// Widget placed into the layout. For most types this is the input
		// widget itself. The "file" type wraps the input line edit together
		// with a button opening a file selection dialog.
		QWidget *editorWidget = nullptr;

		const std::string type = baseType(param);

		if ( isListType(param) ) {
			// The field looks like a combo box, its drop-down opens the
			// editor for the single values. The selection dialogs of the
			// values are offered there, not in the field itself.
			auto *edit = new ListEdit;
			edit->setValue(idx.sibling(idx.row(),2).data().toString());
			connect(edit, SIGNAL(currentTextChanged(QString)), this, SLOT(optionTextChanged(QString)));
			connect(edit->lineEdit(), SIGNAL(editingFinished()), this, SLOT(optionTextEdited()));
			inputWidget = edit;

			const Parameter *definition = param;
			edit->setPopupHandler([edit, definition]() {
				showListEditor(edit, definition);
			});
		}
		else if ( param->definition->values.empty() ) {
			// No predefined values to choose from. Lists are handled above,
			// so the selection dialogs are offered in the field itself.
			StringEdit *edit = new StringEdit;
			edit->setValue(idx.sibling(idx.row(),2).data().toString());
			connect(edit, SIGNAL(editingFinished()), this, SLOT(optionTextEdited()));
			connect(edit, SIGNAL(textEdited(QString)), this, SLOT(optionTextChanged(QString)));
			inputWidget = edit;

			if ( (type == "file") || (type == "directory") ) {
				// Offer a file or directory selection dialog as well.
				editorWidget = makePathEditor(edit, param);
			}
			else if ( type == "time" ) {
				// Offer a date and time selection dialog as well.
				editorWidget = makeTimeEditor(edit, param);
			}
			else if ( type == "color" ) {
				// Offer a color selection dialog as well.
				editorWidget = makeColorEditor(edit, param);
			}
		}
		else {
			ComboEdit *combo = new ComboEdit;
			// remove whitespaces allowing line breaks in descriptions with value list
			for ( const auto &value : param->definition->values ) {
				combo->addItem(QString(value.c_str()).trimmed());
			}
			combo->setValue(idx.sibling(idx.row(),2).data().toString());
			// Evaluate on every change, including values assigned
			// programmatically, but finish the editing only when the line
			// edit is left or an item is picked. Both on currentTextChanged
			// would hide the evaluation hint right after showing it.
			connect(combo, SIGNAL(currentTextChanged(QString)), this, SLOT(optionTextChanged(QString)));
			connect(combo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(optionTextEdited()));
			connect(combo, SIGNAL(activated(int)), this, SLOT(optionTextEdited()));
			inputWidget = combo;
		}

		// Make the parameter name clickable to open a value editor showing the
		// full dotted variable name and the parameter details. Available for all
		// parameters (files and directories keep their selection button too).
		name->setCursor(Qt::PointingHandCursor);
		name->setToolTip(tr("Click to edit the value"));
		FancyViewItemEdit *input = inputWidget;
		name->onClick = [input, param]() { openValueEditor(input, param); };

		if ( !editorWidget ) {
			editorWidget = inputWidget->widget();
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
		paramLayout->addWidget(editorWidget);
	}

	nameLayout->addStretch();
	auto btnReset = new IconButton(_iconReset);
	nameLayout->addWidget(btnReset);
	auto btnEdit = new IconButton(_iconEdit);
	nameLayout->addWidget(btnEdit);

	// Built when the tooltip is requested, not while the panel is created.
	// Owned by the input widget, so it is destroyed together with it.
	QWidget *inputWidgetWidget = inputWidget->widget();
	new LazyToolTip(inputWidgetWidget,
	                [this, inputWidgetWidget, param](bool verbose) {
		updateToolTip(inputWidgetWidget, param, verbose);
	});

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

		// Wrapping and encoding the full description is only done when the
		// tooltip is requested. Owned by 'help', so it is destroyed with it.
		new LazyToolTip(help, [help, descText](bool) {
			if ( !help->toolTip().isEmpty() ) {
				return;
			}

			QString content(string2Block(descText, 80).c_str());
			content = encodeHTML(content);
			help->setToolTip(QString("<p style='white-space:pre'>%1</p>")
			                 .arg(content));
		});

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

	// An editable combo box forwards the signals of its line edit, so the item
	// must be reachable from there as well.
	if ( auto *combo = qobject_cast<QComboBox*>(inputWidget->widget()) ) {
		if ( combo->lineEdit() ) {
			combo->lineEdit()->setProperty("viewItem",
			                               QVariant::fromValue<FancyViewItem>(item));
		}
	}
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

	// Locked again: the value is not edited anymore, drop its evaluation
	if ( state ) {
		if ( _optionEditHint ) {
			_optionEditHint->hide();
		}
	}
	// Unlocked for editing: continue right away at the end of the value
	else {
		QWidget *input = item.input->widget();
		input->setFocus(Qt::OtherFocusReason);

		auto *lineEdit = qobject_cast<QLineEdit*>(input);
		if ( !lineEdit ) {
			if ( auto *combo = qobject_cast<QComboBox*>(input) ) {
				lineEdit = combo->lineEdit();
			}
		}

		if ( lineEdit ) {
			lineEdit->setCursorPosition(lineEdit->text().size());
		}

		// Report right away what is wrong with the current value
		evaluateInput(input);
	}
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
                              QString &eval, bool verbose) {
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
				     << valueTest << "' must be a SeisComP time"<< endl;
			}
			eval += "<b>Value must be a SeisComP time:</b> ";
		}
		else if ( !valueTest.empty() && !param->definition->range.empty() ) {
			// Times contain ':' themselves, so their range is separated by
			// '..' and cannot be checked by the generic range test below.
			QString rangeBegin, rangeEnd;
			splitTimeRange(param->definition->range.c_str(), rangeBegin, rangeEnd);

			const ParsedTime rangeMin = parseTime(rangeBegin);
			const ParsedTime rangeMax = parseTime(rangeEnd);

			if ( (rangeMin.valid && (value < rangeMin.time))
			  || (rangeMax.valid && (value > rangeMax.time)) ) {
				if ( verbose && !symbolURIString.empty() ) {
					cerr << symbolURIString << param->variableName << " = '"
					     << valueTest << "' is not in range: '"
					     << param->definition->range << "'" << endl;
				}
				eval += "<b>Out of range value:</b> ";
			}
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
	// color
	else if ( (type == "color") || (type == "list:color") ) {
		QColor color;
		if ( !valueTest.empty() && !parseColor(valueTest.c_str(), color) ) {
			if ( verbose && !symbolURIString.empty() ) {
				cerr << symbolURIString << param->variableName << " = '"
				     << valueTest
				     << "' must be a color given as keyword, hexadecimal "
				        "digits or rgb()" << endl;
			}
			eval += "<b>Value must be a color:</b> ";
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

	// test if values are in range, times are checked above
	if ( !param->definition->range.empty()
	  && (type != "time") && (type != "list:time") ) {

		double value;
		vector<string> toks;

		// The range values are separated either by the commonly used '..' or
		// by ':'. The former must be matched as a whole, splitting by single
		// characters would break the decimal point of the limits.
		const string &rangeDef = param->definition->range;
		auto sep = rangeDef.find("..");
		if ( sep != string::npos ) {
			toks.push_back(rangeDef.substr(0, sep));
			toks.push_back(rangeDef.substr(sep + 2));
		}
		else {
			Core::split(toks, rangeDef.c_str(), ":");
		}
		// lowest(), not min(): the latter is the smallest positive value
		double rangeMin = std::numeric_limits<double>::lowest();
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
void FancyView::updateToolTip(QWidget *w, Seiscomp::System::Parameter *param,
                             bool verbose) {
	bool isOverridden = param->symbol.stage > _configStage;
	vector<string> values;
	QString eval;
	string errmsg;
	if ( Config::Config::Eval(param->symbol.content, values, true, nullptr, &errmsg) ) {
		// value must not be a list, strings may contain commas and are not tested
		if ( (param->definition->type.size() < 5)
		     || ((param->definition->type != "string")
		         && (param->definition->type != "gradient")
		         && (param->definition->type.substr(0, 5) != "list:")) ) {
			string valueTest = param->symbol.content;
			// value to test contains a comma which is not supported
			string symbolURIString = param->symbol.uri.empty() ? "" : param->symbol.uri + ": ";
			if ( valueTest.find(',') != std::string::npos ) {
				if ( verbose ) {
					cerr << symbolURIString << param->variableName << " = '"
					     << valueTest << "' is not described as list " << endl;
				}

				eval += "<b>Value is not described as list</b>";
			}
		}

		for ( const auto& value : values ) {
			if ( !eval.isEmpty() ) {
				eval += "<br/>";
			}
			FancyView::evaluateValue(value, param, eval, verbose);
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
void FancyView::optionTextChanged(const QString &) {
	evaluateInput(static_cast<QWidget*>(sender()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::evaluateInput(QWidget *w) {
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
	if ( Config::Config::Eval(item.input->value().toStdString(), values, true, nullptr, &errmsg) ) {
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

			// Report the issues of the edited value on the console as well
			if ( FancyView::evaluateValue(value, param, eval, true) ) {
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
		if ( errmsg.compare("Empty rvalue") == 0 ) {
			pal.setColor(QPalette::WindowText, QColor(255,127,0));
			eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");

			_optionEditHint->setText(QString("<b>Information</b><br/><br/>Empty string will be saved as \"\""));
		}
		else {
			pal.setColor(QPalette::WindowText, QColor(128,32,32));
			eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");

			_optionEditHint->setText(QString("<b>Error</b><br/><br/>%1").arg(eval));
		}
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

	updateToolTip(w, param);
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

	cat->activeBinding = nullptr;

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
		QMessageBox::critical(nullptr, "Internal error",
		                      "The type must not be empty.");
		return;
	}

	BindingCategory *cat = reinterpret_cast<BindingCategory*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Binding *typeBinding = cat->binding(type.toStdString());
	if ( !typeBinding ) {
		QMessageBox::critical(nullptr, "Internal error",
		                      "The selected type is not available.");
		return;
	}

	NewCatBindingDialog dlg(cat, type.toStdString(), this);
	if ( dlg.exec() != QDialog::Accepted ) return;

	Binding *nb = cat->instantiate(typeBinding, dlg.name().c_str());
	if ( !nb ) {
		QMessageBox::critical(nullptr, "Internal error",
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

		if ( auto *cm = qobject_cast<ConfigurationTreeItemModel*>(model()) ) {
			cm->setModified();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FancyView::removeStruct() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Structure *s = reinterpret_cast<Structure*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Container *c = reinterpret_cast<Container*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	// Find the module the structure belongs to. Bindings are written as a
	// whole and do not require the removal to be tracked.
	Module *mod = nullptr;
	for ( auto idx = item.index.parent(); idx.isValid(); idx = idx.parent() ) {
		if ( idx.data(ConfigurationTreeItemModel::Type).toInt() ==
		     ConfigurationTreeItemModel::TypeModule ) {
			mod = reinterpret_cast<Module*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			break;
		}
	}

	// The container holds the last reference, keep the structure alive until
	// its parameters have been processed.
	StructurePtr structure(s);

	if ( !c->remove(s) ) {
		cerr << "ERROR: failed to remove structure from container, registered "
		        "structures: " << c->structures.size() << endl;
		return;
	}

	// Delete the configured parameters of the structure from the file
	if ( mod ) {
		markRemoved(mod, structure.get(), _configStage);
	}

	if ( auto *cm = qobject_cast<ConfigurationTreeItemModel*>(model()) ) {
		cm->setModified();
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
		_currentItem = nullptr;
	}

	// The item may live in a section which was not created yet
	realizePath(curr);

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
		_currentItem = nullptr;
	}

	// TODO: implement keyboard search
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
