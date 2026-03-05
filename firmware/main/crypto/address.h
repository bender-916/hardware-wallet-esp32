/**
 * @file address.h
 * @brief Bitcoin address encoding
 */

#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    ADDRESS_TYPE_LEGACY = 0,        // P2PKH (starts with 1)
    ADDRESS_TYPE_NESTED_SEGWIT = 1, // P2SH-P2WPKH (starts with 3)
    ADDRESS_TYPE_NATIVE_SEGWIT = 2, // P2WPKH (starts with bc1q)
    ADDRESS_TYPE_TAPROOT = 3        // P2TR (starts with bc1p)
} address_type_t;

/**
 * @brief Generate address from public key
 * @param pubkey 33-byte compressed public key
 * @param type Address type
 * @param address_out Output buffer (min 64 bytes)
 * @param max_len Buffer size
 * @return true on success
 */
bool address_from_pubkey(const uint8_t *pubkey, address_type_t type,
                         char *address_out, size_t max_len);

/**
 * @brief Decode address to script pubkey
 * @param address Address string
 * @param script_out Output buffer for script
 * @param max_len Buffer size
 * @return Script length, or 0 on error
 */
size_t address_to_script(const char *address, uint8_t *script_out, size_t max_len);

/**
 * @brief Validate address format
 * @param address Address string
 * @return true if valid format
 */
bool address_validate(const char *address);

/**
 * @brief Get address type from string
 * @param address Address string
 * @return Address type, or -1 if unknown
 */
int address_get_type(const char *address);

#endif // ADDRESS_H
