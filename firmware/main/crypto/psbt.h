/**
 * @file psbt.h
 * @brief PSBT (Partially Signed Bitcoin Transaction) parsing and signing
 */

#ifndef PSBT_H
#define PSBT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bip32.h"

#define PSBT_MAGIC          0x70736274  // "psbt"
#define PSBT_MAX_INPUTS     100
#define PSBT_MAX_OUTPUTS    100

// PSBT key types
#define PSBT_GLOBAL_VERSION             0x00
#define PSBT_GLOBAL_XPUB                0x01
#define PSBT_IN_NON_WITNESS_UTXO        0x00
#define PSBT_IN_WITNESS_UTXO            0x01
#define PSBT_IN_PARTIAL_SIG             0x02
#define PSBT_IN_SIGHASH_TYPE            0x03
#define PSBT_IN_REDEEM_SCRIPT           0x04
#define PSBT_IN_WITNESS_SCRIPT          0x05
#define PSBT_IN_BIP32_DERIVATION        0x06
#define PSBT_OUT_REDEEM_SCRIPT          0x00
#define PSBT_OUT_WITNESS_SCRIPT         0x01
#define PSBT_OUT_BIP32_DERIVATION       0x02

/**
 * @brief PSBT input structure
 */
typedef struct {
    uint8_t prev_txid[32];
    uint32_t prev_vout;
    uint32_t sequence;
    
    // Witness UTXO info
    uint64_t amount;
    uint8_t *witness_script;
    size_t witness_script_len;
    uint8_t *redeem_script;
    size_t redeem_script_len;
    
    // Derivation path for signing
    uint32_t derivation_path[5];
    size_t derivation_path_len;
    
    // Signature (after signing)
    uint8_t signature[72];
    size_t signature_len;
    
    // Sighash type
    uint32_t sighash_type;
    
    // Flags
    bool has_witness_utxo;
    bool has_non_witness_utxo;
    bool is_signed;
} psbt_input_t;

/**
 * @brief PSBT output structure
 */
typedef struct {
    uint64_t amount;
    uint8_t *script_pubkey;
    size_t script_pubkey_len;
    char address[64];
    
    // For change detection
    uint32_t derivation_path[5];
    size_t derivation_path_len;
    bool is_change;
} psbt_output_t;

/**
 * @brief PSBT transaction structure
 */
typedef struct {
    uint32_t version;
    uint32_t locktime;
    
    psbt_input_t *inputs;
    size_t inputs_count;
    
    psbt_output_t *outputs;
    size_t outputs_count;
    
    // Calculated values
    uint64_t total_input;
    uint64_t total_output;
    uint64_t fee;
} psbt_t;

/**
 * @brief Parse PSBT from raw bytes
 * @param data Raw PSBT data
 * @param len Data length
 * @param psbt Output structure
 * @return 0 on success, error code otherwise
 */
int psbt_parse(const uint8_t *data, size_t len, psbt_t *psbt);

/**
 * @brief Sign a PSBT input
 * @param psbt PSBT structure
 * @param input_index Index of input to sign
 * @param node HD node with private key
 * @return true on success
 */
bool psbt_sign_input(psbt_t *psbt, size_t input_index, const hd_node_t *node);

/**
 * @brief Sign all inputs we can
 * @param psbt PSBT structure
 * @param master Master HD node
 * @return Number of inputs signed
 */
size_t psbt_sign_all(psbt_t *psbt, const hd_node_t *master);

/**
 * @brief Serialize PSBT to bytes
 * @param psbt PSBT structure
 * @param out Output buffer
 * @param max_len Buffer size
 * @return Bytes written, or 0 on error
 */
size_t psbt_serialize(const psbt_t *psbt, uint8_t *out, size_t max_len);

/**
 * @brief Free PSBT resources
 */
void psbt_free(psbt_t *psbt);

/**
 * @brief Calculate transaction fee
 * @param psbt PSBT structure
 * @return Fee in satoshis
 */
uint64_t psbt_calculate_fee(psbt_t *psbt);

/**
 * @brief Check if output is change (to our wallet)
 * @param psbt PSBT structure
 * @param output_index Output index
 * @param master Master HD node for comparison
 * @return true if change
 */
bool psbt_is_change_output(psbt_t *psbt, size_t output_index, const hd_node_t *master);

#endif // PSBT_H
