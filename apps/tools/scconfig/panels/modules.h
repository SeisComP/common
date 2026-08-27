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


#ifndef SEISCOMP_CONFIGURATION_GUI_PANEL_MODULES_H
#define SEISCOMP_CONFIGURATION_GUI_PANEL_MODULES_H


#include "../gui.h"


class SearchWidget;


class ModulesPanel : public ConfiguratorPanel {
	Q_OBJECT


	public:
		ModulesPanel(QWidget *parent = 0);


	public:
		void setModel(ConfigurationTreeItemModel *model) override;
		void aboutToClose();

		/**
		 * @brief Selects the configuration of a particular module.
		 * @param name The module name, case insensitive.
		 * @return Whether the module was found or not.
		 */
		bool setCurrentModule(const QString &name) override;

		/**
		 * @brief Navigates to a parameter or a section of the module.
		 * @param name The full name, case insensitive.
		 * @return Whether it was found or not.
		 */
		bool setCurrentParameter(const QString &name) override;


	private slots:
		void applyModel();

		void moduleSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*);
		void moduleSelected(QTreeWidgetItem*,int);
		void moduleChanged(const QModelIndex &index);

		void openSearch();
		void closeSearch();


	private:
		bool               _modified;
		QAbstractItemView *_moduleView;
		QTreeWidget       *_moduleTree;
		SearchWidget      *_searchWidget;
};


#endif
