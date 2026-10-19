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



#ifndef SEISCOMP_CONFIGURATION_FANCYVIEW_H
#define SEISCOMP_CONFIGURATION_FANCYVIEW_H


#ifndef Q_MOC_RUN
#include <seiscomp/system/model.h>
#endif

#include <QAbstractItemView>
#include <QHash>

#include <functional>
#include <QScrollArea>


class QAbstractButton;
class QBoxLayout;
class QLabel;
class SearchWidget;


struct FancyViewItemEdit {
	virtual ~FancyViewItemEdit() {}
	virtual void setValue(const QString &value) = 0;
	virtual QString value() const = 0;
	virtual QWidget *widget() = 0;
};


struct FancyViewItem {
	FancyViewItem(const QModelIndex &idx = QModelIndex(),
	              QWidget *c = 0);

	bool isValid() const { return index.isValid(); }
	void updated();

	QPersistentModelIndex  index;
	QWidget               *container;
	QWidget               *label{nullptr};
	FancyViewItemEdit     *input{nullptr};
	QAbstractButton       *editControl{nullptr};
	QAbstractButton       *reset{nullptr};
	QWidget               *description{nullptr};
	QAbstractButton       *toggle{nullptr};

	QWidget               *block{nullptr};
};


class FancyView : public QAbstractItemView {
	Q_OBJECT


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		FancyView(QWidget *parent = 0);

		static constexpr int DefaultExpandLevel = 1;

		/**
		 * @brief Sets the nesting levels a module configuration is opened
		 *        with, applies to the views created afterwards.
		 * @param level The number of levels, 0 shows no parameter at all
		 *        and a negative value opens all levels. The default is
		 *        DefaultExpandLevel.
		 */
		static void setDefaultExpandLevel(int level);

		/**
		 * @brief Returns the nesting levels a module configuration is opened
		 *        with, the largest integer if all levels are opened.
		 */
		static int expandLevelDefault();

		/**
		 * @brief Shows the current configuration again with the nesting
		 *        levels set by setDefaultExpandLevel.
		 * @param force Whether to show it again also if the levels are the
		 *        ones it is shown with, which resets the sections opened or
		 *        closed in the meantime.
		 */
		void applyDefaultExpandLevel(bool force = false);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QRect visualRect(const QModelIndex &index) const override;
		void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
		QModelIndex indexAt(const QPoint &point) const override;

		void setModel(QAbstractItemModel * model) override;
		void setRootIndex(const QModelIndex &index) override;

		/**
		 * @brief Selects a parameter or a section of the shown configuration
		 *        and scrolls to it.
		 * @param name The full name of the parameter or the section, case
		 *        insensitive.
		 * @return Whether it was found or not.
		 */
		bool showParameter(const QString &name);
		void setConfigStage(Seiscomp::Environment::ConfigStage);

		/**
		 * @brief Evaluate a value based on properities adding to a text string
		 * @param value The value to evaluate
		 * @param param The object of properties
		 * @param eval The text string to return
		 * @return True if issues were found, false if no issues were found
		 */
		static bool evaluateValue(const std::string& value,
		                          const Seiscomp::System::Parameter *param,
		                          QString &eval, bool verbose = false);


	// ------------------------------------------------------------------
	//  Signals
	// ------------------------------------------------------------------
	signals:
		void rootIndexChanged();


	// ------------------------------------------------------------------
	//  Protected slots
	// ------------------------------------------------------------------
	protected slots:
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
		                 const QVector<int> &roles = QVector<int>()) override;


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

		void resizeEvent(QResizeEvent *event) override;
		void scrollContentsBy(int dx, int dy) override;

		QRegion visualRegionForSelection(const QItemSelection &selection) const override;

		void currentChanged(const QModelIndex &curr, const QModelIndex &prev) override;
		void keyboardSearch(const QString &search) override;


	// ------------------------------------------------------------------
	//  Private slots
	// ------------------------------------------------------------------
	private slots:
		void editChanged(bool);
		void resetValue();
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
		void evaluateInput(QWidget *w);
		void updateToolTip(QWidget *w, Seiscomp::System::Parameter *param,
		                   bool verbose = true);
		void updateContentGeometry();
		QWidget *createWidgetFromIndex(const QModelIndex &idx, int level);

		/**
		 * @brief Creates the parameters and the sub sections of a container
		 * @param childLevel The number of nesting levels to expand, counted
		 *        from the children which are created here.
		 */
		void populateChildren(QBoxLayout *l, const QModelIndex &idx,
		                      int childLevel, const QString &emptyText);

		/**
		 * @brief Creates the parameters of a container in a frame of their own
		 * @return The frame, nullptr if the container holds no parameter
		 */
		QWidget *populateParameters(QBoxLayout *l, const QModelIndex &idx);

		/**
		 * @brief Creates the content of a container or defers it until the
		 *        container is opened the first time.
		 * @param level The number of nesting levels to expand, counted from
		 *        the container itself. A value below one keeps it collapsed.
		 */
		void populateContainer(QBoxLayout *l, const QModelIndex &idx,
		                       int level, const QString &emptyText,
		                       QAbstractButton *toggle);

		void setupToggle(const QModelIndex &idx, QAbstractButton *toggle);

		void toggleClicked(const QModelIndex &idx, QAbstractButton *toggle,
		                   bool checked);

		/**
		 * @brief Expands or collapses all containers next to the given one
		 * @param withTree Whether the containers below them follow
		 */
		void toggleLevel(const QModelIndex &idx, bool expand, bool withTree);

		void toggleTree(const QModelIndex &idx, bool expand);

		QAbstractButton *toggleAt(const QModelIndex &idx) const;

		QModelIndex levelParent(const QModelIndex &idx) const;

		bool isTreeExpanded(const QModelIndex &idx) const;

		void updateBlockLines();

		void updateBlockLines(const QModelIndex &idx);

		/**
		 * @brief Checks whether all containers next to the given one are open
		 * @param withTree Whether the containers below them must be open too
		 */
		bool isLevelExpanded(const QModelIndex &idx, bool withTree) const;

		void realizeSection(const QModelIndex &idx);

		void realizePath(const QModelIndex &idx);

		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::BindingCategory *cat, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Binding *binding, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Section *sec, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Group *group, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Structure *struc, bool collapsed);
		FancyViewItem add(QLayout *layout, const QModelIndex &idx);


	private:
		typedef QHash<QPersistentModelIndex, FancyViewItem> ViewItems;
		Seiscomp::Environment::ConfigStage _configStage{Seiscomp::Environment::CS_CONFIG_APP};

		int           _expandLevel;
		QWidget      *_rootWidget{nullptr};
		ViewItems     _viewItems;
		QIcon         _iconEdit;
		QIcon         _iconReset;
		QWidget      *_currentItem{nullptr};
		QLabel       *_optionEditHint{nullptr};
		void         *_blockPopulate{nullptr};

		QHash<QPersistentModelIndex, std::function<void ()>> _pendingSections;
};


#endif
