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
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QSet>
#include <QSettings>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QToolButton>
#include <QVBoxLayout>

#include <functional>
#include <memory>

#include <seiscomp/system/environment.h>
#include <seiscomp/system/model.h>

#include "coloreditor.h"


using namespace Seiscomp;
using namespace Seiscomp::System;


namespace {


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


// Fills the rectangle with the color above a checkerboard, so that a
// transparency is recognizable, and frames it.
void paintColor(QPainter &p, const QRect &rect, const QColor &color) {
	p.save();
	p.setClipRect(rect);
	p.fillRect(rect, Qt::white);

	const int box = qMax(qMin(rect.width(), rect.height()) / 4, 2);
	for ( int row = 0; (row * box) < rect.height(); ++row ) {
		for ( int col = (row % 2); (col * box) < rect.width(); col += 2 ) {
			p.fillRect(rect.x() + col * box, rect.y() + row * box, box, box,
			           QColor(0xC0, 0xC0, 0xC0));
		}
	}

	p.fillRect(rect, color);
	p.setPen(QApplication::palette().color(QPalette::Mid));
	p.drawRect(rect.adjusted(0, 0, -1, -1));
	p.restore();
}


// A square filled with the given color above a checkerboard
QPixmap colorTile(const QColor &color, int size) {
	QPixmap pixmap(size, size);

	QPainter p(&pixmap);
	paintColor(p, pixmap.rect(), color);
	p.end();

	return pixmap;
}


// The layout of the grid of swatches. Beyond the colors in use it offers up
// to three more rows of basic colors.
const int SwatchColumns = 8;
const int SwatchFreeRows = 3;


// Basic colors filling the free rows of the grid, sorted by shades of grey
// first and by hue afterwards.
const char *SchemeColors[] = {
	"#000000", "#202020", "#666666", "#808080", "#999999",
	"#A0A0A4", "#C0C0C0", "#CCCCCC", "#FFFFFF",
	"#A00000", "#FF0000", "#FF6633", "#FFA000", "#FFFD00", "#FFFF00",
	"#82AD58", "#00A000", "#00FF00", "#00FFFF",
	"#02589E", "#C0C0FF", "#0000A0", "#FF00FF"
};


// A configured color with the name of its parameter
using NamedColor = QPair<QString, QColor>;


// The module a parameter belongs to
Module *moduleOf(const Parameter *param) {
	Core::BaseObject *node = param ? param->parent : nullptr;

	while ( node ) {
		if ( auto *mod = Module::Cast(node) ) {
			return mod;
		}

		auto *container = Container::Cast(node);
		if ( !container ) {
			break;
		}

		node = container->parent;
	}

	return nullptr;
}


// Collects the color parameters below 'container' whose name starts with
// 'prefix'. Taken is the value in use: the configured one or, where the
// parameter is not set, the default of its description.
void collectColors(const Container *container, const std::string &prefix,
                   QList<NamedColor> &colors) {
	for ( const auto &param : container->parameters ) {
		if ( !param->definition || (param->definition->type != "color")
		     || (param->variableName.compare(0, prefix.size(), prefix) != 0) ) {
			continue;
		}

		const std::string &value = param->symbol.stage > Environment::CS_UNDEFINED
		                           ? param->symbol.content
		                           : param->definition->defaultValue;

		QColor color;
		if ( parseColor(QString::fromStdString(value), color) ) {
			colors.append({QString::fromStdString(param->variableName), color});
		}
	}

	for ( const auto &group : container->groups ) {
		collectColors(group.get(), prefix, colors);
	}
}


// The colors of the scheme section of the global module. They are offered as
// swatches so that a configuration stays consistent with the appearance of
// the GUI applications. A color shared by several parameters is offered once.
QList<NamedColor> schemeColors(const Parameter *param) {
	QList<NamedColor> colors;

	auto *mod = moduleOf(param);
	auto *global = (mod && mod->model) ? mod->model->module("global") : nullptr;

	if ( global ) {
		for ( const auto &section : global->sections ) {
			collectColors(section.get(), "scheme.colors.", colors);
		}
	}

	QList<NamedColor> offered;
	QSet<QRgb> taken;

	for ( const auto &entry : colors ) {
		if ( !taken.contains(entry.second.rgba()) ) {
			taken.insert(entry.second.rgba());
			offered.append(entry);
		}
	}

	// Fill the free rows of the grid with the basic colors which are not
	// configured anywhere. Where no scheme is available, e.g. without the
	// description of the global module, they are all there is to offer.
	const int rows = (offered.size() + SwatchColumns - 1) / SwatchColumns;
	const int limit = (rows + SwatchFreeRows) * SwatchColumns;

	for ( size_t i = 0; (i < sizeof(SchemeColors) / sizeof(*SchemeColors))
	                    && (offered.size() < limit); ++i ) {
		const QColor color(SchemeColors[i]);

		if ( !taken.contains(color.rgba()) ) {
			taken.insert(color.rgba());
			offered.append({QString(), color});
		}
	}

	return offered;
}


// A grid of color swatches replacing the standard colors of the dialog,
// which Qt keeps as plain RGB. Each swatch here carries its alpha channel
// and sets it along with the color.
QWidget *makeSwatchGrid(const QList<NamedColor> &colors,
                        const std::function<void (const QColor &)> &pick) {
	auto *widget = new QWidget;
	auto *grid = new QGridLayout(widget);
	grid->setContentsMargins(0, 0, 0, 0);
	grid->setSpacing(1);

	const int size = QFontMetrics(QApplication::font()).ascent();
	int index = 0;

	for ( const auto &entry : colors ) {
		const QColor color = entry.second;

		auto *swatch = new QToolButton;
		swatch->setAutoRaise(true);
		swatch->setFocusPolicy(Qt::NoFocus);
		swatch->setCursor(Qt::PointingHandCursor);
		swatch->setIconSize(QSize(size, size));
		swatch->setIcon(QIcon(colorTile(color, size)));
		swatch->setToolTip(entry.first.isEmpty()
		                   ? formatColor(color)
		                   : QString("%1\n%2").arg(entry.first,
		                                           formatColor(color)));

		QObject::connect(swatch, &QToolButton::clicked, swatch,
		                 [pick, color]() { pick(color); });

		grid->addWidget(swatch, index / SwatchColumns, index % SwatchColumns);
		++index;
	}

	grid->setColumnStretch(SwatchColumns, 1);

	return widget;
}


// Paints the preview of the dialog with the alpha channel above a
// checkerboard. Qt fills it with the color alone, where a transparency is
// not recognizable.
class ColorPreview : public QObject {
	public:
		ColorPreview(QWidget *preview, const std::function<QColor()> &colorOf)
		: QObject(preview), _preview(preview), _colorOf(colorOf) {
			preview->installEventFilter(this);
		}

	protected:
		bool eventFilter(QObject *watched, QEvent *event) override {
			if ( event->type() != QEvent::Paint ) {
				return QObject::eventFilter(watched, event);
			}

			QPainter p(_preview);
			paintColor(p, _preview->rect(), _colorOf());
			p.end();

			// Painted, the one of the dialog is not needed
			return true;
		}

	private:
		QWidget                *_preview;
		std::function<QColor()> _colorOf;
};


// The preview of the current color of a dialog. Its class is private to Qt
// and sits in a namespace since Qt 6, so it is looked up by name.
QWidget *colorPreview(QColorDialog *chooser) {
	for ( auto *child : chooser->findChildren<QWidget*>() ) {
		if ( QString(child->metaObject()->className())
		     .endsWith("QColorShowLabel") ) {
			return child;
		}
	}

	return nullptr;
}


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


const char *CustomColorsKey = "ColorDialog/customColors";
const int CustomRows = 2;


QColor customColor(int index) {
	const QColor rgb = QColorDialog::customColor(index);
	const auto stored = QSettings("gempa", "scconfig").value(CustomColorsKey).toStringList();

	if ( index < stored.size() ) {
		const QColor color(stored[index]);
		if ( color.isValid() && (color.rgb() == rgb.rgb()) ) {
			return color;
		}
	}

	return rgb;
}


void setCustomColor(int index, const QColor &color) {
	QColorDialog::setCustomColor(index, color);

	QSettings settings("gempa", "scconfig");
	auto stored = settings.value(CustomColorsKey).toStringList();
	while ( stored.size() <= index ) {
		stored << QString();
	}

	stored[index] = color.name(QColor::HexArgb);
	settings.setValue(CustomColorsKey, stored);
}


class CustomSwatches : public QWidget {
	public:
		CustomSwatches(const std::function<void (const QColor &)> &pick)
		: _pick(pick) {
			auto *grid = new QGridLayout(this);
			grid->setContentsMargins(0, 0, 0, 0);
			grid->setSpacing(1);

			const int size = QFontMetrics(QApplication::font()).ascent();

			for ( int i = 0; i < QColorDialog::customCount(); ++i ) {
				auto *swatch = new QToolButton;
				swatch->setAutoRaise(true);
				swatch->setCheckable(true);
				swatch->setFocusPolicy(Qt::NoFocus);
				swatch->setCursor(Qt::PointingHandCursor);
				swatch->setIconSize(QSize(size, size));

				connect(swatch, &QToolButton::clicked, this, [this, i]() {
					select(i);
					_pick(customColor(i));
				});

				grid->addWidget(swatch, i % CustomRows, i / CustomRows);
				_swatches.append(swatch);
				_colors.append(QColor());
				refresh(i);
			}

			grid->setColumnStretch(grid->columnCount(), 1);
		}

		void add(const QColor &color) {
			setCustomColor(_next, color);
			refresh(_next);
			select((_next + 1) % _swatches.size());
		}

		void showColor(const QColor &color) {
			_shown = color;
			updateChecked();
		}

	private:
		void refresh(int index) {
			_colors[index] = customColor(index);
			auto *swatch = _swatches[index];
			swatch->setIcon(QIcon(colorTile(_colors[index], swatch->iconSize().width())));
			swatch->setToolTip(formatColor(_colors[index]));
			updateChecked();
		}

		void select(int index) {
			_next = index;
			updateChecked();
		}

		void updateChecked() {
			for ( int i = 0; i < _swatches.size(); ++i ) {
				_swatches[i]->setChecked((i == _next) && _shown.isValid()
				                         && (_colors[i].rgba() == _shown.rgba()));
			}
		}

		std::function<void (const QColor &)> _pick;
		QList<QToolButton*>                  _swatches;
		QVector<QColor>                      _colors;
		QColor                               _shown;
		int                                  _next{0};
};


// The grids of swatches of a color dialog, the standard colors first and the
// custom colors after them. Their class is private to Qt and was renamed
// along the versions, so they are looked up by name.
QList<QWidget*> swatchGrids(QColorDialog *chooser) {
	QList<QWidget*> grids;

	for ( auto *child : chooser->findChildren<QWidget*>() ) {
		const QString name = child->metaObject()->className();

		if ( name.endsWith("QWellArray") || name.endsWith("QColorWell") ) {
			grids.append(child);
		}
	}

	return grids;
}


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


bool replaceGrid(QColorDialog *chooser, QWidget *grid, QWidget *widget) {
	if ( !insertBefore(chooser->layout(), grid, widget) ) {
		return false;
	}

	grid->hide();

	// The label in front of the grid belongs to the new one
	for ( auto *label : chooser->findChildren<QLabel*>() ) {
		if ( label->buddy() == grid ) {
			label->setBuddy(widget);
			break;
		}
	}

	return true;
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


}


// Parses a color given as a W3C color keyword, as hexadecimal digits with an
// optional leading '#' (RGB, RGBA, RRGGBB or RRGGBBAA) or in the functional
// notation rgb(r,g,b) or rgba(r,g,b,a).
bool parseColor(const QString &value, QColor &color) {
	const QString text = value.trimmed();
	if ( text.isEmpty() ) {
		return false;
	}

	// Hexadecimal digits, read before Qt is asked: it expects the alpha
	// channel first and would take "#RRGGBBAA" for "#AARRGGBB".
	static const QRegularExpression hex("\\A#?([0-9a-fA-F]+)\\z");
	const auto match = hex.match(text);

	if ( match.hasMatch() ) {
		const QString digits = match.captured(1);
		QColor named;

		switch ( digits.size() ) {
			case 3:
			case 6:
				named = QColor("#" + digits);
				break;

			case 4:
				// Qt parses no short form carrying an alpha channel, so
				// every digit is doubled into the long form.
				named = QColor("#" + QString(2, digits.at(3))
				               + QString(2, digits.at(0))
				               + QString(2, digits.at(1))
				               + QString(2, digits.at(2)));
				break;

			case 8:
				named = QColor("#" + digits.right(2) + digits.left(6));
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

	return false;
}


// A square filled with the given color, shown inside the input field
QIcon colorSwatch(const QColor &color) {
	const int size = QFontMetrics(QApplication::font()).ascent();
	QPixmap pixmap(size, size);
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


bool selectColor(QWidget *parent, const Parameter *param,
                 const QString &currentValue, QString &value) {
	const QString title = QObject::tr("Select the color for %1")
	                      .arg(QString::fromStdString(param->variableName));

	QColor current;
	if ( !parseColor(currentValue, current) ) {
		current = Qt::white;
	}

	QDialog dialog(parent);
	dialog.setWindowTitle(title);

	auto *layout = new QVBoxLayout(&dialog);

	// The dialog is embedded so that the notation to store can be
	// chosen along with the color.
	// The options are set before the color is handed over so that the
	// alpha field exists by then.
	auto *chooser = new QColorDialog;
	chooser->setWindowFlags(Qt::Widget);
	chooser->setOptions(QColorDialog::ShowAlphaChannel |
	                    QColorDialog::NoButtons |
	                    QColorDialog::DontUseNativeDialog);
	chooser->setCurrentColor(current);

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

	// The dialog builds its widgets only when no native dialog is in use.
	// Where the platform offers one they are created when the window is
	// shown, so they cannot be looked up before that. The handlers share
	// them through this block.
	struct ColorFields {
		QLineEdit *html{nullptr};
		QSpinBox  *alpha{nullptr};
	};

	auto fields = std::make_shared<ColorFields>();

	layout->addWidget(chooser);

	auto *format = new QComboBox;
	format->addItem(QObject::tr("Hexadecimal"), ColorHex);
	format->addItem(QObject::tr("Color keyword"), ColorKeyword);
	format->addItem(QObject::tr("rgb() or rgba()"), ColorFunction);
	format->setCurrentIndex(colorFormatOf(currentValue));

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

	// The color as it will be stored. Whether the current color of the
	// dialog carries the alpha channel depends on the visibility of its
	// alpha field, the spin box always holds it.
	auto colorOf = [chooser, fields]() {
		auto color = chooser->currentColor();
		if ( fields->alpha ) {
			color.setAlpha(fields->alpha->value());
		}

		return color;
	};

	auto valueOf = [format](const QColor &color) {
		QString text;
		switch ( format->currentIndex() ) {
			case ColorKeyword:
				text = colorKeyword(color);
				break;

			case ColorFunction:
				text = colorFunction(color);
				break;

			default:
				break;
		}

		return text.isEmpty() ? formatColor(color) : text;
	};

	// Writes the color including its alpha channel into the HTML field,
	// which the dialog itself fills with the six digits of the color
	// only. Unless forced, a field being typed into is left alone: the
	// dialog applies every keystroke and the text would be rewritten
	// under the cursor.
	auto showHtml = [fields, colorOf](bool force) {
		if ( !fields->html || (!force && fields->html->hasFocus()) ) {
			return;
		}

		fields->html->setText("#" + formatColor(colorOf()));
	};

	// Queued: the dialog writes its HTML field on the same signal and
	// would drop the alpha channel again.
	QObject::connect(chooser, &QColorDialog::currentColorChanged,
	                 chooser, [showHtml]() { showHtml(false); },
	                 Qt::QueuedConnection);

	// The dialog reads its HTML field itself but interprets eight digits
	// with the alpha channel first, so the value is applied again here.
	// Not a color: the color in use is shown again.
	auto applyTyped = [chooser, fields, showHtml]() {
		QColor typed;
		if ( fields->html && parseColor(fields->html->text(), typed) ) {
			chooser->setCurrentColor(typed);
			if ( fields->alpha ) {
				fields->alpha->setValue(typed.alpha());
			}
		}

		showHtml(true);
	};

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

	dialog.show();

	// The widgets of the dialog exist from here on. The HTML field is
	// recognized by its content, the line edits of the spin boxes carry
	// numbers, and the alpha channel is the last spin box.
	for ( auto *field : chooser->findChildren<QLineEdit*>() ) {
		if ( field->text().startsWith('#') ) {
			fields->html = field;
			break;
		}
	}

	auto spinBoxes = chooser->findChildren<QSpinBox*>();
	fields->alpha = spinBoxes.isEmpty() ? nullptr : spinBoxes.last();

	if ( fields->html ) {
		// The dialog confines the field to six digits. Depending on its
		// version it does so by an input mask, by the maximum length or
		// by a validator, all three are lifted here.
		fields->html->setInputMask(QString());
		fields->html->setMaxLength(9);
		fields->html->setValidator(new QRegularExpressionValidator(
			QRegularExpression("#?[0-9a-fA-F]{0,8}"), fields->html));
		fields->html->setToolTip(QObject::tr("Two hexadecimal digits beyond "
		                                     "the six of the color set the "
		                                     "alpha channel"));

		QObject::connect(fields->html, &QLineEdit::editingFinished, chooser,
		                 [applyTyped]() { applyTyped(); });
	}

	if ( fields->alpha ) {
		QObject::connect(fields->alpha, QOverload<int>::of(&QSpinBox::valueChanged),
		                 chooser, [showHtml]() { showHtml(false); });
	}

	// The preview of the dialog shows the color without its alpha
	// channel. Owned by the preview, so it goes together with it.
	if ( auto *preview = colorPreview(chooser) ) {
		new ColorPreview(preview, colorOf);

		QObject::connect(chooser, &QColorDialog::currentColorChanged,
		                 preview, [preview]() { preview->update(); });

		if ( fields->alpha ) {
			QObject::connect(fields->alpha,
			                 QOverload<int>::of(&QSpinBox::valueChanged),
			                 preview, [preview]() { preview->update(); });
		}
	}

	auto pick = [chooser, fields, showHtml](const QColor &color) {
		chooser->setCurrentColor(color);
		if ( fields->alpha ) {
			fields->alpha->setValue(color.alpha());
		}

		showHtml(true);
	};

	const auto grids = swatchGrids(chooser);

	if ( !grids.isEmpty() ) {
		auto *swatches = makeSwatchGrid(schemeColors(param), pick);
		if ( !replaceGrid(chooser, grids.front(), swatches) ) {
			delete swatches;
		}
	}

	QPushButton *addCustom = nullptr;
	for ( auto *button : chooser->findChildren<QPushButton*>() ) {
		if ( button != screenPicker ) {
			addCustom = button;
			break;
		}
	}

	if ( (grids.size() > 1) && addCustom ) {
		auto *custom = new CustomSwatches(pick);

		if ( replaceGrid(chooser, grids[1], custom) ) {
			addCustom->setAutoDefault(false);
			QObject::disconnect(addCustom, SIGNAL(clicked()), chooser, nullptr);
			QObject::connect(addCustom, &QPushButton::clicked, custom,
			                 [custom, colorOf]() { custom->add(colorOf()); });

			if ( fields->html ) {
				auto showColor = [custom, fields]() {
					QColor color;
					parseColor(fields->html->text(), color);
					custom->showColor(color);
				};

				QObject::connect(fields->html, &QLineEdit::textChanged,
				                 custom, showColor);
				showColor();
			}
		}
		else {
			delete custom;
		}
	}

	chooser->setCurrentColor(current);
	if ( fields->alpha ) {
		fields->alpha->setValue(current.alpha());
	}

	showHtml(true);

	if ( dialog.exec() != QDialog::Accepted ) {
		// Dialog cancelled: keep the current value untouched.
		return false;
	}

	const QColor color = colorOf();
	if ( !color.isValid() ) {
		return false;
	}

	value = valueOf(color);
	return true;
}
