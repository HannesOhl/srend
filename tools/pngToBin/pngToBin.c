#include "ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <inttypes.h>

// png uses big-endian
#define CHUNK_TYPE(i,j,k,l) ( (uint32_t) (l) << 24 | (uint32_t) (k) << 16 |	\
			      (uint32_t) (j) <<  8 | (uint32_t) (i) <<  0 )

#define IHDR CHUNK_TYPE('I', 'H', 'D', 'R')
#define IDAT CHUNK_TYPE('I', 'D', 'A', 'T')
#define IEND CHUNK_TYPE('I', 'E', 'N', 'D')

typedef struct {
	uint32_t length;
	uint32_t type;
	uint32_t crc;
} Chunk;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t bit_depth;
	uint32_t color_type;
} Ihdr;
Ihdr ihdr;

void header_print(uint8_t* b) {

	printf("8 byte png header:\n");
	printf("byte    1 (hex, should be   89): %02hhX\n", b[0]);
	printf("bytes 2-4 (asc, should be  PNG): %c%c%c\n", b[1], b[2], b[3]);
	printf("bytes 5-6 (hex, should be 0D0A): %02hhX%02hhX\n", b[4], b[5]);
	printf("byte    7 (hex, should be   1A): %02hhX\n", b[6]);
	printf("byte    8 (hex, should be   0A): %02hhX\n", b[7]);
}

bool header_check(uint8_t* b) {
	if (b[0] != 0x89) return false;
	if (b[1] != 0x50) return false;
	if (b[2] != 0x4E) return false;
	if (b[3] != 0x47) return false;
	if (b[4] != 0x0D) return false;
	if (b[5] != 0x0A) return false;
	if (b[6] != 0x1A) return false;
	if (b[7] != 0x0A) return false;
	return true;
}

void signature_verify(FILE* png) {

	const size_t HDR_SIZE = 8;

	uint8_t buffer_hdr[HDR_SIZE];
	fread(buffer_hdr, 1, HDR_SIZE, png);

	header_print(buffer_hdr);
	if (!header_check(buffer_hdr)) {
		fprintf(stderr, "Error, wrong header!\n");
		exit(2);
	} else {
		printf("PNG-header is correct!\n\n");
	}
}

/*
 *	Chunk layout:
 *  	 	|   Length  |  Chunk type  |    Chunk data    	|  CRC       |
 *    	  	|   4 bytes |  4 bytes     |    Length bytes  	|  4 bytes   |
 *
 *	The pointer into the file has to point to the beginnig of a chunk on
 *	function entry!
 *
 *      We return info about the chunk, ie. length, type and the CRC. Upon leaving
 * 	the function the pointer into the file points to the next chunk.
 *
 *      NO pointer to the chunk data is returned!
 */
Chunk chunk_get(FILE* png, size_t* chunk_current) {

	Chunk res = {};

	*chunk_current += 1;

	uint8_t buf[4];
	fread(buf, 1, 4, png);
	uint32_t length = 0;
	memcpy(&length, buf, 4);
	length = ntohl(length);
	res.length = length;

	uint8_t type[4];
	fread(type, 1, 4, png);
	memcpy(&res.type, type, 4);

	uint32_t type_hex = 0;
	memcpy(&type_hex, type, 4);


	printf("chunk %zu:\n", *chunk_current);
	printf("\tchunk info:\n");
	printf("\t\tlength = %lu\n", length);
	printf("\t\t  type = %.4s\n", type);
	printf("\t\t  type = 0x%08" PRIx32 "\n", type_hex);

	// move pointer into file to start of next chunk
	long offset = (long) length + 4;
	fseek(png, offset, SEEK_CUR);

	return res;
}

/*
 *	Upon entry the pointer into the file points to the first chunk after the IHDR.
 *      Upon exit the same is true.
 *
 */
void ihdr_info_print(FILE* png, Chunk c) {

		uint8_t ihdr_data[c.length];
		fread(ihdr_data, 1, c.length, png);

		uint32_t width      	    = 0; // +0
		uint32_t height     	    = 0; // +4
		 uint8_t bit_depth  	    = 0; // +8
		 uint8_t color_type 	    = 0; // +9
		 uint8_t compression_method = 0; // +10
		 uint8_t filter_method 	    = 0; // +11
		 uint8_t interlace_method   = 0; // +12

		memcpy(&width		  , ihdr_data +  0, 4);
		ihdr.width = ntohl(width);
		memcpy(&height		  , ihdr_data +  4, 4);
		ihdr.height = ntohl(height);
		memcpy(&bit_depth	  , ihdr_data +  8, 1);
		ihdr.bit_depth = bit_depth;
		memcpy(&color_type	  , ihdr_data +  9, 1);
		ihdr.color_type = color_type;
		memcpy(&compression_method, ihdr_data + 10, 1);
		memcpy(&filter_method     , ihdr_data + 11, 1);
		memcpy(&interlace_method  , ihdr_data + 12, 1);

		printf("\tdata info:\n");
		printf("\t\twidth              = %lu\n", ihdr.width);
		printf("\t\theight             = %lu\n", ihdr.height);
		printf("\t\tbit_depth          = %u\n", bit_depth);
		printf("\t\tcolor_type         = %u\n", color_type);
		printf("\t\tcompression_method = %u\n", compression_method);
		printf("\t\tfilter_method      = %u\n", filter_method);
		printf("\t\tinterlace_method   = %u\n", interlace_method);

		uint8_t ihdr_crc[4];
		fread(ihdr_crc, 1, 4, png);
		uint32_t crc = 0;
		memcpy(&crc, ihdr_crc, 4);

		printf("\tcrc: \n\t\t%08" PRIx32 "\n", crc);
		return;
}

void chunk_idat_number_calculate(FILE* png, size_t* out) {

	size_t chunk_idat_number = 0;
	size_t data_size         = 0;
	size_t chunk_current     = 0;
	Chunk c = chunk_get(png, &chunk_current);
	while (c.type != IEND) {
		if (c.type == IDAT) {
			chunk_idat_number += 1;
			data_size += c.length;
		}
		c = chunk_get(png, &chunk_current);
	}
	printf("Number of IDAT chunks = %zu\n", chunk_idat_number);
	printf("all chunks need %zu Byte memory.\n", data_size);
	*out = data_size;
}

void write_stream_compressed(FILE* png, uint8_t* buffer) {

	size_t offset = 0;
	size_t chunk_current = 0;

	Chunk c = chunk_get(png, &chunk_current);
	long pos_after_chunk = ftell(png);
	printf("c.length = %u\n", c.length);
	fseek(png, -(long)(c.length + 4), SEEK_CUR);
	ihdr_info_print(png, c);
	fseek(png, pos_after_chunk, SEEK_SET);
	while (c.type != IEND) {
		if (c.type == IDAT) {

			long pos_after_chunk = ftell(png);

			fseek(png, -(long)(c.length + 4), SEEK_CUR);

			fread(buffer + offset, 1, c.length, png);
			offset += c.length;

			fseek(png, pos_after_chunk, SEEK_SET);
		}
		c = chunk_get(png, &chunk_current);
	}

	return;
}

void out_name_create(char** argv, char* out_name, size_t out_size) {

	// compute base name (strip extension if present)
	const char* dot = strrchr(argv[1], '.');
	size_t base_len = dot ? (size_t)(dot - argv[1]) : strlen(argv[1]);

	// copy base name to out_name and append ".bin"
	char base[512];
	if (base_len >= sizeof(base)) base_len = sizeof(base) - 1;
	memcpy(base, argv[1], base_len);
	base[base_len] = '\0';
	snprintf(out_name, out_size, "%s.bin", base);
}

int main(int argc, char** argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: ./xpngToBin <input.png>\n");
		exit(1);
	}

	FILE* png = fopen(argv[1], "rb");


	char out_name[1024];
	out_name_create(argv, out_name, 1024);

	FILE* out = fopen(out_name, "wb");

	signature_verify(png);

	size_t chunks_idat_size = 0;
	chunk_idat_number_calculate(png, &chunks_idat_size);

	rewind(png);
	signature_verify(png);

	uint8_t* buffer = calloc((size_t) chunks_idat_size, 1);
	write_stream_compressed(png, buffer);

	//fwrite(buffer, 1, chunks_idat_size, out);

	int rc = decompress_idat_and_write_rgba(buffer, chunks_idat_size,
            					ihdr.width, ihdr.height,
						ihdr.bit_depth, ihdr.color_type,
						out_name);
	if (rc != 0) {
		fprintf(stderr, "Failed to decompress/unfilter: %d\n", rc);
	}


	free(buffer);
	fclose(png);
	fclose(out);

	return 0;
}

