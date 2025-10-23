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

#include <seiscomp/core/build_version.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/system/environment.h>

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
InformationPanel::InformationPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	_name = "Information";
	_icon = QIcon(":/scconfig/icons/menu_scconfig_information.svg");
	setHeadline("Information");
	setDescription("SeisComP and environment variables");

	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);

	QTableWidget *table = new QTableWidget;
	table->setFrameShape(QFrame::NoFrame);
	table->setColumnCount(2);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->horizontalHeader()->setStretchLastSection(true);
	table->setAlternatingRowColors(true);
	table->setSelectionMode(QAbstractItemView::NoSelection);

	l->addWidget(table);

	auto headerFont = font();
	headerFont.setBold(true);

	int row = table->rowCount();
	table->insertRow(row);
	QTableWidgetItem *header = new QTableWidgetItem(tr("SeisComP path variables"));
	header->setFlags(Qt::ItemIsEnabled);
	header->setFont(headerFont);
	table->setItem(row, 0, header);
	table->setSpan(row, 0, 1, 2);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	addRow(table, "@ROOTDIR@", env->installDir().c_str());
	addRow(table, "@SYSTEMCONFIGDIR@", env->appConfigDir().c_str());
	addRow(table, "@DEFAULTCONFIGDIR@", env->globalConfigDir().c_str());
	addRow(table, "@DATADIR@", env->shareDir().c_str());
	addRow(table, "@CONFIGDIR@", env->configDir().c_str());
	addRow(table, "@LOGDIR@", env->logDir().c_str());

	row = table->rowCount();
	table->insertRow(row);
	header = new QTableWidgetItem(tr("Environment variables"));
	header->setFlags(Qt::ItemIsEnabled);
	header->setFont(headerFont);
	table->setItem(row, 0, header);
	table->setSpan(row, 0, 1, 2);

	addRow(table, "$SEISCOMP_ROOT", getenv("SEISCOMP_ROOT"));
	addRow(table, "$LD_LIBRARY_PATH", getenv("LD_LIBRARY_PATH"));
	addRow(table, "$MANPATH", getenv("MANPATH"));
	addRow(table, "$PATH", getenv("PATH"));
	addRow(table, "$PYTHONPATH", getenv("PYTHONPATH"));

	row = table->rowCount();
	table->insertRow(row);
	header = new QTableWidgetItem(tr("Software information"));
	header->setFlags(Qt::ItemIsEnabled);
	header->setFont(headerFont);
	table->setItem(row, 0, header);
	table->setSpan(row, 0, 1, 2);

	addRow(table, "Version", Seiscomp::Core::CurrentVersion.toString().data());
	addRow(table, "API Version", Seiscomp::Core::CurrentVersion.api().toString().data());
	addRow(table, "Data Schema Version", QString("%1.%2.%3")
		.arg(Seiscomp::DataModel::Version().Major)
		.arg(Seiscomp::DataModel::Version().Minor)
		.arg(Seiscomp::DataModel::Version().Revision)
	);

	#ifdef SC_GIT_REVISION
	addRow(table, "Git Revision", SC_GIT_REVISION);
	#endif
	#ifdef SC_BUILD_SYSTEM
	addRow(table, "Build System", SC_BUILD_SYSTEM);
	#endif
	#ifdef SC_COMPILER_VERSION
	addRow(table, "Compiler Version", SC_COMPILER_VERSION);
	#endif
	#ifdef SC_OS_VERSION
	addRow(table, "Operating System", SC_OS_VERSION);
	#endif

	table->resizeColumnsToContents();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
