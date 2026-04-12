#ifndef EXT_H
#define EXT_H

#include <stdint.h>
#include <stddef.h>

int decompress_idat_and_write_rgba(const uint8_t *idat_z, size_t idat_z_len,
                                   uint32_t width, uint32_t height,
                                   uint8_t bit_depth, uint8_t color_type,
                                   const char *out_filename);
#endif

