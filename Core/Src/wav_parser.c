#include "wav_parser.h"


int parseWavHeader(wav_header_t *wavHeader)
{
	clearWavHeader(wavHeader);

	uint32_t rdBuff[HEADER_BUFF_SIZE]; // read the data into 4-byte boundaries
	if(readWavHeader((char *) rdBuff, sizeof(wav_header_t)) != PARSE_SUCCESS)
	{
		DEBUG_PRINTF("Failed to read in the .wav header.\r\n");
		return PARSE_FAIL;
	}

	int endianness = checkSysEndianness();
	// "RIFF" chunk descriptor portion
	wavHeader->ChunkID   = rdBuff[0];
	wavHeader->ChunkSize = rdBuff[1];
	wavHeader->Format    = rdBuff[2];
	// "fmt" sub-chunk
	wavHeader->Subchunk1ID   = rdBuff[3];
	wavHeader->Subchunk1Size = rdBuff[4];

	wavHeader->AudioFormat   = (endianness == SYS_BIG_ENDIAN) ? rdBuff[5] >> 16 : rdBuff[5];
	wavHeader->NumChannels   = (endianness == SYS_BIG_ENDIAN) ? rdBuff[5]       : rdBuff[5] >> 16;

	wavHeader->SampleRate    = rdBuff[6];
	wavHeader->ByteRate      = rdBuff[7];
	wavHeader->BlockAlign    = (endianness == SYS_BIG_ENDIAN) ? rdBuff[8] >> 16 : rdBuff[8];
	wavHeader->BitsPerSample = (endianness == SYS_BIG_ENDIAN) ? rdBuff[8]       : rdBuff[8] >> 16;

	// "data" sub-chunk (or potentially "list" subchunk. Will verify later)
	wavHeader->Subchunk2ID   = rdBuff[9];
	wavHeader->Subchunk2Size = rdBuff[10];



	switch(checkSysEndianness())
	{
	case SYS_LITTLE_ENDIAN:
		wavHeader->ChunkID = __builtin_bswap32(wavHeader->ChunkID);
		wavHeader->Format  = __builtin_bswap32(wavHeader->Format);
		wavHeader->Subchunk1ID = __builtin_bswap32(wavHeader->Subchunk1ID);
		wavHeader->Subchunk2ID = __builtin_bswap32(wavHeader->Subchunk2ID);
		break;


	case SYS_BIG_ENDIAN:
		wavHeader->ChunkSize     = __builtin_bswap32(wavHeader->ChunkSize);
		wavHeader->Subchunk1Size = __builtin_bswap32(wavHeader->Subchunk1Size);

		wavHeader->AudioFormat = __builtin_bswap16(wavHeader->AudioFormat);
		wavHeader->NumChannels = __builtin_bswap16(wavHeader->NumChannels);

		wavHeader->SampleRate = __builtin_bswap32(wavHeader->SampleRate);
		wavHeader->ByteRate   = __builtin_bswap32(wavHeader->ByteRate);

		wavHeader->BlockAlign    = __builtin_bswap16(wavHeader->BlockAlign);
		wavHeader->BitsPerSample = __builtin_bswap16(wavHeader->BitsPerSample);

		wavHeader->Subchunk2Size = __builtin_bswap32(wavHeader->Subchunk2Size);
		// Note: All the actual .WAV PCM data from here on out is little-endian, so be sure to perform a conversion for every sample if on big endian system
		break;
	}

	// Some .WAV files have a "list" sub-chunk that describes metadata such as artist, duration, etc.
	// We will be trying to skip this section in order to jump straight into the "data" subchunk
	wav_error_code_t errType;
	do
	{
		errType = verifyWavHeader(wavHeader);

		if(errType != WAV_VALID)
		{
			if(errType == SUBCHUNK_2_LIST_ERR) // good news, this is a fixable error
			{
				// perform the fix
				if(skipListSubChunk(wavHeader) != PARSE_SUCCESS)
				{
					return PARSE_FAIL;
				}
			}
			else // wav file is corrupted/not a wav file at all/uses unsupported feature of this project (modes other than PCM)
			{
				return PARSE_FAIL;
			}
		}
	} while(errType == SUBCHUNK_2_LIST_ERR);


	return PARSE_SUCCESS;
}

wav_error_code_t verifyWavHeader(wav_header_t *wavHeader)
{
	if(wavHeader->ChunkID != CHUNK_ID_RIFF)
	{
		return CHUNK_ID_ERR;
	}
	else if(wavHeader->Format != FORMAT_WAVE)
	{
		return FORMAT_ERR;
	}
	else if(wavHeader->Subchunk1ID != SUBCHUNK_1_ID_FMT)
	{
		return SUBCHUNK_1_ID_ERR;
	}
	else if(wavHeader->Subchunk1Size != SUBCHUNK_1_SIZE_PCM)
	{
		return SUBCHUNK_1_SIZE_ERR;
	}
	else if(wavHeader->AudioFormat != AUDIO_FORMAT_PCM)
	{
		return AUDIO_FORMAT_ERR;
	}
	else if(wavHeader->BitsPerSample != BITS_PER_SAMPLE_16) // only support signed 16-bit PCM
	{
		return BITS_PER_SAMPLE_ERR;
	}
	else if(wavHeader->ByteRate != ((wavHeader->SampleRate * wavHeader->NumChannels * wavHeader->BitsPerSample) >> 3))
	{
		return BYTE_RATE_ERR;
	}
	else if(wavHeader->BlockAlign != ((wavHeader->NumChannels * wavHeader->BitsPerSample) >> 3))
	{
		return BLOCK_ALIGN_ERR;
	}
	else if(wavHeader->Subchunk2ID != SUBCHUNK_2_ID_DATA) // we want Subchunk2ID to be DATA, since we have no use for LIST subchunk
	{
		if(wavHeader->Subchunk2ID == SUBCHUNK_2_ID_LIST) // if it is the LIST subchunk, we will have to fetch more data
		{
			return SUBCHUNK_2_LIST_ERR;
		}
		else // the data is just corrupted at this point, abort
		{
			return SUBCHUNK_2_ID_ERR;
		}
	}
	return WAV_VALID; // We're good to go!
}

int skipListSubChunk(wav_header_t *wavHeader)
{
	uint32_t bytesToSkip = wavHeader->Subchunk2Size;
	char listSubchunk2Buff[bytesToSkip];
	if(readWavHeader((char *) listSubchunk2Buff, bytesToSkip) != PARSE_SUCCESS)
	{
		DEBUG_PRINTF("Failed to read past the 'list' subchunk of the .wav header.\r\n");
		return PARSE_FAIL;
	}

	// Now the file system should be on the "data" chunk. Let's read that in next.
	uint32_t dataSubchunk2Buff[2];
	if(readWavHeader((char *) dataSubchunk2Buff, 8) != PARSE_SUCCESS)
	{
		DEBUG_PRINTF("Failed to read in the 'data' subchunk of the .wav header.\r\n");
		return PARSE_FAIL;
	}
	wavHeader->Subchunk2ID   = dataSubchunk2Buff[0];
	wavHeader->Subchunk2Size = dataSubchunk2Buff[1];

	switch(checkSysEndianness())
	{
		case SYS_LITTLE_ENDIAN:
			wavHeader->Subchunk2ID = __builtin_bswap32(wavHeader->Subchunk2ID);
			break;
		case SYS_BIG_ENDIAN:
			wavHeader->Subchunk2Size = __builtin_bswap32(wavHeader->Subchunk2Size);
			break;
	}
	return PARSE_SUCCESS;
}


void clearWavHeader(wav_header_t *wavHeader)
{
	memset(wavHeader, -1, sizeof(wav_header_t)); // init all struct members to 0xFF or 0xFFFF
}


int readWavHeader(char *rdBuff, uint32_t numBytes)
{
	// grab 44 byte header from storage platform (SD card in this case)
	if(sdReadFile((void *)rdBuff, numBytes) != SD_SUCCESS)
	{
		return PARSE_FAIL;
	}

	return PARSE_SUCCESS;
}

int checkSysEndianness()
{
	uint16_t testVal = 0x0001;
	char *testPtr = (char *) &testVal;
	if(*testPtr == 0x0)
	{
		return SYS_BIG_ENDIAN;
	}
	return SYS_LITTLE_ENDIAN;
}

