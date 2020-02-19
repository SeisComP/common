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


#include <seiscomp/gui/core/osx.h>
#include <QMainWindow>
#import <Cocoa/Cocoa.h>

namespace Seiscomp {
namespace Gui {
namespace Mac {


bool isLion() {
	NSString *string = [NSString string];
	// this selector was added only in Lion. so we can check if it's responding, we are on Lion
	return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}


bool addFullscreen(QMainWindow *w) {
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
	if ( isLion() ) { // checks if lion is running
		NSView *nsview = (NSView *)w->winId();
		NSWindow *nswindow = [nsview window];
		[nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		return true;
	}
#endif
	return false;
}


}
}
}
