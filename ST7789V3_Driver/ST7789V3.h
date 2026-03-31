/**
 *
 * @file         ST7789V3.h
 * @brief        Header for ST7789V3.c file.
 *               This file contains the common defines * of the ST7789V3 driver.
 * @author Anthony / SleepPandas
 */

#ifndef __ST7789V3_H
#define __ST7789V3_H

#include <fonts/fonts.h>
#include <stdint.h>


// --------- Register Maps -----------
#define ST7789V3_rst
#define ST7789V3_backlight

// SLPOUT
#define Sleep_Out 0x11U
#define Sleep_In 0x10U

// --------- RGB888 Color Definitions (24-bit) -----------
// Format 24 bit: 0xRRGGBB to be Converted

#define BLACK 0x000000U
#define WHITE 0xFFFFFFU
#define RED 0xFF0000U
#define GREEN 0x00FF00U
#define BLUE 0x0000FFU
#define YELLOW 0xFFFF00U
#define CYAN 0x00FFFFU
#define MAGENTA 0xFF00FFU
#define ORANGE 0xFF6A00U
#define PINK 0xFF69B4U
#define PURPLE 0x800080U
#define LIME 0x32FF00U
#define NAVY 0x000080U
#define DARK_GREEN 0x006400U
#define MAROON 0x800000U
#define OLIVE 0x808000U
#define TEAL 0x008080U
#define SILVER 0xC0C0C0U
#define GRAY 0x808080U
#define DARK_GRAY 0x404040U
#define LIGHT_GRAY 0xD3D3D3U
#define GOLD 0xFFD700U
#define SKY_BLUE 0x87CEEBU

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

typedef enum {
  ST7789_STATE_READY = 0,
  ST7789_STATE_BUSY = 1,
  ST7789_STATE_ERROR = 2,
} ST7789V3_State;

typedef struct ST7789V3_Config ST7789V3_Config;

// These callbacks let the app react when an async transfer finishes.
typedef void (*ST7789V3_Callback)(ST7789V3_Config *config, void *user_data);

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

struct ST7789V3_Config {
  // --- function pointers ---
  int8_t (*spi_write)(uint16_t len, const uint8_t *pData);
  int8_t (*spi_write_dma)(uint16_t len, const uint8_t *pData);
  void (*delay_ms)(uint32_t milliseconds);

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

  // Display Inversion State
  Inversion_Mode Inversion_Mode;

  // DMA State (can be changed by External Factors)
  volatile ST7789V3_State State;

  // Active transfer info used by the DMA/async path.
  const uint8_t *active_buffer;
  uint16_t active_length;
  int8_t last_error;

  // User callbacks for transfer complete and transfer error.
  ST7789V3_Callback tx_complete_callback;
  ST7789V3_Callback tx_error_callback;
  void *callback_user_data;
};

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
 * @brief Draws a colored pixel at x,y position dates in 24bit Hex Color
 * 0xRRGGBB
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

/**
 * @brief Draws a string on the display
 * @param config, x, y, str, hexcolor, font
 * @note Supports \n and \r for new lines and carriage returns
 * @note Supports line wrap if the next character exceeds LCD width
 */

void DrawString(ST7789V3_Config *config, uint16_t x, uint16_t y,
                const char *str, uint32_t hexcolor, const FontDef *font);

/**
 * @brief Draws a line between two points using Bresenham's line algorithm
 */

void DrawLine(ST7789V3_Config *config, uint16_t x0, uint16_t y0, uint16_t x1,
              uint16_t y1, uint32_t hexcolor);

/**
 * @brief Draws an outlined rectangle (no fill)
 */
void DrawRectangle(ST7789V3_Config *config, uint16_t x, uint16_t y,
                   uint16_t width, uint16_t height, uint32_t hexcolor);

/**
 * @brief Draws a filled rectangle
 */
void DrawFilledRectangle(ST7789V3_Config *config, uint16_t x, uint16_t y,
                         uint16_t width, uint16_t height, uint32_t hexcolor);

/**
 * @brief Draws a circle outline using the midpoint circle algorithm
 */
void DrawCircle(ST7789V3_Config *config, uint16_t x_center, uint16_t y_center,
                uint16_t radius, uint32_t hexcolor);

/**
 * @brief Draws a filled circle using the midpoint circle algorithm
 */
void DrawFilledCircle(ST7789V3_Config *config, uint16_t x_center,
                      uint16_t y_center, uint16_t radius, uint32_t hexcolor);

#endif /* __ST7789V3_H */
