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


#ifndef SEISCOMP_CONFIGURATION_GUI_H
#define SEISCOMP_CONFIGURATION_GUI_H


#ifndef Q_MOC_RUN
#include <seiscomp/system/model.h>
#endif

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QItemDelegate>
#include <QMainWindow>
#include <QListWidget>
#include <QToolBar>
#include <QTreeView>
#include <QLabel>
#include <QTimer>
#include <QSettings>
#include <QLayout>


class QTreeWidget;
class QTreeWidgetItem;


inline void setMargin(QLayout *layout, int margin) {
	layout->setContentsMargins(margin, margin, margin, margin);
}

QColor blend(const QColor &c1, const QColor &c2, int percentOfC1);


const QColor infoBackground(198, 229, 250);
const QColor infoForeground(25, 149, 202);

const QColor successBackground(224, 240, 197);
const QColor successForeground(136, 163, 29);

const QColor warningBackground(255, 247, 180);
const QColor warningForeground(181, 129, 5);

const QColor errorBackground(253, 215, 199);
const QColor errorForeground(219, 87, 40);


class ConfigurationTreeItemModel : public QStandardItemModel {
	Q_OBJECT

	public:
		enum ItemType {
			TypeNone = 0,
			TypeBindings,
			TypeModule,
			TypeNetwork,
			TypeStation,
			TypeBinding,
			TypeCategory,
			TypeCategoryBinding,
			TypeSection,
			TypeGroup,
			TypeStruct,
			TypeParameter
		};

		enum UserData {
			Initial = Qt::UserRole + 1,
			Level = Qt::UserRole + 2,
			Link  = Qt::UserRole + 3,
			Type  = Qt::UserRole + 4
		};


	public:
		ConfigurationTreeItemModel(QObject *parent,
		                           Seiscomp::System::Model * = NULL,
		                           Seiscomp::Environment::ConfigStage = Seiscomp::Environment::CS_CONFIG_APP);

	public:
		void setModel(Seiscomp::System::Model *, Seiscomp::Environment::ConfigStage);
		Seiscomp::System::Model *model() const { return _model; }

		void setModel(Seiscomp::System::ModuleBinding *);

		//! Returns whether the model is modified or not
		bool isModified() const { return _modified; }
		void saved();


	public:
		Seiscomp::Environment::ConfigStage configStage() const;

		bool setData(const QModelIndex &, const QVariant &, int role = Qt::EditRole) override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;


	public slots:
		//! Sets the modified state
		void setModified(bool m = true);


	signals:
		void modificationChanged(bool changed);

	private:
		void reset(QModelIndex idx);
		void updateDerivedParameters(
			const QModelIndex &, Seiscomp::System::Parameter*,
			Seiscomp::System::SymbolMapItem *);

	private:
		Seiscomp::Environment::ConfigStage _configStage;
		Seiscomp::System::Model           *_model;
		bool                               _modified;
};


class ConfiguratorPanel : public QWidget {
	Q_OBJECT

	public:
		ConfiguratorPanel(bool usesExternalConfiguration, QWidget *parent)
		: QWidget(parent), _model(NULL),
		  _usesExternalConfiguration(usesExternalConfiguration) {}

	public:
		virtual void setModel(ConfigurationTreeItemModel *model);
		virtual void saved();

		//! Event handler that can be reimplemented in derived classes. It
		//! is called whenever a panel is activated and shown.
		virtual void activated() {}

		bool isExternalConfigurationUsed() const { return _usesExternalConfiguration; }

		QString title() const { return _name; }

		void setHeadline(QString head, QString sub = {});
		QString headline() const { return _headline; }
		QString subHeadline() const { return _subHeadline; }

		void setDescription(QString desc);
		QString description() const { return _description; }

		QIcon icon() const { return _icon; }


	signals:
		void reloadRequested();
		void headlineChanged(QString head, QString sub);
		void descriptionChanged(QString desc);


	protected:
		ConfigurationTreeItemModel *_model;
		QString                     _name;
		QString                     _headline;
		QString                     _subHeadline;
		QString                     _description;
		QIcon                       _icon;
		bool                        _usesExternalConfiguration;
};


class StatusLabel : public QLabel {
	public:
		StatusLabel(QWidget *parent = nullptr);

		void setInfoText(const QString &);
		void setSuccessText(const QString &);
		void setWarningText(const QString &);
		void setErrorText(const QString &);

	protected:
		void paintEvent(QPaintEvent *) override;

	private:
		void setIcon(const QString &name);
		void setColors(QColor color, QColor background);

	private:
		QPixmap _icon;
};


class Configurator : public QMainWindow {
	Q_OBJECT

	public:
		Configurator(Seiscomp::Environment::ConfigStage stage,
		             QWidget *parent = nullptr);
		~Configurator();

		bool setModel(Seiscomp::System::Model *model);


	protected:
		void changeEvent(QEvent *event) override;
		void showEvent(QShowEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	private:
		void applyTheme();
		void updateModeLabel();

	private slots:
		void wizard();
		void resetAll();
		void save();
		void showAbout();
		void showHelp();
		void sectionChanged(QListWidgetItem*, QListWidgetItem*);
		void panelHeadlineChanged(const QString &text, const QString &subText);
		void panelDescriptionChanged(const QString &);

		void switchToSystemMode();
		void switchToUserMode();

		void switchMode();
		void showStatusMessage(const QString &msg);
		void showWarningMessage(const QString &msg);

		void statusTimer();


	private:
		QSettings                   _settings;
		QToolBar                   *_toolBar;

		typedef QPair<QListWidgetItem*,ConfiguratorPanel*> Panel;
		ConfigurationTreeItemModel *_model;
		QSortFilterProxyModel      *_proxy;

		QList<Panel>                _panels;

		QWidget                    *_modeLabel;
		QLabel                     *_headline;
		QLabel                     *_description;
		QListWidget                *_listWidget;
		StatusLabel                *_statusLabel;

		QListWidgetItem            *_infoItem;
		QListWidgetItem            *_configItem;
		QListWidgetItem            *_inventoryItem;
		QListWidgetItem            *_bindingsItem;

		QWidget                    *_issueLog;

		bool                        _firstShow;

		QTimer                      _statusTimer;

		Seiscomp::Environment::ConfigStage _configurationStage;
};



#endif
