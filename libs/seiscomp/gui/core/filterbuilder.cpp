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


#include <seiscomp/gui/core/filterbuilder.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/math/filter.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <memory>
#include <vector>


namespace Seiscomp {
namespace Gui {


namespace {


enum ParameterKind {
	Integer,
	Real,
	GroundMotion
};


struct ParameterSpec {
	const char    *label;
	const char    *unit;
	ParameterKind  kind;
	double         minimum;
	double         maximum;
	double         defaultValue;
	int            decimals;
	bool           isFrequency;
};


struct StageSpec {
	const char                 *key;
	const char                 *label;
	const char                 *description;
	std::vector<ParameterSpec>  parameters;
};


// Keys of the composite stages. They cannot clash with filter names
// because identifiers of the filter grammar cannot contain '@'.
const QString BandsKey = "@BANDS";
const QString ExpressionKey = "@EXPR";


enum BandMode {
	RootSumSquare,
	Sum,
	SumOfAbsolutes,
	BandModeCount
};


const char *bandModeNames[] = {
	"root-sum-square",
	"sum",
	"sum of absolute values"
};


const ParameterSpec OrderParameter = {
	"Order", "", Integer, 1, 10, 3, 0, false
};


ParameterSpec frequency(const char *label, double defaultValue) {
	return { label, "Hz", Real, 0.001, 10000, defaultValue, 3, true };
}


ParameterSpec seconds(const char *label, double defaultValue) {
	return { label, "s", Real, 0.001, 86400, defaultValue, 3, false };
}


const ParameterSpec InputParameter = {
	"Input", "", GroundMotion, 0, 2, 1, 0, false
};


// Filters with a fixed parameter list which are offered as single stages.
// Everything else of the filter grammar is handled by the composite stages.
const std::vector<StageSpec> &stageSpecs() {
	static const std::vector<StageSpec> specs = {
		{ "BW", "Butterworth bandpass",
		  "Butterworth bandpass filter", {
			OrderParameter,
			frequency("Lower corner", 1.0),
			frequency("Upper corner", 8.0)
		}},
		{ "BW_HP", "Butterworth highpass",
		  "Butterworth highpass filter", {
			OrderParameter,
			frequency("Corner", 1.0)
		}},
		{ "BW_LP", "Butterworth lowpass",
		  "Butterworth lowpass filter", {
			OrderParameter,
			frequency("Corner", 8.0)
		}},
		{ "BW_BS", "Butterworth bandstop",
		  "Butterworth bandstop (notch) filter", {
			OrderParameter,
			frequency("Lower corner", 0.1),
			frequency("Upper corner", 0.3)
		}},
		{ "RMHP", "Remove mean",
		  "Running mean highpass: removes the mean of a sliding window", {
			seconds("Window", 10.0)
		}},
		{ "ITAPER", "Initial taper",
		  "Cosine taper at the beginning of the data", {
			seconds("Length", 10.0)
		}},
		{ "AVG", "Moving average",
		  "Smoothing with a moving average", {
			seconds("Window", 1.0)
		}},
		{ "MEDIAN", "Moving median",
		  "Smoothing with a moving median", {
			seconds("Window", 1.0)
		}},
		{ "BPENV", "Bandpass envelope",
		  "Envelope of the data in a frequency band", {
			frequency("Center frequency", 2.0),
			{ "Bandwidth", "oct", Real, 0.01, 10, 1.0, 2, false },
			OrderParameter
		}},
		{ "INT", "Integrate",
		  "Integrate the data (e.g. velocity to displacement)", {} },
		{ "DIFF", "Differentiate",
		  "Differentiate the data (e.g. velocity to acceleration)", {} },
		{ "WA", "Wood-Anderson",
		  "Simulate a Wood-Anderson seismometer", {
			InputParameter
		}},
		{ "WWSSN_SP", "WWSSN short period",
		  "Simulate a WWSSN short period seismometer", {
			InputParameter
		}},
		{ "WWSSN_LP", "WWSSN long period",
		  "Simulate a WWSSN long period seismometer", {
			InputParameter
		}}
	};

	return specs;
}


const StageSpec *findSpec(const QString &key) {
	// BW_BP is registered as an alias of BW
	QString name = key == "BW_BP" ? QString("BW") : key;
	for ( const auto &spec : stageSpecs() ) {
		if ( name == spec.key ) {
			return &spec;
		}
	}

	return nullptr;
}


QVariantList defaultValues(const StageSpec &spec) {
	QVariantList values;
	for ( const auto &param : spec.parameters ) {
		values.append(param.defaultValue);
	}
	return values;
}


QVariantList makeBand(double lower, double upper) {
	return QVariantList() << lower << upper;
}


QVariantList defaultBandValues() {
	QVariantList bands;
	bands.append(QVariant(makeBand(0.7, 2.0)));
	bands.append(QVariant(makeBand(2.0, 6.0)));
	return QVariantList() << 4 << int(RootSumSquare) << QVariant(bands);
}


QString formatNumber(double value) {
	return QString::number(value, 'g', 10);
}


QString formatBands(const QVariantList &values) {
	int order = values[0].toInt();
	int mode = values[1].toInt();
	QVariantList bands = values[2].toList();

	QStringList terms;
	for ( const QVariant &band : bands ) {
		QVariantList corners = band.toList();
		QString term = QString("BW(%1,%2,%3)")
		               .arg(order)
		               .arg(formatNumber(corners[0].toDouble()),
		                    formatNumber(corners[1].toDouble()));

		switch ( mode ) {
			case RootSumSquare:
				terms.append(term + "^2");
				break;
			case SumOfAbsolutes:
				terms.append("|" + term + "|");
				break;
			default:
				terms.append(term);
				break;
		}
	}

	if ( terms.isEmpty() ) {
		return QString();
	}

	if ( mode == RootSumSquare ) {
		return QString("(%1)^0.5").arg(terms.join("+"));
	}

	// Addition binds stronger than chaining, no parentheses required
	return terms.join("+");
}


QString formatStage(const QString &key, const QVariantList &values) {
	if ( key == ExpressionKey ) {
		return values.value(0).toString().trimmed();
	}

	if ( key == BandsKey ) {
		return formatBands(values);
	}

	const StageSpec *spec = findSpec(key);
	if ( !spec || spec->parameters.empty() ) {
		return key;
	}

	QStringList args;
	for ( int i = 0; i < values.size(); ++i ) {
		if ( spec->parameters[i].kind == Real ) {
			args.append(formatNumber(values[i].toDouble()));
		}
		else {
			args.append(QString::number(values[i].toInt()));
		}
	}

	return QString("%1(%2)").arg(key, args.join(","));
}


const char *groundMotionName(int value) {
	switch ( value ) {
		case 0:
			return "displacement";
		case 1:
			return "velocity";
		case 2:
			return "acceleration";
		default:
			return "?";
	}
}


QString describeStage(const QString &key, const QVariantList &values) {
	if ( key == ExpressionKey ) {
		return QObject::tr("Expression: %1").arg(values.value(0).toString());
	}

	if ( key == BandsKey ) {
		QStringList bands;
		for ( const QVariant &band : values[2].toList() ) {
			QVariantList corners = band.toList();
			bands.append(QString("%1-%2 Hz")
			             .arg(formatNumber(corners[0].toDouble()),
			                  formatNumber(corners[1].toDouble())));
		}

		return QObject::tr("Multi-band %1, order %2: %3")
		       .arg(bandModeNames[values[1].toInt()])
		       .arg(values[0].toInt())
		       .arg(bands.join(", "));
	}

	const StageSpec *spec = findSpec(key);
	if ( !spec ) {
		return key;
	}

	QStringList parts;
	for ( int i = 0; i < values.size(); ++i ) {
		const ParameterSpec &param = spec->parameters[i];
		if ( param.kind == GroundMotion ) {
			parts.append(groundMotionName(values[i].toInt()));
		}
		else if ( param.kind == Integer ) {
			parts.append(QString("%1 %2").arg(param.label).arg(values[i].toInt()));
		}
		else {
			parts.append(QString("%1 %2")
			             .arg(formatNumber(values[i].toDouble()), param.unit)
			             .trimmed());
		}
	}

	QString text = spec->label;
	if ( !parts.isEmpty() ) {
		text += QString(": %1").arg(parts.join(", "));
	}

	return text;
}


/**
 * Splits a string at separators which are neither inside parentheses
 * nor inside |...|. Separators are given as strings of one or two
 * characters.
 */
QStringList splitTopLevel(const QString &text, const QStringList &separators) {
	QStringList parts;
	int depth = 0;
	bool insideAbs = false;
	int start = 0;

	for ( int i = 0; i < text.size(); ++i ) {
		QChar c = text[i];

		if ( c == '(' ) {
			++depth;
			continue;
		}

		if ( c == ')' ) {
			--depth;
			continue;
		}

		if ( c == '|' ) {
			insideAbs = !insideAbs;
			continue;
		}

		if ( depth != 0 || insideAbs ) {
			continue;
		}

		for ( const QString &sep : separators ) {
			if ( text.mid(i, sep.size()) != sep ) {
				continue;
			}

			// A sign of an exponent such as 1e+2 does not separate
			if ( sep == "+" && i >= 2
			  && (text[i-1] == 'e' || text[i-1] == 'E')
			  && (text[i-2].isDigit() || text[i-2] == '.') ) {
				continue;
			}

			parts.append(text.mid(start, i - start));
			i += sep.size() - 1;
			start = i + 1;
			break;
		}
	}

	parts.append(text.mid(start));
	return parts;
}


//! Removes parentheses around the whole string
QString stripOuterParentheses(const QString &text) {
	if ( !text.startsWith('(') || !text.endsWith(')') ) {
		return text;
	}

	int depth = 0;
	for ( int i = 0; i < text.size(); ++i ) {
		if ( text[i] == '(' ) {
			++depth;
		}
		else if ( text[i] == ')' ) {
			--depth;
			// The first parenthesis is closed before the end
			if ( depth == 0 && i < text.size() - 1 ) {
				return text;
			}
		}
	}

	return text.mid(1, text.size() - 2);
}


bool parseSimpleStage(const QString &segment, QString &key, QVariantList &values) {
	static const QRegularExpression stageRe(
		"^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*(?:\\(([^()]*)\\))?\\s*$"
	);

	QRegularExpressionMatch match = stageRe.match(segment);
	if ( !match.hasMatch() ) {
		return false;
	}

	const StageSpec *spec = findSpec(match.captured(1));
	if ( !spec ) {
		return false;
	}

	QString args = match.captured(2).trimmed();
	QStringList argList;
	if ( !args.isEmpty() ) {
		argList = args.split(',');
	}

	if ( argList.size() != static_cast<int>(spec->parameters.size()) ) {
		return false;
	}

	values.clear();
	for ( int i = 0; i < argList.size(); ++i ) {
		bool ok;
		double value = argList[i].trimmed().toDouble(&ok);
		if ( !ok ) {
			return false;
		}

		if ( spec->parameters[i].kind == Real ) {
			values.append(value);
		}
		else {
			values.append(static_cast<int>(value));
		}
	}

	key = spec->key;
	return true;
}


/**
 * Recognizes combinations of Butterworth bandpasses with the same order:
 * (BW(o,a,b)^2+BW(o,c,d)^2)^0.5, BW(o,a,b)+BW(o,c,d) and
 * |BW(o,a,b)|+|BW(o,c,d)|.
 */
bool parseBands(const QString &segment, QVariantList &values) {
	static const QString number = "[-+]?(?:\\d+\\.?\\d*|\\.\\d+)(?:[eE][-+]?\\d+)?";
	static const QRegularExpression rssRe("^\\((.*)\\)\\^0*\\.50*$");
	static const QRegularExpression bwRe(
		QString("^BW(?:_BP)?\\((\\d+),(%1),(%1)\\)$").arg(number)
	);

	QString text = segment;
	text.remove(QRegularExpression("\\s"));

	int mode;
	QStringList terms;

	QRegularExpressionMatch rss = rssRe.match(text);
	if ( rss.hasMatch() ) {
		mode = RootSumSquare;
		terms = splitTopLevel(rss.captured(1), QStringList() << "+");
		for ( QString &term : terms ) {
			if ( !term.endsWith("^2") ) {
				return false;
			}
			term.chop(2);
		}
	}
	else {
		terms = splitTopLevel(stripOuterParentheses(text), QStringList() << "+");
		if ( terms.size() < 2 ) {
			// A single band is a simple stage
			return false;
		}

		bool absolute = terms[0].startsWith('|');
		mode = absolute ? SumOfAbsolutes : Sum;
		for ( QString &term : terms ) {
			if ( absolute ) {
				if ( term.size() < 2 || !term.startsWith('|') || !term.endsWith('|') ) {
					return false;
				}
				term = term.mid(1, term.size() - 2);
			}
		}
	}

	int order = -1;
	QVariantList bands;
	for ( const QString &term : terms ) {
		QRegularExpressionMatch match = bwRe.match(term);
		if ( !match.hasMatch() ) {
			return false;
		}

		int termOrder = match.captured(1).toInt();
		if ( order >= 0 && termOrder != order ) {
			return false;
		}
		order = termOrder;

		bands.append(QVariant(makeBand(match.captured(2).toDouble(),
		                               match.captured(3).toDouble())));
	}

	values = QVariantList() << order << mode << QVariant(bands);
	return true;
}


/**
 * Splits a filter string into stages. Plain filters become single stages,
 * Butterworth band combinations become a multi-band stage and everything
 * else an expression stage which keeps the text.
 */
QList<QPair<QString, QVariantList>> parseStages(const QString &filter) {
	QList<QPair<QString, QVariantList>> stages;

	if ( filter.trimmed().isEmpty() ) {
		return stages;
	}

	const QStringList segments = splitTopLevel(filter, QStringList() << ">>" << "->");
	for ( const QString &segment : segments ) {
		QString key;
		QVariantList values;

		if ( parseSimpleStage(segment, key, values) ) {
			stages.append(qMakePair(key, values));
		}
		else if ( parseBands(segment, values) ) {
			stages.append(qMakePair(BandsKey, values));
		}
		else {
			stages.append(qMakePair(ExpressionKey,
			                        QVariantList() << segment.trimmed()));
		}
	}

	return stages;
}


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterBuilderDialog::FilterBuilderDialog(QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f)
, _stageType(nullptr)
, _expression(nullptr)
, _bandOrder(nullptr)
, _bandMode(nullptr)
, _bands(nullptr)
, _fsamp(0)
, _valid(true)
, _saveRequested(false)
, _blockUpdates(false) {
	setWindowTitle(tr("Filter builder"));

	_updateTimer = new QTimer(this);
	_updateTimer->setSingleShot(true);
	_updateTimer->setInterval(0);
	connect(_updateTimer, SIGNAL(timeout()), this, SLOT(updateFromStages()));

	_liveTimer = new QTimer(this);
	_liveTimer->setSingleShot(true);
	_liveTimer->setInterval(300);
	connect(_liveTimer, SIGNAL(timeout()), this, SLOT(emitLiveFilter()));

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	QFormLayout *nameLayout = new QFormLayout;
	_name = new QLineEdit;
	_name->setPlaceholderText(tr("Name shown in the filter list"));
	nameLayout->addRow(tr("Name:"), _name);
	layout->addLayout(nameLayout);

	QHBoxLayout *stageLayout = new QHBoxLayout;

	QVBoxLayout *listLayout = new QVBoxLayout;
	listLayout->addWidget(new QLabel(tr("Stages in processing order (drag to reorder):")));

	_stages = new QListWidget;
	_stages->setDragDropMode(QAbstractItemView::InternalMove);
	_stages->setDefaultDropAction(Qt::MoveAction);
	_stages->setSelectionMode(QAbstractItemView::SingleSelection);
	listLayout->addWidget(_stages);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	_btnAdd = new QToolButton;
	_btnAdd->setIcon(icon("add"));
	_btnAdd->setToolTip(tr("Add a stage"));
	_btnAdd->setPopupMode(QToolButton::InstantPopup);
	QMenu *addMenu = new QMenu(_btnAdd);
	for ( const auto &spec : stageSpecs() ) {
		QAction *action = addMenu->addAction(QString("%1 (%2)").arg(spec.label, spec.key));
		action->setData(QString(spec.key));
		action->setToolTip(spec.description);
	}
	addMenu->addSeparator();
	QAction *bandsAction = addMenu->addAction(tr("Multi-band combination"));
	bandsAction->setData(BandsKey);
	QAction *expressionAction = addMenu->addAction(tr("Expression"));
	expressionAction->setData(ExpressionKey);
	connect(addMenu, &QMenu::triggered, this, [this](QAction *action) {
		addStage(action->data().toString());
	});
	_btnAdd->setMenu(addMenu);

	_btnRemove = new QToolButton;
	_btnRemove->setIcon(icon("remove"));
	_btnRemove->setToolTip(tr("Remove the selected stage"));
	_btnUp = new QToolButton;
	_btnUp->setIcon(icon("up"));
	_btnUp->setToolTip(tr("Move the selected stage up"));
	_btnDown = new QToolButton;
	_btnDown->setIcon(icon("down"));
	_btnDown->setToolTip(tr("Move the selected stage down"));

	buttonLayout->addWidget(_btnAdd);
	buttonLayout->addWidget(_btnRemove);
	buttonLayout->addWidget(_btnUp);
	buttonLayout->addWidget(_btnDown);
	buttonLayout->addStretch();
	listLayout->addLayout(buttonLayout);

	stageLayout->addLayout(listLayout, 3);

	_paramGroup = new QGroupBox(tr("Selected stage"));
	_paramLayout = new QFormLayout;
	_paramGroup->setLayout(_paramLayout);
	stageLayout->addWidget(_paramGroup, 2);

	layout->addLayout(stageLayout);

	QFormLayout *stringLayout = new QFormLayout;
	_filterString = new QLineEdit;
	_filterString->setToolTip(tr("The filter string as stored in the "
	                             "configuration. It can be edited directly."));
	stringLayout->addRow(tr("Filter string:"), _filterString);
	_status = new QLabel;
	_status->setWordWrap(true);
	stringLayout->addRow(QString(), _status);
	layout->addLayout(stringLayout);

	_liveApply = new QCheckBox(tr("Apply live to the traces"));
	_liveApply->setVisible(false);
	layout->addWidget(_liveApply);

	QDialogButtonBox *buttons = new QDialogButtonBox;
	_btnOk = buttons->addButton(QDialogButtonBox::Ok);
	buttons->addButton(QDialogButtonBox::Cancel);
	_btnSave = buttons->addButton(tr("Save to configuration"), QDialogButtonBox::AcceptRole);
	_btnSave->setToolTip(tr("Use the filter and add it permanently to the "
	                        "configured filters"));
	_btnSave->setVisible(false);
	layout->addWidget(buttons);

	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	connect(_btnOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(_btnSave, SIGNAL(clicked()), this, SLOT(saveClicked()));

	connect(_btnRemove, SIGNAL(clicked()), this, SLOT(removeStage()));
	connect(_btnUp, SIGNAL(clicked()), this, SLOT(moveStageUp()));
	connect(_btnDown, SIGNAL(clicked()), this, SLOT(moveStageDown()));
	connect(_stages, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
	        this, SLOT(currentStageChanged()));

	// Drag and drop moves are reported as row insert/remove pairs or as
	// moves depending on the Qt version, so catch all of them.
	connect(_stages->model(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
	        this, SLOT(scheduleUpdate()));
	connect(_stages->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
	        this, SLOT(scheduleUpdate()));
	connect(_stages->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
	        this, SLOT(scheduleUpdate()));

	connect(_filterString, SIGNAL(textEdited(QString)),
	        this, SLOT(filterStringEdited(QString)));
	connect(_liveApply, SIGNAL(toggled(bool)), this, SLOT(liveApplyToggled(bool)));

	currentStageChanged();
	validate();

	resize(760, 440);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setFilterName(const QString &name) {
	_name->setText(name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString FilterBuilderDialog::filterName() const {
	return _name->text().trimmed();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setFilter(const QString &filter) {
	_filterString->setText(filter.trimmed());
	setStages(filter);
	validate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString FilterBuilderDialog::filter() const {
	return _filterString->text().trimmed();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setSamplingFrequency(double fsamp) {
	_fsamp = fsamp;
	validate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setLiveApplyEnabled(bool enable) {
	_liveApply->setVisible(enable);
	if ( !enable ) {
		_liveApply->setChecked(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FilterBuilderDialog::isLiveApplyActive() const {
	return _liveApply->isVisible() && _liveApply->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setSaveEnabled(bool enable) {
	_btnSave->setVisible(enable);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FilterBuilderDialog::saveRequested() const {
	return _saveRequested;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FilterBuilderDialog::ValidateFilter(const QString &filter, QString *error) {
	if ( filter.trimmed().isEmpty() ) {
		if ( error ) {
			*error = tr("The filter is empty");
		}
		return false;
	}

	std::string strError;
	std::unique_ptr<Math::Filtering::InPlaceFilter<double>> f(
		Math::Filtering::InPlaceFilter<double>::Create(filter.toStdString(), &strError)
	);

	if ( !f ) {
		if ( error ) {
			*error = strError.empty() ? tr("Syntax error") : QString(strError.c_str());
		}
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QListWidgetItem *FilterBuilderDialog::createStageItem(const QString &key,
                                                      const QVariantList &values) {
	QListWidgetItem *item = new QListWidgetItem;
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
	item->setData(Qt::UserRole, key);
	item->setData(Qt::UserRole + 1, values);
	updateStageItem(item);
	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::updateStageItem(QListWidgetItem *item) {
	QString key = item->data(Qt::UserRole).toString();
	QVariantList values = item->data(Qt::UserRole + 1).toList();
	item->setText(describeStage(key, values));
	item->setToolTip(formatStage(key, values));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::addStage(const QString &key) {
	QVariantList values;

	if ( key == BandsKey ) {
		values = defaultBandValues();
	}
	else if ( key == ExpressionKey ) {
		values << QString();
	}
	else {
		const StageSpec *spec = findSpec(key);
		if ( !spec ) {
			return;
		}
		values = defaultValues(*spec);
	}

	int row = _stages->currentRow() + 1;
	if ( row <= 0 ) {
		row = _stages->count();
	}

	QListWidgetItem *item = createStageItem(key, values);
	_stages->insertItem(row, item);
	_stages->setCurrentItem(item);
	scheduleUpdate();

	if ( _expression ) {
		_expression->setFocus();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::removeStage() {
	delete _stages->currentItem();
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::moveStageUp() {
	int row = _stages->currentRow();
	if ( row <= 0 ) {
		return;
	}

	QListWidgetItem *item = _stages->takeItem(row);
	_stages->insertItem(row - 1, item);
	_stages->setCurrentItem(item);
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::moveStageDown() {
	int row = _stages->currentRow();
	if ( row < 0 || row >= _stages->count() - 1 ) {
		return;
	}

	QListWidgetItem *item = _stages->takeItem(row);
	_stages->insertItem(row + 1, item);
	_stages->setCurrentItem(item);
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::currentStageChanged() {
	QListWidgetItem *item = _stages->currentItem();
	int row = _stages->currentRow();

	_btnRemove->setEnabled(item != nullptr);
	_btnUp->setEnabled(item != nullptr && row > 0);
	_btnDown->setEnabled(item != nullptr && row < _stages->count() - 1);

	buildParameterEditor(item);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::buildParameterEditor(QListWidgetItem *item) {
	while ( _paramLayout->rowCount() > 0 ) {
		_paramLayout->removeRow(0);
	}

	_stageType = nullptr;
	_expression = nullptr;
	_bandOrder = nullptr;
	_bandMode = nullptr;
	_bands = nullptr;

	if ( !item ) {
		QLabel *hint = new QLabel(tr("Add a stage with the + button or select "
		                             "a stage to edit its parameters."));
		hint->setWordWrap(true);
		_paramLayout->addRow(hint);
		return;
	}

	QString key = item->data(Qt::UserRole).toString();
	QVariantList values = item->data(Qt::UserRole + 1).toList();

	// The composite stages follow the single filters in the type list
	int bandsIndex = static_cast<int>(stageSpecs().size());
	int expressionIndex = bandsIndex + 1;

	_stageType = new QComboBox;
	int currentIndex = -1;
	for ( size_t i = 0; i < stageSpecs().size(); ++i ) {
		_stageType->addItem(stageSpecs()[i].label);
		_stageType->setItemData(static_cast<int>(i), stageSpecs()[i].description, Qt::ToolTipRole);
		if ( key == stageSpecs()[i].key ) {
			currentIndex = static_cast<int>(i);
		}
	}
	_stageType->addItem(tr("Multi-band combination"));
	_stageType->setItemData(bandsIndex, tr("Combination of Butterworth "
	                                       "bandpasses with the same order"),
	                        Qt::ToolTipRole);
	_stageType->addItem(tr("Expression"));
	_stageType->setItemData(expressionIndex, tr("Any part of the filter "
	                                            "grammar as text"),
	                        Qt::ToolTipRole);

	if ( key == BandsKey ) {
		currentIndex = bandsIndex;
	}
	else if ( key == ExpressionKey ) {
		currentIndex = expressionIndex;
	}

	_stageType->setCurrentIndex(currentIndex);
	connect(_stageType, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(stageTypeChanged(int)));
	_paramLayout->addRow(tr("Type:"), _stageType);

	if ( key == ExpressionKey ) {
		buildExpressionEditor(values);
		return;
	}

	if ( key == BandsKey ) {
		buildMultiBandEditor(values);
		return;
	}

	const StageSpec *spec = findSpec(key);
	if ( !spec ) {
		return;
	}

	for ( size_t i = 0; i < spec->parameters.size(); ++i ) {
		const ParameterSpec &param = spec->parameters[i];
		QWidget *editor;

		if ( param.kind == GroundMotion ) {
			QComboBox *combo = new QComboBox;
			for ( int v = 0; v <= 2; ++v ) {
				combo->addItem(groundMotionName(v));
			}
			combo->setCurrentIndex(values[static_cast<int>(i)].toInt());
			connect(combo, SIGNAL(currentIndexChanged(int)),
			        this, SLOT(stageParameterChanged()));
			editor = combo;
		}
		else if ( param.kind == Integer ) {
			QSpinBox *spin = new QSpinBox;
			spin->setRange(static_cast<int>(param.minimum), static_cast<int>(param.maximum));
			spin->setValue(values[static_cast<int>(i)].toInt());
			spin->setKeyboardTracking(false);
			connect(spin, SIGNAL(valueChanged(int)),
			        this, SLOT(stageParameterChanged()));
			editor = spin;
		}
		else {
			QDoubleSpinBox *spin = new QDoubleSpinBox;
			spin->setRange(param.minimum, param.maximum);
			spin->setDecimals(param.decimals);
			spin->setValue(values[static_cast<int>(i)].toDouble());
			// Typing "15" should not apply "1" in between
			spin->setKeyboardTracking(false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
			spin->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
#endif
			if ( param.unit[0] ) {
				spin->setSuffix(QString(" %1").arg(param.unit));
			}
			connect(spin, SIGNAL(valueChanged(double)),
			        this, SLOT(stageParameterChanged()));
			editor = spin;
		}

		editor->setProperty("parameterIndex", static_cast<int>(i));
		_paramLayout->addRow(QString("%1:").arg(param.label), editor);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::buildExpressionEditor(const QVariantList &values) {
	_expression = new QLineEdit;
	_expression->setText(values.value(0).toString());
	_expression->setPlaceholderText("|BW(3,1,8)| or BW(3,1,8)^2");
	_expression->setToolTip(tr("Any expression of the filter grammar: "
	                           "filters, numbers, + - * / ^, |...| for the "
	                           "absolute value and parentheses"));
	connect(_expression, SIGNAL(textEdited(QString)),
	        this, SLOT(expressionEdited(QString)));
	_paramLayout->addRow(tr("Expression:"), _expression);
	expressionEdited(_expression->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::buildMultiBandEditor(const QVariantList &values) {
	_bandOrder = new QSpinBox;
	_bandOrder->setRange(1, 10);
	_bandOrder->setValue(values[0].toInt());
	_bandOrder->setKeyboardTracking(false);
	_bandOrder->setToolTip(tr("Order of all Butterworth bandpasses"));
	_paramLayout->addRow(tr("Order:"), _bandOrder);

	_bandMode = new QComboBox;
	_bandMode->addItem(tr("Root-sum-square"));
	_bandMode->setItemData(RootSumSquare, "(BW^2+BW^2)^0.5", Qt::ToolTipRole);
	_bandMode->addItem(tr("Sum"));
	_bandMode->setItemData(Sum, "BW+BW", Qt::ToolTipRole);
	_bandMode->addItem(tr("Sum of absolute values"));
	_bandMode->setItemData(SumOfAbsolutes, "|BW|+|BW|", Qt::ToolTipRole);
	_bandMode->setCurrentIndex(values[1].toInt());
	_paramLayout->addRow(tr("Combine:"), _bandMode);

	_bands = new QTableWidget(0, 2);
	_bands->setHorizontalHeaderLabels(QStringList() << tr("Lower (Hz)") << tr("Upper (Hz)"));
	_bands->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	_bands->verticalHeader()->setVisible(false);
	_bands->setSelectionBehavior(QAbstractItemView::SelectRows);
	_bands->setSelectionMode(QAbstractItemView::SingleSelection);
	_bands->setMinimumHeight(110);

	for ( const QVariant &band : values[2].toList() ) {
		QVariantList corners = band.toList();
		int row = _bands->rowCount();
		_bands->insertRow(row);
		_bands->setItem(row, 0, new QTableWidgetItem(formatNumber(corners[0].toDouble())));
		_bands->setItem(row, 1, new QTableWidgetItem(formatNumber(corners[1].toDouble())));
	}

	QToolButton *add = new QToolButton;
	add->setIcon(icon("add"));
	add->setToolTip(tr("Add a band"));
	QToolButton *remove = new QToolButton;
	remove->setIcon(icon("remove"));
	remove->setToolTip(tr("Remove the selected band"));

	QWidget *bandWidget = new QWidget;
	QVBoxLayout *bandLayout = new QVBoxLayout;
	bandLayout->setContentsMargins(0, 0, 0, 0);
	bandLayout->addWidget(_bands);
	QHBoxLayout *bandButtons = new QHBoxLayout;
	bandButtons->addWidget(add);
	bandButtons->addWidget(remove);
	bandButtons->addStretch();
	bandLayout->addLayout(bandButtons);
	bandWidget->setLayout(bandLayout);
	_paramLayout->addRow(tr("Bands:"), bandWidget);

	connect(_bandOrder, SIGNAL(valueChanged(int)), this, SLOT(multiBandChanged()));
	connect(_bandMode, SIGNAL(currentIndexChanged(int)), this, SLOT(multiBandChanged()));
	connect(_bands, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(multiBandChanged()));
	connect(add, SIGNAL(clicked()), this, SLOT(addBand()));
	connect(remove, SIGNAL(clicked()), this, SLOT(removeBand()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::stageTypeChanged(int index) {
	QListWidgetItem *item = _stages->currentItem();
	if ( !item || index < 0 ) {
		return;
	}

	QString oldKey = item->data(Qt::UserRole).toString();
	QVariantList oldValues = item->data(Qt::UserRole + 1).toList();
	const StageSpec *oldSpec = findSpec(oldKey);

	int bandsIndex = static_cast<int>(stageSpecs().size());
	int expressionIndex = bandsIndex + 1;

	QString key;
	QVariantList values;

	if ( index == expressionIndex ) {
		// Keep the stage as it is, just as editable text
		key = ExpressionKey;
		values << formatStage(oldKey, oldValues);
	}
	else if ( index == bandsIndex ) {
		key = BandsKey;
		values = defaultBandValues();
		// Start from a bandpass with its order and corners
		if ( oldSpec && QString(oldSpec->key) == "BW" ) {
			QVariantList bands;
			bands.append(QVariant(makeBand(oldValues[1].toDouble(), oldValues[2].toDouble())));
			values = QVariantList() << oldValues[0] << int(RootSumSquare) << QVariant(bands);
		}
	}
	else {
		const StageSpec &spec = stageSpecs()[index];
		key = spec.key;
		values = defaultValues(spec);

		// Keep values of parameters which have the same meaning, e.g. the
		// order and corners when switching between Butterworth types.
		if ( oldSpec ) {
			for ( size_t i = 0; i < spec.parameters.size() && i < oldSpec->parameters.size(); ++i ) {
				if ( spec.parameters[i].kind == oldSpec->parameters[i].kind
				  && spec.parameters[i].isFrequency == oldSpec->parameters[i].isFrequency
				  && QString(spec.parameters[i].unit) == oldSpec->parameters[i].unit ) {
					values[static_cast<int>(i)] = oldValues[static_cast<int>(i)];
				}
			}
		}
		else if ( oldKey == BandsKey && spec.parameters.size() == 3
		       && spec.parameters[1].isFrequency ) {
			// The first band of a multi-band stage
			QVariantList bands = oldValues[2].toList();
			values[0] = oldValues[0];
			if ( !bands.isEmpty() ) {
				values[1] = bands[0].toList()[0];
				values[2] = bands[0].toList()[1];
			}
		}
	}

	item->setData(Qt::UserRole, key);
	item->setData(Qt::UserRole + 1, values);
	updateStageItem(item);

	// Rebuilding the editor deletes the sender, so defer it
	QTimer::singleShot(0, this, SLOT(currentStageChanged()));
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::stageParameterChanged() {
	QListWidgetItem *item = _stages->currentItem();
	QWidget *editor = qobject_cast<QWidget*>(sender());
	if ( !item || !editor ) {
		return;
	}

	int index = editor->property("parameterIndex").toInt();
	QVariantList values = item->data(Qt::UserRole + 1).toList();
	if ( index < 0 || index >= values.size() ) {
		return;
	}

	if ( QComboBox *combo = qobject_cast<QComboBox*>(editor) ) {
		values[index] = combo->currentIndex();
	}
	else if ( QSpinBox *spin = qobject_cast<QSpinBox*>(editor) ) {
		values[index] = spin->value();
	}
	else if ( QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox*>(editor) ) {
		values[index] = spin->value();
	}

	item->setData(Qt::UserRole + 1, values);
	updateStageItem(item);
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::expressionEdited(const QString &text) {
	QListWidgetItem *item = _stages->currentItem();
	if ( !item || !_expression ) {
		return;
	}

	QString error;
	QPalette pal = palette();
	if ( !ValidateFilter(text, &error) ) {
		pal.setColor(QPalette::Text, Qt::red);
		_expression->setToolTip(tr("Invalid expression: %1").arg(error));
	}
	else {
		_expression->setToolTip(QString());
	}
	_expression->setPalette(pal);

	if ( item->data(Qt::UserRole + 1).toList().value(0).toString() == text ) {
		return;
	}

	item->setData(Qt::UserRole + 1, QVariantList() << text);
	updateStageItem(item);
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::multiBandChanged() {
	QListWidgetItem *item = _stages->currentItem();
	if ( !item || !_bands ) {
		return;
	}

	QVariantList bands;
	bool complete = true;

	_bands->blockSignals(true);
	for ( int row = 0; row < _bands->rowCount(); ++row ) {
		double corners[2];
		for ( int col = 0; col < 2; ++col ) {
			QTableWidgetItem *cell = _bands->item(row, col);
			bool ok = false;
			if ( cell ) {
				corners[col] = cell->text().trimmed().toDouble(&ok);
				ok = ok && corners[col] > 0;
				cell->setForeground(ok ? palette().color(QPalette::Text) : QColor(Qt::red));
			}
			complete = complete && ok;
		}

		if ( complete ) {
			bands.append(QVariant(makeBand(corners[0], corners[1])));
		}
	}
	_bands->blockSignals(false);

	// Keep the last valid state until every corner is a valid number
	if ( !complete ) {
		return;
	}

	item->setData(Qt::UserRole + 1, QVariantList()
	              << _bandOrder->value() << _bandMode->currentIndex()
	              << QVariant(bands));
	updateStageItem(item);
	scheduleUpdate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::addBand() {
	if ( !_bands ) {
		return;
	}

	// Continue above the last band
	double lower = 1.0;
	double upper = 2.0;
	int last = _bands->rowCount() - 1;
	if ( last >= 0 && _bands->item(last, 1) ) {
		bool ok;
		double value = _bands->item(last, 1)->text().toDouble(&ok);
		if ( ok && value > 0 ) {
			lower = value;
			upper = value * 2;
		}
	}

	_bands->blockSignals(true);
	int row = _bands->rowCount();
	_bands->insertRow(row);
	_bands->setItem(row, 0, new QTableWidgetItem(formatNumber(lower)));
	_bands->setItem(row, 1, new QTableWidgetItem(formatNumber(upper)));
	_bands->blockSignals(false);
	_bands->selectRow(row);

	multiBandChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::removeBand() {
	if ( !_bands || _bands->currentRow() < 0 ) {
		return;
	}

	_bands->removeRow(_bands->currentRow());
	multiBandChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::filterStringEdited(const QString &text) {
	setStages(text);
	validate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::scheduleUpdate() {
	if ( _blockUpdates ) {
		return;
	}

	_updateTimer->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::updateFromStages() {
	QStringList parts;
	for ( int i = 0; i < _stages->count(); ++i ) {
		QListWidgetItem *item = _stages->item(i);
		parts.append(formatStage(item->data(Qt::UserRole).toString(),
		                         item->data(Qt::UserRole + 1).toList()));
	}

	_filterString->setText(parts.join(">>"));

	// The current row may have changed after a drag and drop
	int row = _stages->currentRow();
	_btnUp->setEnabled(row > 0);
	_btnDown->setEnabled(row >= 0 && row < _stages->count() - 1);

	validate();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setStages(const QString &filter) {
	_blockUpdates = true;
	int row = _stages->currentRow();
	_stages->clear();
	for ( const auto &stage : parseStages(filter) ) {
		_stages->addItem(createStageItem(stage.first, stage.second));
	}
	if ( row >= _stages->count() ) {
		row = _stages->count() - 1;
	}
	_stages->setCurrentRow(row);
	_blockUpdates = false;

	currentStageChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::validate() {
	QString error;
	_valid = ValidateFilter(filter(), &error);

	QStringList warnings;
	double nyquist = _fsamp * 0.5;

	auto checkBand = [&](int stage, double lower, double upper) {
		if ( lower >= upper ) {
			warnings.append(tr("The lower corner of stage %1 is not below "
			                   "the upper corner").arg(stage));
		}
		if ( _fsamp > 0 && upper >= nyquist ) {
			warnings.append(tr("Upper corner of stage %1 is not below the "
			                   "Nyquist frequency of %2 Hz")
			                .arg(stage).arg(formatNumber(nyquist)));
		}
	};

	for ( int i = 0; i < _stages->count(); ++i ) {
		QListWidgetItem *item = _stages->item(i);
		QString key = item->data(Qt::UserRole).toString();
		QVariantList values = item->data(Qt::UserRole + 1).toList();

		if ( key == ExpressionKey ) {
			QString stageError;
			if ( !ValidateFilter(values.value(0).toString(), &stageError) ) {
				// Point to the stage which breaks the whole filter
				error = tr("stage %1: %2").arg(i + 1).arg(stageError);
			}
			continue;
		}

		if ( key == BandsKey ) {
			QVariantList bands = values[2].toList();
			if ( bands.isEmpty() ) {
				error = tr("stage %1 has no bands").arg(i + 1);
			}
			for ( const QVariant &band : bands ) {
				QVariantList corners = band.toList();
				checkBand(i + 1, corners[0].toDouble(), corners[1].toDouble());
			}
			continue;
		}

		const StageSpec *spec = findSpec(key);
		if ( !spec ) {
			continue;
		}

		if ( spec->parameters.size() >= 3
		  && spec->parameters[1].isFrequency && spec->parameters[2].isFrequency ) {
			checkBand(i + 1, values[1].toDouble(), values[2].toDouble());
			continue;
		}

		if ( _fsamp > 0 ) {
			for ( size_t p = 0; p < spec->parameters.size(); ++p ) {
				if ( spec->parameters[p].isFrequency
				  && values[static_cast<int>(p)].toDouble() >= nyquist ) {
					warnings.append(tr("%1 of stage %2 is not below the Nyquist "
					                   "frequency of %3 Hz")
					                .arg(spec->parameters[p].label)
					                .arg(i + 1)
					                .arg(formatNumber(nyquist)));
				}
			}
		}
	}

	QPalette pal = _filterString->palette();
	if ( !_valid ) {
		pal.setColor(QPalette::Text, Qt::red);
		_status->setText(tr("Invalid filter: %1").arg(error));
	}
	else if ( !warnings.isEmpty() ) {
		pal.setColor(QPalette::Text, palette().color(QPalette::Text));
		_status->setText(tr("Valid, but: %1").arg(warnings.join("; ")));
	}
	else {
		pal.setColor(QPalette::Text, palette().color(QPalette::Text));
		_status->setText(tr("Valid filter"));
	}
	_filterString->setPalette(pal);

	_btnOk->setEnabled(_valid);
	_btnSave->setEnabled(_valid);

	if ( _valid && isLiveApplyActive() ) {
		_liveTimer->start();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::emitLiveFilter() {
	if ( _valid && isLiveApplyActive() ) {
		emit filterChanged(filter());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::liveApplyToggled(bool checked) {
	if ( checked ) {
		emitLiveFilter();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::saveClicked() {
	_saveRequested = true;
	accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
