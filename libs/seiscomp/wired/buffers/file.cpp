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


#include <seiscomp/wired/buffers/file.h>
#include <sys/stat.h>


namespace Seiscomp {
namespace Wired {

namespace {

const char *MimeTypes[FileBuffer::TypeQuantity] = {
	"text/html",
	"text/css",
	"text/javascript",
	"application/json",
	"image/png",
	"image/jpeg",
	"image/svg+xml",
	"text/xml",
	"application/font-woff",
	"font/ttf"
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FileBuffer::FileBuffer(int max_size)
: Buffer(max_size), fp(NULL), fplen(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FileBuffer::~FileBuffer() {
	if ( fp ) fclose(fp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FileBuffer::open(const std::string &fn, const char *mode) {
	return open(fn.c_str(), mode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FileBuffer::open(const char *fn, const char *mode) {
	if ( fp != NULL ) return false;

	struct stat fstat;

	if ( stat(fn, &fstat) < 0 )
		return false;

	if ( !S_ISREG(fstat.st_mode) )
		return false;

	lastModified = fstat.st_mtime;

	fp = fopen(fn, mode);
	if ( fp == NULL ) return false;

	fseek(fp, 0L, SEEK_END);
	fplen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if ( fplen == -1L ) {
		fclose(fp);
		fp = NULL;
		fplen = 0;
		return false;
	}
	else if ( fplen == 0 ) {
		header.clear();
		data.clear();
		return true;
	}

	size_t sz = fplen;
	if ( (int)sz > maxBufferSize ) sz = maxBufferSize;
	data.resize(sz);

	return updateBuffer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FileBuffer::updateBuffer() {
	// Erase header since we are transfering the data blocked
	header.clear();
	size_t rb = fread(&data[0], 1, data.size(), fp);
	data.resize(rb);
	return data.size() > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t FileBuffer::length() const {
	return fplen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *FileBuffer::mimeType(const char *filenameExtension) {
	if ( !strcasecmp(filenameExtension, "html") )
		return MimeTypes[HTML];
	else if ( !strcasecmp(filenameExtension, "css") )
		return MimeTypes[CSS];
	else if ( !strcasecmp(filenameExtension, "js") )
		return MimeTypes[JS];
	else if ( !strcasecmp(filenameExtension, "json") )
		return MimeTypes[JSON];
	else if ( !strcasecmp(filenameExtension, "png") )
		return MimeTypes[PNG];
	else if ( !strcasecmp(filenameExtension, "jpg")
	       || !strcasecmp(filenameExtension, "jpeg") )
		return MimeTypes[JPG];
	else if ( !strcasecmp(filenameExtension, "svg") )
		return MimeTypes[SVG];
	else if ( !strcasecmp(filenameExtension, "xml") )
		return MimeTypes[XML];
	else if ( !strcasecmp(filenameExtension, "woff") )
		return MimeTypes[WOFF];
	else if ( !strcasecmp(filenameExtension, "ttf") )
		return MimeTypes[TTF];
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *FileBuffer::mimeType(Type type) {
	return MimeTypes[type];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
