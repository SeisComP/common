/***************************************************************************
 *   Copyright (C) by gempa GmbH                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_TEST_MODULE test_certificate_store


#define SEISCOMP_COMPONENT test_certificate_store
#include <seiscomp/logging/log.h>
#include <seiscomp/config/config.h>
#include <seiscomp/utils/certstore.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/unittest/unittests.h>

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#include <fstream>


using namespace std;
using namespace Seiscomp::Logging;
using namespace Seiscomp::Util;
namespace bu = boost::unit_test;


namespace {


string read_file(const string &fn) {
	streampos fsize;
	string content;
	ifstream file;

	file.open(fn.c_str(), ios::binary);
	if ( !file.is_open() )
		return content;

	file.seekg(0, ios::end);
	fsize = file.tellg();
	file.seekg(0, ios::beg);

	content.resize(fsize);
	file.read(&content[0], content.size());
	file.close();

	return content;
}

EC_KEY *set_private_key_from_PEM(const string &pemkey) {
	EVP_PKEY *pkey = EVP_PKEY_new();
	BIO *bio = BIO_new_mem_buf(const_cast<char*>(pemkey.c_str()), pemkey.size());

	if ( !PEM_read_bio_PrivateKey(bio, &pkey, nullptr, nullptr) ) {
		EVP_PKEY_free(pkey);
		return nullptr;
	}

	BIO_free(bio);

	EC_KEY *eckey = EVP_PKEY_get1_EC_KEY(pkey);

	if ( !eckey ) {
		EVP_PKEY_free(pkey);
		return nullptr;
	}

	EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
	EVP_PKEY_free(pkey);

	return eckey;
}


}


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(load_store) {
	enableConsoleLogging(getAll());
	CertificateStore store;
	BOOST_REQUIRE(store.init("./data/certs"));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(get_context) {
	CertificateStore store;
	BOOST_REQUIRE(store.init("./data/certs"));
	const CertificateContext *ctx = store.getContext("792241d4");
	BOOST_REQUIRE(ctx);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(get_certificate_by_time) {
	CertificateStore store;
	BOOST_REQUIRE(store.init("./data/certs"));
	const CertificateContext *ctx = store.getContext("792241d4");
	BOOST_REQUIRE(ctx);
	BOOST_REQUIRE(ctx->findCertificate(Seiscomp::Core::Time(2020, 4, 25)));
	BOOST_REQUIRE(ctx->findCertificate(Seiscomp::Core::Time(2020, 4, 23)));
	BOOST_REQUIRE(!ctx->findCertificate(Seiscomp::Core::Time(2019, 12, 31)));
	BOOST_REQUIRE(!ctx->findCertificate(Seiscomp::Core::Time(2030, 4, 25)));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(get_certificate_by_signature) {
	CertificateStore store;
	BOOST_REQUIRE(store.init("./data/certs"));
	const CertificateContext *ctx = store.getContext("792241d4");
	BOOST_REQUIRE(ctx);

	string asciiKey = read_file("./data/gempa-gmbh.priv.pem");
	BOOST_REQUIRE(!asciiKey.empty());

	EC_KEY *privateECKey = set_private_key_from_PEM(asciiKey);
	BOOST_REQUIRE(privateECKey);
	BOOST_REQUIRE(ECDSA_size(privateECKey) < 80);

	unsigned char digest[32];
	unsigned int digest_len;

	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
	EVP_DigestInit(mdctx, EVP_sha256());

	for ( int i = 0; i < 100; ++i ) {
		int v = ::rand();
		EVP_DigestUpdate(mdctx, &v, sizeof(v));
	}
	EVP_DigestFinal_ex(mdctx, digest, &digest_len);
	EVP_MD_CTX_destroy(mdctx);

	BOOST_REQUIRE(digest_len == sizeof(digest));

	ECDSA_SIG *signature = ECDSA_do_sign(digest, digest_len, privateECKey);
	unsigned char signbuf[128];
	unsigned char *pp = signbuf;
	int signbuf_len = i2d_ECDSA_SIG(signature, &pp);

	BOOST_REQUIRE((unsigned long)signbuf_len <= sizeof(signbuf));

	ECDSA_SIG_free(signature);

	const X509 *cert;

	pp = signbuf;
	signature = d2i_ECDSA_SIG(0, (const unsigned char **)&pp, signbuf_len);
	cert = ctx->findCertificate((const char*)digest, digest_len, signature);
	ECDSA_SIG_free(signature);
	BOOST_REQUIRE(cert);

	// Modify the signature and verify again, must fail
	signbuf[0] += 1;

	pp = signbuf;
	signature = d2i_ECDSA_SIG(0, (const unsigned char **)&pp, signbuf_len);
	cert = ctx->findCertificate((const char*)digest, digest_len, signature);
	ECDSA_SIG_free(signature);
	BOOST_REQUIRE(!cert);

	// Modify the digest and verify again, must fail
	signbuf[0] -= 1;
	digest[0] += 1;

	pp = signbuf;
	signature = d2i_ECDSA_SIG(0, (const unsigned char **)&pp, signbuf_len);
	cert = ctx->findCertificate((const char*)digest, digest_len, signature);
	ECDSA_SIG_free(signature);
	BOOST_REQUIRE(!cert);

	// Revert again, must succeed
	digest[0] -= 1;

	pp = signbuf;
	signature = d2i_ECDSA_SIG(0, (const unsigned char **)&pp, signbuf_len);
	cert = ctx->findCertificate((const char*)digest, digest_len, signature);
	ECDSA_SIG_free(signature);
	BOOST_REQUIRE(cert);

	EC_KEY_free(privateECKey);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(validate_records) {
	CertificateStore &store = CertificateStore::global();
	BOOST_REQUIRE(store.init("./data/certs"));
	BOOST_REQUIRE(store.isValid());
	Seiscomp::IO::RecordStreamPtr rs = Seiscomp::IO::RecordStream::Open("file://./data/data-signed.mseed");
	rs->setDataHint(Seiscomp::Record::DATA_ONLY);
	BOOST_REQUIRE(rs);
	int numberOfRecords = 0;
	Seiscomp::RecordPtr rec;
	while ( (rec = rs->next()) ) {
		BOOST_CHECK(rec->authentication() == Seiscomp::Record::SIGNATURE_VALIDATED);
		BOOST_CHECK(rec->authority() == "gempa GmbH");
		++numberOfRecords;
	}

	std::cerr << "Read " << numberOfRecords << " records" << std::endl;
	BOOST_CHECK(numberOfRecords == 10);
}
