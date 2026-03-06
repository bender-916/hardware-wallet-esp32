/**
 * @file key_management.h
 * @brief Secure Key Management for BIP32/BIP39
 * @details On-the-fly derivation, DRAM-only storage, constant-time ops
 */

#ifndef KEY_MANAGEMENT_H
#define KEY_MANAGEMENT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "security/hmac.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SEED_SIZE 64
#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_SIZE 33
#define EXTENDED_KEY_SIZE 112
#define CHAIN_CODE_SIZE 32
#define MAX_PATH_DEPTH 10
#define MNEMONIC_MAX_SIZE 256

/**
 * @brief Secure key context
 * @details All sensitive data in DRAM, encrypted by hardware
 */
typedef struct {
    uint8_t seed[SEED_SIZE];           // BIP39 seed (cleared after use)
    uint8_t master_key[PRIVATE_KEY_SIZE];
    uint8_t chain_code[CHAIN_CODE_SIZE];
    uint32_t derivation_path[MAX_PATH_DEPTH];
    uint8_t path_depth;
    bool has_mnemonic;
    bool has_private_key;
    bool master_key_derived;
} secure_key_ctx_t;

/**
 * @brief Key derivation path
 */
typedef struct {
    uint32_t components[MAX_PATH_DEPTH];
    uint8_t depth;
} derivation_path_t;

/**
 * @brief Initialize key management
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_init(void);

/**
 * @brief Derive master key from mnemonic
 * @param mnemonic BIP39 mnemonic
 * @param passphrase Optional passphrase (can be NULL)
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_derive_master(const char *mnemonic, const char *passphrase);

/**
 * @brief Derive child key from path
 * @param path Derivation path
 * @param private_key Output (32 bytes)
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_derive_child(const derivation_path_t *path, uint8_t private_key[PRIVATE_KEY_SIZE]);

/**
 * @brief Derive key for signing
 * @brief path Derivation path (m/84'/0'/0'/0/0 for SegWit)
 * @param private_key Output private key
 * @param chain_code Output chain code
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_derive_signing_key(const derivation_path_t *path,
                                      uint8_t private_key[PRIVATE_KEY_SIZE],
                                      uint8_t chain_code[CHAIN_CODE_SIZE]);

/**
 * @brief Clear all keys from memory
 * @details Secure zeroing of all sensitive data
 */
void key_mgmt_clear_keys(void);

/**
 * @brief Check if mnemonic is loaded
 * @return true if mnemonic available
 */
bool key_mgmt_has_mnemonic(void);

/**
 * @brief Check if master key is derived
 * @return true if master key available
 */
bool key_mgmt_has_master_key(void);

/**
 * @brief Get extended public key (xpub)
 * @param path Derivation path
 * @param xpub_out Output buffer (max 112 chars)
 * @param xpub_len Output length
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_get_xpub(const derivation_path_t *path, char *xpub_out, size_t *xpub_len);

/**
 * @brief Parse derivation path string
 * @param path_str Path string (e.g., "m/84'/0'/0'/0/0")
 * @param path Output path structure
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_parse_path(const char *path_str, derivation_path_t *path);

/**
 * @brief Sign hash with derived key
 * @param path Derivation path
 * @param hash Message hash (32 bytes)
 * @param signature Output (64 bytes)
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_sign_hash(const derivation_path_t *path, const uint8_t hash[32], uint8_t signature[64]);

/**
 * @brief Secure clear memory
 * @param ptr Pointer to clear
 * @param len Bytes to clear
 */
void key_mgmt_secure_clear(void *ptr, size_t len);

/**
 * @brief Generate new mnemonic
 * @param entropy_bits 128 or 256
 * @param mnemonic_out Output buffer
 * @param max_len Buffer size
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_generate_mnemonic(uint16_t entropy_bits, char *mnemonic_out, size_t max_len);

/**
 * @brief Validate mnemonic
 * @param mnemonic Mnemonic to validate
 * @return true if valid
 */
bool key_mgmt_validate_mnemonic(const char *mnemonic);

/**
 * @brief Use HMAC efuse for key derivation
 * @param slot HMAC key slot
 * @param input Input data
 * @param input_len Length
 * @param output Output buffer
 * @param output_len Output length
 * @return ESP_OK on success
 */
esp_err_t key_mgmt_hmac_efuse_derivation(hmac_key_slot_t slot,
                                          const uint8_t *input, size_t input_len,
                                          uint8_t *output, size_t output_len);

#ifdef __cplusplus
}
#endif

#endif // KEY_MANAGEMENT_H
