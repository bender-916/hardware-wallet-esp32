#ifndef SD_CARD_H
#define SD_CARD_H

int sd_card_init(void);
int sd_card_write(const char* filename, const uint8_t* data, size_t len);
int sd_card_read(const char* filename, uint8_t* data, size_t max_len);

#endif
