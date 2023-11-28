#include "CryptoUtils.h"
#include <string.h>

CCryptoUtils::CCryptoUtils() {
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    SSL_library_init();

    ERR_load_ERR_strings();
    ERR_load_EVP_strings();
    ERR_load_crypto_strings();

    ENGINE_load_dynamic();
    ENGINE_load_openssl();
}

CCryptoUtils::~CCryptoUtils() {
    // FIPS_mode_set(0);

    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_id_callback(NULL);

    ENGINE_cleanup();
    // CONF_modules_unload(1);

    ERR_free_strings();
    EVP_cleanup();

    CRYPTO_cleanup_all_ex_data();
}

/**
 * openssl enc -help
 * @brief CCryptoUtils::evpCrypto
 * @param alg: des-ede, des-ede3, des-ede-cbc, des-ede3-cbc, aes-128-ecb, aes-192-ecb
 * @param dat
 * @param datl
 * @param key
 * @param keyl
 * @param iv
 * @param ivl
 * @param Enc
 * @param out
 * @param outl
 * @return
 */
bool CCryptoUtils::evpCrypto(const char *alg, unsigned char *dat, int datl, unsigned char *key, int keyl,
                             unsigned char *iv, int ivl, int Enc, unsigned char *out, int &outl) {
    bool bRet = false;

    const EVP_CIPHER *cipher                      = NULL;
    unsigned char    *buffer                      = NULL;
    unsigned char     evp_key[EVP_MAX_KEY_LENGTH] = {0};
    unsigned char     evp_iv[EVP_MAX_IV_LENGTH]   = {0};

    int len = 0;
    int ret = 0;

    memset(evp_iv, 0x00, EVP_MAX_IV_LENGTH);
    memset(evp_key, 0x00, EVP_MAX_KEY_LENGTH);

    cipher = getEvpCipher(alg);

    if (cipher != NULL) {
        switch (Enc) {
        // decrypt 0
        case FN_DECRYPT: {
            EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();

            if (key != NULL) {
                keyl = EVP_MAX_KEY_LENGTH > keyl ? keyl : EVP_MAX_KEY_LENGTH;
                memcpy(evp_key, key, keyl);
            }

            if (ivl > 0 && iv) {
                ivl = EVP_MAX_IV_LENGTH > ivl ? ivl : EVP_MAX_IV_LENGTH;
                memcpy(evp_iv, iv, ivl);
            }

            EVP_CIPHER_CTX_init(c);
            ret = EVP_DecryptInit_ex(c, cipher, NULL, key ? evp_key : NULL, iv ? evp_iv : NULL);
            if (ret == 1) {
                ret = EVP_CIPHER_CTX_set_padding(c, 0);
                if (ret == 1) {
                    buffer = (unsigned char *) OPENSSL_malloc(datl + EVP_CIPHER_CTX_block_size(c));
                    memset(buffer, 0x00, sizeof(buffer));

                    ret = EVP_DecryptUpdate(c, buffer, &len, dat, datl);
                    if (ret == 1) {
                        outl += len;
                        len = datl - len;
                        ret = EVP_DecryptFinal_ex(c, buffer + outl, &len);
                        if (ret == 1) {
                            outl += len;
                            memcpy(out, buffer, outl);
                            out[outl] = '\0';
                            bRet      = true;
                        }
                    }
                    OPENSSL_free(buffer);
                }
            }
            EVP_CIPHER_CTX_cleanup(c);
            EVP_CIPHER_CTX_free(c);
        } break;

        // encrypt 1
        case FN_ENCRYPT: {
            EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();

            if (key != NULL) {
                keyl = EVP_MAX_KEY_LENGTH > keyl ? keyl : EVP_MAX_KEY_LENGTH;
                memcpy(evp_key, key, keyl);
            }

            if (ivl > 0 && iv) {
                ivl = EVP_MAX_IV_LENGTH > ivl ? ivl : EVP_MAX_IV_LENGTH;
                memcpy(evp_iv, iv, ivl);
            }

            EVP_CIPHER_CTX_init(c);
            ret = EVP_EncryptInit_ex(c, cipher, NULL, key ? evp_key : NULL, iv ? evp_iv : NULL);
            if (ret == 1) {
                ret = EVP_CIPHER_CTX_set_padding(c, 0);
                if (ret == 1) {
                    buffer = (unsigned char *) OPENSSL_malloc(datl + EVP_CIPHER_CTX_block_size(c));
                    memset(buffer, 0x00, sizeof(buffer));

                    ret = EVP_EncryptUpdate(c, buffer, &len, dat, datl);
                    if (ret == 1) {
                        outl += len;
                        len = datl - len;
                        ret = EVP_EncryptFinal_ex(c, buffer + outl, &len);
                        if (ret == 1) {
                            outl += len;
                            memcpy(out, buffer, outl);
                            out[outl] = '\0';
                            bRet      = true;
                        }
                    }
                    OPENSSL_free(buffer);
                }
            }
            EVP_CIPHER_CTX_cleanup(c);
            EVP_CIPHER_CTX_free(c);
        } break;

        default:
            break;
        }
    }
    return bRet;
}

/**
 * @brief CCryptoUtils::randomHex
 * @param num
 * @param out
 * @return
 */
bool CCryptoUtils::randomHex(int num, char *out) {
    /*
    BIGNUM* bn;

    bn = BN_new();

    int bits = num * 4;
    BN_rand(bn, bits, -1, 0);

    char* buf = BN_bn2hex(bn);
    //puts(buf);

    memcpy(out, buf, bits);
    out[bits] = '\0';

    BN_free(bn);
    */
    int            bits = num / 2;
    unsigned char *buf  = new unsigned char[bits + 1];
    memset(buf, 0x00, bits + 1);
    RAND_bytes(buf, bits);
    for (int i = 0; i < bits; ++i) {
        sprintf(out + i * 2, "%02X", buf[i]);
    }
    out[num] = '\0';
    delete[] buf;

    return true;
}

/**
 * @brief CCryptoUtils::calculateTDesMAC
 * @param dat
 * @param datl
 * @param key
 * @param icv
 * @param mac
 * @return
 */
bool CCryptoUtils::calculateTDesMAC(unsigned char *dat, int datl, unsigned char *key, int keyl, unsigned char *icv,
                                    int icvl, unsigned char mac[]) {
    const unsigned char PADDING[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const unsigned char TEMPICV[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    int  i = 0, ret = 0, outl = 0;
    bool bRet = false;

    unsigned char des_key[8];
    unsigned char _icv[8];

    memset(des_key, 0x00, sizeof(des_key));
    memset(_icv, 0x00, sizeof(_icv));

    if (icv == NULL) {
        memcpy(_icv, TEMPICV, 8);
    } else {
        memcpy(_icv, icv, 8);
    }

    /* If only one block */
    memcpy(mac, icv, 8);
    //  DES CBC mode
    memcpy(des_key, key, 8);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);

    ret = EVP_EncryptInit_ex(ctx, EVP_des_cbc(), NULL, des_key, _icv);
    if (ret == 1) {
        EVP_CIPHER_CTX_set_padding(ctx, 0);
        for (i = 0; i < datl / 8; i++) {
            ret = EVP_EncryptUpdate(ctx, mac, &outl, dat + i * 8, 8);
            if (ret != 1) {
                bRet = false;
                break;
            }
        }

        if (ret == 1) ret = EVP_EncryptFinal_ex(ctx, mac, &outl);

        bRet = (ret == 1) ? true : false;
    }
    EVP_CIPHER_CTX_free(ctx);
    ctx = NULL;

    if (bRet) {
        bRet = false;

        // 3DES mode
        ctx = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(ctx);

        if (EVP_EncryptInit_ex(ctx, EVP_des_ede_cbc(), NULL, key, mac) == 1) {
            EVP_CIPHER_CTX_set_padding(ctx, 0);
            if (datl % 8 != 0) {
                ret = EVP_EncryptUpdate(ctx, mac, &outl, dat + i * 8, datl % 8);
            }

            ret = EVP_EncryptUpdate(ctx, mac, &outl, PADDING, 8 - (datl % 8));
            if (ret == 1) {
                ret = EVP_EncryptFinal_ex(ctx, mac, &outl);
                if (ret == 1) {
                    bRet = true;
                }
            }
        }

        EVP_CIPHER_CTX_free(ctx);
    }

    return bRet;
}

/*
7992739871
从校验位开始，从右往左，偶数位乘2（例如，1*2=2），然后将两位数字的个位与十位相加（例如，16：1+6=7，18：1+8=9）；
把得到的数字加在一起（本例中得到67）；
将数字的和取模10（本例中得到7），再用10去减（本例中得到3），得到校验位。
*/
bool CCryptoUtils::algorithmLuhn(const char *strNum, int &out) {
    int  nSum      = 0;
    int  nDigits   = strlen(strNum);
    int  nParity   = (nDigits - 1) % 2;
    char cDigit[2] = "\0";
    for (int i = nDigits; i > 0; i--) {
        cDigit[0]  = strNum[i - 1];
        int nDigit = atoi(cDigit);

        if (nParity != i % 2) nDigit = nDigit * 2;

        nSum += nDigit / 10;
        nSum += nDigit % 10;
    }
    out = nSum % 10 == 0 ? 0 : (10 - (nSum % 10));

    return true;
}

const EVP_CIPHER *CCryptoUtils::getEvpCipher(const char *evpalg) {
    const EVP_CIPHER *cipher = NULL;

    if (evpalg != NULL) cipher = EVP_get_cipherbyname(evpalg);

    if (cipher == NULL) return NULL;

    return cipher;
}

template <typename... Args>
bool CCryptoUtils::evpCipher(const char *alg, unsigned char *dat, int datl, unsigned char *key, int keyl,
                             unsigned char *out, int &outl, int enc, Args &&...args) {
    bool           bRet = false;
    unsigned char *iv   = NULL;
    int            ivl  = 0;

    size_t n = sizeof...(args);
    if (n > 0) {
        auto ttt = std::forward(args...);
        auto tps = std::forward_as_tuple(args...);
        auto icv = std::get<0>(tps);
    } else {
    }

    const EVP_CIPHER *cipher                      = NULL;
    unsigned char    *buffer                      = NULL;
    unsigned char     evp_key[EVP_MAX_KEY_LENGTH] = {0};
    unsigned char     evp_iv[EVP_MAX_IV_LENGTH]   = {0};

    int len = 0;
    int ret = 0;

    memset(evp_iv, 0x00, EVP_MAX_IV_LENGTH);
    memset(evp_key, 0x00, EVP_MAX_KEY_LENGTH);

    cipher = getEvpCipher(alg);

    if (cipher != NULL) {
        switch (enc) {
        // decrypt 0
        case DECRYPT: {
            EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();

            if (key != NULL) {
                keyl = EVP_MAX_KEY_LENGTH > keyl ? keyl : EVP_MAX_KEY_LENGTH;
                memcpy(evp_key, key, keyl);
            }

            if (ivl > 0 && iv) {
                ivl = EVP_MAX_IV_LENGTH > ivl ? ivl : EVP_MAX_IV_LENGTH;
                memcpy(evp_iv, iv, ivl);
            }

            EVP_CIPHER_CTX_init(c);
            ret = EVP_DecryptInit_ex(c, cipher, NULL, key ? evp_key : NULL, iv ? evp_iv : NULL);
            if (ret == 1) {
                ret = EVP_CIPHER_CTX_set_padding(c, 0);
                if (ret == 1) {
                    buffer = (unsigned char *) OPENSSL_malloc(datl + EVP_CIPHER_CTX_block_size(c));
                    memset(buffer, 0x00, sizeof(buffer));

                    ret = EVP_DecryptUpdate(c, buffer, &len, dat, datl);
                    if (ret == 1) {
                        outl += len;
                        len = datl - len;
                        ret = EVP_DecryptFinal_ex(c, buffer + outl, &len);
                        if (ret == 1) {
                            outl += len;
                            memcpy(out, buffer, outl);
                            out[outl] = '\0';
                            bRet      = true;
                        }
                    }
                    OPENSSL_free(buffer);
                }
            }
            EVP_CIPHER_CTX_cleanup(c);
            EVP_CIPHER_CTX_free(c);
        } break;

        // encrypt 1
        case ENCRYPT: {
            EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();

            if (key != NULL) {
                keyl = EVP_MAX_KEY_LENGTH > keyl ? keyl : EVP_MAX_KEY_LENGTH;
                memcpy(evp_key, key, keyl);
            }

            if (ivl > 0 && iv) {
                ivl = EVP_MAX_IV_LENGTH > ivl ? ivl : EVP_MAX_IV_LENGTH;
                memcpy(evp_iv, iv, ivl);
            }

            EVP_CIPHER_CTX_init(c);
            ret = EVP_EncryptInit_ex(c, cipher, NULL, key ? evp_key : NULL, iv ? evp_iv : NULL);
            if (ret == 1) {
                ret = EVP_CIPHER_CTX_set_padding(c, 0);
                if (ret == 1) {
                    buffer = (unsigned char *) OPENSSL_malloc(datl + EVP_CIPHER_CTX_block_size(c));
                    memset(buffer, 0x00, sizeof(buffer));

                    ret = EVP_EncryptUpdate(c, buffer, &len, dat, datl);
                    if (ret == 1) {
                        outl += len;
                        len = datl - len;
                        ret = EVP_EncryptFinal_ex(c, buffer + outl, &len);
                        if (ret == 1) {
                            outl += len;
                            memcpy(out, buffer, outl);
                            out[outl] = '\0';
                            bRet      = true;
                        }
                    }
                    OPENSSL_free(buffer);
                }
            }
            EVP_CIPHER_CTX_cleanup(c);
            EVP_CIPHER_CTX_free(c);
        } break;

        default:
            break;
        }
    }
    return bRet;
}
