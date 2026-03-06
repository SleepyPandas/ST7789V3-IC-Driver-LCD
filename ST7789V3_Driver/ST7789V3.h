/**
 *
 * @file         ST7789V3.h
 * @brief        Header for ST7789V3.c file.
 *               This file contains the common defines of the ST7789V3 driver.
 *
 */

#ifndef __ST7789V3_H
#define __ST7789V3_H

#include <stdint.h>

// --------- Register Maps -----------
#define ST7789V3_rst
#define ST7789V3_backlight

// SLPOUT
#define Sleep_Out 0x11U
#define Sleep_In 0x10U

typedef enum {
  High = 1,
  Low = 0,
} GPIO_Pinstate;

typedef enum {
  DATA = 1,
  CMD = 0,
} Trans_State;

// DC LOW Command DC High Data

typedef struct {
  // --- function pointers ---
  int8_t (*spi_write)(uint16_t len, const uint8_t *pData);
  int8_t (*delay_ms)(uint32_t milliseconds);

  // FOR DMA INTEGRATION TODO: LATER
  // int8_t (*wait_for_tx_complete_DMA)();

  int8_t (*set_cs)(GPIO_Pinstate state);
  int8_t (*set_dc)(Trans_State state);
  int8_t (*set_rst)(GPIO_Pinstate state);
  int8_t (*set_backlight)(GPIO_Pinstate state);

  // --- LCD attributes ---
  uint16_t LCD_Width;
  uint16_t LCD_Height;

  // Offset memory start point based on LCD size
  uint8_t Col_Offset;
  uint8_t Row_Offset;

} ST7789V3_Config;

void ST7789V3_init(ST7789V3_Config *config);

int8_t WriteCmd(ST7789V3_Config *config);

int8_t WriteData(ST7789V3_Config *config);

int8_t SetWindow(ST7789V3_Config *config);

int8_t DrawPixel(ST7789V3_Config *config);

int8_t DrawChar(ST7789V3_Config *config);

#endif /* __ST7789V3_H */
