/**
 * @file bip39.h
 * @brief BIP39 mnemonic generation and validation
 */

#ifndef BIP39_H
#define BIP39_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// BIP39 entropy strengths
#define BIP39_ENTROPY_LEN_128   16      // 12 words
#define BIP39_ENTROPY_LEN_256   32      // 24 words

// Mnemonic buffer sizes
#define BIP39_MNEMONIC_BUF_LEN  256     // Max mnemonic string length
#define BIP39_SEED_LEN          64      // 512 bits

/**
 * @brief Generate a new BIP39 mnemonic
 * @param entropy_len 16 for 12 words, 32 for 24 words
 * @param mnemonic_out Buffer to receive mnemonic string
 * @param max_len Size of output buffer
 * @return true on success
 */
bool bip39_generate_mnemonic(uint8_t entropy_len, char *mnemonic_out, size_t max_len);

/**
 * @brief Validate a BIP39 mnemonic
 * @param mnemonic Mnemonic string to validate
 * @return true if valid
 */
bool bip39_validate_mnemonic(const char *mnemonic);

/**
 * @brief Convert mnemonic to seed
 * @param mnemonic Mnemonic string
 * @param passphrase Optional passphrase (can be NULL or empty)
 * @param seed_out 64-byte buffer for seed output
 * @return true on success
 */
bool bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase, uint8_t *seed_out);

/**
 * @brief Get word count from mnemonic
 * @param mnemonic Mnemonic string
 * @return Number of words (0 if invalid)
 */
int bip39_get_word_count(const char *mnemonic);

/**
 * @brief Get a single word from the wordlist by index
 * @param index Word index (0-2047)
 * @return Word string, or NULL if invalid index
 */
const char* bip39_get_word(uint16_t index);

/**
 * @brief Find word index in wordlist
 * @param word Word to find
 * @return Index (0-2047), or -1 if not found
 */
int bip39_find_word(const char *word);

#endif // BIP39_H
