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


#define SEISCOMP_COMPONENT CertificateStore

#include <seiscomp/core/system.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/utils/certstore.h>
#include <seiscomp/utils/files.h>

#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;
using namespace Seiscomp::Core;

namespace {

bool fromASN1Time(Time &time, const ASN1_TIME *asn1Time) {
	string str(reinterpret_cast<char*>(asn1Time->data),
	           static_cast<size_t>(asn1Time->length));
	if ( asn1Time->type ==  V_ASN1_UTCTIME ) {
		return time.fromString(str.c_str(), "%y%m%d%H%M%SZ");
	}
	else if ( asn1Time->type ==  V_ASN1_GENERALIZEDTIME ) {
		return time.fromString(str.c_str(), "%Y%m%d%H%M%SZ");
	}
	else {
		SEISCOMP_ERROR("Could not convert ASN1_TIME, error: Unknown format");
		return false;
	}
}

}


namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CertificateStore CertificateStore::_global;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CertificateContext::CertificateContext()
: _cert(0), _begin(0), _end(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CertificateContext::~CertificateContext() {
	for ( Certs::iterator it = _certs.begin(); it != _certs.end(); ++it ) {
		X509_free(it->second);
	}

	for ( CRLs::iterator it = _crls.begin(); it != _crls.end(); ++it ) {
		X509_CRL_free(it->second);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const X509 *CertificateContext::findCertificate(const Core::Time &referenceTime) const {
	SEISCOMP_DEBUG("Certificate lookup");

	time_t seconds = referenceTime;
	if ( _cert ) {
		// Check the last cached certificate
		if ( (!_begin || X509_cmp_time(_begin, &seconds) == -1)
		  && (!_end || X509_cmp_time(_end, &seconds) == 1) ) {
			SEISCOMP_DEBUG("  Reusing cached certifcate");
			return _cert;
		}
	}

	// Iterate through available certificates
	SEISCOMP_DEBUG("  Find matching certificate\n");

	for ( Certs::const_reverse_iterator it = _certs.rbegin();
	      it != _certs.rend(); ++it ) {
		X509 *x509 = it->second;
		if ( !x509 ) {
			continue;
		}

		// The value returned is an internal pointer which MUST NOT be freed up after the call
		const ASN1_INTEGER *serial = X509_get0_serialNumber(x509);
		long serialNumber = ASN1_INTEGER_get(serial);

		SEISCOMP_DEBUG("    Cert(Serial: %ld): Checking certificate",
		               serialNumber);
		const ASN1_TIME *begin = X509_get0_notBefore(x509);
		if ( begin ) {
			int res = X509_cmp_time(begin, &seconds);
			if ( res == 0 ) {
				SEISCOMP_DEBUG("    Cert(Serial: %ld): X509_cmp_time failed: %s",
				               serialNumber, ERR_error_string(ERR_get_error(), 0));
				continue;
			}
			else if ( res > 0 ) {
				Time time;
				fromASN1Time(time, begin);
				SEISCOMP_DEBUG("    Cert(Serial: %ld): Time %s is before certificate begin %s",
				               serialNumber, referenceTime.iso().c_str(), time.iso().c_str());
				continue;
			}
		}

		const ASN1_TIME *end = X509_get0_notAfter(x509);
		if ( end ) {
			int res = X509_cmp_time(end, &seconds);
			if ( res == 0 ) {
				SEISCOMP_DEBUG("    Cert(Serial: %ld): X509_cmp_time failed: %s",
				               serialNumber, ERR_error_string(ERR_get_error(), 0));
				continue;
			}
			else if ( res < 0 ) {
				Time time;
				fromASN1Time(time, begin);
				SEISCOMP_DEBUG("    Cert(Serial: %ld): Time %s is behind certificate end %s",
				               serialNumber, referenceTime.iso().c_str(), time.iso().c_str());
				continue;
			}
		}

		SEISCOMP_DEBUG("    Cert(Serial: %ld): Passed", serialNumber);

		// TODO check CRLs

		_cert = x509;
		_begin = begin;
		_end = end;

		return _cert;
	}

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const X509 *CertificateContext::findCertificate(const char *digest, size_t nDigest,
                                                const ECDSA_SIG *signature) const {
	SEISCOMP_DEBUG("Certificate EC signature lookup");

	int verification_status;

	if ( _cert ) {
		// Check the last cached certificate
		EVP_PKEY *pkey = X509_get0_pubkey(_cert);
		if ( pkey ) {
			EC_KEY *ec_key = EVP_PKEY_get0_EC_KEY(pkey);
			if ( ec_key ) {
				verification_status = ECDSA_do_verify(
					reinterpret_cast<const unsigned char*>(digest), nDigest,
					signature, ec_key
				);

				if ( verification_status == 1 ) {
					SEISCOMP_DEBUG("  Reusing cached certifcate");
					return _cert;
				}
			}
		}
	}

	// Iterate through available certificates
	SEISCOMP_DEBUG("  Find matching certificate\n");

	X509 *cert = 0;

	for ( Certs::const_reverse_iterator it = _certs.rbegin();
	      it != _certs.rend(); ++it ) {
		X509 *x509 = it->second;
		if ( !x509 ) {
			continue;
		}

		// The value returned is an internal pointer which MUST NOT be freed up after the call
		const ASN1_INTEGER *serial = X509_get0_serialNumber(x509);
		long serialNumber = ASN1_INTEGER_get(serial);

		SEISCOMP_DEBUG("    Cert(Serial: %ld): Checking certificate",
		               serialNumber);

		EVP_PKEY *pkey = X509_get0_pubkey(x509);
		if ( !pkey ) {
			SEISCOMP_DEBUG("      No public key");
			continue;
		}
		EC_KEY *ec_key = EVP_PKEY_get0_EC_KEY(pkey);
		if ( !ec_key ) {
			SEISCOMP_DEBUG("      No public EC key");
			continue;
		}

		verification_status = ECDSA_do_verify(
			reinterpret_cast<const unsigned char*>(digest), nDigest,
			signature, ec_key
		);

		if ( verification_status != 1 ) {
			SEISCOMP_DEBUG("      Verification failed");
			continue;
		}

		SEISCOMP_DEBUG("      Verification OK");
		SEISCOMP_DEBUG("    Cert(Serial: %ld): Passed", serialNumber);
		cert = x509;
		break;
	}

	if ( !cert )
		return 0;

	_cert = cert;
	_begin = X509_get0_notBefore(_cert);
	_end = X509_get0_notAfter(_cert);

	return _cert;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//bool CertificateContext::isRevoked(const X509 *cert,
//                                   const Core::Time &referenceTime) const {
//	return true;
//}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CertificateStore::CertificateStore() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CertificateStore::~CertificateStore() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CertificateContext *CertificateStore::getContext(const char *hash, size_t len) {
	return getContext(string(hash, len));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CertificateContext *CertificateStore::getContext(const string &hash) {
	std::lock_guard<std::mutex> l(_storeMutex);

	SEISCOMP_DEBUG("Certificate context lookup");
	SEISCOMP_DEBUG("  Hash : %s", hash.c_str());

	Lookup::iterator it = _lookup.find(hash);
	if ( it != _lookup.end() ) {
		SEISCOMP_DEBUG("  Return cached context");
		return it->second.get();
	}

	CertificateContextPtr ctx = new CertificateContext;
	if ( !loadCerts(ctx->_certs, hash, _baseDirectory) ) {
		return 0;
	}

	if ( ctx->_certs.empty() ) {
		SEISCOMP_DEBUG("  No certificates found");
		return 0;
	}

//	if ( !loadCRLs(ctx->_crls, hash, _baseDirectory) ) {
//		return 0;
//	}

	SEISCOMP_INFO("Loaded X509 certs and CRLs from directory %s",
	              _baseDirectory.c_str());

	_lookup[hash] = ctx;
	return ctx.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CertificateStore::init(const std::string &baseDirectory) {
	_lookup.clear();

	if ( !pathExists(baseDirectory) ) {
		SEISCOMP_ERROR("Directory '%s' does not exist", baseDirectory.c_str());
		return false;
	}

	_baseDirectory = baseDirectory;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CertificateStore::loadCerts(CertificateContext::Certs &certs, const string &hash,
                                 const std::string &baseDirectory) {
	if ( baseDirectory.empty() ) {
		SEISCOMP_ERROR("%s: Failed to load X509 Certs: No directory given",
		               hash.c_str());
		return false;
	}

	string basename = hash + ".";

	try {
		boost::filesystem::recursive_directory_iterator it(baseDirectory);
		boost::filesystem::recursive_directory_iterator end;
		for ( ; it != end; ++it ) {
			if ( boost::filesystem::is_regular_file(*it) ) {
				string absFilename = it->path().string();
				string filename = SC_FS_FILE_PATH(SC_FS_PATH(absFilename)).string();
				if ( !boost::starts_with(filename, basename) ) {
					continue;
				}

				X509 *cert = nullptr;

				BIO *bio_cert = BIO_new_file(absFilename.c_str(), "rb");
				PEM_read_bio_X509(bio_cert, &cert, nullptr, nullptr);
				BIO_free(bio_cert);

				if ( cert == nullptr ) {
					SEISCOMP_ERROR("%s: Failed to load X509 cert from file %s",
					               hash.c_str(), absFilename.c_str());
					return false;
				}

				if ( !certs.insert(make_pair(filename, cert)).second ) {
					// unlikely: same file name already exists in store
					X509_free(cert);
				}
			}
		}
	}
	catch ( boost::filesystem::filesystem_error &error ) {
		SEISCOMP_ERROR("%s: %s", hash.c_str(), error.what());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CertificateStore::loadCRLs(CertificateContext::CRLs &crls,
                                const string &hash,
                                const std::string &baseDirectory) {
	if ( baseDirectory.empty() ) {
		SEISCOMP_ERROR("%s: Failed to load CRLs: No directory given",
		               hash.c_str());
		return false;
	}

	string basename = hash + ".r";

	try {
		boost::filesystem::recursive_directory_iterator it(baseDirectory);
		boost::filesystem::recursive_directory_iterator end;
		for ( ; it != end; ++it ) {
			if ( boost::filesystem::is_regular_file(*it) ) {
				string absFilename = it->path().string();
				string filename = SC_FS_FILE_PATH(SC_FS_PATH(absFilename)).string();
				if ( !boost::starts_with(filename, basename) ) {
					continue;
				}

				X509_CRL *crl = nullptr;

				BIO *bio_cert = BIO_new_file(absFilename.c_str(), "rb");
				PEM_read_bio_X509_CRL(bio_cert, &crl, nullptr, nullptr);
				BIO_free(bio_cert);

				if ( crl == nullptr ) {
					SEISCOMP_ERROR("%s: Failed to load CRL from file %s",
					               hash.c_str(), absFilename.c_str());
					return false;
				}

				if ( !crls.insert(make_pair(filename, crl)).second ) {
					// unlikely: same file name already exists in store
					X509_CRL_free(crl);
				}
			}
		}
	}
	catch ( boost::filesystem::filesystem_error &error ) {
		SEISCOMP_ERROR("%s: %s", hash.c_str(), error.what());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CertificateStore::validate(const char *authority, size_t len,
                                const char *digest, size_t nDigest,
                                const ECDSA_SIG *signature,
                                const X509 **matchedCertificate) {
	const CertificateContext *ctx = getContext(authority, len);
	if ( !ctx ) return false;

	const X509 *cert = ctx->findCertificate(digest, nDigest, signature);
	if ( !cert ) return false;

	if ( matchedCertificate ) *matchedCertificate = cert;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CertificateStore::validate(const std::string &hash,
                                const char *digest, size_t nDigest,
                                const ECDSA_SIG *signature,
                                const X509 **matchedCertificate) {
	return validate(
		&hash[0], hash.size(),
		digest, nDigest, signature, matchedCertificate
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
