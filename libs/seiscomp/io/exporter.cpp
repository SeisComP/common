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



#include <seiscomp/io/exporter.h>
#include <seiscomp/core/interfacefactory.ipp>

#include <fstream>
#include <iostream>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::IO::Exporter, SC_SYSTEM_CORE_API);


namespace Seiscomp {
namespace IO {

namespace {

template <int N>
struct SinkBuf : std::streambuf {
	SinkBuf(ExportSink *s) : sink(s) {
		setp(out, out + N);
	}

	~SinkBuf() { sync(); }

	virtual int overflow(int c) override {
		if ( traits_type::eq_int_type(traits_type::eof(), c))
			return traits_type::eof();

		if ( sync() )
			return traits_type::eof();

		traits_type::assign(*pptr(), traits_type::to_char_type(c));
		pbump(1);

		return traits_type::not_eof(c);
	}

	virtual int sync() override {
		if ( pbase() == pptr() ) return 0;

		int bytes = pptr() - pbase();
		pbase()[bytes] = '\0';
		int res = sink->write(pbase(), bytes);
		// Reset put pointer
		setp(out, out + N);
		return res == bytes ? 0 : 1;
	}

	ExportSink *sink;
	char        out[N+1];
};


}


IMPLEMENT_SC_ABSTRACT_CLASS(Exporter, "Exporter");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter::Exporter() {
	_prettyPrint = false;
	_indentation = 2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter::~Exporter() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter *Exporter::Create(const char *type) {
	return ExporterFactory::Create(type);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Exporter::setFormattedOutput(bool enable) {
	_prettyPrint = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Exporter::setIndent(int i) {
	_indentation = i;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(std::streambuf* buf, Core::BaseObject *obj) {
	return put(buf, obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(std::string filename, Core::BaseObject *obj) {
	if ( filename != "-" ) {
		std::ofstream ofs(filename.c_str(), std::ios_base::out);
		if ( !ofs.good() ) return false;
		return put(ofs.rdbuf(), obj);
	}

	return put(std::cout.rdbuf(), obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(ExportSink *sink, Core::BaseObject *obj) {
	SinkBuf<512> buf(sink);
	return write(&buf, obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(std::streambuf* buf, const ExportObjectList &objects) {
	return put(buf, objects);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(std::string filename, const ExportObjectList &objects) {
	if ( filename != "-" ) {
		std::ofstream ofs(filename.c_str(), std::ios_base::out);
		if ( !ofs.good() ) return false;
		return put(ofs.rdbuf(), objects);
	}

	return put(std::cout.rdbuf(), objects);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::write(ExportSink *sink, const ExportObjectList &objects) {
	SinkBuf<512> buf(sink);
	return write(&buf, objects);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Exporter::put(std::streambuf* buf, const ExportObjectList &objects) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
