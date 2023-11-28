#pragma once

#include <cstdlib>
#include <openssl/aes.h>
#include <openssl/cmac.h>
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

#if OPENSSL_VERSION_NUMBER > 0x10100000L
#include <openssl/rsa.h>
#endif
#include <openssl/bn.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define EVP_CIPHER_CTX_define                                                                                          \
    EVP_CIPHER_CTX *ctx;                                                                                               \
    EVP_CIPHER_CTX  _ctx
#define EVP_CIPHER_CTX_create &_ctx
#define EVP_CIPHER_CTX_free EVP_CIPHER_CTX_cleanup
#else
#define EVP_CIPHER_CTX_define EVP_CIPHER_CTX *ctx
#define EVP_CIPHER_CTX_create EVP_CIPHER_CTX_new()
#endif

#if defined OPENSSL_VERSION_MAJOR && (OPENSSL_VERSION_MAJOR >= 3)
#define OPENSSL3
#endif

#ifndef OPENSSL3
#define EVP_MAC_update CMAC_Update
#define EVP_MAC_final(ctx, out, outl, outsize) CMAC_Final(ctx, out, outl)
#define EVP_MAC_CTX CMAC_CTX
#define EVP_MAC_CTX_new(mac) CMAC_CTX_new()
#define EVP_MAC_CTX_free CMAC_CTX_free
#define EVP_MAC_init(ctx, key, keyLength, params) CMAC_Init(ctx, key, keyLength, params, NULL)
#endif

// old openssl
// #pragma comment(lib, "libeay32.lib")
// #pragma comment(lib, "ssleay32.lib")

// new openssl
// #pragma comment(lib, "libcrypto.lib")
// #pragma comment(lib, "libssl.lib")
// #pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "libcrypto32MT.lib")
#pragma comment(lib, "libssl32MT.lib")
#pragma comment(lib, "Crypt32.lib")

enum ENCRYPT_TYPE { FN_DECRYPT = 0, FN_ENCRYPT };

const unsigned char PADDING[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

class CCryptoUtils {
  public:
    CCryptoUtils();
    virtual ~CCryptoUtils();

    bool evpCrypto(const char *alg, unsigned char *dat, int datl, unsigned char *key, int keyl, unsigned char *iv,
                   int ivl, int Enc, unsigned char *out, int &outl);

    template <typename... Args>
    bool evpCipher(const char *alg, unsigned char *dat, int datl, unsigned char *key, int keyl, unsigned char *out,
                   int &outl, int enc, Args &&...args);

    bool randomHex(int num, char *out);

    bool calculateTDesMAC(unsigned char *dat, int datl, unsigned char *key, int keyl, unsigned char *icv, int icvl,
                          unsigned char mac[8]);

    bool algorithmLuhn(const char *strNum, int &out);

  private:
    const EVP_CIPHER *getEvpCipher(const char *evpalg);
};
