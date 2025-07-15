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


#ifndef SEISCOMP_GUI_CORE_SPINNINGLABEL_H
#define SEISCOMP_GUI_CORE_SPINNINGLABEL_H


#include <QLabel>
#include <QVariantAnimation>

#include <seiscomp/gui/qt.h>


namespace Seiscomp::Gui {


class SC_GUI_API SpinningLabel : public QLabel {
	Q_OBJECT
	Q_PROPERTY(int duration READ duration WRITE setDuration)
	Q_PROPERTY(QEasingCurve easingCurve READ easingCurve WRITE setEasingCurve)

	public:
		explicit SpinningLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

	public:
		/**
		 * @brief Starts the spinning animation if the label is shown or visible.
		 * This is the default state.
		 */
		void start();

		/**
		 * @brief Stops the spinning animation if the label is shown or visible.
		 */
		void stop();

		int duration() const;
		void setDuration(int msecs);

		QEasingCurve easingCurve() const;
		void setEasingCurve(const QEasingCurve &easing);

	protected:
		void showEvent(QShowEvent *event) override;
		void hideEvent(QHideEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

	private slots:
		void animationChanged(const QVariant &);

	protected:
		QVariantAnimation _animation;
		double            _angle;
		bool              _shouldRun{true};
};


}


#endif
