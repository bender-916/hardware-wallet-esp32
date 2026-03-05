/**
 * @file bip44.c
 * @brief BIP44 implementation
 */

#include "bip44.h"
#include "address.h"
#include <string.h>

void bip44_path_init(bip44_path_t *path) {
    if (path) {
        path->purpose = BIP44_PURPOSE;
        path->coin_type = BIP44_COIN_BTC;
        path->account = 0;
        path->change = BIP44_CHANGE_EXTERNAL;
        path->index = 0;
    }
}

void bip44_build_derivation_path(const bip44_path_t *path, uint32_t *out_path) {
    if (!path || !out_path) return;
    
    out_path[0] = path->purpose;
    out_path[1] = path->coin_type;
    out_path[2] = BIP32_HARDENED | path->account;  // Hardened
    out_path[3] = path->change;
    out_path[4] = path->index;
}

bool bip44_get_account_xpub(const hd_node_t *master, uint32_t account, 
                            char *xpub_out, size_t max_len) {
    if (!master || !xpub_out) return false;
    
    // Create copy of master node for derivation
    hd_node_t node;
    memcpy(&node, master, sizeof(hd_node_t));
    
    // Derive: m/44'/0'/account'
    uint32_t path[3] = {
        BIP44_PURPOSE,
        BIP44_COIN_BTC,
        BIP32_HARDENED | account
    };
    
    if (!bip32_derive_path(&node, path, 3)) {
        bip32_node_clear(&node);
        return false;
    }
    
    // Get xpub
    bool result = bip32_get_xpub(&node, xpub_out, max_len);
    
    bip32_node_clear(&node);
    return result;
}

bool bip44_derive_address(const hd_node_t *master, const bip44_path_t *path,
                          char *address_out, size_t max_len) {
    if (!master || !path || !address_out) return false;
    
    // Create copy of master node
    hd_node_t node;
    memcpy(&node, master, sizeof(hd_node_t));
    
    // Build full derivation path
    uint32_t full_path[5];
    bip44_build_derivation_path(path, full_path);
    
    // Derive to the address level
    if (!bip32_derive_path(&node, full_path, 5)) {
        bip32_node_clear(&node);
        return false;
    }
    
    // Generate address from public key
    bool result = address_from_pubkey(node.public_key, ADDRESS_TYPE_LEGACY,
                                      address_out, max_len);
    
    bip32_node_clear(&node);
    return result;
}

bool bip44_derive_addresses(const hd_node_t *master, uint32_t account,
                            uint32_t change, uint32_t start_index,
                            uint32_t count, char addresses[][64]) {
    if (!master || !addresses || count == 0) return false;
    
    bip44_path_t path;
    bip44_path_init(&path);
    path.account = account;
    path.change = change;
    
    for (uint32_t i = 0; i < count; i++) {
        path.index = start_index + i;
        if (!bip44_derive_address(master, &path, addresses[i], 64)) {
            return false;
        }
    }
    
    return true;
}
