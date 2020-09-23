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


#include "lz4/lz4.c"
#include "lz4/lz4frame.c"
#include "lz4/lz4hc.c"
#include "lz4/xxhash.c"

#include <seiscomp/io/streams/filter/lz4.h>
#include <stdexcept>


namespace ext {
namespace boost {
namespace iostreams {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
lz4_base::lz4_base(size_t inputBufferSize)
: _inputBufferSize(inputBufferSize)
, _outputBufferSize(0)
, _outputBuffer(nullptr)
, _outputBuffered(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
lz4_base::~lz4_base() {
	cleanup_();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void lz4_base::cleanup_() {
	if ( _outputBuffer ) {
		delete[] _outputBuffer;
		_outputBuffer = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
lz4_compress_base::lz4_compress_base(size_t inputBufferSize)
: lz4_base(inputBufferSize)
, _ctx(nullptr) {
	MEM_INIT(&_prefs, 0, sizeof(_prefs));
	_prefs.compressionLevel = LZ4HC_CLEVEL_DEFAULT;
	_prefs.autoFlush = 1;
	_outputBufferSize = std::max(LZ4F_compressFrameBound(inputBufferSize, nullptr), size_t(64));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
lz4_compress_base::~lz4_compress_base() {
	cleanup_();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool lz4_compress_base::init() {
	LZ4F_errorCode_t r;

	assert(!_outputBuffer);
	_outputBuffer = new char[_outputBufferSize];
	_outputBuffered = 0;

	r = LZ4F_createCompressionContext(&_ctx, LZ4F_VERSION);
	if ( LZ4F_isError(r) ) {
		cleanup_();
		return false;
	}

	r = LZ4F_compressBegin(_ctx, _outputBuffer, _outputBufferSize, &_prefs);
	if ( LZ4F_isError(r) ) {
		cleanup_();
		return false;
	}

	_outputBuffered = r;

	/*
	std::cerr << "INIT >>>>>>>>>>> " << _outputBuffered << std::endl;
	for ( std::streamsize i = 0; i < _outputBuffered; ++i ) {
		std::cerr << int(_outputBuffer[i]) << std::endl;
	}
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool lz4_compress_base::compress(const char *s, std::streamsize n) {
	LZ4F_errorCode_t r;

	r = LZ4F_compressUpdate(_ctx, _outputBuffer, _outputBufferSize, s, n, nullptr);
	if ( LZ4F_isError(r) ) {
		return false;
	}

	_outputBuffered = r;

	/*
	std::cerr << "COMPRESS >>>>>>> " << _outputBuffered << std::endl;
	for ( std::streamsize i = 0; i < _outputBuffered; ++i ) {
		std::cerr << int(_outputBuffer[i]) << std::endl;
	}
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void lz4_compress_base::cleanup_() {
	lz4_base::cleanup_();

	if ( _ctx ) {
		LZ4F_freeCompressionContext(_ctx);
		_ctx = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void lz4_compress_base::done() {
	if ( _ctx ) {
		_outputBuffered = LZ4F_compressEnd(_ctx, _outputBuffer, _outputBufferSize, nullptr);
		if ( LZ4F_isError(_outputBuffered) ) {
			cleanup_();
			throw std::runtime_error(LZ4F_getErrorName(_outputBuffered));
			//std::cerr << "ERR: " << LZ4F_getErrorCode(_outputBuffered) << ": " << LZ4F_getErrorName(_outputBuffered) << std::endl;
		}
	}
	else
		_outputBuffered = 0;

	/*
	std::cerr << "DONE >>>>>>>>>>> " << _outputBuffered << std::endl;
	for ( std::streamsize i = 0; i < _outputBuffered; ++i ) {
		std::cerr << int(_outputBuffer[i]) << std::endl;
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
lz4_decompress_base::lz4_decompress_base(size_t inputBufferSize)
: lz4_base(inputBufferSize)
, _ctx(nullptr)
, _inputBuffer(nullptr)
, _inputBufferPos(0)
, _inputBuffered(0)
, _outputBufferPos(0) {
	_outputBufferSize = std::max(inputBufferSize, size_t(128));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
lz4_decompress_base::~lz4_decompress_base() {
	cleanup_();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool lz4_decompress_base::init() {
	assert(!_ctx);
	assert(!_inputBuffer);
	assert(!_outputBuffer);

	LZ4F_errorCode_t r;

	_inputBuffer = new char[_inputBufferSize];
	_outputBuffer = new char[_outputBufferSize];

	r = LZ4F_createDecompressionContext(&_ctx, LZ4F_VERSION);
	if ( LZ4F_isError(r) ) {
		cleanup_();
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool lz4_decompress_base::decompress() {
	LZ4F_errorCode_t r;

	_outputBufferPos = 0;

	size_t usedInput = _inputBuffered;
	size_t usedOutput = _outputBufferSize;

	r = LZ4F_decompress(_ctx, _outputBuffer, &usedOutput,
	                    _inputBuffer, &usedInput, nullptr);
	if ( LZ4F_isError(r) ) {
		cleanup_();
		return false;
	}

	/*
	for ( size_t i = 0; i < usedOutput; ++i ) {
		std::cerr << _outputBuffer[i] << " (" << int(_outputBuffer[i]) << ")" << std::endl;
	}
	*/

	if ( usedInput < size_t(_inputBuffered) ) {
		_inputBufferPos = _inputBuffered - usedInput;
		std::copy(_inputBuffer + usedInput,
		          _inputBuffer + _inputBuffered, _inputBuffer);
		_inputBuffered = _inputBufferPos;
	}
	else
		_inputBufferPos = 0;

	_outputBuffered = usedOutput;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void lz4_decompress_base::cleanup_() {
	lz4_base::cleanup_();

	if ( _inputBuffer ) {
		delete[] _inputBuffer;
		_inputBuffer = nullptr;
	}

	if ( _ctx ) {
		LZ4F_freeDecompressionContext(_ctx);
		_ctx = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
