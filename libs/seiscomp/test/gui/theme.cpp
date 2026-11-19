/***************************************************************************
 * Copyright (C) Daniel Siervo                                              *
 * Author: Daniel Siervo <emetdan@gmail.com>                                 *
 *                                                                         *
 * This file is distributed under the GNU Affero General Public License,    *
 * version 3.0, as described in the repository LICENSE.md.                   *
 ***************************************************************************/

#define SEISCOMP_TEST_MODULE SeisComP
#include <seiscomp/unittest/unittests.h>
#include "../../gui/core/theme.h"

using namespace Seiscomp::Gui;

BOOST_AUTO_TEST_SUITE(seiscomp_gui_theme)

BOOST_AUTO_TEST_CASE(theme_names) {
	for ( const auto *name : {"system", "light", "dark", "midnight"} ) {
		BOOST_CHECK(Theme::isValid(name));
	}
	BOOST_CHECK(!Theme::isValid("unknown"));
	BOOST_CHECK(!Theme::isValid(""));
}

BOOST_AUTO_TEST_CASE(platform_palette_is_preserved) {
	QPalette base;
	base.setColor(QPalette::Window, QColor("#123456"));
	BOOST_CHECK(Theme::palette("system", base) == base);
	BOOST_CHECK(Theme::palette("light", base) == base);
}

BOOST_AUTO_TEST_CASE(dark_palette_contrast) {
	for ( const auto *name : {"dark", "midnight"} ) {
		auto pal = Theme::palette(name, QPalette());
		BOOST_CHECK(pal.color(QPalette::WindowText).lightness()
		            > pal.color(QPalette::Window).lightness());
		BOOST_CHECK(pal.color(QPalette::Text).lightness()
		            > pal.color(QPalette::Base).lightness());
		BOOST_CHECK(pal.color(QPalette::Link).lightness()
		            > pal.color(QPalette::Window).lightness());
		BOOST_CHECK(pal.color(QPalette::Disabled, QPalette::Text)
		            != pal.color(QPalette::Active, QPalette::Text));
	}
	BOOST_CHECK(Theme::palette("dark", QPalette())
	            != Theme::palette("midnight", QPalette()));
}

BOOST_AUTO_TEST_SUITE_END()
