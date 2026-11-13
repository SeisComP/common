/***************************************************************************
 * Copyright (C) Daniel Siervo                                              *
 * Author: Daniel Siervo <emetdan@gmail.com>                                 *
 *                                                                         *
 * This file is distributed under the GNU Affero General Public License,    *
 * version 3.0, as described in the repository LICENSE.md.                   *
 ***************************************************************************/

#ifndef SEISCOMP_GUI_THEME_H
#define SEISCOMP_GUI_THEME_H

#include <seiscomp/gui/core/scheme.h>
#include <QPalette>
#include <QString>

namespace Seiscomp::Gui::Theme {

/** @brief Return whether a name identifies a built-in appearance theme. */
SC_GUI_API bool isValid(const QString &name);

/**
 * @brief Construct a theme palette without modifying the supplied palette.
 * @param name A built-in theme name.
 * @param base The platform palette for system, or a Fusion palette otherwise.
 * @return The supplied palette for system/light, or a dark theme palette.
 */
SC_GUI_API QPalette palette(const QString &name, const QPalette &base);

/**
 * @brief Set scientific display defaults for a dark appearance theme.
 * @param colors The display colors to update.
 * @param name A built-in theme name. System/light leave colors unchanged.
 * Apply before Scheme::fetch() so explicit configuration takes precedence.
 */
SC_GUI_API void apply(Scheme::Colors &colors, const QString &name);

} // namespace Seiscomp::Gui::Theme
#endif
