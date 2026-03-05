/**
 * @file bip32.c
 * @brief BIP32 HD Key Derivation Implementation
 */

#include "bip32.h"
#include "hash.h"
#include "rand.h"
#include <string.h>
#include <stdlib.h>

// Base58 alphabet
static const char base58[] = 
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// secp256k1 curve parameters (simplified - use trezor-crypto in production)
static const uint8_t secp256k1_n[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
};

bool bip32_from_seed(const uint8_t *seed, hd_node_t *node) {
    if (!seed || !node) return false;
    
    // HMAC-SHA512("Bitcoin seed", seed)
    uint8_t I[64];
    hmac_sha512((const uint8_t*)"Bitcoin seed", 12, seed, 64, I);
    
    // Split into private key and chain code
    memcpy(node->private_key, I, 32);
    memcpy(node->chain_code, I + 32, 32);
    
    // Validate private key (not zero, not >= n)
    bool valid = true;
    for (int i = 0; i < 32; i++) {
        if (node->private_key[i] != 0) {
            valid = true;
            break;
        }
    }
    
    // Check < n (simplified - would use bignum comparison)
    
    // Calculate public key
    bip32_public_from_private(node->private_key, node->public_key);
    
    // Set node metadata
    node->depth = 0;
    node->child_num = 0;
    memset(node->fingerprint, 0, 4);
    
    // Clear intermediate data
    memset(I, 0, sizeof(I));
    
    return valid;
}

bool bip32_derive_private(hd_node_t *parent, uint32_t index) {
    if (!parent) return false;
    
    uint8_t I[64];
    uint8_t data[37];
    
    if (index & BIP32_HARDENED) {
        // Hardened child: HMAC-SHA512(Key, 0x00 || ser256(kpar) || ser32(index))
        data[0] = 0;
        memcpy(data + 1, parent->private_key, 32);
    } else {
        // Normal child: HMAC-SHA512(Key, serP(point(kpar)) || ser32(index))
        memcpy(data, parent->public_key, 33);
    }
    
    // Encode index as 4 bytes big-endian
    data[33] = (index >> 24) & 0xFF;
    data[34] = (index >> 16) & 0xFF;
    data[35] = (index >> 8) & 0xFF;
    data[36] = index & 0xFF;
    
    hmac_sha512(parent->chain_code, 32, data, 37, I);
    
    // Child key = (parse256(I_L) + kpar) mod n
    // Simplified - would use bignum arithmetic
    memcpy(parent->private_key, I, 32);
    memcpy(parent->chain_code, I + 32, 32);
    
    // Update public key
    bip32_public_from_private(parent->private_key, parent->public_key);
    
    // Update metadata
    parent->depth++;
    parent->child_num = index;
    // Fingerprint would be first 4 bytes of hash160(parent public key)
    
    memset(I, 0, sizeof(I));
    memset(data, 0, sizeof(data));
    
    return true;
}

bool bip32_derive_path(hd_node_t *node, const uint32_t *path, size_t path_len) {
    if (!node || !path) return false;
    
    for (size_t i = 0; i < path_len; i++) {
        if (!bip32_derive_private(node, path[i])) {
            return false;
        }
    }
    return true;
}

bool bip32_public_from_private(const uint8_t *private_key, uint8_t *public_key_out) {
    // This would use secp256k1 point multiplication
    // Placeholder implementation - use trezor-crypto in production
    
    // For demo, just set a placeholder
    memset(public_key_out, 0, 33);
    public_key_out[0] = 0x02;  // Compressed public key prefix
    
    // Copy first 32 bytes of private key (INSECURE - demo only)
    memcpy(public_key_out + 1, private_key, 32);
    
    return true;
}

// Base58 encode
static size_t base58_encode(const uint8_t *data, size_t len, char *out, size_t max_len) {
    size_t i, j, high, zcount = 0;
    
    // Count leading zeros
    while (zcount < len && data[zcount] == 0) zcount++;
    
    // Calculate output size
    size_t size = (len - zcount) * 138 / 100 + 1;
    uint8_t *buf = (uint8_t*)calloc(size, 1);
    
    for (i = zcount; i < len; i++) {
        uint32_t carry = data[i];
        for (j = size; j-- > 0;) {
            carry += buf[j] << 8;
            buf[j] = carry % 58;
            carry /= 58;
        }
    }
    
    // Skip leading zeros in output
    for (j = 0; j < size && buf[j] == 0; j++);
    
    // Add leading '1's for each leading zero byte in input
    size_t out_idx = 0;
    while (zcount-- > 0 && out_idx < max_len - 1) {
        out[out_idx++] = '1';
    }
    
    // Convert to base58 characters
    for (; j < size && out_idx < max_len - 1; j++) {
        out[out_idx++] = base58[buf[j]];
    }
    out[out_idx] = '\0';
    
    free(buf);
    return out_idx;
}

bool bip32_get_xprv(const hd_node_t *node, char *xprv_out, size_t max_len) {
    if (!node || !xprv_out || max_len < 112) return false;
    
    uint8_t ext_key[78];
    
    // Version: xprv (mainnet private)
    ext_key[0] = 0x04;
    ext_key[1] = 0x88;
    ext_key[2] = 0xAD;
    ext_key[3] = 0xE4;
    
    // Depth
    ext_key[4] = node->depth;
    
    // Parent fingerprint
    memcpy(ext_key + 5, node->fingerprint, 4);
    
    // Child number
    ext_key[9] = (node->child_num >> 24) & 0xFF;
    ext_key[10] = (node->child_num >> 16) & 0xFF;
    ext_key[11] = (node->child_num >> 8) & 0xFF;
    ext_key[12] = node->child_num & 0xFF;
    
    // Chain code
    memcpy(ext_key + 13, node->chain_code, 32);
    
    // Private key with 0x00 prefix
    ext_key[45] = 0;
    memcpy(ext_key + 46, node->private_key, 32);
    
    // Double SHA256 checksum
    uint8_t hash[32];
    sha256(ext_key, 78, hash);
    sha256(hash, 32, hash);
    
    // Extend with checksum (but base58 encode 78 bytes + 4 = 82)
    // For simplicity, encode 78 bytes
    base58_encode(ext_key, 78, xprv_out, max_len);
    
    memset(hash, 0, sizeof(hash));
    memset(ext_key, 0, sizeof(ext_key));
    
    return true;
}

bool bip32_get_xpub(const hd_node_t *node, char *xpub_out, size_t max_len) {
    if (!node || !xpub_out || max_len < 112) return false;
    
    uint8_t ext_key[78];
    
    // Version: xpub (mainnet public)
    ext_key[0] = 0x04;
    ext_key[1] = 0x88;
    ext_key[2] = 0xB2;
    ext_key[3] = 0x1E;
    
    // Depth
    ext_key[4] = node->depth;
    
    // Parent fingerprint
    memcpy(ext_key + 5, node->fingerprint, 4);
    
    // Child number
    ext_key[9] = (node->child_num >> 24) & 0xFF;
    ext_key[10] = (node->child_num >> 16) & 0xFF;
    ext_key[11] = (node->child_num >> 8) & 0xFF;
    ext_key[12] = node->child_num & 0xFF;
    
    // Chain code
    memcpy(ext_key + 13, node->chain_code, 32);
    
    // Public key (compressed, 33 bytes)
    memcpy(ext_key + 45, node->public_key, 33);
    
    base58_encode(ext_key, 78, xpub_out, max_len);
    
    memset(ext_key, 0, sizeof(ext_key));
    
    return true;
}

void bip32_node_clear(hd_node_t *node) {
    if (node) {
        memset(node->private_key, 0, sizeof(node->private_key));
        memset(node->public_key, 0, sizeof(node->public_key));
        memset(node->chain_code, 0, sizeof(node->chain_code));
    }
}
