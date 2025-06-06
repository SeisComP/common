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


#ifndef SEISCOMP_CONFIGURATION_EDITOR_H__
#define SEISCOMP_CONFIGURATION_EDITOR_H__


#ifndef Q_MOC_RUN
#include <seiscomp/system/model.h>
#endif

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextEdit>
#include <QWidget>


class QLineEdit;
class QListWidget;
class QSplitter;
class QTableWidget;

class ConfigHighlighter : public QSyntaxHighlighter {
	public:
		ConfigHighlighter(QTextDocument *parent = NULL);

		virtual void highlightBlock(const QString & text);

	private:
		struct Rule {
			QRegularExpression pattern;
			QTextCharFormat format;
		};

		QVector<Rule> _rules;

		QTextCharFormat _keywordFormat;
		QTextCharFormat _commentFormat;
		QTextCharFormat _quotationFormat;
		QTextCharFormat _variableFormat;
		QTextCharFormat _placeHolderFormat;
};


class ConfigEditor : public QTextEdit {
	Q_OBJECT

	public:
		ConfigEditor(QWidget *parent = NULL);
		ConfigEditor(const QString &text, QWidget *parent = NULL);

	public:
		int lineNumberAreaWidth();

		void setErrorLine(int line);
		void gotoLine(int line);

	protected:
		void resizeEvent(QResizeEvent *event);

	private slots:
		void updateExtraSelections();

		void updateLineNumberAreaWidth(int newBlockCount);
		void updateLineNumberArea(const QRect &, int);

		void layoutChanged(const QRectF &);
		void scrollBarValueChanged(int);

	private:
		void init();

	private:
		QWidget *_lineNumbers;
		int _errorLine;
};


class ConfigFileWidget : public QWidget {
	Q_OBJECT

	public:
		typedef QPair<int,QString> Error;

		ConfigFileWidget(QWidget *parent = NULL);

	public:
		ConfigEditor *editor() const;

		bool loadFile(const QString &filename);
		bool saveFile(const QString &filename);
		void setErrors(const QList<Error> &, bool selectFirst = false);

	private slots:
		void setError(int);

	private:
		ConfigEditor *_editor;
		QListWidget  *_errorlist;
		QSplitter    *_splitter;
};


class ConfigConflictWidget : public QWidget {
	Q_OBJECT

	public:
		ConfigConflictWidget(QWidget *parent = NULL);

	public:
		void setConflicts(const QList<Seiscomp::System::ConfigDelegate::CSConflict> &);

	public slots:
		void fixConflicts();

	private:
		QList<Seiscomp::System::ConfigDelegate::CSConflict> _conflicts;
		QListWidget *_list;
};


class ConfigChangesWidget : public QWidget {
	Q_OBJECT

	public:
		ConfigChangesWidget(QWidget *parent = NULL);

	public:
		void setChanges(const Seiscomp::System::ConfigDelegate::ChangeList &);

	private:
		QTableWidget *_table;
};


#endif
