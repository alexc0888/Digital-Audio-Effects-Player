#include "my_sdcard.h"
#include "fatfs.h"


int sdMount()
{
	int status = SD_SUCCESS;
	if(f_mount(&SDFatFS, (TCHAR const*) SDPath, 0) != FR_OK)
	{
	  DEBUG_PRINTF("Unable to mount disk\r\n");
	  status = SD_FAIL;
	}
	return status;
}

int sdGetFileList(char fileList[MAX_FILE_NUM][64 + 1], char *dirPath, int *numFiles)
{
	int status = SD_SUCCESS;
	DIR dir;
	FILINFO fInfo;
	*numFiles = 0;

	// Need to first open up the directory
	if(f_opendir(&dir, dirPath) != FR_OK)
	{
		DEBUG_PRINTF("Failed to open directory: '%s'\r\n", dirPath);
		status = SD_FAIL;
	}

	// fetch all the files in the given directory
	do
	{
		if(f_readdir(&dir, &fInfo) != FR_OK)
		{
			DEBUG_PRINTF("Failed to read directory item. Aborting...\r\n");
			status = SD_FAIL;
			break;
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
					break;
				}
			}
		}
	} while(fInfo.fname[0] != 0);

	f_rewinddir(&dir); // macro for rewinding the directory to the start for next time

	return status;
}
