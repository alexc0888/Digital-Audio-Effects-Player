#include "oled_driver.h"
#include "font8x8_basic.h"

typedef struct {
	char track_name[15];
	int playing;
	float seek_percent;
} track_t;

track_t track_list_type[7];

char track_list[MAX_FILE_NUM][64 + 1]; // 64 + 1 corresponds to MAX_FILE_NAME_LEN constant in main.c
uint32_t image_buffer_track_playing[128*128];


void setTrackList(char fileList[MAX_FILE_NUM][64 + 1], uint8_t numFiles)
{
	// Clear the old track list
	for(uint8_t file = 0; file < MAX_FILE_NUM; file++)
	{
		strcpy(track_list[file], " ");
	}
	// Assign the new track list from the SD Card read
	for(uint8_t file = 0; file < numFiles; file++)
	{
		strcpy(track_list[file], fileList[file]);
	}
}

void OLED_1in5_rgb_Init(void) {
	//Hardware reset
	OLED_Reset();

	//Set the initialization register
	OLED_InitReg();
	HAL_Delay(200);

	//Turn on the OLED display
	OLED_WriteReg(0xAF);
}

void OLED_1in5_rgb_Clear(uint8_t r, uint8_t g, uint8_t b) {
	int i;

	OLED_WriteReg(0x15);
	OLED_WriteData(0);
	OLED_WriteData(127);
	OLED_WriteReg(0x75);
	OLED_WriteData(0);
	OLED_WriteData(127);
	// fill!
	OLED_WriteReg(0x5C);

	uint8_t buffer[128 * 128 * 3];
	for (i = 0; i < OLED_1in5_RGB_WIDTH * OLED_1in5_RGB_HEIGHT * 3; i += 3) {
//		if (i % 2 == 0) {
//			OLED_WriteData(0xFF);
//		} else {
//			OLED_WriteData(0x00);
//		}
//		OLED_WriteData(0xFF);
//		OLED_WriteData(0xFF);
//		OLED_WriteData(0xFF);
		buffer[i] = r;
		buffer[i + 1] = g;
		buffer[i + 2] = b;
	}
	OLED_StartWrite();
	OLED_WriteSeq(buffer, 128 * 128 * 3);
	OLED_EndWrite();
}

void OLED_1in5_rgb_Display(uint32_t *Image) {
	uint32_t i, j;
	uint8_t r, g, b;

	OLED_WriteReg(0x15);
	OLED_WriteData(0);
	OLED_WriteData(127);
	OLED_WriteReg(0x75);
	OLED_WriteData(0);
	OLED_WriteData(127);

	// fill!
	OLED_WriteReg(0x5C);

	uint8_t buffer[128*128*3];
	int buffer_index = 0;
	for (i = 0; i < OLED_1in5_RGB_HEIGHT; i++) {
		for (j = 0; j < OLED_1in5_RGB_WIDTH; j++) {
			r = (Image[j + i * OLED_1in5_RGB_HEIGHT] >> 16);
			g = (Image[j + i * OLED_1in5_RGB_HEIGHT] >> 8) & 0xFF;
			b = Image[j + i * OLED_1in5_RGB_HEIGHT] & 0xFF;
			r = r * 63 / 255;
			g = g * 63 / 255;
			b = b * 63 / 255;

//			OLED_WriteData(r);
//			OLED_WriteData(g);
//			OLED_WriteData(b);
			buffer[buffer_index++] = r;
			buffer[buffer_index++] = g;
			buffer[buffer_index++] = b;
		}
	}
	OLED_StartWrite();
	OLED_WriteSeq(buffer, 128 * 128 * 3);
	OLED_EndWrite();
}

void draw_char(uint32_t *image, uint32_t startX, uint32_t startY, char c, uint32_t color) {
	wchar_t unicode_char = (wchar_t) c;

	char *bitmap = font8x8_basic[unicode_char];

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (((bitmap[i] >> j) & 0x1)) {
				image[startX + (startY + i) * 128 + j] = color;
			}
		}
	}
}

void draw_text(uint32_t *image, uint32_t startX, uint32_t startY, char *text, uint32_t color) {
	uint8_t y_inc = 0;
	uint8_t x_inc = 0;
	for (int i = 0; text[i] != '\0'; i++) {
		if (text[i] == ' ') {
			x_inc++;
			continue;
		}
		if (text[i] == '\n') {
			y_inc++;
			x_inc = 0;
			continue;
		}
		if (startX + x_inc * 8 >= 120) {
			break;
		}
		draw_char(image, startX + (x_inc++) * 8, startY + y_inc * 8, text[i], color);
	}
}


void render_track_list(int current_selection) {
	current_selection %= 7;

	uint32_t image_buffer[128*128];
	for (int i = 0; i < 128*128; i++)
		image_buffer[i] = 0x00;

	draw_text(image_buffer, 4, 4, "Track selection", 0xFFFFFFFF);
	for (int i = 0; i < 128; i++) {
		image_buffer[i + 15 * 128] = 0xFFFFFFFF;
	}
	for (int i = 1; i < 8; i++) {
		uint32_t startX = 4;
		uint32_t startY = i * (128/8) + 4;

		if (current_selection + 1 == i) {
			for (int r = i * 128/8; r < (i+1) * 128/8; r++) {
				for (int c = 0; c < 128; c++) {
					image_buffer[c + r * 128] = 0xFFFFFFFF;
				}
			}
			draw_text(image_buffer, startX, startY, track_list[i - 1], 0x00000000);
		} else {
			draw_text(image_buffer, startX, startY, track_list[i - 1], 0xFFFFFFFF);
		}
	}

	OLED_1in5_rgb_Display(image_buffer);
}

void render_track_playing(int current_selection, int current_duration, int total_duration) {
	current_selection %= 7;
	float seek_percent = (float) current_duration / (float) total_duration;

//	uint32_t image_buffer[128*128];
	for (int i = 0; i < 128*128; i++)
		image_buffer_track_playing[i] = 0x00;

	draw_text(image_buffer_track_playing, 12, 4, "Playing track", 0xFFFFFFFF);


	int max_row_len = 12;
	int start_row = 40;
	char track_name_out[max_row_len + 1];
	int count = 0;
	int row_count = 0;
	for (int i = 0; track_list[current_selection][i] != '\0'; i++) {
		char c = track_list[current_selection][i];
		track_name_out[count++] = c;
		if (count == max_row_len) {
			track_name_out[max_row_len] = '\0';
			draw_text(image_buffer_track_playing, (128 - (count * 8))/2, start_row + row_count * 10, track_name_out, 0xFFFFFFFF);
			count = 0;
			row_count++;
		}
	}
	track_name_out[count] = '\0';
	draw_text(image_buffer_track_playing, (128 - (count * 8))/2, start_row + row_count * 10, track_name_out, 0xFFFFFFFF);

	int seek_start_pos = 20;
	int seek_row = 100;
	int seek_end_pos = 128 - seek_start_pos;
	int seek_pos = seek_start_pos + (128 - 2 * seek_start_pos) * seek_percent;
	for (int i = seek_start_pos; i < seek_pos; i++) {
		image_buffer_track_playing[i + seek_row * 128] = 0x00FF0000;
	}
	for (int i = seek_pos; i < seek_end_pos; i++) {
		image_buffer_track_playing[i + seek_row * 128] = 0xFFFFFFFF;
	}

	char start_time[20];
	char end_time[20];

	sprintf(start_time, "%d:%02d", current_duration/60, current_duration%60);
	sprintf(end_time, "%d:%02d", total_duration/60, total_duration%60);
	draw_text(image_buffer_track_playing, seek_start_pos - 8, seek_row - 10, start_time, 0xFFFFFFFF);
	draw_text(image_buffer_track_playing, seek_end_pos - 24, seek_row - 10, end_time, 0xFFFFFFFF);
	OLED_1in5_rgb_Display(image_buffer_track_playing);
}

void render_track_progress(int current_selection, int current_duration, int total_duration)
{
		current_selection %= 7;
		float seek_percent = (float) current_duration / (float) total_duration;

//		uint32_t image_buffer[128*128];

		int seek_start_pos = 20;
		int seek_row = 100;
		int seek_end_pos = 128 - seek_start_pos;
		int seek_pos = seek_start_pos + (128 - 2 * seek_start_pos) * seek_percent;
		for (int i = seek_start_pos; i < seek_pos; i++) {
			image_buffer_track_playing[i + seek_row * 128] = 0x00FF0000;
		}
		for (int i = seek_pos; i < seek_end_pos; i++) {
			image_buffer_track_playing[i + seek_row * 128] = 0xFFFFFFFF;
		}

		char start_time[20];
		char end_time[20];

		sprintf(start_time, "%d:%02d", current_duration/60, current_duration%60);
		sprintf(end_time, "%d:%02d", total_duration/60, total_duration%60);
		draw_text(image_buffer_track_playing, seek_start_pos - 8, seek_row - 10, start_time, 0xFFFFFFFF);
		draw_text(image_buffer_track_playing, seek_end_pos - 24, seek_row - 10, end_time, 0xFFFFFFFF);
		OLED_1in5_rgb_Display(image_buffer_track_playing);
}

void OLED_InitReg(void) {
	OLED_WriteReg(0xfd);  // command lock
	OLED_WriteData(0x12);
	OLED_WriteReg(0xfd);  // command lock
	OLED_WriteData(0xB1);

	OLED_WriteReg(0xae);  // display off
	OLED_WriteReg(0xa4);  // Normal Display mode

	OLED_WriteReg(0x15);  //set column address
	OLED_WriteData(0x00);     //column address start 00
	OLED_WriteData(0x7f);     //column address end 127
	OLED_WriteReg(0x75);  //set row address
	OLED_WriteData(0x00);     //row address start 00
	OLED_WriteData(0x7f);     //row address end 127

	OLED_WriteReg(0xB3);
	OLED_WriteData(0xF1);

	OLED_WriteReg(0xCA);
	OLED_WriteData(0x7F);

	OLED_WriteReg(0xa0);  //set re-map & data format
	OLED_WriteData(0xB4);     //Horizontal address increment

	OLED_WriteReg(0xa1);  //set display start line
	OLED_WriteData(0x00);     //start 00 line

	OLED_WriteReg(0xa2);  //set display offset
	OLED_WriteData(0x00);

	OLED_WriteReg(0xAB);
	OLED_WriteReg(0x01);

	OLED_WriteReg(0xB4);
	OLED_WriteData(0xA0);
	OLED_WriteData(0xB5);
	OLED_WriteData(0x55);

	OLED_WriteReg(0xC1);
	OLED_WriteData(0xC8);
	OLED_WriteData(0x80);
	OLED_WriteData(0xC0);

	OLED_WriteReg(0xC7);
	OLED_WriteData(0x0F);

	OLED_WriteReg(0xB1);
	OLED_WriteData(0x32);

	OLED_WriteReg(0xB2);
	OLED_WriteData(0xA4);
	OLED_WriteData(0x00);
	OLED_WriteData(0x00);

	OLED_WriteReg(0xBB);
	OLED_WriteData(0x17);

	OLED_WriteReg(0xB6);
	OLED_WriteData(0x01);

	OLED_WriteReg(0xBE);
	OLED_WriteData(0x05);

	OLED_WriteReg(0xA6);
}
