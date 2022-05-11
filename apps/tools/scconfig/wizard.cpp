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

#include "wizard.h"
#include <seiscomp/config/config.h>
#include <seiscomp/system/environment.h>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QToolBar>

#include <iostream>
#include <sstream>


#define DRY_RUN 0


using namespace std;


WizardWidget::Node::Node(Node *p, Input *i, Node *next)
: parent(p), next(next), input(i) {}


WizardWidget::Node::~Node() {
	while ( child ) {
		Node *n = child;
		child = child->next;
		delete n;
	}
}


WizardPage::WizardPage(QWidget *parent) : QWidget(parent) {}


void WizardPage::setTitle(const QString &title) {
	_title = title;
}


void WizardPage::setSubTitle(const QString &subtitle) {
	_subtitle = subtitle;
}


const QString &WizardPage::title() const {
	return _title;
}


const QString &WizardPage::subtitle() const {
	return _subtitle;
}


WizardWidget::WizardWidget(WizardModel *model, QWidget *parent)
: QDialog(parent), _model(model) {
	resize(500,400);
	setWindowTitle(tr("SeisComP: Setup wizard"));

	_currentInput = nullptr;

	_ranSetup = false;
	_procSeisComP = nullptr;
	_currentPage = nullptr;

	_buttonBack = new QPushButton(tr("Back"));
	_buttonNext = new QPushButton(tr("Next"));
	_buttonCancel = new QPushButton(tr("Cancel"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->setMargin(6);
	buttonLayout->addStretch();
	buttonLayout->addWidget(_buttonBack);
	buttonLayout->addWidget(_buttonNext);
	buttonLayout->addWidget(_buttonCancel);

	_header = new QWidget;
	_header->setBackgroundRole(QPalette::Base);
	_header->setAutoFillBackground(true);

	_titleLabel = new QLabel;
	_subtitleLabel = new QLabel;

	QVBoxLayout *headerLayout = new QVBoxLayout;
	headerLayout->addWidget(_titleLabel);
	headerLayout->addWidget(_subtitleLabel);
	headerLayout->setMargin(fontMetrics().ascent());
	_header->setLayout(headerLayout);
	_header->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	_content = new QWidget;
	_contentLayout = new QVBoxLayout;
	_contentLayout->setMargin(fontMetrics().ascent());
	_content->setLayout(_contentLayout);

	_headerBreak = new QFrame;
	_headerBreak->setFrameShape(QFrame::HLine);

	QFrame *hrule = new QFrame;
	hrule->setFrameShape(QFrame::HLine);

	QVBoxLayout *pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(0);
	pageLayout->setMargin(0);
	pageLayout->addWidget(_header);
	pageLayout->addWidget(_headerBreak);
	pageLayout->addWidget(_content);
	pageLayout->addWidget(hrule);
	pageLayout->addLayout(buttonLayout);

	setLayout(pageLayout);

	setTabOrder(_buttonNext, _buttonCancel);
	setTabOrder(_buttonCancel, _buttonBack);

	_buttonNext->setDefault(true);

	connect(_buttonBack, SIGNAL(clicked()), this, SLOT(back()));
	connect(_buttonNext, SIGNAL(clicked()), this, SLOT(next()));
	connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	// Build model tree
	_modelTree = new Node(nullptr, nullptr);

	for ( ModelIterator it = _model->begin(); it != _model->end(); ++it ) {
		QString modname = it->first.c_str();
		addGroups(_modelTree, modname, it->second);
	}

#if DRY_RUN
	dumpNode(_modelTree);
#endif

	_modelTree->activeChild = _modelTree->child;

	_currentNode = _modelTree;
	setPage(createIntroPage());
}


WizardWidget::~WizardWidget() {
	if ( _modelTree ) delete _modelTree;
}


bool WizardWidget::ranSetup() const {
	return _ranSetup;
}


void WizardWidget::reject() {
	if ( _procSeisComP ) {
		if ( QMessageBox::question(this, tr("Abort setup"),
		                           tr("Setup is still running. Do you really\n"
		                              "want to abort the process? This is not\n"
		                              "recommended and can lead to undefined results."),
		                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No )
			return;
	}

	QDialog::reject();
}


void WizardWidget::addGroups(Node *parent, const QString &modname,
                             const SetupGroups &groups) {
	for ( Seiscomp::System::SchemaSetupGroup *group : groups ) {
		addInputs(parent, modname, group, group->inputs,
		          (group->name + ".").c_str());
	}
}


void WizardWidget::dumpNode(Node *node, int level) {
	for ( int i = 0; i < level; ++i ) cerr << " ";

	std::cerr << "[";
	if ( node->input ) {
		std::cerr << qPrintable(node->path);
	}
	else {
		std::cerr << "_";
	}
	if ( node->isOption )
		std::cerr << "*";
	if ( !node->optionValue.isEmpty() )
		std::cerr << " : " << qPrintable(node->optionValue);

	std::cerr << "]" << std::endl;
	Node *child = node->child;
	while ( child ) {
		dumpNode(child, level + 1);
		child = child->next;
	}
}


void WizardWidget::addInputs(Node *parent, const QString &modname,
                             Group *g, const Inputs &inputs,
                             const QString &path) {
	Node *childs = parent->child;
	if ( childs ) {
		while ( childs->next )
			childs = childs->next;
	}

	for ( const Seiscomp::System::SchemaSetupInputPtr &input : inputs ) {
		Node *n = new Node(parent, input.get());

		n->path = path + input->name.c_str();
		n->value = input->defaultValue.c_str();
		n->modname = modname;
		n->group = g;
		n->isOption = !input->options.empty();

		if ( !childs ) {
			childs = n;
			parent->child = childs;
		}
		else {
			childs->next = n;
			childs = childs->next;
		}

		Node *options = n->child;
		for ( const Seiscomp::System::SchemaSetupInputOptionPtr &option : input->options ) {
			Node *optionNode = new Node(n, input.get());

			optionNode->path = n->path + "." + option->value.c_str();
			optionNode->modname = modname;
			optionNode->group = g;
			optionNode->isOption = false;
			optionNode->optionValue = option->value.c_str();

			if ( !options ) {
				options = optionNode;
				n->child = options;
			}
			else {
				options->next = optionNode;
				options = options->next;
			}

			addInputs(optionNode, modname, g, option->inputs, n->path + ".");
		}
	}
}


void WizardWidget::back() {
	_buttonCancel->setText(tr("Cancel"));
	_buttonNext->setText(tr("Next"));
	_buttonNext->setProperty("finished", QVariant());
	_buttonNext->setEnabled(true);

	_currentNode = _path.pop();
	if ( _currentNode == _modelTree )
		setPage(createIntroPage());
	else
		setPage(createCurrentPage());
}


void WizardWidget::next() {
	if ( _buttonNext->property("finished").toBool() == true ) {
		finish();
		return;
	}

	if ( _currentNode == _modelTree ) {
		_path.push(_currentNode);
		_currentNode = _currentNode->child;
		setPage(createCurrentPage());
		return;
	}

	_currentNode->activeChild = nullptr;
	_path.push(_currentNode);

	// Choice input?
	if ( _currentNode->isOption ) {
		Node *child = _currentNode->child;
		for ( ; child; child = child->next ) {
			if ( child->optionValue == _currentNode->value ) {
				if ( child->child ) {
					_currentNode->activeChild = child;
					_currentNode = child->child;
					setPage(createCurrentPage());
					return;
				}

				break;
			}
		}
	}

	// Goto next node
	Node *next = _currentNode->next;

	while ( !next && _currentNode->parent ) {
		_currentNode = _currentNode->parent;
		if ( !_currentNode->optionValue.isEmpty() )
			continue;
		next = _currentNode->next;
	}

	if ( !next ) {
		setPage(createExtroPage());
		_buttonNext->setProperty("finished", true);
		_buttonNext->setText(tr("Finish"));
	}
	else {
		_currentNode = next;
		setPage(createCurrentPage());
	}
}


QByteArray &operator<<(QByteArray &ar, WizardWidget::Node *n) {
	if ( n->input ) {
		ar.append(n->modname);
		ar.append(".");
		ar.append(n->path);
		ar.append(" = ");

		stringstream ss;
		Seiscomp::Config::Symbol sym;
		sym.values.push_back(n->value.toStdString());
		Seiscomp::Config::Config::writeValues(ss, &sym);

		ar.append(ss.str().c_str());
		ar.append("\n");
	}

	if ( n->activeChild ) {
		if ( n->isOption )
			ar << n->activeChild->child;
		else
			ar << n->activeChild;
	}

	if ( n->next )
		ar << n->next;

	return ar;
}


QByteArray &operator<<(QByteArray &ar, WizardWidget::Node &n) {
	return ar << &n;
}


void WizardWidget::finish() {
	setPage(createOutputPage());

#if !DRY_RUN
	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	_procSeisComP = new QProcess(this);
	_procSeisComP->start(QString("%1/bin/seiscomp --stdin setup")
	                     .arg(env->installDir().c_str()),
	                     QProcess::Unbuffered | QProcess::ReadWrite);
	if ( !_procSeisComP->waitForStarted() ) {
		cerr << "Failed to start 'seiscomp'" << endl;
		delete _procSeisComP;
		_procSeisComP = nullptr;
		return;
	}
#endif

	_ranSetup = true;

	_buttonBack->setEnabled(false);
	_buttonNext->setEnabled(false);

	QByteArray data;
	data << _modelTree;

#if !DRY_RUN
	_procSeisComP->write(data);
	_procSeisComP->closeWriteChannel();
	_procSeisComP->setReadChannel(QProcess::StandardOutput);

	connect(_procSeisComP, SIGNAL(readyReadStandardOutput()),
	        this, SLOT(readProcStdOut()));
	connect(_procSeisComP, SIGNAL(readyReadStandardError()),
	        this, SLOT(readProcStdErr()));
	connect(_procSeisComP, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this, SLOT(finishedProc(int, QProcess::ExitStatus)));
#else
	std::cerr << data.toStdString() << std::endl;
	finishedProc(0, QProcess::NormalExit);
#endif
}


void WizardWidget::setPage(WizardPage *p) {
	if ( _currentPage ) delete _currentPage;

	_currentPage = p;

	_buttonBack->setEnabled(!_path.isEmpty());

	_titleLabel->setText(_currentPage->title());
	_subtitleLabel->setText(_currentPage->subtitle());

	QFont f = _titleLabel->font();
	f.setBold(true);

	if ( _currentPage->subtitle().isEmpty() ) {
		f.setPointSize(font().pointSize()*150/100);
		_headerBreak->setVisible(false);
		_subtitleLabel->setVisible(false);
		_content->setBackgroundRole(QPalette::Base);
		_content->setAutoFillBackground(true);
	}
	else {
		_headerBreak->setVisible(true);
		_subtitleLabel->setVisible(true);
		_content->setBackgroundRole(QPalette::NoRole);
		_content->setAutoFillBackground(false);
	}

	_titleLabel->setFont(f);

	_contentLayout->addWidget(_currentPage);

	if ( _currentInput ) _currentInput->setFocus();
	_currentInput = nullptr;
}


WizardPage *WizardWidget::createIntroPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(tr("Introduction"));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	QLabel *text = new QLabel;
	text->setWordWrap(true);
	text->setText(tr("This wizard will guide you through the steps "
	                 "of the initial setup. Use the back and next "
	                 "buttons to navigate through the pages and "
	                 "press cancel to close this wizard without "
	                 "implications to your configuration."));

	l->addWidget(text);
	l->addStretch();

	return w;
}


WizardPage *WizardWidget::createExtroPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(tr("Finished"));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	QLabel *text = new QLabel;
	text->setWordWrap(true);
	text->setText(tr("All setup questions have been answered. "
	                 "You can now go back again to correct settings "
	                 "or press 'Finish' to create the configuration."));

	l->addWidget(text);
	l->addStretch();

	return w;
}


WizardPage *WizardWidget::createOutputPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(tr("Setup system"));
	w->setSubTitle(tr("Running seiscomp setup"));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	_procStatus = new QLabel;
	l->addWidget(_procStatus);
	_procStatus->setText(tr("Waiting ..."));

	_logPanel = new QTextEdit;
	_logPanel->setReadOnly(true);
	_logPanel->setLineWrapMode(QTextEdit::NoWrap);
	l->addWidget(_logPanel);

	return w;
}


WizardPage *WizardWidget::createCurrentPage() {
	WizardPage *w = new WizardPage;

	w->setTitle(_currentNode->modname);
	w->setSubTitle(_currentNode->group->name.c_str());

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	_currentInput = nullptr;

	if ( _currentNode->input->type == "boolean" ) {
		QCheckBox *checkBox = new QCheckBox();
		checkBox->setText(_currentNode->input->text.c_str());
		checkBox->setChecked(_currentNode->value == "true");
		l->addWidget(checkBox);
		_currentInput = checkBox;

		connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(checkStateChanged(bool)));
	}
	else if ( _currentNode->input->options.empty() ) {
		QLabel *text = new QLabel;
		text->setText(_currentNode->input->text.c_str());
		l->addWidget(text);
		QLineEdit *edit = new QLineEdit;
		edit->setText(_currentNode->value);
		if ( _currentNode->input->echo == "password" )
			edit->setEchoMode(QLineEdit::Password);
		else if ( _currentNode->input->echo == "none" )
			edit->setEchoMode(QLineEdit::NoEcho);
		l->addWidget(edit);
		_currentInput = edit;

		connect(edit, SIGNAL(textChanged(const QString &)),
		        this, SLOT(textChanged(const QString &)));
	}
	else {
		QLabel *text = new QLabel;
		text->setText(_currentNode->input->text.c_str());
		l->addWidget(text);

		for ( size_t i = 0; i < _currentNode->input->options.size(); ++i ) {
			QRadioButton *radio = new QRadioButton;
			radio->setText(_currentNode->input->options[i]->value.c_str());
			radio->setToolTip(_currentNode->input->options[i]->description.c_str());
			if ( _currentNode->value == radio->text() )
				radio->setChecked(true);
			l->addWidget(radio);

			if ( i == 0 ) _currentInput = radio;

			connect(radio, SIGNAL(toggled(bool)), this, SLOT(radioStateChanged(bool)));
		}
	}

	l->addStretch();

	if ( !_currentNode->input->description.empty() ) {
		l->addSpacing(12);
		QLabel *desc = new QLabel;
		desc->setWordWrap(true);
		desc->setText(_currentNode->input->description.c_str());
		l->addWidget(desc);
	}

	/*
	QLabel *path = new QLabel;
	path->setWordWrap(true);
	path->setText(_currentNode->path);
	l->addWidget(path);
	QFont f = path->font();
	f.setItalic(true);
	path->setFont(f);
	*/

	return w;
}


void WizardWidget::textChanged(const QString &text) {
	_currentNode->value = text;
}


void WizardWidget::checkStateChanged(bool e) {
	_currentNode->value = e?"true":"false";
}


void WizardWidget::radioStateChanged(bool e) {
	if ( e )
		_currentNode->value = static_cast<QRadioButton*>(sender())->text();
}


void WizardWidget::readProcStdOut() {
	QString text = _procSeisComP->readAllStandardOutput();
	_logPanel->setTextColor(_logPanel->palette().color(QPalette::Text));
	_logPanel->insertPlainText(text);
}


void WizardWidget::readProcStdErr() {
	QString text = _procSeisComP->readAllStandardError();
	_logPanel->setTextColor(_logPanel->palette().color(QPalette::Disabled, QPalette::Text));
	_logPanel->insertPlainText(text);
}


void WizardWidget::finishedProc(int res, QProcess::ExitStatus stat) {
	delete _procSeisComP;
	_procSeisComP = nullptr;

	if ( res != 0 ) {
		QPalette pal = _procStatus->palette();
		pal.setColor(QPalette::WindowText, Qt::red);
		_procStatus->setPalette(pal);
		QFont f = _procStatus->font();
		f.setBold(true);
		_procStatus->setFont(f);
		_procStatus->setText(tr("Setup finished with exit code %1.").arg(res));
	}
	else {
		QPalette pal = _procStatus->palette();
		pal.setColor(QPalette::WindowText, Qt::darkGreen);
		_procStatus->setPalette(pal);
		QFont f = _procStatus->font();
		f.setBold(true);
		_procStatus->setFont(f);
		_procStatus->setText(tr("Setup ran successfully."));
	}

	_buttonBack->setEnabled(true);
	_buttonNext->setEnabled(false);
	_buttonCancel->setText(tr("Close"));
}
