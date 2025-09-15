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
 * Renders an icon in the requested size and optional color.
 *
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param size The requested size
 * @param dpr The device pixel ratio
 * @param color An optional color.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QString &name, const QSize &size, double dpr, const QColor &c = QColor());

/**
 * @brief Convenience function which takes a primarly color.
 * @param fm The fontMetrics used to calculate the icon size.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param color The primary color of the icon. This will ignore the application palette.
 * @param dpr The device pixel ratio
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QFontMetrics &fm, const QString &name, const QColor &color, double dpr);

/**
 * @brief Convenience function which takes a primarly color.
 * @param parent The parent widget to extract the pixmap for.
 * @param name The name of the icon which is effectively the basename of the SVG file.
 * @param color The primary color of the icon. This will ignore the application palette.
 * @param scale The size scaling factor.
 * @return The pixmap possibly invalid if an invalid name was given.
 */
QPixmap pixmap(const QWidget *parent, const QString &name, const QColor &color = QColor(), double scale = 1.0);


}


#endif
