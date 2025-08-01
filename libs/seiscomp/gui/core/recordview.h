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



#ifndef SEISCOMP_GUI_RECORDVIEW_H
#define SEISCOMP_GUI_RECORDVIEW_H


#ifndef Q_MOC_RUN
#include <seiscomp/gui/core/recordviewitem.h>
#include <seiscomp/gui/core/timescale.h>
#include <seiscomp/math/filter.h>
#endif
#include <QScrollArea>
#include <QTimer>
#include <QSet>


namespace Seiscomp {
namespace DataModel {

SC_GUI_API
bool operator<(const Seiscomp::DataModel::WaveformStreamID& left,
               const Seiscomp::DataModel::WaveformStreamID& right);

}
}

namespace Seiscomp {

class RecordSequence;

namespace Gui {


class TimeScale;
class RecordStreamThread;


class SC_GUI_API RecordView : public QWidget {
	Q_OBJECT

	public:
		enum SelectionMode {
			NoSelection = 0,
			SingleSelection,
			ExtendedSelection
		};

		enum SelectionOperationFlag {
			SelectNone   = 0x00,
			Select       = 0x01,
			SelectPlus   = 0x02,
			SelectMinus  = 0x04,
			SelectAll    = Select | SelectPlus | SelectMinus
		};

		Q_DECLARE_FLAGS(SelectionOperation, SelectionOperationFlag)


	public:
		//! Default c'tor
		//! The mode defaults to ringbuffer with a buffer
		//! size of 30 minutes
		RecordView(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags(),
		           TimeScale *timeScale = 0);

		//! Creates a RecordView using a time window
		RecordView(const Seiscomp::Core::TimeWindow&,
		           QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags(),
		           TimeScale *timeScale = 0);

		//! Creates a RecordView using a timespan and
		//! a ringbuffer
		RecordView(const Seiscomp::Core::TimeSpan&,
		           QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags(),
		           TimeScale *timeScale = 0);

		~RecordView();


	public:
		//! Sets the timewindow used for the stream data.
		//! Incoming records will only be inserted when they
		//! fit into the timewindow.
		void setTimeWindow(const Seiscomp::Core::TimeWindow& tw);

		//! Sets the buffersize used for the stream data.
		//! When setting the buffersize, incoming records will be
		//! appended to the end of the stream. Records that do not
		//! fit into the buffer will be removed from the front of
		//! the buffer: ringbuffer mode
		void setBufferSize(const Seiscomp::Core::TimeSpan& ts);

		//! Returns the number of record stream
		int rowCount() const;

		//! Returns the number of visible streams
		int visibleRowCount() const;

		//! Returns the stream ID used for a row
		DataModel::WaveformStreamID streamID(int row) const;

		//! Returns whether a row is enabled or not
		bool isEnabled(int row) const;

		//! Returns the item in row 'row'
		RecordViewItem* itemAt(int row) const;

		//! Returns the item for streamID
		RecordViewItem* item(const DataModel::WaveformStreamID& streamID) const;

		//! Adds a new item to the view.
		//! If there already has been added an item
		//! for streamID, nullptr is returned.
		RecordViewItem* addItem(const DataModel::WaveformStreamID&,
		                        const QString& stationCode,
		                        int slotCount = 0);

		//! Adds an existing item to the view
		bool addItem(RecordViewItem* item);

		//! Removes a particular row
		bool removeItem(int row);

		//! Removes a RecordViewItem
		bool removeItem(RecordViewItem*);

		//! Removes and returns the item from the given row in the recordview;
		//! otherwise returns nullptr.
		RecordViewItem* takeItem(int row);

		//! Removes the item from the view and returns true or false
		bool takeItem(RecordViewItem*);

		//! Cycles through all rows that row 'row' is the first
		//! row in the view
		void cycleToRow(int row);

		//! Removes all available rows
		void clear();

		//! Clears all available records
		void clearRecords();

		//! Clears all available markers
		void clearMarker();

		//! Sets the height of a row in pixels
		void setRowHeight(int height, bool allowStretch = false);

		//! Returns the current row height
		int rowHeight() const;

		//! Sets the default row height
		void setDefaultRowHeight(int height);

		//! Sets the number of column to use when a new row is created
		void setDefaultItemColumns(int numCols);

		//! Sets the spacing between each row in pixels
		void setRowSpacing(int spacing);

		//! Sets the width of the label for each row
		void setLabelWidth(int width);

		//! Returns the label width
		int labelWidth() const;

		//! Sets the spacing between the label and the recordwidget
		void setHorizontalSpacing(int);

		//! Returns the horizontal spacing
		int horizontalSpacing() const;

		//! En-/Disables showing frames around the labels and the
		//! recordwidgets
		void setFramesEnabled(bool);

		//! Sets frame margin of the labels and the recordwidgets
		void setFrameMargin(int margin);

		//! Sets the zoom factor used for zoom in/out slots
		void setZoomFactor(float factor);
		float zoomFactor() const;

		//! Returns the current zoom spot
		QPointF zoomSpot() const;

		void setMinimumRowHeight(int);
		void setMaximumRowHeight(int);
		void setRelativeRowHeight(int desiredNumberOfTraces);

		//! Returns the current alignment
		Seiscomp::Core::Time alignment() const;

		//! Returns the current item
		RecordViewItem* currentItem() const;

		QList<RecordViewItem*> selectedItems() const;
		bool hasSelectedItems() const;

		double timeRangeMin() const;
		double timeRangeMax() const;

		double timeScale() const;

		Core::Time mapToTime(int x) const;

		//! Returns all streams belonging to a station
		QList<RecordViewItem*> stationStreams(const std::string& networkCode,
		                                      const std::string& stationCode) const;

		//! Copies the view state from another recordview
		bool copyState(RecordView *from);

		//! Moves all items to another RecordView
		bool moveItemsTo(RecordView *to);

		//! Moves selected streams to another RecordView
		bool moveSelectionTo(RecordView *to);

		//! Sets the default actions for interactive keyboard
		//! navigation:
		//!  - Left/Right: Moves the records left/right
		//!  - '+'/'-': Zooms the records vertically
		//!  - 'y'/'x': Zooms tehe records horizontally
		//!  - 'f': Toggles filtering
		//!  - 'r': Toggles showing filtered and raw records
		void setDefaultActions();

		//! Sets the datasource to read records from.
		//! This method initializes the reader thread and
		//! holds it until start() is called
		bool setDataSource(const QString& streamURL);

		//! Connects to the data source
		virtual bool connectToDataSource();

		//! Starts reading from the set data source
		virtual bool start();

		//! Stops reading from the data source
		virtual void stop();

		//! Returns the current recordstream thread
		RecordStreamThread* recordStreamThread() const;

		QWidget* infoWidget() const;
		TimeScale* timeWidget() const;

		/**
		 * @brief Returns the time window covered by all available data
		 * @return The data time window
		 */
		Core::TimeWindow coveredTimeRange() const;

		Core::TimeWindow visibleTimeRange() const;


	public slots:
		void setRecordUpdateInterval(int ms);

		//! Feeds a record into the RecordView
		//! A new row is appended if rec's stream id
		//! is fed for the first time
		bool feed(const Seiscomp::Record *rec);
		bool feed(const Seiscomp::RecordPtr rec);
		bool feed(Seiscomp::Record *rec);

		void scrollLeft();
		void scrollLeftSlowly();
		void scrollRight();
		void scrollRightSlowly();
		void scrollLineUp();
		void scrollLineDown();
		void scrollPageUp();
		void scrollPageDown();
		void scrollToTop();
		void scrollToBottom();

		void selectPreviousRow();
		void selectNextRow();

		void selectFirstRow();
		void selectLastRow();

		void enableFilter(bool);
		void enableFilter(int, bool);

		//! Whether to show the current selected recordstream or
		//! both recordstreams
		void showAllRecords(bool enable);

		//! Whether to show record borders
		void showRecordBorders(bool enable);

		//! Whether to show engineering values with unit prefix such as
		//! milli, kilo, etc.
		void showEngineeringValues(bool enable);

		//! Whether to draw the background using alternating colors
		//! The item background will be drawn using QPalette::Base and
		//! QPalette::AlternateBase
		void setAlternatingRowColors(bool enable);

		//! Enables/disables the scrollbar
		void showScrollBar(bool show);

		//! Whether to automatically insert new items for new records
		//! when using feed(...)
		void setAutoInsertItem(bool enable);

		void setAbsoluteTimeEnabled(bool enable);

		void setAutoScale(bool enable);
		void setAutoMaxScale(bool enable);

		bool isFilterEnabled() const;

		void setScale(double t, double a = 0.0f);
		void setTimeRange(double t1, double t2);
		void setSelection(double t1, double t2);

		void move(double offset);

		void setSelectionEnabled(bool);
		void setSelectionMode(SelectionMode mode);

		void clearSelection();

		//! Sets the justification of the records regarding the
		//! alignment time. 0 (Left) ... 1 (Right)
		void setJustification(float);

		//! Aligns the RecordView regarding the justification in
		//! the current viewport on the set alignment time
		void align();

		void horizontalZoom(float factor);
		void horizontalZoomIn()  { horizontalZoom( _zoomFactor); }
		void horizontalZoomOut() { horizontalZoom(1.0f/_zoomFactor); }

		void verticalZoom(float factor);
		void verticalZoomIn()  { verticalZoom(_zoomFactor); }
		void verticalZoomOut() { verticalZoom(1.0f/_zoomFactor); }

		void zoom(float factor);
		void zoomIn() { zoom(_zoomFactor); }
		void zoomOut() { zoom(1.0f/_zoomFactor); }

		void scaleAmplitudesUp();
		void scaleAmplitudesDown();

		void scaleVisibleAmplitudes();
		void scaleAllRecords();

		//! Sets the zoom spot. p is in global screen coordinates
		void setZoomSpotFromGlobal(const QPoint& p);

		//! Sets the relative zoom spot in logical coords
		void setZoomSpot(const QPointF& p);

		//! Sets the zoom rectangle in global coordinates.
		void setZoomRectFromGlobal(const QRect &rect);

		void setZoomRect(const QRectF &rect);

		//! Sort the items by text of the label using the text in
		//! row 'row'
		void sortByText(int row);

		//! Sort the items by text of the label using first text in
		//! row 'row1' and then text in row 'row2'
		void sortByText(int row1, int row2);

		//! Sort the items by text of the label using first text in
		//! row 'row1' and then text in row 'row2' and then text in row 'row3'
		void sortByText(int row1, int row2, int row3);

		//! Sort the items by text of the label using first text in
		//! row 'row1' and then text in row 'row2' and then text in row 'row3'
		//! and then text in row 'row4'.
		void sortByText(int row1, int row2, int row3, int row4);

		//! Sort the items by the value set in column 'column'
		void sortByValue(int column);

		//! Sort the items by the value set in column1 and then
		//! by value in column2
		void sortByValue(int column1, int column2);

		//! Sort the items by the value set in column1 then
		//! by value in column2 and finally by value in column3
		void sortByValue(int column1, int column2, int column3);

		//! Sort the items by the value set in column1 then
		//! by value in column2 then by value in column3 and finally by value
		//! in column4
		void sortByValue(int column1, int column2, int column3, int column4);

		//! Sort the items by text of a row and then by the value set in
		//! a column
		void sortByTextAndValue(int row, int column);

		//! Sort the items by the data set with RecordViewItem::setData
		void sortByData();

		//! Finds a row by its text using regular expressions.
		//! The first occurence according the sorting is returned.
		//! If no item matches then -1 is returned.
		int findByText(int row, const QRegularExpression &regexp, int startRow = 0) const;

		//! Sort the items by the time value of the markers with
		//! text markerText
		void sortByMarkerTime(const QString& markerText);

		void setAlignment(const Seiscomp::Core::Time& time);

		//! Aligns all rows on their marker with text set to
		//! text.
		void alignOnMarker(const QString& text);

		//! Sets the cursor text for all rows
		void setCursorText(const QString& text);

		//! Selects an item
		void setItemSelected(RecordViewItem* item, bool select);
		void deselectAllItems();

		void setCurrentItem(RecordViewItem* item);

		void ensureVisible(int row);

		void showSlot(int slot);
		void showComponent(char componentCode);

		//! Enables zooming by drawing a zoomrect with
		//! the mouse
		void setZoomEnabled(bool = true);

		/**
		 * @brief Enables rubber band selection with the mouse
		 * @param enable Whether to enable or disable it
		 * @param filter Which operations are enabled
		 */
		void setRubberBandSelectionEnabled(bool enable = true,
		                                   SelectionOperation filter = SelectAll);

		/**
		 * @brief Restores the default selection mode (rows) and disables
		 *        either rubberband selection or zooming.
		 */
		void restoreSelectionMode();

		void setDefaultDisplay();

		//! Sets the parameters used to filter the traces
		void setFilter(RecordWidget::Filter *filter);
		bool setFilterByName(const QString&);

		//! Returns the set filter instance
		RecordWidget::Filter *filter() const;

		void updateRecords();

		void setRecordBorderDrawMode(RecordWidget::RecordBorderDrawMode mode);


	signals:
		void updatedRecords();
		void fedRecord(RecordViewItem*, const Seiscomp::Record*);

		void updatedInterval(double da, double dt, double ofs);
		void toggledFilter(bool);

		void scaleChanged(double time, double amplitude);
		void timeRangeChanged(double tmin, double tmax);
		void selectionChanged(double smin, double smax);
		void alignmentChanged(const Seiscomp::Core::Time&);

		void amplScaleChanged(double);

		//! This signal will be emitted whenever a new item
		//! has been automatically added to the view.
		//! Connected classes can set the columns of the item
		//! depending on the first record used for creation
		//! of the item.
		void addedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*);

		//! This signal is emitted whenever an item will be enabled
		//! or disabled
		void changedItem(RecordViewItem*, bool enabled);

		//! This signal will be emitted when a time (absolut time)
		//! has been selected inside a RecordWidget
		void selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time);

		void progressStarted();
		void progressChanged(int value);
		void progressFinished();

		//! This signal will be emitted whenever the selection
		//! changes and when in SingleSelection mode
		void currentItemChanged(RecordViewItem* current, RecordViewItem* last);

		//! This signal is emitted whenever the selection changes.
		void selectionChanged();

		void cursorTextChanged(const QString&);

		//! This signal is emitted when a filter string is dropped into the
		//! recordview and the filter has been set and enabled successfully
		void filterChanged(const QString&);

	#ifndef Q_MOC_RUN
	// Hack to set the signal public for Qt 4 wich are protected by default.
	public:
	#endif
		/**
		 * @brief This signal is being emitted if the user selected an area
		 *        with the rubber band selection tool.
		 * @param rect The selected rectangle in global coordinates
		 * @param tmin The minimum absolute time of the selected time range
		 * @param tmax The maximum absolute time of the selected time range
		 * @param operation The operation to apply.
		 */
		void selectedRubberBand(QRect rect,
		                        QList<Seiscomp::Gui::RecordViewItem*>,
		                        double tmin, double tmax,
		                        Seiscomp::Gui::RecordView::SelectionOperation operation);


	private slots:
		void onItemClicked(RecordViewItem*, bool buttonDown = false,
		                   Qt::KeyboardModifiers = Qt::NoModifier);
		void selectedTime(Seiscomp::Core::Time);
		void sliderAction(int action);


	protected:
		//! This method can be reimplemented in derived
		//! classes to create a custom record widget
		virtual RecordWidget *createRecordWidget(
			const DataModel::WaveformStreamID &streamID
		) const;

		//! This method can be reimplemented in derived
		//! classes to create a custom label
		virtual RecordLabel *createLabel(RecordViewItem*) const;

		bool event(QEvent* event);
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);

		void changeEvent(QEvent* e);
		void resizeEvent(QResizeEvent *event);

		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);

		void closeThread();


	private:
		void setupUi();

		void colorItem(RecordViewItem* item, int row);
		void colorItem(RecordViewItem*);
		void scaleContent();

		template <typename T>
		void sortRows(std::list< std::pair<T, RecordViewItem*> >&);

		void layoutRows();

		void applyBufferChange();

		double mapToUnit(int x) const;
		// bool buildFilter(const QString& text, std::vector<Seiscomp::Math::Filtering::IIR::Filter<float>* >* filterList);


	private:
		enum Mode {
			TIME_WINDOW,
			RING_BUFFER
		};

		typedef QMap<DataModel::WaveformStreamID, RecordViewItem*> Items;
		typedef QVector<RecordViewItem*> Rows;
		typedef QSet<RecordViewItem*> SelectionList;

		SelectionMode            _selectionMode{NoSelection};

		RecordStreamThread      *_thread{nullptr};
		RecordViewItem          *_currentItem{nullptr};

		TimeScale               *_timeScaleWidget{nullptr};
		QScrollArea             *_scrollArea{nullptr};
		QWidget                 *_timeScaleInfo{nullptr};
		QLayout                 *_timeScaleAuxLayout{nullptr};

		QAction                 *_filterAction{nullptr};
		QAction                 *_absTimeAction{nullptr};

		QTimer                   _recordUpdateTimer;

		SelectionList            _selectedItems;

		Mode                     _mode{RING_BUFFER};
		Seiscomp::Core::Time     _timeStart;
		Seiscomp::Core::TimeSpan _timeSpan;

		Items                    _items;
		Rows                     _rows;
		Core::Time               _alignment;

		QPointF                  _zoomSpot{0.5, 0.5};

		int                      _rowHeight;
		int                      _minRowHeight;
		int                      _maxRowHeight{-1};
		int                      _numberOfRows{-1};
		int                      _defaultRowHeight{16};
		float                    _zoomFactor{2.0f};

		double                   _tmin{0}, _tmax{0};

		double                   _timeScale{1.0 / 3.0}; // pixels per second
		double                   _minTimeScale{0.0};
		double                   _amplScale{0.0}; // amplitude units per pixel

		bool                     _filtering{false}; // the filter state
		bool                     _alternatingColors{false};
		bool                     _showAllRecords{false};
		bool                     _showRecordBorders{false};
		bool                     _showEngineeringValues{true};
		bool                     _autoInsertItems{true};
		bool                     _autoScale{false};
		bool                     _autoMaxScale{false};

		bool                     _frames{false};
		int                      _frameMargin{0};
		int                      _horizontalSpacing{0};
		int                      _rowSpacing{0};

		int                      _labelWidth{70};
		int                      _labelColumns{3};

		RecordWidget::RecordBorderDrawMode _recordBorderDrawMode;

		RecordWidget::Filter    *_filter{nullptr};

	friend class RecordViewItem;
};


}
}


#endif
