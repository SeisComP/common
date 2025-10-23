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


#define SEISCOMP_COMPONENT MSEEDRECORD
#include <seiscomp/logging/log.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/core/arrayfactory.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/utils/certstore.h>

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

#include <libmseed.h>
#include <mseedformat.h>

#include <cctype>
#include <cstring>
#include <string_view>


// Some additional MS2 defines
#define pMS2B1000_SIZE 4


namespace Seiscomp::IO {


namespace {


/* callback function for libmseed-function msr3_pack(...) */
void _Record_Handler(char *record, int reclen, void *out) {
	reinterpret_cast<std::ostream*>(out)->write(record, reclen);
}


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


struct MSEEDLogger {
	MSEEDLogger() {
		ms_loginit(MSEEDLogger::print, "[libmseed] ", MSEEDLogger::diag, "[libmseed] ");
	}

	static void print(const char */*msg*/) {
		// SEISCOMP_DEBUG("%s", msg);
	}
	static void diag(const char */*msg*/) {
		// SEISCOMP_WARNING("%s", msg);
	}
};


static MSEEDLogger __logger__;


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IMPLEMENT_SC_CLASS_DERIVED(MSeedRecord, Record, "MSeedRecord");
REGISTER_RECORD(MSeedRecord, "mseed");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(Array::DataType dt, Hint h)
: Record(dt, h) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(MSRecord *rec, Array::DataType dt, Hint h)
: Record(dt, h) {
	set(rec);
}
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
			auto *netHeader = pMS2FSDH_NETWORK(_raw.typedData());
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
			auto *staHeader = pMS2FSDH_STATION(_raw.typedData());
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
			auto *locHeader = pMS2FSDH_LOCATION(_raw.typedData());
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
			auto *chaHeader = pMS2FSDH_CHANNEL(_raw.typedData());
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
	if ( _hint == SAVE_RAW ) {
		if ( _format == V2 ) {
			bool swapflag = _byteOrder & MSSWAP_HEADER;
			int year, yday, hour, minute, second, microsecond;
			time.get2(&year, &yday, &hour, &minute, &second, &microsecond);

			uint16_t tmp;

			tmp = year; tmp = HO2u(tmp, swapflag);
			*pMS2FSDH_YEAR(_raw.typedData()) = tmp;

			tmp = yday + 1; tmp = HO2u(tmp, swapflag);
			*pMS2FSDH_DAY(_raw.typedData()) = tmp;

			*pMS2FSDH_HOUR(_raw.typedData()) = uint8_t(hour);
			*pMS2FSDH_MIN(_raw.typedData()) = uint8_t(minute);
			*pMS2FSDH_SEC(_raw.typedData()) = uint8_t(second);
			*pMS2FSDH_UNUSED(_raw.typedData()) = 0;
			tmp = microsecond / 100; tmp = HO2u(tmp, swapflag);
			*pMS2FSDH_FSEC(_raw.typedData()) = tmp;
		}
	}

	Record::setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time MSeedRecord::TimeFromNST(int64_t t) {
	return Core::Time::FromEpoch(t / NSTMODULUS, (t % NSTMODULUS) / (NSTMODULUS / 1000000));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord &MSeedRecord::operator=(const MSeedRecord &msrec) {
	if ( &msrec != this ) {
		Record::operator=(msrec);
		_format = msrec._format;
		_raw = msrec._raw;
		_data = msrec._data ? msrec._data->clone() : nullptr;
		_recordType = msrec._recordType;
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
	_recordType = qual;
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
		unpackData(_recordLength, const_cast<char*>(_raw.typedData()));
	}

	return _data.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::unpackData(int reclen, const char *data) const {
	MSRecord *pmsr = nullptr;

	if ( !data ) {
		return;
	}

	int r = msr3_parse(data, reclen, &pmsr, MSF_UNPACKDATA, 0);
	if ( r != MS_NOERROR ) {
		throw LibmseedException(fmt::format("Unpacking of MiniSEED record failed: {}", r));
	}

	try {
		unpackData(pmsr);
	}
	catch ( ... ) {
		msr3_free(&pmsr);
		throw;
	}

	msr3_free(&pmsr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::unpackData(const MSRecord *pmsr) const {
	Array::DataType dt = _datatype;
	_data = nullptr;

	if ( pmsr->numsamples == _nsamp ) {
		switch ( pmsr->sampletype ) {
			case 'i':
				_data = ArrayFactory::Create(dt, Array::INT, _nsamp, pmsr->datasamples);
				break;
			case 'f':
				_data = ArrayFactory::Create(dt, Array::FLOAT, _nsamp, pmsr->datasamples);
				break;
			case 'd':
				if ( dt < Array::DOUBLE ) {
					// We need double precision in order to store doubles.
					dt = Array::DOUBLE;
				}
				_data = ArrayFactory::Create(dt, Array::DOUBLE, _nsamp, pmsr->datasamples);
				break;
			case 'a':
			case 't':
				_data = ArrayFactory::Create(dt, Array::CHAR, _nsamp, pmsr->datasamples);
				break;
		}
	}
	else {
		throw LibmseedException("The number of the unpacked data samples differs from the sample number in fixed data header.");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::set(int reclen, const char *data) {
	MSRecord *prec{};
	int r = msr3_parse(data, reclen, &prec, _hint == DATA_ONLY ? MSF_UNPACKDATA : 0, 0);
	if ( r != MS_NOERROR ) {
		throw LibmseedException(fmt::format("Unpacking of MiniSEED record failed: {}", r));
	}

	set(prec);
	msr3_free(&prec);
	if ( _fsamp <= 0 ) {
		throw LibmseedException("Unpacking of MiniSEED record failed, invalid sample rate");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::set(const MSRecord *rec) noexcept {
	char net[9];
	char sta[9];
	char loc[9];
	char cha[4];
	ms_sid2nslc_n(rec->sid, net, sizeof(net), sta, sizeof(sta), loc, sizeof(loc), cha, sizeof(cha));
	_net = net; _sta = sta; _loc = loc; _cha = cha;

	_stime = TimeFromNST(rec->starttime);

	_byteOrder = rec->swapflag;
	_format = rec->formatversion == 2 ? V2 : V3;
	_recordLength = rec->reclen;
	_nsamp = rec->samplecnt;
	_encodingFlag = true;

	if ( rec->samprate < 0 ) {
		// Negative sampling rates are sampling distances.
		_fsamp = 1.0 / rec->samprate;
	}
	else {
		_fsamp = rec->samprate;
	}

	_data = nullptr;
	_recordType = rec->sampletype;
	_encoding = rec->encoding;
	_encodingFlag = true;

	if ( rec->extra && (rec->extralength > 0) ) {
		rapidjson::Document doc;
		doc.Parse(rec->extra, rec->extralength);
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


	if ( _hint == SAVE_RAW ) {
		_raw.setData(rec->reclen, rec->record);
	}
	else {
		_raw.clear();

		if ( _hint == DATA_ONLY ) {
			try {
				unpackData(rec);
			}
			catch ( LibmseedException &e ) {
				_nsamp = 0;
				_fsamp = 0;
				_data = nullptr;
				SEISCOMP_ERROR("%s", e.what());
			}
		}
	}

	_endTime = TimeFromNST(msr3_endtime(rec));

	// Check authentication
	Util::CertificateStore &cs = Util::CertificateStore::global();
	if ( (rec->formatversion == 2) && cs.isValid() ) {
		bool isSigned = false;
		bool swapflag = _byteOrder & MSSWAP_HEADER;

		auto blkt_offset = HO2u(*pMS2FSDH_BLOCKETTEOFFSET(rec->record), swapflag);
		uint16_t blkt_type;
		uint16_t next_blkt;

		while ( blkt_offset != 0 ) {
			blkt_type = HO2u(*reinterpret_cast<const uint16_t*>(rec->record + blkt_offset), swapflag);
			next_blkt = HO2u(*reinterpret_cast<const uint16_t*>(rec->record + blkt_offset + 2), swapflag);

			if ( blkt_type == 2000 ) {
				auto *blkt = rec->record + blkt_offset;

				if ( *pMS2B2000_NUMHEADERS(blkt) != 1 ) {
					continue;
				}

				if ( HO2u(*pMS2B2000_DATAOFFSET(blkt), swapflag) < 21 ) {
					continue;
				}

				auto b2000DataOffset = HO2u(*pMS2B2000_DATAOFFSET(blkt), swapflag);
				auto b2000DataLength = HO2u(*pMS2B2000_LENGTH(blkt), swapflag);
				size_t maxHeaderLength = b2000DataOffset - 15;
				const char *opaqHeaders = pMS2B2000_PAYLOAD(blkt);
				std::string_view header;
				for ( size_t i = 0; i < maxHeaderLength; ++i ) {
					if ( opaqHeaders[i] == '~' ) {
						header = { opaqHeaders, i };
						break;
					}
				}

				if ( header.empty() ) {
					continue;
				}

				if ( !header.compare(0, 5, "SIGN/") ) {
					header = header.substr(5);
					size_t nSignature = b2000DataLength - b2000DataOffset;

					unsigned char digest[EVP_MAX_MD_SIZE];
					unsigned int nDigest = 0;

					EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
					EVP_DigestInit(mdctx, EVP_sha256());
					EVP_DigestUpdate(mdctx, pMS2FSDH_DATAQUALITY(rec->record), 1);
					EVP_DigestUpdate(
						mdctx,
						pMS2FSDH_STATION(rec->record),
						(char*)pMS2FSDH_DATAOFFSET(rec->record) - (char*)pMS2FSDH_STATION(rec->record)
					);
					EVP_DigestUpdate(
						mdctx,
						rec->record + HO2u(*pMS2FSDH_DATAOFFSET(rec->record), swapflag),
						rec->reclen - HO2u(*pMS2FSDH_DATAOFFSET(rec->record), swapflag)
					);
					EVP_DigestFinal_ex(mdctx, reinterpret_cast<unsigned char*>(digest), &nDigest);
					EVP_MD_CTX_destroy(mdctx);

					const unsigned char *pp = reinterpret_cast<const unsigned char *>(blkt) + b2000DataOffset;

					isSigned = true;
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
										const_cast<MSeedRecord*>(this)->_authority.assign(reinterpret_cast<char*>(ASN1_STRING_data(str)), size_t(ASN1_STRING_length(str)));
#else
										const_cast<MSeedRecord*>(this)->_authority.assign(reinterpret_cast<const char*>(ASN1_STRING_get0_data(str)), ASN1_STRING_length(str));
#endif
									}
								}
								else {
									SEISCOMP_WARNING("MSEED: Failed to extract certificate authority (O)");
								}
							}
						}

						const_cast<MSeedRecord*>(this)->_authenticationStatus = SIGNATURE_VALIDATED;
					}
				}
			}

			blkt_offset = next_blkt;
		}

		if ( !isSigned ) {
			const_cast<MSeedRecord*>(this)->_authenticationStatus = NOT_SIGNED;
		}
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
#define HEADER_BLOCK_LEN MINRECLEN
	int reclen = -1;
	bool swapflag = false;
	char header[HEADER_BLOCK_LEN];
	std::vector<char> buffer;

	while ( is.good() ) {
		if ( is.read(header, sizeof(header)) ) {
			if ( MS2_ISVALIDHEADER(header) ) {
				buffer.reserve(512);
				buffer.resize(sizeof(header));
				memcpy(buffer.data(), header, sizeof(header));

				if constexpr ( sizeof(header) < 64 ) {
					auto p = is.tellg();
					if ( !fill(is, buffer, 64) ) {
						error(is, "Blockette reading error", p);
					}
				}

				// Remember current position
				auto p = is.tellg();

				if ( !MS_ISVALIDYEARDAY(*pMS2FSDH_YEAR(buffer.data()), *pMS2FSDH_DAY(buffer.data())) ) {
					swapflag = true;
				}

				auto blkt_offset = HO2u(*pMS2FSDH_BLOCKETTEOFFSET(buffer.data()), swapflag);
				uint16_t blkt_type;
				uint16_t next_blkt;

				while ( blkt_offset != 0 ) {
					if ( !fill(is, buffer, blkt_offset + 4) ) {
						error(is, "Blockette reading error", p);
					}

					blkt_type = HO2u(*reinterpret_cast<uint16_t*>(buffer.data() + blkt_offset), swapflag);
					next_blkt = HO2u(*reinterpret_cast<uint16_t*>(buffer.data() + blkt_offset + 2), swapflag);

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
						if ( !fill(is, buffer, blkt_offset + 4 + pMS2B1000_SIZE) ) {
							error(is, "Blockette 1000 reading error", p);
						}

						reclen = static_cast<uint64_t>(1) << *pMS2B1000_RECLEN (buffer.data() + blkt_offset);
						break;
					}

					blkt_offset = next_blkt;
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
						if ( MS2_ISVALIDHEADER(header) || _isHeader(header) ) {
							reclen = buffer.size();
							// Set current stream position back to header start
							is.seekg(-sizeof(header), std::ios::cur);
							break;
						}
						else {
							if ( buffer.size() < MAXRECLENv2 ) {
								buffer.resize(buffer.size() + sizeof(header));
								memcpy(buffer.data() + buffer.size() - sizeof(header), header, sizeof(header));
							}
							else {
								error(is, "MiniSEED Record exceeds " STR(MAXRECLENv2) " bytes", p);
							}
						}
					}

					if ( is.eof() and isPowerOfTwo(buffer.size()) ) {
						reclen = buffer.size();
					}
				}

				if ( reclen > 0 ) {
					if ( reclen > MAXRECLEN ) {
						throw Core::StreamException("MiniSEED Record exceeds 2**20 bytes");
					}

					if ( !fill(is, buffer, reclen) ) {
						error(is, "Fatal error occured while reading record from stream", p);
					}

					break;
				}
			}
		}

		// Skip over to the next header
	}

	if ( reclen <= 0 ) {
		if ( is.eof() ) {
			throw Core::EndOfStreamException();
		}
		else {
			throw LibmseedException("Retrieving the record length failed");
		}
	}

	if ( reclen < MINRECLEN ) {
		throw Core::EndOfStreamException("Invalid MiniSEED record, too small");
	}

	set(reclen, buffer.data());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::write(std::ostream &out) {
	if ( !_data ) {
		if ( !_raw.data() ) {
			throw Core::StreamException("No writable data found");
		}
		else {
			data();
		}
	}

	MSRecord *pmsr;
	pmsr = msr3_init(nullptr);
	if (!pmsr) {
		throw Core::StreamException("msr_init failed");
	}

	pmsr->reclen = _recordLength ? _recordLength : 512;
	pmsr->formatversion = 2;

	ms_nslc2sid(
		pmsr->sid, sizeof(pmsr->sid), 0,
		_net.data(), _sta.data(), _loc.data(), _cha.data()
	);

	// Convert to nanoseconds
	pmsr->starttime = startTime().repr().time_since_epoch().count() * 1000;
	pmsr->samprate = _fsamp;
	pmsr->sampletype = _recordType;
	pmsr->numsamples = _data->size();

	ArrayPtr data;

	if ( _encodingFlag && (_encoding >= 0) ) {
		switch ( _encoding ) {
			case DE_ASCII: {
				pmsr->encoding = DE_ASCII;
				pmsr->sampletype = 'a';
				data = ArrayFactory::Create(Array::CHAR, _data.get());
				break;
			}
			case DE_FLOAT32: {
				pmsr->encoding = DE_FLOAT32;
				pmsr->sampletype = 'f';
				data = ArrayFactory::Create(Array::FLOAT, _data.get());
				break;
			}
			case DE_FLOAT64: {
				pmsr->encoding = DE_FLOAT64;
				pmsr->sampletype = 'd';
				data = ArrayFactory::Create(Array::DOUBLE, _data.get());
				break;
			}
			case DE_INT16:
			case DE_INT32:
			case DE_STEIM1:
			case DE_STEIM2: {
				pmsr->encoding = _encoding;
				pmsr->sampletype = 'i';
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
			}
			default:
				SEISCOMP_WARNING("Unknown encoding type found %s(%c)! Switch to Integer-Steim2 encoding", ms_encodingstr(_encoding), _encoding);
				pmsr->encoding = DE_STEIM2;
				pmsr->sampletype = 'i';
				data = ArrayFactory::Create(Array::INT, _data.get());
				break;
		}
	}
	else {
		data = _data;

		switch ( _data->dataType() ) {
			case Array::CHAR:
				pmsr->encoding = DE_ASCII;
				pmsr->sampletype = 't';
				break;
			case Array::INT:
				pmsr->encoding = DE_STEIM2;
				pmsr->sampletype = 'i';
				break;
			case Array::FLOAT:
				pmsr->encoding = DE_FLOAT32;
				pmsr->sampletype = 'f';
				break;
			case Array::DOUBLE:
				pmsr->encoding = DE_FLOAT64;
				pmsr->sampletype = 'd';
				break;
			default:
				SEISCOMP_WARNING("Unknown data type %d! Switch to Integer-Steim2 encoding",
				                 static_cast<int>(data->dataType()));
				pmsr->encoding = DE_STEIM2;
				pmsr->sampletype = 'i';
				data = ArrayFactory::Create(Array::INT,_data.get());
				break;
		}
	}

	pmsr->datasamples = const_cast<void*>(data->data());
	pmsr->datasize = data->size() * data->elementSize();
	if ( _timequal > 0 ) {
		auto json = Core::stringify("{\"FDSN\":{\"Time\":{\"Quality\":%d}}}", _timequal);
		pmsr->extra = reinterpret_cast<char*>(libmseed_memory.malloc(json.size() + 1));
		pmsr->extralength = json.size();
		memcpy(pmsr->extra, json.data(), pmsr->extralength + 1);
	}

	int64_t psamples;
	msr3_pack(pmsr, _Record_Handler, &out, &psamples, MSF_FLUSHDATA, 0);
	pmsr->datasamples = nullptr;
	msr3_free(&pmsr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
