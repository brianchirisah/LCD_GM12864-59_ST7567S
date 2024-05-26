/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "LCDFonts.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define transmit(data, length) HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), data, length, Timeout)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


//Global variables
//uint8_t LCD_font[] = {};
uint8_t Page_addr[2] 		= { 0x00, 0xB0 };
uint8_t Col_addr_MSB[2] 	= { 0x00, 0x10 };
uint8_t Col_addr_LSB[2] 	= { 0x00, 0x00 };
uint8_t Blank_line_data[2]  = { 0x40, 0x00 };
uint16_t Data_size = 2;
uint32_t Timeout = 5;
uint8_t page_no;
uint8_t data_prefix = 0x40;
const uint8_t LCD_address 	= 0x3F; // I2C device address for the ST7567S
uint8_t dot_pattern = 0b00000000;
uint8_t page_previous = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ===================================================================================================================================
//  # SINGLE PIXEL DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a horizontal line on screen.
 * @param  Length in pixels (max 132pixels)
 * @param  x-coordinate in pixels (max-132pixels) left end
 * @param  y-coordinate in pixels (max-64pixels) left end
 */

void draw_pixel(double x_coordinate, double y_coordinate){

	double page = floor(y_coordinate/8);
	uint8_t pixel_draw[2] = {0x40, 0x80};
	dot_pattern = 0b00000001 << ((uint8_t) y_coordinate % 8);
	pixel_draw[1]=dot_pattern;
	// Extracting 4 MSBs then bitwise OR with '0b0001 0000'
	Col_addr_MSB[1]= (((uint8_t)x_coordinate >> 4) | 0x10);
	// Extracting 4 LSBs by bitwise ANDing with '0b0000 1111'
	Col_addr_LSB[1] = ((uint8_t)x_coordinate & 0x0F);
	Page_addr[1] = (0xB0 + page);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // // Set Column Least significant Byte Address
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), pixel_draw, 2, Timeout);
	HAL_Delay(1);
}

// ===================================================================================================================================
//  # SINGLE HORIZONTAL LINE DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a horizontal line on screen.
 * @param  Length in pixels (max 132pixels)
 * @param  x-coordinate in pixels (max-132pixels) left end
 * @param  y-coordinate in pixels (max-64pixels) left end
 */

void draw_horizontal_line(double length, double x_coordinate, double y_coordinate){

	double page = floor(y_coordinate/8);
	uint8_t pixel_draw[2] = {0x40, 0x80};
	dot_pattern = 0b00000001 << ((uint8_t) y_coordinate % 8);
	pixel_draw[1]=dot_pattern;
	// Extracting 4 MSBs then bitwise OR with '0b0001 0000'
	Col_addr_MSB[1]= (((uint8_t)x_coordinate >> 4) | 0x10);
	// Extracting 4 LSBs by bitwise ANDing with '0b0000 1111'
	Col_addr_LSB[1] = ((uint8_t)x_coordinate & 0x0F);
	// set starting position
	Page_addr[1] = (0xB0 + page);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // // Set Column Least significant Byte Address
	for (uint8_t x=0;x<length;x++){
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), pixel_draw, 2, Timeout);
	}
	HAL_Delay(1);
}
// ===================================================================================================================================
//  # MULTIPLE HORIZONTAL LINE DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a horizontal line on screen.
 * @param  Length in pixels (max 132pixels)
 * @param  x-coordinate in pixels (max-132pixels) left end
 * @param  y-coordinate in pixels (max-64pixels) left end
 * @param  number of lines (max-64pixels)
 * @param  separation in pixels
 *
 */
void multi_horizontal_line(double length, double x_coordinate, double y_coordinate, uint8_t count, double separation) {

	double page_current = 0;
	uint8_t pixel_draw[2] = { 0x40, 0xFF };
	uint8_t page_prev=0;
	uint8_t draw_count=0;
	// Extracting 4 MSBs then bitwise OR with '0b0001 0000'
	Col_addr_MSB[1]= (((uint8_t)x_coordinate >> 4) | 0x10);
	// Extracting 4 LSBs by bitwise ANDing with '0b0000 1111'
	Col_addr_LSB[1] = ((uint8_t)x_coordinate & 0x0F);
	dot_pattern = 0;

	for (uint8_t line_no = 1; line_no < count+1; line_no++) {
		page_current = floor(y_coordinate / 8);
		if (page_current ==0){
			page_prev=0;
		}else{
			page_prev=floor((y_coordinate-separation)/8);
		}
		if (separation < 8 && page_current== page_prev) {
			Page_addr[1] = (0xB0 + page_current);
			if (dot_pattern==0 || page_current!= page_prev){
				dot_pattern=1;
			}
			if (draw_count==0){
				dot_pattern=0;
			}
			dot_pattern = dot_pattern | 0b00000001 << ((uint8_t) y_coordinate % 8);
			pixel_draw[1] = dot_pattern;
			HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
			HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
			HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // // Set Column Least significant Byte Address
			for (int n = 0; n < length; n++) {
				HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), pixel_draw, 2, Timeout);
				draw_count++;
			}
			HAL_Delay(1);
		}
		else {
			draw_horizontal_line(length, x_coordinate, y_coordinate);
			HAL_Delay(1);
		}
		y_coordinate = y_coordinate + (separation);
		HAL_Delay(1);
	}
	HAL_Delay(1);
}

// ===================================================================================================================================
//  # VERTICAL LINE DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a single vertical line on screen. ************************* * * * * * Might consider option for dashed / dotted lines
 * @param  height in pixels (max 64-pixels)
 * @param  x-coordinate in pixels (max-132pixels) top end
 * @param  y-coordinate in pixels (max-64pixels) top end
 *
 */

/*void draw_vertical_line(double height, double x_coordinate, double y_coordinate){

	uint8_t pixel_draw[2] = {0x40, 0xFF};//pixel_draw_val};
	uint8_t bit_pattern = 0b11111111;
	uint8_t executed_flag = 0b0;
	double max_pages = ceil(height/8) ;
	// Extracting 4 MSBs then bitwise OR with '0b0001 0000'
	Col_addr_MSB[1]= (((uint8_t)x_coordinate) >> 4) | 0x10;
	// Extracting 4 LSBs by bitwise ANDing with '0b0000 1111'
	Col_addr_LSB[1] = ((uint8_t)x_coordinate) & 0x0F;
	Page_addr[1] = (0xB0 + ( floor( y_coordinate / 8 )));

	for (int f= 0; f < max_pages; f++) {
		uint8_t start_page = ( floor( y_coordinate / 8 ));
		uint8_t end_page = ( floor( (y_coordinate+height) / 8 ));

		if(f==0 && start_page == end_page){
			bit_pattern = bit_pattern & 0b11111111 << ((uint8_t)y_coordinate%8);
			executed_flag=1;
			bit_pattern = bit_pattern & 0b11111111 >> (8-((uint8_t)height + (uint8_t)y_coordinate%8));
		}
		else if (((uint8_t)y_coordinate%8)!=0 && executed_flag==0) {
			executed_flag=0;
			bit_pattern = bit_pattern & 0b11111111 << ((uint8_t)y_coordinate%8);

			height = height - (8-((uint8_t)y_coordinate%8));
		}
		else if(height < 8 ){
			bit_pattern = bit_pattern & 0b11111111 >> ((uint8_t)y_coordinate%8);
			height = height - (8-(height + (((uint8_t)y_coordinate%8))));
		}
		else if(height>8) {
			bit_pattern = 0b11111111 ;
			height = height - 8;
			HAL_Delay(1);
		}

		pixel_draw[1] =bit_pattern;
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // Set Column Least significant Byte Address
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), pixel_draw, 2, Timeout);
		Page_addr[1]++;
		start_page++;
	}
}*/

// ===================================================================================================================================
//  # VERTICAL LINE DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a single vertical line on screen. ************************* * * * * * Might consider option for dashed / dotted lines
 * @param  height in pixels (max 64-pixels)
 * @param  x-coordinate in pixels (max-132pixels) top end
 * @param  y-coordinate in pixels (max-64pixels) top end
 */

void draw_vertical_line(double height, double x_coordinate, double y_coordinate){

	uint8_t i=0;
	uint8_t pixel_draw[2] = {0x40, 0xFF};//pixel_draw_val};
	uint8_t bit_pattern = 0b11111111;
	uint8_t bit_pattern_array[(uint8_t)(ceil(height/8))];
	// Extracting 4 MSBs then bitwise OR with '0b0001 0000'
	Col_addr_MSB[1]= (((uint8_t)x_coordinate) >> 4) | 0x10;
	// Extracting 4 LSBs by bitwise ANDing with '0b0000 1111'
	Col_addr_LSB[1] = ((uint8_t)x_coordinate) & 0x0F;

	while (height >= 8){
		Page_addr[1] = (0xB0 + ( floor( y_coordinate / 8 )));
		double_t pixels=0;
		pixels = ((i+1)*8)-(pixels + y_coordinate);
		pixel_draw[1]= bit_pattern & 0b11111111 << (8-(uint8_t)pixels);
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // Set Column Least significant Byte Address
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), pixel_draw, 2, Timeout);
		y_coordinate= y_coordinate + pixels;
		height = height - pixels;
		i++;
		HAL_Delay(1);
	}
	//for (i=i; i>0; i--){
	bit_pattern = bit_pattern & 0b11111111 << ((uint8_t)y_coordinate%8);
	bit_pattern = bit_pattern & 0b11111111 >> (8-((uint8_t)height + (uint8_t)y_coordinate%8));
	pixel_draw[1] =bit_pattern;
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // Set Column Least significant Byte Address
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), pixel_draw, 2, Timeout);
	HAL_Delay(1);

}

// ===================================================================================================================================
//  # CUBOID DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a cuboid on screen.
 * @param  Length in pixels (2-132pixels)
 * @param  Height in pixels (2-132pixels)
 * @param  top left corner x and y coordinate array
 * @param  filling (1 = on , 0 = Off)
 */

void draw_cuboid(uint8_t length, uint8_t height, uint8_t x_coordinate, uint8_t y_coordinate, uint8_t filling){

	// horizontal lines
	multi_horizontal_line(length,x_coordinate,y_coordinate, 2, height-1);
	// Left Side vertical
	draw_vertical_line(height,x_coordinate,y_coordinate);
	// Right side vertical
	draw_vertical_line(height,(x_coordinate+length-1),y_coordinate);

	HAL_Delay(1);
}

// ===================================================================================================================================
//  # CIRCLE DRAWING FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Draws a circle on screen, implements Bresenham's circle drawing algorithm.
 * @param  Length in pixels (2-132pixels)
 * @param  Height in pixels (2-132pixels)
 * @param  top left corner x and y coordinate array
 * @param  filling (1 = on , 0 = Off)
 * */

void Draw_Circle(int16_t x_center, int16_t y_center, int16_t radius, uint16_t fill) {
	int16_t x = 0, y = radius;
	int16_t d = 3 - 2 * radius;
	volatile static uint16_t track=0;
	while (y >= x) {
		draw_pixel(x_center + x, y_center + y);
		draw_pixel(x_center - x, y_center + y);
		draw_pixel(x_center + x, y_center - y);
		draw_pixel(x_center - x, y_center - y);
		draw_pixel(x_center + y, y_center + x);
		draw_pixel(x_center - y, y_center + x);
		draw_pixel(x_center + y, y_center - x);
		draw_pixel(x_center - y, y_center - x);
		x++;
		track = x;
		if (d > 0) {
			y--;
			d = d + 4 * (x - y) + 10;
		} else {
			d = d + 4 * x + 6;
		}
	}
	track++;

}


// ===================================================================================================================================
//  # MULTILINE TEXT WRITER FUNCTION #
// ===================================================================================================================================
/**
 * @brief  Writes 1 or more lines of text to the LCD screen.
 * @param  The row to start the text (range 1 to 8 rows)
 * @param  The text to write to the LCD surrounded by ""
 * @param  Font position in the array (0-indexed)
 * @param  Text inversion (1 = on , 0 = Off)
 */

void multi_line_text(double start_page, const char string_in[], uint8_t font_name, uint8_t inversion) {
	double max_char_per_row;
	double pages_needed;
	const uint8_t LCD_pixels_width = 132;
	const double LCD_pages = 8;
	uint8_t char_width = font_properties[font_name][0];
	uint8_t font_offset= font_properties[font_name][1];
	uint8_t input_pos = 0;
	uint8_t char_hex_array[char_width + 1];
	uint8_t input_char;
	uint8_t pages_avail;
	uint8_t max_input_len;
	uint8_t Send_size;
	uint8_t page_counter=1;
	uint8_t input_len = strlen(string_in);
	uint16_t hex_font_pos;
	// calculate how many characters will fit on a single row (max_char_per_row)
	max_char_per_row = 132 / char_width;
	// calculate how many characters will fit on the screen (max_input_len)
	max_input_len = (9 - start_page) * max_char_per_row;
	//max usable pages calculation
	Send_size = (char_width + 1);
	char_hex_array[0] = data_prefix;
	if (input_len > max_input_len) {

		//Error
		return;
	}
	// if input_len <= max_input_len : execute code, else print error text
	pages_needed = ceil(input_len / max_char_per_row);
	pages_avail = (LCD_pages - (start_page-1));
	if ( pages_avail < pages_needed) {
		// Error
		return;
	}

	for (uint8_t page = start_page; page < (pages_needed+start_page); page++) {
		Page_addr[1] = (0xAF + page);

		transmit(Page_addr, 2);
		transmit(Col_addr_MSB, 2);
		transmit(Col_addr_LSB, 2);
		//HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
		//HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2, Timeout); // Set Column Most significant Byte Address
		//HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2, Timeout); // // Set Column Least significant Byte Address
		if (page >start_page) {
			input_pos=max_char_per_row;
			page_counter++;
			max_char_per_row = max_char_per_row + (LCD_pixels_width/char_width);
		}
		for (uint8_t line_pos = input_pos; line_pos < max_char_per_row; line_pos++) {
			input_char = string_in[line_pos];
			hex_font_pos = (char_width * (input_char - font_offset));
			if (input_len <= 0) return;

			for (uint8_t x = line_pos; x < line_pos + char_width; x++) {
				for (uint8_t char_hex = 0; char_hex < char_width ; char_hex++) { //Sequentially saves the contents of the char_hex_array to the LCD
					if (inversion==1){
						char_hex_array[char_hex + 1] = ~LCDFonts[font_name][hex_font_pos + char_hex];
						continue;
					}
					char_hex_array[char_hex + 1] = LCDFonts[font_name][hex_font_pos + char_hex];
					//HAL_Delay(1);
				}
			}

			transmit(char_hex_array, Send_size);
			//HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1),char_hex_array, Send_size, Timeout);
			//HAL_Delay(25);
			input_len--;
		}
	}
}




// ===================================================================================================================================
//  # This function will set the cursor position to the top left corner (Page 0, Column 0) #
// ===================================================================================================================================

void reset_cursor(void){
	Page_addr[1] = 0xB0;
	Col_addr_MSB[1] = 0x10;
	Col_addr_LSB[1] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2,Timeout); // Set Column Most significant Byte Address
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2,Timeout); // // Set Column Least significant Byte Address
}


// ===================================================================================================================================
//  # This function will clear the screen by writing 0x00 on every line , from left to right,top to bottom #
// ===================================================================================================================================

void clear_screen(void) {

	uint8_t Blank_line_data[2]  = { 0x40, 0x00 };

	for (uint8_t c = 0; c < 8; c++) { // sending to 8 rows (pages) (DDRAM), each row is 1 Byte (8 Bits high)
		Page_addr[1] = (0xB0+c);
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Page_addr, 2, Timeout); // Set Page Address (Row Address)
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, 2,Timeout); // Set Column Most significant Byte Address
		HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, 2,Timeout); // // Set Column Least significant Byte Address
		for (uint8_t d = 0; d < 131; d++) { // sending to 132 columns (DDRAM), 1 byte at a time
			HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Blank_line_data, // Send data prefixed by 0x40 (ST7567S Manual page
					2, Timeout);
			//HAL_Delay(5); // This line can be un_commented to activate and / or adjust delay to slow down the execution
		}
	}
	reset_cursor();
}


// ===================================================================================================================================
//  # This function will turn on the LED if an address is found , otherwise it'll blink it 10 times in 5 seconds #
// ===================================================================================================================================

void I2C_device_address_finder(void){

	uint8_t x;
	uint8_t y;
	HAL_StatusTypeDef result =0;
	for (x = 0x00; x < 0x77; x++) {
		HAL_I2C_IsDeviceReady(&hi2c1, (x << 1), 3, 10);
		if (result == HAL_OK) {
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		}
		else{
			for (y = 0; y < 10; y++){
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				HAL_Delay(500);
			}
		}
	}
}

// ===================================================================================================================================
//  # This function will clear the screen by writing 0x00 on every line , from left to right,top to bottom #
// ===================================================================================================================================
void LCD_Init() {

	uint8_t Restart[2] 				= { 0x00, 0xE2 };
	uint8_t Bias_select[2] 			= { 0x00, 0xA2 };
	uint8_t SEG_direction[2] 		= { 0x00, 0xA0 };
	uint8_t COM_direction[2] 		= { 0x00, 0xC8 };
	uint8_t Regulation_ratio[2]		= { 0x00, 0x25 };
	uint8_t Set_EV_com[2] 			= { 0x00, 0x81 };
	uint8_t Set_EV[2] 				= { 0x00, 0x20 };
	uint8_t Booster[2] 				= { 0x00, 0x2C };
	uint8_t Regulator[2] 			= { 0x00, 0x2E };
	uint8_t Follower[2] 			= { 0x00, 0x2F };
	uint8_t Display_ON[2] 			= { 0x00, 0xAF };

	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Restart, Data_size, Timeout);
	//HAL_Delay(5); // If LCD is not working, try un_commenting this line with the 5ms delay
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Bias_select, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), SEG_direction, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), COM_direction, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Regulation_ratio, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Set_EV_com, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Set_EV, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Booster, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Regulator, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Follower, Data_size, Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_MSB, Data_size,Timeout);
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Col_addr_LSB, Data_size,Timeout);
	HAL_Delay(1);
	clear_screen();
	HAL_I2C_Master_Transmit(&hi2c1, (LCD_address << 1), Display_ON, Data_size, Timeout);

}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();


	/* USER CODE BEGIN 2 */


	//************************************************************************************************************
	//************************************************************************************************************
	//
	//************************************************************************************************************
	//************************************************************************************************************
	LCD_Init();
	clear_screen();
	multi_line_text(1,"1",0,0);
	multi_line_text(2,"2",0,0);
	multi_line_text(3,"3",0,0);
	multi_line_text(4,"4",0,0);
	multi_line_text(5,"5",0,0);
	multi_line_text(6,"6",0,0);
	multi_line_text(7,"7",0,0);
	multi_line_text(8,"8",0,0);
	//multi_line_text(4,"abc@WXYZ123&$#",0,1);
	//multi_line_text(6,"abc@WXYZ123&$#",1,0);
	//multi_line_text(8,"abc@WXYZ123&$#",1,1);
	//draw_cuboid(24,24,5,43,1);
	//draw_horizontal_line(32,16,16);
	//draw_vertical_line(35,7,2);
	//multi_horizontal_line(131, 0, 0, 32, 2);
	Draw_Circle(63,31,30,1);
	//draw_cuboid(29, 6,20, 8, 11);


	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
