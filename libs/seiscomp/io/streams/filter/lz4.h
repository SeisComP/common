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


#ifndef SEISCOMP_IO_STREAMS_FILTER_LZ4_H
#define SEISCOMP_IO_STREAMS_FILTER_LZ4_H


#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/pipeline.hpp>

#include <iostream>
#include <vector>

#include <lz4/lz4frame_static.h>


namespace ext {
namespace boost {
namespace iostreams {


using namespace ::boost::iostreams;


struct lz4_base {
	lz4_base(size_t inputBufferSize);
	~lz4_base();

	void cleanup_();

	size_t           _inputBufferSize;
	size_t           _outputBufferSize;
	char            *_outputBuffer;
	std::streamsize  _outputBuffered;
};


struct lz4_compress_base : lz4_base {
	lz4_compress_base(size_t inputBufferSize);
	~lz4_compress_base();

	void cleanup_();

	bool init();
	bool compress(const char *s, std::streamsize n);
	void done();

	LZ4F_cctx *_ctx;
	LZ4F_preferences_t _prefs;
};


template <typename Ch>
class basic_l4z_compressor : private lz4_compress_base {
	public:
		typedef char char_type;
		struct category
		: dual_use
		, filter_tag
		, multichar_tag
		, closable_tag
		, optimally_buffered_tag
		{};

		explicit basic_l4z_compressor(size_t bufferSize = 128)
		: lz4_compress_base(bufferSize), _bufferSize(bufferSize) {}

		std::streamsize optimal_buffer_size() const { return _bufferSize; }

		template<typename Source>
		std::streamsize read(Source &, char_type *, std::streamsize) {
			// Read is not supported
			return -1;
		}

		template<typename Sink>
		std::streamsize write(Sink &snk, const char_type *s, std::streamsize n) {
			if ( !_ctx ) {
				if ( !init() )
					return -1;

				if ( _outputBuffered )
					::boost::iostreams::write(snk, _outputBuffer, _outputBuffered);
			}

			if ( !compress((const char*)s, n * sizeof(char_type)) )
				return -1;

			if ( _outputBuffered )
				::boost::iostreams::write(snk, _outputBuffer, _outputBuffered);

			return n;
		}

		template<typename Sink>
		void close(Sink &snk, std::ios_base::openmode) {
			done();

			if ( _outputBuffered )
				::boost::iostreams::write(snk, _outputBuffer, _outputBuffered);

			cleanup_();
		}

	private:
		size_t _bufferSize;
};
BOOST_IOSTREAMS_PIPABLE(basic_l4z_compressor, 1)


typedef basic_l4z_compressor<char> lz4_compressor;



struct lz4_decompress_base : lz4_base {
	lz4_decompress_base(size_t inputBufferSize);
	~lz4_decompress_base();

	void cleanup_();

	bool init();
	bool decompress();

	LZ4F_dctx       *_ctx;
	char            *_inputBuffer;
	std::streamsize  _inputBufferPos;
	std::streamsize  _inputBuffered;
	std::streamsize  _outputBufferPos;
};


template <typename Ch>
class basic_l4z_decompressor : private lz4_decompress_base {
	public:
		typedef char char_type;
		struct category
		: dual_use
		, filter_tag
		, multichar_tag
		{};

		explicit basic_l4z_decompressor(size_t bufferSize = 128)
		: lz4_decompress_base(bufferSize), _bufferSize(bufferSize) {}

		template<typename Source>
		std::streamsize read(Source &snk, char_type *s, std::streamsize n) {
			if ( !_ctx ) {
				if ( !init() )
					return -1;
			}

			std::streamsize remaining = n;
			while ( remaining ) {
				// Copy as much from the output buffer as possible
				std::streamsize toCopy = std::min(remaining, _outputBuffered - _outputBufferPos);
				if ( toCopy ) {
					/*
					std::cerr << "<<<<<<<< " << toCopy << std::endl;
					for ( size_t i = 0; i < toCopy; ++i )
						std::cerr << "s[" << (n - remaining + i) << "] = " << _outputBuffer[_outputBufferPos + i]
						          << " (" << int(_outputBuffer[_outputBufferPos + i]) << ")"
						          << std::endl;
					*/

					std::copy(_outputBuffer + _outputBufferPos,
					          _outputBuffer + _outputBufferPos + toCopy, s + n - remaining);
					_outputBufferPos += toCopy;
					remaining -= toCopy;
				}
				else {
					if ( _inputBufferPos ) {
						_inputBuffered = _inputBufferPos;
						if ( decompress() ) {
							if ( _outputBuffered ) {
								continue;
							}
						}
						else
							return n-remaining;
					}

					// Need more uncompressed data
					_inputBuffered = ::boost::iostreams::read(snk, _inputBuffer + _inputBufferPos, _inputBufferSize - _inputBufferPos);
					_inputBufferPos = 0;
					if ( _inputBuffered <= 0 )
						return n-remaining;

					_inputBuffered += _inputBufferPos;

					if ( !decompress() )
						return n-remaining;
				}
			}

			return n;
		}

		template<typename Sink>
		std::streamsize write(Sink &, const char_type *, std::streamsize) {
			// Write is not supported
			return -1;
		}

	private:
		size_t _bufferSize;
};
BOOST_IOSTREAMS_PIPABLE(basic_l4z_decompressor, 1)


typedef basic_l4z_decompressor<char> lz4_decompressor;


}
}
}


#endif
