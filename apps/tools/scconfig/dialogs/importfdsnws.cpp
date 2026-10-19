/***************************************************************************
 * Copyright (C) 2017 by gempa GmbH                                        *
 *                                                                         *
 * All Rights Reserved.                                                    *
 *                                                                         *
 * NOTICE: All information contained herein is, and remains                *
 * the property of gempa GmbH and its suppliers, if any. The intellectual  *
 * and technical concepts contained herein are proprietary to gempa GmbH   *
 * and its suppliers.                                                      *
 * Dissemination of this information or reproduction of this material      *
 * is strictly forbidden unless prior written permission is obtained       *
 * from gempa GmbH.                                                        *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
 ***************************************************************************/


#define SEISCOMP_COMPONENT scconfig

#include <seiscomp/logging/log.h>
#include <seiscomp/datamodel/inventory.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/importer.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/system/pluginregistry.h>

#include <algorithm>

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QDeadlineTimer>
#include <QHash>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QScrollBar>
#include <QSet>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QTextEdit>
#include <QTimer>
#include <QXmlStreamReader>

#include <functional>

#include "importfdsnws.h"
#include "../http.h"
#include "../icon.h"


using namespace std;
using namespace Seiscomp;


namespace {


// Registry of FDSN data centres. All services whose URL contains
// STATION_SERVICE are offered in addition to the presets.
const char *DATACENTER_URL = "https://www.fdsn.org/ws/datacenters/1/query";
const QString STATION_SERVICE = "/fdsnws/station/1";
const QString SERVICE_BASE = "/fdsnws";
const QString NAME_SEPARATOR = QStringLiteral(" \u2013 ");

// Number of stations a data centre offers for the current constraints,
// set by the server test only.
const int CountRole = Qt::UserRole + 1;

// Appended to the tool tip, and the background of the entries offering the
// most stations.
const QString STATIONS_FOUND = QStringLiteral(" - stations found: ");
const QColor HIGHLIGHT_COLOR = QColor("#aaff7f");

// Background of a date and time which is incomplete while being edited.
const QColor INVALID_COLOR = QColor("#ffa000");
const int DATACENTER_TIMEOUT = 30000;


class MemStreamBuf : public basic_streambuf<char> {
	public:
		MemStreamBuf(char* p, size_t n) {
			setg(p, p, p + n);
		}
};


// Resets a QDateTimeEdit to its minimum, which renders as the special value
// text, on Del or Backspace. If the value is unset, the calendar popup would
// start at the minimum date, so the current time is set before it opens.
class DateTimeEditFilter : public QObject {
	public:
		using QObject::QObject;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override {
			auto *edit = static_cast<QDateTimeEdit*>(obj);

			switch ( event->type() ) {
				case QEvent::KeyPress:
				{
					// Deleting the complete text unsets the date and time,
					// deleting a part of it edits the digits.
					int key = static_cast<QKeyEvent*>(event)->key();
					auto *line = edit->findChild<QLineEdit*>();

					if ( (key == Qt::Key_Delete || key == Qt::Key_Backspace)
					  && line && line->selectedText() == line->text() ) {
						edit->setDateTime(edit->minimumDateTime());
						return true;
					}
					break;
				}

				case QEvent::FocusOut:
					// The spin box would replace an incomplete input by the
					// previous value. It is kept instead and stays marked,
					// only the text field is told that it lost the focus.
					if ( !edit->hasAcceptableInput() ) {
						if ( auto *line = edit->findChild<QLineEdit*>() ) {
							QCoreApplication::sendEvent(line, event);
						}

						edit->update();
						return true;
					}
					break;

				case QEvent::MouseButtonPress:
					if ( edit->dateTime() == edit->minimumDateTime()
					  && hitsCalendarArrow(edit, static_cast<QMouseEvent*>(event)) ) {
						auto now = QDateTime::currentDateTimeUtc();
						now.setTime(QTime(now.time().hour(), now.time().minute()));
						edit->setDateTime(now);
					}
					break;

				default:
					break;
			}

			return QObject::eventFilter(obj, event);
		}

	private:
		static bool hitsCalendarArrow(QDateTimeEdit *edit, QMouseEvent *event) {
			QStyleOptionComboBox opt;
			opt.initFrom(edit);
			return edit->style()->subControlRect(QStyle::CC_ComboBox, &opt,
			                                     QStyle::SC_ComboBoxArrow, edit)
			           .contains(event->pos());
		}
};


// Appends further parameters to a query URL.
std::string withParameters(const QString &url, const char *parameters) {
	return (url + (url.contains('?') ? "&" : "?") + parameters).toStdString();
}


// A test which was cancelled or failed shows a dash instead of a number.
QString stationCount(int count) {
	return count < 0 ? QStringLiteral("-") : QString::number(count);
}


// Epochs of one station share network and station code and count as one.
int countStations(const QString &response) {
	QSet<QString> stations;

	const auto rows = response.split('\n');
	for ( const auto &row : rows ) {
		auto line = row.trimmed();
		if ( line.isEmpty() || line.startsWith('#') ) {
			continue;
		}

		auto columns = line.split('|');
		if ( columns.count() > 1 ) {
			stations.insert(columns[0] + '.' + columns[1]);
		}
	}

	return stations.count();
}


// Shows "<name> - <url without the service base>" and stores the same
// shortened URL, which is also offered as tool tip.
void setDataCenterItem(QComboBox *combo, int index, const QString &name, const QString &url) {
	auto text = url;
	if ( text.endsWith(SERVICE_BASE) ) {
		text.chop(SERVICE_BASE.length());
	}

	combo->setItemText(index, name.isEmpty() ? text : name + NAME_SEPARATOR + text);
	combo->setItemData(index, text);
	combo->setItemData(index, text, Qt::ToolTipRole);
}


// The text format of the web service carries ISO time stamps of which only
// the date is shown.
QString dateOf(const QString &time) {
	auto date = QDateTime::fromString(time, Qt::ISODate).date();
	return date.isValid() ? date.toString("yyyy-MM-dd") : time;
}


// Renders the data centre entries as a bold name followed by the URL which is
// elided at the end. Items without URL, e.g. the presets and status messages,
// are left to the base implementation. Separators are drawn like Qt's internal
// combo box delegate does, which is replaced by this one.
class DataCenterDelegate : public QStyledItemDelegate {
	public:
		explicit DataCenterDelegate(QComboBox *combo)
		: QStyledItemDelegate(combo), _combo(combo) {}

	protected:
		void paint(QPainter *painter, const QStyleOptionViewItem &option,
		           const QModelIndex &index) const override {
			if ( isSeparator(index) ) {
				QStyleOption opt;
				opt.rect = option.rect;
				if ( auto *view = qobject_cast<const QAbstractItemView*>(option.widget) ) {
					opt.rect.setWidth(view->viewport()->width());
				}

				_combo->style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator,
				                               &opt, painter, _combo);
				return;
			}

			QStyleOptionViewItem opt(option);
			initStyleOption(&opt, index);

			int pos = opt.text.indexOf(NAME_SEPARATOR);
			if ( pos < 0 ) {
				QStyledItemDelegate::paint(painter, option, index);
				return;
			}

			auto name = opt.text.left(pos);
			auto rest = opt.text.mid(pos);
			auto font = opt.font;
			font.setBold(true);

			auto *style = opt.widget ? opt.widget->style() : QApplication::style();
			opt.text.clear();
			style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

			auto rect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
			auto group = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

			painter->save();
			painter->setPen(opt.palette.color(group, opt.state & QStyle::State_Selected ?
			                                         QPalette::HighlightedText : QPalette::Text));

			painter->setFont(font);
			painter->drawText(rect, Qt::AlignVCenter, name);

			rect.setLeft(rect.left() + QFontMetrics(font).horizontalAdvance(name));
			painter->setFont(opt.font);

			// The number of stations keeps its space, the URL is elided.
			auto count = index.data(CountRole);
			if ( count.isValid() ) {
				auto number = stationCount(count.toInt());
				QFontMetrics metrics(opt.font);

				// The item ends at the viewport, so without an inset the
				// number would touch the scroll bar of the popup.
				rect.setWidth(rect.width() - metrics.horizontalAdvance(' '));

				painter->drawText(rect, Qt::AlignVCenter | Qt::AlignRight, number);
				rect.setWidth(qMax(0, rect.width()
				                      - metrics.horizontalAdvance(number + "  ")));
			}

			painter->drawText(rect, Qt::AlignVCenter,
			                  QFontMetrics(opt.font).elidedText(rest, Qt::ElideRight, rect.width()));
			painter->restore();
		}

		QSize sizeHint(const QStyleOptionViewItem &option,
		               const QModelIndex &index) const override {
			if ( isSeparator(index) ) {
				int pm = _combo->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, _combo);
				return QSize(pm, pm);
			}

			auto size = QStyledItemDelegate::sizeHint(option, index);

			auto text = index.data(Qt::DisplayRole).toString();
			int pos = text.indexOf(NAME_SEPARATOR);
			if ( pos < 0 ) {
				return size;
			}

			// The name is rendered bold and thus needs more space.
			auto name = text.left(pos);
			auto font = option.font;
			font.setBold(true);

			size.setWidth(size.width() + QFontMetrics(font).horizontalAdvance(name)
			                           - QFontMetrics(option.font).horizontalAdvance(name));

			auto count = index.data(CountRole);
			if ( count.isValid() ) {
				size.setWidth(size.width() + QFontMetrics(option.font).horizontalAdvance(
					"  " + stationCount(count.toInt()) + " "));
			}

			return size;
		}

	private:
		static bool isSeparator(const QModelIndex &index) {
			return index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator");
		}

		QComboBox *_combo;
};


// Draws the check box of the selection column centered, the default
// implementation puts it at the left edge of the cell. A click anywhere in
// the cell toggles the state.
class CheckDelegate : public QStyledItemDelegate {
	public:
		using QStyledItemDelegate::QStyledItemDelegate;

		// Centered within the item, used by the header as well.
		static QRect indicatorRect(const QStyleOption &option, const QWidget *widget) {
			auto *style = widget ? widget->style() : QApplication::style();
			QSize size(style->pixelMetric(QStyle::PM_IndicatorWidth, &option, widget),
			           style->pixelMetric(QStyle::PM_IndicatorHeight, &option, widget));

			return QStyle::alignedRect(option.direction, Qt::AlignCenter, size, option.rect);
		}

	protected:
		void paint(QPainter *painter, const QStyleOptionViewItem &option,
		           const QModelIndex &index) const override {
			QStyleOptionViewItem opt(option);
			initStyleOption(&opt, index);
			opt.features &= ~QStyleOptionViewItem::HasCheckIndicator;

			auto *style = opt.widget ? opt.widget->style() : QApplication::style();
			style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

			opt.rect = indicatorRect(opt, opt.widget);
			opt.state &= ~QStyle::State_HasFocus;
			opt.state |= index.data(Qt::CheckStateRole).toInt() == Qt::Checked ?
			             QStyle::State_On : QStyle::State_Off;

			style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, painter, opt.widget);
		}

		bool editorEvent(QEvent *event, QAbstractItemModel *model,
		                 const QStyleOptionViewItem &option,
		                 const QModelIndex &index) override {
			if ( event->type() != QEvent::MouseButtonRelease
			  || !(index.flags() & Qt::ItemIsUserCheckable) ) {
				return QStyledItemDelegate::editorEvent(event, model, option, index);
			}

			return model->setData(index,
			                      index.data(Qt::CheckStateRole).toInt() == Qt::Checked ?
			                      Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
		}
};


// Adds a check box to the first header section which switches the selection
// of all rows at once. Its state is derived from the table and can therefore
// not diverge from it.
class SelectHeader : public QHeaderView {
	public:
		explicit SelectHeader(QTableWidget *table, std::function<void ()> changed)
		: QHeaderView(Qt::Horizontal, table), _table(table)
		, _changed(std::move(changed)) {
			setSectionsClickable(true);
			setSortIndicatorShown(true);

			connect(table, &QTableWidget::itemChanged, this, [this]() {
				updateSection(0);
			});
		}

	protected:
		void paintSection(QPainter *painter, const QRect &rect, int index) const override {
			QHeaderView::paintSection(painter, rect, index);
			if ( index ) {
				return;
			}

			QStyleOptionButton opt;
			opt.initFrom(this);
			opt.rect = indicatorRect(rect);

			switch ( state() ) {
				case Qt::Checked:
					opt.state |= QStyle::State_On;
					break;
				case Qt::Unchecked:
					opt.state |= QStyle::State_Off;
					break;
				default:
					opt.state |= QStyle::State_NoChange;
					break;
			}

			style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, painter, this);
		}

		QSize sectionSizeFromContents(int index) const override {
			auto size = QHeaderView::sectionSizeFromContents(index);
			if ( !index ) {
				size.setWidth(indicatorRect(QRect()).width() + 2 * margin());
			}

			return size;
		}

		void mousePressEvent(QMouseEvent *event) override {
			if ( logicalIndexAt(event->pos()) ) {
				QHeaderView::mousePressEvent(event);
				return;
			}

			auto check = state() == Qt::Checked ? Qt::Unchecked : Qt::Checked;

			{
				// One notification for the whole table rather than one per
				// row, which would be quadratic.
				QSignalBlocker blocker(_table);

				for ( int i = 0; i < _table->rowCount(); ++i ) {
					if ( auto *item = _table->item(i, 0) ) {
						item->setCheckState(check);
					}
				}
			}

			updateSection(0);

			if ( _changed ) {
				_changed();
			}
		}

	private:
		int margin() const {
			return style()->pixelMetric(QStyle::PM_HeaderMargin, nullptr, this);
		}

		QRect indicatorRect(const QRect &rect) const {
			QStyleOptionButton opt;
			opt.initFrom(this);
			opt.rect = rect;

			return CheckDelegate::indicatorRect(opt, this);
		}

		Qt::CheckState state() const {
			int rows = _table->rowCount();
			int checked = 0;

			for ( int i = 0; i < rows; ++i ) {
				auto *item = _table->item(i, 0);
				if ( item && item->checkState() == Qt::Checked ) {
					++checked;
				}
			}

			if ( !rows || checked == rows ) {
				return Qt::Checked;
			}

			return checked ? Qt::PartiallyChecked : Qt::Unchecked;
		}

		QTableWidget *_table;
		std::function<void ()> _changed;
};

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void setup(QDateTimeEdit *edit) {
	edit->setDateTime(edit->minimumDateTime());
	edit->setToolTip(QObject::tr("Select all and press Del or Backspace to unset "
	                             "the date and time"));
	edit->installEventFilter(new DateTimeEditFilter(edit));
}

ImportFDSNWSDialog::ImportFDSNWSDialog(QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	auto *sideBar = new QWidget(this);
	_uiSideBar.setupUi(sideBar);

	auto margins = sideBar->contentsMargins();
	margins.setRight(9);
	sideBar->setContentsMargins(margins);

	_ui.sideBar->setWidget(sideBar);
	_ui.sideBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	_ui.sideBar->setFrameShape(QFrame::NoFrame);

	_ui.tableStations->setItemDelegateForColumn(0, new CheckDelegate(_ui.tableStations));
	_ui.tableStations->setHorizontalHeader(new SelectHeader(_ui.tableStations, [this]() {
		updateConstraints();
	}));

	// The codes are read from the left, their headers follow the content.
	for ( int i = 1; i <= 4; ++i ) {
		_ui.tableStations->horizontalHeaderItem(i)->setTextAlignment(
			Qt::AlignLeft | Qt::AlignVCenter
		);
	}
	_ui.tableStations->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
	_ui.tableStations->horizontalHeader()->setStretchLastSection(true);
	_uiSideBar.comboGeoType->setCurrentIndex(0);
	_uiSideBar.frameGeoBoundingBox->setVisible(false);
	_uiSideBar.frameGeoCircle->setVisible(false);

	setup(_uiSideBar.dtStart);
	setup(_uiSideBar.dtEnd);
	setup(_uiSideBar.dtStartBefore);
	setup(_uiSideBar.dtStartAfter);
	setup(_uiSideBar.dtEndBefore);
	setup(_uiSideBar.dtEndAfter);

	// A date and time is incomplete for a moment while its digits are edited.
	// It is marked and blocks fetching until it is complete again.
	for ( auto *edit : sideBar->findChildren<QDateTimeEdit*>() ) {
		if ( auto *line = edit->findChild<QLineEdit*>() ) {
			connect(line, &QLineEdit::textChanged, this, &ImportFDSNWSDialog::checkDates);
		}
	}

	auto now = QDateTime::currentDateTimeUtc();
	now.setTime(QTime());

	_uiSideBar.dtStart->setDateTime(now);

	_presetCount = _uiSideBar.comboServerURL->count();
	_uiSideBar.comboServerURL->setItemDelegate(new DataCenterDelegate(_uiSideBar.comboServerURL));

	_serverTitle = _uiSideBar.groupBox_5->title();

	// The presets read "<name>: <url>" and are reformatted like the entries
	// fetched from the data centre registry.
	for ( int i = 0; i < _presetCount; ++i ) {
		auto text = _uiSideBar.comboServerURL->itemText(i);
		int pos = text.indexOf(": ");
		setDataCenterItem(_uiSideBar.comboServerURL, i,
		                  pos < 0 ? QString() : text.left(pos),
		                  pos < 0 ? text : text.mid(pos + 2));
	}

	connect(_uiSideBar.comboServerURL, qOverload<int>(&QComboBox::activated),
	        this, &ImportFDSNWSDialog::showServerURL);
	connect(_uiSideBar.comboServerURL, qOverload<int>(&QComboBox::currentIndexChanged),
	        this, &ImportFDSNWSDialog::showServerURL);

	showServerURL();

	connect(_ui.btnFetch, &QPushButton::clicked, this, &ImportFDSNWSDialog::fetch);

	// Every widget which contributes to the query updates the URL preview. A
	// change of a constraint also invalidates the numbers of stations.
	auto constraintChanged = [this]() {
		// The listed streams belong to the constraints they were fetched
		// with. Changing the selection does not take this path, it only
		// rewrites the code constraints.
		clearTable();
		clearStationCounts();
	};

	connect(_uiSideBar.comboServerURL, &QComboBox::currentTextChanged, this, [this]() {
		// The streams and the format belong to the server they were fetched
		// from
		clearTable();
		updateURL();
		updateServerTitle();

		// The mark follows whether the field shows one of the best servers.
		highlightServer();
	});
	connect(_uiSideBar.comboGeoType, qOverload<int>(&QComboBox::currentIndexChanged),
	        this, constraintChanged);

	for ( auto *edit : sideBar->findChildren<QDateTimeEdit*>() ) {
		connect(edit, &QDateTimeEdit::dateTimeChanged, this, constraintChanged);
	}

	for ( auto *spin : sideBar->findChildren<QDoubleSpinBox*>() ) {
		connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged),
		        this, constraintChanged);
	}

	for ( auto *edit : {_uiSideBar.editNetworks, _uiSideBar.editStations,
	                    _uiSideBar.editLocations, _uiSideBar.editChannels} ) {
		connect(edit, &QLineEdit::textChanged, this, constraintChanged);
	}

	connect(_uiSideBar.btnTestServers, &QPushButton::clicked,
	        this, &ImportFDSNWSDialog::testServers);

	connect(_uiSideBar.checkResponses, &QCheckBox::toggled,
	        this, &ImportFDSNWSDialog::updateURL);

	connect(_uiSideBar.editNetworks, &QLineEdit::textChanged,
	        this, &ImportFDSNWSDialog::updateFilename);

	updateFilename();

	// The URL is rendered in the size of the constraint input fields.
	auto urlFont = _ui.textURL->font();
	const auto &refFont = _uiSideBar.dtStart->font();
	if ( refFont.pixelSize() > 0 ) {
		urlFont.setPixelSize(refFont.pixelSize());
	}
	else {
		urlFont.setPointSizeF(refFont.pointSizeF());
	}

	_ui.textURL->setFont(urlFont);

	// The field holds a single line and shares its frame with the copy button
	// in front of it, which takes the background of the text.
	int line = _ui.textURL->fontMetrics().lineSpacing()
	         + 2 * static_cast<int>(_ui.textURL->document()->documentMargin());

	_ui.frameURL->setBackgroundRole(QPalette::Base);
	_ui.frameURL->setAutoFillBackground(true);

	// The label keeps the height of a single line field, so it stays level
	// with the text when the scroll bar appears.
	_ui.labelURL->setFixedHeight(line + 2 * _ui.frameURL->frameWidth());

	int iconSize = _ui.textURL->fontMetrics().height();
	_ui.btnCopyURL->setIcon(icon("copy"));
	_ui.btnCopyURL->setIconSize(QSize(iconSize, iconSize));
	_ui.btnCopyURL->setFixedSize(line, line);

	connect(_ui.btnCopyURL, &QToolButton::clicked, this, [this]() {
		QApplication::clipboard()->setText(_ui.textURL->toPlainText());
	});

	// The horizontal scroll bar appears below the text once the URL is too
	// long, and the field grows by its height rather than letting it cover
	// the text.
	auto resizeURL = [this, line]() {
		auto *bar = _ui.textURL->horizontalScrollBar();
		_ui.textURL->setFixedHeight(line + (bar->maximum() > 0 ? bar->sizeHint().height() : 0));
	};

	connect(_ui.textURL->horizontalScrollBar(), &QScrollBar::rangeChanged,
	        this, resizeURL);
	resizeURL();

	connect(_ui.tableStations, &QTableWidget::itemChanged, this,
	        [this](QTableWidgetItem *item) {
		if ( item->column() == 0 ) {
			updateConstraints();
		}
	});

	updateServerTitle();

	// Nothing is fetched yet.
	setSaveEnabled(false);

	connect(_uiSideBar.comboGeoType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
		switch ( index ) {
			case 0:
				_uiSideBar.frameGeoBoundingBox->setVisible(false);
				_uiSideBar.frameGeoCircle->setVisible(false);
				break;
			case 1:
				_uiSideBar.frameGeoBoundingBox->setVisible(true);
				_uiSideBar.frameGeoCircle->setVisible(false);
				break;
			case 2:
				_uiSideBar.frameGeoBoundingBox->setVisible(false);
				_uiSideBar.frameGeoCircle->setVisible(true);
				break;
		}
	});

	queryDataCenters();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::accept() {
	if ( !_fetched ) {
		return;
	}

	// FDSN StationXML is converted, it is read by a plugin
	Seiscomp::IO::ImporterPtr imp;
	if ( _format == "xml" ) {
		imp = Seiscomp::IO::Importer::Create("fdsnxml");
		if ( !imp ) {
			auto registry = Seiscomp::System::PluginRegistry::Instance();
			registry->addPluginName("fdsnxml");
			registry->loadPlugins();
			imp = Seiscomp::IO::Importer::Create("fdsnxml");
			if ( !imp ) {
				QStringList errors;
				for ( const auto &err : registry->errors() ) {
					errors.push_back(err.data());
				}

				QMessageBox::critical(nullptr,
				                      tr("FDSN StationXML not supported"),
				                      tr("The 'fdsnxml' plugin could not be loaded or does not "
				                         "provide the required feature:\n%1").arg(errors.join("\n")));
				return;
			}
		}
	}

	HttpRequest req;
	req.setLabelText(tr("Requesting channels and responses"));

	string query = request(saveParameters().c_str());
	SEISCOMP_DEBUG("FDSNWS: %s", query.c_str());

	int code = req.get(query.c_str());
	if ( code == 204 ) {
		QMessageBox::critical(nullptr, tr("No content"),
		                      tr("No inventory returned: Check your request!"));
		return;
	}
	else if ( code != 200 ) {
		QMessageBox::critical(nullptr, tr("No content"),
		                      tr("The request did not return valid content, "
		                         "status: %1").arg(code));
		return;
	}

	Seiscomp::DataModel::InventoryPtr inventory;
	MemStreamBuf buf(const_cast<char*>(req.response().constData()), req.response().size());

	if ( imp ) {
		Seiscomp::Core::BaseObjectPtr obj = imp->read(&buf);
		inventory = Seiscomp::DataModel::Inventory::Cast(obj);
		if ( !inventory ) {
			QMessageBox::critical(nullptr, tr("Error"), tr("Conversion from StationXML failed"));
			return;
		}
	}
	else {
		// Delivered in the format of the file, read without conversion
		Seiscomp::IO::XMLArchive ar;
		ar.open(&buf);
		ar >> inventory;
		ar.close();

		if ( !inventory ) {
			QMessageBox::critical(nullptr, tr("Error"), tr("Import failed"));
			return;
		}
	}

	auto name = _uiSideBar.editFilename->text();
	if ( name.isEmpty() ) {
		name = defaultFilename();
	}

	auto *env = Seiscomp::Environment::Instance();
	auto output = env->installDir() + "/etc/inventory/" + name.toStdString();

	Seiscomp::IO::XMLArchive ar;
	ar.create(output.data());
	ar.setFormattedOutput(true);
	ar << inventory;
	ar.close();

	QDialog::accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::queryDataCenters() {
	setDataCenters({{tr("Fetching data centres ..."), QString()}});

	auto *nam = new QNetworkAccessManager(this);
	auto *reply = nam->get(QNetworkRequest(QUrl(DATACENTER_URL)));

	// The timers die with the reply, hence they do not fire after completion.
	QTimer::singleShot(DATACENTER_TIMEOUT, reply, [reply]() {
		reply->abort();
	});

	// Until the list is complete only the presets would be tested, so the
	// test button counts the remaining seconds down.
	auto *button = _uiSideBar.btnTestServers;
	auto text = button->text();
	QDeadlineTimer deadline(DATACENTER_TIMEOUT);

	auto showRemaining = [button, text, deadline]() {
		auto seconds = (deadline.remainingTime() + 999) / 1000;
		button->setText(tr("%1 (%2 s)").arg(text).arg(seconds));
	};

	auto *countdown = new QTimer(reply);
	connect(countdown, &QTimer::timeout, this, showRemaining);
	countdown->start(1000);
	showRemaining();

	connect(reply, &QNetworkReply::finished, this, [this, reply, countdown, button, text]() {
		// The reply is only deleted later, the countdown must not tick in
		// between and overwrite the restored text.
		countdown->stop();
		button->setText(text);

		updateDataCenters(reply);
		reply->deleteLater();
	});
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::updateDataCenters(QNetworkReply *reply) {
	if ( reply->error() != QNetworkReply::NoError ) {
		// The request is only cancelled by the timeout above.
		setDataCenters({{
			reply->error() == QNetworkReply::OperationCanceledError ?
			tr("No server added: Timeout exceeded") :
			tr("No server added: %1").arg(reply->errorString()),
			QString()
		}});
		return;
	}

	QStringList urls;
	DataCenters dataCenters;

	auto doc = QJsonDocument::fromJson(reply->readAll());
	for ( auto dataCenter : doc.object().value("datacenters").toArray() ) {
		auto name = dataCenter.toObject().value("name").toString();

		for ( auto repository : dataCenter.toObject().value("repositories").toArray() ) {
			for ( auto service : repository.toObject().value("services").toArray() ) {
				auto url = service.toObject().value("url").toString();
				int pos = url.indexOf(STATION_SERVICE);
				if ( pos < 0 ) {
					continue;
				}

				auto base = url.left(pos);
				if ( urls.contains(base, Qt::CaseInsensitive) ) {
					continue;
				}

				urls.append(base);
				dataCenters.append({name, base});
			}
		}
	}

	std::sort(dataCenters.begin(), dataCenters.end(),
	          [](const DataCenters::value_type &lhs, const DataCenters::value_type &rhs) {
		return lhs.first.compare(rhs.first, Qt::CaseInsensitive) < 0;
	});

	setDataCenters(dataCenters);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Replaces everything below the presets with a separator and the given entries.
// An entry without URL is a status message and is not selectable.
void ImportFDSNWSDialog::setDataCenters(const DataCenters &dataCenters) {
	auto *combo = _uiSideBar.comboServerURL;

	while ( combo->count() > _presetCount ) {
		combo->removeItem(_presetCount);
	}

	if ( dataCenters.isEmpty() ) {
		return;
	}

	combo->insertSeparator(_presetCount);

	auto *model = qobject_cast<QStandardItemModel*>(combo->model());
	for ( const auto &dataCenter : dataCenters ) {
		if ( dataCenter.second.isEmpty() ) {
			combo->addItem(dataCenter.first);
			if ( model ) {
				model->item(combo->count() - 1)->setEnabled(false);
			}
		}
		else {
			combo->addItem(QString());
			setDataCenterItem(combo, combo->count() - 1, dataCenter.first, dataCenter.second);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Index of the list entry the edit field shows, -1 if the URL was typed in.
int ImportFDSNWSDialog::currentDataCenter() const {
	auto *combo = _uiSideBar.comboServerURL;
	auto url = combo->currentText();

	if ( url.isEmpty() ) {
		return -1;
	}

	for ( int i = 0; i < combo->count(); ++i ) {
		if ( combo->itemData(i).toString() == url ) {
			return i;
		}
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The service base is not part of the list entries and not expected to be
// typed in either, it is always appended here.
QString ImportFDSNWSDialog::serverURL() const {
	return _uiSideBar.comboServerURL->currentText() + SERVICE_BASE;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The list shows "<name> - <url>", the edit field shows the URL alone.
void ImportFDSNWSDialog::showServerURL() {
	auto url = _uiSideBar.comboServerURL->currentData();
	if ( url.isValid() ) {
		_uiSideBar.comboServerURL->setEditText(url.toString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Names the selected data centre in the title of the group box. A URL which
// does not belong to any entry of the list is reported as custom.
void ImportFDSNWSDialog::updateServerTitle() {
	auto name = tr("custom");
	int index = currentDataCenter();

	if ( index >= 0 ) {
		auto text = _uiSideBar.comboServerURL->itemText(index);
		int pos = text.indexOf(NAME_SEPARATOR);
		name = pos < 0 ? QString() : text.left(pos);
	}

	_uiSideBar.groupBox_5->setTitle(
		name.isEmpty() ? _serverTitle : _serverTitle + NAME_SEPARATOR + name
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The file the inventory is written to unless another name is given.
QString ImportFDSNWSDialog::defaultFilename() const {
	auto networks = _uiSideBar.editNetworks->text().remove(' ').replace(',', '_');
	return "inventory_" + (networks.isEmpty() ? QString("all") : networks) + ".xml";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The name follows the network constraint as long as it was not edited.
void ImportFDSNWSDialog::updateFilename() {
	auto name = defaultFilename();
	if ( _uiSideBar.editFilename->text() == _defaultFilename ) {
		_uiSideBar.editFilename->setText(name);
	}

	_defaultFilename = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The station query of the current server with all constraints applied, as
// shown in the dialog.
QString ImportFDSNWSDialog::queryURL() const {
	return queryURL(serverURL());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The same query for an arbitrary server, used to test them all.
QString ImportFDSNWSDialog::queryURL(const QString &base) const {
	QString url = base + "/station/1/query?";

	auto add = [&url](const QString &name, const QString &value) {
		url += name + "=" + value + "&";
	};

	auto addTime = [&add](const QString &name, const QDateTimeEdit *edit) {
		if ( edit->dateTime() > edit->minimumDateTime() ) {
			add(name, edit->dateTime().toString("yyyy-MM-ddTHH:mm:00"));
		}
	};

	auto addText = [&add](const QString &name, const QLineEdit *edit) {
		if ( !edit->text().isEmpty() ) {
			add(name, edit->text());
		}
	};

	auto addValue = [&add](const QString &name, const QDoubleSpinBox *spin) {
		add(name, QString::number(spin->value()));
	};

	addTime("starttime", _uiSideBar.dtStart);
	addTime("endtime", _uiSideBar.dtEnd);
	addTime("startbefore", _uiSideBar.dtStartBefore);
	addTime("startafter", _uiSideBar.dtStartAfter);
	addTime("endbefore", _uiSideBar.dtEndBefore);
	addTime("endafter", _uiSideBar.dtEndAfter);

	addText("network", _uiSideBar.editNetworks);
	addText("station", _uiSideBar.editStations);
	addText("location", _uiSideBar.editLocations);
	addText("channel", _uiSideBar.editChannels);

	if ( _uiSideBar.comboGeoType->currentIndex() == 1 ) {
		// Bounding Box
		addValue("minlatitude", _uiSideBar.spinMinLatitude);
		addValue("maxlatitude", _uiSideBar.spinMaxLatitude);
		addValue("minlongitude", _uiSideBar.spinMinLongitude);
		addValue("maxlongitude", _uiSideBar.spinMaxLongitude);
	}
	else if ( _uiSideBar.comboGeoType->currentIndex() == 2 ) {
		// Circle
		addValue("latitude", _uiSideBar.spinLatitude);
		addValue("longitude", _uiSideBar.spinLongitude);
		addValue("minradius", _uiSideBar.spinMinRadius);
		addValue("maxradius", _uiSideBar.spinMaxRadius);
	}

	// Trailing '?' of a query without constraints or '&' of the last one.
	url.chop(1);

	return url;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Appends the parameters to the query of the current constraints.
std::string ImportFDSNWSDialog::request(const char *parameters) const {
	return withParameters(queryURL(), parameters);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Asks the server for the formats it delivers the inventory in, as listed by
// the format parameter of its WADL. SeisComP XML is preferred as it is
// written without conversion, its current name scml over sc3ml. FDSN
// StationXML is supported by every server.
QString ImportFDSNWSDialog::queryFormat() {
	HttpRequest req;
	req.setLabelText(tr("Requesting supported formats"));

	const QString url = serverURL() + "/station/1/application.wadl";
	SEISCOMP_DEBUG("FDSNWS: %s", qPrintable(url));

	if ( req.get(url) != 200 ) {
		SEISCOMP_WARNING("FDSNWS: Failed to read %s, requesting FDSN StationXML",
		                 qPrintable(url));
		return "xml";
	}

	QSet<QString> formats;
	QXmlStreamReader xml(req.response());
	bool inFormat = false;

	while ( !xml.atEnd() ) {
		xml.readNext();

		if ( xml.isStartElement() ) {
			if ( xml.name() == QLatin1String("param") ) {
				inFormat = xml.attributes().value("name") == QLatin1String("format");
			}
			else if ( inFormat && (xml.name() == QLatin1String("option")) ) {
				formats.insert(xml.attributes().value("value").toString());
			}
		}
		else if ( xml.isEndElement() && (xml.name() == QLatin1String("param")) ) {
			inFormat = false;
		}
	}

	if ( formats.contains("scml") ) {
		return "scml";
	}

	if ( formats.contains("sc3ml") ) {
		return "sc3ml";
	}

	return "xml";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Format and level of the request which writes the inventory.
std::string ImportFDSNWSDialog::saveParameters() const {
	return QString("format=%1&level=%2")
	       .arg(_format, _uiSideBar.checkResponses->isChecked() ? "response" : "channel")
	       .toStdString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Narrows the code constraints down to what is selected in the table.
void ImportFDSNWSDialog::updateConstraints() {
	auto *table = _ui.tableStations;

	// A channel level request fills the table with thousands of rows, so the
	// codes seen so far are kept in a set instead of being searched for in
	// the lists, which would be quadratic.
	QStringList codes[3];
	QSet<QString> seen[3];

	for ( int i = 0; i < table->rowCount(); ++i ) {
		auto *select = table->item(i, 0);
		if ( !select || select->checkState() != Qt::Checked ) {
			continue;
		}

		// Station, location and channel column, in that order.
		for ( int c = 0; c < 3; ++c ) {
			auto *item = table->item(i, c + 2);
			if ( !item ) {
				continue;
			}

			auto code = item->text();
			if ( c == 1 && code.isEmpty() ) {
				// The service denotes a blank location code this way.
				code = "--";
			}

			if ( !seen[c].contains(code) ) {
				seen[c].insert(code);
				codes[c].append(code);
			}
		}
	}

	// All three are constraints, they are reported as one change.
	{
		QSignalBlocker stationBlocker(_uiSideBar.editStations);
		QSignalBlocker locationBlocker(_uiSideBar.editLocations);
		QSignalBlocker channelBlocker(_uiSideBar.editChannels);

		_uiSideBar.editStations->setText(codes[0].join(","));
		_uiSideBar.editLocations->setText(codes[1].join(","));
		_uiSideBar.editChannels->setText(codes[2].join(","));
	}

	// Nothing selected is nothing to save.
	setSaveEnabled(!codes[0].isEmpty());
	clearStationCounts();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Shows the complete request the import button issues, and nothing while
// there is nothing to import.
void ImportFDSNWSDialog::updateURL() {
	_ui.textURL->setPlainText(
		_fetched ? QString::fromStdString(request(saveParameters().c_str())) : QString()
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Asks every server of the list how many stations it offers for the current
// constraints. Only the codes are needed, so the station level is requested.
void ImportFDSNWSDialog::testServers() {
	auto *combo = _uiSideBar.comboServerURL;

	// Changing item data makes an editable combo box replace the text of its
	// line edit with the text of the current item.
	QSignalBlocker blocker(combo);
	auto url = combo->currentText();

	// A URL typed by the user is not part of the list yet. It is added as a
	// preset so that it takes part in the comparison and keeps its number.
	// One entry is reserved for it, an earlier one is replaced.
	if ( currentDataCenter() < 0 && !url.isEmpty() ) {
		if ( _customIndex < 0 ) {
			_customIndex = _presetCount++;
			combo->insertItem(_customIndex, QString());
		}

		combo->setItemData(_customIndex, QVariant(), CountRole);
		setDataCenterItem(combo, _customIndex, tr("custom"), url);
	}

	// One window serves the whole test. It is sized once for the longest
	// label, so it does not grow while the servers are asked in turn.
	auto label = [this](const QString &base) { return tr("Testing %1").arg(base); };

	HttpRequest req;
	req.keepOpen();

	QFontMetrics metrics(req.font());
	QString longest;

	for ( int i = 0; i < combo->count(); ++i ) {
		auto text = label(combo->itemData(i).toString());
		if ( metrics.horizontalAdvance(text) > metrics.horizontalAdvance(longest) ) {
			longest = text;
		}
	}

	req.setLabelText(longest);
	req.setSkipButtons(tr("Skip this"), tr("Skip all"));

	// The presets are offered by the registry as well, every server is asked
	// only once.
	QHash<QString, int> counts;
	int most = 0;
	bool skipped = false;

	for ( int i = 0; i < combo->count(); ++i ) {
		auto base = combo->itemData(i).toString();
		if ( base.isEmpty() ) {
			continue;
		}

		auto known = counts.find(base);
		if ( known == counts.end() ) {
			if ( skipped ) {
				// The servers behind the button press are not asked at all.
				known = counts.insert(base, -1);
			}
			else {
				auto query = withParameters(queryURL(base + SERVICE_BASE),
				                            "format=text&level=station");
				SEISCOMP_DEBUG("FDSNWS: %s", query.c_str());

				req.setLabelText(label(base));

				// A cancelled or failed test is marked as such, 204 means the
				// server answered but has nothing to offer.
				int code = req.get(query.c_str());
				known = counts.insert(base, code == 200 ?
				                            countStations(QString(req.response())) :
				                            code == 204 ? 0 : -1);

				skipped = req.skippedAll();
			}
		}

		combo->setItemData(i, *known, CountRole);
		combo->setItemData(i, base + STATIONS_FOUND + stationCount(*known),
		                   Qt::ToolTipRole);
		most = qMax(most, *known);
	}

	req.hide();

	// Of the servers offering the most stations the alphabetically first one
	// is selected.
	int winner = -1;

	for ( int i = 0; i < combo->count(); ++i ) {
		auto count = combo->itemData(i, CountRole);
		bool best = most > 0 && count.isValid() && count.toInt() == most;

		combo->setItemData(i, best ? QVariant(QBrush(HIGHLIGHT_COLOR)) : QVariant(),
		                   Qt::BackgroundRole);

		if ( best && (winner < 0 || combo->itemText(i) < combo->itemText(winner)) ) {
			winner = i;
		}
	}

	if ( winner >= 0 ) {
		url = combo->itemData(winner).toString();
	}

	combo->setEditText(url);
	highlightServer();

	// The selection may have changed while the signals were blocked.
	updateServerTitle();
	updateURL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Marks the edit field while it shows one of the servers offering the most
// stations.
void ImportFDSNWSDialog::highlightServer() {
	auto *combo = _uiSideBar.comboServerURL;

	// Only the entries offering the most stations carry a background.
	int index = currentDataCenter();
	bool best = index >= 0 && combo->itemData(index, Qt::BackgroundRole).isValid();

	// An unresolved palette overrides nothing and thus restores the colours
	// the edit field inherits from the application.
	QPalette palette;

	if ( best ) {
		palette.setColor(QPalette::Base, HIGHLIGHT_COLOR);
	}

	combo->lineEdit()->setPalette(palette);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// The numbers belong to the constraints they were queried with and are
// dropped as soon as one of them changes.
void ImportFDSNWSDialog::clearStationCounts() {
	auto *combo = _uiSideBar.comboServerURL;

	QSignalBlocker blocker(combo);
	auto url = combo->currentText();
	bool cleared = false;

	for ( int i = 0; i < combo->count(); ++i ) {
		if ( !combo->itemData(i, CountRole).isValid() ) {
			continue;
		}

		combo->setItemData(i, QVariant(), CountRole);
		combo->setItemData(i, QVariant(), Qt::BackgroundRole);
		combo->setItemData(i, combo->itemData(i).toString(), Qt::ToolTipRole);
		cleared = true;
	}

	if ( cleared ) {
		combo->setEditText(url);
		highlightServer();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::checkDates() {
	bool valid = true;

	for ( auto *edit : {_uiSideBar.dtStart, _uiSideBar.dtEnd,
	                    _uiSideBar.dtStartBefore, _uiSideBar.dtStartAfter,
	                    _uiSideBar.dtEndBefore, _uiSideBar.dtEndAfter} ) {
		bool acceptable = edit->hasAcceptableInput();

		// Only the text is marked, not the button opening the calendar.
		if ( auto *line = edit->findChild<QLineEdit*>() ) {
			// An unresolved palette restores the inherited colours.
			QPalette palette;
			if ( !acceptable ) {
				palette.setColor(QPalette::Base, INVALID_COLOR);
			}

			line->setPalette(palette);
		}

		valid = valid && acceptable;
	}

	_ui.btnFetch->setEnabled(valid);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Importing needs streams which are listed and selected. The shown URL is the
// request the import button issues, so it is only shown when it can be used.
void ImportFDSNWSDialog::setSaveEnabled(bool enabled) {
	_fetched = enabled;
	_ui.okButton->setEnabled(enabled);

	// Copying the blank field would clear the clipboard.
	_ui.btnCopyURL->setEnabled(enabled);

	if ( enabled ) {
		_ui.okButton->setDefault(true);
	}
	else {
		_ui.btnFetch->setDefault(true);
	}

	updateURL();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::clearTable() {
	while ( _ui.tableStations->rowCount() ) {
		_ui.tableStations->removeRow(0);
	}

	setSaveEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImportFDSNWSDialog::fetch() {
	// The format of the inventory written is a property of the server
	_format = queryFormat();

	string query = request("format=text&level=channel");
	SEISCOMP_DEBUG("FDSNWS: %s", query.c_str());

	HttpRequest req;
	req.setLabelText(tr("Requesting channels"));

	int code = req.get(query.c_str());
	if ( code == 200 ) {
		// Filling the table must not be taken for a change of the selection.
		QSignalBlocker blocker(_ui.tableStations);

		clearTable();

		// With sorting enabled a row is moved as soon as its item in the sort
		// column is set, so the remaining items of that row would end up in
		// the wrong one and leave cells empty.
		_ui.tableStations->setSortingEnabled(false);

		QStringList rows = QString(req.response()).split("\n");
		int idx = 0;
		int iRow = 0;

		int colNet = 0;
		int colSta = 1;
		int colLoc = 2;
		int colCha = 3;
		int colLat = 4;
		int colLon = 5;
		int colElev = 6;
		int colDepth = 7;
		int colAzimuth = 8;
		int colDip = 9;
		int colSensor = 10;
		int colSPS = 14;
		int colStart = 15;
		int colEnd = 16;

		foreach ( QString row, rows ) {
			row = row.trimmed();
			if ( row.isEmpty() ) continue;

			SEISCOMP_DEBUG("FDSNWS: %s", qPrintable(row));

			if ( row[0] == '#' ) {
				// This is the very first valid line -> header
				if ( !idx ) {
					QStringList headers = row.mid(1).trimmed().split("|");
					int col = 0;

					foreach ( const QString &header, headers ) {
						if ( header == "Network" ) {
							colNet = col;
						}
						else if ( header == "Station" ) {
							colSta = col;
						}
						else if ( header == "Location" ) {
							colLoc = col;
						}
						else if ( header == "Channel" ) {
							colCha = col;
						}
						else if ( header == "Latitude" ) {
							colLat = col;
						}
						else if ( header == "Longitude" ) {
							colLon = col;
						}
						else if ( header == "Elevation" ) {
							colElev = col;
						}
						else if ( header == "Depth" ) {
							colDepth = col;
						}
						else if ( header == "Azimuth" ) {
							colAzimuth = col;
						}
						else if ( header == "Dip" ) {
							colDip = col;
						}
						else if ( header == "SensorDescription" ) {
							colSensor = col;
						}
						else if ( header == "SampleRate" ) {
							colSPS = col;
						}
						else if ( header == "StartTime" ) {
							colStart = col;
						}
						else if ( header == "EndTime" ) {
							colEnd = col;
						}

						++col;
					}

					SEISCOMP_DEBUG("FDSNWS: Header: %s", qPrintable(row));
				}
			}
			else {
				QStringList columns = row.split("|");
				int columnCount = columns.count();

				if ( colNet >= columnCount
				  || colSta >= columnCount
				  || colLoc >= columnCount
				  || colCha >= columnCount
				  || colLat >= columnCount
				  || colLon >= columnCount
				  || colElev >= columnCount
				  || colDepth >= columnCount
				  || colAzimuth >= columnCount
				  || colDip >= columnCount
				  || colSPS >= columnCount
				  || colStart >= columnCount
				  || colEnd >= columnCount ) {
					SEISCOMP_WARNING("FDSNWS: Some columns indexes are out of range, got %d columns", columnCount);
					continue;
				}

				auto *select = new QTableWidgetItem;
				select->setFlags(select->flags() & ~Qt::ItemIsEditable);
				select->setCheckState(Qt::Checked);

				_ui.tableStations->setRowCount(iRow+1);
				_ui.tableStations->setItem(iRow, 0, select);
				_ui.tableStations->setItem(iRow, 1, new QTableWidgetItem(columns[colNet]));
				_ui.tableStations->setItem(iRow, 2, new QTableWidgetItem(columns[colSta]));
				_ui.tableStations->setItem(iRow, 3, new QTableWidgetItem(columns[colLoc]));
				_ui.tableStations->setItem(iRow, 4, new QTableWidgetItem(columns[colCha]));
				_ui.tableStations->setItem(iRow, 5, new QTableWidgetItem(columns[colSPS]));
				_ui.tableStations->setItem(iRow, 6, new QTableWidgetItem(columns[colLat]));
				_ui.tableStations->setItem(iRow, 7, new QTableWidgetItem(columns[colLon]));
				_ui.tableStations->setItem(iRow, 8, new QTableWidgetItem(columns[colElev]));
				_ui.tableStations->setItem(iRow, 9, new QTableWidgetItem(dateOf(columns[colStart])));
				_ui.tableStations->setItem(iRow, 10, new QTableWidgetItem(dateOf(columns[colEnd])));
				_ui.tableStations->setItem(iRow, 11, new QTableWidgetItem(columns[colSensor]));

				++iRow;
			}

			++idx;
		}

		// Reactivate sorting
		_ui.tableStations->setSortingEnabled(true);
		_ui.tableStations->sortItems(_ui.tableStations->horizontalHeader()->sortIndicatorSection(),
		                             _ui.tableStations->horizontalHeader()->sortIndicatorOrder());

		_ui.tableStations->resizeColumnsToContents();
		_ui.tableStations->horizontalHeader()->setStretchLastSection(true);

		// There is nothing to save if the response held no channel at all.
		setSaveEnabled(_ui.tableStations->rowCount() > 0);
	}
	else if ( code == 204 ) {
		clearTable();
	}
	else {
		setSaveEnabled(false);

		QDialog errorDlg;

		errorDlg.setWindowTitle(tr("FDSNWS Error"));

		QVBoxLayout *vl = new QVBoxLayout;

		QLabel *headline = new QLabel;
		headline->setText(tr("Status: %1").arg(code));

		QPalette pal = headline->palette();
		pal.setColor(QPalette::WindowText, Qt::darkRed);
		pal.setColor(QPalette::Window, QColor(255,224,224));
		headline->setPalette(pal);
		headline->setAutoFillBackground(true);
		headline->setMargin(9);

		vl->addWidget(headline);

		QTextEdit *edit = new QTextEdit;
		edit->setText(req.response());
		edit->setReadOnly(true);
		edit->setWordWrapMode(QTextOption::WordWrap);
		vl->addWidget(edit);

		QPushButton *closeButton = new QPushButton;
		closeButton->setText(tr("Close"));
		connect(closeButton, SIGNAL(clicked(bool)), &errorDlg, SLOT(reject()));

		QHBoxLayout *hl = new QHBoxLayout;
		hl->addStretch();
		hl->addWidget(closeButton);

		vl->addLayout(hl);

		errorDlg.setLayout(vl);
		errorDlg.exec();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
