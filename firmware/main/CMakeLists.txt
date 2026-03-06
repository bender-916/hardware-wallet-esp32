# Hardware Wallet ESP32-S3 - Main Component CMakeLists.txt
# Secure refactor with ESP32-S3 native security features

idf_component_register(
    SRCS "main.c"
         # Security modules
         "security/flash_enc.c"
         "security/secure_boot.c"
         "security/jtag.c"
         "security/hmac.c"
         # Wallet modules
         "wallet/state_machine.c"
         "wallet/key_management.c"
         "wallet/pin.c"
         # UI modules
         "ui/wizard.c"
         "ui/pin_entry.c"
         "ui/confirm.c"
         "ui/menu.c"
         # Crypto modules (existing)
         "crypto/bip39.c"
         "crypto/bip32.c"
         "crypto/bip44.c"
         "crypto/psbt.c"
         "crypto/address.c"
         "crypto/rand.c"
         "crypto/hash.c"
         # Drivers (existing)
         "drivers/oled.c"
         "drivers/sd_card.c"
         "drivers/buttons.c"
         "drivers/storage.c"
         # App modules (existing)
         "app/app.c"
         "app/ui.c"
         "app/menu.c"
    
    INCLUDE_DIRS "." 
                 "include" 
                 "security"
                 "wallet"
                 "ui"
                 "crypto"
                 "drivers"
                 "app"
    
    REQUIRES driver 
             fatfs 
             nvs_flash 
             spiffs 
             esp_timer
             mbedtls
             efuse
             boot_support
             secure_boot
             flash_encryption
)
