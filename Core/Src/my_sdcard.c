#include "my_sdcard.h"
#include "fatfs.h"


int sdMount()
{
	if(f_mount(&SDFatFS, (TCHAR const*) SDPath, 0) != FR_OK)
	{
	  DEBUG_PRINTF("Unable to mount disk\r\n");
	  return SD_FAIL;
	}
	return SD_SUCCESS;
}

int sdOpenFile(char *filePath)
{
	if(f_open(&SDFile, filePath, FA_OPEN_EXISTING | FA_READ) != FR_OK)
	{
		DEBUG_PRINTF("Failed to open file: '%s'. Double-check the path and the existence of the file.\r\n", filePath);
		return SD_FAIL;
	}
	return SD_SUCCESS;
}

int sdReadFile(void *rdBuff, uint32_t bytesToRead)
{
	int bytesRead;
	if(f_read(&SDFile, rdBuff, bytesToRead, (void*) &bytesRead) != FR_OK)
	{
		DEBUG_PRINTF("Failed to read file. Double-check the file permissions.\r\n");
		return SD_FAIL;
	}
	if(bytesRead < bytesToRead)
	{
		DEBUG_PRINTF("NOTE: Reached end of file... ");
		DEBUG_PRINTF("Read %d bytes.\r\n", bytesRead);
		return SD_EOF;
	}
	DEBUG_PRINTF("Read %d bytes.\r\n", bytesRead);
	return SD_SUCCESS;
}

int sdCloseFile()
{
	if(f_close(&SDFile) != FR_OK)
	{
		DEBUG_PRINTF("Failed to close the currently open file. Double-check if a file was ever opened.\r\n");
		return SD_FAIL;
	}
	return SD_SUCCESS;
}

int sdUnmount()
{
	if(f_mount(0, (TCHAR const*) SDPath, 0) != FR_OK)
	{
	  DEBUG_PRINTF("Unable to unmount disk\r\n");
		return SD_FAIL;
	}
		return SD_SUCCESS;
}


int sdGetFileList(char fileList[MAX_FILE_NUM][64 + 1], char *dirPath, int *numFiles)
{
	DIR dir;
	FILINFO fInfo;
	*numFiles = 0;

	// Need to first open up the directory
	if(f_opendir(&dir, dirPath) != FR_OK)
	{
		DEBUG_PRINTF("Failed to open directory: '%s'\r\n", dirPath);
		return SD_FAIL;
	}

	// fetch all the files in the given directory
	do
	{
		if(f_readdir(&dir, &fInfo) != FR_OK)
		{
			DEBUG_PRINTF("Failed to read directory item. Aborting...\r\n");
			return SD_FAIL;
		}
		if(fInfo.fname[0] != 0) // have not reached end of dir, found valid file or dir
		{
			if(!(fInfo.fattrib & AM_DIR)) // not a dir, so its a file
			{
				strcpy(fileList[*numFiles], fInfo.fname); // grab the file name and save
				(*numFiles)++;
				if(*numFiles >= MAX_FILE_NUM)
				{
					DEBUG_PRINTF("WARNING: Reached the limit on how many files we can read per directory (limit = %d files per dir\r\n", MAX_FILE_NUM);
					DEBUG_PRINTF("Grabbing the first %d files in directory %s\r\n", MAX_FILE_NUM, dirPath);
				}
			}
		}
	} while((fInfo.fname[0] != 0) && (*numFiles < MAX_FILE_NUM));

	f_rewinddir(&dir); // macro for rewinding the directory to the start for next time
	if(f_closedir(&dir) != FR_OK) 	// close directory when finished
	{
		DEBUG_PRINTF("Failed to close directory: '%s'\r\n", dirPath);
		return SD_FAIL;
	}
	return SD_SUCCESS;
}

int sdLoadSong(char *songName)
{
	int len = strlen(SONG_DIR) + 1 + strlen(songName); // +1 for the extra '/' char
	if(len >= MAX_PATH_LEN)
	{
		DEBUG_PRINTF("ERROR: Failed to load song '%s/%s' because song path length (%d) exceeds maximum length of %d bytes.\r\n", SONG_DIR, songName, len, MAX_PATH_LEN - 1);
		DEBUG_PRINTF("Please shorten the length of the path.\r\n\n");
		return SD_FAIL;
	}
	char songPath[MAX_PATH_LEN] = SONG_DIR;
	strcat(songPath, "/");
	strcat(songPath, songName);

	int status = sdOpenFile(songPath);
	return status;
}

int sdReadSong(int16_t songBuff[SONG_BUFF_SIZE])
{
	return sdReadFile(songBuff, SONG_BUFF_SIZE * sizeof(int16_t));
}

