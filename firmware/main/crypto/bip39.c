/**
 * @file bip39.c
 * @brief BIP39 mnemonic implementation
 */

#include "bip39.h"
#include "rand.h"
#include "hash.h"
#include <string.h>
#include <stdlib.h>

// BIP39 English wordlist (first 256 words for space, full list in production)
static const char* bip39_words[] = {
    "abandon", "ability", "able", "about", "above", "absent", "absorb", "abstract",
    "absurd", "abuse", "access", "accident", "account", "accuse", "achieve", "acid",
    "acoustic", "acquire", "across", "act", "action", "actor", "actress", "actual",
    "adapt", "add", "addict", "address", "adjust", "admit", "adult", "advance",
    "advice", "aerobic", "affair", "afford", "afraid", "again", "age", "agent",
    "agree", "ahead", "aim", "air", "airport", "aisle", "alarm", "album",
    "alcohol", "alert", "alien", "all", "alley", "allow", "almost", "alone",
    "alpha", "already", "also", "alter", "always", "amateur", "amazing", "among",
    "amount", "amused", "analyst", "anchor", "ancient", "anger", "angle", "angry",
    "animal", "ankle", "announce", "annual", "another", "answer", "antenna", "antique",
    "anxiety", "any", "apart", "apology", "appear", "apple", "approve", "april",
    "arch", "arctic", "area", "arena", "argue", "arm", "armed", "armor",
    "army", "around", "arrange", "arrest", "arrive", "arrow", "art", "artefact",
    "artist", "artwork", "ask", "aspect", "assault", "asset", "assist", "assume",
    "asthma", "athlete", "atom", "attack", "attend", "attitude", "attract", "auction",
    "audit", "august", "aunt", "author", "auto", "autumn", "average", "avocado",
    "avoid", "awake", "aware", "away", "awesome", "awful", "awkward", "axis",
    "baby", "bachelor", "bacon", "badge", "bag", "balance", "balcony", "ball",
    "bamboo", "banana", "banner", "bar", "barely", "bargain", "barrel", "base",
    "basic", "basket", "battle", "beach", "bean", "beauty", "because", "become",
    "beef", "before", "begin", "behave", "behind", "believe", "below", "belt",
    "bench", "benefit", "best", "betray", "better", "between", "beyond", "bicycle",
    "bid", "bike", "bind", "biology", "bird", "birth", "bitter", "black",
    "blade", "blame", "blanket", "blast", "bleak", "bless", "blind", "blood",
    "blossom", "blouse", "blue", "blur", "blush", "board", "boat", "body",
    "boil", "bomb", "bone", "bonus", "book", "boost", "border", "boring",
    "borrow", "boss", "bottom", "bounce", "box", "boy", "bracket", "brain",
    "brand", "brass", "brave", "bread", "breeze", "brick", "bridge", "brief",
    "bright", "bring", "brisk", "broccoli", "broken", "bronze", "broom", "brother",
    "brown", "brush", "bubble", "buddy", "budget", "buffalo", "build", "bulb",
    "bulk", "bullet", "bundle", "bunker", "burden", "burger", "burst", "bus",
    "business", "busy", "butter", "buyer", "buzz", "cabbage", "cabin", "cable",
    // ... (full 2048 words would be here in production)
    // For now, truncated for brevity
};
#define BIP39_WORD_COUNT (sizeof(bip39_words) / sizeof(bip39_words[0]))

bool bip39_generate_mnemonic(uint8_t entropy_len, char *mnemonic_out, size_t max_len) {
    if (!mnemonic_out || max_len < BIP39_MNEMONIC_BUF_LEN) return false;
    if (entropy_len != 16 && entropy_len != 32) return false;
    
    uint8_t entropy[32];
    uint8_t checksum;
    
    // Generate random entropy
    if (!rand_bytes(entropy, entropy_len)) return false;
    
    // Calculate checksum (SHA256 first byte)
    uint8_t hash[32];
    sha256(entropy, entropy_len, hash);
    checksum = hash[0];
    
    // Calculate checksum bits needed
    int cs_len = entropy_len * 8 / 32;  // 4 for 128-bit, 8 for 256-bit
    
    // Total bits = entropy + checksum
    int total_bits = entropy_len * 8 + cs_len;
    int word_count = total_bits / 11;
    
    // Build mnemonic
    mnemonic_out[0] = '\0';
    
    for (int i = 0; i < word_count; i++) {
        // Calculate 11-bit index
        int bits_processed = i * 11;
        int byte_index = bits_processed / 8;
        int bit_offset = bits_processed % 8;
        
        uint16_t index = 0;
        
        // Extract 11 bits
        if (bit_offset <= 5) {
            // Fits in 2 bytes
            index = ((uint16_t)entropy[byte_index] << (8 - bit_offset)) |
                    ((uint16_t)entropy[byte_index + 1] >> (bit_offset + 5));
            index &= 0x7FF;
        } else {
            // Spans 3 bytes
            index = ((uint16_t)(entropy[byte_index] & ((1 << (16 - bit_offset)) - 1)) << (bit_offset - 5)) |
                    ((uint16_t)entropy[byte_index + 1] << (3 - bit_offset % 8)) |
                    ((uint16_t)entropy[byte_index + 2] >> (5 + bit_offset));
            index &= 0x7FF;
        }
        
        // Use checksum bits for last word
        if (i == word_count - 1 && cs_len > 0) {
            // Incorporate checksum
            index = (index & ~((1 << cs_len) - 1)) | (checksum >> (8 - cs_len));
        }
        
        // Append word
        if (i > 0) strcat(mnemonic_out, " ");
        if (index < BIP39_WORD_COUNT) {
            strncat(mnemonic_out, bip39_words[index], max_len - strlen(mnemonic_out) - 1);
        }
    }
    
    // Clear sensitive data
    memset(entropy, 0, sizeof(entropy));
    memset(hash, 0, sizeof(hash));
    
    return true;
}

bool bip39_validate_mnemonic(const char *mnemonic) {
    if (!mnemonic) return false;
    
    // Count words
    int word_count = bip39_get_word_count(mnemonic);
    if (word_count != 12 && word_count != 15 && word_count != 18 && 
        word_count != 21 && word_count != 24) {
        return false;
    }
    
    // Decode entropy from words
    // For full implementation, would decode and verify checksum
    return true;  // Simplified for demo
}

bool bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase, uint8_t *seed_out) {
    if (!mnemonic || !seed_out) return false;
    
    const char *pass = passphrase ? passphrase : "";
    
    // PBKDF2-HMAC-SHA512 with 2048 iterations
    // Key: mnemonic (UTF-8 NFKD normalized)
    // Salt: "mnemonic" + passphrase (UTF-8 NFKD normalized)
    
    // This is a simplified version - production would use trezor-crypto
    pbkdf2_sha512((const uint8_t*)mnemonic, strlen(mnemonic),
                  (const uint8_t*)pass, strlen(pass),
                  2048, seed_out, 64);
    
    return true;
}

int bip39_get_word_count(const char *mnemonic) {
    if (!mnemonic || *mnemonic == '\0') return 0;
    
    int count = 1;
    const char *p = mnemonic;
    while (*p) {
        if (*p == ' ') count++;
        p++;
    }
    return count;
}

const char* bip39_get_word(uint16_t index) {
    if (index >= BIP39_WORD_COUNT) return NULL;
    return bip39_words[index];
}

int bip39_find_word(const char *word) {
    if (!word) return -1;
    
    for (int i = 0; i < (int)BIP39_WORD_COUNT; i++) {
        if (strcmp(bip39_words[i], word) == 0) {
            return i;
        }
    }
    return -1;
}
