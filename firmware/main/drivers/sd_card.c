#include "sd_card.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"

int sd_card_init(void) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    
    sdspi_device_config_t device_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    device_cfg.gpio_cs = 4;
    device_cfg.host_id = host.slot;
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdspi_mount("/sd", &host, &device_cfg, &mount_config, &card);
    
    return ret == ESP_OK ? 0 : -1;
}

int sd_card_write(const char* filename, const uint8_t* data, size_t len) {
    char path[256];
    snprintf(path, sizeof(path), "/sd/%s", filename);
    
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    
    fwrite(data, 1, len, f);
    fclose(f);
    return 0;
}

int sd_card_read(const char* filename, uint8_t* data, size_t max_len) {
    char path[256];
    snprintf(path, sizeof(path), "/sd/%s", filename);
    
    FILE* f = fopen(path, "rb");
    if (!f) return -1;
    
    size_t read = fread(data, 1, max_len, f);
    fclose(f);
    return read;
}
