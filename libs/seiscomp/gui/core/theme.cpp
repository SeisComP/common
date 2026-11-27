/***************************************************************************
 * Copyright (C) Daniel Siervo                                              *
 * Author: Daniel Siervo <emetdan@gmail.com>                                 *
 *                                                                         *
 * This file is distributed under the GNU Affero General Public License,    *
 * version 3.0, as described in the repository LICENSE.md.                   *
 ***************************************************************************/

#include "theme.h"

namespace Seiscomp::Gui::Theme {

bool isValid(const QString &name) {
	return name == "system" || name == "light" || name == "dark" || name == "midnight";
}

// The caller supplies the platform palette for system, Fusion for light.
QPalette palette(const QString &name, const QPalette &base) {
	QPalette result(base);
	if ( name == "dark" ) {
		result.setColor(QPalette::Window, QColor("#222020"));
		result.setColor(QPalette::Base, QColor("#3D3D3D"));
		result.setColor(QPalette::AlternateBase, QColor("#5C5B5A"));
		result.setColor(QPalette::WindowText, QColor("#E7E4E0"));
		result.setColor(QPalette::Text, QColor("#E7E4E0"));
		result.setColor(QPalette::Button, QColor("#424245"));
		result.setColor(QPalette::ButtonText, QColor("#E7E4E0"));
		result.setColor(QPalette::Dark, QColor("#3D3D3D"));
		result.setColor(QPalette::Mid, QColor("#5E5C5B"));
		result.setColor(QPalette::Light, QColor("#5E5C5B"));
		result.setColor(QPalette::Midlight, QColor("#424245"));
		result.setColor(QPalette::Shadow, QColor("#3D3D3D"));
		result.setColor(QPalette::Highlight, QColor("#12608A"));
		result.setColor(QPalette::HighlightedText, QColor("#E5EDF7"));
		result.setColor(QPalette::Link, QColor("#66CCFF"));
		result.setColor(QPalette::LinkVisited, QColor("#B69ADE"));
		result.setColor(QPalette::ToolTipBase, QColor("#424245"));
		result.setColor(QPalette::ToolTipText, QColor("#E7E4E0"));
		result.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#808080"));
		result.setColor(QPalette::Disabled, QPalette::Text, QColor("#808080"));
		result.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#808080"));
		result.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor("#808080"));
		result.setColor(QPalette::Disabled, QPalette::ToolTipText, QColor("#808080"));
	}
	if ( name == "midnight" ) {
		result.setColor(QPalette::Window, QColor("#151E29"));
		result.setColor(QPalette::Base, QColor("#101923"));
		result.setColor(QPalette::AlternateBase, QColor("#152131"));
		result.setColor(QPalette::WindowText, QColor("#C7D5E6"));
		result.setColor(QPalette::Text, QColor("#C7D5E6"));
		result.setColor(QPalette::Button, QColor("#1D2B3C"));
		result.setColor(QPalette::ButtonText, QColor("#C7D5E6"));
		result.setColor(QPalette::Dark, QColor("#101923"));
		result.setColor(QPalette::Mid, QColor("#354A62"));
		result.setColor(QPalette::Light, QColor("#354A62"));
		result.setColor(QPalette::Midlight, QColor("#1D2B3C"));
		result.setColor(QPalette::Shadow, QColor("#101923"));
		result.setColor(QPalette::Highlight, QColor("#294763"));
		result.setColor(QPalette::HighlightedText, QColor("#E5EDF7"));
		result.setColor(QPalette::Link, QColor("#8BAED1"));
		result.setColor(QPalette::LinkVisited, QColor("#B69ADE"));
		result.setColor(QPalette::ToolTipBase, QColor("#1D2B3C"));
		result.setColor(QPalette::ToolTipText, QColor("#C7D5E6"));
		result.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#586B82"));
		result.setColor(QPalette::Disabled, QPalette::Text, QColor("#586B82"));
		result.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#586B82"));
		result.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor("#586B82"));
		result.setColor(QPalette::Disabled, QPalette::ToolTipText, QColor("#586B82"));
	}
	return result;
}

// Defaults only: Application subsequently reads scheme.* user overrides.
void apply(Scheme::Colors &colors, const QString &name) {
	if ( name == "dark" ) {
		colors.picks.manual = QColor("#B3E7C3");
		colors.arrivals.manual = QColor("#83CDA4");
		colors.records.foreground = QColor("#C4CBD2");
		colors.records.alternateForeground = QColor("#B7C0CA");
		colors.records.gridPen.setColor(QColor("#727A84"));
		colors.picks.automatic = QColor("#B9A0E8");
		colors.arrivals.automatic = QColor("#FF7F73");
		colors.arrivals.theoretical = QColor("#66CCFF");
	}
	if ( name == "midnight" ) {
		colors.records.background = QColor("#101720");
		colors.records.alternateBackground = QColor("#131D28");
		colors.records.foreground = QColor("#B9C7D6");
		colors.records.alternateForeground = QColor("#A8BBCE");
		colors.records.gridPen.setColor(QColor("#42566E"));
		colors.records.subGridPen.setColor(QColor("#00000000"));
		colors.records.offset.setColor(QColor("#547089"));
		colors.records.alignment = QColor("#8BAED1");
		colors.records.gaps = QColor("#40E5BA70");
		colors.records.overlaps = QColor("#40B69ADE");
		colors.records.states.unrequested = QColor("#88101720");
		colors.records.states.requested = QColor("#24E5BA70");
		colors.records.states.inProgress = QColor("#1C73BED1");
		colors.records.states.notAvailable = QColor("#2CEC8D8D");
		colors.recordView.selectedTraceZoom = QColor("#386083AA");
		colors.picks.manual = QColor("#B3E7C3");
		colors.arrivals.manual = QColor("#83CDA4");
		colors.picks.automatic = QColor("#B69ADE");
		colors.arrivals.automatic = QColor("#EC8D8D");
		colors.arrivals.theoretical = QColor("#73BED1");
		colors.picks.undefined = QColor("#98A6BA");
		colors.arrivals.undefined = QColor("#98A6BA");
		colors.picks.disabled = QColor("#56677C");
		colors.arrivals.disabled = QColor("#56677C");
		colors.magnitudes.set = QColor("#E5BA70");
		colors.magnitudes.unset = QColor("#98A6BA");
		colors.magnitudes.disabled = QColor("#56677C");
		colors.stations.text = QColor("#D3DFED");
		colors.stations.associated = QColor("#69AFA8");
		colors.stations.disabled = QColor("#56677C");
	}
}

} // namespace Seiscomp::Gui::Theme
