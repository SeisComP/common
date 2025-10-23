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


// #define USE_RAPIDJSON
#define SEISCOMP_COMPONENT core/io/records/mseed

#include <seiscomp/logging/log.h>
#include <seiscomp/core/arrayfactory.h>
#include <seiscomp/core/endianess.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/utils/certstore.h>

#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/io/records/mseed/decoder/format.h>
#include <seiscomp/io/records/mseed/encoder/format.h>
#include <seiscomp/io/records/mseed/encoder/steim1.h>
#include <seiscomp/io/records/mseed/encoder/steim2.h>
#include <seiscomp/io/records/mseed/encoder/uncompressed.h>

#include <rapidjson/document.h>

#include <openssl/asn1.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#if OPENSSL_VERSION_NUMBER > 0x10101000
#include <openssl/x509err.h>
#endif

#include <cmath>
#include <cctype>
#include <cstring>
#include <string_view>


using namespace std;


namespace Seiscomp::IO {
namespace {


bool isPowerOfTwo(size_t n) {
	if ( !n ) {
		return false;
	}

	while ( n != 1 ) {
		if ( n % 2 ) {
			return false;
		}
		n >>= 1;
	}

	return true;
}


void mstrncpy(std::string &dest, const char *source, int length) {
	for ( ; length; ++source, --length ) {
		if ( *source == '\0' ) {
			break;
		}

		if ( *source != ' ' ) {
			dest.push_back(*source);
		}
	}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_CLASS_DERIVED(MSeedRecord, Record, "MSeedRecord");
REGISTER_RECORD(MSeedRecord, "mseed");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(Array::DataType dt, Hint h)
: Record(dt, h) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(const MSeedRecord &msrec) {
	*this = msrec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(const Record &rec, int reclen)
: Record(rec) {
	_recordLength = reclen;
	_data = rec.data() ? rec.data()->clone() : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setNetworkCode(std::string net) {
	if ( _hint == SAVE_RAW ) {
		if ( _format == V2 ) {
			using namespace MSEED::V2;
			auto *netHeader = Network::Get(_raw.typedData());
			for ( size_t i = 0; i < 2; ++i ) {
				netHeader[i] = net.size() > 0 ? net[i] : ' ';
			}
		}
	}

	Record::setNetworkCode(net);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setStationCode(std::string sta) {
	if ( _hint == SAVE_RAW ) {
		if ( _format == V2 ) {
			using namespace MSEED::V2;
			auto *staHeader = Station::Get(_raw.typedData());
			for ( size_t i = 0; i < 5; ++i ) {
				staHeader[i] = sta.size() > i ? sta[i] : ' ';
			}
		}
	}

	Record::setStationCode(sta);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setLocationCode(std::string loc) {
	if ( _hint == SAVE_RAW ) {
		if ( _format == V2 ) {
			using namespace MSEED::V2;
			auto *locHeader = Location::Get(_raw.typedData());
			for ( size_t i = 0; i < 2; ++i ) {
				locHeader[i] = loc.size() > i ? loc[i] : ' ';
			}
		}
	}

	Record::setLocationCode(loc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setChannelCode(std::string cha) {
	if ( _hint == SAVE_RAW ) {
		if ( _format == V2 ) {
			using namespace MSEED::V2;
			auto *chaHeader = Channel::Get(_raw.typedData());
			for ( size_t i = 0; i < 3; ++i ) {
				chaHeader[i] = cha.size() > i ? cha[i] : ' ';
			}
		}
	}

	Record::setChannelCode(cha);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setStartTime(const Core::Time &time) {
	using namespace MSEED;
	if ( _hint == SAVE_RAW ) {
		if ( _format == V2 ) {
			using namespace MSEED::V2;
			bool swapflag = _byteOrder & MSEED::SwapHeader;
			int year, yday, hour, minute, second, microsecond;
			time.get2(&year, &yday, &hour, &minute, &second, &microsecond);

			*Year::Get(_raw.typedData()) = swap(year, swapflag);
			*YDay::Get(_raw.typedData()) = swap(yday + 1, swapflag);

			*Hour::Get(_raw.typedData()) = hour;
			*Minute::Get(_raw.typedData()) = minute;
			*Second::Get(_raw.typedData()) = second;
			*Unused::Get(_raw.typedData()) = 0;
			*FSecond::Get(_raw.typedData()) = swap(microsecond / 100, swapflag);
		}
	}

	Record::setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t MSeedRecord::Detect(const void *ptr, size_t len, Format *format) {
	using namespace MSEED;

	if ( (len >= V2::HeaderLength) && V2::isValidHeader(ptr) ) {
		if ( format ) {
			*format = V2;
		}

		using namespace MSEED::V2;

		bool swapflag = false;
		if ( !isValidYearDay(*Year::Get(ptr), *YDay::Get(ptr)) ) {
			swapflag = true;
		}

		auto blkt_offset = swap(*BlocketteOffset::Get(ptr), swapflag);
		uint16_t blkt_type;
		uint16_t next_blkt;

		const char *data = reinterpret_cast<const char*>(ptr);

		while ( blkt_offset != 0 ) {
			blkt_type = swap(*reinterpret_cast<const uint16_t*>(data + blkt_offset), swapflag);
			next_blkt = swap(*reinterpret_cast<const uint16_t*>(data + blkt_offset + 2), swapflag);

			if ( next_blkt != 0 ) {
				if ( (next_blkt < 4 || (next_blkt - 4) <= blkt_offset) ) {
					SEISCOMP_ERROR("Invalid blockette offset less than or equal to current offset");
					return -1;
				}
			}

			if ( blkt_type == 1000 ) {
				return static_cast<uint64_t>(1) << *B1000RecLength::Get(data + blkt_offset);
			}

			blkt_offset = next_blkt;
		}
	}
	else if ( (len >= V3::HeaderLength) && V3::isValidHeader(ptr) ) {
		if ( format ) {
			*format = V3;
		}

		return V3::HeaderLength +
		       Core::Endianess::Converter::ToLittleEndian(*V3::SIDLength::Get(ptr)) +
		       Core::Endianess::Converter::ToLittleEndian(*V3::ExtraLength::Get(ptr)) +
		       Core::Endianess::Converter::ToLittleEndian(*V3::DataLength::Get(ptr));
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord &MSeedRecord::operator=(const MSeedRecord &msrec) {
	if ( &msrec != this ) {
		Record::operator=(msrec);
		_format = msrec._format;
		_raw = msrec._raw;
		_data = msrec._data ? msrec._data->clone() : nullptr;
		_quality = msrec._quality;
		_byteOrder = msrec._byteOrder;
		_encoding = msrec._encoding;
		_encodingFlag = msrec._encodingFlag;
		_recordLength = msrec._recordLength;
		_endTime = msrec._endTime;
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setDataQuality(char qual) {
	_quality = qual;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Array *MSeedRecord::raw() const {
	return &_raw;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Array *MSeedRecord::data() const {
	if ( _raw.data() && (!_data || _datatype != _data->dataType()) ) {
		unpackData(_raw.typedData(), _raw.size());
	}

	return _data.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::unpackData(const char *rec, size_t reclen) const {
	using namespace MSEED;

	Array::DataType dt = _datatype;
	_data = nullptr;

	int dataLength;
	const char *data;
	int64_t nsamples;

	if ( _format == V2 ) {
		auto dataOffset = swap(*V2::DataOffset::Get(rec), _byteOrder & SwapHeader);
		if ( dataOffset >= reclen ) {
			throw LibmseedException("Data frames outside of record");
		}

		dataLength = reclen - dataOffset;
		data = rec + dataOffset;
	}
	else if ( _format == V3 ) {
		using C = Core::Endianess::Converter;
		using namespace V3;

		auto sidLength = *SIDLength::Get(rec);
		auto extraLength = C::ToLittleEndian(*ExtraLength::Get(rec));
		size_t dataOffset = HeaderLength + sidLength + extraLength;
		dataLength = C::ToLittleEndian(*DataLength::Get(rec));

		if ( dataOffset >= reclen ) {
			throw LibmseedException("Data section outside of record");
		}

		if ( dataOffset + dataLength > reclen ) {
			throw LibmseedException("Data section outside of record");
		}

		data = rec + dataOffset;
	}
	else {
		throw LibmseedException("Invalid miniSEED format");
	}

	switch ( static_cast<EncodingType>(_encoding) ) {
		case EncodingType::ASCII:
			if ( _nsamp != dataLength ) {
				throw LibmseedException("Number of characters does not match data size");
			}
			else {
				auto cdata = new CharArray(_nsamp);
				memcpy(cdata->typedData(), data, dataLength);
				nsamples = dataLength;
				_data = cdata;
			}
			break;
		case EncodingType::INT16:
		{
			auto idata = new IntArray(_nsamp);
			for ( int i = 0 ; i < _nsamp; ++i ) {
				(*idata)[i] = reinterpret_cast<const int16_t*>(data)[i];
			}
			if ( _byteOrder & SwapPayload ) {
				Core::Endianess::swap(idata->typedData(), idata->size());
			}
			nsamples = _nsamp;
			_data = idata;
			break;
		}
		case EncodingType::INT32:
		{
			auto idata = new IntArray(_nsamp);
			memcpy(idata->typedData(), data, _nsamp * 4);
			if ( _byteOrder & SwapPayload ) {
				Core::Endianess::swap(idata->typedData(), idata->size());
			}
			nsamples = _nsamp;
			_data = idata;
			break;
		}
		case EncodingType::FLOAT32:
		{
			auto fdata = new FloatArray(_nsamp);
			memcpy(fdata->typedData(), data, _nsamp * 4);
			if ( _byteOrder & SwapPayload ) {
				Core::Endianess::swap(fdata->typedData(), fdata->size());
			}
			nsamples = _nsamp;
			_data = fdata;
			break;
		}
		case EncodingType::FLOAT64:
		{
			auto ddata = new DoubleArray(_nsamp);
			memcpy(ddata->typedData(), data, _nsamp * 8);
			if ( _byteOrder & SwapPayload ) {
				Core::Endianess::swap(ddata->typedData(), ddata->size());
			}
			nsamples = _nsamp;
			_data = ddata;
			break;
		}
		default:
			SEISCOMP_WARNING("Unsupported data type: %d, assuming STEIM1", static_cast<int>(_encoding));
		case EncodingType::STEIM1:
		{
			auto idata = new IntArray(_nsamp);
			nsamples = decodeSteim1(
				reinterpret_cast<int32_t*>(const_cast<char*>(data)), dataLength, _nsamp,
				idata->typedData(), idata->size() * idata->elementSize(),
				_byteOrder & SwapPayload
			);
			_data = idata;
			break;
		}
		case EncodingType::STEIM2:
		{
			auto idata = new IntArray(_nsamp);
			nsamples = decodeSteim2(
				reinterpret_cast<int32_t*>(const_cast<char*>(data)), dataLength, _nsamp,
				idata->typedData(), idata->size() * idata->elementSize(),
				_byteOrder & SwapPayload
			);
			_data = idata;
			break;
		}
		case EncodingType::GEOSCOPE24:
		case EncodingType::GEOSCOPE163:
		case EncodingType::GEOSCOPE164:
		{
			auto fdata = new FloatArray(_nsamp);
			nsamples = decodeGEOSCOPE(
				const_cast<char*>(data), _nsamp,
				fdata->typedData(), _data->size() * _data->elementSize(),
				static_cast<EncodingType>(_encoding), _byteOrder & SwapPayload
			);
			_data = fdata;
			break;
		}
		case EncodingType::CDSN:
		{
			auto idata = new IntArray(_nsamp);
			nsamples = decodeCDSN(
				reinterpret_cast<int16_t*>(const_cast<char*>(data)), _nsamp,
				idata->typedData(), _data->size() * _data->elementSize(),
				_byteOrder & SwapPayload
			);
			_data = idata;
			break;
		}
		case EncodingType::SRO:
		{
			auto idata = new IntArray(_nsamp);
			nsamples = decodeSRO(
				reinterpret_cast<int16_t*>(const_cast<char*>(data)), _nsamp,
				idata->typedData(), _data->size() * _data->elementSize(),
				_byteOrder & SwapPayload
			);
			_data = idata;
			break;
		}
		case EncodingType::DWWSSN:
		{
			auto idata = new IntArray(_nsamp);
			nsamples = decodeDWWSSN(
				reinterpret_cast<int16_t*>(const_cast<char*>(data)), _nsamp,
				idata->typedData(), _data->size() * _data->elementSize(),
				_byteOrder & SwapPayload
			);
			break;
		}
	}

	if ( nsamples != _nsamp ) {
		SEISCOMP_WARNING("%s.%s.%s.%s: inconsistent sample count",
		                 _net, _sta, _loc, _cha);
	}

	if ( dt != Array::DT_QUANTITY ) {
		// Convert the data to the final datatype
		_data = ArrayFactory::Create(dt, _data.get());
	}

	// Final check
	if ( _data->size() != _nsamp ) {
		SEISCOMP_WARNING("%s.%s.%s.%s: inconsistent sample count",
		                 _net, _sta, _loc, _cha);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::updateAuthentication(const char *rec, size_t reclen, size_t blkt_offset) {
	using namespace MSEED::V2;

	_authenticationStatus = NOT_SIGNED;

	Util::CertificateStore &cs = Util::CertificateStore::global();
	if ( !cs.isValid() ) {
		return;
	}

	bool swapflag = _byteOrder & MSEED::SwapHeader;

	auto *blkt = rec + blkt_offset;
	if ( *B2000HeaderCount::Get(blkt) != 1 ) {
		return;
	}

	if ( MSEED::swap(*B2000DataOffset::Get(blkt), swapflag) < 21 ) {
		return;
	}

	auto b2000DataOffset = MSEED::swap(*B2000DataOffset::Get(blkt), swapflag);
	auto b2000DataLength = MSEED::swap(*B2000Length::Get(blkt), swapflag);
	size_t maxHeaderLength = b2000DataOffset - 15;
	const char *opaqHeaders = B2000Payload::Get(blkt);
	std::string_view header;
	for ( size_t i = 0; i < maxHeaderLength; ++i ) {
		if ( opaqHeaders[i] == '~' ) {
			header = { opaqHeaders, i };
			break;
		}
	}

	if ( header.empty() ) {
		return;
	}

	if ( header.compare(0, 5, "SIGN/") ) {
		return;
	}

	header = header.substr(5);
	size_t nSignature = b2000DataLength - b2000DataOffset;

	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int nDigest = 0;

	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
	EVP_DigestInit(mdctx, EVP_sha256());
	EVP_DigestUpdate(mdctx, DataQuality::Get(rec), 1);

	EVP_DigestUpdate(
		mdctx,
		Station::Get(rec),
		(char*)DataOffset::Get(rec) - (char*)Station::Get(rec)
	);
	EVP_DigestUpdate(
		mdctx,
		rec + MSEED::swap(*DataOffset::Get(rec), swapflag),
		reclen - MSEED::swap(*DataOffset::Get(rec), swapflag)
	);
	EVP_DigestFinal_ex(mdctx, reinterpret_cast<unsigned char*>(digest), &nDigest);
	EVP_MD_CTX_destroy(mdctx);

	const unsigned char *pp = reinterpret_cast<const unsigned char *>(blkt) + b2000DataOffset;

	const X509 *cert;
	if ( !cs.validate(header, reinterpret_cast<const char*>(digest),
	                  nDigest, pp, nSignature, &cert) ) {
		const_cast<MSeedRecord*>(this)->_authenticationStatus = SIGNATURE_VALIDATION_FAILED;
		SEISCOMP_WARNING("MSEED: Signature validation failed");
	}
	else {
		if ( cert ) {
			X509_NAME *name = X509_get_subject_name(const_cast<X509*>(cert));
			if ( name ) {
				int pos = X509_NAME_get_index_by_NID(name, NID_organizationName, -1);
				if ( pos != -1 ) {
					X509_NAME_ENTRY *e = X509_NAME_get_entry(name, pos);
					ASN1_STRING *str = X509_NAME_ENTRY_get_data(e);
					if ( ASN1_STRING_type(str) != V_ASN1_UTF8STRING ) {
						unsigned char *utf8 = 0;
						int length = ASN1_STRING_to_UTF8(&utf8, str);
						if ( length > 0 ) {
							const_cast<MSeedRecord*>(this)->_authority.assign(reinterpret_cast<char*>(utf8), size_t(length));
						}
						if ( utf8 ) {
							OPENSSL_free(utf8);
						}
					}
					else {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
						_authority.assign(reinterpret_cast<char*>(ASN1_STRING_data(str)), size_t(ASN1_STRING_length(str)));
#else
						_authority.assign(reinterpret_cast<const char*>(ASN1_STRING_get0_data(str)), ASN1_STRING_length(str));
#endif
					}
				}
				else {
					SEISCOMP_WARNING("MSEED: Failed to extract certificate authority (O)");
				}
			}
		}

		_authenticationStatus = SIGNATURE_VALIDATED;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::saveSpace() const {
	if ( _hint == SAVE_RAW && _data) {
		_data = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* MSeedRecord::copy() const {
	return new MSeedRecord(*this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::useEncoding(bool flag) {
	_encodingFlag = flag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setLittleEndian(bool flag) {
	if ( flag ) {
		_byteOrder |= MSEED::TargetLittleEndian;
	}
	else {
		_byteOrder &= ~MSEED::TargetLittleEndian;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setOutputRecordLength(int reclen) {
	_recordLength = reclen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool _isHeader(const char *header) {
	return (std::isdigit(*(header + 0))
	     && std::isdigit(*(header + 1))
	     && std::isdigit(*(header + 2))
	     && std::isdigit(*(header + 3))
	     && std::isdigit(*(header + 4))
	     && std::isdigit(*(header + 5))
	     && std::isalpha(*(header + 6))
	     && (*(header + 7) == ' ' || *(header + 7) == '\0'));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

bool fill(std::istream &is, std::vector<char> &buffer, size_t newSize) {
	size_t oldSize = buffer.size();
	if ( newSize > oldSize ) {
		buffer.resize(newSize);
		is.read(buffer.data() + oldSize, buffer.size() - oldSize);
		return is.good();
	}
	else
		return true;
}

void error(std::istream &is, const char *msg, size_t fp) {
	if ( is.eof() ) {
		throw Core::EndOfStreamException();
	}
	else {
		is.seekg(fp);
		throw Core::StreamException(msg);
	}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::read(std::istream &is) {
	using namespace MSEED;

	int reclen = -1;
	bool swapflag = false;
	char header[std::max(V2::HeaderLength, V3::HeaderLength)];

	auto &buffer = _raw.impl();

	_data = nullptr;
	_byteOrder &= TargetLittleEndian;
	_encoding = -1;
	_net = _sta = _loc = _cha = {};

	while ( is.good() ) {
		if ( is.read(header, std::min(V2::HeaderLength, V3::HeaderLength)) ) {
			if ( V2::isValidHeader(header) ) {
				if constexpr ( V2::HeaderLength > V3::HeaderLength ) {
					auto p = is.tellg();
					if ( !is.read(header + V3::HeaderLength, V2::HeaderLength - V3::HeaderLength) ) {
						error(is, "Header v2 reading error", p);
					}
				}

				buffer.reserve(512);
				buffer.resize(V2::HeaderLength);
				memcpy(buffer.data(), header, V2::HeaderLength);

				if constexpr ( V2::HeaderLength < 64 ) {
					auto p = is.tellg();
					if ( !fill(is, buffer, 64) ) {
						error(is, "Blockette reading error", p);
					}
				}

				// Remember current position
				auto p = is.tellg();

				using namespace MSEED::V2;

				if ( !isValidYearDay(*Year::Get(buffer.data()), *YDay::Get(buffer.data())) ) {
					_byteOrder |= SwapHeader;
					swapflag = true;
					swapHeader(header);
				}

				auto hours = *Hour::Get(header);
				auto minutes = *Minute::Get(header);
				auto seconds = *Second::Get(header);
				auto fsec = *FSecond::Get(header) * 100;

				_stime = Core::Time::FromYearDay(*Year::Get(header), *YDay::Get(header));
				_stime += Core::TimeSpan(hours * 3600 + minutes * 60 + seconds, fsec);

				if ( *TimeCorrection::Get(header) != 0 && !(*ActivityFlags::Get(header) & 0x02) ) {
					_stime += Core::TimeSpan(0, *TimeCorrection::Get(header) * 100);
				}

				mstrncpy(_net, Network::Get(header), 2);
				mstrncpy(_sta, Station::Get(header), 5);
				mstrncpy(_loc, Location::Get(header), 2);
				mstrncpy(_cha, Channel::Get(header), 3);

				auto blkt_offset = *BlocketteOffset::Get(header);
				uint16_t blkt_type;
				uint16_t next_blkt;
				int blocketteCount = 0;
				int ofsB1000 = 0, ofsB2000 = 0;

				while ( blkt_offset != 0 ) {
					if ( !fill(is, buffer, blkt_offset + 4) ) {
						error(is, "Blockette reading error", p);
					}

					blkt_type = swap(*reinterpret_cast<uint16_t*>(buffer.data() + blkt_offset), swapflag);
					next_blkt = swap(*reinterpret_cast<uint16_t*>(buffer.data() + blkt_offset + 2), swapflag);

					if ( next_blkt != 0 ) {
						if ( (next_blkt < 4 || (next_blkt - 4) <= blkt_offset) ) {
							error(is, "Invalid blockette offset less than or equal to current offset", p);
						}

						if ( !fill(is, buffer, next_blkt) ) {
							error(is, "Blockette reading error", p);
						}
					}

					// Found a 1000 blockette
					if ( blkt_type == 1000 ) {
						ofsB1000 = blkt_offset;

						if ( !fill(is, buffer, blkt_offset + BHeadLength + B1000Length) ) {
							error(is, "Blockette 1000 reading error", p);
						}

						reclen = static_cast<uint64_t>(1) << *B1000RecLength::Get(buffer.data() + blkt_offset);
						_encoding = *B1000Encoding::Get(buffer.data() + blkt_offset);
						if ( *B1000ByteOrder::Get(buffer.data() + blkt_offset) > 0 ) {
							if constexpr ( Core::Endianess::Current::LittleEndian ) {
								_byteOrder |= SwapPayload;
							}
						}
						else if ( *B1000ByteOrder::Get(buffer.data() + blkt_offset) == 0 ) {
							if constexpr ( Core::Endianess::Current::BigEndian ) {
								_byteOrder |= SwapPayload;
							}
						}
					}
					else if ( blkt_type == 1001 ) {
						if ( !fill(is, buffer, blkt_offset + BHeadLength + B1001Length) ) {
							error(is, "Blockette 1001 reading error", p);
						}

						_stime += Core::TimeSpan(0, *B1001MicroSecond::Get(buffer.data() + blkt_offset));
						_timequal = *B1001TimingQuality::Get(buffer.data() + blkt_offset);
					}
					else if ( blkt_type == 2000 ) {
						ofsB2000 = blkt_offset;
					}

					blkt_offset = next_blkt;
					++blocketteCount;
				}

				if ( blocketteCount != *BlocketteCount::Get(buffer.data()) ) {
					SEISCOMP_WARNING("%s.%s.%s.%s: blockette count does not match",
					                 _net, _sta, _loc, _cha);
				}

				if ( reclen < 0 ) {
					// Read current record to next power of two
					size_t oldSize = buffer.size();
					size_t newSize = 1;

					while ( newSize < oldSize ) {
						newSize <<= 1;
					}

					if ( !fill(is, buffer, newSize) ) {
						error(is, "Data read error", p);
					}

					while ( is.read(header, sizeof(header)) ) {
						if ( isValidHeader(header) || _isHeader(header) ) {
							reclen = buffer.size();
							// Set current stream position back to header start
							is.seekg(-sizeof(header), std::ios::cur);
							break;
						}
						else {
							if ( buffer.size() < MaximumRecordLength ) {
								buffer.resize(buffer.size() + sizeof(header));
								memcpy(buffer.data() + buffer.size() - sizeof(header), header, sizeof(header));
							}
							else {
								error(is, "miniSEED Record exceeds maximum allowed record length", p);
							}
						}
					}

					if ( is.eof() and isPowerOfTwo(buffer.size()) ) {
						reclen = buffer.size();
					}
				}

				if ( reclen <= 0 ) {
					continue;
				}

				if ( size_t(reclen) > MaximumRecordLength ) {
					throw Core::StreamException("miniSEED2 records exceeds 2**20 bytes");
				}

				if ( !fill(is, buffer, reclen) ) {
					error(is, "Fatal error occured while reading record from stream", p);
				}

				_format = V2;
				_recordLength = reclen;
				_nsamp = *SampleCount::Get(header);
				if ( (_nsamp <= 0) || (_nsamp > 1000000) ) {
					throw LibmseedException("Invalid sample count in header");
				}

				_fsamp = 0.0;

				int64_t sfNum{0}, sfDen{0};

				if ( *SamplingRateF::Get(header) > 0 ) {
					sfNum = *SamplingRateF::Get(header);
					sfDen = 1;
				}
				else {
					sfNum = 1;
					sfDen = -*SamplingRateF::Get(header);
				}

				if ( *SamplingRateM::Get(header) > 0 ) {
					sfNum *= *SamplingRateM::Get(header);
				}
				else {
					sfDen *= -*SamplingRateM::Get(header);
				}

				if ( sfDen > 0 ) {
					_fsamp = static_cast<double>(sfNum) / static_cast<double>(sfDen);
				}

				if ( sfNum > 0 ) {
					_endTime = _stime + Core::TimeSpan(0, _nsamp * sfDen * 1000000 / sfNum);
				}
				else {
					_endTime = _stime;
				}

				_quality = *DataQuality::Get(header);
				_encodingFlag = true;

				if ( !ofsB1000 && (_byteOrder & SwapHeader) ) {
					_byteOrder |= SwapPayload;
				}

				if ( ofsB2000 ) {
					updateAuthentication(buffer.data(), reclen, ofsB2000);
				}

				break;
			}
			else if ( V3::isValidHeader(header) ) {
				using namespace V3;

				using C = Core::Endianess::Converter;
				// Remember current position
				auto p = is.tellg();

				if constexpr ( V3::HeaderLength > V2::HeaderLength ) {
					if ( !is.read(header + V2::HeaderLength, V3::HeaderLength - V2::HeaderLength) ) {
						error(is, "Header v3 reading error", p);
					}
				}

				if constexpr ( Core::Endianess::Current::BigEndian ) {
					_byteOrder |= SwapHeader;
				}

				_encoding = C::ToLittleEndian(*Encoding::Get(header));

				if ( (_encoding == static_cast<int16_t>(EncodingType::STEIM1)) ||
				     (_encoding == static_cast<int16_t>(EncodingType::STEIM2)) ) {
					if constexpr ( Core::Endianess::Current::LittleEndian ) {
						_byteOrder |= SwapPayload;
					}
				}
				else {
					if constexpr ( Core::Endianess::Current::BigEndian ) {
						_byteOrder |= SwapPayload;
					}
				}

				uint8_t sidLength;
				uint16_t extraLength;
				uint32_t dataLength;

				sidLength = *SIDLength::Get(header);
				extraLength = C::ToLittleEndian(*ExtraLength::Get(header));
				dataLength = C::ToLittleEndian(*DataLength::Get(header));

				reclen = HeaderLength + sidLength + extraLength + dataLength;

				if ( !dataLength ) {
					// Skip empty records
					continue;
				}

				buffer.reserve(reclen);
				buffer.resize(V3::HeaderLength);
				memcpy(buffer.data(), header, V3::HeaderLength);

				if ( size_t(reclen) > MaximumRecordLength ) {
					throw Core::StreamException("miniSEED3 record exceeds 2**20 bytes");
				}

				if ( !fill(is, buffer, reclen) ) {
					error(is, "Fatal error occured while reading record from stream", p);
				}

				if ( !sid2nslc({ SID::Get(buffer.data()), sidLength }, _net, _sta, _loc, _cha) ) {
					throw Core::StreamException("Invalid SID");
				}

				_format = V3;
				_recordLength = reclen;
				_nsamp = C::ToLittleEndian(*SampleCount::Get(header));
				_fsamp = C::ToLittleEndian(*SamplingRate::Get(header));

				if ( _fsamp < 0 ) {
					_fsamp = -1.0 / _fsamp;
				}

				_stime = Core::Time::FromYearDay(
					C::ToLittleEndian(*Year::Get(header)),
					C::ToLittleEndian(*YDay::Get(header))
				);

				_stime += Core::TimeSpan(
					C::ToLittleEndian(*Hour::Get(header)) * 3600 +
					C::ToLittleEndian(*Minute::Get(header)) * 60 +
					C::ToLittleEndian(*Second::Get(header)),
					C::ToLittleEndian(*Nanoseconds::Get(header)) / 1000
				);

				_endTime = _stime + Core::TimeSpan(0, _nsamp * 1000000 / _fsamp + 0.5);

				if ( extraLength > 0 ) {
					rapidjson::Document doc;
					// Insitu parsing is valid as the buffer the record is pointing to is a
					// private temporary buffer created in read().
					// It will speed up the parser tremendously.
					doc.Parse(buffer.data() + HeaderLength + sidLength, extraLength);
					if ( !doc.HasParseError() ) {
						auto it = doc.FindMember("FDSN");
						if ( it != doc.MemberEnd() ) {
							it = it->value.FindMember("Time");
							if ( it != doc.MemberEnd() ) {
								it = it->value.FindMember("Quality");
								if ( it != doc.MemberEnd() ) {
									if ( it->value.IsInt() ) {
										_timequal = it->value.GetInt();
									}
								}
							}
						}
					}
					else {
						// Mmmhhh, what to do? Ignore the entire record or just the extra header
						// document?
					}
				}

				break;
			}
		}
	}

	if ( reclen <= 0 ) {
		if ( is.eof() ) {
			throw Core::EndOfStreamException();
		}
		else {
			throw LibmseedException("Retrieving the record length failed");
		}
	}

	if ( reclen < 40 ) {
		throw Core::EndOfStreamException("Invalid miniSEED record, too small");
	}

	if ( _fsamp <= 0 ) {
		throw LibmseedException("Unpacking of miniSEED record failed, invalid sample rate");
	}

	_encodingFlag = true;

	if ( _hint != SAVE_RAW ) {
		if ( _hint == DATA_ONLY ) {
			try {
				unpackData(_raw.typedData(), _raw.size());
			}
			catch ( LibmseedException &e ) {
				_raw.clear();
				_nsamp = 0;
				_fsamp = 0;
				_data = nullptr;
				throw;
			}
		}
		_raw.clear();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::write(std::ostream &out) {
	using namespace MSEED;

	if ( !_data ) {
		if ( !_raw.data() ) {
			throw Core::StreamException("No writable data found");
		}
		else {
			data();
		}
	}

	int freqn, freqd;
	if ( !getFraction(freqn, freqd, _fsamp) ) {
		SEISCOMP_ERROR("%s.%s.%s.%s: invalid sampling rate: %f",
		               _net, _sta, _loc, _cha, _fsamp);
		return;
	}

	auto format = new V2::StandardFormat(_net, _sta, _loc, _cha, freqn, freqd);
	auto exp = static_cast<int>(ceil(log2(_recordLength)) + 0.5);
	format->recordSize = exp ? exp : 9;
	format->bigEndian = !(_byteOrder & TargetLittleEndian);

	ArrayPtr data;
	EncoderPtr encoder;

	if ( _encodingFlag && (_encoding >= 0) ) {
		switch ( static_cast<EncodingType>(_encoding) ) {
			case EncodingType::ASCII:
				encoder = new Uncompressed<char*>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::CHAR, _data.get());
				break;
			case EncodingType::FLOAT32:
				encoder = new Uncompressed<float>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::FLOAT, _data.get());
				break;
			case EncodingType::FLOAT64:
				encoder = new Uncompressed<double>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::DOUBLE, _data.get());
				break;
			case EncodingType::INT16:
			case EncodingType::INT32:
				encoder = new Uncompressed<int32_t>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
			case EncodingType::STEIM1:
				encoder = new Steim1<int32_t>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
			case EncodingType::STEIM2:
				encoder = new Steim2<int32_t>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
			default:
				SEISCOMP_WARNING("Unknown encoding type found: %d, use to Steim2 "
				                 "encoding", static_cast<int>(_encoding), _encoding);
				encoder = new Steim2<int32_t>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
		}
	}
	else {
		data = _data;

		switch ( data->dataType() ) {
			case Array::CHAR:
				encoder = new Uncompressed<char*>(format, freqn, freqd);
				break;
			case Array::INT:
				encoder = new Steim2<int32_t>(format, freqn, freqd);
				break;
			case Array::FLOAT:
				encoder = new Uncompressed<float>(format, freqn, freqd);
				break;
			case Array::DOUBLE:
				encoder = new Uncompressed<double>(format, freqn, freqd);
				break;
			default:
				SEISCOMP_WARNING("Unknown data type %d, use Steim2 encoding",
				                 static_cast<int>(data->dataType()));
				encoder = new Steim2<int32_t>(format, freqn, freqd);
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
		}
	}

	if ( !data ) {
		SEISCOMP_ERROR("No data");
		return;
	}

	encoder->setBufferCallback([&out](CharArray *buf) {
		out.write(buf->typedData(), buf->size());
	});
	encoder->setTime(_stime);
	encoder->setTimingQuality(_timequal);
	encoder->push(data->size(), data->data());
	encoder->flush();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
