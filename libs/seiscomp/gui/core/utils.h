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



#ifndef SEISCOMP_GUI_UTIL_H
#define SEISCOMP_GUI_UTIL_H


#include <seiscomp/gui/qt.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/defs.h>
#include <QWidget>
#include <vector>


class QLabel;


namespace Seiscomp {
namespace Gui {


SC_GUI_API extern QChar degrees;

SC_GUI_API extern std::string colorConvertError;

SC_GUI_API bool fromString(QColor &value, const std::string &str);
SC_GUI_API QColor readColor(const std::string &query, const std::string &str,
                            const QColor &base, bool *ok = nullptr);

SC_GUI_API Qt::PenStyle stringToPenStyle(const std::string &str);
SC_GUI_API Qt::PenStyle readPenStyle(const std::string &query, const std::string &str,
                                     Qt::PenStyle base, bool *ok = nullptr);

SC_GUI_API Qt::BrushStyle stringToBrushStyle(const std::string &str);
SC_GUI_API Qt::BrushStyle readBrushStyle(const std::string &query, const std::string &str,
                                         Qt::BrushStyle base, bool *ok = nullptr);

SC_GUI_API QString latitudeToString(double lat, bool withValue = true, bool withUnit = true, int precision = 2);
SC_GUI_API QString longitudeToString(double lon, bool withValue = true, bool withUnit = true, int precision = 2);
SC_GUI_API QString depthToString(double depth, int precision = 0);
SC_GUI_API QString timeToString(const Core::Time &t, const char *fmt, bool addTimeZone = false);
SC_GUI_API void timeToLabel(QLabel *label, const Core::Time &t, const char *fmt, bool addTimeZone = false);
SC_GUI_API QString elapsedTimeString(const Core::TimeSpan &dt);

SC_GUI_API void setMaxWidth(QWidget *w, int numCharacters);
SC_GUI_API void fixWidth(QWidget *w, int numCharacters);

SC_GUI_API void setBold(QWidget *w, bool bold = true);
SC_GUI_API void setItalic(QWidget *w, bool italic = true);


class SC_GUI_API ElideFadeDrawer : public QObject {
	public:
		ElideFadeDrawer(QObject *parent = 0);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};


class SC_GUI_API EllipsisDrawer : public QObject {
	public:
		EllipsisDrawer(QObject *parent = 0);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};


template <typename T>
class ObjectChangeList : public std::vector<std::pair<typename Core::SmartPointer<T>::Impl, bool> > {
};


}
}


#endif
