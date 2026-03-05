/**
 * @file rand.c
 * @brief Random number implementation using ESP32 hardware RNG
 */

#include "rand.h"
#include "esp_random.h"
#include <string.h>

void rand_init(void) {
    // ESP32 hardware RNG is automatically initialized
    // Optionally reseed with entropy
}

bool rand_bytes(uint8_t *out, size_t len) {
    if (!out || len == 0) return false;
    
    // ESP32 has a hardware RNG that passes FIPS 140-2 tests
    // It's automatically enabled and provides true random numbers
    // when WiFi/BT are enabled, or uses oscillator jitter otherwise
    
    esp_fill_random(out, len);
    return true;
}

uint32_t rand_uint32(void) {
    return esp_random();
}

uint32_t rand_range(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    
    uint32_t range = max - min + 1;
    uint32_t r;
    
    // Avoid modulo bias
    uint32_t threshold = -range % range;
    
    do {
        r = esp_random();
    } while (r < threshold);
    
    return min + (r % range);
}
