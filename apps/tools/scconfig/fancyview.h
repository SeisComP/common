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



#ifndef SEISCOMP_CONFIGURATION_FANCYVIEW_H__
#define SEISCOMP_CONFIGURATION_FANCYVIEW_H__


#ifndef Q_MOC_RUN
#include <seiscomp/system/model.h>
#endif

#include <QAbstractItemView>
#include <QScrollArea>


class QLabel;
class QBoxLayout;



struct FancyViewItemEdit {
	virtual ~FancyViewItemEdit() {}
	virtual void setValue(const QString &value) = 0;
	virtual QString value() const = 0;
	virtual QWidget *widget() = 0;
};


struct FancyViewItem {
	FancyViewItem(const QModelIndex &idx = QModelIndex(),
	              QWidget *c = NULL);

	bool isValid() const { return index.isValid(); }

	QPersistentModelIndex  index;
	QWidget               *container;
	QWidget               *label;
	FancyViewItemEdit     *input;
	QWidget               *description;
};


class FancyView : public QAbstractItemView {
	Q_OBJECT


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		FancyView(QWidget *parent = 0);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QRect visualRect(const QModelIndex &index) const override;
		void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
		QModelIndex indexAt(const QPoint &point) const override;

		void setModel(QAbstractItemModel * model) override;
		void setRootIndex(const QModelIndex &index) override;
		void setConfigStage(Seiscomp::Environment::ConfigStage);


	// ------------------------------------------------------------------
	//  Protected slots
	// ------------------------------------------------------------------
	protected slots:
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
		                 const QVector<int> &roles = QVector<int>()) override;
		void rowsInserted(const QModelIndex &parent, int start, int end) override;
		void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		bool eventFilter(QObject *o, QEvent *e) override;
		bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
		QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
		                       Qt::KeyboardModifiers modifiers) override;

		int horizontalOffset() const override;
		int verticalOffset() const override;

		bool isIndexHidden(const QModelIndex &index) const override;

		void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command) override;

		void mousePressEvent(QMouseEvent *event) override;

		void mouseMoveEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;

		void paintEvent(QPaintEvent *event) override;
		void resizeEvent(QResizeEvent *event) override;
		void scrollContentsBy(int dx, int dy) override;

		QRegion visualRegionForSelection(const QItemSelection &selection) const override;

		void currentChanged(const QModelIndex &curr, const QModelIndex &prev) override;
		void keyboardSearch(const QString &search) override;


	// ------------------------------------------------------------------
	//  Private slots
	// ------------------------------------------------------------------
	private slots:
		void lockChanged(bool);
		void optionTextEdited();
		void optionTextChanged(const QString &);
		void optionToggled(bool);
		void bindingCategoryChanged(int);
		void addStruct();
		void removeStruct();
		void addCategoryBinding();
		void removeCategoryBinding();


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		void updateToolTip(QWidget *w, Seiscomp::System::Parameter *param);
		void updateContentGeometry();
		QWidget *createWidgetFromIndex(const QModelIndex &idx, const QString &);

		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::BindingCategory *cat, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Binding *binding, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Section *sec, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item, Seiscomp::System::Group *group);
		bool add(QBoxLayout *&layout, FancyViewItem &item, Seiscomp::System::Structure *struc);
		FancyViewItem add(QLayout *layout, const QModelIndex &idx);

		/**
		 * @brief Evaluate a value based on properities adding to a text string
		 * @param value The value to evaluate
		 * @param param The object of properties
		 * @param eval The text string to return
		 * @return True if issues were found, false if no issues were found
		 */
		bool evaluateValue(const std::string& value,
		                   const Seiscomp::System::Parameter *param,
		                   QString &eval, bool verbose);

	private:
		typedef QHash<QPersistentModelIndex, FancyViewItem> ViewItems;
		Seiscomp::Environment::ConfigStage _configStage;
		QWidget *_rootWidget;
		ViewItems _viewItems;
		QIcon _lockIcon;
		QIcon _unlockIcon;
		QIcon _traceIcon;
		QWidget *_currentItem;
		QLabel *_optionEditHint;
		void *_blockPopulate;
};


#endif
