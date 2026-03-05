/**
 * @file hash.c
 * @brief Hash implementation using ESP-IDF mbedTLS
 */

#include "hash.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "mbedtls/ripemd160.h"
#include "mbedtls/md.h"
#include <string.h>

void sha256(const uint8_t *data, size_t len, uint8_t *hash_out) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);  // 0 = SHA256
    mbedtls_sha256_update(&ctx, data, len);
    mbedtls_sha256_finish(&ctx, hash_out);
    mbedtls_sha256_free(&ctx);
}

void double_sha256(const uint8_t *data, size_t len, uint8_t *hash_out) {
    uint8_t intermediate[32];
    sha256(data, len, intermediate);
    sha256(intermediate, 32, hash_out);
    memset(intermediate, 0, sizeof(intermediate));
}

void sha512(const uint8_t *data, size_t len, uint8_t *hash_out) {
    mbedtls_sha512_context ctx;
    mbedtls_sha512_init(&ctx);
    mbedtls_sha512_starts(&ctx, 0);  // 0 = SHA512
    mbedtls_sha512_update(&ctx, data, len);
    mbedtls_sha512_finish(&ctx, hash_out);
    mbedtls_sha512_free(&ctx);
}

void ripemd160(const uint8_t *data, size_t len, uint8_t *hash_out) {
    mbedtls_ripemd160_context ctx;
    mbedtls_ripemd160_init(&ctx);
    mbedtls_ripemd160_starts(&ctx);
    mbedtls_ripemd160_update(&ctx, data, len);
    mbedtls_ripemd160_finish(&ctx, hash_out);
    mbedtls_ripemd160_free(&ctx);
}

void hash160_ripemd160(const uint8_t *data, size_t len, uint8_t *hash_out) {
    uint8_t sha_hash[32];
    sha256(data, len, sha_hash);
    ripemd160(sha_hash, 32, hash_out);
    memset(sha_hash, 0, sizeof(sha_hash));
}

void hmac_sha256(const uint8_t *key, size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t *hmac_out) {
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_hmac(md_info, key, key_len, data, data_len, hmac_out);
}

void hmac_sha512(const uint8_t *key, size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t *hmac_out) {
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    mbedtls_md_hmac(md_info, key, key_len, data, data_len, hmac_out);
}

void pbkdf2_sha512(const uint8_t *password, size_t password_len,
                   const uint8_t *salt, size_t salt_len,
                   uint32_t iterations, uint8_t *key_out, size_t key_len) {
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA512, 
                                   password, password_len,
                                   salt, salt_len,
                                   iterations, key_len, key_out);
}
