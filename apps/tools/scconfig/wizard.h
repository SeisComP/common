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


#ifndef SEISCOMP_CONFIGURATION_GUI_WIZARD_H__
#define SEISCOMP_CONFIGURATION_GUI_WIZARD_H__


#ifndef Q_MOC_RUN
#include <seiscomp/system/schema.h>
#endif

#include <QDialog>
#include <QProcess>
#include <QStack>
#include <QWidget>

#include <string>
#include <list>
#include <map>


class QFrame;
class QLabel;
class QTextEdit;

typedef std::list<Seiscomp::System::SchemaSetupGroup*> SetupGroups;
typedef std::map<std::string, SetupGroups> WizardModel;


class WizardPage : public QWidget {
	public:
		WizardPage(QWidget *parent = 0);

		void setTitle(const QString &title);
		void setSubTitle(const QString &subtitle);

		const QString &title() const;
		const QString &subtitle() const;


	private:
		QString _title;
		QString _subtitle;
};



class WizardWidget : public QDialog {
	Q_OBJECT

	public:
		WizardWidget(WizardModel *model, QWidget *parent = NULL);
		~WizardWidget();

		void reject();

		//! Returns whether a process has started or not
		//! to trigger reload of configuration files downstream.
		bool ranSetup() const;


	private slots:
		void back();
		void next();
		void finish();

		void textChanged(const QString &text);
		void checkStateChanged(bool);
		void radioStateChanged(bool);

		void readProcStdOut();
		void readProcStdErr();
		void finishedProc(int, QProcess::ExitStatus);

	private:
		void setPage(WizardPage *p);

		WizardPage *createIntroPage();
		WizardPage *createExtroPage();
		WizardPage *createOutputPage();
		WizardPage *createCurrentPage();

	private:
		typedef Seiscomp::System::SchemaSetupInput Input;
		typedef Seiscomp::System::SchemaSetupGroup Group;

		typedef WizardModel::iterator ModelIterator;
		typedef std::vector<Seiscomp::System::SchemaSetupInputPtr> Inputs;

		struct Node {
			Node(Node *p, Input *i, Node *next = nullptr);

			~Node();

			// Parent node
			Node         *parent{nullptr};
			// Right sibling
			Node         *next{nullptr};
			// First child node
			Node         *child{nullptr};

			// Selected child if it is a choice
			Node         *activeChild{nullptr};

			QString       modname;
			Group        *group{nullptr};
			Input        *input{nullptr};
			QString       value;
			QString       path;
			QString       optionValue;
			bool          isOption{false};
		};

		void addGroups(Node *, const QString &modname, const SetupGroups &);
		void addInputs(Node *, const QString &modname, Group *g,
		               const Inputs &, const QString &path);
		void dumpNode(Node *node, int level = 0);

		WizardModel   *_model;
		Node          *_modelTree;
		QStack<Node*>  _path;

		QLabel        *_titleLabel;
		QLabel        *_subtitleLabel;
		QFrame        *_headerBreak;
		QWidget       *_header;
		QWidget       *_content;
		QLayout       *_contentLayout;
		QPushButton   *_buttonBack;
		QPushButton   *_buttonNext;
		QPushButton   *_buttonCancel;

		Node          *_currentNode;
		WizardPage    *_currentPage;
		QWidget       *_currentInput;

		QProcess      *_procSeisComP;
		QTextEdit     *_logPanel;
		QLabel        *_procStatus;

		bool           _ranSetup;

	friend QByteArray &operator<<(QByteArray&, Node&);
	friend QByteArray &operator<<(QByteArray&, Node*);
};


#endif
