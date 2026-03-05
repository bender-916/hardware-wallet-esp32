/**
 * @file bip32.h
 * @brief BIP32 Hierarchical Deterministic Key Derivation
 */

#ifndef BIP32_H
#define BIP32_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BIP32_KEY_LEN       32
#define BIP32_CHAIN_LEN     32
#define BIP32_EXT_KEY_LEN   78

// Hardened key flag
#define BIP32_HARDENED      0x80000000

typedef struct {
    uint8_t private_key[BIP32_KEY_LEN];
    uint8_t public_key[33];     // Compressed public key
    uint8_t chain_code[BIP32_CHAIN_LEN];
    uint32_t depth;
    uint32_t child_num;
    uint8_t fingerprint[4];
} hd_node_t;

/**
 * @brief Create master node from seed
 * @param seed 64-byte seed (from BIP39)
 * @param node Output node structure
 * @return true on success
 */
bool bip32_from_seed(const uint8_t *seed, hd_node_t *node);

/**
 * @brief Derive child key (private parent -> private child)
 * @param parent Parent node (will be modified)
 * @param index Child index (use BIP32_HARDENED for hardened)
 * @return true on success
 */
bool bip32_derive_private(hd_node_t *parent, uint32_t index);

/**
 * @brief Derive path from master
 * @param node Master node
 * @param path Array of derivation indices
 * @param path_len Number of path elements
 * @return true on success
 */
bool bip32_derive_path(hd_node_t *node, const uint32_t *path, size_t path_len);

/**
 * @brief Get extended private key (xprv) string
 * @param node HD node
 * @param xprv_out Output buffer (min 112 bytes)
 * @param max_len Buffer size
 * @return true on success
 */
bool bip32_get_xprv(const hd_node_t *node, char *xprv_out, size_t max_len);

/**
 * @brief Get extended public key (xpub) string
 * @param node HD node
 * @param xpub_out Output buffer (min 112 bytes)
 * @param max_len Buffer size
 * @return true on success
 */
bool bip32_get_xpub(const hd_node_t *node, char *xpub_out, size_t max_len);

/**
 * @brief Calculate public key from private key
 * @param private_key 32-byte private key
 * @param public_key_out 33-byte compressed public key output
 * @return true on success
 */
bool bip32_public_from_private(const uint8_t *private_key, uint8_t *public_key_out);

/**
 * @brief Clear sensitive data from node
 */
void bip32_node_clear(hd_node_t *node);

#endif // BIP32_H
