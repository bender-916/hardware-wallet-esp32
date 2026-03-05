idf_component_register(
    SRCS "main.c"
         "app/app.c"
         "app/ui.c"
         "app/menu.c"
         "crypto/bip39.c"
         "crypto/bip32.c"
         "crypto/bip44.c"
         "crypto/psbt.c"
         "crypto/address.c"
         "crypto/rand.c"
         "crypto/hash.c"
         "drivers/oled.c"
         "drivers/sd_card.c"
         "drivers/buttons.c"
         "drivers/storage.c"
    INCLUDE_DIRS "." "include" "app" "crypto" "drivers"
    REQUIRES driver fatfs nvs_flash spiffs esp_timer
)
