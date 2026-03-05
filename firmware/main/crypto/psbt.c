/**
 * @file psbt.c
 * @brief PSBT implementation
 */

#include "psbt.h"
#include "hash.h"
#include "address.h"
#include <string.h>
#include <stdlib.h>

// Read compact size
static size_t read_compact_size(const uint8_t *data, size_t *offset) {
    uint8_t first = data[(*offset)++];
    if (first < 0xFD) return first;
    if (first == 0xFD) {
        size_t val = data[*offset] | (data[*offset + 1] << 8);
        *offset += 2;
        return val;
    }
    if (first == 0xFE) {
        size_t val = data[*offset] | (data[*offset + 1] << 8) |
                     (data[*offset + 2] << 16) | (data[*offset + 3] << 24);
        *offset += 4;
        return val;
    }
    // 0xFF - 64-bit (not needed for our use case)
    *offset += 8;
    return 0;
}

// Write compact size
static size_t write_compact_size(uint8_t *data, size_t val) {
    if (val < 0xFD) {
        data[0] = (uint8_t)val;
        return 1;
    }
    if (val <= 0xFFFF) {
        data[0] = 0xFD;
        data[1] = val & 0xFF;
        data[2] = (val >> 8) & 0xFF;
        return 3;
    }
    data[0] = 0xFE;
    data[1] = val & 0xFF;
    data[2] = (val >> 8) & 0xFF;
    data[3] = (val >> 16) & 0xFF;
    data[4] = (val >> 24) & 0xFF;
    return 5;
}

int psbt_parse(const uint8_t *data, size_t len, psbt_t *psbt) {
    if (!data || !psbt) return -1;
    
    memset(psbt, 0, sizeof(psbt_t));
    
    size_t offset = 0;
    
    // Check magic bytes
    if (len < 5 || data[0] != 0x70 || data[1] != 0x73 || 
        data[2] != 0x62 || data[3] != 0x74 || data[4] != 0xFF) {
        return -2;  // Invalid magic
    }
    offset = 5;
    
    // Parse global section
    while (offset < len) {
        // Check for separator
        if (data[offset] == 0x00) {
            offset++;
            break;
        }
        
        // Read key
        size_t key_len = read_compact_size(data, &offset);
        if (offset + key_len > len) return -3;
        
        uint8_t key_type = data[offset++];
        
        // Skip key data and value
        size_t key_data_len = key_len - 1;
        offset += key_data_len;
        
        size_t value_len = read_compact_size(data, &offset);
        offset += value_len;
    }
    
    // Parse inputs section
    while (offset < len) {
        if (data[offset] == 0x00) {
            offset++;
            break;
        }
        
        // For simplicity, just count inputs and skip key-value pairs
        // Full implementation would parse each field
        
        size_t key_len = read_compact_size(data, &offset);
        offset += key_len;
        
        size_t value_len = read_compact_size(data, &offset);
        offset += value_len;
        
        psbt->inputs_count++;
    }
    
    // Parse outputs section
    while (offset < len) {
        if (data[offset] == 0x00) {
            offset++;
            break;
        }
        
        size_t key_len = read_compact_size(data, &offset);
        offset += key_len;
        
        size_t value_len = read_compact_size(data, &offset);
        offset += value_len;
        
        psbt->outputs_count++;
    }
    
    // Allocate inputs and outputs
    psbt->inputs = (psbt_input_t*)calloc(psbt->inputs_count, sizeof(psbt_input_t));
    psbt->outputs = (psbt_output_t*)calloc(psbt->outputs_count, sizeof(psbt_output_t));
    
    // In a full implementation, would re-parse to extract actual data
    
    return 0;
}

bool psbt_sign_input(psbt_t *psbt, size_t input_index, const hd_node_t *node) {
    if (!psbt || input_index >= psbt->inputs_count || !node) return false;
    
    psbt_input_t *input = &psbt->inputs[input_index];
    
    // Calculate sighash
    // For witness inputs: sighash = hash256(serialized tx with input being signed)
    uint8_t sighash[32];
    uint8_t preimage[256];  // Simplified - would build proper preimage
    
    // Calculate hash to sign
    double_sha256(preimage, 0, sighash);
    
    // Sign with ECDSA
    // This would use secp256k1 signing from trezor-crypto
    // ecdsa_sign(node->private_key, sighash, input->signature, &input->signature_len);
    
    input->is_signed = true;
    
    return true;
}

size_t psbt_sign_all(psbt_t *psbt, const hd_node_t *master) {
    if (!psbt || !master) return 0;
    
    size_t signed_count = 0;
    
    for (size_t i = 0; i < psbt->inputs_count; i++) {
        // Check if we have derivation path for this input
        if (psbt->inputs[i].derivation_path_len > 0) {
            // Derive key for this input
            hd_node_t node;
            memcpy(&node, master, sizeof(hd_node_t));
            
            if (bip32_derive_path(&node, psbt->inputs[i].derivation_path, 
                                  psbt->inputs[i].derivation_path_len)) {
                if (psbt_sign_input(psbt, i, &node)) {
                    signed_count++;
                }
            }
            
            bip32_node_clear(&node);
        }
    }
    
    return signed_count;
}

size_t psbt_serialize(const psbt_t *psbt, uint8_t *out, size_t max_len) {
    if (!psbt || !out || max_len < 5) return 0;
    
    size_t offset = 0;
    
    // Magic bytes
    out[offset++] = 0x70;
    out[offset++] = 0x73;
    out[offset++] = 0x62;
    out[offset++] = 0x74;
    out[offset++] = 0xFF;
    
    // Global section (empty for now)
    out[offset++] = 0x00;
    
    // Input section placeholder
    out[offset++] = 0x00;
    
    // Output section placeholder
    out[offset++] = 0x00;
    
    return offset;
}

void psbt_free(psbt_t *psbt) {
    if (!psbt) return;
    
    if (psbt->inputs) {
        for (size_t i = 0; i < psbt->inputs_count; i++) {
            free(psbt->inputs[i].witness_script);
            free(psbt->inputs[i].redeem_script);
        }
        free(psbt->inputs);
    }
    
    if (psbt->outputs) {
        for (size_t i = 0; i < psbt->outputs_count; i++) {
            free(psbt->outputs[i].script_pubkey);
        }
        free(psbt->outputs);
    }
    
    memset(psbt, 0, sizeof(psbt_t));
}

uint64_t psbt_calculate_fee(psbt_t *psbt) {
    if (!psbt) return 0;
    
    psbt->total_input = 0;
    psbt->total_output = 0;
    
    for (size_t i = 0; i < psbt->inputs_count; i++) {
        psbt->total_input += psbt->inputs[i].amount;
    }
    
    for (size_t i = 0; i < psbt->outputs_count; i++) {
        psbt->total_output += psbt->outputs[i].amount;
    }
    
    psbt->fee = psbt->total_input - psbt->total_output;
    return psbt->fee;
}

bool psbt_is_change_output(psbt_t *psbt, size_t output_index, const hd_node_t *master) {
    if (!psbt || output_index >= psbt->outputs_count || !master) return false;
    
    psbt_output_t *output = &psbt->outputs[output_index];
    
    // Check if derivation path indicates change (index 1 in path)
    if (output->derivation_path_len >= 4) {
        return output->derivation_path[3] == 1;  // Change index
    }
    
    return false;
}
