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


#define SEISCOMP_COMPONENT ImportBinary

#include "binary.h"
#include <seiscomp/logging/log.h>
#include <seiscomp/io/archive/binarchive.h>

#include <iostream>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace DataModel {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_IMPORTER_INTERFACE(ImporterBinary, "binary");
REGISTER_IMPORTER_INTERFACE(ImporterVBinary, "vbinary");
REGISTER_EXPORTER_INTERFACE(ExporterBinary, "binary");
REGISTER_EXPORTER_INTERFACE(ExporterVBinary, "vbinary");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::BaseObject *ImporterBinary::get(std::streambuf* buf) {
	IO::BinaryArchive ar;
	ar.open(buf);
	Core::BaseObject *obj = nullptr;
	try {
		ar >> obj;
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("%s", e.what());
		_hasErrors = true;
		obj = nullptr;
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::BaseObject *ImporterVBinary::get(std::streambuf* buf) {
	IO::VBinaryArchive ar;
	ar.open(buf);
	Core::BaseObject *obj = nullptr;
	try {
		ar >> obj;
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("%s", e.what());
		_hasErrors = true;
		obj = nullptr;
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExporterBinary::put(std::streambuf* buf, Core::BaseObject *obj) {
	IO::BinaryArchive ar;
	if ( !ar.create(buf) )
		return false;
	ar << obj;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExporterBinary::put(std::streambuf* buf, const IO::ExportObjectList &objects) {
	IO::BinaryArchive ar;
	if ( !ar.create(buf) )
		return false;
	for ( IO::ExportObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it ) {
		Core::BaseObject *obj = *it;
		ar << obj;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExporterVBinary::put(std::streambuf* buf, Core::BaseObject *obj) {
	IO::VBinaryArchive ar;
	if ( !ar.create(buf) )
		return false;
	ar << obj;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExporterVBinary::put(std::streambuf* buf, const IO::ExportObjectList &objects) {
	IO::VBinaryArchive ar;
	if ( !ar.create(buf) )
		return false;
	for ( IO::ExportObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it ) {
		Core::BaseObject *obj = *it;
		ar << obj;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
