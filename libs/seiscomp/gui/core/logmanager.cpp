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

#define SEISCOMP_COMPONENT Gui::LogManager

#include "logmanager.h"
#include "logmanager_p.h"

#include <seiscomp/core/datetime.h>
#include <seiscomp/core/enumeration.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/utils.h>

#include <QItemDelegate>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QToolButton>

#include <boost/algorithm/string.hpp>

#define SC_D (*_d_ptr)

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


constexpr auto LogManagerGroup = "LogManager";

enum class ChannelColumns : uint8_t {
	Name,
	Subscribe,
	Notify,
	Count
};


template< size_t N >
constexpr size_t length(char const (&/*value*/)[N]) {
	return N - 1;
}

struct LogLevel {
	const char *title;
	const char *name;
	const char *iconName;
	QColor color;
};

const std::vector<LogLevel> LogLevels = {
	{"Undefined", "undefined", "log", Qt::black},
	{"Critical", "critical", "critical", QColor(0xff00d7)},
	{"Error", "error", "error", QColor(0xff2222)},
	{"Warning", "warning", "warning_inv", QColor(0xf29100)},
	{"Notice", "notice", "note", QColor(0x008bda)},
	{"Info", "info", "info", QColor(0x00a97e)},
	{"Debug", "debug", "debug", QColor(0x8a8a8a)}
};


QIcon logIcon(int level) {
	if ( level < 0 || static_cast<size_t>(level) >= LogLevels.size() ) {
		return {};
	}

	const auto &logLevel = LogLevels[level];
	return icon(logLevel.iconName, logLevel.color);
}


inline void setupLogAction(QAction *action, int level) {
	action->setData(level);
	action->setIcon(logIcon(level));
}


MAKEENUM(
	Columns,
	EVALUES(
		Time,
		Level,
		Component,
		Context,
		Message
	),
	ENAMES(
		"Time",
		"",
		"Component",
		"Context",
		"Message"
	)
);


inline QString rowsToText(const QModelIndexList &indexes) {
	QString content;

	foreach ( const QModelIndex &index, indexes ) {
		// index is just the first column of a row
		content += index.siblingAtColumn(Time).data(Qt::DisplayRole).toString();
		content += " [";
		content += LogLevels[index.siblingAtColumn(Level).data(Qt::UserRole).toInt()].name;
		if ( index.siblingAtColumn(Component).data(Qt::DisplayRole).isValid() ) {
			content += "/";
			content += index.siblingAtColumn(Component).data(Qt::DisplayRole).toString();
		}
		content += "] ";
		if ( index.siblingAtColumn(Context).data(Qt::DisplayRole).isValid() ) {
			content += "(";
			content += index.siblingAtColumn(Context).data(Qt::DisplayRole).toString();
			content += ") ";
		}
		content += index.siblingAtColumn(Message).data(Qt::DisplayRole).toString();
		content += "\n";
	}

	return content;
}


inline QStyleOptionButton checkBoxStyle(const QStyleOptionViewItem &options) {
	QStyleOptionButton styleOption;
	QRect rect = QApplication::style()->subElementRect(
		QStyle::SE_CheckBoxIndicator, &styleOption
	);
	QPoint pos(options.rect.x() + options.rect.width() / 2 - rect.width() / 2,
	           options.rect.y() + options.rect.height() / 2 - rect.height() / 2);
	styleOption.rect = QRect(pos, rect.size());
	return styleOption;
}


class HeaderView : public QHeaderView {
	public:
		HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

	private slots:
		void showContextMenu(const QPoint &pos);
};


class CheckBoxDelegate : public QItemDelegate {
	public:
		CheckBoxDelegate(QObject *parent) : QItemDelegate(parent) {}

		void paint(QPainter *painter, const QStyleOptionViewItem &option,
		           const QModelIndex &index) const override {
			QVariant data = index.data(Qt::CheckStateRole);
			if ( !data.isValid() ) {
				QItemDelegate::paint(painter, option, index);
				return;
			}

			QStyleOptionButton styleOption = checkBoxStyle(option);
			styleOption.state |= QStyle::State_Enabled;
			styleOption.state |= data.toBool() ? QStyle::State_On : QStyle::State_Off;
			QApplication::style()->drawControl(QStyle::CE_CheckBox, &styleOption, painter);
		}

		bool editorEvent(QEvent *event,
		                 QAbstractItemModel *model,
		                 const QStyleOptionViewItem &option,
		                 const QModelIndex &index) override {
			if ( event->type() == QEvent::MouseButtonPress ) {
				auto *mouseEvent = static_cast<QMouseEvent *>(event);
				if ( mouseEvent->button() == Qt::LeftButton ) {
					QVariant data = index.model()->data(index, Qt::CheckStateRole);
					const QRect &rect = checkBoxStyle(option).rect;
					if ( data.isValid() && rect.contains(mouseEvent->pos() ) ) {
						Qt::CheckState state = data.toInt() == Qt::Checked
						                           ? Qt::Unchecked : Qt::Checked;
						return model->setData(index, state, Qt::CheckStateRole);
					}
				}
			}

			return QItemDelegate::editorEvent(event, model, option, index);
		}
};


class IconItemDelegate : public QStyledItemDelegate {
	public:
		using QStyledItemDelegate::QStyledItemDelegate;

	protected:
		void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
			QStyledItemDelegate::initStyleOption(option, index);
			option->features &= ~QStyleOptionViewItem::HasDecoration;
		}

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
			QStyledItemDelegate::paint(painter, option, index);
			index.data(Qt::DecorationRole).value<QIcon>().paint(
				painter,
				option.rect.left(), option.rect.top() + (option.rect.height() - option.decorationSize.height()) / 2,
				option.rect.width(), option.decorationSize.height()
			);
		}
};



class LogManagerSettings : public QDialog {
	public:
		LogManagerSettings(QWidget *parent);

		void loadUi();
		void saveUi();

		/**
		 * @brief restoreSettings Restores last session settings
		 */
		void restoreSettings();

		/**
		 * @brief saveSettings Save current session settings
		 * e.g. Log level selection or sort order
		 */
		void saveSettings() const;

		[[nodiscard]]
		int bufferSize() const {
			return _bufferSize;
		}

		[[nodiscard]]
		QList<int> subscriptions() const {
			return _subscriptions;
		}

		[[nodiscard]]
		QList<int> notifications() const {
			return _notifications;
		}

		[[nodiscard]]
		bool onlyIncreasingWarnLevel() const {
			return _onlyIncreasingWarnLevel;
		}

		[[nodiscard]]
		bool logComponent() const {
			return _logComponent;
		}

		[[nodiscard]]
		bool logContext() const {
			return _logContext;
		}

	private:
		void confirm();

		Ui::LogManagerSettings _ui;
		int                    _bufferSize{1000};
		QList<int>             _subscriptions;
		QList<int>             _notifications;
		bool                   _onlyIncreasingWarnLevel{true};
		bool                   _logComponent{false};
		bool                   _logContext{false};
};


class LogLevelSortFilterModel : public QSortFilterProxyModel {
	public:
		LogLevelSortFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {
			setDynamicSortFilter(true);
			setSortRole(Qt::UserRole);
		}

		using LogLevels = QList<int>;

		/**
		 * @brief setLogLevels Set log levels as filter
		 * @param levels The log levels
		 */
		void setLogLevels(const LogLevels &levels) {
			_levels = levels;
			invalidateFilter();
		}

	protected:
		[[nodiscard]]
		bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
			int level = sourceModel()->index(
				sourceRow, Level, sourceParent
			).data(Qt::UserRole).toInt();

			if ( !_levels.isEmpty() && !_levels.contains(level) ) {
				return false;
			}

			return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
		}

	private:
		LogLevels _levels;
};


} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent)
: QHeaderView(orientation, parent) {
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &HeaderView::customContextMenuRequested,
	        this, &HeaderView::showContextMenu);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HeaderView::showContextMenu(const QPoint &pos) {
	auto *view = qobject_cast<QHeaderView*>(sender());
	QAbstractItemModel *model = view->model();
	if ( !model ) {
		return;
	}

	int sections = view->count();
	int hiddenSections = view->hiddenSectionCount();
	bool last = (sections - 1 == hiddenSections);

	QMenu menu;
	for ( int section = 0; section < sections; ++section ) {
		QString text = model->headerData(section, Qt::Horizontal).toString();
		QAction *action = menu.addAction(text);
		if ( !action ) {
			continue;
		}

		bool enabled = !view->isSectionHidden(section);
		action->setCheckable(true);
		action->setChecked(enabled);
		action->setDisabled(last && enabled);
		action->setData(section);
	}

	QAction* action = menu.exec(QCursor::pos());
	if ( !action ) {
		return;
	}

	int col = action->data().toInt();
	view->setSectionHidden(col, !action->isChecked());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogManagerSettings::LogManagerSettings(QWidget *parent)
    : QDialog(parent) {
	_ui.setupUi(this);

	auto *gridLayout = new QGridLayout;
	gridLayout->setContentsMargins({});
	gridLayout->setHorizontalSpacing(QFontMetrics(font()).averageCharWidth() * 4);
	_ui.frameChannels->setLayout(gridLayout);

	connect(_ui.okButton, &QPushButton::clicked, this, &LogManagerSettings::confirm);

	bool subscriptions[Logging::LL_QUANTITY] = {};
	bool notifications[Logging::LL_QUANTITY] = {false, true,  true, true,
	                                            false, false, false};

	subscriptions[Logging::LL_CRITICAL] = false;
	subscriptions[Logging::LL_NOTICE] = true;
	switch ( SCCoreApp->logVerbosity() ) {
		case 4:
			subscriptions[Logging::LL_DEBUG] = true;
		case 3:
			subscriptions[Logging::LL_INFO] = true;
		case 2:
			subscriptions[Logging::LL_WARNING] = true;
		case 1:
			subscriptions[Logging::LL_ERROR] = true;
		default:
			break;
	}

	auto *label = new QLabel(tr("Channel"));
	setBold(label, true);
	gridLayout->addWidget(label, 0, int(ChannelColumns::Name));

	label = new QLabel(tr("Notify"));
	setBold(label, true);
	gridLayout->addWidget(label, 0, int(ChannelColumns::Notify));

	label = new QLabel(tr("Subscribe"));
	setBold(label, true);
	gridLayout->addWidget(label, 0, int(ChannelColumns::Subscribe));

	for ( int level = Logging::LL_CRITICAL, row = 1;
	      level < Logging::LL_QUANTITY; ++level, ++row ) {
		auto *channel = new QWidget;
		{
			auto *hLayout = new QHBoxLayout;
			hLayout->setContentsMargins({});
			auto* label = new QLabel;
			label->setPixmap(logIcon(level).pixmap(QFontMetrics(font()).ascent()));
			label->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
			hLayout->addWidget(label);
			label = new QLabel(LogLevels[level].title);
			hLayout->addWidget(label);
			channel->setLayout(hLayout);
		}
		gridLayout->addWidget(channel, row, int(ChannelColumns::Name));

		auto *checkSubscription = new QCheckBox;
		if ( subscriptions[level] ) {
			checkSubscription->setChecked(true);
			_subscriptions.append(level);
		}
		else {
			checkSubscription->setChecked(false);
		}
		gridLayout->addWidget(checkSubscription, row, int(ChannelColumns::Subscribe));

		auto *checkNotify = new QCheckBox;
		if ( notifications[level] ) {
			checkNotify->setChecked(true);
			_notifications.append(level);
		}
		else {
			checkNotify->setChecked(false);
		}
		gridLayout->addWidget(checkNotify, row, int(ChannelColumns::Notify));
	}

	restoreSettings();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManagerSettings::loadUi() {
	_ui.spinBufferSize->setValue(_bufferSize);

	auto *grid = static_cast<QGridLayout*>(_ui.frameChannels->layout());
	for ( int level = Logging::LL_CRITICAL, row = 1;
	      level < Logging::LL_QUANTITY; ++level, ++row ) {
		auto *item = grid->itemAtPosition(row, int(ChannelColumns::Subscribe));
		static_cast<QCheckBox*>(item->widget())->setChecked(_subscriptions.indexOf(level) >= 0);
		item = grid->itemAtPosition(row, int(ChannelColumns::Notify));
		static_cast<QCheckBox*>(item->widget())->setChecked(_notifications.indexOf(level) >= 0);
	}

	_ui.sendOnly->setChecked(_onlyIncreasingWarnLevel);
	_ui.logComponent->setChecked(_logComponent);
	_ui.logContext->setChecked(_logContext);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManagerSettings::saveUi() {
	_bufferSize = _ui.spinBufferSize->value();

	_subscriptions.clear();
	_notifications.clear();

	auto *grid = static_cast<QGridLayout*>(_ui.frameChannels->layout());
	for ( int level = Logging::LL_CRITICAL, row = 1;
	      level < Logging::LL_QUANTITY; ++level, ++row ) {
		auto *item = grid->itemAtPosition(row, int(ChannelColumns::Subscribe));
		if ( static_cast<QCheckBox*>(item->widget())->isChecked() ) {
			_subscriptions.append(level);
		}

		item = grid->itemAtPosition(row, int(ChannelColumns::Notify));
		if ( static_cast<QCheckBox*>(item->widget())->isChecked() ) {
			_notifications.append(level);
		}
	}

	_onlyIncreasingWarnLevel = _ui.sendOnly->isChecked();
	_logComponent = _ui.logComponent->isChecked();
	_logContext = _ui.logContext->isChecked();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManagerSettings::restoreSettings() {
	QSettings &settings = SCApp->settings();
	settings.beginGroup(LogManagerGroup);

	// buffer
	settings.beginGroup("buffer");

	QVariant value = settings.value("lines");
	if ( value.isValid() ) {
		bool ok = false;
		QString text = value.toString();
		int size = text.toInt(&ok);
		if ( ok && size > 0 ) {
			_bufferSize = size;
		}
	}

	settings.endGroup(); // buffer

	value = settings.value("increasingWarnLevel");
	if ( value.isValid() ) {
		_onlyIncreasingWarnLevel = value.toBool();
	}

	_logComponent = SCCoreApp->logComponent();
	_logContext = SCCoreApp->logContext();

	settings.endGroup(); // LogManager
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManagerSettings::saveSettings() const {
	QSettings &settings = SCApp->settings();
	settings.beginGroup(LogManagerGroup);

	// buffer
	settings.beginGroup("buffer");
	settings.setValue("lines", _bufferSize);
	settings.endGroup();

	settings.setValue("increasingWarnLevel", _onlyIncreasingWarnLevel);

	settings.endGroup(); // LogManager
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManagerSettings::confirm() {
	saveUi();
	accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct LogManager::LogEntry {
	time_t   time;
	uint32_t microseconds;
	int      level;
	QString  msg;
	QString  component;
	QString  fileName;
	int      lineNum;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogManager::LogManager(QWidget *parent)
: QMainWindow(parent)
, _d_ptr(new LogManagerPrivate) {
	qRegisterMetaType<LogEntry>("LogEntry");

	SC_D.ui.setupUi(this);

	// Override default copy behavior
	addAction(SC_D.ui.actionCopy);
	connect(SC_D.ui.actionCopy, &QAction::triggered, this, &LogManager::copyLog);

	SC_D.ui.clear->setIcon(icon("clear"));

	setWindowFlags(Qt::Window);

	connect(SC_D.ui.clear, &QAction::triggered, this, &LogManager::clearView);

	// Create filter input field and use the place holder frame as container
	connect(SC_D.ui.editFilter, &QLineEdit::textChanged, this, &LogManager::filterChanged);

	connect(SC_D.ui.actionFilter, &QAction::triggered, this, [this](bool) {
		SC_D.ui.editFilter->setFocus();
	});
	addAction(SC_D.ui.actionFilter);
	SC_D.ui.labelFilter->setPixmap(icon("filter").pixmap(24));

	SC_D.model = new QStandardItemModel(this);

	SC_D.proxyModel = new LogLevelSortFilterModel(this);
	SC_D.proxyModel->setSourceModel(SC_D.model);

	auto *header = new HeaderView(Qt::Horizontal);
	header->setModel(SC_D.proxyModel);
	header->setSortIndicatorShown(true);
	SC_D.ui.tableView->setHorizontalHeader(header);

	SC_D.ui.tableView->setShowGrid(false);
	SC_D.ui.tableView->setItemDelegateForColumn(Level, new IconItemDelegate(SC_D.ui.tableView));
	SC_D.ui.tableView->setModel(SC_D.proxyModel);
	SC_D.ui.tableView->setSortingEnabled(true);
	SC_D.ui.tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	SC_D.ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	SC_D.ui.tableView->verticalHeader()->hide();
	SC_D.ui.tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	SC_D.ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(SC_D.ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
	        this, &LogManager::selectionChanged);
	connect(SC_D.ui.tableView, &QTableView::customContextMenuRequested,
	        this, &LogManager::onContextMenuRequested);

	header->setStretchLastSection(true);
	header->setSectionsMovable(true);
	header->setSectionsClickable(true);

	SC_D.ui.comboFilterSource->clear();
	SC_D.ui.comboFilterSource->addItem(tr("Filter all"));
	SC_D.ui.comboFilterSource->addItem(tr("Message"));
	SC_D.ui.comboFilterSource->addItem(tr("Time"));
	SC_D.ui.comboFilterSource->addItem(tr("Component"));
	SC_D.ui.comboFilterSource->addItem(tr("Context"));
	SC_D.ui.comboFilterSource->setCurrentIndex(0);
	SC_D.proxyModel->setFilterKeyColumn(-1);
	connect(SC_D.ui.comboFilterSource, qOverload<int>(&QComboBox::currentIndexChanged),
	        this, [this](int index) {
		switch ( index ) {
			default:
			case 0:
				SC_D.proxyModel->setFilterKeyColumn(-1);
				break;
			case 1:
				SC_D.proxyModel->setFilterKeyColumn(Message);
				break;
			case 2:
				SC_D.proxyModel->setFilterKeyColumn(Time);
				break;
			case 3:
				SC_D.proxyModel->setFilterKeyColumn(Component);
				break;
			case 4:
				SC_D.proxyModel->setFilterKeyColumn(Context);
				break;
		}
	});

	SC_D.filterActions = new QActionGroup(this);
	SC_D.filterActions->setExclusive(false);
	connect(SC_D.filterActions, &QActionGroup::triggered,
	        this, &LogManager::logLevelSelectionChanged);

	SC_D.filterActions->addAction(SC_D.ui.critical);
	SC_D.filterActions->addAction(SC_D.ui.error);
	SC_D.filterActions->addAction(SC_D.ui.warning);
	SC_D.filterActions->addAction(SC_D.ui.notice);
	SC_D.filterActions->addAction(SC_D.ui.info);
	SC_D.filterActions->addAction(SC_D.ui.debug);

	SC_D.ui.tableView->viewport()->setMouseTracking(true);

	logLevelSelectionChanged();

	setWindowTitle("Client application log");
	setObjectName("LogManager");

	SC_D.ui.settings->setIcon(icon("settings"));

	SC_D.ui.toolBar->addAction(SC_D.ui.clear);
	SC_D.ui.toolBar->addSeparator();
	SC_D.ui.toolBar->addAction(SC_D.ui.settings);
	SC_D.ui.toolBar->addSeparator();
	SC_D.ui.toolBar->addAction(SC_D.ui.critical);
	SC_D.ui.toolBar->addAction(SC_D.ui.error);
	SC_D.ui.toolBar->addAction(SC_D.ui.warning);
	SC_D.ui.toolBar->addAction(SC_D.ui.notice);
	SC_D.ui.toolBar->addAction(SC_D.ui.info);
	SC_D.ui.toolBar->addAction(SC_D.ui.debug);

	SC_D.model->setColumnCount(Columns::Quantity);

	SC_D.ui.tableView->horizontalHeader()->setSortIndicator(Time, Qt::DescendingOrder);

	for ( int i = 0; i < Columns::Quantity; ++i ) {
		SC_D.model->setHeaderData(
		    i, Qt::Horizontal, EColumnsNames::name(i), Qt::DisplayRole
		);

		SC_D.model->setHeaderData(
		    i, Qt::Horizontal, i == Level? "Log level": EColumnsNames::name(i), Qt::ToolTipRole
		);
	}

	setupLogAction(SC_D.ui.critical, Logging::LL_CRITICAL);
	setupLogAction(SC_D.ui.error, Logging::LL_ERROR);
	setupLogAction(SC_D.ui.warning, Logging::LL_WARNING);
	setupLogAction(SC_D.ui.notice, Logging::LL_NOTICE);
	setupLogAction(SC_D.ui.info, Logging::LL_INFO);
	setupLogAction(SC_D.ui.debug, Logging::LL_DEBUG);

	SC_D.settings = new LogManagerSettings(this);
	connect(SC_D.ui.settings, &QAction::triggered, this, &LogManager::showSettings);

	// Possibly connected via threads
	connect(this, &LogManager::newLog, this, &LogManager::addLog);

	logComponent(static_cast<LogManagerSettings*>(SC_D.settings)->logComponent());
	logContext(static_cast<LogManagerSettings*>(SC_D.settings)->logContext());

	installEventFilter(this);

	restoreSettings();
	subscribe();

	logLevelSelectionChanged();
	setLevel();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogManager::~LogManager() {
	saveSettings();

	delete _d_ptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LogManager::setup(const Seiscomp::Util::Url &url) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::activate() {
	if ( isHidden() ) {
		show();
	}
	if ( isMinimized() ) {
		showNormal();
	}
	if ( !isActiveWindow() ) {
		activateWindow();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::clearView() {
	SC_D.model->removeRows(0, SC_D.model->rowCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::copyLog() {
	auto *model = SC_D.ui.tableView->selectionModel();
	QModelIndexList indexes = model->selectedRows();
	QApplication::clipboard()->setText(rowsToText(indexes));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::onContextMenuRequested(const QPoint &pos) {
	QMenu menu;
	menu.addAction(SC_D.ui.actionCopyCell);
	menu.addAction(SC_D.ui.actionCopy);

	QModelIndex index = SC_D.ui.tableView->indexAt(pos);
	if ( index.isValid() ) {
		SC_D.ui.actionCopyCell->setData(index.data(Qt::DisplayRole).toString());
		SC_D.ui.actionCopyCell->setEnabled(true);
	}
	else {
		SC_D.ui.actionCopyCell->setEnabled(false);
	}

	auto *action = menu.exec(SC_D.ui.tableView->mapToGlobal(pos));
	if ( action == SC_D.ui.actionCopyCell ) {
		QApplication::clipboard()->setText(SC_D.ui.actionCopyCell->data().toString());
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::filterChanged(const QString &filter) {
	SC_D.proxyModel->setFilterFixedString(filter);
	SC_D.proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::logLevelSelectionChanged() {
	LogLevelSortFilterModel::LogLevels levels;
	foreach ( QAction *action, SC_D.filterActions->actions() ) {
		if ( action->isChecked() ) {
			levels.append(action->data().toInt());
		}
	}

	static_cast<LogLevelSortFilterModel*>(SC_D.proxyModel)->setLogLevels(levels);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::selectionChanged() {
	QItemSelectionModel *model = SC_D.ui.tableView->selectionModel();
	if ( !model ) {
		SC_D.ui.actionCopy->setEnabled(false);
		return;
	}

	SC_D.ui.actionCopy->setEnabled(model->hasSelection());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::restoreSettings() {
	const QString &objName = objectName();
	const QString &groupName = objName.isEmpty() ? LogManagerGroup : objName;

	QSettings &settings = SCApp->settings();
	settings.beginGroup(groupName);
	QVariant value = settings.value("geometry");

	// geometry
	if ( value.isValid() ) {
		restoreGeometry(value.toByteArray());
	}
	else {
		auto *screen = QGuiApplication::primaryScreen();
		if ( screen ) {
			setGeometry(
				QStyle::alignedRect(
				    Qt::LeftToRight,
				    Qt::AlignCenter,
				    size(),
				    screen->availableGeometry()
				)
			);
		}
	}

	value = settings.value("state");
	if ( value.isValid() ) {
		restoreState(value.toByteArray());
	}

	// table header
	QVariant data = settings.value("header");
	if ( data.isValid() ) {
		SC_D.ui.tableView->horizontalHeader()->restoreState(data.toByteArray());
	}
	else {
		SC_D.ui.tableView->setColumnHidden(Component, !static_cast<LogManagerSettings*>(SC_D.settings)->logComponent());
		SC_D.ui.tableView->setColumnHidden(Context, !static_cast<LogManagerSettings*>(SC_D.settings)->logContext());
	}

	settings.endGroup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::saveSettings() {
	const QString &objName = objectName();
	const QString &groupName = objName.isEmpty() ? LogManagerGroup : objName;

	QSettings &settings = SCApp->settings();
	settings.beginGroup(groupName);

	// geometry
	settings.setValue("geometry", saveGeometry());

	// state
	settings.setValue("state", saveState());

	// table header
	settings.setValue("header",  SC_D.ui.tableView->horizontalHeader()->saveState());

	settings.endGroup();

	static_cast<LogManagerSettings*>(SC_D.settings)->saveSettings();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::setLevel(int level) {
	_level = level;
	emit levelChanged(level);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LogManager::eventFilter(QObject *obj, QEvent *ev) {
	if ( ev->type() == QEvent::WindowActivate ) {
		setLevel();
	}

	return QWidget::eventFilter(obj, ev);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::log(const char *channelName, Seiscomp::Logging::LogLevel level,
                     const char *msg, time_t time, uint32_t microseconds) {
	LogEntry entry;
	entry.time = time;
	entry.microseconds = microseconds;
	entry.level = static_cast<int>(level);
	entry.msg = msg;

	if ( static_cast<LogManagerSettings*>(SC_D.settings)->logComponent() ) {
		entry.component = component();
	}

	if ( static_cast<LogManagerSettings*>(SC_D.settings)->logContext() ) {
		const auto *fn = fileName();
		if ( !strncmp(fn, SEISCOMP_SOURCE_DIR, length(SEISCOMP_SOURCE_DIR)) ) {
			fn += length(SEISCOMP_SOURCE_DIR);
		}
		if ( *fn && (*fn == '/') ) {
			++fn;
		}
		entry.fileName = fn;
		entry.lineNum = lineNum();
	}

	emit newLog(entry);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QWidget* LogManager::dataView() {
	return SC_D.ui.tableView;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::addLog(const LogEntry &entry) {
	// Ignore the undefined level
	if ( entry.level <= 0 ) {
		return;
	}

	// Remove rows exceeding row limit
	if ( static_cast<LogManagerSettings*>(SC_D.settings)->bufferSize() > 0 ) {
		while ( SC_D.model->rowCount() > static_cast<LogManagerSettings*>(SC_D.settings)->bufferSize() ) {
			SC_D.model->removeRows(0, 1);
		}
	}

	QList<QStandardItem*> items;
	for ( int col = 0; col < Columns::Quantity; ++col ) {
		auto *item = new QStandardItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if ( col == Level ) {
			item->setTextAlignment(Qt::AlignHCenter);
		}
		items.push_back(item);
	}

	// Time
	auto *item = items[Time];
	Core::Time t = Core::Time(entry.time, entry.microseconds);
	qint64 ms = entry.time * 1000 + entry.microseconds / 1000;

	item->setData(QString::fromStdString(t.toString("%F %T")), Qt::DisplayRole);
	item->setData(QString::fromStdString(t.toString("%F %T.%f")), Qt::ToolTipRole);
	item->setData(ms, Qt::UserRole);

	// Level
	item = items[Level];
	item->setIcon(logIcon(entry.level));
	item->setData(entry.level, Qt::UserRole);

	// Component
	if ( static_cast<LogManagerSettings*>(SC_D.settings)->logComponent() ) {
		QString c = component();
		item = items[Component];
		item->setData(c, Qt::DisplayRole);
		item->setData(c, Qt::ToolTipRole);
		item->setData(c, Qt::UserRole);
	}

	// Context
	if ( static_cast<LogManagerSettings*>(SC_D.settings)->logContext() ) {
		item = items[Context];
		QString data = QString("%1:%2").arg(entry.fileName).arg(entry.lineNum);
		item->setData(data, Qt::DisplayRole);
		item->setData(data, Qt::ToolTipRole);
		item->setData(data, Qt::UserRole);
	}

	// Message
	item = items[Message];
	item->setData(entry.msg, Qt::DisplayRole);
	item->setData(entry.msg, Qt::ToolTipRole);
	item->setData(entry.msg, Qt::UserRole);
	if ( entry.level < 4 ) {
		item->setForeground(LogLevels[entry.level].color);
	}

	SC_D.model->appendRow(items);

	if ( !hasFocus() && static_cast<LogManagerSettings*>(SC_D.settings)->notifications().contains(entry.level) &&
	     (!static_cast<LogManagerSettings*>(SC_D.settings)->onlyIncreasingWarnLevel() || _level > entry.level) ) {
		setLevel(entry.level);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::subscribe() {
	Logging::Output::clear();
	foreach (int i, static_cast<LogManagerSettings*>(SC_D.settings)->subscriptions()) {
		Logging::Output::subscribe(
			Seiscomp::Logging::getGlobalChannel(LogLevels[i].name)
		);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogManager::showSettings() {
	static_cast<LogManagerSettings*>(SC_D.settings)->loadUi();

	if ( static_cast<LogManagerSettings*>(SC_D.settings)->exec() != QDialog::Accepted ) {
		return;
	}

	logComponent(static_cast<LogManagerSettings*>(SC_D.settings)->logComponent());
	logContext(static_cast<LogManagerSettings*>(SC_D.settings)->logContext());

	// Remove rows exeeding new buffer limit
	if ( static_cast<LogManagerSettings*>(SC_D.settings)->bufferSize() > 0 ) {
		while ( SC_D.model->rowCount() > static_cast<LogManagerSettings*>(SC_D.settings)->bufferSize() ) {
			SC_D.model->removeRows(0, 1);
		}
	}

	subscribe();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogStateLabel::LogStateLabel(LogManager *manager, QWidget *parent)
: QLabel(parent)
, _manager(manager) {
	Q_ASSERT_X(manager, "LogStateLabel()", "Nullpointer in LogManager argument");
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogStateLabel::mousePressEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		_manager->activate();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogStateLabel::logStateChanged() {
	int level = _manager->level();
	if ( level < 0 || static_cast<size_t>(level) >= LogLevels.size() ) {
		setToolTip(tr("Show log manager"));
		setPixmap(Gui::pixmap(this, "log"));
	}
	else {
		const auto &logLevel = LogLevels[level];
		setToolTip(tr("New log entries up to level '%1' found").arg(logLevel.title));
		setPixmap(Gui::pixmap(this, logLevel.iconName, logLevel.color));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LogStateLabel::init() {
	connect(_manager, &LogManager::levelChanged, this, &LogStateLabel::logStateChanged);
	logStateChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Seiscomp::Gui
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
