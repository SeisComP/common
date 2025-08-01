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


#ifndef SEISCOMP_GUI_CORE_ICON_H
#define SEISCOMP_GUI_CORE_ICON_H


#include <QIcon>
#include <QPixmap>


namespace Seiscomp::Gui {


/**
 * @brief Returns an icon by name.
 *
 * Icons will be looked up as SVG resources with path :/sc/icons/{name}.svg.
 *
 * The returned icon gets a dedicated QIconEngine which will render the icon
 * in a given colour or using the application palette for the different states.
 *
 * Rendering the icon in a given colour involves blending a coloured quad with
 * blending mode SourceAtop on the rendered icon. This will effectively render
 * the icon single coloured. As Qt does not support currentColor with SVG yet,
 * there is no other option. Multi-coloured icon are a special case anyway and
 * not easy to fit with all possible application palettes, e.g. dark mode.
 *
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @return The icon possibly invalid if an invalid name was given.
 */
QIcon icon(QString name);

/**
 * @brief Convenience function which takes a primarly color.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param cOnOff The primary color of the icon. This will ignore the application palette.
 * @return The icon possibly invalid if an invalid name was given.
 */
QIcon icon(QString name, const QColor &cOnOff);

/**
 * @brief Convenience function which takes an on and an off color.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param cOn The color of the icon for state = On.
 * @param cOff The color of the icon for state = Off.
 * @return The icon possibly invalid if an invalid name was given.
 */
QIcon icon(QString name, const QColor &cOn, const QColor &cOff);

/**
 * @brief Returns an icon pixmap by name.
 *
 * This is a convenience function to extrace a pixmap from an icon for a given
 * font metrics.
 *
 * @param fm The fontMetrics used to calculate the icon size.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param scale An optional size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QFontMetrics &fm, const QString &name, double scale = 1.0);

/**
 * @brief Returns an icon pixmap by name.
 *
 * This is a convenience function to extrace a pixmap from an icon for a given
 * widget in its font size.
 *
 * @param parent The parent widget to extract the pixmap for.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param scale An optional size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QWidget *parent, const QString &name, double scale = 1.0);

/**
 * @brief Convenience function which takes a primarly color.
 * @param fm The fontMetrics used to calculate the icon size.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param cOnOff The primary color of the icon. This will ignore the application palette.
 * @param scale An optional size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &cOnOff, double scale = 1.0);

/**
 * @brief Convenience function which takes a primarly color.
 * @param parent The parent widget to extract the pixmap for.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param cOnOff The primary color of the icon. This will ignore the application palette.
 * @param scale An optional size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &cOnOff, double scale = 1.0);

/**
 * @brief Convenience function which takes an on and an off color.
 * @param fm The fontMetrics used to calculate the icon size.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param cOn The color of the icon for state = On.
 * @param cOff The color of the icon for state = Off.
 * @param scale An optional size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &cOn, const QColor &cOff, double scale = 1.0);

/**
 * @brief Convenience function which takes an on and an off color.
 * @param parent The parent widget to extract the pixmap for.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param cOn The color of the icon for state = On.
 * @param cOff The color of the icon for state = Off.
 * @param scale An optional size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &cOn, const QColor &cOff, double scale = 1.0);

/**
 * @brief Convenience function which extracts a pixmap from a given icon.
 * @param fm The fontMetrics used to calculate the icon size.
 * @param icon
 * @param scale An optional size scaling factor.
 * @param mode
 * @param state
 * @return The pixmap possibly invalid if an invalid icon was given.
 */
QPixmap pixmap(const QFontMetrics &fm, const QIcon &icon,
               double scale = 1.0,
               QIcon::Mode mode = QIcon::Normal,
               QIcon::State state = QIcon::Off);

/**
 * @brief Convenience function which extracts a pixmap from a given icon.
 * @param parent The parent widget to extract the pixmap for.
 * @param icon
 * @param scale An optional size scaling factor.
 * @param mode
 * @param state
 * @return The pixmap possibly invalid if an invalid icon was given.
 */
QPixmap pixmap(const QWidget *parent, const QIcon &icon,
               double scale = 1.0,
               QIcon::Mode mode = QIcon::Normal,
               QIcon::State state = QIcon::Off);


}


#endif
