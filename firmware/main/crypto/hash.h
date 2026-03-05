/**
 * @file hash.h
 * @brief Hash function wrappers
 */

#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief SHA256 hash
 * @param data Input data
 * @param len Input length
 * @param hash_out 32-byte output buffer
 */
void sha256(const uint8_t *data, size_t len, uint8_t *hash_out);

/**
 * @brief Double SHA256 (Bitcoin standard)
 * @param data Input data
 * @param len Input length
 * @param hash_out 32-byte output buffer
 */
void double_sha256(const uint8_t *data, size_t len, uint8_t *hash_out);

/**
 * @brief SHA512 hash
 * @param data Input data
 * @param len Input length
 * @param hash_out 64-byte output buffer
 */
void sha512(const uint8_t *data, size_t len, uint8_t *hash_out);

/**
 * @brief RIPEMD160 hash
 * @param data Input data
 * @param len Input length
 * @param hash_out 20-byte output buffer
 */
void ripemd160(const uint8_t *data, size_t len, uint8_t *hash_out);

/**
 * @brief HASH160 (RIPEMD160(SHA256(data)))
 * @param data Input data
 * @param len Input length
 * @param hash_out 20-byte output buffer
 */
void hash160_ripemd160(const uint8_t *data, size_t len, uint8_t *hash_out);

/**
 * @brief HMAC-SHA256
 */
void hmac_sha256(const uint8_t *key, size_t key_len, 
                 const uint8_t *data, size_t data_len,
                 uint8_t *hmac_out);

/**
 * @brief HMAC-SHA512
 */
void hmac_sha512(const uint8_t *key, size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t *hmac_out);

/**
 * @brief PBKDF2-HMAC-SHA512
 */
void pbkdf2_sha512(const uint8_t *password, size_t password_len,
                   const uint8_t *salt, size_t salt_len,
                   uint32_t iterations, uint8_t *key_out, size_t key_len);

#endif // HASH_H
