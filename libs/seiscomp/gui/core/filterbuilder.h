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


#ifndef SEISCOMP_GUI_FILTERBUILDER_H
#define SEISCOMP_GUI_FILTERBUILDER_H


#include <seiscomp/gui/qt.h>
#include <QDialog>
#include <QVariant>


class QCheckBox;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTimer;
class QToolButton;


namespace Seiscomp {
namespace Gui {


/**
 * @brief Dialog to compose a filter chain such as
 *        RMHP(10)>>ITAPER(30)>>BW(3,1,8) without typing the string.
 *
 * Stages are listed in processing order and can be reordered by drag and
 * drop. The parameters of the selected stage are edited with spin boxes.
 * The resulting string is shown and validated with the same parser that
 * is used to create the filter later.
 *
 * Besides single filters two composite stages are supported:
 * - A multi-band stage which combines Butterworth bandpasses of the same
 *   order, e.g. (BW(4,0.7,2)^2+BW(4,2,6)^2)^0.5.
 * - An expression stage which holds any other part of the filter grammar
 *   as text, e.g. |DIFF|*2.
 */
class SC_GUI_API FilterBuilderDialog : public QDialog {
	Q_OBJECT

	public:
		FilterBuilderDialog(QWidget *parent = nullptr,
		                    Qt::WindowFlags f = Qt::WindowFlags());

	public:
		void setFilterName(const QString &name);
		QString filterName() const;

		void setFilter(const QString &filter);
		QString filter() const;

		/**
		 * @brief Sets the sampling frequency of the data the filter is
		 *        applied to. If set, corner frequencies at or above the
		 *        Nyquist frequency are reported.
		 * @param fsamp The sampling frequency in Hz or 0 if unknown
		 */
		void setSamplingFrequency(double fsamp);

		//! Shows the "Apply live" option which emits filterChanged
		void setLiveApplyEnabled(bool enable);
		bool isLiveApplyActive() const;

		//! Shows the button to save the filter permanently
		void setSaveEnabled(bool enable);
		//! Returns whether the dialog was closed with the save button
		bool saveRequested() const;

		/**
		 * @brief Checks whether a filter string can be created.
		 * @param filter The filter string
		 * @param error Optional pointer to the error description
		 * @return true if valid, false otherwise
		 */
		static bool ValidateFilter(const QString &filter, QString *error = nullptr);


	signals:
		//! Emitted while "Apply live" is checked and the filter is valid
		void filterChanged(const QString &filter);


	private slots:
		void addStage(const QString &key);
		void removeStage();
		void moveStageUp();
		void moveStageDown();
		void currentStageChanged();
		void stageTypeChanged(int index);
		void stageParameterChanged();
		void expressionEdited(const QString &text);
		void multiBandChanged();
		void addBand();
		void removeBand();
		void filterStringEdited(const QString &text);
		void scheduleUpdate();
		void updateFromStages();
		void emitLiveFilter();
		void liveApplyToggled(bool);
		void saveClicked();


	private:
		QListWidgetItem *createStageItem(const QString &key,
		                                 const QVariantList &values);
		void updateStageItem(QListWidgetItem *item);
		void buildParameterEditor(QListWidgetItem *item);
		void buildExpressionEditor(const QVariantList &values);
		void buildMultiBandEditor(const QVariantList &values);
		void setStages(const QString &filter);
		void validate();


	private:
		QLineEdit    *_name;
		QListWidget  *_stages;
		QToolButton  *_btnAdd;
		QToolButton  *_btnRemove;
		QToolButton  *_btnUp;
		QToolButton  *_btnDown;
		QGroupBox    *_paramGroup;
		QFormLayout  *_paramLayout;
		QComboBox    *_stageType;
		QLineEdit    *_expression;
		QSpinBox     *_bandOrder;
		QComboBox    *_bandMode;
		QTableWidget *_bands;
		QLineEdit    *_filterString;
		QLabel       *_status;
		QCheckBox    *_liveApply;
		QPushButton  *_btnSave;
		QPushButton  *_btnOk;
		QTimer       *_updateTimer;
		QTimer       *_liveTimer;
		double        _fsamp;
		bool          _valid;
		bool          _saveRequested;
		bool          _blockUpdates;
};


}
}


#endif
