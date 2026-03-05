/**
 * @file errors.h
 * @brief Error codes for the hardware wallet
 */

#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    WALLET_OK = 0,
    
    // Initialization errors
    WALLET_ERR_NOT_INITIALIZED = -1,
    WALLET_ERR_ALREADY_INITIALIZED = -2,
    
    // PIN errors
    WALLET_ERR_INVALID_PIN = -10,
    WALLET_ERR_PIN_LOCKED = -11,
    WALLET_ERR_PIN_TOO_SHORT = -12,
    WALLET_ERR_PIN_TOO_LONG = -13,
    
    // Mnemonic errors
    WALLET_ERR_INVALID_MNEMONIC = -20,
    WALLET_ERR_MNEMONIC_CHECKSUM = -21,
    
    // Crypto errors
    WALLET_ERR_CRYPTO_FAIL = -30,
    WALLET_ERR_DERIVATION_FAIL = -31,
    WALLET_ERR_SIGN_FAIL = -32,
    
    // Storage errors
    WALLET_ERR_STORAGE_FAIL = -40,
    WALLET_ERR_STORAGE_FULL = -41,
    WALLET_ERR_STORAGE_CORRUPT = -42,
    
    // SD Card errors
    WALLET_ERR_SD_NOT_FOUND = -50,
    WALLET_ERR_SD_MOUNT_FAIL = -51,
    WALLET_ERR_SD_WRITE_FAIL = -52,
    WALLET_ERR_SD_READ_FAIL = -53,
    WALLET_ERR_FILE_NOT_FOUND = -54,
    
    // PSBT errors
    WALLET_ERR_PSBT_PARSE = -60,
    WALLET_ERR_PSBT_INVALID = -61,
    WALLET_ERR_PSBT_NO_INPUTS = -62,
    WALLET_ERR_PSBT_NO_OUTPUTS = -63,
    
    // Hardware errors
    WALLET_ERR_DISPLAY_INIT = -70,
    WALLET_ERR_BUTTON_INIT = -71,
    
    // Security errors
    WALLET_ERR_TAMPER_DETECTED = -80,
    WALLET_ERR_SECURE_BOOT_FAIL = -81,
    
    // Generic
    WALLET_ERR_UNKNOWN = -100
} wallet_err_t;

// Get error message string
const char* wallet_err_str(wallet_err_t err);

#endif // ERRORS_H
