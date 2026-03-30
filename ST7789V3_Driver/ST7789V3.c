

/**
 *
 * @file         ST7789V3.c
 * @brief
 *               This file will contain the main logic for using the ST7789V3
 * chips
 * @author Anthony / SleepPandas
 */

#include "ST7789V3.h"
#include <stdint.h>
#include <sys/types.h>

// ------------ PRIVATE HELPER FUNCTIONS ------------------

static void WriteCommand(ST7789V3_Config *config, uint8_t cmd) {
  config->set_dc(CMD);
  config->set_cs(LOW);
  config->spi_write(1, &cmd);
  config->set_cs(HIGH);
}

static void WriteData(ST7789V3_Config *config, uint8_t data) {
  config->set_dc(DATA);
  config->set_cs(LOW);
  config->spi_write(1, &data);
  config->set_cs(HIGH);
}

void ST7789V3_init(ST7789V3_Config *config) {
  //   1. Hardware RST pulse GPIO only, no SPI
  //   2. Delay(~120ms after RST goes HIGH)
  //   3. Now start SPI communication
  //   4. SLPOUT(0x11) + delay
  //    other stuff.... for initilization Perhaps Based on users config?

  //

  config->set_rst(HIGH);
  config->delay_ms(10);
  config->set_rst(LOW);
  config->delay_ms(10);
  config->set_rst(HIGH);
  config->delay_ms(120);

  // SPI communication to WAKE
  WriteCommand(config, Sleep_Out);
  config->delay_ms(120);

  // By default set the Color mode to 16 bit
  SetColorMode(config, bit_16);

  // Memory Access Control

  // configure offsets

  // Divide by 2 because Pixels are split equally between left and right sides
  // of the lcd
  config->Col_Offset = (240 - config->LCD_Width) / 2;
  config->Row_Offset = (320 - config->LCD_Height) / 2;

  // Rotation, color Order default RGB

  // Display Inversion
  InvertDisplay(config, INVON);

  config->Inversion_Mode = INVON;

  // Display On and Show Buffer items

  DISPLAYON(config);

  // Set backlight on
  // config->set_backlight(HIGH);
}

int8_t SetColorMode(ST7789V3_Config *config, Color_Mode bitdepth) {
  WriteCommand(config, COLMODE);
  WriteData(config, bitdepth);
  config->Bit_Depth = bitdepth;

  return 0;
}

void DISPLAYON(ST7789V3_Config *config) {
  WriteCommand(config, Display_On_Register);
}

void DISPLAYOFF(ST7789V3_Config *config) {
  WriteCommand(config, Display_Off_Register);
}

/**
 * @brief Hard reset all settings to default
 */
void HardReset(ST7789V3_Config *config) {
  config->set_rst(HIGH);
  config->delay_ms(10);
  config->set_rst(LOW);
  config->delay_ms(10);
  config->set_rst(HIGH);
  config->delay_ms(120);
}

void InvertDisplay(ST7789V3_Config *config, Inversion_Mode Inversion) {
  // config->set_dc(CMD);
  // config->set_cs(LOW);

  if (Inversion == INVON) {
    WriteCommand(config, INVON_REG);
  } else {
    WriteCommand(config, INVOFF_REG);
  }

  config->Inversion_Mode = Inversion;

  // config->set_cs(HIGH);
}

// Needs to know where the bounds of the lcd are
int8_t SetWindow(ST7789V3_Config *config, uint16_t X_Start, uint16_t X_End,
                 uint16_t Y_Start, uint16_t Y_End) {

  uint16_t X_StartOffset = X_Start + config->Col_Offset;
  uint16_t X_EndOffset = X_End + config->Col_Offset;
  uint16_t Y_StartOffset = Y_Start + config->Row_Offset;
  uint16_t Y_EndOffset = Y_End + config->Row_Offset;

  uint8_t High_Temp = (X_StartOffset >> 8);
  uint8_t Low_Temp = (X_StartOffset & 0xFFU);
  // Horizontal X , write the start write the end of X
  WriteCommand(config, CASET);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);
  High_Temp = (X_EndOffset >> 8);
  Low_Temp = (X_EndOffset & 0xFFU);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);

  WriteCommand(config, RASET);

  High_Temp = (Y_StartOffset >> 8);
  Low_Temp = (Y_StartOffset & 0xFFU);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);

  High_Temp = (Y_EndOffset >> 8);
  Low_Temp = (Y_EndOffset & 0xFFU);

  WriteData(config, High_Temp);
  WriteData(config, Low_Temp);

  WriteCommand(config, RAMWR);
  // Error code for wrong bounds? or exceeded bounds?
  return 0;
}

void SleepMode(ST7789V3_Config *config, Sleep_State sleepstate) {
  if (sleepstate == 0) {
    WriteCommand(config, Sleep_Out);
  } else {
    WriteCommand(config, Sleep_In);
  }
}

// =============== Graphical Functions ================

/**
 * @brief Helper to draw a horizontal line efficiently using
 * More efficient than calling DrawPixel in a loop or Drawline
 */

static uint16_t ConvertRGB888toRGB565(uint32_t hexcolor) {
  uint8_t Red5 = (hexcolor >> 19) & 0x1FU;
  uint8_t Green6 = (hexcolor >> 10) & 0x3FU;
  uint8_t Blue5 = (hexcolor >> 3) & 0x1FU;
  uint16_t color565 = (Red5 << 11) | (Green6 << 5) | Blue5;
  return color565;
}

/**
 * @brief Fill the entire LCD with a single RGB565 color. e.g
 * take orange convert it to 5 bit 6 bit 5 bit
 TODO: Make switch case for 12 bit 16, 18 bit
 */

void FillScreen(ST7789V3_Config *config, uint32_t hexcolor) {
  // Cover the whole display [0, Width-1] x [0, Height-1]
  SetWindow(config, 0, config->LCD_Width - 1, 0, config->LCD_Height - 1);

  uint32_t total_pixels = (config->LCD_Width) * (config->LCD_Height);
  // take a default Hex color say 24 bit and extract it #FF FF FF white
  uint16_t color565 = ConvertRGB888toRGB565(hexcolor);

  config->set_dc(DATA);
  config->set_cs(LOW);
  // Write Data

  uint8_t color565_High = (color565 >> 8) & 0xFFU;
  uint8_t color565_Low = color565 & 0xFFU;

  for (uint32_t i = 0; i < total_pixels; i++) {
    config->spi_write(1, &color565_High);
    config->spi_write(1, &color565_Low);
  }

  config->set_cs(HIGH);
}

int8_t DrawPixel(ST7789V3_Config *config, uint16_t x, uint16_t y,
                 uint32_t hexcolor) {
  // Bounds check — don't draw outside the display
  if (x >= config->LCD_Width || y >= config->LCD_Height) {
    return -1;
  }

  // 1. Set the window to exactly this 1×1 pixel (SetWindow ends with RAMWR)
  SetWindow(config, x, x, y, y);

  uint16_t color565 = ConvertRGB888toRGB565(hexcolor);

  uint8_t color565_High = (color565 >> 8) & 0xFFU;
  uint8_t color565_Low = color565 & 0xFFU;

  config->set_dc(DATA);
  config->set_cs(LOW);
  config->spi_write(1, &color565_High);
  config->spi_write(1, &color565_Low);
  config->set_cs(HIGH);

  return 0;
}

void DrawChar(ST7789V3_Config *config, uint16_t x, uint16_t y, char user_char,
              uint32_t hexcolor, const FontDef *font) {
  // Check if character is within valid printable ASCII range (32 to 126)
  if (user_char < ' ' || user_char > '~') {
    return;
  }

  // Calculate starting index of character in the font data array
  uint32_t char_offset = (user_char - ' ') * font->height * font->bytes_per_row;

  // Iterate over each pixel row of the character
  for (uint8_t row = 0; row < font->height; row++) {
    // Iterate over each pixel column
    for (uint8_t col = 0; col < font->width; col++) {
      // Group bits by the byte they reside in (8 bits per byte)
      uint8_t byte_index = col / 8;
      // The bit position within that specific byte
      uint8_t bit_index = col % 8;

      // Extract the row data for this specific row and byte column
      uint8_t line_data =
          font->data[char_offset + (row * font->bytes_per_row) + byte_index];

      // Font arrays store pixels horizontally MSB -> LSB
      if (line_data & (1 << (7 - bit_index))) {
        // Pixel is foreground, draw it
        DrawPixel(config, x + col, y + row, hexcolor);
      }
    }
  }
}

void SetRotation(ST7789V3_Config *config, Orientation orientation) {
  uint8_t madctl_value = 0;

  // The ST7789V3 supports 4 standard rotations: 0, 90, 180, 270 degrees but we
  // use a typedef enum because I am not going to remember the values
  switch (orientation) {
  case Portrait: // 0 Degrees (Portrait)
    madctl_value = 0x00;
    config->LCD_Width = 172;
    config->LCD_Height = 320;
    break;
  case Landscape:        // 90 Degrees (Landscape)
    madctl_value = 0x60; // MV = 1, MX = 1
    config->LCD_Width = 320;
    config->LCD_Height = 172;
    break;
  case Portrait_Inverted: // 180 Degrees (Portrait Inverted)
    madctl_value = 0xC0;  // MY = 1, MX = 1
    config->LCD_Width = 172;
    config->LCD_Height = 320;
    break;
  case Landscape_Inverted: // 270 Degrees (Landscape Inverted)
    madctl_value = 0xA0;   // MY = 1, MV = 1
    config->LCD_Width = 320;
    config->LCD_Height = 172;
    break;
  }

  // Update offsets based on the MV bit (bit 5).
  // When MV=1, X and Y axes are swapped in hardware.
  if (madctl_value & 0x20) {
    // Landscape Mode (Hardware axes swapped, Physical Max: 320x240)
    config->Col_Offset = (320 - config->LCD_Width) / 2;
    config->Row_Offset = (240 - config->LCD_Height) / 2;
  } else {
    // Portrait Mode (Normal axes, Physical Max: 240x320)
    config->Col_Offset = (240 - config->LCD_Width) / 2;
    config->Row_Offset = (320 - config->LCD_Height) / 2;
  }

  // Send Memory Data Access Control (MADCTL) Command
  WriteCommand(config, MADCTL);
  WriteData(config, madctl_value);
}

void DrawString(ST7789V3_Config *config, uint16_t x, uint16_t y,
                const char *str, uint32_t hexcolor, const FontDef *font) {
  uint16_t start_x = x;
  // until null terminator
  while (*str) {

    if (*str == '\n') {
      // Handle newline explicitly: go to next line and reset X
      y += font->height;
      x = start_x;

    } else if (*str == '\r') {
      // Carriage return just resets X
      x = start_x;

    } else {

      // Line wrap if the next character exceeds LCD width
      if (x + font->width > config->LCD_Width) {
        y += font->height;
        x = start_x;
      }

      DrawChar(config, x, y, *str, hexcolor, font);
      x += font->width;
    }
    str++;
  }
}

// =============== Shape Drawing Functions ================

/**
 * @brief Helper to draw a horizontal line efficiently using
 * More efficient than calling DrawPixel in a loop or Drawline
 */

static void DrawHLine(ST7789V3_Config *config, uint16_t x, uint16_t y,
                      uint16_t length, uint32_t hexcolor) {
  if (y >= config->LCD_Height || x >= config->LCD_Width)
    return;

  // Clamp length to display boundary (trim any overflow)
  if (x + length > config->LCD_Width) {
    length = config->LCD_Width - x;
  }

  SetWindow(config, x, x + length - 1, y, y);

  // Convert to RGB565
  uint16_t color565 = ConvertRGB888toRGB565(hexcolor);

  uint8_t color565_High = (color565 >> 8) & 0xFFU;
  uint8_t color565_Low = color565 & 0xFFU;

  config->set_dc(DATA);
  config->set_cs(LOW);
  for (uint16_t i = 0; i < length; i++) {
    config->spi_write(1, &color565_High);
    config->spi_write(1, &color565_Low);
  }
  config->set_cs(HIGH);
}

void DrawLine(ST7789V3_Config *config, uint16_t x0, uint16_t y0, uint16_t x1,
              uint16_t y1, uint32_t hexcolor) {

  // Bresenham's line algorithm using integer-only arithmetic
  // This completely avoids floating point math that most MCUs aren;t efficient
  // at due to lack of FPU
  int16_t dx = (x1 > x0) ? (int16_t)(x1 - x0) : -(int16_t)(x0 - x1);
  int16_t dy = -((y1 > y0) ? (int16_t)(y1 - y0) : -(int16_t)(y0 - y1));

  int16_t sx = (x0 < x1) ? 1 : -1;
  int16_t sy = (y0 < y1) ? 1 : -1;

  int16_t err = dx + dy;

  int16_t cx = (int16_t)x0;
  int16_t cy = (int16_t)y0;

  while (1) {
    DrawPixel(config, (uint16_t)cx, (uint16_t)cy, hexcolor);

    if (cx == (int16_t)x1 && cy == (int16_t)y1)
      break;

    int16_t e2 = 2 * err;

    if (e2 >= dy) {
      err += dy;
      cx += sx;
    }
    if (e2 <= dx) {
      err += dx;
      cy += sy;
    }
  }
}

void DrawRectangle(ST7789V3_Config *config, uint16_t x, uint16_t y,
                   uint16_t width, uint16_t height, uint32_t hexcolor) {

  if (x >= config->LCD_Width || y >= config->LCD_Height)
    return;
  if (x + width > config->LCD_Width) {
    width = config->LCD_Width - x;
  }
  if (y + height > config->LCD_Height) {
    height = config->LCD_Height - y;
  }

  SetWindow(config, x, x + width - 1, y, y);

  DrawHLine(config, x, y, width, hexcolor);
  DrawLine(config, x, y, x, y + height - 1, hexcolor);
  DrawHLine(config, x, y + height - 1, width, hexcolor);
  DrawLine(config, x + width - 1, y, x + width - 1, y + height - 1, hexcolor);
}

void DrawFilledRectangle(ST7789V3_Config *config, uint16_t x, uint16_t y,
                         uint16_t width, uint16_t height, uint32_t hexcolor) {
  // Clamp to display boundaries
  if (x >= config->LCD_Width || y >= config->LCD_Height)
    return;

  if (x + width > config->LCD_Width) {
    width = config->LCD_Width - x;
  }
  if (y + height > config->LCD_Height) {
    height = config->LCD_Height - y;
  }

  // Set the entire rectangle as the draw window, then bulk write
  SetWindow(config, x, x + width - 1, y, y + height - 1);

  for (uint16_t i = 0; i < height; i++) {
    DrawHLine(config, x, y + i, width, hexcolor);
  }
}