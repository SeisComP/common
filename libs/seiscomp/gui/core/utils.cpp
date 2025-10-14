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

#define SEISCOMP_COMPONENT Gui::Utils

#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/utils.h>

#include <cstdio>
#include <cmath>
#include <stdexcept>

#include <boost/assign.hpp>

#include <QEvent>
#include <QLabel>
#include <QPainter>


namespace Seiscomp::Gui {


QChar degrees = (ushort)0x00b0;
std::string colorConvertError;


namespace {

int fromHexChar(char c) {
	if ( c >= '0' && c <= '9' ) {
		return c - '0';
	}

	if ( c >= 'a' && c <= 'f' ) {
		return (c - 'a') + 10;
	}

	if ( c >= 'A' && c <= 'F' ) {
		return (c - 'A') + 10;
	}

	throw std::invalid_argument("invalid hex character");
}

}


bool fromString(QColor& value, std::string_view str) {
	if ( str.empty() ) {
		colorConvertError = "invalid color: empty string";
		return false;
	}

	QColor tmp(QString::fromStdString(std::string(str)));
	if ( tmp.isValid() ) {
		value = tmp;
		return true;
	}

	int rgba[4] = { 0, 0, 0, 255 };
	bool ok = true;

	if ( str.substr(0, 4) == "rgb(" ) {
		if ( str[str.size() - 1] != ')' ) {
			colorConvertError = Core::stringify("invalid color %s: missing "
			                                    "closing bracket", str);
			return false;
		}
		str = str.substr(4, str.size() - 4 - 1);

		for ( int i = 0; i < 3; ++i ) {
			auto tok = Core::tokenize(str, ",");
			if ( !tok.data() ) {
				colorConvertError = Core::stringify("invalid color %s: rgb() "
				                                    "expects 3 components",
				                                    str);
				return false;
			}

			ok = Core::fromString(rgba[i], Core::trim(tok)) && ok;
		}

		if ( !ok ) {
			colorConvertError = Core::stringify("invalid color %s: wrong "
			                                    "format of component inside "
			                                    "rgb()", str);
			return false;
		}
	}
	else if ( str.substr(0, 5) == "rgba(" ) {
		if ( str[str.size() - 1] != ')' ) {
			colorConvertError = Core::stringify("invalid color %s: missing "
			                                    "closing bracket", str);
			return false;
		}
		str = str.substr(5, str.size() - 5 - 1);

		for ( auto &component : rgba ) {
			auto tok = Core::tokenize(str, ",");
			if ( !tok.data() ) {
				colorConvertError = Core::stringify("invalid color %s: rgba() "
				                                    "expects 4 components",
				                                    str);
				return false;
			}

			ok = Core::fromString(component, Core::trim(tok)) && ok;
		}

		if ( !ok ) {
			colorConvertError = Core::stringify("invalid color %s: wrong "
			                                    "format of component inside "
			                                    "rgba()", str);
			return false;
		}
	}
	else {
		if ( str.size() != 3 && str.size() != 4
		 && str.size() != 6 && str.size() != 8 ) {
			colorConvertError = Core::stringify("invalid color %s: expected 3, "
			                                    "4, 6 or 8 characters", str);
			return false;
		}

		try {
			switch ( str.size() ) {
				case 3:
					rgba[0] = (fromHexChar(str[0]) << 4) | fromHexChar(str[0]);
					rgba[1] = (fromHexChar(str[1]) << 4) | fromHexChar(str[1]);
					rgba[2] = (fromHexChar(str[2]) << 4) | fromHexChar(str[2]);
					break;
				case 4:
					rgba[0] = (fromHexChar(str[0]) << 4) | fromHexChar(str[0]);
					rgba[1] = (fromHexChar(str[1]) << 4) | fromHexChar(str[1]);
					rgba[2] = (fromHexChar(str[2]) << 4) | fromHexChar(str[2]);
					rgba[3] = (fromHexChar(str[3]) << 4) | fromHexChar(str[3]);
					break;
				case 6:
					rgba[0] = (fromHexChar(str[0]) << 4) | fromHexChar(str[1]);
					rgba[1] = (fromHexChar(str[2]) << 4) | fromHexChar(str[3]);
					rgba[2] = (fromHexChar(str[4]) << 4) | fromHexChar(str[5]);
					break;
				case 8:
					rgba[0] = (fromHexChar(str[0]) << 4) | fromHexChar(str[1]);
					rgba[1] = (fromHexChar(str[2]) << 4) | fromHexChar(str[3]);
					rgba[2] = (fromHexChar(str[4]) << 4) | fromHexChar(str[5]);
					rgba[3] = (fromHexChar(str[6]) << 4) | fromHexChar(str[7]);
					break;
				default:
					colorConvertError = Core::stringify("invalid color %s: "
					                                    "wrong format", str);
					return false;
			}
		}
		catch ( ... ) {
			colorConvertError = Core::stringify("invalid color %s: wrong "
			                                    "format", str);
			return false;
		}
	}

	value.setRed(rgba[0]);
	value.setGreen(rgba[1]);
	value.setBlue(rgba[2]);
	value.setAlpha(rgba[3]);

	return true;
}

QColor readColor(const std::string &query, const std::string &str,
                 const QColor &base, bool *ok) {
	QColor r;
	r.setNamedColor(str.c_str());
	if ( r.isValid() ) {
		if ( ok ) {
			*ok = true;
		}
		return r;
	}

	r = base;

	if ( fromString(r, str) ) {
		if ( ok ) {
			*ok = true;
		}
	}
	else {
		SEISCOMP_ERROR("%s: %s", query.c_str(), colorConvertError.c_str());
		if ( ok ) {
			*ok = false;
		}
	}

	return r;
}

Qt::PenStyle stringToPenStyle(const std::string &str) {
	static const std::map<std::string, Qt::PenStyle> styleNameMap =
		boost::assign::map_list_of<std::string, Qt::PenStyle>
		("customdashline", Qt::CustomDashLine)
		("dashdotdotline", Qt::DashDotDotLine)
		("dashdotline", Qt::DashDotLine)
		("dashline", Qt::DashLine)
		("dotline", Qt::DotLine)
		("nopen", Qt::NoPen)
		("solidline", Qt::SolidLine);
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return styleNameMap.at(lower);
}

Qt::PenStyle readPenStyle(const std::string &query, const std::string &str,
                          Qt::PenStyle base, bool *ok) {
	Qt::PenStyle r(base);

	try {
		r = stringToPenStyle(str);
		if ( ok ) {
			*ok = true;
		}
	}
	catch ( const std::out_of_range & ) {
		SEISCOMP_ERROR("%s: invalid pen style", query.c_str());
		if ( ok ) {
			*ok = false;
		}
	}

	return r;
}

Qt::BrushStyle stringToBrushStyle(const std::string &str) {
	static const std::map<std::string, Qt::BrushStyle> styleNameMap =
		boost::assign::map_list_of<std::string, Qt::BrushStyle>
		("solid", Qt::SolidPattern)
		("dense1", Qt::Dense1Pattern)
		("dense2", Qt::Dense2Pattern)
		("dense3", Qt::Dense3Pattern)
		("dense4", Qt::Dense4Pattern)
		("dense5", Qt::Dense5Pattern)
		("dense6", Qt::Dense6Pattern)
		("dense7", Qt::Dense7Pattern)
		("nobrush", Qt::NoBrush)
		("horizontal", Qt::HorPattern)
		("vertical", Qt::VerPattern)
		("cross", Qt::CrossPattern)
		("bdiag", Qt::BDiagPattern)
		("fdiag", Qt::FDiagPattern)
		("diagcross", Qt::DiagCrossPattern);
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return styleNameMap.at(lower);
}

Qt::BrushStyle readBrushStyle(const std::string &query, const std::string &str,
                              Qt::BrushStyle base, bool *ok) {
	Qt::BrushStyle r(base);

	try {
		r = stringToBrushStyle(str);
		if ( ok ) {
			*ok = true;
		}
	}
	catch ( const std::out_of_range & ) {
		SEISCOMP_ERROR("invalid brush style in %s: %s",
		               query.c_str(), str.c_str());
		if ( ok ) {
			*ok = false;
		}
	}

	return r;
}

QString latitudeToString(double lat, bool withValue, bool withUnit,
                         int precision) {
	if ( withValue && withUnit ) {
		return QString("%1%2 %3")
			.arg(fabs(lat), 0, 'f', precision)
			.arg(degrees)
			.arg(lat >= 0.0?"N":"S");
	}

	if ( !withValue ) {
		return QString("%1 %2")
			.arg(degrees)
			.arg(lat >= 0.0?"N":"S");
	}

	if ( !withUnit ) {
		return QString("%1")
			.arg(fabs(lat), 0, 'f', precision);
	}

	return {};
}

QString longitudeToString(double lon, bool withValue, bool withUnit,
                          int precision) {
	if ( withValue && withUnit ) {
		return QString("%1%2 %3")
			.arg(fabs(lon), 0, 'f', precision)
			.arg(degrees)
			.arg(lon >= 0.0?"E":"W");
	}

	if ( !withValue ) {
		return QString("%1 %2").arg(degrees, lon >= 0.0?"E":"W");
	}

	if ( !withUnit ) {
		return QString("%1").arg(fabs(lon), 0, 'f', precision);
	}

	return {};
}

QString depthToString(double depth, int precision) {
	return QString("%1")
		.arg(depth, 0, 'f', precision);
}


QString timeToString(const Core::Time &t, const char *fmt, bool addTimeZone) {
	QString s;

	if ( SCScheme.dateTime.useLocalTime ) {
		s = t.toLocalTime().toString(fmt).c_str();
		if ( addTimeZone ) {
			s += " ";
			s += Core::Time::LocalTimeZone().c_str();
		}
	}
	else {
		s = t.toString(fmt).c_str();
		if ( addTimeZone ) {
			s += " UTC";
		}
	}

	return s;
}


void timeToLabel(QLabel *label, const Core::Time &t, const char *fmt,
                 bool addTimeZone) {
	label->setToolTip((t.iso() + " UTC").c_str());
	label->setText(timeToString(t, fmt, addTimeZone));
}


QString elapsedTimeString(const Core::TimeSpan &dt) {
	int d{0};
	int h{0};
	int m{0};
	int s{0};
	QLatin1Char fill('0');
	dt.get(&d, &h, &m, &s);

	if ( d ) {
		return QString("O.T. +%1d %2h").arg(d,2).arg(h, 2, 10, fill);
	}

	if ( h ) {
		return QString("O.T. +%1h %2m").arg(h,2).arg(m, 2, 10, fill);
	}

	return QString("O.T. +%1m %2s").arg(m,2).arg(s, 2, 10, fill);
}


QString numberToEngineering(double value, int precision) {
	static const char* neg_units[] = {"m", "µ", "n", "p", "f", "a", "z", "y",
	                                  "r", "q"};
	static const char* pos_units[] = {"k", "M", "G", "T", "P", "E", "Z", "Y",
	                                  "R", "Q"};

	if ( value == 0 ) {
		return QString("%1 ").arg(value, 0, 'f', 1);
	}

	int fi = static_cast<int>(floor(log10(abs(value)) / 3));
	if ( !fi ) {
		return QString("%1 ").arg(value, 0, 'f', 1);
	}

	const char *prefix;

	if ( fi < 0 ) {
		if ( fi < -static_cast<int>(sizeof(neg_units) / sizeof(char*)) ) {
			fi = -static_cast<int>(sizeof(neg_units) / sizeof(char*));
		}
		prefix = neg_units[-fi - 1];
	}
	else {
		if ( fi > static_cast<int>(sizeof(pos_units) / sizeof(char*)) ) {
			fi = static_cast<int>(sizeof(pos_units) / sizeof(char*));
		}
		prefix = pos_units[fi - 1];
	}

	value /= pow(10, fi * 3);

	return QString("%1 %2").arg(value, 0, 'f', precision).arg(prefix);
}


double hypocentralDistance(double epicentral, double depth, double elev) {
	double hDist = Math::Geo::deg2km(epicentral); // [km]
	double vDist = depth + elev * 1E-3;           // [km]
	return Math::Geo::km2deg(sqrt(hDist * hDist + vDist * vDist));
}


double computeDistance(double lat1, double lon1, double depth1,
                       double lat2, double lon2, double elev2,
                       double *az, double *baz, double *epicentral) {
	double delta;
	Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, az, baz);
	if ( epicentral ) {
		*epicentral = delta;
	}
	if ( SCScheme.distanceHypocentral ) {
		delta = hypocentralDistance(delta, depth1, elev2);
	}
	return delta;
}


void setMaxWidth(QWidget *w, int numCharacters) {
	QFont f = w->font();
	QFontMetrics fm(f);
	w->setMaximumWidth(QT_FM_WIDTH(fm, "W")*numCharacters);
}


void fixWidth(QWidget *w, int numCharacters) {
	QFont f = w->font();
	QFontMetrics fm(f);
	w->setFixedWidth(QT_FM_WIDTH(fm, "W")*numCharacters);
}


void setBold(QWidget *w, bool bold) {
	QFont f = w->font();
	f.setBold(true);
	w->setFont(f);
}


void setItalic(QWidget *w, bool italic) {
	QFont f = w->font();
	f.setItalic(italic);
	w->setFont(f);
}


ElideFadeDrawer::ElideFadeDrawer(QObject *parent) : QObject(parent) {}

bool ElideFadeDrawer::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::Paint ) {
		auto *q = static_cast<QLabel*>(obj);
		QPainter painter(q);
		QFontMetrics fm(q->font());
		QRect rect = q->contentsRect();
		int flags = q->alignment() | Qt::TextSingleLine;

		if ( QT_FM_WIDTH(fm, q->text()) > rect.width() ) {
			//QPixmap pixmap(rect.size());//, QImage::Format_ARGB32);
			QImage image(rect.size() * qApp->devicePixelRatio(), QImage::Format_ARGB32_Premultiplied);
			image.setDevicePixelRatio(qApp->devicePixelRatio());
			image.fill(Qt::transparent);

			QPainter p(&image);
			p.setPen(painter.pen());
			p.setFont(painter.font());

			/*
			QLinearGradient gradient(rect.topLeft(), rect.topRight());
			QColor from = q->palette().color(QPalette::WindowText);
			QColor to = from;
			to.setAlpha(0);
			gradient.setColorAt(0.8, from);
			gradient.setColorAt(1.0, to);
			p.setPen(QPen(gradient, 0));
			*/
			p.drawText(rect, flags, q->text());

			QLinearGradient alphaGradient(rect.topLeft(), rect.topRight());
			alphaGradient.setColorAt(0.8, QColor(0,0,0,255));
			alphaGradient.setColorAt(1.0, QColor(0,0,0,0));
			p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
			p.fillRect(rect, alphaGradient);

			painter.drawImage(rect, image);
		}
		else {
			painter.setPen(q->palette().color(QPalette::WindowText));
			painter.drawText(rect, flags, q->text());
		}

		return true;
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}


EllipsisDrawer::EllipsisDrawer(QObject *parent) : QObject(parent) {}

bool EllipsisDrawer::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::Paint ) {
		auto *q = static_cast<QLabel*>(obj);
		QPainter painter(q);
		QFontMetrics fm(q->font());
		QRect rect = q->contentsRect();

		if ( QT_FM_WIDTH(fm, q->text()) > rect.width() ) {
			int eWidth = QT_FM_WIDTH(fm, "...");
			painter.drawText(rect.adjusted(0,0,-eWidth,0), Qt::TextSingleLine,
			                 q->text());
			painter.drawText(rect.adjusted(rect.width()-eWidth,0,0,0),
			                 Qt::TextSingleLine, "...");
		}
		else {
			painter.drawText(rect, Qt::TextSingleLine, q->text());
		}

		return true;
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}


QIcon iconFromURL(const QString &url) {
	if ( url.isEmpty() ) {
		return {};
	}

	QUrl urlParsed(url);
	auto scheme = urlParsed.scheme();
	if ( scheme == "qrc" ) {
		return QIcon(QString(":%1").arg(urlParsed.path()));
	}

	if ( scheme == "file" ) {
		return QIcon(urlParsed.path());
	}

	if ( scheme == "sc" ) {
		return icon(urlParsed.path());
	}

	return QIcon(url);
}


bool isDarkMode() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#else
	const QPalette defaultPalette;
	const auto text = defaultPalette.color(QPalette::WindowText);
	const auto window = defaultPalette.color(QPalette::Window);
	return text.lightness() > window.lightness();
#endif // QT_VERSION
}


const ColorTheme &currentColorTheme() {
	static ColorTheme colorTheme[2] = {
		{
			QColor(52, 131, 105), // green
			QColor(255, 116, 0),  // orange
			QColor(0, 136, 159),  // petrol
			QColor(31, 121, 181), // blue
			QColor(232, 45, 41),  // red
			QColor(249, 221, 220),// lightRed
			QColor(255, 255, 255) // white
		},
		{
			QColor(63, 162, 129), // green
			QColor(255, 139, 41), // orange
			QColor(0, 155, 178),  // petrol
			QColor(31, 141, 207), // blue
			QColor(232, 45, 41),  // red
			QColor(249, 221, 220),// lightRed
			QColor(255, 255, 255) // white
		}
	};

	return colorTheme[isDarkMode() ? 1 : 0];
}


} // ns Seiscomp::Gui
