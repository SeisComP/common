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
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
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


// Only filters with a fixed parameter list are offered as stages. Everything
// else is still accepted as raw filter string.
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


QString formatNumber(double value) {
	return QString::number(value, 'g', 10);
}


QString formatStage(const QString &key, const QVariantList &values) {
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
 * Splits a plain filter chain into stages. Returns false if the string
 * contains anything else than known stages joined by >>, e.g. arithmetic
 * expressions, nested filters or unknown filters.
 */
bool parseStages(const QString &filter,
                 QList<QPair<QString, QVariantList>> &stages) {
	static const QRegularExpression stageRe(
		"^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*(?:\\(([^()]*)\\))?\\s*$"
	);

	stages.clear();

	if ( filter.trimmed().isEmpty() ) {
		return true;
	}

	const QStringList tokens = filter.split(">>");
	for ( const QString &token : tokens ) {
		QRegularExpressionMatch match = stageRe.match(token);
		if ( !match.hasMatch() ) {
			return false;
		}

		QString key = match.captured(1);
		const StageSpec *spec = findSpec(key);
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

		QVariantList values;
		for ( int i = 0; i < argList.size(); ++i ) {
			bool ok;
			double value = argList[i].trimmed().toDouble(&ok);
			if ( !ok ) {
				return false;
			}

			const ParameterSpec &param = spec->parameters[i];
			if ( param.kind == Real ) {
				values.append(value);
			}
			else {
				values.append(static_cast<int>(value));
			}
		}

		stages.append(qMakePair(QString(spec->key), values));
	}

	return true;
}


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterBuilderDialog::FilterBuilderDialog(QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f)
, _stageType(nullptr)
, _fsamp(0)
, _rawMode(false)
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

	_rawHint = new QLabel(tr("This filter cannot be shown as a list of stages. "
	                         "Edit the filter string below or clear it to "
	                         "start over."));
	_rawHint->setWordWrap(true);
	_rawHint->setVisible(false);
	listLayout->addWidget(_rawHint);

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

	resize(720, 420);
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
	const StageSpec *spec = findSpec(key);
	if ( !spec ) {
		return;
	}

	if ( _rawMode ) {
		// Starting over from an unrepresentable string
		_stages->clear();
		setRawMode(false);
	}

	int row = _stages->currentRow() + 1;
	if ( row <= 0 ) {
		row = _stages->count();
	}

	QListWidgetItem *item = createStageItem(spec->key, defaultValues(*spec));
	_stages->insertItem(row, item);
	_stages->setCurrentItem(item);
	scheduleUpdate();
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

	if ( !item ) {
		QLabel *hint = new QLabel(tr("Add a stage with the + button or select "
		                             "a stage to edit its parameters."));
		hint->setWordWrap(true);
		_paramLayout->addRow(hint);
		return;
	}

	QString key = item->data(Qt::UserRole).toString();
	QVariantList values = item->data(Qt::UserRole + 1).toList();
	const StageSpec *spec = findSpec(key);
	if ( !spec ) {
		return;
	}

	_stageType = new QComboBox;
	int currentIndex = 0;
	for ( size_t i = 0; i < stageSpecs().size(); ++i ) {
		_stageType->addItem(stageSpecs()[i].label, QString(stageSpecs()[i].key));
		if ( &stageSpecs()[i] == spec ) {
			currentIndex = static_cast<int>(i);
		}
	}
	_stageType->setCurrentIndex(currentIndex);
	_stageType->setToolTip(spec->description);
	connect(_stageType, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(stageTypeChanged(int)));
	_paramLayout->addRow(tr("Type:"), _stageType);

	for ( size_t i = 0; i < spec->parameters.size(); ++i ) {
		const ParameterSpec &param = spec->parameters[i];
		QWidget *editor;

		if ( param.kind == GroundMotion ) {
			QComboBox *combo = new QComboBox;
			for ( int v = 0; v <= 2; ++v ) {
				combo->addItem(groundMotionName(v));
			}
			combo->setCurrentIndex(values[i].toInt());
			connect(combo, SIGNAL(currentIndexChanged(int)),
			        this, SLOT(stageParameterChanged()));
			editor = combo;
		}
		else if ( param.kind == Integer ) {
			QSpinBox *spin = new QSpinBox;
			spin->setRange(static_cast<int>(param.minimum), static_cast<int>(param.maximum));
			spin->setValue(values[i].toInt());
			spin->setKeyboardTracking(false);
			connect(spin, SIGNAL(valueChanged(int)),
			        this, SLOT(stageParameterChanged()));
			editor = spin;
		}
		else {
			QDoubleSpinBox *spin = new QDoubleSpinBox;
			spin->setRange(param.minimum, param.maximum);
			spin->setDecimals(param.decimals);
			spin->setValue(values[i].toDouble());
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
void FilterBuilderDialog::stageTypeChanged(int index) {
	QListWidgetItem *item = _stages->currentItem();
	if ( !item || index < 0 ) {
		return;
	}

	const StageSpec &spec = stageSpecs()[index];
	QVariantList oldValues = item->data(Qt::UserRole + 1).toList();
	const StageSpec *oldSpec = findSpec(item->data(Qt::UserRole).toString());
	QVariantList values = defaultValues(spec);

	// Keep values of parameters which have the same meaning, e.g. the order
	// and corners when switching between Butterworth types.
	if ( oldSpec ) {
		for ( size_t i = 0; i < spec.parameters.size(); ++i ) {
			if ( i >= oldSpec->parameters.size() ) {
				break;
			}

			if ( spec.parameters[i].kind == oldSpec->parameters[i].kind
			  && spec.parameters[i].isFrequency == oldSpec->parameters[i].isFrequency
			  && QString(spec.parameters[i].unit) == oldSpec->parameters[i].unit ) {
				values[static_cast<int>(i)] = oldValues[static_cast<int>(i)];
			}
		}
	}

	item->setData(Qt::UserRole, QString(spec.key));
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
	if ( _rawMode ) {
		return;
	}

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
	QList<QPair<QString, QVariantList>> stages;

	if ( !parseStages(filter, stages) ) {
		setRawMode(true);
		return;
	}

	_blockUpdates = true;
	int row = _stages->currentRow();
	_stages->clear();
	for ( const auto &stage : stages ) {
		_stages->addItem(createStageItem(stage.first, stage.second));
	}
	if ( row >= _stages->count() ) {
		row = _stages->count() - 1;
	}
	_stages->setCurrentRow(row);
	_blockUpdates = false;

	setRawMode(false);
	currentStageChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::setRawMode(bool raw) {
	_rawMode = raw;
	_stages->setEnabled(!raw);
	_paramGroup->setEnabled(!raw);
	_btnRemove->setEnabled(!raw && _stages->currentItem());
	_btnUp->setEnabled(!raw && _stages->currentRow() > 0);
	_btnDown->setEnabled(!raw && _stages->currentRow() >= 0
	                     && _stages->currentRow() < _stages->count() - 1);
	_rawHint->setVisible(raw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterBuilderDialog::validate() {
	QString error;
	_valid = ValidateFilter(filter(), &error);

	QStringList warnings;

	for ( int i = 0; _valid && !_rawMode && i < _stages->count(); ++i ) {
		QListWidgetItem *item = _stages->item(i);
		const StageSpec *spec = findSpec(item->data(Qt::UserRole).toString());
		QVariantList values = item->data(Qt::UserRole + 1).toList();
		if ( !spec ) {
			continue;
		}

		if ( _fsamp > 0 ) {
			double nyquist = _fsamp * 0.5;
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

		if ( spec->parameters.size() >= 3
		  && spec->parameters[1].isFrequency && spec->parameters[2].isFrequency
		  && values[1].toDouble() >= values[2].toDouble() ) {
			warnings.append(tr("The lower corner of stage %1 is not below "
			                   "the upper corner").arg(i + 1));
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
