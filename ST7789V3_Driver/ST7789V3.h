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
#include <sys/_intsup.h>
#include <fonts/fonts.h>

// --------- Register Maps -----------
#define ST7789V3_rst
#define ST7789V3_backlight

// SLPOUT
#define Sleep_Out 0x11U
#define Sleep_In 0x10U

// --------- RGB888 Color Definitions (24-bit) -----------
// Format 24 bit: 0xRRGGBB to be Converted

#define COLOR_BLACK 0x000000U
#define COLOR_WHITE 0xFFFFFFU
#define COLOR_RED 0xFF0000U
#define COLOR_GREEN 0x00FF00U
#define COLOR_BLUE 0x0000FFU
#define COLOR_YELLOW 0xFFFF00U
#define COLOR_CYAN 0x00FFFFU
#define COLOR_MAGENTA 0xFF00FFU
#define COLOR_ORANGE 0xFF6A00U
#define COLOR_PINK 0xFF69B4U
#define COLOR_PURPLE 0x800080U
#define COLOR_LIME 0x32FF00U
#define COLOR_NAVY 0x000080U
#define COLOR_DARK_GREEN 0x006400U
#define COLOR_MAROON 0x800000U
#define COLOR_OLIVE 0x808000U
#define COLOR_TEAL 0x008080U
#define COLOR_SILVER 0xC0C0C0U
#define COLOR_GRAY 0x808080U
#define COLOR_DARK_GRAY 0x404040U
#define COLOR_LIGHT_GRAY 0xD3D3D3U
#define COLOR_GOLD 0xFFD700U
#define COLOR_SKY_BLUE 0x87CEEBU

typedef enum {
  HIGH = 1,
  LOW = 0,
} GPIO_Pinstate;

// Send Command or Data
typedef enum {
  DATA = 1,
  CMD = 0,
} Trans_State;

// Color Modes 12 bit 14, 16 etc
typedef enum {
  bit_12 = 0x03U,
  bit_16 = 0x05U,
  bit_18 = 0x06U,
} Color_Mode;

typedef enum {
  INVON = 1,
  INVOFF = 0,
} Inversion_Mode;

typedef enum {
  Asleep = 0,
  Awake = 1,
} Sleep_State;

typedef enum {
  Portrait = 0,
  Landscape = 1,
  Portrait_Inverted = 2,
  Landscape_Inverted = 3,
} Orientation;

#define Display_On_Register 0x29U
#define Display_Off_Register 0x28U

// Column Address Set
#define CASET 0x2AU
#define RASET 0x2BU
#define RAMWR 0x2CU
#define COLMODE 0x3AU
#define MADCTL 0x36U

// For inverting Displays
#define INVOFF_REG 0x20U
#define INVON_REG 0x21U

// DC LOW Command DC High Data

typedef struct {
  // --- function pointers ---
  int8_t (*spi_write)(uint16_t len, const uint8_t *pData);
  void (*delay_ms)(uint32_t milliseconds);

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

  // Color Mode Bit depth
  Color_Mode Bit_Depth;

  //Display Inversion State
  Inversion_Mode Inversion_Mode;

} ST7789V3_Config;

void ST7789V3_init(ST7789V3_Config *config);

// Set the "bounding box" of the LCD screen

int8_t SetWindow(ST7789V3_Config *config, uint16_t X_Start, uint16_t X_End,
                 uint16_t Y_Start, uint16_t Y_End);


int8_t SetColorMode(ST7789V3_Config *config, Color_Mode bitdepth);

void DISPLAYON(ST7789V3_Config *config);

void DISPLAYOFF(ST7789V3_Config *config);

void HardReset(ST7789V3_Config *config);

void InvertDisplay(ST7789V3_Config *config, Inversion_Mode Inversion);

void SleepMode(ST7789V3_Config *config, Sleep_State sleepstate);

// =============== Graphical Functions ================
/** 
* @brief Draws a colored pixel at x,y position dates in 24bit Hex Color 0xRRGGBB
* @param config, x, y, hexcolor 
*/

int8_t DrawPixel(ST7789V3_Config *config, uint16_t x, uint16_t y,
                 uint32_t hexcolor);


void DrawChar(ST7789V3_Config *config, uint16_t x, uint16_t y, char user_char,
              uint32_t hexcolor, const FontDef *font);

void FillScreen(ST7789V3_Config *config, uint32_t hexcolor);


/** 
* @brief Sets the rotation of the display
* @param config, orientation 
*/

void SetRotation(ST7789V3_Config *config, Orientation orientation);
// To be inplemented but will USE DMA  
/** TODO: implement */
void WriteDataBuffer(ST7789V3_Config *config);
/** TODO: implement */
void LCD_DrawImageDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                      const uint16_t *image_data);

                  

#endif /* __ST7789V3_H */
