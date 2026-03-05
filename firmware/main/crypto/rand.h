/**
 * @file rand.h
 * @brief Secure random number generation
 */

#ifndef RAND_H
#define RAND_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Initialize random number generator
 */
void rand_init(void);

/**
 * @brief Generate random bytes
 * @param out Output buffer
 * @param len Number of bytes to generate
 * @return true on success
 */
bool rand_bytes(uint8_t *out, size_t len);

/**
 * @brief Generate random uint32
 */
uint32_t rand_uint32(void);

/**
 * @brief Generate random number in range [min, max]
 */
uint32_t rand_range(uint32_t min, uint32_t max);

#endif // RAND_H
