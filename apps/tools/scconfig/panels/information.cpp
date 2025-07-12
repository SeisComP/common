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


#include "information.h"

#include <seiscomp/system/environment.h>

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>


InformationPanel::InformationPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	_name = "Information";
	_icon = QIcon(":/res/icons/info.png");
	setHeadline("Information");
	setDescription("System paths and variables");

	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);

	QTableWidget *table = new QTableWidget;
	table->setFrameShape(QFrame::NoFrame);
	table->setColumnCount(2);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setStretchLastSection(true);
	table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	table->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");
	table->setAlternatingRowColors(true);
	table->setSelectionMode(QAbstractItemView::NoSelection);

	l->addWidget(table);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	addRow(table, "PATH", getenv("PATH"));
	addRow(table, "ROOTDIR", env->installDir().c_str());
	addRow(table, "SYSTEMCONFIGDIR", env->appConfigDir().c_str());
	addRow(table, "DEFAULTCONFIGDIR", env->globalConfigDir().c_str());
	addRow(table, "DATADIR", env->shareDir().c_str());
	addRow(table, "CONFIGDIR", env->configDir().c_str());
	addRow(table, "LOGDIR", env->logDir().c_str());

	table->resizeColumnsToContents();
}


void InformationPanel::addRow(QTableWidget *t, const QString &name, const QString &value) {
	int row = t->rowCount();
	t->insertRow(row);
	QTableWidgetItem *nameItem = new QTableWidgetItem(name);
	QTableWidgetItem *valueItem = new QTableWidgetItem(value);
	nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	t->setItem(row, 0, nameItem);
	t->setItem(row, 1, valueItem);
}
