/**
 * @file bip44.h
 * @brief BIP44 multi-account hierarchy for deterministic wallets
 */

#ifndef BIP44_H
#define BIP44_H

#include <stdint.h>
#include <stdbool.h>
#include "bip32.h"

// BIP44 path components
#define BIP44_PURPOSE       0x8000002C  // 44' hardened
#define BIP44_COIN_BTC      0x80000000  // 0' for Bitcoin mainnet
#define BIP44_COIN_TBTC     0x80000001  // 1' for Bitcoin testnet

// Change values
#define BIP44_CHANGE_EXTERNAL   0   // External chain (receiving addresses)
#define BIP44_CHANGE_INTERNAL   1   // Internal chain (change addresses)

/**
 * @brief BIP44 derivation path
 */
typedef struct {
    uint32_t purpose;   // Should be 44' for BIP44
    uint32_t coin_type; // 0' for BTC mainnet, 1' for testnet
    uint32_t account;   // Account index
    uint32_t change;    // 0 = external, 1 = internal
    uint32_t index;     // Address index
} bip44_path_t;

/**
 * @brief Initialize BIP44 path
 */
void bip44_path_init(bip44_path_t *path);

/**
 * @brief Build derivation path array for HD derivation
 * @param path BIP44 path struct
 * @param out_path Array of 5 uint32_t values
 */
void bip44_build_derivation_path(const bip44_path_t *path, uint32_t *out_path);

/**
 * @brief Derive account xpub (for watch-only wallets)
 * @param master Master HD node
 * @param account Account index
 * @param xpub_out Extended public key output buffer
 * @param max_len Buffer size
 * @return true on success
 */
bool bip44_get_account_xpub(const hd_node_t *master, uint32_t account, 
                            char *xpub_out, size_t max_len);

/**
 * @brief Derive address at path
 * @param master Master HD node
 * @param path BIP44 path
 * @param address_out Address output buffer
 * @param max_len Buffer size
 * @return true on success
 */
bool bip44_derive_address(const hd_node_t *master, const bip44_path_t *path,
                          char *address_out, size_t max_len);

/**
 * @brief Derive first N addresses for verification
 * @param master Master HD node
 * @param account Account index
 * @param change 0 for external, 1 for internal
 * @param start_index Starting index
 * @param count Number of addresses to derive
 * @param addresses Array of address strings (each should be ~64 bytes)
 */
bool bip44_derive_addresses(const hd_node_t *master, uint32_t account,
                            uint32_t change, uint32_t start_index,
                            uint32_t count, char addresses[][64]);

#endif // BIP44_H
