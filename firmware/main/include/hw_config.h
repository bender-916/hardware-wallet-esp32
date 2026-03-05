/**
 * @file hw_config.h
 * @brief Hardware configuration and pin definitions for ESP32-S3 Bitcoin Hardware Wallet
 */

#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "driver/gpio.h"

// ============== SPI BUS (SD Card) ==============
#define SD_MISO_PIN     GPIO_NUM_6
#define SD_MOSI_PIN     GPIO_NUM_5
#define SD_CLK_PIN      GPIO_NUM_7
#define SD_CS_PIN       GPIO_NUM_4
#define SD_DET_PIN      GPIO_NUM_3

// ============== I2C Bus (OLED) ==============
#define OLED_SDA_PIN    GPIO_NUM_8
#define OLED_SCL_PIN    GPIO_NUM_9
#define OLED_ADDR       0x3C
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

// ============== Buttons (Active LOW) ==============
#define BTN_UP_PIN      GPIO_NUM_16
#define BTN_DOWN_PIN    GPIO_NUM_17
#define BTN_OK_PIN      GPIO_NUM_14
#define BTN_CANCEL_PIN  GPIO_NUM_15
#define BTN_DEBOUNCE_MS 50

// ============== Status Indicators ==============
#define LED_STATUS_PIN  GPIO_NUM_18
#define BUZZER_PIN      GPIO_NUM_19

// ============== USB (Native) ==============
// GPIO20 = USB_D-, GPIO21 = USB_D+

// ============== Crypto Parameters ==============
#define SEED_SIZE       64      // 512 bits (32-byte seed + 32-byte chain code)
#define MNEMONIC_LEN    24      // 24 words = 256 bits entropy
#define PIN_MIN_LEN     6
#define PIN_MAX_LEN     8
#define PIN_MAX_ATTEMPTS 5

// ============== Derivation Paths ==============
#define BIP44_PURPOSE   0x8000002C  // 44' hardened
#define BIP44_COIN_BTC  0x80000000  // 0' hardened (Bitcoin mainnet)

// ============== Storage Keys ==============
#define NVS_NAMESPACE   "wallet"
#define NVS_KEY_INITIALIZED "init"
#define NVS_KEY_PIN_HASH    "pin_hash"
#define NVS_KEY_PIN_SALT    "pin_salt"
#define NVS_KEY_SEED_BLOB   "seed"

// ============== File Paths (SD Card) ==============
#define SD_MOUNT_POINT   "/sdcard"
#define SD_UNSIGNED_DIR  "/sdcard/unsigned"
#define SD_SIGNED_DIR    "/sdcard/signed"
#define SD_XPUB_DIR      "/sdcard/xpub"
#define SD_BACKUP_DIR    "/sdcard/backup"

#endif // HW_CONFIG_H
