// THIS FILE IS CURRENTLY VIBECODED (until i take a look at DEFLATE in detail)
// png_inflate_and_unfilter.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <zlib.h>

/* Paeth predictor from PNG spec */
static inline uint8_t paeth_predictor(int a, int b, int c) {
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    if (pa <= pb && pa <= pc) return (uint8_t)a;
    if (pb <= pc) return (uint8_t)b;
    return (uint8_t)c;
}

/* Decompress zlib-wrapped buffer into *out, returns 0 on success */
static int decompress_zlib_buffer(const uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len) {
    if (!in || !out || !out_len) return -1;
    int ret;
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    strm.next_in = (Bytef *)in;
    strm.avail_in = (uInt)in_len;

    if (inflateInit(&strm) != Z_OK) return -2;

    size_t cap = in_len ? in_len * 3 : 65536; // initial guess
    uint8_t *buf = malloc(cap);
    if (!buf) { inflateEnd(&strm); return -3; }

    do {
        if (strm.total_out >= cap) {
            size_t newcap = cap * 2;
            uint8_t *tmp = realloc(buf, newcap);
            if (!tmp) { free(buf); inflateEnd(&strm); return -4; }
            buf = tmp;
            cap = newcap;
        }
        strm.next_out = buf + strm.total_out;
        strm.avail_out = (uInt)(cap - strm.total_out);

        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR || ret == Z_STREAM_ERROR) {
            free(buf);
            inflateEnd(&strm);
            return -5;
        }
    } while (ret != Z_STREAM_END);

    *out_len = (size_t)strm.total_out;
    *out = buf;

    inflateEnd(&strm);
    return 0;
}

/*
 * Reconstruct scanlines (PNG filters) and convert to 8-bit RGBA.
 *
 * idat_z: compressed zlib buffer
 * idat_z_len: length
 * width,height,bit_depth,color_type from IHDR
 * out_pixels: allocated by function, caller free()
 * out_size: number of bytes (width*height*4)
 *
 * Returns 0 on success, negative on error.
 */
int decompress_idat_and_write_rgba(const uint8_t *idat_z, size_t idat_z_len,
                                   uint32_t width, uint32_t height,
                                   uint8_t bit_depth, uint8_t color_type,
                                   const char *out_filename) {
    if (!idat_z || !out_filename) return -1;
    if (bit_depth != 8 && bit_depth != 16) {
        fprintf(stderr, "Only bit depths 8 and 16 are supported here (got %u)\n", bit_depth);
        return -2;
    }

    /* channels per color type */
    int channels;
    switch (color_type) {
        case 0: channels = 1; break; // grayscale
        case 2: channels = 3; break; // RGB
        case 3: fprintf(stderr, "Palette (color type 3) not supported by this function.\n"); return -3;
        case 4: channels = 2; break; // grayscale+alpha
        case 6: channels = 4; break; // RGBA
        default: fprintf(stderr, "Unknown color type %u\n", color_type); return -4;
    }

    int bytes_per_sample = bit_depth / 8; // 1 or 2
    size_t bpp = (size_t)channels * bytes_per_sample; // bytes per pixel (for filter left reference)
    /* row data length (no filter byte) */
    uint64_t row_bytes = (uint64_t)width * bpp;
    if (row_bytes > SIZE_MAX / 2) { fprintf(stderr, "row_bytes too large\n"); return -5; }

    /* decompress zlib data */
    uint8_t *decomp = NULL;
    size_t decomp_len = 0;
    int rc = decompress_zlib_buffer(idat_z, idat_z_len, &decomp, &decomp_len);
    if (rc != 0) {
        fprintf(stderr, "zlib decompress failed (%d)\n", rc);
        return -6;
    }

    /* Expect at least height * (1 + row_bytes) bytes */
    uint64_t expected_min = (uint64_t)height * (1 + row_bytes);
    if (decomp_len < expected_min) {
        fprintf(stderr, "decompressed size (%zu) smaller than expected (%" PRIu64 ")\n", decomp_len, expected_min);
        free(decomp);
        return -7;
    }

    /* allocate output RGBA */
    uint64_t pixel_count = (uint64_t)width * height;
    if (pixel_count == 0) { free(decomp); return -8; }
    uint64_t out_bytes = pixel_count * 4;
    uint8_t *out = malloc(out_bytes);
    if (!out) { free(decomp); return -9; }

    /* buffers for previous and current reconstructed rows (raw samples) */
    uint8_t *prev_row = calloc(1, row_bytes);
    uint8_t *cur_row  = malloc(row_bytes);
    uint8_t *recon_row = malloc(row_bytes);
    if (!prev_row || !cur_row || !recon_row) {
        free(decomp); free(out); free(prev_row); free(cur_row); free(recon_row);
        return -10;
    }

    size_t pos = 0;
    size_t out_off = 0;

    for (uint32_t y = 0; y < height; ++y) {
        if (pos >= decomp_len) { fprintf(stderr, "unexpected EOF in decompressed data\n"); rc = -11; goto cleanup; }
        uint8_t filter_type = decomp[pos++];
        /* read row bytes */
        memcpy(cur_row, decomp + pos, row_bytes);
        pos += row_bytes;

        /* apply filter to produce recon_row */
        switch (filter_type) {
            case 0: // None
                for (uint64_t i = 0; i < row_bytes; ++i) recon_row[i] = cur_row[i];
                break;
            case 1: // Sub
                for (uint64_t i = 0; i < row_bytes; ++i) {
                    uint8_t left = (i >= bpp) ? recon_row[i - bpp] : 0;
                    recon_row[i] = (uint8_t)(cur_row[i] + left);
                }
                break;
            case 2: // Up
                for (uint64_t i = 0; i < row_bytes; ++i) {
                    uint8_t up = prev_row[i];
                    recon_row[i] = (uint8_t)(cur_row[i] + up);
                }
                break;
            case 3: // Average
                for (uint64_t i = 0; i < row_bytes; ++i) {
                    uint8_t left = (i >= bpp) ? recon_row[i - bpp] : 0;
                    uint8_t up   = prev_row[i];
                    uint8_t val = (uint8_t)(cur_row[i] + ((left + up) >> 1));
                    recon_row[i] = val;
                }
                break;
            case 4: // Paeth
                for (uint64_t i = 0; i < row_bytes; ++i) {
                    int left = (i >= bpp) ? recon_row[i - bpp] : 0;
                    int up = prev_row[i];
                    int up_left = (i >= bpp) ? prev_row[i - bpp] : 0;
                    uint8_t pr = paeth_predictor(left, up, up_left);
                    recon_row[i] = (uint8_t)(cur_row[i] + pr);
                }
                break;
            default:
                fprintf(stderr, "Unsupported filter type %u at row %u\n", filter_type, y);
                rc = -12;
                goto cleanup;
        }

        /* convert recon_row (samples) to RGBA 8-bit per pixel */
        uint8_t *rptr = recon_row;
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t R = 0, G = 0, B = 0, A = 255;
            if (bit_depth == 8) {
                if (color_type == 6) { // RGBA
                    R = rptr[0]; G = rptr[1]; B = rptr[2]; A = rptr[3];
                } else if (color_type == 2) { // RGB
                    R = rptr[0]; G = rptr[1]; B = rptr[2]; A = 255;
                } else if (color_type == 0) { // grayscale
                    R = G = B = rptr[0]; A = 255;
                } else if (color_type == 4) { // gray+alpha
                    R = G = B = rptr[0]; A = rptr[1];
                }
                rptr += bpp;
            } else { // bit_depth == 16, take MSB of each sample
                if (color_type == 6) {
                    R = rptr[0]; G = rptr[2]; B = rptr[4]; A = rptr[6];
                } else if (color_type == 2) {
                    R = rptr[0]; G = rptr[2]; B = rptr[4]; A = 255;
                } else if (color_type == 0) {
                    R = G = B = rptr[0]; A = 255;
                } else if (color_type == 4) {
                    R = G = B = rptr[0]; A = rptr[2];
                }
                rptr += bpp;
            }
            out[out_off++] = R;
            out[out_off++] = G;
            out[out_off++] = B;
            out[out_off++] = A;
        }

        /* swap prev_row/recon_row for next iteration (prev = recon) */
        uint8_t *tmp = prev_row;
        prev_row = recon_row;
        recon_row = tmp;
    }

    /* write output to file */
    FILE *fout = fopen(out_filename, "wb");
    if (!fout) { fprintf(stderr, "Failed to open %s for writing\n", out_filename); rc = -13; goto cleanup; }
    if (fwrite(out, 1, out_bytes, fout) != out_bytes) {
        fprintf(stderr, "Failed to write all pixel bytes to %s\n", out_filename);
        fclose(fout);
        rc = -14;
        goto cleanup;
    }
    uint32_t width_out = width;
    uint32_t height_out = height;
    fwrite(&width_out, 1, 4, fout);
    fwrite(&height_out, 1, 4, fout);
    fclose(fout);
    printf("Wrote %zu RGBA bytes (%u x %u) to %s\n", (size_t)out_bytes, width, height, out_filename);
    rc = 0;

cleanup:
    free(decomp);
    free(out);
    free(prev_row);
    free(cur_row);
    free(recon_row);
    return rc;
}

