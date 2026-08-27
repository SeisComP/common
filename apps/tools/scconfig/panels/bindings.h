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


#ifndef SEISCOMP_CONFIGURATION_GUI_PANEL_BINDINGS_H__
#define SEISCOMP_CONFIGURATION_GUI_PANEL_BINDINGS_H__

#include "../gui.h"


class QAction;
class QPushButton;
class QTreeView;
class SearchWidget;


class BindingView : public QWidget {
	Q_OBJECT

	public:
		BindingView(QWidget *parent = 0);

	public:
		void clear();

		void setModel(ConfigurationTreeItemModel *base, QAbstractItemModel *model);
		void setRootIndex(const QModelIndex & index);
		QModelIndex rootIndex() const { return _rootIndex; }

		//! Navigates to a parameter or a section of the shown binding
		bool showParameter(const QString &name);
		void saved();

	signals:
		void closeRequested();

	private slots:
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsRemoved(const QModelIndex &parent, int start, int end);

		void openSearch();
		void closeSearch();


	private:
		QAbstractItemModel                     *_model;
		QPersistentModelIndex                   _rootIndex;
		QWidget                                *_header;
		SearchWidget                           *_searchWidget;
		QLabel                                 *_icon;
		QLabel                                 *_label;
		QPushButton                            *_closeButton;
		ConfigurationTreeItemModel             *_bindingModel;
		QAbstractItemView                      *_view;
};


class BindingsPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		BindingsPanel(QWidget *parent = 0);

	private:
		//! The part of the panel state which survives a model rebuild
		struct ViewState {
			QString module;         //!< Selected module
			QString profile;        //!< Selected profile, empty if none
			QString network;        //!< Network of the opened station folder
			QString station;        //!< Station of the opened station folder
			QString bindingModule;  //!< Station binding shown in the binding view
			bool    profileBinding{false}; //!< The binding view shows the profile
		};

		ViewState currentViewState() const;
		void restoreViewState(const ViewState &state);

	protected:
		void setModel(ConfigurationTreeItemModel *model) override;
		void saved() override;

	private:
		//! Shows the parameters of the binding at idx in the binding view.
		void openBinding(const QModelIndex &idx);
		void deleteStation(const QModelIndex &idx);
		void deleteProfile(const QModelIndex &idx);
		void selectBindings(QItemSelectionModel *, void*, const QModelIndex &parent = QModelIndex());
		void syncProfileRemoval(QAbstractItemModel *, void*, const QModelIndex &parent = QModelIndex());
		void clearModuleBindings(const QModelIndex &idx);
		void removeModuleBindings(const QModelIndex &idx, const QString &mod,
		                          const QString *name);

		typedef QMap<QString, QSet<QString> > ModuleBindingMap;
		void collectModuleBindings(ModuleBindingMap &, const QModelIndex &idx);

	public:
		/**
		 * @brief Selects a module and shows all its profiles.
		 * @param name The module name, case insensitive.
		 * @return Whether the module was found or not.
		 */
		bool setCurrentModule(const QString &name) override;

		/**
		 * @brief Selects a station and shows all its bindings.
		 * @param networkCode The network code, case insensitive.
		 * @param stationCode The station code, case insensitive.
		 * @return Whether the station was found or not.
		 */
		bool setCurrentStation(const QString &networkCode,
		                       const QString &stationCode) override;

		/**
		 * @brief Navigates to a parameter or a section of the shown binding.
		 * @param name The full name, case insensitive.
		 * @return Whether it was found or not.
		 */
		bool setCurrentParameter(const QString &name) override;

		void updateIndication();
		bool assignProfile(const QModelIndex &, const QString &module,
		                   const QString &profile);

	private slots:
		void applyModel();

		// BindingsView slots
		void moduleBindingsChanged(const QModelIndex &, const QModelIndex &);

		void bindingActivated(const QModelIndex &);
		void bindingDoubleClicked(const QModelIndex &);
		void changeFolder(const QModelIndex &);

		void folderViewContextMenu(const QPoint&);
		void modulesViewContextMenu(const QPoint&);
		void modulesFolderViewContextMenu(const QPoint&);

		// ModuleView slots
		void moduleTreeCurrentChanged(const QModelIndex &, const QModelIndex &);
		void profileActivated(const QModelIndex &);
		void profileDoubleClicked(const QModelIndex &);

		// FolderView tools
		void folderLevelUp();
		void deleteItem();
		void switchToStationsIconView();
		void switchToStationsListView();
		void switchToProfileIconView();
		void switchToProfileListView();

		// ModulesView tools
		void addProfile();
		void deleteProfile();


	private:
		QAbstractItemView               *_stationsTreeView;
		QListView                       *_stationsFolderView;
		QAbstractItemView               *_modulesView;
		QListView                       *_modulesFolderView;
		BindingView                     *_bindingView;
		QStandardItemModel              *_bindingsModel;
		QStandardItemModel              *_profilesModel;
		QAction                         *_folderLevelUp;
		QAction                         *_deleteItem;
		QAction                         *_addProfile;
		QAction                         *_deleteProfile;
		Seiscomp::System::ModuleBinding *_currentBinding{nullptr};
};


#endif
