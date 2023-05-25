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
#include <seiscomp/utils/certstore.h>

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
#include <cctype>


namespace Seiscomp {
namespace IO {

namespace {


/* callback function for libmseed-function msr_pack(...) */
void _Record_Handler(char *record, int reclen, void *packed) {
	/* to make the data available to the overloaded operator<< */
	reinterpret_cast<CharArray *>(packed)->append(reclen, record);
}


bool isPowerOfTwo(size_t n) {
	if ( !n ) return false;

	while ( n != 1 ) {
		if ( n % 2 ) return false;
		n >>= 1;
	}

	return true;
}


struct MSEEDLogger {
	MSEEDLogger() {
		ms_loginit(MSEEDLogger::print, "[libmseed] ", MSEEDLogger::diag, "[libmseed] ");
	}

	static void print(char */*msg*/) {
		// SEISCOMP_DEBUG("%s", msg);
	}
	static void diag(char */*msg*/) {
		// SEISCOMP_WARNING("%s", msg);
	}
};


static MSEEDLogger __logger__;


}


IMPLEMENT_SC_CLASS_DERIVED(MSeedRecord, Record, "MSeedRecord");
REGISTER_RECORD(MSeedRecord, "mseed");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(Array::DataType dt, Hint h)
: Record(dt, h)
, _data(0)
, _seqno(0)
, _rectype('D')
, _srfact(0)
, _srmult(0)
, _byteorder(0)
, _encoding(0)
, _srnum(0)
, _srdenom(0)
, _reclen(0)
, _nframes(0)
, _leap(0)
, _etime(Seiscomp::Core::Time(0,0))
, _encodingFlag(true)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(MSRecord *rec, Array::DataType dt, Hint h)
: Record(dt, h, rec->network, rec->station, rec->location, rec->channel,
         Seiscomp::Core::Time(hptime_t(rec->starttime / HPTMODULUS),
                              hptime_t(rec->starttime % HPTMODULUS)),
         int(rec->samplecnt), rec->samprate, rec->Blkt1001 ? rec->Blkt1001->timing_qual : -1)
, _data(0)
, _seqno(rec->sequence_number)
, _rectype(rec->dataquality)
, _srfact(rec->fsdh->samprate_fact)
, _srmult(rec->fsdh->samprate_mult)
, _byteorder(rec->byteorder)
, _encoding(rec->encoding)
, _reclen(rec->reclen)
, _encodingFlag(true)
{
	if ( _hint == SAVE_RAW ) {
		_raw.setData(rec->reclen,rec->record);
	}
	else if ( _hint == DATA_ONLY ) {
		try {
			_setDataAttributes(rec->reclen, rec->record);
		}
		catch ( LibmseedException &e ) {
			_nsamp = 0;
			_fsamp = 0;
			_data = nullptr;
			SEISCOMP_ERROR("%s", e.what());
		}
	}

	_srnum = 0;
	_srdenom = 1;

	if ( (_srfact > 0) && (_srmult > 0) ) {
		_srnum = _srfact*_srmult;
		_srdenom = 1;
	}

	if ( _srfact > 0 && _srmult < 0 ) {
		_srnum = _srfact;
		_srdenom = -_srmult;
	}

	if ( (_srfact < 0) && (_srmult > 0) ) {
		_srnum = _srmult;
		_srdenom = -_srfact;
	}

	if ( (_srfact < 0) && (_srmult < 0) ) {
		_srnum = 1;
		_srdenom = _srfact*_srmult;
	}

	_nframes = 0;
	if ( rec->Blkt1001 ) {
		_nframes = rec->Blkt1001->framecnt;
	}

	_leap = 0;
	if ( rec->fsdh->start_time.sec > 59 ) {
		_leap = rec->fsdh->start_time.sec-59;
	}

	hptime_t hptime = msr_endtime(rec);
	_etime = Seiscomp::Core::Time(hptime_t(hptime / HPTMODULUS), hptime_t(hptime % HPTMODULUS));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(const MSeedRecord &msrec)
: Record(msrec)
, _seqno(msrec.sequenceNumber())
, _rectype(msrec.dataQuality())
, _srfact(msrec.sampleRateFactor())
, _srmult(msrec.sampleRateMultiplier())
, _byteorder(msrec.byteOrder())
, _encoding(msrec.encoding())
, _srnum(msrec.sampleRateNumerator())
, _srdenom(msrec.sampleRateDenominator())
, _nframes(msrec.frameNumber())
, _leap(msrec.leapSeconds())
, _etime(msrec.endTime())
, _encodingFlag(true)
{
	_reclen = msrec._reclen;
	_raw = msrec._raw;
	_data = msrec._data?msrec._data->clone():nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::MSeedRecord(const Record &rec, int reclen)
: Record(rec)
, _seqno(0)
, _rectype('D')
, _srfact(0)
, _srmult(0)
, _byteorder(0)
, _encoding(0)
, _srnum(0)
, _srdenom(0)
, _nframes(0)
, _leap(0)
, _etime(Seiscomp::Core::Time())
, _encodingFlag(false)
{
	_reclen = reclen;
	_data = rec.data()?rec.data()->clone():nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord::~MSeedRecord() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setNetworkCode(std::string net) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[3];
		strncpy(tmp, net.c_str(), 2);
		memcpy(header->network, tmp, 2);
	}

	Record::setNetworkCode(net);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setStationCode(std::string sta) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[6];
		strncpy(tmp, sta.c_str(), 5);
		memcpy(header->station, tmp, 5);
	}

	Record::setStationCode(sta);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setLocationCode(std::string loc) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[3];
		strncpy(tmp, loc.c_str(), 2);
		memcpy(header->location, tmp, 2);
	}

	Record::setLocationCode(loc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setChannelCode(std::string cha) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		char tmp[4];
		strncpy(tmp, cha.c_str(), 3);
		memcpy(header->channel, tmp, 3);
	}

	Record::setChannelCode(cha);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setStartTime(const Core::Time& time) {
	if ( _hint == SAVE_RAW ) {
		struct fsdh_s *header = reinterpret_cast<struct fsdh_s *>(_raw.typedData());
		hptime_t hptime = hptime_t(time.seconds() * HPTMODULUS) + hptime_t(time.microseconds());
		ms_hptime2btime(hptime, &header->start_time);
		MS_SWAPBTIME(&header->start_time);
	}

	Record::setStartTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MSeedRecord& MSeedRecord::operator=(const MSeedRecord &msrec) {
	if ( &msrec != this ) {
		Record::operator=(msrec);
		_raw = msrec._raw;
		_data = msrec._data?msrec._data->clone():nullptr;
		_seqno = msrec.sequenceNumber();
		_rectype = msrec.dataQuality();
		_srfact = msrec.sampleRateFactor();
		_srmult = msrec.sampleRateMultiplier();
		_byteorder = msrec.byteOrder();
		_encoding = msrec.encoding();
		_srnum = msrec.sampleRateNumerator();
		_srdenom = msrec.sampleRateDenominator();
		_reclen = msrec._reclen;
		_nframes = msrec.frameNumber();
		_leap = msrec.leapSeconds();
		_etime = msrec.endTime();
	}

	return (*this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::sequenceNumber() const {
	return _seqno;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setSequenceNumber(int seqno) {
	_seqno = seqno;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char MSeedRecord::dataQuality() const {
	return _rectype;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setDataQuality(char qual) {
	_rectype = qual;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::sampleRateFactor() const {
	return _srfact;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setSampleRateFactor(int srfact) {
	_srfact = srfact;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::sampleRateMultiplier() const {
	return _srmult;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::setSampleRateMultiplier(int srmult) {
	_srmult = srmult;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int8_t MSeedRecord::byteOrder() const {
	return _byteorder;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int8_t MSeedRecord::encoding() const {
	return _encoding;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::sampleRateNumerator() const {
	return _srnum;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::sampleRateDenominator() const {
	return _srdenom;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::frameNumber() const {
	return _nframes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::Time& MSeedRecord::endTime() const {
	return _etime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::recordLength() const {
	return _reclen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MSeedRecord::leapSeconds() const {
	return _leap;
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
		_setDataAttributes(_reclen, const_cast<char*>(_raw.typedData()));
	}

	return _data.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::_setDataAttributes(int reclen, char *data) const {
	MSRecord *pmsr = nullptr;

	if ( !data ) return;

	int r = msr_unpack(data, reclen, &pmsr, 1, 0);
	if ( r != MS_NOERROR ) {
		throw LibmseedException(fmt::format("Unpacking of Mini SEED record failed: {}", r));
	}

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
				_data = ArrayFactory::Create(dt, Array::CHAR, _nsamp, pmsr->datasamples);
				break;
		}

		// Check authentication
		Util::CertificateStore &cs = Util::CertificateStore::global();
		if ( cs.isValid() ) {
			bool isSigned = false;
			BlktLink *blk = pmsr->blkts;
			for ( ; blk; blk = blk->next ) {
				if ( blk->blkt_type == 2000 ) {
					blkt_2000_s *opaq = reinterpret_cast<blkt_2000_s*>(blk->blktdata);
					if ( opaq->numheaders != 1 ) continue;

					int maxHeaderLength = opaq->data_offset - 15;
					if ( maxHeaderLength < 6 ) continue;

					const char *opaqHeaders = opaq->payload;
					std::string header;
					for ( int i = 0; i < maxHeaderLength; ++i ) {
						if ( opaqHeaders[i] == '~' ) {
							header.assign(opaqHeaders, opaqHeaders + i);
							break;
						}
					}

					if ( header.empty() ) continue;

					if ( !strncmp(header.c_str(), "SIGN/", 5) ) {
						header.erase(header.begin(), header.begin() + 5);
						long lenSignature = opaq->length - opaq->data_offset;

						unsigned char digest_buffer[EVP_MAX_MD_SIZE];
						unsigned int digest_len;

						EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
						EVP_DigestInit(mdctx, EVP_sha256());
						EVP_DigestUpdate(mdctx, pmsr->record + offsetof(struct fsdh_s, dataquality), sizeof(pmsr->fsdh->dataquality));
						EVP_DigestUpdate(mdctx, pmsr->record + offsetof(struct fsdh_s, station), offsetof(struct fsdh_s, data_offset) - offsetof(struct fsdh_s, station));
						EVP_DigestUpdate(mdctx, pmsr->record + pmsr->fsdh->data_offset, (1 << pmsr->Blkt1000->reclen) - pmsr->fsdh->data_offset);
						EVP_DigestFinal_ex(mdctx, reinterpret_cast<unsigned char*>(digest_buffer), &digest_len);

						EVP_MD_CTX_destroy(mdctx);

						const unsigned char *pp = reinterpret_cast<const unsigned char *>(opaq) + opaq->data_offset - 4;
						ECDSA_SIG *signature = d2i_ECDSA_SIG(nullptr, &pp, lenSignature);
						if ( !signature ) {
							SEISCOMP_ERROR("MSEED: Failed to extract signature from opaque headers");
							continue;
						}

						isSigned = true;
						const X509 *cert;
						if ( !cs.validate(header, reinterpret_cast<const char*>(digest_buffer),
						                  digest_len, signature, &cert) ) {
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
											if ( length > 0 )
												const_cast<MSeedRecord*>(this)->_authority.assign(reinterpret_cast<char*>(utf8), size_t(length));
											if ( utf8 )
												OPENSSL_free(utf8);
										}
										else {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
											const_cast<MSeedRecord*>(this)->_authority.assign(reinterpret_cast<char*>(ASN1_STRING_data(str)), size_t(ASN1_STRING_length(str)));
#else
											const_cast<MSeedRecord*>(this)->_authority.assign(reinterpret_cast<const char*>(ASN1_STRING_get0_data(str)), ASN1_STRING_length(str));
#endif
										}
									}
									else
										SEISCOMP_WARNING("MSEED: Failed to extract certificate authority (O)");
								}
							}

							const_cast<MSeedRecord*>(this)->_authenticationStatus = SIGNATURE_VALIDATED;
						}

						ECDSA_SIG_free(signature);
					}
				}
			}

			if ( !isSigned )
				const_cast<MSeedRecord*>(this)->_authenticationStatus = NOT_SIGNED;
		}
	}
	else {
		msr_free(&pmsr);
		throw LibmseedException("The number of the unpacked data samples differs from the sample number in fixed data header.");
	}

	msr_free(&pmsr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::saveSpace() const {
	if (_hint == SAVE_RAW && _data) {
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
	_reclen = reclen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool _isHeader(const char *header) {
	return (std::isdigit(*(header+0))
	     && std::isdigit(*(header+1))
	     && std::isdigit(*(header+2))
	     && std::isdigit(*(header+3))
	     && std::isdigit(*(header+4))
	     && std::isdigit(*(header+5))
	     && std::isalpha(*(header+6))
	     && (*(header+7) == ' ' || *(header+7) == '\0'));
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
#define HEADER_BLOCK_LEN 64
	int reclen = -1;
	MSRecord *prec = nullptr;
	bool swapflag = false;
	char header[HEADER_BLOCK_LEN];
	std::vector<char> buffer;

	while ( is.good() ) {
		if ( is.read(header, sizeof(header))
		 and MS_ISVALIDHEADER(header) ) {
			buffer.reserve(512);
			buffer.resize(sizeof(header));
			memcpy(buffer.data(), header, sizeof(header));

			// Remember current position
			size_t p = is.tellg();

			fsdh_s *fsdh = reinterpret_cast<fsdh_s*>(buffer.data());
			uint16_t blkt_offset;
			uint16_t blkt_type;
			uint16_t next_blkt;

			if ( !MS_ISVALIDYEARDAY(fsdh->start_time.year, fsdh->start_time.day) )
				swapflag = true;

			blkt_offset = fsdh->blockette_offset;
			if ( swapflag ) ms_gswap2(&blkt_offset);

			while ( blkt_offset != 0 ) {
				if ( !fill(is, buffer, blkt_offset + 4) )
					error(is, "Blockette reading error", p);

				blkt_type = *reinterpret_cast<uint16_t*>(buffer.data() + blkt_offset);
				next_blkt = *reinterpret_cast<uint16_t*>(buffer.data() + blkt_offset + 2);

				if ( swapflag ) {
					ms_gswap2(&blkt_type);
					ms_gswap2(&next_blkt);
				}

				if ( next_blkt != 0 ) {
					if ( (next_blkt < 4 || (next_blkt - 4) <= blkt_offset) )
						error(is, "Invalid blockette offset less than or equal to current offset", p);

					if ( !fill(is, buffer, next_blkt) )
						error(is, "Blockette reading error", p);
				}

				// Found a 1000 blockette
				if ( blkt_type == 1000 ) {
					if ( !fill(is, buffer, blkt_offset + 4 + sizeof(blkt_1000_s)) )
						error(is, "Blockette 1000 reading error", p);

					blkt_1000_s *blkt_1000;blkt_1000 = reinterpret_cast<blkt_1000_s*>(buffer.data() + blkt_offset + 4);
					reclen = (unsigned int)1 << blkt_1000->reclen;
					break;
				}

				blkt_offset = next_blkt;
			}

			if ( reclen < 0 ) {
				// Read current record to next power of two
				size_t oldSize = buffer.size();
				size_t newSize = 1;

				while ( newSize < oldSize )
					newSize <<= 1;

				if ( !fill(is, buffer, newSize) )
					error(is, "Data read error", p);

				while ( is.read(header, sizeof(header)) ) {
					if ( MS_ISVALIDHEADER(header)
					  or MS_ISVALIDBLANK(header)
					  or _isHeader(header) ) {
						reclen = buffer.size();
						// Set current stream position back to header start
						is.seekg(-sizeof(header), std::ios::cur);
						break;
					}
					else {
						if ( buffer.size() < MAXRECLEN ) {
							buffer.resize(buffer.size() + sizeof(header));
							memcpy(buffer.data() + buffer.size() - sizeof(header), header, sizeof(header));
						}
						else
							error(is, "Mini SEED Record exceeds 2**20 bytes", p);
					}
				}

				if ( is.eof() and isPowerOfTwo(buffer.size()) )
					reclen = buffer.size();
			}

			if ( reclen > 0 ) {
				if ( reclen > MAXRECLEN )
					throw Core::StreamException("Mini SEED Record exceeds 2**20 bytes");

				if ( !fill(is, buffer, reclen) )
					error(is, "Fatal error occured while reading record from stream", p);

				break;
			}
		}

		// Skip over to the next header
	}

	if ( reclen <= 0 ) {
		if ( is.eof() )
			throw Core::EndOfStreamException();
		else
			throw LibmseedException("Retrieving the record length failed");
	}

	if ( reclen < MINRECLEN ) {
		throw Core::EndOfStreamException("Invalid Mini SEED record, too small");
	}

	int r = msr_unpack(buffer.data(), reclen, &prec, 0, 0);
	if ( r != MS_NOERROR ) {
		throw LibmseedException(fmt::format("Unpacking of Mini SEED record failed: {}", r));
	}

	*this = MSeedRecord(prec, this->_datatype, this->_hint);
	msr_free(&prec);
	if ( _fsamp <= 0 ) {
		throw LibmseedException("Unpacking of Mini SEED record failed, invalid sample rate");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedRecord::write(std::ostream& out) {
	if (!_data) {
		if (!_raw.data())
			throw Core::StreamException("No writable data found");
		else
			data();
	}

	MSRecord *pmsr;
	pmsr = msr_init(nullptr);
	if (!pmsr)
		throw Core::StreamException("msr_init failed");

	/* Set MSRecord header values */
	pmsr->reclen = _reclen;
	pmsr->sequence_number = _seqno;
	strcpy(pmsr->network,_net.c_str());
	strcpy(pmsr->station,_sta.c_str());
	strcpy(pmsr->location,_loc.c_str());
	strcpy(pmsr->channel,_cha.c_str());
	pmsr->dataquality = _rectype;
	pmsr->starttime = ms_timestr2hptime(const_cast<char *>(_stime.iso().c_str()));
	pmsr->samprate = _fsamp;
	pmsr->byteorder = 1;
	pmsr->numsamples = _data->size();
	ArrayPtr data;

	struct blkt_1001_s blkt1001;
	memset(&blkt1001, 0, sizeof (struct blkt_1001_s));

	if ( _timequal >= 0 )
		blkt1001.timing_qual = _timequal <= 100 ? uint8_t(_timequal) : 100;

	if ( !msr_addblockette(pmsr, reinterpret_cast<char *>(&blkt1001), sizeof(struct blkt_1001_s), 1001, 0) ) {
		throw LibmseedException("Error adding 1001 blockette");
	}

	if ( _encodingFlag ) {
		switch ( _encoding ) {
			case DE_ASCII: {
				pmsr->encoding = DE_ASCII;
				pmsr->sampletype = 'a';
				data = ArrayFactory::Create(Array::CHAR, _data.get());
				pmsr->datasamples = const_cast<void*>(data->data());
				break;
			}
			case DE_FLOAT32: {
				pmsr->encoding = DE_FLOAT32;
				pmsr->sampletype = 'f';
				data = ArrayFactory::Create(Array::FLOAT, _data.get());
				pmsr->datasamples = const_cast<void*>(data->data());
				break;
			}
			case DE_FLOAT64: {
				pmsr->encoding = DE_FLOAT64;
				pmsr->sampletype = 'd';
				data = ArrayFactory::Create(Array::DOUBLE, _data.get());
				pmsr->datasamples = const_cast<void*>(data->data());
				break;
			}
			case DE_INT16:
			case DE_INT32:
			case DE_STEIM1:
			case DE_STEIM2: {
				pmsr->encoding = _encoding;
				pmsr->sampletype = 'i';
				data = ArrayFactory::Create(Array::INT, _data.get());
				pmsr->datasamples = const_cast<void*>(data->data());
				break;
			}
			default: {
				SEISCOMP_WARNING("Unknown encoding type found %s(%c)! Switch to Integer-Steim2 encoding", ms_encodingstr(_encoding), _encoding);
				pmsr->encoding = DE_STEIM2;
				pmsr->sampletype = 'i';
				data = ArrayFactory::Create(Array::INT, _data.get());
				pmsr->datasamples = const_cast<void*>(data->data());
			}
		}
	}
	else {
		switch ( _data->dataType() ) {
			case Array::CHAR:
				pmsr->encoding = DE_ASCII;
				pmsr->sampletype = 'a';
				pmsr->datasamples = const_cast<void*>(_data->data());
				break;
			case Array::INT:
				pmsr->encoding = DE_STEIM2;
				pmsr->sampletype = 'i';
				pmsr->datasamples = const_cast<void*>(_data->data());
				break;
			case Array::FLOAT:
				pmsr->encoding = DE_FLOAT32;
				pmsr->sampletype = 'f';
				pmsr->datasamples = const_cast<void*>(_data->data());
				break;
			case Array::DOUBLE:
				pmsr->encoding = DE_FLOAT64;
				pmsr->sampletype = 'd';
				pmsr->datasamples = const_cast<void*>(_data->data());
				break;
			default: {
				SEISCOMP_WARNING("Unknown data type %d! Switch to Integer-Steim2 encoding",
				                 static_cast<int>(_data->dataType()));
				pmsr->encoding = DE_STEIM2;
				pmsr->sampletype = 'i';
				data = ArrayFactory::Create(Array::INT,_data.get());
				pmsr->datasamples = const_cast<void*>(data->data());
			}
		}
	}

	/* Pack the record(s) */
	CharArray packed;
	int64_t psamples;

	msr_pack(pmsr, _Record_Handler, &packed, &psamples, 1, 0);
	pmsr->datasamples = 0;
	msr_free(&pmsr);

	out.write(packed.typedData(), packed.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
