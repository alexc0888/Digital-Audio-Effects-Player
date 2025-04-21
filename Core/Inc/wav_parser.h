/*
 * wav_parser.h
 *
 *  Created on: Apr 5, 2025
 *      Author: alexc0888 (Alex Chitsazzadeh)
 */

#ifndef __WAV_PARSER__
#define __WAV_PARSER__

#include "my_sdcard.h"

typedef struct
{
	// "RIFF" chunk descriptor portion
	uint32_t ChunkID;       // 0x0
	uint32_t ChunkSize;     // 0x4
	uint32_t Format;        // 0x8

	// "fmt" sub-chunk
	uint32_t Subchunk1ID;   // 0xC
	uint32_t Subchunk1Size; // 0x10
	uint16_t AudioFormat;   // 0x14
	uint16_t NumChannels;   // 0x16
	uint32_t SampleRate;    // 0x18
	uint32_t ByteRate;      // 0x1C
	uint16_t BlockAlign;    // 0x20
	uint16_t BitsPerSample; // 0x22

	// "data" sub-chunk
	uint32_t Subchunk2ID;   // 0x24
	uint32_t Subchunk2Size; // 0x28
	                        // 0x2C and beyond is the actual PCM data portion of the wav file
} wav_header_t;

#define PARSE_SUCCESS 0x1
#define PARSE_FAIL    0x0
#define SYS_LITTLE_ENDIAN 0x1
#define SYS_BIG_ENDIAN    0x0

#define HEADER_BUFF_SIZE sizeof(wav_header_t) / sizeof(uint32_t)

#define CHUNK_ID_RIFF     0x52494646
#define FORMAT_WAVE       0x57415645
#define SUBCHUNK_1_ID_FMT 0x666d7420
#define SUBCHUNK_1_SIZE_PCM 16 // 16 means the file is encoded with PCM, otherwise means there may be compression which is not supported by this project
#define AUDIO_FORMAT_PCM   1 // again, means the file is encoded with PCM, no compression applied
#define SUBCHUNK_2_ID_DATA 0x64617461
#define SUBCHUNK_2_ID_LIST 0x4c495354
#define BITS_PER_SAMPLE_16 16

typedef enum
{
	WAV_VALID,

	// "riff" part errs
	CHUNK_ID_ERR,
	CHUNK_SIZE_ERR,
	FORMAT_ERR,

	// "fmt" part errs
	SUBCHUNK_1_ID_ERR,
	SUBCHUNK_1_SIZE_ERR,
	AUDIO_FORMAT_ERR,
	BITS_PER_SAMPLE_ERR,
	BYTE_RATE_ERR,
	BLOCK_ALIGN_ERR,

	// "data" part errs
	SUBCHUNK_2_ID_ERR,
	SUBCHUNK_2_LIST_ERR // We are ignoring the list subchunk, so if we get this, we need to clobber subchunk_2 with the "data" chunk
} wav_error_code_t;

int parseWavHeader(wav_header_t *);

wav_error_code_t verifyWavHeader(wav_header_t *);
int skipListSubChunk(wav_header_t *);
void clearWavHeader(wav_header_t *);
int readWavHeader(char *, uint32_t);

int checkSysEndianness();


#endif
