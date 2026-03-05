/**
 * @file address.c
 * @brief Bitcoin address implementation
 */

#include "address.h"
#include "hash.h"
#include <string.h>

// Base58 alphabet
static const char base58[] = 
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// Bech32 charset
static const char bech32_charset[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

bool address_from_pubkey(const uint8_t *pubkey, address_type_t type,
                         char *address_out, size_t max_len) {
    if (!pubkey || !address_out) return false;
    
    uint8_t hash160[20];
    hash160_ripemd160(pubkey, 33, hash160);
    
    switch (type) {
        case ADDRESS_TYPE_LEGACY: {
            // P2PKH: 1 + RIPEMD160(SHA256(pubkey)) + checksum
            if (max_len < 35) return false;
            
            uint8_t data[25];
            data[0] = 0x00;  // Mainnet P2PKH version
            memcpy(data + 1, hash160, 20);
            
            // Checksum (double SHA256)
            uint8_t checksum[32];
            double_sha256(data, 21, checksum);
            memcpy(data + 21, checksum, 4);
            
            // Base58 encode
            size_t j = 0;
            for (size_t i = 0; i < 25; i++) {
                uint32_t carry = data[i];
                for (size_t k = 0; k < j; k++) {
                    carry = carry * 58 + (uint8_t)address_out[k];
                    address_out[k] = carry & 0xFF;
                    carry >>= 8;
                }
                while (carry) {
                    address_out[j++] = carry % 58;
                    carry /= 58;
                }
            }
            
            // Reverse and convert to characters
            for (size_t i = 0; i < j / 2; i++) {
                char tmp = address_out[i];
                address_out[i] = base58[(uint8_t)address_out[j - 1 - i]];
                address_out[j - 1 - i] = base58[(uint8_t)tmp];
            }
            address_out[j] = '\0';
            break;
        }
        
        case ADDRESS_TYPE_NATIVE_SEGWIT: {
            // P2WPKH: bc1q + 20-byte witness program (bech32)
            if (max_len < 45) return false;
            
            // Human-readable part
            strcpy(address_out, "bc1q");
            
            // Convert to bech32 (simplified)
            char *out = address_out + 4;
            for (int i = 0; i < 20; i++) {
                *out++ = bech32_charset[(hash160[i] >> 3) & 0x1F];
                *out++ = bech32_charset[((hash160[i] & 0x07) << 2) | 
                                        ((i < 19 ? hash160[i + 1] >> 6 : 0) & 0x03)];
            }
            *out = '\0';
            break;
        }
        
        default:
            return false;
    }
    
    return true;
}

size_t address_to_script(const char *address, uint8_t *script_out, size_t max_len) {
    if (!address || !script_out) return 0;
    
    // P2PKH: OP_DUP OP_HASH160 <20 bytes> OP_EQUALVERIFY OP_CHECKSIG
    if (address[0] == '1' && max_len >= 25) {
        script_out[0] = 0x76;  // OP_DUP
        script_out[1] = 0xA9;  // OP_HASH160
        script_out[2] = 0x14;  // Push 20 bytes
        
        // Decode base58 and extract hash160
        // Simplified - would need proper base58 decode
        
        script_out[23] = 0x88; // OP_EQUALVERIFY
        script_out[24] = 0xAC; // OP_CHECKSIG
        
        return 25;
    }
    
    // P2WPKH: OP_0 <20 bytes>
    if (strncmp(address, "bc1q", 4) == 0 && max_len >= 22) {
        script_out[0] = 0x00;  // OP_0
        script_out[1] = 0x14;  // Push 20 bytes
        // Extract witness program from bech32
        return 22;
    }
    
    return 0;
}

bool address_validate(const char *address) {
    if (!address) return false;
    
    // Check prefix
    if (address[0] == '1' || address[0] == '3') {
        // Base58 check - validate checksum
        return true;  // Simplified
    }
    
    if (strncmp(address, "bc1", 3) == 0) {
        // Bech32 check - validate checksum
        return true;  // Simplified
    }
    
    return false;
}

int address_get_type(const char *address) {
    if (!address) return -1;
    
    if (address[0] == '1') return ADDRESS_TYPE_LEGACY;
    if (address[0] == '3') return ADDRESS_TYPE_NESTED_SEGWIT;
    if (strncmp(address, "bc1q", 4) == 0) return ADDRESS_TYPE_NATIVE_SEGWIT;
    if (strncmp(address, "bc1p", 4) == 0) return ADDRESS_TYPE_TAPROOT;
    
    return -1;
}
