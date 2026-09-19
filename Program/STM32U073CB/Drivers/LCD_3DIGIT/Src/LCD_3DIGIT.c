/*
 * LCD_3DIGIT.c
 *
 *  Created on: 31 juil. 2026
 *      Author: valentin
 */

#include "LCD_3DIGIT.h"


static uint16_t font_index(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0' + 1);
    else if (c >= 'A' && c <= 'Z')
        return (c - 'A' + 11);
    else if (c >= 'a' && c <= 'z')
        return (c - 'a'+ 11);
    else if (c == '*')
            return 37;
    else
        return 0; // space / unknown
}

void LCD_WriteDigit(const char* str)
{
	uint32_t RAM_reg0 = 0;
	uint32_t RAM_reg1 = 0;
	uint32_t RAM_reg2 = 0;
	uint32_t RAM_reg3 = 0;

	uint8_t car1 = 0;
	uint8_t car2 = 0;
	uint8_t car3 = 0;

	HAL_LCD_Clear(&hlcd);

	car1 = font7seg[font_index(*str)];
	str++;
	car2 = font7seg[font_index(*str)];
	str++;
	car3 = font7seg[font_index(*str)];
	str++;

	RAM_reg0 = (0 << 5) |
			   (((car3 >> 4) & 1) << 4) |
			   (0 << 3) |
			   (((car2 >> 4) & 1) << 2) |
			   (0 << 1) |
			   (((car1 >> 4) & 1) << 0);

	HAL_LCD_Write(&hlcd, 0, 0xFFFFFFFF, RAM_reg0);

	RAM_reg1 = (((car3 >> 5) & 1) << 5) |
			   (((car3 >> 3) & 1) << 4) |
			   (((car2 >> 5) & 1) << 3) |
			   (((car2 >> 3) & 1) << 2) |
			   (((car1 >> 5) & 1) << 1) |
			   (((car1 >> 3) & 1) << 0);

	HAL_LCD_Write(&hlcd, 2, 0xFFFFFFFF, RAM_reg1);

	RAM_reg2 = (((car3 >> 6) & 1) << 5) |
			   (((car3 >> 1) & 1) << 4) |
			   (((car2 >> 6) & 1) << 3) |
			   (((car2 >> 1) & 1) << 2) |
			   (((car1 >> 6) & 1) << 1) |
			   (((car1 >> 1) & 1) << 0);

	HAL_LCD_Write(&hlcd, 4, 0xFFFFFFFF, RAM_reg2);

	RAM_reg3 = (((car3 >> 7) & 1) << 5) |
			   (((car3 >> 2) & 1) << 4) |
			   (((car2 >> 7) & 1) << 3) |
			   (((car2 >> 2) & 1) << 2) |
			   (((car1 >> 7) & 1) << 1) |
			   (((car1 >> 2) & 1) << 0);

	HAL_LCD_Write(&hlcd, 6, 0xFFFFFFFF, RAM_reg3);

	HAL_LCD_UpdateDisplayRequest(&hlcd);

}
