/*
 * LCD_3DIGIT.h
 *
 *  Created on: 31 juil. 2026
 *      Author: valentin
 */

#ifndef LCD_3DIGIT_INC_LCD_3DIGIT_H_
#define LCD_3DIGIT_INC_LCD_3DIGIT_H_

#include <stdint.h>
#include "main.h"
#include "7segment_font.h"

extern LCD_HandleTypeDef hlcd;

void LCD_WriteDigit(const char* str);

#endif /* LCD_3DIGIT_INC_LCD_3DIGIT_H_ */
